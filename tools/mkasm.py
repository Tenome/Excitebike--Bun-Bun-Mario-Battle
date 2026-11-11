#!/usr/bin/env python3
"""Emit byte-matching ca65 source for a .bs ROM from bsdis.py's code map.

    python tools/mkasm.py orig/s1.bs build/s1/code.json asm/s1 [--names config/s1/names.txt]
    python tools/build.py s1           # assemble + link + compare (see tools/build.py)

Writes asm/<ver>/bankNN.s (one per 32 KB LoROM bank), asm/<ver>/link.cfg and
asm/<ver>/main.inc. Code is emitted with explicit address-size prefixes and
.a8/.a16/.i8/.i16 state so the output encodes byte-for-byte; everything
else is .byte data. Labels are local to each bank file; cross-bank operands
stay numeric (they will become imported symbols when banks are split into
modules).
"""
import argparse
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bsdis import HWREG  # noqa: E402

BANK_SIZE = 0x8000


def load_names(path):
    names = {}
    if path and os.path.exists(path):
        for line in open(path):
            t = line.split('#')[0].split()
            if len(t) >= 2:
                names[int(t[0], 16)] = t[1]
    return names


class Emitter:
    def __init__(self, rom, code, names):
        self.rom = rom
        self.insn = {int(k, 16): v for k, v in code['insn'].items()}
        self.mx = {}
        for k, v in code.get('mx', {}).items():
            self.mx[int(k, 16)] = v
        self.funcs = {int(k, 16) for k in code['funcs']}
        self.names = names
        self.labels = set(self.funcs)
        self.placed = set()     # labels actually emitted (filled by a first pass)
        # branch / jump targets become labels
        for pc, (mn, mode, size, op) in self.insn.items():
            t = self.target(pc, mn, mode, size, op)
            if t is not None and t in self.insn:
                self.labels.add(t)

    @staticmethod
    def target(pc, mn, mode, size, op):
        bank = pc & 0xFF0000
        if mode == 'rel':
            return bank | ((pc + 2 + (op - 256 if op & 0x80 else op)) & 0xFFFF)
        if mode == 'rell':
            return bank | ((pc + 3 + (op - 0x10000 if op & 0x8000 else op)) & 0xFFFF)
        if mn in ('JSR', 'JMP') and mode == 'abs':
            return bank | op
        if mn in ('JSL', 'JML') and mode == 'long':
            return op
        return None

    def lab(self, a):
        if a in self.names:
            return self.names[a]
        if a in self.funcs:
            return 'sub_%06X' % a
        return 'L%06X' % a

    def byte(self, off):
        return self.rom[off]

    def operand(self, pc, mn, mode, size, op):
        bank = pc & 0xFF0000
        t = self.target(pc, mn, mode, size, op)
        same_bank_label = t is not None and t in self.placed and (t & 0xFF0000) == bank
        if mode == 'imp':
            return ''
        if mode == 'acc':
            return 'a'
        if mode in ('immm', 'immx', 'imm8'):
            return '#$%0*X' % (2 * (size - 1), op)
        if mode in ('rel', 'rell'):
            return self.lab(t) if same_bank_label else '$%06X' % t
        if mn in ('JSR', 'JMP') and mode == 'abs':
            return self.lab(t) if same_bank_label else 'a:$%04X' % op
        if mn in ('JSL', 'JML') and mode == 'long':
            return self.lab(t) if same_bank_label else '$%06X' % op
        if mn == 'PEA':
            return '$%04X' % op
        if mn == 'PEI':
            return '($%02X)' % op
        dp = '$%02X' % op
        ab = '$%04X' % op
        lg = '$%06X' % op
        reg = HWREG.get(op) if mode in ('abs', 'absx', 'absy') else None
        abp = 'a:' + (reg if reg else ab)
        return {
            'dp': 'z:' + dp, 'dpx': 'z:' + dp + ',x', 'dpy': 'z:' + dp + ',y',
            'idp': '(' + dp + ')', 'idpx': '(' + dp + ',x)', 'idpy': '(' + dp + '),y',
            'ildp': '[' + dp + ']', 'ildpy': '[' + dp + '],y', 'sr': dp + ',s',
            'isry': '(' + dp + ',s),y', 'abs': abp, 'absx': abp + ',x', 'absy': abp + ',y',
            'long': 'f:' + lg, 'longx': 'f:' + lg + ',x', 'iabs': '(' + ab + ')',
            'iabsx': '(' + ab + ',x)', 'ilabs': '[' + ab + ']',
        }[mode]

    def bank_source(self, bnum):
        base = 0x800000 | (bnum << 16) | 0x8000
        out = ['; Bank $%02X (file $%05X-$%05X)' % (0x80 | bnum, bnum * BANK_SIZE, bnum * BANK_SIZE + BANK_SIZE - 1),
               '.p816', '.smart -', '.include "hwregs.inc"',
               '.segment "BANK%02X"' % bnum, '']
        a_state = i_state = None
        data = []

        def flush():
            for i in range(0, len(data), 16):
                out.append('  .byte ' + ','.join('$%02X' % b for b in data[i:i + 16]))
            data.clear()

        off = 0
        starts = sorted(pc for pc in self.insn if (pc & 0xFFFF8000) == base & 0xFFFF8000 and (pc >> 16) == (base >> 16))
        start_set = set(starts)
        while off < BANK_SIZE:
            pc = base + off
            fo = bnum * BANK_SIZE + off
            ins = self.insn.get(pc)
            ok = ins is not None and off + ins[2] <= BANK_SIZE
            if ok:
                # an instruction overlapping a later instruction start is emitted as data
                for i in range(1, ins[2]):
                    if pc + i in start_set:
                        ok = False
                        break
            if not ok:
                if pc in self.labels:
                    flush()
                    out.append('%s:' % self.lab(pc))
                    self.placed.add(pc)
                data.append(self.rom[fo])
                off += 1
                if pc + 1 in self.labels or len(data) >= 16 and (pc + 1) in start_set:
                    flush()
                continue
            flush()
            mn, mode, size, op = ins
            if pc in self.labels:
                if pc in self.funcs:
                    out.append('')
                out.append('%s:' % self.lab(pc))
                self.placed.add(pc)
            if mn in ('MVN', 'MVP', 'BRK', 'COP', 'WDM'):
                raw = self.rom[fo:fo + size]
                out.append('  .byte ' + ','.join('$%02X' % b for b in raw) + '   ; %s' % mn)
            else:
                if mode == 'immm':
                    want = '.a8' if size == 2 else '.a16'
                    if a_state != want:
                        out.append('  ' + want)
                        a_state = want
                elif mode == 'immx':
                    want = '.i8' if size == 2 else '.i16'
                    if i_state != want:
                        out.append('  ' + want)
                        i_state = want
                ops = self.operand(pc, mn, mode, size, op)
                out.append('  %-4s %s' % (mn.lower(), ops))
            off += size
        flush()
        return '\n'.join(out) + '\n'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('rom')
    ap.add_argument('code_json')
    ap.add_argument('outdir')
    ap.add_argument('--names')
    a = ap.parse_args()
    rom = open(a.rom, 'rb').read()
    code = json.load(open(a.code_json))
    names = load_names(a.names)
    em = Emitter(rom, code, names)
    os.makedirs(a.outdir, exist_ok=True)
    nb = len(rom) // BANK_SIZE
    for b in range(nb):
        em.bank_source(b)       # first pass: learn which labels get placed
    for b in range(nb):
        with open(os.path.join(a.outdir, 'bank%02X.s' % b), 'w', newline='\n') as f:
            f.write(em.bank_source(b))
    mem = ['MEMORY {']
    seg = ['SEGMENTS {']
    for b in range(nb):
        mem.append('  M%02X: start = $%02X8000, size = $8000, fill = yes, fillval = $FF, file = %%O;' % (b, 0x80 | b))
        seg.append('  BANK%02X: load = M%02X, type = ro;' % (b, b))
    with open(os.path.join(a.outdir, 'link.cfg'), 'w', newline='\n') as f:
        f.write('\n'.join(mem + ['}'] + seg + ['}']) + '\n')
    print('wrote %d banks to %s' % (nb, a.outdir))


if __name__ == '__main__':
    main()
