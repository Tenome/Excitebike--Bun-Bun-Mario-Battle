#!/usr/bin/env python3
"""List routines that call only hand-written code (so a hand-written version
can also run in the port), smallest first.

    python tools/nexttargets.py s1 [--min 10] [--max 200]
"""
import argparse
import bisect
import json


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('--min', type=int, default=8)
    ap.add_argument('--max', type=int, default=400)
    a = ap.parse_args()
    j = json.load(open('build/%s/code.json' % a.ver))
    ins = {int(k, 16): v for k, v in j['insn'].items()}
    funcs = sorted(int(k, 16) for k in j['funcs'])
    hand = set()
    for l in open('c/hand/%s/hand.txt' % a.ver):
        t = l.split()
        if t and not l.startswith('#'):
            hand.add(int(t[0], 16))
    names = {}
    for l in open('config/%s/names.txt' % a.ver):
        t = l.split('#')[0].split()
        if len(t) >= 2:
            names[int(t[0], 16)] = t[1]
    jt = {int(k, 16): [int(x, 16) for x in v['targets']] for k, v in j.get('jt', {}).items()}
    body, callees, tails = {}, {}, {}
    for pc in sorted(ins):
        f = funcs[bisect.bisect_right(funcs, pc) - 1]
        body.setdefault(f, []).append(pc)
        mn, mode, size, op = ins[pc]
        if mn in ('JSR', 'JSL') and mode in ('abs', 'long'):
            callees.setdefault(f, set()).add((pc & 0xFF0000) | op if mode == 'abs' else op)
        if mn == 'JSR' and mode == 'iabsx':
            callees.setdefault(f, set()).update(jt.get(pc, [None]))
    out = []
    for f in funcs:
        if f in hand:
            continue
        cs = callees.get(f, set())
        if cs <= hand:
            n = len(body.get(f, []))
            if a.min <= n <= a.max:
                out.append((n, f))
    for n, f in sorted(out):
        print('%06X %-22s %d' % (f, names.get(f, ''), n))
    print(len(out), 'candidates')


if __name__ == '__main__':
    main()
