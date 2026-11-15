#!/usr/bin/env python3
"""Compare the two engines routine by routine, in the hand-written C.

    python tools/abdiff.py [--a s1] [--b s4] [--out docs/ENGINES.md]

Pairs each Gen A routine with its Gen B twin through the cross-version
function maps (build/xm/s1_s2.txt, s2_s3.txt, s3_s4.txt; made by
tools/xmatch.py), then compares the two C bodies with the RAM named
(c/hand/<ver>/ram.h): comments, whitespace and ROM addresses (constants of
$8000 and up: return addresses, tables, routines) are left out, so two
routines count as the same when they do the same thing to the same named
state even though the engines lay RAM and ROM out differently.

Each Gen A routine ends up in one of:
  same     the C is equal after that normalisation;
  changed  paired, but the C differs (similarity in %, with a short
           line-level diff count);
  only A   no Gen B twin (or its twin is not hand-written C).
and the Gen B routines nothing pairs with are listed as 'only B'.
"""
import argparse
import difflib
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import port_hand  # noqa: E402
import ramname    # noqa: E402

ORDER = ['s1', 's2', 's3', 's4']


def hand_table(ver):
    """pc24 -> C function name (name_mMxX) from c/hand/<ver>/hand.txt."""
    out = {}
    for line in open(os.path.join(ROOT, 'c', 'hand', ver, 'hand.txt'), encoding='utf-8'):
        if line.startswith('#') or not line.strip():
            continue
        f = line.split()
        if len(f) >= 5:
            out.setdefault(int(f[0], 16), []).append('%s_m%sx%s' % (f[4], f[1], f[2]))
    return out


def xmap(a, b):
    """pc24 in a -> pc24 in b following build/xm (same and changed pairs)."""
    i, j = ORDER.index(a), ORDER.index(b)
    step = 1 if j > i else -1
    m = None
    for k in range(i, j, step):
        x, y = ORDER[k], ORDER[k + step]
        fwd = step == 1
        name = '%s_%s.txt' % ((x, y) if fwd else (y, x))
        pairs = {}
        for line in open(os.path.join(ROOT, 'build', 'xm', name)):
            if line.startswith('#'):
                continue
            f = line.split()
            if len(f) < 3 or '-' in f[0] or '-' in f[1]:
                continue
            p, q = int(f[0], 16), int(f[1], 16)
            if fwd:
                pairs[p] = q
            else:
                pairs[q] = p
        m = pairs if m is None else {s: pairs[t] for s, t in m.items() if t in pairs}
    return m


def bodies(ver):
    out = {}
    for f in glob.glob(os.path.join(ROOT, 'c', 'hand', ver, '*.c')):
        for k, v in port_hand.extract_functions(open(f, encoding='utf-8').read()).items():
            out.setdefault(k, v[1])
    return out


ROM = re.compile(r'\b0x(?:[0-9A-Fa-f]{2})?[89A-Fa-f][0-9A-Fa-f]{3}\b')


def norm(body, names):
    body = ramname.name_text(body, names)
    body = re.sub(r'/\*.*?\*/|//[^\n]*', '', body, flags=re.S)
    body = re.sub(r'^[^\n]*\(', '(', body, count=1)            # drop the function's own name
    body = re.sub(r'\b(\w+)_m[01]x[01]\b', r'\1', body)        # variant suffixes
    body = ROM.sub('ROM', body)
    return [l.strip() for l in body.split('\n') if l.strip()]


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--a', default='s1')
    ap.add_argument('--b', default='s4')
    ap.add_argument('--out')
    a = ap.parse_args()
    ha, hb = hand_table(a.a), hand_table(a.b)
    ba, bb = bodies(a.a), bodies(a.b)
    na, nb = ramname.load(a.a), ramname.load(a.b)
    m = xmap(a.a, a.b)
    # callee names: a Gen A routine's twin may carry another name
    # (QueueSound / QueueSoundB); compare calls by the twin's name
    rename = {}
    for pc, q in m.items():
        if pc in ha and q in hb:
            x, y = ha[pc][0].rsplit('_m', 1)[0], hb[q][0].rsplit('_m', 1)[0]
            if x != y:
                rename[x] = y
    call = re.compile(r'\b(\w+)(?=\(\))')

    def norm_a(body):
        return [call.sub(lambda c: rename.get(c.group(1), c.group(1)), l) for l in norm(body, na)]
    by_name_b = {}
    for q, fs in hb.items():
        by_name_b.setdefault(fs[0].rsplit('_m', 1)[0], q)
    same, changed, only_a = [], [], []
    used_b = set()
    for pc in sorted(ha):
        fa = [f for f in ha[pc] if f in ba]
        if not fa:
            continue
        fname = fa[0]
        q = m.get(pc)
        fb = [f for f in hb.get(q, []) if f in bb] if q is not None else []
        if not fb:          # the maps lost it: pair by the routine's name
            base = fname.rsplit('_m', 1)[0]
            q = by_name_b.get(base)
            if q is not None and q not in m.values():
                fb = [f for f in hb[q] if f in bb]
        if not fb:
            only_a.append((pc, fname))
            continue
        # pair the same M/X variant where both exist
        suffix = fname[-5:]
        gb = next((f for f in fb if f.endswith(suffix)), fb[0])
        used_b.add(q)
        x, y = norm_a(ba[fname]), norm(bb[gb], nb)
        if x == y:
            same.append((pc, q, fname))
        else:
            r = difflib.SequenceMatcher(None, x, y).ratio()
            d = sum(1 for l in difflib.ndiff(x, y) if l[:1] in '+-')
            changed.append((pc, q, fname, gb, r, d))
    only_b = sorted((q, hb[q][0]) for q in hb if q not in used_b and any(f in bb for f in hb[q]))

    lines = ['# Gen A and Gen B, routine by routine',
             '',
             'Made by `tools/abdiff.py --a %s --b %s` from the hand-written C, with the RAM' % (a.a, a.b),
             'named on both sides and ROM addresses masked. Pairs come from the',
             'cross-version function maps (`build/xm`).',
             '',
             '| | Routines |', '|---|---|',
             '| same | %d |' % len(same), '| changed | %d |' % len(changed),
             '| only in %s | %d |' % (a.a.upper(), len(only_a)), '| only in %s | %d |' % (a.b.upper(), len(only_b)),
             '',
             '## Changed (most similar first)', '',
             '| %s | %s | routine | similarity | lines differing |' % (a.a.upper(), a.b.upper()),
             '|---|---|---|---|---|']
    for pc, q, fname, gb, r, d in sorted(changed, key=lambda t: -t[4]):
        lines.append('| `%06X` | `%06X` | %s | %d%% | %d |' % (pc, q, fname.rsplit('_m', 1)[0], round(r * 100), d))
    lines += ['', '## Only in %s' % a.a.upper(), '']
    lines += ['* `%06X` %s' % (pc, f.rsplit('_m', 1)[0]) for pc, f in only_a]
    lines += ['', '## Only in %s' % a.b.upper(), '']
    lines += ['* `%06X` %s' % (q, f.rsplit('_m', 1)[0]) for q, f in only_b]
    lines += ['', '## Same', '']
    lines += [', '.join('%s' % f.rsplit('_m', 1)[0] for _, _, f in same)]
    text = '\n'.join(lines) + '\n'
    if a.out:
        open(a.out, 'w', encoding='utf-8', newline='\n').write(text)
    print('same %d changed %d only-%s %d only-%s %d' % (len(same), len(changed), a.a, len(only_a), a.b, len(only_b)))


if __name__ == '__main__':
    main()
