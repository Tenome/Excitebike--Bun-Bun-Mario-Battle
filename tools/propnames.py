#!/usr/bin/env python3
"""Carry routine names from one version to another through an xmatch map.

    python tools/propnames.py build/xm/s1_s2.txt config/s1/names.txt config/s2/names.txt [--min 0.9]

Only function matches are used ('same', or 'changed' at or above --min).
Existing names in the target file win; new ones are appended with a comment.
"""
import argparse
import os


def load(path):
    names, order = {}, []
    if os.path.exists(path):
        for line in open(path):
            t = line.split('#')[0].split()
            if len(t) >= 2:
                names[int(t[0], 16)] = t[1]
                order.append(int(t[0], 16))
    return names


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('xmatch')
    ap.add_argument('src')
    ap.add_argument('dst')
    ap.add_argument('--min', type=float, default=0.9)
    a = ap.parse_args()
    src, dst = load(a.src), load(a.dst)
    used = set(dst.values())
    new = []
    for line in open(a.xmatch):
        t = line.split()
        if len(t) < 3 or t[0].startswith('#') or '------' in (t[0], t[1]):
            continue
        fa, fb = int(t[0], 16), int(t[1], 16)
        ok = t[2] == 'same' or (t[2] == 'changed' and float(t[3]) >= a.min)
        if ok and fa in src and fb not in dst and src[fa] not in used:
            new.append((fb, src[fa], t[2] if t[2] == 'same' else 'changed %s' % t[3]))
            used.add(src[fa])
    os.makedirs(os.path.dirname(a.dst), exist_ok=True)
    with open(a.dst, 'a', newline='\n') as f:
        if not dst:
            f.write('# pc24  name   (propagated from %s via %s)\n' % (a.src, a.xmatch))
        for fb, n, how in sorted(new):
            f.write('%06X %s    # from %s (%s)\n' % (fb, n, a.src, how))
    print('%s: %d names added' % (a.dst, len(new)))


if __name__ == '__main__':
    main()
