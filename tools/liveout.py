#!/usr/bin/env python3
"""Which registers/flags do callers read after a routine returns?

    python tools/liveout.py s1 DmaFillZero [more names or pc24 ...]

For each call site of the routine, walk forward from the return point (both
branch directions, through JMP/BRA) until every one of A, X, Y and the
C/Z/N/V flags has been either written (dead) or read (live). Calls and
returns in that walk count as reading A, X and Y (conservative), but not the
flags. The union over all call sites is the routine's live-out set: what a
hand-written replacement must reproduce (tools/difftest816.py compare spec).
"""
import json
import os
import sys

REGS = ('A', 'X', 'Y', 'C', 'Z', 'N', 'V')

# (uses, defs) by mnemonic; addressing-mode index uses are added separately.
UD = {
    'LDA': ('', 'ANZ'), 'LDX': ('', 'XNZ'), 'LDY': ('', 'YNZ'),
    'STA': ('A', ''), 'STX': ('X', ''), 'STY': ('Y', ''), 'STZ': ('', ''),
    'ADC': ('AC', 'ACNZV'), 'SBC': ('AC', 'ACNZV'),
    'AND': ('A', 'ANZ'), 'ORA': ('A', 'ANZ'), 'EOR': ('A', 'ANZ'),
    'CMP': ('A', 'CNZ'), 'CPX': ('X', 'CNZ'), 'CPY': ('Y', 'CNZ'),
    'BIT': ('A', 'NZV'), 'TSB': ('A', 'Z'), 'TRB': ('A', 'Z'),
    'ASL': ('', 'CNZ'), 'LSR': ('', 'CNZ'), 'ROL': ('C', 'CNZ'), 'ROR': ('C', 'CNZ'),
    'INC': ('', 'NZ'), 'DEC': ('', 'NZ'),
    'INX': ('X', 'XNZ'), 'INY': ('Y', 'YNZ'), 'DEX': ('X', 'XNZ'), 'DEY': ('Y', 'YNZ'),
    'TAX': ('A', 'XNZ'), 'TAY': ('A', 'YNZ'), 'TXA': ('X', 'ANZ'), 'TYA': ('Y', 'ANZ'),
    'TXY': ('X', 'YNZ'), 'TYX': ('Y', 'XNZ'), 'TSX': ('', 'XNZ'), 'TXS': ('X', ''),
    'TCD': ('A', 'NZ'), 'TDC': ('', 'ANZ'), 'TCS': ('A', ''), 'TSC': ('', 'ANZ'),
    'XBA': ('A', 'ANZ'), 'PHA': ('A', ''), 'PHX': ('X', ''), 'PHY': ('Y', ''),
    'PLA': ('', 'ANZ'), 'PLX': ('', 'XNZ'), 'PLY': ('', 'YNZ'), 'PLB': ('', 'NZ'), 'PLD': ('', 'NZ'),
    'PHP': ('CZNV', ''), 'PLP': ('', 'CZNV'),
    'CLC': ('', 'C'), 'SEC': ('', 'C'), 'CLV': ('', 'V'),
    'BPL': ('N', ''), 'BMI': ('N', ''), 'BVC': ('V', ''), 'BVS': ('V', ''),
    'BCC': ('C', ''), 'BCS': ('C', ''), 'BNE': ('Z', ''), 'BEQ': ('Z', ''),
    'XCE': ('C', 'C'), 'MVN': ('AXY', 'AXY'), 'MVP': ('AXY', 'AXY'),
}
INDEX_USE = {'dpx': 'X', 'absx': 'X', 'longx': 'X', 'idpx': 'X', 'iabsx': 'X',
             'dpy': 'Y', 'absy': 'Y', 'idpy': 'Y', 'ildpy': 'Y', 'isry': 'Y'}
BR = {'BPL', 'BMI', 'BVC', 'BVS', 'BCC', 'BCS', 'BNE', 'BEQ'}


