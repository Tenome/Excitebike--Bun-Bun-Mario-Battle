#!/usr/bin/env python3
"""Carry hand-written routines over to another version when the code is identical.

    python tools/port_hand.py s1 s4 [--out c/hand/s4/from_s1.c]

Follows the cross-version function maps (build/xm/s1_s2.txt, s2_s3.txt,
s3_s4.txt) from each routine in c/hand/<src>/hand.txt to its counterpart and
keeps those whose instruction bytes are identical: same operands, so the same
RAM, tables and hardware. Their C is copied with the routine address in the
comment and the pushed return addresses (push16 constants inside the routine)
moved by the address difference; static helpers they use come along. Routines
that call a hand routine the target version does not have are left out.
Writes the C file and prints hand.txt / names.txt lines to add; the copies
still have to pass tools/difftest816.py on the target.
"""
import argparse
import json
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def load_map(f):
    m = {}
    for line in open(f):
        t = line.split()
        if len(t) >= 3 and t[0] != '------' and t[1] != '------':
            m[t[0]] = t[1]
    return m


def routine_bytes(ver, f):
    code = json.load(open(os.path.join(ROOT, 'build', ver, 'code.json')))
    rom = open(os.path.join(ROOT, 'orig', ver + '.bs'), 'rb').read()
    ins = {int(k, 16): v for k, v in code['insn'].items()}
    funcs = sorted(int(k, 16) for k in code['funcs'])
    i = funcs.index(f)
    end = funcs[i + 1] if i + 1 < len(funcs) else f + 0x100
    o = lambda a: ((a >> 16) & 0x7F) << 15 | (a & 0x7FFF)
    return b''.join(rom[o(p):o(p) + ins[p][2]] for p in sorted(ins) if f <= p < end), end


def routine_insns(ver, f):
    code = json.load(open(os.path.join(ROOT, 'build', ver, 'code.json')))
    ins = {int(k, 16): v for k, v in code['insn'].items()}
    funcs = sorted(int(k, 16) for k in code['funcs'])
    i = funcs.index(f)
    end = funcs[i + 1] if i + 1 < len(funcs) else f + 0x100
    return [(p, ins[p][0], ins[p][1], ins[p][3]) for p in sorted(ins) if f <= p < end]


def operand_map(src_ins, dst_ins):
    """old -> new data addresses when the two routines differ only in
    absolute/long operands (same mnemonics and modes); None otherwise."""
    if len(src_ins) != len(dst_ins):
        return None
    m = {}
    for (pa, ma, oa, va), (pb, mb, ob, vb) in zip(src_ins, dst_ins):
        if ma != mb or oa != ob:
            return None
        if va == vb:
            continue
        if oa in ('abs', 'absx', 'absy', 'long', 'longx') and isinstance(va, int) and isinstance(vb, int):
            if ma in ('JSR', 'JSL', 'JMP', 'JML'):
                return None                  # calls elsewhere: not a plain data move
            if m.get(va, vb) != vb:
                return None
            m[va] = vb
        elif oa in ('rel', 'rell'):
            continue                         # branch distances move with the code
        else:
            return None
    return m


