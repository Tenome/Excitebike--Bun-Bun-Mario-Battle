#!/usr/bin/env python3
"""Name the RAM and register operands of the hand-written C.

    python tools/ramname.py s4            # write the names into c/hand/s4/*.c
    python tools/ramname.py s4 --check    # list what would change

The names come from c/hand/<ver>/ram.h, c/hand/long_ram.h and
c/hand/snes_regs.h (lines `#define NAME 0xADDR`). Only address operands are
rewritten: ABS/ABSX/ABSY(0xADDR), DP/DPX/DPY(0xAD) and LONG/LONGX(0xBBADDR);
immediates, table addresses and comments stay as they are. A name whose
comment starts with [X] is a record field and goes on indexed operands only.
A comment holding [+$N] marks a buffer or array of N bytes: an absolute or
long address inside it with no name of its own is written NAME + 0xOFF
(OAM_BUF + 0x11, RIDER_X + 0x01 for rider 1).
Two cases are left alone because the address would mean something else there:
  * DP operands in a function that moves the direct page (D = ...);
  * absolute operands in a function that sets the data bank to $7F.
Direct-page names are applied only from $20 up ($00-$1F is scratch).

tools/port_hand.py calls expand() to turn the names back into hex before it
translates a routine to another dump, and name_text() on its output when the
target has a ram.h of its own.
"""
import argparse
import glob
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEF = re.compile(r'^#define\s+(\w+)\s+0x([0-9A-Fa-f]+)\b')
OPER = re.compile(r'\b(ABS|ABSX|ABSY|DP|DPX|DPY|LONG|LONGX)\((0x[0-9A-Fa-f]+|[A-Z_][A-Z0-9_]*)\)')
INDEXED = ('ABSX', 'ABSY', 'DPX', 'DPY', 'LONGX')
DEF_INDEXED = re.compile(r'/\*\s*\[X\]')   # "/* [X] ..." : indexed operands only
DEF_SPAN = re.compile(r'\[\+\$([0-9A-Fa-f]+)\]')   # "[+$200]": a buffer of that size
COMP_OPER = re.compile(r'\b(ABS|ABSX|ABSY|DP|DPX|DPY|LONG|LONGX)\((0x[0-9A-Fa-f]+) \+ ')
SPAN_OPER = re.compile(r'\b(ABS|ABSX|ABSY|LONG|LONGX)\(([A-Z_][A-Z0-9_]*) \+ 0x([0-9A-Fa-f]+)\)')
DP_MIN = 0x20


class Names(dict):
    """name -> address, plus the names that apply to indexed operands only
    (the fields of a record addressed as base + X) and the sizes of the
    buffers whose inner addresses are written NAME + offset."""
    def __init__(self):
        super().__init__()
        self.indexed = set()
        self.spans = {}


def load(ver):
    """name -> address for <ver> (its ram.h, the long addresses every dump
    shares and the register names); empty when the version has no ram.h."""
    names = Names()
    ram = os.path.join(ROOT, 'c', 'hand', ver, 'ram.h')
    if not os.path.exists(ram):
        return names
    for path in (os.path.join(ROOT, 'c', 'hand', 'snes_regs.h'),
                 os.path.join(ROOT, 'c', 'hand', 'long_ram.h'), ram):
        if not os.path.exists(path):
            continue
        for line in open(path, encoding='utf-8'):
            m = DEF.match(line)
            if m:
                names[m.group(1)] = int(m.group(2), 16)
                if DEF_INDEXED.search(line):
                    names.indexed.add(m.group(1))
                s = DEF_SPAN.search(line)
                if s:
                    names.spans[m.group(1)] = int(s.group(1), 16)
    return names


def width(mode, addr):
    if mode.startswith('LONG'):
        return 6
    return 2 if mode.startswith('DP') else 4


def chunks(text):
    """Split a file at the closing braces of top-level functions, so each
    piece holds one function and the comment above it."""
    out, start = [], 0
    for m in re.finditer(r'\n\}\n', text):
        out.append(text[start:m.end()])
        start = m.end()
    out.append(text[start:])
    return out


def moves_dp(chunk):
    return bool(re.search(r'(?<![\w.])D = (?!\$0000\b)', chunk))


