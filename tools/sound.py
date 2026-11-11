#!/usr/bin/env python3
"""Sound samples: list and export the BRR samples as WAV for the mods/ folder.

    python tools/sound.py list s1
    python tools/sound.py export s1 OUTDIR [--rate 16000]

The sound driver and the song sets reach the SPC700 as IPL upload streams
(table at $80:88CE, see tools/spcblocks.py). The driver sets the sample
directory to $5C00 (DSP register $5D = $5C, ARAM $0557). Each stream's chunks
sit back to back in the ROM file, so a sample in ARAM is a contiguous range
of the ROM too. This walks every block, builds the ARAM image the game has
after loading the driver plus that block, reads the directory entries
(4 bytes: start, loop start) and maps each sample back to its ROM address.

Export writes OUTDIR/sound/BBAAAA_<dir>_<blocks>b.wav (mono, 16-bit) and a
sound/README listing loop points. Replace a sample by putting a WAV with the
same leading address in mods/<mod>/sound/: the port resamples it to the
original length (the BRR block count is fixed by the ARAM layout), encodes it
to BRR and keeps the original loop flags and loop point.
"""
import argparse
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))

DIR = 0x5C00


def rd(rom, addr):
    return rom[((addr >> 16 & 0x7F) << 15) | (addr & 0x7FFF)]


def foff(addr):
    return ((addr >> 16 & 0x7F) << 15) | (addr & 0x7FFF)


def to_snes(off):
    return 0x800000 | (off >> 15) << 16 | 0x8000 | (off & 0x7FFF)


def stream_chunks(rom, start):
    """[(aram, length, file offset)] of one upload stream."""
    off = foff(start)
    out = []
    while True:
        n = rom[off] | rom[off + 1] << 8
        dest = rom[off + 2] | rom[off + 3] << 8
        off += 4
        if n == 0:
            break
        out.append((dest, n, off))
        off += n
    return out


def blocks(rom, table=0x8088CE, count=6):
    res = []
    for i in range(count):
        lo, hi, bk = rd(rom, table + i), rd(rom, table + count + i), rd(rom, table + 2 * count + i)
        res.append(stream_chunks(rom, bk << 16 | hi << 8 | lo))
    return res


def aram_image(rom, chunk_lists):
    ram = bytearray(0x10000)
    where = [None] * 0x10000                 # ARAM byte -> ROM file offset
    for chunks in chunk_lists:
        for dest, n, off in chunks:
            for i in range(n):
                if dest + i < 0x10000:
                    ram[dest + i] = rom[off + i]
                    where[dest + i] = off + i
    return ram, where


def brr_blocks(ram, start):
    p, n = start, 0
    while p + 9 <= 0x10000 and n < 8192:
        n += 1
        if ram[p] & 1:
            return n
        p += 9
    return 0


def brr_decode(data):
    out = []
    p1 = p2 = 0
    for b in range(0, len(data) - 8, 9):
        h = data[b]
        shift, filt = h >> 4, (h >> 2) & 3
        for i in range(16):
            byte = data[b + 1 + i // 2]
            nib = (byte >> 4) if i % 2 == 0 else (byte & 15)
            if nib >= 8:
                nib -= 16
            s = (nib << shift) >> 1 if shift <= 12 else (-2048 if nib < 0 else 0)
            if filt == 1:
                s += p1 + (-p1 >> 4)
            elif filt == 2:
                s += 2 * p1 + ((-3 * p1) >> 5) - p2 + (p2 >> 4)
            elif filt == 3:
                s += 2 * p1 + ((-13 * p1) >> 6) - p2 + ((3 * p2) >> 4)
            s = max(-32768, min(32767, s))
            p2, p1 = p1, s
            out.append(max(-32768, min(32767, s * 2)))
    return out


def samples(rom):
    bl = blocks(rom)
    found = {}
    sets = [[bl[0]]] + [[bl[0], b] for b in bl[1:]]
    for si, cl in enumerate(sets):
        ram, where = aram_image(rom, cl)
        for e in range(64):
            a = DIR + e * 4
            start = ram[a] | ram[a + 1] << 8
            loop = ram[a + 2] | ram[a + 3] << 8
            if start == 0 or where[start] is None or where[a] is None:
                continue
            n = brr_blocks(ram, start)
            if n == 0 or where[start + n * 9 - 1] is None:
                continue
            off = where[start]
            if where[start + n * 9 - 1] != off + n * 9 - 1:
                continue                     # not contiguous in ROM
            key = off
            s = found.setdefault(key, {'rom': to_snes(off), 'aram': start, 'blocks': n,
                                        'loop': loop - start if start <= loop < start + n * 9 else None,
                                        'dir': set(), 'sets': set()})
            s['dir'].add(e)
            s['sets'].add(si)
    return [found[k] for k in sorted(found)]


def cmd_list(a):
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    for s in samples(rom):
        print('$%06X  ARAM $%04X  %4d blocks (%5d samples)  loop %s  dir %s  sets %s' % (
            s['rom'], s['aram'], s['blocks'], s['blocks'] * 16,
            '-' if s['loop'] is None else '+%d' % (s['loop'] // 9 * 16),
            ','.join('%02X' % d for d in sorted(s['dir'])), ','.join(map(str, sorted(s['sets'])))))


def cmd_export(a):
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    d = os.path.join(a.out, 'sound')
    os.makedirs(d, exist_ok=True)
    lines = ['Samples (BRR in ROM) - replace with a WAV of the same leading address.',
             'Sets: 0 = driver (sound effects, voices), 1-4 = song sets, 5 = credits.', '']
    for s in samples(rom):
        off = foff(s['rom'])
        pcm = brr_decode(rom[off:off + s['blocks'] * 9])
        name = '%06X_d%02X_%db' % (s['rom'], min(s['dir']), s['blocks'])
        with open(os.path.join(d, name + '.wav'), 'wb') as f:
            data = struct.pack('<%dh' % len(pcm), *pcm)
            f.write(b'RIFF' + struct.pack('<I', 36 + len(data)) + b'WAVEfmt ' +
                    struct.pack('<IHHIIHH', 16, 1, 1, a.rate, a.rate * 2, 2, 16) +
                    b'data' + struct.pack('<I', len(data)) + data)
        lines.append('%s.wav  %d samples  loop %s  sets %s' % (
            name, len(pcm), '-' if s['loop'] is None else 'from sample %d' % (s['loop'] // 9 * 16),
            ','.join(map(str, sorted(s['sets'])))))
    open(os.path.join(d, 'README.txt'), 'w').write('\n'.join(lines) + '\n')
    print('exported %d samples to %s' % (len(lines) - 3, d))


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest='cmd', required=True)
    l = sub.add_parser('list')
    l.add_argument('ver')
    e = sub.add_parser('export')
    e.add_argument('ver')
    e.add_argument('out')
    e.add_argument('--rate', type=int, default=16000)
    a = ap.parse_args()
    {'list': cmd_list, 'export': cmd_export}[a.cmd](a)


if __name__ == '__main__':
    main()