def main():
    ver, targets = sys.argv[1], sys.argv[2:]
    code = json.load(open('build/%s/code.json' % ver))
    insn = {int(k, 16): v for k, v in code['insn'].items()}
    names = {}
    np = 'config/%s/names.txt' % ver
    if os.path.exists(np):
        for line in open(np):
            t = line.split('#')[0].split()
            if len(t) >= 2:
                names[t[1]] = int(t[0], 16)
    jt = {int(k, 16): [int(x, 16) for x in v['targets']] for k, v in code.get('jt', {}).items()}
    for tname in targets:
        tgt = names.get(tname) or int(tname, 16)
        sites = []
        for pc, (mn, mode, size, op) in insn.items():
            if mn in ('JSR', 'JSL') and mode in ('abs', 'long'):
                t = (pc & 0xFF0000) | op if mode == 'abs' else op
                if t == tgt:
                    sites.append(pc)
            if mode in ('iabsx', 'iabs') and tgt in jt.get(pc, []):
                sites.append(pc)
        live = set()
        tail = False
        for s in sites:
            mn, mode, size, op = insn[s]
            if mn == 'JMP':           # tail dispatch: the routine returns to our caller's caller
                tail = True
                continue
            start = (s & 0xFF0000) | ((s + size) & 0xFFFF)
            # DFS with per-path "decided" sets
            work = [(start, frozenset())]
            seen = set()
            while work:
                pc, decided = work.pop()
                if (pc, decided) in seen or len(seen) > 20000:
                    continue
                seen.add((pc, decided))
                if pc not in insn:
                    continue
                mn, mode, size, op = insn[pc]
                u, d = UD.get(mn, ('', ''))
                if mn in ('REP', 'SEP'):
                    u, d = '', ''.join(f for f, bit in (('C', 1), ('Z', 2), ('V', 0x40), ('N', 0x80)) if op & bit)
                if mn in ('ASL', 'LSR', 'ROL', 'ROR', 'INC', 'DEC') and mode == 'acc':
                    u, d = u + 'A', d + 'A'
                u = set(u) | (set(INDEX_USE.get(mode, '')))
                if mn in ('JSR', 'JSL', 'RTS', 'RTL', 'RTI', 'JML') or mode in ('iabs', 'iabsx', 'ilabs'):
                    u |= {'A', 'X', 'Y'}
                    live |= (u - decided)
                    if mn in ('RTS', 'RTL', 'RTI', 'JML') or mode in ('iabs', 'iabsx', 'ilabs') and mn == 'JMP':
                        continue
                    # after a call, assume it may define nothing we track except via its own code
                live |= (u - decided)
                decided = decided | frozenset(u) | frozenset(d)
                if decided >= set(REGS):
                    continue
                nxt = (pc & 0xFF0000) | ((pc + size) & 0xFFFF)
                if mn in BR:
                    t = (pc & 0xFF0000) | ((nxt + (op - 256 if op & 0x80 else op)) & 0xFFFF)
                    work.append((t, decided))
                    work.append((nxt, decided))
                elif mn in ('BRA', 'BRL'):
                    rel = (op - 256 if op & 0x80 else op) if mn == 'BRA' else (op - 0x10000 if op & 0x8000 else op)
                    work.append(((pc & 0xFF0000) | ((nxt + rel) & 0xFFFF), decided))
                elif mn == 'JMP' and mode == 'abs':
                    work.append(((pc & 0xFF0000) | op, decided))
                elif mn in ('STP', 'WAI', 'BRK', 'COP'):
                    continue
                else:
                    work.append((nxt, decided))
        print('%-22s sites %3d  live-out: %s%s' % (tname, len(sites), ','.join(r for r in REGS if r in live) or '-',
                                                   '  (also tail-dispatched: callers of the dispatcher decide)' if tail else ''))


if __name__ == '__main__':
    main()
