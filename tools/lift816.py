#!/usr/bin/env python3
"""Mechanical 65816 -> C lifter (faithful, one C function per entry variant).

    python tools/lift816.py s1            # -> c/auto/s1/lifted.c, lifted.h

Every instruction becomes one C statement over the machine model in
c/runtime816.h (registers A X Y S D DB, flags, the real stack in WRAM), so the
output can be diffed against the original routine by tools/difftest816.py.
It is the starting point for hand-written readable C, not the end product.

Control flow: labels + goto inside a function; JSR/JSL push the real return
address and call the target's C function; RTS/RTL pop it; JMP/branch to
another function's entry is a tail call; resolved jump tables become switch
statements on the pointer value. Backward jumps tick a loop guard so a
routine that waits on hardware cannot hang the test harness.
"""
import argparse
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bsdis import Disasm, Rom, BRANCHES  # noqa: E402
from op65816 import OPS, operand_size  # noqa: E402

COND = {'BPL': '!fn', 'BMI': 'fn', 'BVC': '!fv', 'BVS': 'fv',
        'BCC': '!fc', 'BCS': 'fc', 'BNE': '!fz', 'BEQ': 'fz'}


def load_names(path):
    names = {}
    if path and os.path.exists(path):
        for line in open(path):
            t = line.split('#')[0].split()
            if len(t) >= 2:
                names[int(t[0], 16)] = t[1]
    return names


