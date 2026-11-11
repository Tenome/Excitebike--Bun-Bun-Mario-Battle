#!/usr/bin/env python3
"""Match functions between two versions by address-independent signature.

    python tools/xmatch.py build/s1/code.json build/s2/code.json [--out FILE]

Each function's signature is its instruction stream in address order from the
entry up to the next function entry (or a gap). Operands that look like ROM
addresses (>= $8000 in abs/long modes, branch targets) are masked; RAM and
register operands and immediates are kept. Functions with equal signatures
are 'same'; the rest are paired greedily by similarity ('changed') or left
'unmatched'.
"""
import argparse
import difflib
import json

ADDR_MODES = {'abs', 'absx', 'absy', 'long', 'longx', 'iabs', 'iabsx', 'ilabs'}


def load(path):
    j = json.load(open(path))
    insn = {int(k, 16): v for k, v in j['insn'].items()}
    funcs = sorted(int(k, 16) for k in j['funcs'])
    return insn, funcs


def bodies(insn, funcs):
    fset = set(funcs)
    out = {}
    for f in funcs:
        toks, pc, n = [], f, 0
        while pc in insn and n < 4000:
            mn, mode, size, op = insn[pc]
            if mode in ('rel', 'rell'):
                t = mn
            elif mode in ADDR_MODES and (op & 0xFFFF) >= 0x8000:
                t = '%s %s @' % (mn, mode)
            else:
                t = '%s %s %X' % (mn, mode, op)
            toks.append(t)
            pc = (pc & 0xFF0000) | ((pc + size) & 0xFFFF)
            n += 1
            if pc in fset:
                break
        out[f] = toks
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('a')
    ap.add_argument('b')
    ap.add_argument('--out')
    ap.add_argument('--min-ratio', type=float, default=0.6)
    args = ap.parse_args()
    ia, fa = load(args.a)
    ib, fb = load(args.b)
    ba, bb = bodies(ia, fa), bodies(ib, fb)
    sig_b = {}
    for f, t in bb.items():
        sig_b.setdefault(tuple(t), []).append(f)
    pairs, used_b = {}, set()
    for f, t in ba.items():
        c = [g for g in sig_b.get(tuple(t), []) if g not in used_b]
        if c:
            g = min(c, key=lambda g: abs((g & 0xFFFF) - (f & 0xFFFF)))
            pairs[f] = (g, 1.0)
            used_b.add(g)
    # fuzzy pass, ordered by address so neighbours line up
    rest_b = [g for g in fb if g not in used_b]
    for f in fa:
        if f in pairs:
            continue
        best, br = None, 0
        for g in rest_b:
            if g in used_b or (g >> 16) != (f >> 16):
                continue
            la, lb = len(ba[f]), len(bb[g])
            if min(la, lb) / max(la, lb, 1) < args.min_ratio:
                continue
            r = difflib.SequenceMatcher(None, ba[f], bb[g], autojunk=False).quick_ratio()
            if r > br:
                r = difflib.SequenceMatcher(None, ba[f], bb[g], autojunk=False).ratio()
                if r > br:
                    best, br = g, r
        if best is not None and br >= args.min_ratio:
            pairs[f] = (best, br)
            used_b.add(best)
    same = sum(1 for v in pairs.values() if v[1] == 1.0)
    changed = len(pairs) - same
    lines = ['# %s vs %s: %d funcs vs %d; same %d, changed %d, only-A %d, only-B %d' % (
        args.a, args.b, len(fa), len(fb), same, changed, len(fa) - len(pairs), len(fb) - len(used_b))]
    for f in fa:
        if f in pairs:
            g, r = pairs[f]
            lines.append('%06X %06X %s' % (f, g, 'same' if r == 1.0 else 'changed %.2f' % r))
        else:
            lines.append('%06X ------ only-A' % f)
    for g in fb:
        if g not in used_b:
            lines.append('------ %06X only-B' % g)
    text = '\n'.join(lines) + '\n'
    if args.out:
        open(args.out, 'w').write(text)
    print(lines[0])


if __name__ == '__main__':
    main()
