#!/usr/bin/env python3
"""Merge ROM read maps into a data-region map.

    python tools/romregions.py s1 'E:/.../romread/s1/*/romread.bin' [--gap 16]

Classes per byte: code (static map), cpu-data (read by the CPU, not code),
dma (read by DMA), both, unread, and pad ($FF/$00 runs never read).
Writes build/<ver>/regions.txt (contiguous regions) and prints per-bank totals.
"""
import argparse
import glob
import json
import os


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('maps')
    ap.add_argument('--gap', type=int, default=16)
    a = ap.parse_args()
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    n = len(rom)
    acc = bytearray(n)
    files = glob.glob(a.maps)
    for f in files:
        b = open(f, 'rb').read()
        for i in range(min(n, len(b))):
            acc[i] |= b[i]
    code = json.load(open('build/%s/code.json' % a.ver))
    is_code = bytearray(n)
    for k, v in code['insn'].items():
        pc = int(k, 16)
        off = ((pc >> 16) & 0x1F) * 0x8000 + (pc & 0x7FFF)
        for i in range(v[2]):
            if off + i < n:
                is_code[off + i] = 1
    cls = []
    for i in range(n):
        if is_code[i]:
            c = 'code'
        elif acc[i] & 2 and acc[i] & 1:
            c = 'cpu+dma'
        elif acc[i] & 2:
            c = 'dma'
        elif acc[i] & 1:
            c = 'cpu'
        else:
            c = 'unread'
        cls.append(c)
    # padding: unread runs of a single fill byte >= 64
    i = 0
    while i < n:
        if cls[i] == 'unread':
            j = i
            while j < n and cls[j] == 'unread' and rom[j] == rom[i]:
                j += 1
            if j - i >= 64 and rom[i] in (0xFF, 0x00):
                for k in range(i, j):
                    cls[k] = 'pad'
            i = max(j, i + 1)
        else:
            i += 1
    # regions, merging small gaps of the same class
    regions = []
    i = 0
    while i < n:
        c = cls[i]
        j = i + 1
        while j < n:
            if cls[j] == c:
                j += 1
                continue
            k = j
            while k < n and k - j < a.gap and cls[k] != c:
                k += 1
            if k < n and k - j < a.gap and cls[k] == c and c not in ('pad', 'unread'):
                j = k
                continue
            break
        regions.append((i, j, c))
        i = j
    tot = {}
    for c in cls:
        tot[c] = tot.get(c, 0) + 1
    out = ['# ROM regions for %s from %d read maps' % (a.ver, len(files)),
           '# totals: ' + ', '.join('%s %d' % kv for kv in sorted(tot.items())), '']
    for s, e, c in regions:
        if e - s < 4 and c in ('unread',):
            continue
        out.append('$%02X:%04X-$%02X:%04X  %6d  %s' % (0x80 | s >> 15, 0x8000 | (s & 0x7FFF),
                                                     0x80 | (e - 1) >> 15, 0x8000 | ((e - 1) & 0x7FFF), e - s, c))
    open('build/%s/regions.txt' % a.ver, 'w').write('\n'.join(out) + '\n')
    print(out[1])
    for b in range(n // 0x8000):
        seg = cls[b * 0x8000:(b + 1) * 0x8000]
        cnt = {c: seg.count(c) for c in ('code', 'cpu', 'dma', 'cpu+dma', 'unread', 'pad')}
        print('$%02X ' % (0x80 | b) + ' '.join('%s=%d' % (k, v) for k, v in cnt.items() if v))


if __name__ == '__main__':
    main()