class Lifter:
    def __init__(self, rom, code, names):
        self.rom = Rom(rom)
        self.dis = Disasm(self.rom)          # for exit_states / plausibility
        self.funcs = {int(k, 16): [(v[1] == '1', v[3] == '1') for v in vs]
                      for k, vs in code['funcs'].items()}
        self.jt = {int(k, 16): [int(t, 16) for t in v['targets']] for k, v in code.get('jt', {}).items()}
        self.names = names

    def fname(self, pc, m, x):
        base = self.names.get(pc, 'sub_%06X' % pc)
        return '%s_m%dx%d' % (base, int(m), int(x))

    def variants(self):
        for pc in sorted(self.funcs):
            for m, x in sorted(set(self.funcs[pc])):
                yield pc, m, x

    # ------------------------------------------------------------------
    def decode(self, pc, m, x):
        op = self.rom.byte(pc)
        if op is None:
            return None
        mn, mode = OPS[op]
        size = 1 + operand_size(mode, m, x)
        val = 0
        for i in range(1, size):
            b = self.rom.byte((pc & 0xFF0000) | ((pc + i) & 0xFFFF))
            if b is None:
                return None
            val |= b << (8 * (i - 1))
        return mn, mode, size, val

    def body(self, entry, m, x):
        """Trace one variant: {(pc, m, x): (insn, successors)}."""
        nodes = {}
        work = [(entry, m, x, ())]
        while work:
            pc, cm, cx, stk = work.pop()
            key = (pc, cm, cx)
            if key in nodes:
                continue
            d = self.decode(pc, cm, cx)
            if d is None:
                nodes[key] = (None, [])
                continue
            mn, mode, size, val = d
            bank = pc & 0xFF0000
            nxt = bank | ((pc + size) & 0xFFFF)
            nm, nx = cm, cx
            nstk = stk
            if mn == 'REP':
                nm = cm and not val & 0x20; nx = cx and not val & 0x10
            elif mn == 'SEP':
                nm = cm or bool(val & 0x20); nx = cx or bool(val & 0x10)
            elif mn == 'PHP':
                nstk = (stk + ((cm, cx),))[-8:]
            elif mn == 'PLP' and stk:
                nm, nx = stk[-1]; nstk = stk[:-1]
            succ = []
            info = {'nxt': nxt, 'state_after': (nm, nx)}
            if mn in BRANCHES:
                t = bank | ((nxt + (val - 256 if val & 0x80 else val)) & 0xFFFF)
                info['target'] = t
                if not (t in self.funcs and t != entry):
                    succ.append((t, nm, nx, nstk))
                succ.append((nxt, nm, nx, nstk))
            elif mn in ('BRA', 'BRL') or (mn == 'JMP' and mode == 'abs') or (mn == 'JML' and mode == 'long'):
                if mn == 'BRA':
                    t = bank | ((nxt + (val - 256 if val & 0x80 else val)) & 0xFFFF)
                elif mn == 'BRL':
                    t = bank | ((nxt + (val - 0x10000 if val & 0x8000 else val)) & 0xFFFF)
                elif mn == 'JMP':
                    t = bank | val
                else:
                    t = val
                info['target'] = t
                if not (t in self.funcs and t != entry) and self.rom.off(t) is not None:
                    succ.append((t, nm, nx, nstk))
            elif (mn == 'JSR' and mode == 'abs') or (mn == 'JSL' and self.rom.off(val) is not None):
                t = bank | val if mn == 'JSR' else val
                info['target'] = t
                ex = self.dis.exit_states(t, cm, cx) or {(cm, cx)}
                info['exits'] = sorted(ex)
                for em, exx in ex:
                    succ.append((nxt, em, exx, nstk))
            elif mn == 'JSR' and mode == 'iabsx':
                ex = set()
                for t in self.jt.get(pc, []):
                    ex |= self.dis.exit_states(t, cm, cx) or {(cm, cx)}
                ex = ex or {(cm, cx)}
                info['exits'] = sorted(ex)
                for em, exx in ex:
                    succ.append((nxt, em, exx, nstk))
            elif mn in ('RTS', 'RTL', 'RTI', 'STP', 'WAI', 'BRK', 'COP', 'JSL') or mn in ('JMP', 'JML'):
                pass
            else:
                succ.append((nxt, nm, nx, nstk))
            nodes[key] = (d + (info,), [(s[0], s[1], s[2]) for s in succ])
            for s in succ:
                work.append(s)
        return nodes

    # ------------------------------------------------------------------
    def ea(self, mode, val):
        return {
            'dp': 'DP(0x%02X)', 'dpx': 'DPX(0x%02X)', 'dpy': 'DPY(0x%02X)',
            'idp': 'IDP(0x%02X)', 'idpx': 'IDPX(0x%02X)', 'idpy': 'IDPY(0x%02X)',
            'ildp': 'ILDP(0x%02X)', 'ildpy': 'ILDPY(0x%02X)', 'sr': 'SR(0x%02X)',
            'isry': 'ISRY(0x%02X)', 'abs': 'ABS(0x%04X)', 'absx': 'ABSX(0x%04X)',
            'absy': 'ABSY(0x%04X)', 'long': 'LONG(0x%06X)', 'longx': 'LONGX(0x%06X)',
        }[mode] % val

    def operand(self, mode, val, wide):
        if mode in ('immm', 'immx', 'imm8'):
            return '0x%0*X' % (4 if wide else 2, val)
        return ('rd16(%s)' if wide else 'rd8(%s)') % self.ea(mode, val)

    def stmt(self, pc, d, m, x):
        mn, mode, size, val, info = d
        wa, wx = not m, not x
        s8a, s8x = ('16' if wa else '8'), ('16' if wx else '8')
        o = lambda w: self.operand(mode, val, w)  # noqa: E731
        bank = pc >> 16
        if mn in ('LDA', 'LDX', 'LDY'):
            w = wa if mn == 'LDA' else wx
            return '%s%s(%s);' % (mn.lower(), '16' if w else '8', o(w))
        if mn in ('STA', 'STX', 'STY', 'STZ'):
            w = wa if mn in ('STA', 'STZ') else wx
            src = {'STA': 'A', 'STX': 'X', 'STY': 'Y', 'STZ': '0'}[mn]
            return '%s(%s, %s);' % ('wr16' if w else 'wr8', self.ea(mode, val), src if w else '(u8)' + src)
        if mn in ('ORA', 'AND', 'EOR', 'ADC', 'SBC'):
            return '%s%s(%s);' % (mn.lower(), s8a, o(wa))
        if mn == 'CMP':
            return 'cmp%s(%sA, %s);' % (s8a, '' if wa else '(u8)', o(wa))
        if mn in ('CPX', 'CPY'):
            r = mn[2]
            return 'cmp%s(%s%s, %s);' % (s8x, '' if wx else '(u8)', r, o(wx))
        if mn == 'BIT':
            return 'bit%s(%s, %d);' % (s8a, o(wa), 1 if mode == 'immm' else 0)
        if mn in ('ASL', 'LSR', 'ROL', 'ROR', 'INC', 'DEC', 'TSB', 'TRB'):
            f = mn.lower() + s8a
            if mode == 'acc':
                return ('A = %s(A);' % f) if wa else ('SET_A8(%s((u8)A));' % f)
            if mn in ('TSB', 'TRB'):
                return '{ u32 ea = %s; %s(ea, %s(%s(ea))); }' % (self.ea(mode, val), 'wr16' if wa else 'wr8', f, 'rd16' if wa else 'rd8')
            return '{ u32 ea = %s; %s(ea, %s(%s(ea))); }' % (self.ea(mode, val), 'wr16' if wa else 'wr8', f, 'rd16' if wa else 'rd8')
        if mn in ('INX', 'INY', 'DEX', 'DEY'):
            r = mn[2]
            f = ('inc' if mn[0] == 'I' else 'dec') + s8x
            return '%s = %s(%s%s);' % (r, f, '' if wx else '(u8)', r)
        tr = {
            'TAX': ('X', 'A', wx), 'TAY': ('Y', 'A', wx), 'TXA': ('A', 'X', wa), 'TYA': ('A', 'Y', wa),
            'TXY': ('Y', 'X', wx), 'TYX': ('X', 'Y', wx), 'TSX': ('X', 'S', wx),
        }
        if mn in tr:
            dst, src, w = tr[mn]
            if dst == 'A':
                return ('A = %s; nz16(A);' % src) if w else ('SET_A8(%s); nz8((u8)%s);' % (src, src))
            return ('%s = %s; nz16(%s);' % (dst, src, dst)) if w else ('%s = (u8)%s; nz8((u8)%s);' % (dst, src, dst))
        if mn == 'TXS': return 'S = X;'
        if mn == 'TCS': return 'S = A;'
        if mn == 'TCD': return 'D = A; nz16(D);'
        if mn == 'TDC': return 'A = D; nz16(A);'
        if mn == 'TSC': return 'A = S; nz16(A);'
        if mn == 'XBA': return 'A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);'
        if mn == 'PHA': return 'push16(A);' if wa else 'push8((u8)A);'
        if mn == 'PHX': return 'push16(X);' if wx else 'push8((u8)X);'
        if mn == 'PHY': return 'push16(Y);' if wx else 'push8((u8)Y);'
        if mn == 'PHB': return 'push8(DB);'
        if mn == 'PHD': return 'push16(D);'
        if mn == 'PHK': return 'push8(0x%02X);' % bank
        if mn == 'PHP': return 'push8(getP());'
        if mn == 'PLA': return 'lda16(pull16());' if wa else 'lda8(pull8());'
        if mn == 'PLX': return 'ldx16(pull16());' if wx else 'ldx8(pull8());'
        if mn == 'PLY': return 'ldy16(pull16());' if wx else 'ldy8(pull8());'
        if mn == 'PLB': return 'DB = pull8(); nz8(DB);'
        if mn == 'PLD': return 'D = pull16(); nz16(D);'
        if mn == 'PLP': return 'setP(pull8());'
        if mn == 'PEA': return 'push16(0x%04X);' % val
        if mn == 'PEI': return 'push16(rd16(DP(0x%02X)));' % val
        if mn == 'PER':
            t = (info['nxt'] + (val - 0x10000 if val & 0x8000 else val)) & 0xFFFF
            return 'push16(0x%04X);' % t
        if mn == 'REP': return 'rep(0x%02X);' % val
        if mn == 'SEP': return 'sep(0x%02X);' % val
        flags = {'CLC': 'fc = 0;', 'SEC': 'fc = 1;', 'CLI': 'fi = 0;', 'SEI': 'fi = 1;',
                 'CLD': 'fd = 0;', 'SED': 'fd = 1;', 'CLV': 'fv = 0;'}
        if mn in flags: return flags[mn]
        if mn in ('XCE', 'NOP'): return ';'
        if mn == 'MVN': return 'mvn(0x%02X, 0x%02X);' % (val & 0xFF, val >> 8)
        if mn == 'MVP': return 'mvp(0x%02X, 0x%02X);' % (val & 0xFF, val >> 8)
        return None

    # ------------------------------------------------------------------
    def lift_variant(self, entry, m, x):
        nodes = self.body(entry, m, x)
        pcs_states = {}
        for (pc, cm, cx) in nodes:
            pcs_states.setdefault(pc, set()).add((cm, cx))

        def lab(pc, cm, cx):
            if len(pcs_states.get(pc, ())) > 1:
                return 'L%06X_m%dx%d' % (pc, int(cm), int(cx))
            return 'L%06X' % pc

        def go(pc, cm, cx, here):
            t = 'goto %s;' % lab(pc, cm, cx)
            if pc <= here:
                return '{ R816_TICK(); %s }' % t
            return t

        def call_expr(t, cm, cx):
            return '%s();' % self.fname(t, cm, cx)

        out = ['void %s(void)' % self.fname(entry, m, x), '{']
        order = sorted(nodes, key=lambda k: (k[0], k[1], k[2]))
        # entry first
        first = (entry, m, x)
        order.remove(first)
        order.insert(0, first)
        targets = set()
        for k, (d, succ) in nodes.items():
            for s in succ:
                targets.add(s)
        prev_fall = None
        for i, key in enumerate(order):
            pc, cm, cx = key
            d, succ = nodes[key]
            if prev_fall is not None and prev_fall != key:
                out.append('    ' + go(prev_fall[0], prev_fall[1], prev_fall[2], order[i - 1][0]))
            prev_fall = None
            out.append('%s: /* $%06X */' % (lab(pc, cm, cx), pc))
            if d is None:
                out.append('    r816_unknown_target(0x%06X, 0); return;' % pc)
                continue
            mn, mode, size, val, info = d
            nxt = info['nxt']
            nm, nx = info['state_after']
            s = self.stmt(pc, d, cm, cx)
            if s is not None:
                out.append('    ' + s)
                prev_fall = (nxt, nm, nx)
                continue
            if mn in BRANCHES:
                t = info['target']
                if t in self.funcs and t != entry:
                    out.append('    if (%s) { %s return; }' % (COND[mn], call_expr(t, nm, nx)))
                else:
                    out.append('    if (%s) %s' % (COND[mn], go(t, nm, nx, pc)))
                prev_fall = (nxt, nm, nx)
            elif mn in ('BRA', 'BRL', 'JMP', 'JML') and 'target' in info:
                t = info['target']
                if t in self.funcs and t != entry:
                    out.append('    %s return;' % call_expr(t, nm, nx))
                elif self.rom.off(t) is None:
                    out.append('    r816_unknown_target(0x%06X, 0x%06X); return;' % (pc, t))
                else:
                    out.append('    ' + go(t, nm, nx, pc))
            elif mn in ('JSR', 'JSL') and 'target' in info:
                t = info['target']
                ret = (pc + size - 1) & 0xFFFF
                push = ('push8(0x%02X); ' % (pc >> 16) if mn == 'JSL' else '') + 'push16(0x%04X); ' % ret
                out.append('    %s%s' % (push, call_expr(t, cm, cx)))
                exits = info['exits']
                if len(exits) == 1:
                    prev_fall = (nxt, exits[0][0], exits[0][1])
                else:
                    sw = ' '.join('if (fm == %d && fx == %d) goto %s;' % (int(em), int(exx), lab(nxt, em, exx)) for em, exx in exits)
                    out.append('    ' + sw + ' r816_unknown_target(0x%06X, 1); return;' % pc)
            elif mn == 'JSL':
                out.append('    r816_unknown_target(0x%06X, 0x%06X); return;' % (pc, val))
            elif mn == 'RTS':
                out.append('    pull16(); return;')
            elif mn == 'RTL':
                out.append('    pull16(); pull8(); return;')
            elif mn == 'RTI':
                out.append('    setP(pull8()); pull16(); pull8(); return;')
            elif mn == 'JMP' and mode == 'iabs':
                tg = self.jt.get(pc, [])
                out.append('    switch (rd16(0x%04X)) {' % val)
                for t in sorted(set(tg)):
                    out.append('    case 0x%04X: %s return;' % (t & 0xFFFF, call_expr(t, cm, cx)))
                out.append('    default: if (!r816_call_pc(0x%06X | rd16(0x%04X))) r816_unknown_target(0x%06X, rd16(0x%04X)); return;' % (pc & 0xFF0000, val, pc, val))
                out.append('    }')
            elif mn in ('JMP', 'JSR') and mode == 'iabsx':
                tg = self.jt.get(pc, [])
                expr = 'rd16(0x%06X + (u16)(0x%04X + X))' % (pc & 0xFF0000, val)
                if mn == 'JSR':
                    out.append('    push16(0x%04X);' % ((pc + size - 1) & 0xFFFF))
                out.append('    switch (%s) {' % expr)
                for t in sorted(set(tg)):
                    out.append('    case 0x%04X: %s %s' % (t & 0xFFFF, call_expr(t, cm, cx), 'break;' if mn == 'JSR' else 'return;'))
                out.append('    default: if (!r816_call_pc(0x%06X | %s)) r816_unknown_target(0x%06X, %s); %s' % (pc & 0xFF0000, expr, pc, expr, 'break;' if mn == 'JSR' else 'return;'))
                out.append('    }')
                if mn == 'JSR':
                    exits = info.get('exits', [(cm, cx)])
                    if len(exits) == 1:
                        prev_fall = (nxt, exits[0][0], exits[0][1])
                    else:
                        out.append('    ' + ' '.join('if (fm == %d && fx == %d) goto %s;' % (int(em), int(exx), lab(nxt, em, exx)) for em, exx in exits) + ' return;')
            else:
                out.append('    r816_unknown_target(0x%06X, 0); return; /* %s */' % (pc, mn))
        if prev_fall is not None:
            out.append('    ' + go(prev_fall[0], prev_fall[1], prev_fall[2], order[-1][0]))
        out.append('}')
        return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('--hand', help='hand.txt: variants implemented by hand (bodies not emitted)')
    a = ap.parse_args()
    hand = set()
    hp = a.hand or 'c/hand/%s/hand.txt' % a.ver
    if os.path.exists(hp):
        for line in open(hp):
            t = line.split('#')[0].split()
            if len(t) >= 3:
                hand.add((int(t[0], 16), t[1] == '1', t[2] == '1'))
    code = json.load(open('build/%s/code.json' % a.ver))
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    names = load_names('config/%s/names.txt' % a.ver)
    lf = Lifter(rom, code, names)
    variants = list(lf.variants())
    outdir = 'c/auto/%s' % a.ver
    os.makedirs(outdir, exist_ok=True)
    hdr = ['/* Generated by tools/lift816.py from %s. Do not edit; regenerate. */' % a.ver,
           '#ifndef LIFTED_%s_H' % a.ver.upper(), '#define LIFTED_%s_H' % a.ver.upper(),
           '#include "../../runtime816.h"']
    for pc, m, x in variants:
        hdr.append('void %s(void);' % lf.fname(pc, m, x))
    hdr += ['typedef struct { u32 pc; u8 m, x; void (*fn)(void); const char *name; } LiftedEntry;',
            'extern const LiftedEntry lifted_table[];', 'extern const unsigned lifted_count;',
            '/* Call the lifted routine at pc24 for the current widths; 0 if none. */',
            'int r816_call_pc(u32 pc24);', '#endif']
    open(os.path.join(outdir, 'lifted.h'), 'w', newline='\n').write('\n'.join(hdr) + '\n')
    body = ['/* Generated by tools/lift816.py from %s. Do not edit; regenerate. */' % a.ver,
            '#include "lifted.h"', '#include <setjmp.h>',
            'extern jmp_buf r816_bail; extern long r816_ticks, r816_limit;',
            '#define R816_TICK() do { if (++r816_ticks > r816_limit) longjmp(r816_bail, 1); } while (0)', '']
    for pc, m, x in variants:
        if (pc, m, x) in hand:
            body.append('/* %s: hand-written in c/hand/%s */' % (lf.fname(pc, m, x), a.ver))
            continue
        body.extend(lf.lift_variant(pc, m, x))
        body.append('')
    body.append('const LiftedEntry lifted_table[] = {')
    for pc, m, x in variants:
        body.append('    {0x%06X, %d, %d, %s, "%s"},' % (pc, int(m), int(x), lf.fname(pc, m, x), lf.fname(pc, m, x)))
    body.append('};')
    body.append('const unsigned lifted_count = %d;' % len(variants))
    body += ['', 'int r816_call_pc(u32 pc24)', '{',
             '    pc24 |= 0x800000;  /* LoROM execution mirror */',
             '    for (unsigned i = 0; i < lifted_count; i++)',
             '        if (lifted_table[i].pc == pc24 && lifted_table[i].m == fm && lifted_table[i].x == fx) {',
             '            lifted_table[i].fn();', '            return 1;', '        }',
             '    return 0;', '}']
    open(os.path.join(outdir, 'lifted.c'), 'w', newline='\n').write('\n'.join(body) + '\n')
    print('%s: %d function variants, %d lines' % (outdir, len(variants), len(body)))


if __name__ == '__main__':
    main()
