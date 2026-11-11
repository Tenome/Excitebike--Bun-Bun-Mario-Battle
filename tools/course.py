#!/usr/bin/env python3
"""Decode the course tables into column lists (the $7F:4000 track buffer).

    python tools/course.py s1            # Gen A tables at $81:8C59/8C6D/8C81
    python tools/course.py s4 --genb     # Gen B tables at $81:8C62/8C76/8C8A

Format (from the loader at $81:A01B / Gen B $81:A11E area):
  for each source byte b (the length table counts source bytes):
    out[y] = b
    if b == $09: out[y+1..y+5] = $09         (written ahead; later bytes overwrite)
    if b & RUN:  n = next byte; out[y..y+n-1] = b; y += n - 1
    y += 1
  RUN is $40 in Gen A (ground = $40) and $80 in Gen B (ground = $80); Gen B
  also randomises some columns afterwards during races ($81:A18B).
The output buffer is $1000 bytes, zero-filled first.
Prints a histogram of codes per course and the expanded length; --dump
writes build/<ver>/course<N>.bin.
"""
import argparse
import collections
import os


def rd(rom, a):
    return rom[((a >> 16) & 0x1F) * 0x8000 + (a & 0x7FFF)]


def decode(rom, data_base, start, length, run_bit=0x40):
    out = bytearray(0x1000)
    x, y, n = start, 0, length
    while n > 0:
        b = rd(rom, data_base + x)
        out[y] = b
        if b == 0x09:
            for k in range(1, 6):
                out[y + k] = 0x09
        if b & run_bit:
            n -= 1
            x += 1
            cnt = rd(rom, data_base + x)
            for k in range(cnt):
                out[y + k] = b
            y += cnt - 1
        x += 1
        y += 1
        n -= 1
    return out[:y], y


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('--genb', action='store_true')
    ap.add_argument('--dump', action='store_true')
    a = ap.parse_args()
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    st, ln, db = (0x818C62, 0x818C76, 0x818C8A) if a.genb else (0x818C59, 0x818C6D, 0x818C81)
    for i in range(10):
        s = rd(rom, st + 2 * i) | rd(rom, st + 2 * i + 1) << 8
        l = rd(rom, ln + 2 * i) | rd(rom, ln + 2 * i + 1) << 8
        cols, n = decode(rom, db, s, l, 0x80 if a.genb else 0x40)
        hist = collections.Counter(cols)
        label = ['St.%s course %d' % ('A1' if i < 4 else 'A2', i % 4 + 1)][0] if i < 8 else 'special %d' % (i - 8)
        if a.genb and i < 8:
            label = 'St.%d course %d' % (3 + i // 4, i % 4 + 1)
        elif i < 8:
            label = 'St.%d course %d' % (1 + i // 4, i % 4 + 1)
        print('%-16s src %3d bytes -> %4d columns; codes: %s' % (
            label, l, n, ' '.join('%02X:%d' % kv for kv in sorted(hist.items()))))
        if a.dump:
            os.makedirs('build/%s' % a.ver, exist_ok=True)
            open('build/%s/course%d.bin' % (a.ver, i), 'wb').write(cols)


if __name__ == '__main__':
    main()
