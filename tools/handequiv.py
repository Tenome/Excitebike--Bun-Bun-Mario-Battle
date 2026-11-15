#!/usr/bin/env python3
"""Is the hand-written C the same code as at an earlier commit?

    python tools/handequiv.py BASE [s1 s2 s3 s4]

For a rename or tidy-up of c/hand: per dump, expand the RAM names
(tools/ramname.py), substitute both sides' simple object macros and enum
constants, drop comments, normalise hex literals and fold constant sums
inside operands, then compare each routine's token stream with the one at
BASE. Prints the routines that differ (and those gone); none means the C is
the same program.
"""
import re, sys, os, glob, subprocess, difflib
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASE = sys.argv[1]
vers = sys.argv[2:] or ['s1', 's2', 's3', 's4']
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import ramname, port_hand

DEFINE = re.compile(r'^#define\s+([A-Z_][A-Z0-9_]*)\s+(.+?)\s*(?:/\*.*)?$', re.M)
ENUM = re.compile(r'enum\s*\{([^}]*)\}', re.S)


def macros(text):
    m = {}
    for k, v in DEFINE.findall(text):
        if '(' in k:
            continue
        m[k] = v.strip()
    for body in ENUM.findall(text):
        body = re.sub(r'/\*.*?\*/', '', body, flags=re.S)
        for part in body.split(','):
            if '=' in part:
                k, v = part.split('=', 1)
                m[k.strip()] = v.strip()
    return m


def norm(text, mac):
    text = re.sub(r'/\*.*?\*/|//[^\n]*', ' ', text, flags=re.S)
    text = re.sub(r'^\s*#.*$', ' ', text, flags=re.M)
    text = re.sub(r'enum\s*\{[^}]*\}\s*;', ' ', text, flags=re.S)
    for _ in range(3):
        text = re.sub(r'\b[A-Z_][A-Z0-9_]*\b', lambda t: mac.get(t.group(0), t.group(0)), text)
    text = re.sub(r'\b0x([0-9A-Fa-f]+)\b', lambda h: str(int(h.group(1), 16)), text)
    # fold "N + M" constant sums
    for _ in range(3):
        text = re.sub(r'\b(\d+) \+ (\d+)\b(?! *\*)', lambda s: str(int(s.group(1)) + int(s.group(2))), text)
    return re.findall(r'\w+|[^\s\w]', text)


def git_show(path):
    return subprocess.run(['git', '-C', ROOT, 'show', '%s:%s' % (BASE, path)], capture_output=True, text=True, encoding='utf-8').stdout


total = 0
for v in vers:
    names = ramname.load(v)
    hdr = ''
    for h in ('c/hand/snes_regs.h', 'c/hand/long_ram.h', 'c/hand/%s/ram.h' % v):
        p = os.path.join(ROOT, h)
        if os.path.exists(p):
            hdr += open(p, encoding='utf-8').read()
    for f in sorted(glob.glob(os.path.join(ROOT, 'c', 'hand', v, '*.c'))):
        rel = os.path.relpath(f, ROOT).replace('\\', '/')
        new = open(f, encoding='utf-8').read()
        old = git_show(rel)
        if not old:
            continue
        nm = macros(hdr + new)
        om = macros(old)
        nf = port_hand.extract_functions(ramname.expand(new, names))
        of = port_hand.extract_functions(old)
        for k in sorted(set(nf) & set(of)):
            a, b = norm(of[k][1], om), norm(nf[k][1], nm)
            if a != b:
                total += 1
                d = [l for l in difflib.unified_diff(a, b, lineterm='', n=0) if l[:1] in '+-' and l[:3] not in ('+++', '---')]
                print('%s %s: %s' % (v, k, ' '.join(d[:16])))
        for k in sorted(set(of) - set(nf)):
            print('%s %s: GONE' % (v, k))
print('functions differing:', total)
