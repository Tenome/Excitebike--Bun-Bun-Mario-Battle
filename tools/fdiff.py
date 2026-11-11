#!/usr/bin/env python3
"""Instruction-level diff of matched functions between two versions.

    python tools/fdiff.py s1 s2 [--min 0 --max 0.999] [--only 809AF9]

Uses build/xm/<a>_<b>.txt pairs and the listings' instruction streams
(ROM-address operands masked as in xmatch). Prints a unified diff per
changed function.
"""
import argparse
import difflib
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from xmatch import bodies, load  # noqa: E402


def fmt(insn, f):
    out, pc, fset = [], f, None
    return out


def lines(insn, funcs, f):
    fs = set(funcs)
    out, pc = [], f
    while pc in insn and len(out) < 2000:
        mn, mode, size, op = insn[pc]
        out.append('%s %s %X' % (mn, mode, op) if not (mode in ('abs', 'absx', 'absy', 'long', 'longx') and (op & 0xFFFF) >= 0x8000)
                   else '%s %s @%04X' % (mn, mode, op & 0xFFFF))
        pc = (pc & 0xFF0000) | ((pc + size) & 0xFFFF)
        if pc in fs:
            break
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('a')
    ap.add_argument('b')
    ap.add_argument('--only')
    a = ap.parse_args()
    ia, fa = load('build/%s/code.json' % a.a)
    ib, fb = load('build/%s/code.json' % a.b)
    for line in open('build/xm/%s_%s.txt' % (a.a, a.b)):
        t = line.split()
        if len(t) < 3 or t[0].startswith('#') or t[2] != 'changed':
            continue
        if a.only and t[0] != a.only.upper():
            continue
        f, g = int(t[0], 16), int(t[1], 16)
        la, lb = lines(ia, fa, f), lines(ib, fb, g)
        print('=== %s $%06X  vs  %s $%06X  (%s)' % (a.a, f, a.b, g, ' '.join(t[2:])))
        for d in difflib.unified_diff(la, lb, lineterm='', n=1):
            if d.startswith(('---', '+++')):
                continue
            print('  ' + d)


if __name__ == '__main__':
    main()
