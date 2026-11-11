#!/usr/bin/env python3
"""Turn bsdis.py's code.json into snesrecomp bank cfg seeds.

    python tools/mkcfg.py build/s1/code.json recomp/s1 [--names config/s1/names.txt]

One bankXX.cfg per bank with a `func` line per discovered entry; entries that
are reached with several M/X states get one `entry_mx_at`-free func line (the
analyzer forks variants itself) and the first state as the declared width.
names.txt (optional): lines of `<pc24 hex> <name>` to give routines real names.
"""
import argparse
import collections
import json
import os


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('code_json')
    ap.add_argument('outdir')
    ap.add_argument('--names')
    a = ap.parse_args()
    funcs = json.load(open(a.code_json))['funcs']
    names = {}
    if a.names and os.path.exists(a.names):
        for line in open(a.names):
            line = line.split('#')[0].split()
            if len(line) >= 2:
                names[int(line[0].replace(':', ''), 16)] = line[1]
    by_bank = collections.defaultdict(list)
    for k, mx in funcs.items():
        pc = int(k, 16)
        by_bank[pc >> 16].append((pc, mx))
    os.makedirs(a.outdir, exist_ok=True)
    for bank, items in sorted(by_bank.items()):
        lines = ['bank = %02X' % (bank & 0x7F)]
        if bank == 0x80:
            lines.append('auto_vectors')
        for pc, mx in sorted(items):
            m, x = int(mx[0][1]), int(mx[0][3])
            nm = names.get(pc, 'sub_%06X' % pc)
            lines.append('func %s %06X entry_mx:%d,%d' % (nm, pc, m, x))
        path = os.path.join(a.outdir, 'bank%02X.cfg' % (bank & 0x7F))
        with open(path, 'w') as f:
            f.write('\n'.join(lines) + '\n')
        print('wrote', path, len(items), 'funcs')


if __name__ == '__main__':
    main()