def extract_functions(src):
    """name -> (text with the comment above it, body); static helpers too."""
    out = {}
    pat = re.compile(r'^((?:/\*(?:.|\n)*?\*/\n)?)((?:static )?(?:inline )?(?:void|u8|u16|u32|int) (\w+)\([^)]*\)\n\{\n(?:.|\n)*?\n\})', re.M)
    for m in pat.finditer(src):
        out[m.group(3)] = (m.group(1) + m.group(2), m.group(2))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('src')
    ap.add_argument('dst')
    ap.add_argument('--out')
    ap.add_argument('--operands', action='store_true', help='also routines that differ only in data addresses')
    a = ap.parse_args()
    chain = {('s1', 's4'): ['s1_s2', 's2_s3', 's3_s4']}[(a.src, a.dst)]
    maps = [load_map(os.path.join(ROOT, 'build', 'xm', c + '.txt')) for c in chain]
    hand = []
    for line in open(os.path.join(ROOT, 'c', 'hand', a.src, 'hand.txt')):
        t = line.split('#')[0].split()
        if len(t) >= 5:
            hand.append(t)
    dst_hand, dst_pcs = set(), set()
    hp = os.path.join(ROOT, 'c', 'hand', a.dst, 'hand.txt')
    mark = '# from %s' % a.src
    for line in open(hp):
        t = line.split('#')[0].split()
        if len(t) >= 5 and mark not in line:
            dst_hand.add(t[4])
            dst_pcs.add(t[0].upper())
    srcs, defines, oneliners = {}, [], {}
    for fn in sorted(os.listdir(os.path.join(ROOT, 'c', 'hand', a.src))):
        if fn.endswith('.c'):
            text = open(os.path.join(ROOT, 'c', 'hand', a.src, fn), encoding='utf-8').read()
            srcs.update(extract_functions(text))
            for m in re.finditer(r'^enum \{(?:.|\n)*?\};', text, re.M):
                if m.group(0) not in defines:
                    defines.append(m.group(0))
            for line in text.splitlines():
                if line.startswith('#define') and line not in defines:
                    defines.append(line)
                m = re.match(r'static inline \w+ \*?(\w+)\([^)]*\)\s*\{.*\}\s*$', line)
                if m:
                    oneliners[m.group(1)] = line
    picked = []
    for t in hand:
        pc, name = t[0], t[4]
        d = pc
        for m in maps:
            d = m.get(d)
            if not d:
                break
        if not d or name in dst_hand or d.upper() in dst_pcs:
            continue
        try:
            x, _ = routine_bytes(a.src, int(pc, 16))
            y, _ = routine_bytes(a.dst, int(d, 16))
        except (ValueError, KeyError):
            continue
        if x == y:
            picked.append((t, d, {}))
        elif a.operands:
            om = operand_map(routine_insns(a.src, int(pc, 16)), routine_insns(a.dst, int(d, 16)))
            if om:
                picked.append((t, d, om))
    names_ok = dst_hand | {t[4] for t, _, _ in picked}
    out_funcs, helpers, lines_hand, lines_names = [], [], [], []
    for t, d, om in picked:
        name = t[4]
        fname = name + '_m%sx%s' % (t[1], t[2])
        if fname not in srcs:
            continue
        text, body = srcs[fname]
        calls = set(re.findall(r'\b(\w+)_m\dx\d\(\)', body)) - {name}
        if any(c not in names_ok for c in calls):
            print('skip %s: calls %s' % (name, ', '.join(sorted(c for c in calls if c not in names_ok))))
            continue
        delta = int(d, 16) - int(t[0], 16)
        lo, hi = int(t[0], 16) & 0xFFFF, (int(t[0], 16) & 0xFFFF) + 0x400
        def fix(m):
            v = int(m.group(1), 16)
            return 'push16(0x%04X)' % ((v + delta) & 0xFFFF) if lo <= v < hi else m.group(0)
        text = re.sub(r'push16\(0x([0-9A-Fa-f]{4})\)', fix, text)
        for old, new in sorted(om.items(), key=lambda kv: -kv[0]):
            w = 6 if old > 0xFFFF else 4
            text = re.sub(r'0x0*%X\b' % old, '0x%0*X' % (w, new), text, flags=re.I)
        text = text.replace('$%s:%s' % (t[0][:2], t[0][2:]), '$%s:%s' % (d[:2], d[2:]))
        for h in re.findall(r'\b(\w+)\(', body):
            if h in srcs and srcs[h][1].startswith('static') and h not in helpers:
                helpers.append(h)
        out_funcs.append(text)
        lines_hand.append('%s  %s %s  %-14s %-22s # from %s %s' % (d, t[1], t[2], t[3], name, a.src, t[0]))
        lines_names.append('%s %-21s # from %s %s' % (d, name, a.src, t[0]))
    protos = sorted({'void %s_m1x1(void);' % c for f in out_funcs for c in re.findall(r'\b(\w+)_m1x1\(\)', f)})
    head = ['/*', ' * Routines identical (instruction for instruction) to %s\'s hand-written' % a.src.upper(),
            ' * ones, carried over by tools/port_hand.py with their return addresses', ' * moved. Checked by tools/difftest816.py on %s.' % a.dst, ' */',
            '#include "hand/hand.h"', '']
    used = '\n'.join(out_funcs + [srcs[h][0] for h in helpers])
    inl = [l for n, l in oneliners.items() if re.search(r'\b%s\(' % n, used)]
    body = head + defines + [''] + inl + [''] + protos + [''] + [srcs[h][0] for h in helpers] + [''] + out_funcs
    outp = a.out or os.path.join(ROOT, 'c', 'hand', a.dst, 'from_%s.c' % a.src)
    open(outp, 'w', encoding='utf-8', newline='\n').write('\n'.join(body) + '\n')
    print('wrote %s: %d routines' % (os.path.relpath(outp, ROOT), len(out_funcs)))
    for rel, new in ((os.path.join('c', 'hand', a.dst, 'hand.txt'), lines_hand),
                     (os.path.join('config', a.dst, 'names.txt'), lines_names)):
        path = os.path.join(ROOT, rel)
        old = [l for l in open(path, encoding='utf-8').read().splitlines() if mark not in l]
        open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(old + new) + '\n')
        print('updated %s (%d lines from %s)' % (rel, len(new), a.src))


if __name__ == '__main__':
    main()
