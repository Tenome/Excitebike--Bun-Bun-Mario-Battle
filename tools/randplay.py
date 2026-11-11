#!/usr/bin/env python3
"""Write a random-input snesrecomp host script (--script) for coverage runs.

    python tools/randplay.py SEED FRAMES out.txt [--p2]

Mixes menu-style taps with long gameplay holds (B/Y throttle plus d-pad), and
periodic Start/A to get through menus. Deterministic for a given seed.
"""
import argparse
import random

TAPS = ['start', 'a', 'b', 'x', 'y', 'select', 'up', 'down', 'left', 'right', 'l', 'r']
HOLD = ['b', 'y', 'a', 'b+right', 'b+up', 'b+down', 'b+left', 'y+up', 'y+down', 'b+a',
        'b+l', 'b+r', 'y+right', 'right', 'up', 'down']


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('seed', type=int)
    ap.add_argument('frames', type=int)
    ap.add_argument('out')
    ap.add_argument('--p2', action='store_true', help='also drive player 2')
    a = ap.parse_args()
    rnd = random.Random(a.seed)
    out = ['turbo on', 'wait %d' % rnd.randint(200, 900)]
    t = 0
    while t < a.frames:
        r = rnd.random()
        if r < 0.45:
            b, n = rnd.choice(TAPS), rnd.randint(2, 6)
        else:
            b, n = rnd.choice(HOLD), rnd.randint(20, 400)
        if a.p2 and rnd.random() < 0.5:
            b += '+p2:' + rnd.choice(TAPS + ['b', 'y'])
        out.append('press %s %d' % (b, n))
        w = rnd.randint(0, 90) if r < 0.45 else rnd.randint(0, 20)
        if w:
            out.append('wait %d' % w)
        t += n + w + 1
    out.append('quit')
    open(a.out, 'w').write('\n'.join(out) + '\n')


if __name__ == '__main__':
    main()