def far_bank(chunk):
    return bool(re.search(r'push8\(0x7F\);\s*DB = ', chunk))


def name_text(text, names):
    """Rewrite hex address operands to names."""
    indexed = getattr(names, 'indexed', set())
    by_plain, by_idx = {}, {}
    for n, a in names.items():
        if n in indexed:
            by_idx.setdefault(a, n)
        else:
            by_plain.setdefault(a, n)
    for a, n in by_plain.items():
        by_idx.setdefault(a, n)
    spans = sorted((names[n], names[n] + size, n)
                   for n, size in getattr(names, 'spans', {}).items())

    def in_span(a):
        for lo, hi, n in spans:
            if lo < a < hi:
                return '%s + 0x%02X' % (n, a - lo)
        return None
    out = []
    for c in chunks(text):
        abs_ok = not far_bank(c)     # DB = $7F: absolute operands are bank $7F
        dp_ok = not moves_dp(c)

        def lookup(mode, a, indexed):
            if mode.startswith('DP'):
                if not dp_ok or a < DP_MIN or a > 0xFF:
                    return None
            elif mode.startswith('ABS'):
                if not abs_ok:
                    return None
            n = (by_idx if indexed else by_plain).get(a)
            if n is None and not mode.startswith('DP'):
                n = in_span(a)
            return n

        def one(m):
            mode, val = m.group(1), m.group(2)
            if not val.startswith('0x'):
                return m.group(0)
            n = lookup(mode, int(val, 16), mode in INDEXED)
            return '%s(%s)' % (mode, n) if n else m.group(0)

        def comp(m):     # ABS(0x01F6 + X): a base with a computed index
            mode, val = m.group(1), m.group(2)
            n = lookup(mode, int(val, 16), True)
            return '%s(%s + ' % (mode, n) if n else m.group(0)
        c = OPER.sub(one, c)
        out.append(COMP_OPER.sub(comp, c))
    return ''.join(out)


def expand(text, names):
    """Turn named address operands back into hex (DP as 2 digits, ABS as 4)."""
    def span(m):
        mode, n, off = m.group(1), m.group(2), int(m.group(3), 16)
        if n not in names:
            return m.group(0)
        a = names[n] + off
        return '%s(0x%0*X)' % (mode, width(mode, a), a)
    text = SPAN_OPER.sub(span, text)

    def one(m):
        mode, val = m.group(1), m.group(2)
        if val.startswith('0x') or val not in names:
            return m.group(0)
        a = names[val]
        return '%s(0x%0*X)' % (mode, width(mode, a), a)
    text = OPER.sub(one, text)

    # names used as plain values (read_pad(..., P1_HELD_LO, ...)), outside
    # comments and strings
    def bare(m):
        w = m.group(0)
        if w[0] in '/"' or w not in names:
            return w
        a = names[w]
        return '0x%0*X' % (2 if a < 0x100 else 4 if a < 0x10000 else 6, a)
    return re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\b[A-Z_][A-Z0-9_]*\b', bare, text, flags=re.S)


def add_include(text):
    if '#include "ram.h"' in text:
        return text
    lines = text.split('\n')
    first = next((i for i, l in enumerate(lines) if l.startswith('#include')), None)
    if first is None:
        return text
    lines.insert(first + 1, '#include "ram.h"')
    return '\n'.join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('ver')
    ap.add_argument('--check', action='store_true')
    a = ap.parse_args()
    names = load(a.ver)
    if not names:
        raise SystemExit('no c/hand/%s/ram.h' % a.ver)
    total = 0
    for f in sorted(glob.glob(os.path.join(ROOT, 'c', 'hand', a.ver, '*.c'))):
        text = open(f, encoding='utf-8').read()
        new = name_text(text, names)
        if new == text:
            continue
        n = sum(1 for x, y in zip(OPER.findall(text), OPER.findall(new)) if x != y)
        total += n
        print('%-14s %d operands' % (os.path.basename(f), n))
        if not a.check:
            open(f, 'w', encoding='utf-8', newline='\n').write(add_include(new))
    print('total', total)


if __name__ == '__main__':
    main()
