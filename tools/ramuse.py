#!/usr/bin/env python3
"""List RAM addresses the code touches, with reader/writer functions.

    python tools/ramuse.py build/s1/code.json [--min 1] > build/s1/ram.txt

Direct-page operands are reported as $00xx (most code runs with D=$0000; the
DMA routines that set D=$4300 show up as low DP addresses too, check by hand).
Long addresses in $7E/$7F are reported as-is.
"""
import argparse
import bisect
import collections
import json

WRITES = {'STA', 'STX', 'STY', 'STZ', 'INC', 'DEC', 'ASL', 'LSR', 'ROL', 'ROR', 'TSB', 'TRB'}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('code_json')
    ap.add_argument('--min', type=int, default=1)
    a = ap.parse_args()
    j = json.load(open(a.code_json))
    funcs = sorted(int(k, 16) for k in j['funcs'])
    use = collections.defaultdict(lambda: {'R': set(), 'W': set(), 'n': 0})
    for k, (mn, mode, size, op) in j['insn'].items():
        pc = int(k, 16)
        if mode in ('dp', 'dpx', 'dpy', 'idp', 'idpx', 'idpy', 'ildp', 'ildpy'):
            addr = op
        elif mode in ('abs', 'absx', 'absy') and op < 0x2000 and mn not in ('JMP', 'JSR', 'PEA'):
            addr = op
        elif mode in ('long', 'longx') and (op >> 16) in (0x7E, 0x7F) and mn not in ('JSL', 'JML'):
            addr = op
        elif mode in ('abs', 'absx', 'absy') and 0x6000 > op >= 0x2000 and op not in range(0x2100, 0x2200) \
                and op not in range(0x4000, 0x4400) and mn not in ('JMP', 'JSR', 'PEA'):
            addr = 0x7E0000 | op   # $2000-$5FFF only maps to WRAM via long addressing; flag it
        else:
            continue
        f = funcs[bisect.bisect_right(funcs, pc) - 1] if funcs and pc >= funcs[0] else pc
        u = use[addr]
        u['W' if mn in WRITES else 'R'].add(f)
        u['n'] += 1
    for addr in sorted(use):
        u = use[addr]
        if u['n'] < a.min:
            continue
        w = ' '.join('%06X' % f for f in sorted(u['W'])[:6])
        r = ' '.join('%06X' % f for f in sorted(u['R'])[:6])
        print('$%06X  n=%-4d W[%d]: %-48s R[%d]: %s' % (addr, u['n'], len(u['W']), w, len(u['R']), r))


if __name__ == '__main__':
    main()
