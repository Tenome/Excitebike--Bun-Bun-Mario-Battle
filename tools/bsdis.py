#!/usr/bin/env python3
"""Recursive-descent 65816 disassembler for the Excitebike BS (Satellaview) ROMs.

The .bs images are 1 MB LoROM memory-pack images.  When the BS-X BIOS boots
one, the pack is visible as LoROM at $80-$9F:8000-FFFF (and the $00-$1F
mirror), which is where the game's own code runs.

Usage:
    python tools/bsdis.py orig/s1.bs --out build/s1      # report + listing
    python tools/bsdis.py orig/s1.bs --entry 80:8000      # extra roots

Outputs (in --out):
    report.txt   coverage, hardware registers, BS-X accesses, unresolved jumps
    listing.asm  annotated listing of every reached instruction, by bank
    code.json    map of code entries (pc -> M/X) for other tools
"""
import argparse
import collections
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from op65816 import OPS, BRANCHES, operand_size  # noqa: E402

# Well-known hardware registers (SNES + BS-X).
HWREG = {
    0x2100: 'INIDISP', 0x2101: 'OBSEL', 0x2102: 'OAMADDL', 0x2103: 'OAMADDH',
    0x2104: 'OAMDATA', 0x2105: 'BGMODE', 0x2106: 'MOSAIC', 0x2107: 'BG1SC',
    0x2108: 'BG2SC', 0x2109: 'BG3SC', 0x210A: 'BG4SC', 0x210B: 'BG12NBA',
    0x210C: 'BG34NBA', 0x210D: 'BG1HOFS', 0x210E: 'BG1VOFS', 0x210F: 'BG2HOFS',
    0x2110: 'BG2VOFS', 0x2111: 'BG3HOFS', 0x2112: 'BG3VOFS', 0x2113: 'BG4HOFS',
    0x2114: 'BG4VOFS', 0x2115: 'VMAIN', 0x2116: 'VMADDL', 0x2117: 'VMADDH',
    0x2118: 'VMDATAL', 0x2119: 'VMDATAH', 0x211A: 'M7SEL', 0x211B: 'M7A',
    0x211C: 'M7B', 0x211D: 'M7C', 0x211E: 'M7D', 0x211F: 'M7X', 0x2120: 'M7Y',
    0x2121: 'CGADD', 0x2122: 'CGDATA', 0x2123: 'W12SEL', 0x2124: 'W34SEL',
    0x2125: 'WOBJSEL', 0x2126: 'WH0', 0x2127: 'WH1', 0x2128: 'WH2', 0x2129: 'WH3',
    0x212A: 'WBGLOG', 0x212B: 'WOBJLOG', 0x212C: 'TM', 0x212D: 'TS', 0x212E: 'TMW',
    0x212F: 'TSW', 0x2130: 'CGWSEL', 0x2131: 'CGADSUB', 0x2132: 'COLDATA',
    0x2133: 'SETINI', 0x2134: 'MPYL', 0x2135: 'MPYM', 0x2136: 'MPYH',
    0x2137: 'SLHV', 0x2138: 'OAMDATAREAD', 0x2139: 'VMDATALREAD',
    0x213A: 'VMDATAHREAD', 0x213B: 'CGDATAREAD', 0x213C: 'OPHCT', 0x213D: 'OPVCT',
    0x213E: 'STAT77', 0x213F: 'STAT78', 0x2140: 'APUIO0', 0x2141: 'APUIO1',
    0x2142: 'APUIO2', 0x2143: 'APUIO3', 0x2180: 'WMDATA', 0x2181: 'WMADDL',
    0x2182: 'WMADDM', 0x2183: 'WMADDH',
    0x4016: 'JOYSER0', 0x4017: 'JOYSER1', 0x4200: 'NMITIMEN', 0x4201: 'WRIO',
    0x4202: 'WRMPYA', 0x4203: 'WRMPYB', 0x4204: 'WRDIVL', 0x4205: 'WRDIVH',
    0x4206: 'WRDIVB', 0x4207: 'HTIMEL', 0x4208: 'HTIMEH', 0x4209: 'VTIMEL',
    0x420A: 'VTIMEH', 0x420B: 'MDMAEN', 0x420C: 'HDMAEN', 0x420D: 'MEMSEL',
    0x4210: 'RDNMI', 0x4211: 'TIMEUP', 0x4212: 'HVBJOY', 0x4213: 'RDIO',
    0x4214: 'RDDIVL', 0x4215: 'RDDIVH', 0x4216: 'RDMPYL', 0x4217: 'RDMPYH',
    0x4218: 'JOY1L', 0x4219: 'JOY1H', 0x421A: 'JOY2L', 0x421B: 'JOY2H',
    0x421C: 'JOY3L', 0x421D: 'JOY3H', 0x421E: 'JOY4L', 0x421F: 'JOY4H',
}
for _ch in range(8):
    for _r, _n in enumerate(['DMAP', 'BBAD', 'A1TL', 'A1TH', 'A1B', 'DASL', 'DASH',
                             'DASB', 'A2AL', 'A2AH', 'NTRL']):
        HWREG[0x4300 + _ch * 16 + _r] = '%s%d' % (_n, _ch)
