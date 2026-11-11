#!/usr/bin/env python3
"""Parse the SPC700 upload streams that SpcUpload ($80:88E0) sends.

    python tools/spcblocks.py s1 [--table 8088CE] [--count 6] [--extract DIR]

Stream format (standard IPL transfer, as read by the uploader with [$00],Y and
a bank increment at $8000): repeated [len16][dest16][len bytes], ending with
len = 0 followed by the SPC start address. Prints each chunk's ARAM range and
the ROM span the stream occupies; --extract writes the chunks as files.
"""
import argparse
import os


def rd(rom, addr):
    b, a = addr >> 16, addr & 0xFFFF
    return rom[((b & 0x1F) << 15) | (a & 0x7FFF)]


def adv(addr):
    b, a = addr >> 16, (addr & 0xFFFF) + 1
    if a > 0xFFFF:
        b, a = b + 1, 0x8000
    return (b << 16) | a


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('--table', default='8088CE')
    ap.add_argument('--count', type=int, default=6)
    ap.add_argument('--extract')
    a = ap.parse_args()
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    t = int(a.table, 16)
    for i in range(a.count):
        lo, hi, bk = rd(rom, t + i), rd(rom, t + a.count + i), rd(rom, t + 2 * a.count + i)
        start = cur = bk << 16 | hi << 8 | lo
        print('block %d at $%06X' % (i, start))
        chunks = []
        for _ in range(64):
            n = rd(rom, cur); cur = adv(cur); n |= rd(rom, cur) << 8; cur = adv(cur)
            dst = rd(rom, cur); cur = adv(cur); dst |= rd(rom, cur) << 8; cur = adv(cur)
            if n == 0:
                print('  end, SPC start $%04X; stream ends at $%06X (%d bytes)' % (dst, cur, sum(c[1] for c in chunks) + 4 * (len(chunks) + 1)))
                break
            data = bytearray()
            for _k in range(n):
                data.append(rd(rom, cur)); cur = adv(cur)
            chunks.append((dst, n, data))
            print('  ARAM $%04X-$%04X  %5d bytes' % (dst, dst + n - 1, n))
        if a.extract:
            os.makedirs(a.extract, exist_ok=True)
            for dst, n, data in chunks:
                open(os.path.join(a.extract, 'block%d_%04X.bin' % (i, dst)), 'wb').write(data)


if __name__ == '__main__':
    main()
