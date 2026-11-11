#!/usr/bin/env python3
"""Per-function catalogue for naming work.

    python tools/catalog.py s1 [--caps 'E:/.../cov/s1m/*/cap.json'] [--xmatch build/xm/s1_s2.txt]

Writes build/<ver>/catalog.md: one row per function with size, executed
fraction (from coverage captures), callers, callees, hardware registers,
RAM touched, current name and the twin in another version.
"""
import argparse
import bisect
import collections
import glob
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bsdis import HWREG  # noqa: E402


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('--caps', action='append', default=[])
    ap.add_argument('--xmatch')
    a = ap.parse_args()
    code = json.load(open('build/%s/code.json' % a.ver))
    insn = {int(k, 16): v for k, v in code['insn'].items()}
    funcs = sorted(int(k, 16) for k in code['funcs'])
    names = {}
    np = 'config/%s/names.txt' % a.ver
    if os.path.exists(np):
        for line in open(np):
            t = line.split('#')[0].split()
            if len(t) >= 2:
                names[int(t[0], 16)] = t[1]
    executed = set()
    for pat in a.caps:
        for c in glob.glob(pat):
            if c.replace('\\', '/').split('/')[-2] == '57':
                pass
            try:
                j = json.load(open(c))
            except Exception:
                continue
            for r in j.get('costs', []):
                pc = int(r['target_pc24'], 16)
                if (pc >> 16) < 0x40:
                    pc |= 0x800000
                executed.add(pc)
    twin = {}
    if a.xmatch and os.path.exists(a.xmatch):
        for line in open(a.xmatch):
            t = line.split()
            if len(t) >= 3 and t[0] != '#' and t[0] != '------' and t[1] != '------':
                twin[int(t[0], 16)] = (t[1], ' '.join(t[2:]))

    def owner(pc):
        i = bisect.bisect_right(funcs, pc) - 1
        return funcs[i] if i >= 0 else None

    body = collections.defaultdict(list)
    for pc in sorted(insn):
        f = owner(pc)
        if f is not None:
            body[f].append(pc)
    callers = collections.defaultdict(set)
    callees = collections.defaultdict(set)
    hw = collections.defaultdict(set)
    ram = collections.defaultdict(set)
    for f, pcs in body.items():
        for pc in pcs:
            mn, mode, size, op = insn[pc]
            if mn in ('JSR', 'JSL') and mode in ('abs', 'long'):
                t = (pc & 0xFF0000) | op if mode == 'abs' else op
                callees[f].add(t)
                callers[t].add(f)
            if mode in ('abs', 'absx', 'absy') and op in HWREG and mn not in ('JSR', 'JMP'):
                hw[f].add(HWREG[op])
            elif mode in ('dp', 'dpx', 'dpy') or (mode in ('abs', 'absx', 'absy') and op < 0x2000 and mn not in ('JSR', 'JMP', 'PEA')):
                ram[f].add(op)
            elif mode in ('long', 'longx') and (op >> 16) in (0x7E, 0x7F):
                ram[f].add(op)
    lab = lambda f: names.get(f, 'sub_%06X' % f)  # noqa: E731
    out = ['# Function catalogue: %s' % a.ver, '',
           '%d functions. "run" = fraction of the function\'s instructions seen executed.' % len(funcs), '',
           '| addr | name | insns | run | callers | callees | hw | ram (first) | twin |',
           '|---|---|---|---|---|---|---|---|---|']
    for f in funcs:
        pcs = body.get(f, [])
        run = sum(1 for p in pcs if p in executed)
        frac = '%d%%' % (100 * run // len(pcs)) if pcs else '-'
        cs = ' '.join(lab(c) for c in sorted(callers[f])[:4]) + (' +%d' % (len(callers[f]) - 4) if len(callers[f]) > 4 else '')
        ce = ' '.join(lab(c) for c in sorted(callees[f])[:4]) + (' +%d' % (len(callees[f]) - 4) if len(callees[f]) > 4 else '')
        h = ' '.join(sorted(hw[f]))[:60]
        r = ' '.join('%X' % x for x in sorted(ram[f])[:8])
        tw = '%s %s' % twin[f] if f in twin else ''
        out.append('| %06X | %s | %d | %s | %s | %s | %s | %s | %s |' % (f, lab(f), len(pcs), frac, cs, ce, h, r, tw))
    path = 'build/%s/catalog.md' % a.ver
    open(path, 'w', encoding='utf-8').write('\n'.join(out) + '\n')
    print('wrote', path)


if __name__ == '__main__':
    main()