# BS-X satellite receiver registers (bus B, $2188-$219F).
for _r in range(0x2188, 0x21A0):
    HWREG[_r] = 'BSX_%04X' % _r


def is_bsx_reg(a16):
    return 0x2188 <= a16 <= 0x219F


class Rom:
    def __init__(self, data):
        self.data = data

    def off(self, addr):
        """LoROM file offset for a 24-bit address, or None if not ROM."""
        bank, a = addr >> 16, addr & 0xFFFF
        b = bank & 0x7F
        if b >= 0x7E:
            return None
        if a < 0x8000:
            return None
        o = ((b & 0x3F) << 15) | (a & 0x7FFF)
        if b >= 0x40:
            o = ((b - 0x40) << 15) | (a & 0x7FFF)
        if o >= len(self.data):
            return None
        return o

    def byte(self, addr):
        o = self.off(addr)
        return None if o is None else self.data[o]

    def word(self, addr):
        lo, hi = self.byte(addr), self.byte((addr & 0xFF0000) | ((addr + 1) & 0xFFFF))
        if lo is None or hi is None:
            return None
        return lo | hi << 8


def parse_addr(s):
    s = s.replace('$', '').replace(':', '')
    return int(s, 16)


class Disasm:
    def __init__(self, rom):
        self.rom = rom
        self.insn = {}              # pc -> (mnem, mode, size, operand, m, x)
        self.flags_at = collections.defaultdict(set)  # pc -> {(m,x)}
        self.funcs = {}             # entry pc -> set of (m,x)
        self.callers = collections.defaultdict(set)
        self.labels = set()
        self.unresolved = []        # (pc, text)
        self.hw = collections.defaultdict(lambda: collections.Counter())  # reg -> {R/W: n}
        self.hw_sites = collections.defaultdict(set)
        self.ext_long = collections.defaultdict(set)  # long target outside ROM -> sites
        self.conflicts = set()
        self.queue = []
        self.jumptables = {}        # site -> (table addr, [targets])
        self.names = {}
        self.resolved = set()       # indirect sites explained by hints
        self.seed_bases = set()     # table starts learned in a previous pass
        self.ptables = {}           # site -> (table addr, m, x), resolved after tracing

    # ------------------------------------------------------------------
    def add_root(self, pc, m=True, x=True, func=True, src=None):
        if func:
            self.funcs.setdefault(pc, set()).add((m, x))
            if src is not None:
                self.callers[pc].add(src)
        self.labels.add(pc)
        self.queue.append((pc, m, x, ()))

    def run(self):
        done = set()
        while True:
            while self.queue:
                pc, m, x, stk = self.queue.pop()
                self.trace(pc, m, x, stk)
            new = [s for s in self.ptables if s not in done]
            if not new:
                break
            bases = {t for t, _, _ in self.ptables.values()} | self.data_bases() | self.seed_bases
            bases = sorted(b for b in bases if not any(b == t + 1 for t, _, _ in self.ptables.values()))
            for site in new:
                done.add(site)
                t, m, x = self.ptables[site]
                limit = min([b for b in bases if b > t] + [t + 256])
                tgts, a = [], t
                while a + 1 < limit and a not in self.insn:
                    if any(tg > t and a >= tg for tg in tgts):
                        break
                    w = self.rom.word(a)
                    if w is None or w < 0x8000 or not self.plausible((t & 0xFF0000) | w, m, x):
                        break
                    tgts.append((t & 0xFF0000) | w)
                    a += 2
                self.jumptables[site] = (t, tgts)
                for tg in tgts:
                    self.add_root(tg, m, x, src=site)

    def trace(self, pc, m, x, stk):
        hist = []
        while True:
            if (m, x) in self.flags_at[pc]:
                return
            op = self.rom.byte(pc)
            if op is None:
                self.unresolved.append((pc, 'execution leaves ROM'))
                return
            mnem, mode = OPS[op]
            size = 1 + operand_size(mode, m, x)
            raw = [self.rom.byte(((pc & 0xFF0000) | ((pc + i) & 0xFFFF))) for i in range(1, size)]
            if any(b is None for b in raw):
                return
            operand = 0
            for i, b in enumerate(raw):
                operand |= b << (8 * i)
            # overlap check
            for i in range(1, size):
                q = (pc & 0xFF0000) | ((pc + i) & 0xFFFF)
                if q in self.insn:
                    self.conflicts.add(pc)
            self.flags_at[pc].add((m, x))
            self.insn[pc] = (mnem, mode, size, operand, m, x)
            hist = (hist + [(pc, mnem, mode, operand)])[-32:]
            bank = pc & 0xFF0000
            nxt = bank | ((pc + size) & 0xFFFF)

            self.note_mem(pc, mnem, mode, operand)

            if mnem == 'REP':
                if operand & 0x20: m = False
                if operand & 0x10: x = False
            elif mnem == 'SEP':
                if operand & 0x20: m = True
                if operand & 0x10: x = True
            elif mnem == 'PHP':
                stk = (stk + ((m, x),))[-8:]
            elif mnem == 'PLP':
                if stk:
                    m, x = stk[-1]
                    stk = stk[:-1]
                else:
                    self.unresolved.append((pc, 'PLP with unknown flags (assumed unchanged)'))

            if mnem in BRANCHES or mnem == 'BRA':
                tgt = bank | ((nxt + (operand - 256 if operand & 0x80 else operand)) & 0xFFFF)
                self.labels.add(tgt)
                self.queue.append((tgt, m, x, stk))
                if mnem == 'BRA':
                    return
            elif mnem == 'BRL':
                tgt = bank | ((nxt + (operand - 0x10000 if operand & 0x8000 else operand)) & 0xFFFF)
                self.labels.add(tgt)
                self.queue.append((tgt, m, x, stk))
                return
            elif mnem == 'JSR' and mode == 'abs' or mnem == 'JSL' and self.rom.off(operand) is not None:
                tgt = bank | operand if mnem == 'JSR' else operand
                self.add_root(tgt, m, x, src=pc)
                exits = self.exit_states(tgt, m, x)
                if exits and exits != {(m, x)}:
                    # callee changes width: continue after the call in each exit width
                    ex = sorted(exits)
                    for em, exx in ex[1:]:
                        self.queue.append((nxt, em, exx, stk))
                    m, x = ex[0]
            elif mnem == 'JSL':
                self.ext_long[operand].add(pc)
            elif mnem == 'JMP' and mode == 'abs':
                tgt = bank | operand
                self.labels.add(tgt)
                self.queue.append((tgt, m, x, stk))
                return
            elif mnem == 'JML' and mode == 'long':
                if self.rom.off(operand) is None:
                    self.ext_long[operand].add(pc)
                else:
                    self.labels.add(operand)
                    self.queue.append((operand, m, x, stk))
                return
            elif mnem in ('JMP', 'JSR') and mode == 'iabsx':
                self.jump_table(pc, bank | operand, m, x, call=(mnem == 'JSR'))
                if mnem == 'JMP':
                    return
            elif mnem == 'JMP' and mode == 'iabs' and self.ptr_table(pc, hist, operand, m, x):
                return
            elif mnem in ('JMP', 'JML') and mode in ('iabs', 'ilabs'):
                self.unresolved.append((pc, '%s indirect via $%04X' % (mnem, operand)))
                return
            elif mnem in ('RTS', 'RTL', 'RTI', 'STP'):
                return
            elif mnem in ('BRK', 'COP'):
                self.unresolved.append((pc, '%s #$%02X' % (mnem, operand)))
            pc = nxt

    def exit_states(self, entry, m, x, _depth=0):
        """Set of (m, x) at the routine's RTS/RTL when entered with (m, x).

        Walks the body on its own (branches, JMP/BRL within the bank, calls
        summarised recursively). Empty set = no return found (or unknown);
        callers then assume the width is preserved."""
        key = (entry, m, x)
        memo = self.__dict__.setdefault('_exit_memo', {})
        if key in memo:
            return memo[key]
        memo[key] = set()               # recursion guard
        if _depth > 24:
            return set()
        out, seen, work = set(), set(), [(entry, m, x, ())]
        steps = 0
        while work and steps < 20000:
            pc, cm, cx, stk = work.pop()
            while steps < 20000:
                steps += 1
                if (pc, cm, cx, stk) in seen:
                    break
                seen.add((pc, cm, cx, stk))
                op = self.rom.byte(pc)
                if op is None:
                    break
                mn, mode = OPS[op]
                size = 1 + operand_size(mode, cm, cx)
                operand = 0
                for i in range(1, size):
                    b = self.rom.byte((pc & 0xFF0000) | ((pc + i) & 0xFFFF))
                    if b is None:
                        break
                    operand |= b << (8 * (i - 1))
                bank = pc & 0xFF0000
                nxt = bank | ((pc + size) & 0xFFFF)
                if mn == 'REP':
                    cm = cm and not operand & 0x20; cx = cx and not operand & 0x10
                elif mn == 'SEP':
                    cm = cm or bool(operand & 0x20); cx = cx or bool(operand & 0x10)
                elif mn == 'PHP':
                    stk = (stk + ((cm, cx),))[-8:]
                elif mn == 'PLP':
                    if stk:
                        cm, cx = stk[-1]; stk = stk[:-1]
                if mn in ('RTS', 'RTL'):
                    out.add((cm, cx)); break
                if mn in ('RTI', 'STP', 'BRK', 'COP') or (mn in ('JMP', 'JML') and mode in ('iabs', 'iabsx', 'ilabs')):
                    break
                if mn in BRANCHES:
                    work.append((bank | ((nxt + (operand - 256 if operand & 0x80 else operand)) & 0xFFFF), cm, cx, stk))
                elif mn == 'BRA':
                    pc = bank | ((nxt + (operand - 256 if operand & 0x80 else operand)) & 0xFFFF); continue
                elif mn == 'BRL':
                    pc = bank | ((nxt + (operand - 0x10000 if operand & 0x8000 else operand)) & 0xFFFF); continue
                elif mn == 'JMP' and mode == 'abs':
                    pc = bank | operand; continue
                elif mn == 'JML' and mode == 'long':
                    if self.rom.off(operand) is None:
                        break
                    pc = operand; continue
                elif (mn == 'JSR' and mode == 'abs') or (mn == 'JSL' and self.rom.off(operand) is not None):
                    t = bank | operand if mn == 'JSR' else operand
                    ex = self.exit_states(t, cm, cx, _depth + 1)
                    if ex and ex != {(cm, cx)}:
                        ex = sorted(ex)
                        for em, exx in ex[1:]:
                            work.append((nxt, em, exx, stk))
                        cm, cx = ex[0]
                pc = nxt
        memo[key] = out
        return out

    def plausible(self, pc, m, x, n=4):
        """Cheap sanity check that pc starts real code."""
        for _ in range(n):
            op = self.rom.byte(pc)
            if op is None or op in (0x00, 0x02, 0x42, 0xDB, 0x01, 0xFF):
                return False
            mn, mode = OPS[op]
            if mn in ('RTS', 'RTL', 'RTI', 'JMP', 'JML', 'BRA', 'BRL'):
                return True
            if mn == 'REP':
                b = self.rom.byte(pc + 1)
                m = m and not (b & 0x20); x = x and not (b & 0x10)
            if mn == 'SEP':
                b = self.rom.byte(pc + 1)
                m = m or bool(b & 0x20); x = x or bool(b & 0x10)
            pc = (pc & 0xFF0000) | ((pc + 1 + operand_size(mode, m, x)) & 0xFFFF)
        return True

    def data_bases(self):
        """Addresses used as abs,X / abs,Y / long,X bases: table starts."""
        out = set()
        for pc, (mn, mode, size, op, m, x) in self.insn.items():
            if mode == 'iabsx':
                out.add((pc & 0xFF0000) | op)
                continue
            if mn in ('JMP', 'JSR'):
                continue
            if mode in ('absx', 'absy') and op >= 0x8000:
                out.add((pc & 0xFF0000) | op)
            elif mode == 'longx' and self.rom.off(op) is not None:
                out.add(op)
        return out

    def ptr_table(self, site, hist, ptr, m, x):
        """Recognise 'LDA tbl,Y / STA ptr / ... LDA tbl,Y / STA ptr+1 / JMP (ptr)'
        and 'LDA tbl,X (16-bit) / STA ptr / JMP (ptr)'."""
        base = hi_base = None
        for i in range(len(hist) - 1):
            a, b = hist[i], hist[i + 1]
            if a[1] == 'LDA' and a[2] in ('absx', 'absy') and b[1] == 'STA'                     and b[2] in ('dp', 'abs') and b[3] == ptr:
                base = a[3]
            if a[1] == 'LDA' and a[2] in ('absx', 'absy') and b[1] == 'STA'                     and b[2] in ('dp', 'abs') and b[3] == ptr + 1:
                hi_base = a[3]
        if base is None:
            return self.ptr_table2(site, hist, ptr, m, x)
        bank = site & 0xFF0000
        if m and hi_base is not None and hi_base != base + 1 and 0 < hi_base - base <= 64:
            # split table: low bytes at base, high bytes at hi_base
            n = hi_base - base
            tg = []
            for i in range(n):
                lo, hi = self.rom.byte(bank | (base + i)), self.rom.byte(bank | (hi_base + i))
                t = bank | lo | hi << 8
                if (t & 0xFFFF) < 0x8000 or not self.plausible(t, m, x):
                    continue
                tg.append(t)
            self.jumptables[site] = (bank | base, tg)
            self.resolved.add(site)
            for t in tg:
                self.add_root(t, m, x, src=site)
            return True
        self.ptables[site] = (bank | base, m, x)
        return True

    def ptr_table2(self, site, hist, ptr, m, x):
        """'LDA tbl,X / STA dp / LDA tbl+1,X / STA dp+1 ... LDA (dp),Y / STA ptr / JMP (ptr)':
        tbl holds pointers to lists of handler pointers."""
        dp = None
        for i in range(len(hist) - 1):
            a, b = hist[i], hist[i + 1]
            if a[1] == 'LDA' and a[2] == 'idpy' and b[1] == 'STA' and b[3] == ptr:
                dp = a[3]
        if dp is None:
            return False
        outer = None
        for i in range(len(hist) - 1):
            a, b = hist[i], hist[i + 1]
            if a[1] == 'LDA' and a[2] in ('absx', 'absy') and b[1] == 'STA'                     and b[2] == 'dp' and b[3] == dp:
                outer = a[3]
        if outer is None:
            return False
        bank = site & 0xFF0000
        base = bank | outer
        lists, a = [], base
        while not lists or a < min(lists):
            w = self.rom.word(a)
            if w is None or w < 0x8000 or len(lists) > 256:
                break
            lists.append(bank | w)
            a += 2
        starts = sorted(set(lists))
        bounds = starts[1:] + [starts[-1] + 0x100] if starts else []
        tg = []
        for st, en in zip(starts, bounds):
            q = st
            while q < en:
                w = self.rom.word(q)
                if w is None or w < 0x8000 or not self.plausible(bank | w, m, x):
                    break
                tg.append(bank | w)
                q += 2
        self.jumptables[site] = (base, tg)
        self.resolved.add(site)
        for t in tg:
            self.add_root(t, m, x, src=site)
        return True

    def jump_table(self, site, table, m, x, call):
        """Heuristic: walk 16-bit pointers in the program bank."""
        bank = table & 0xFF0000
        tgts = []
        a = table
        for _ in range(128):
            if a in self.insn or (a in self.labels and a != table) or (a != table and a in self.seed_bases):
                break
            if any(t > table and a >= t for t in tgts):
                break       # walked into the first handler
            w = self.rom.word(a)
            if w is None or w < 0x8000 or not self.plausible(bank | w, m, x):
                break
            tgts.append(bank | w)
            a = bank | ((a + 2) & 0xFFFF)
        self.jumptables[site] = (table, tgts)
        if not tgts:
            self.unresolved.append((site, 'jump table at $%06X: no entries' % table))
        for t in tgts:
            self.add_root(t, m, x, func=True, src=site)

    def note_mem(self, pc, mnem, mode, operand):
        if mode in ('abs', 'absx', 'absy'):
            if mnem in ('JMP', 'JSR', 'PEA'):
                return
            a16 = operand
            if 0x2100 <= a16 < 0x2200 or 0x4000 <= a16 < 0x4400 or 0x5000 <= a16 < 0x6000:
                rw = 'W' if mnem in ('STA', 'STX', 'STY', 'STZ') else 'R'
                self.hw[a16][rw] += 1
                self.hw_sites[a16].add(pc)
        elif mode in ('long', 'longx'):
            if mnem in ('JSL', 'JML'):
                return
            bank, a16 = operand >> 16, operand & 0xFFFF
            if (bank & 0x7F) < 0x40 and a16 < 0x8000:
                if 0x2100 <= a16 < 0x2200 or 0x4000 <= a16 < 0x4400 or 0x5000 <= a16 < 0x6000:
                    rw = 'W' if mnem in ('STA', 'STZ') else 'R'
                    self.hw[a16][rw] += 1
                    self.hw_sites[a16].add(pc)
            elif self.rom.off(operand) is None and bank not in (0x7E, 0x7F):
                self.ext_long[operand].add(pc)

    # ------------------------------------------------------------------
    def fmt_operand(self, pc, mnem, mode, size, operand):
        n = size - 1
        bank = pc & 0xFF0000
        h = lambda v, w: '$%0*X' % (w, v)  # noqa: E731
        if mode in ('imp',):
            return ''
        if mode == 'acc':
            return 'A'
        if mode in ('immm', 'immx', 'imm8'):
            return '#' + h(operand, 2 * n)
        if mode == 'rel':
            t = bank | ((pc + 2 + (operand - 256 if operand & 0x80 else operand)) & 0xFFFF)
            return self.lab(t)
        if mode == 'rell':
            t = bank | ((pc + 3 + (operand - 0x10000 if operand & 0x8000 else operand)) & 0xFFFF)
            return self.lab(t)
        if mode == 'bm':
            return '$%02X,$%02X' % (operand & 0xFF, operand >> 8)
        dp = h(operand, 2)
        ab = h(operand, 4)
        lg = h(operand, 6)
        if mnem in ('JSR', 'JMP') and mode == 'abs':
            ab = self.lab(bank | operand)
        if mnem in ('JSL', 'JML') and mode == 'long':
            lg = self.lab(operand) if self.rom.off(operand) is not None else lg
        if mode == 'abs' and operand in HWREG:
            ab = HWREG[operand]
        return {
            'dp': dp, 'dpx': dp + ',X', 'dpy': dp + ',Y', 'idp': '(' + dp + ')',
            'idpx': '(' + dp + ',X)', 'idpy': '(' + dp + '),Y', 'ildp': '[' + dp + ']',
            'ildpy': '[' + dp + '],Y', 'sr': dp + ',S', 'isry': '(' + dp + ',S),Y',
            'abs': ab, 'absx': ab + ',X', 'absy': ab + ',Y', 'long': lg,
            'longx': lg + ',X', 'iabs': '(' + ab + ')', 'iabsx': '(' + ab + ',X)',
            'ilabs': '[' + ab + ']',
        }[mode]

    def lab(self, a):
        if a in self.names:
            return self.names[a]
        if a in self.funcs:
            return 'sub_%06X' % a
        return 'L%06X' % a

    def listing(self):
        out = []
        prev_end = None
        for pc in sorted(self.insn):
            mnem, mode, size, operand, m, x = self.insn[pc]
            if prev_end is not None and pc != prev_end:
                out.append('')
            if pc in self.funcs:
                fl = ' '.join('M%dX%d' % (int(a), int(b)) for a, b in sorted(self.funcs[pc]))
                callers = sorted(self.callers.get(pc, ()))
                cs = ', '.join('$%06X' % c for c in callers[:6])
                if len(callers) > 6:
                    cs += ', ... (%d)' % len(callers)
                out.append('')
                out.append('; ---- %s  [%s]  callers: %s' % (self.lab(pc), fl, cs or '-'))
            if pc in self.labels or pc in self.funcs:
                out.append('%s:' % self.lab(pc))
            raw = ' '.join('%02X' % self.rom.byte((pc & 0xFF0000) | ((pc + i) & 0xFFFF)) for i in range(size))
            ops = self.fmt_operand(pc, mnem, mode, size, operand)
            if mode in ('immm', 'immx'):
                mn = mnem + ('.b' if size == 2 else '.w')
            else:
                mn = mnem
            comment = ''
            if len(self.flags_at[pc]) > 1:
                comment = ' ; !! multiple M/X: %s' % sorted(self.flags_at[pc])
            if pc in self.jumptables:
                t, tg = self.jumptables[pc]
                comment += ' ; table $%06X: %s' % (t, ' '.join('%04X' % (v & 0xFFFF) for v in tg))
            out.append('  %06X  %-12s  %-4s %-18s%s' % (pc, raw, mn, ops, comment))
            prev_end = (pc & 0xFF0000) | ((pc + size) & 0xFFFF)
        return '\n'.join(out) + '\n'

    def report(self):
        r = []
        nbytes = sum(v[2] for v in self.insn.values())
        r.append('instructions: %d   code bytes: %d   functions: %d' % (len(self.insn), nbytes, len(self.funcs)))
        banks = collections.Counter((pc >> 16) for pc in self.insn)
        r.append('per bank: ' + ' '.join('$%02X:%d' % (b, n) for b, n in sorted(banks.items())))
        r.append('')
        r.append('== BS-X / unusual hardware accesses')
        for a in sorted(self.hw):
            if is_bsx_reg(a) or 0x5000 <= a < 0x6000:
                sites = ' '.join('$%06X' % s for s in sorted(self.hw_sites[a]))
                r.append('  $%04X %-10s %s  at %s' % (a, HWREG.get(a, ''), dict(self.hw[a]), sites))
        r.append('')
        r.append('== Long references outside the pack (BIOS / other banks)')
        for a in sorted(self.ext_long):
            r.append('  $%06X  from %s' % (a, ' '.join('$%06X' % s for s in sorted(self.ext_long[a]))))
        r.append('')
        r.append('== Hardware registers used')
        for a in sorted(self.hw):
            r.append('  $%04X %-12s %s' % (a, HWREG.get(a, '?'), dict(self.hw[a])))
        r.append('')
        r.append('== Unresolved control flow (%d)' % len(self.unresolved))
        for pc, t in sorted(set(self.unresolved)):
            if pc in self.resolved:
                continue
            r.append('  $%06X  %s' % (pc, t))
        r.append('')
        r.append('== Jump tables (heuristic)')
        for s, (t, tg) in sorted(self.jumptables.items()):
            r.append('  $%06X -> table $%06X  %d entries' % (s, t, len(tg)))
        r.append('')
        r.append('== Overlapping instruction starts: %d' % len(self.conflicts))
        for pc in sorted(self.conflicts)[:50]:
            r.append('  $%06X' % pc)
        r.append('')
        top = sorted(self.callers.items(), key=lambda kv: -len(kv[1]))[:30]
        r.append('== Most-called functions')
        for pc, cs in top:
            r.append('  sub_%06X  %d callers' % (pc, len(cs)))
        return '\n'.join(r) + '\n'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('rom')
    ap.add_argument('--out', default=None)
    ap.add_argument('--entry', action='append', default=[],
                    help='extra root, e.g. 80:8000 or 80:8000:m0x1')
    ap.add_argument('--no-tables', action='store_true')
    ap.add_argument('--hints', help='hints file (default config/<ver>/hints.txt)')
    ap.add_argument('--names', help='names file (default config/<ver>/names.txt)')
    args = ap.parse_args()

    data = open(args.rom, 'rb').read()
    rom = Rom(data)
    seed = set()
    for _pass in range(6):
        d = build(rom, args, seed)
        new = seed | d.data_bases() | {t for t, _, _ in d.ptables.values()}
        if new == seed:
            break
        seed = new
    finish(d, args)


def build(rom, args, seed):
    d = Disasm(rom)
    d.seed_bases = set(seed)
    if args.no_tables:
        d.jump_table = lambda site, table, m, x, call: d.unresolved.append((site, 'jump table $%06X' % table))

    # Vectors live in the BS header area at $80:FFE0 (file $7FE0).
    vec = {'COP': 0xFFE4, 'BRK': 0xFFE6, 'NMI': 0xFFEA, 'IRQ': 0xFFEE, 'RESET': 0xFFFC}
    for name, v in vec.items():
        tgt = rom.word(0x800000 | v)
        d.add_root(0x800000 | tgt, True, True)
    for e in args.entry:
        parts = e.split(':')
        pc = int(parts[0] + parts[1], 16)
        m = x = True
        if len(parts) > 2:
            f = parts[2].lower()
            m = 'm0' not in f
            x = 'x0' not in f
        d.add_root(pc, m, x)
    apply_hints(d, rom, args)
    d.run()
    return d


def _mx(tok):
    tok = (tok or 'm1x1').lower()
    return 'm0' not in tok, 'x0' not in tok


def apply_hints(d, rom, args):
    """Hints file grammar (one per line, # comments, addresses are 24-bit hex):
         code  <pc> [mXxY]                 extra code entry
         table <addr> <count> [mXxY]       table of 16-bit code pointers (same bank)
         table2 <addr> <count> [mXxY]      table of pointers to lists of code pointers
         ltable <addr> <count> [mXxY]      table of 24-bit code pointers
         data  <start> <end>               never decode as code (not enforced yet)
    """
    path = args.hints or os.path.join('config', os.path.splitext(os.path.basename(args.rom))[0], 'hints.txt')
    if not os.path.exists(path):
        return
    for line in open(path):
        t = line.split('#')[0].split()
        if not t:
            continue
        kind = t[0]
        if kind == 'resolved':
            d.resolved.add(int(t[1], 16))
        elif kind == 'code':
            m, x = _mx(t[2] if len(t) > 2 else None)
            d.add_root(int(t[1], 16), m, x)
        elif kind in ('table', 'table2', 'ltable'):
            base, n = int(t[1], 16), int(t[2], 0)
            m, x = _mx(t[3] if len(t) > 3 else None)
            bank = base & 0xFF0000
            if kind == 'table':
                for i in range(n):
                    d.add_root(bank | rom.word(base + 2 * i), m, x, src=base)
            elif kind == 'ltable':
                for i in range(n):
                    a = base + 3 * i
                    d.add_root(rom.word(a) | rom.byte(a + 2) << 16, m, x, src=base)
            else:
                lists = sorted({bank | rom.word(base + 2 * i) for i in range(n)})
                bounds = lists[1:] + [lists[-1] + 0x100]
                for st, en in zip(lists, bounds):
                    a = st
                    while a < en:
                        w = rom.word(a)
                        if w is None or w < 0x8000 or not d.plausible(bank | w, m, x):
                            break
                        d.add_root(bank | w, m, x, src=st)
                        a += 2


def finish(d, args):
    npath = args.names or os.path.join('config', os.path.splitext(os.path.basename(args.rom))[0], 'names.txt')
    if os.path.exists(npath):
        for line in open(npath):
            t = line.split('#')[0].split()
            if len(t) >= 2:
                d.names[int(t[0], 16)] = t[1]

    out = args.out or os.path.join('build', os.path.splitext(os.path.basename(args.rom))[0])
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, 'report.txt'), 'w') as f:
        f.write(d.report())
    with open(os.path.join(out, 'listing.asm'), 'w') as f:
        f.write(d.listing())
    with open(os.path.join(out, 'code.json'), 'w') as f:
        json.dump({'jt': {'%06X' % site: {'table': '%06X' % t, 'targets': ['%06X' % x for x in tg]}
                          for site, (t, tg) in d.jumptables.items()},
                   'insn': {'%06X' % k: [v[0], v[1], v[2], v[3]] for k, v in d.insn.items()},
                   'mx': {'%06X' % k: sorted([int(a) * 2 + int(b) for a, b in d.flags_at[k]]) for k in d.insn},
                   'funcs': {'%06X' % k: sorted(['M%dX%d' % (int(a), int(b)) for a, b in v])
                             for k, v in d.funcs.items()}}, f, indent=0)
    print(d.report().split('\n== Hardware')[0])


if __name__ == '__main__':
    main()
