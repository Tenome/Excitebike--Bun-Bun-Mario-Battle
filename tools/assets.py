#!/usr/bin/env python3
"""Graphics and palette catalogue, export for the port's mods/ folder.

    python tools/assets.py catalog s1 'E:/.../dmalog/*/dma.txt'
        -> build/s1/assets.json (+ a summary)
    python tools/assets.py export s1 OUTDIR [--sheet-width 16]
        -> OUTDIR/gfx/BBAAAA_Nbpp.png (+ .pal.txt), OUTDIR/pal/BBAAAA.txt
    python tools/assets.py check s1 OUTDIR
        -> re-encodes every exported PNG and compares with the ROM

The DMA logs come from the port (BUNBUN_DMALOG=<file>, port/src/r816_bridge.c):
one line per distinct DMA with its source, size, B-bus register, destination
and the PPU state at that moment. From them:

* ROM -> VRAM ($2118/$2119) into a BG character area is BG tiles; the BG mode
  and which BG's base the destination falls in give the bit depth
  (mode 0: 2bpp; mode 1: BG1/BG2 4bpp, BG3 2bpp; mode 3: BG1 8bpp, BG2 4bpp).
  Into the OBJ area (OBSEL): sprite tiles, 4bpp. Into a BG map area: tilemap.
* ROM -> WRAM ($2180) into the palette buffer $1C00-$1DFF: palette, colour
  number (dest - $1C00) / 2. (The buffer is sent to CGRAM each frame.)

Sheet names: BBAAAA_<bpp>bpp_<use>_n<tiles>.png; the port writes exactly
<tiles> tiles (padding at the end of the last row is ignored). Exported PNGs
are indexed, 8x8 tiles left to right, --sheet-width tiles per
row, coloured with a palette that was loaded in the same game mode (the
right sub-palette is chosen per tile by the game's tilemaps and sprite
attributes, which this does not reproduce: colours are a guide, the indices
are what counts). Edit them keeping the palette (indexed) or save as RGB:
the port then matches colours against the .pal.txt beside the PNG.
"""
import argparse
import glob
import json
import os
import sys

from PIL import Image

PAL_BUF = (0x1C00, 0x1E00)


def lorom_off(a):
    if (a & 0xFFFF) < 0x8000 or (a >> 16) in (0x7E, 0x7F):
        return None
    return ((a >> 16) & 0x7F) << 15 | (a & 0x7FFF)


def is_rom(a):
    return lorom_off(a) is not None and ((a >> 16) & 0x7F) < 0x40


def bpp_for(mode, bg):
    m = mode & 7
    if m == 0:
        return 2
    if m == 1:
        return 2 if bg == 2 else 4
    if m == 2:
        return 4
    if m == 3:
        return 8 if bg == 0 else 4
    if m == 4:
        return 8 if bg == 0 else 2
    if m == 5:
        return 4 if bg == 0 else 2
    return 8


def classify(rec):
    src, size, bbad, dmap, dest, bgmode, tadr, sc, obsel, mode = rec
    if bbad in (0x18, 0x19) and not (dmap & 0x08):
        w = dest                               # VRAM word address
        nbytes = size
        # tilemaps: 32x32 words each, count from (bgsc & 3)
        for bg in range(4):
            if bg > 1 and (bgmode & 7) not in (0, 1):
                continue
            base = (sc[bg] >> 2) << 10
            n = {0: 1, 1: 2, 2: 2, 3: 4}[sc[bg] & 3]
            if base <= w < base + n * 0x400 and sc[bg]:
                return {'kind': 'tilemap', 'bg': bg}
        obj1 = (obsel & 7) << 13
        obj2 = obj1 + (((obsel & 0x18) + 8) << 9)
        if obj1 <= w < obj1 + 0x1000 or obj2 <= w < obj2 + 0x1000:
            return {'kind': 'tiles', 'bpp': 4, 'use': 'obj'}
        best = None
        for bg in range(4):
            cb = ((tadr >> (bg * 4)) & 0xF) << 12
            if w >= cb and (best is None or cb > best[1]):
                if bg > 1 and (bgmode & 7) not in (0, 1):
                    continue
                best = (bg, cb)
        if best:
            return {'kind': 'tiles', 'bpp': bpp_for(bgmode, best[0]), 'use': 'bg%d' % (best[0] + 1)}
        return {'kind': 'vram'}
    if bbad == 0x80 and PAL_BUF[0] <= dest < PAL_BUF[1]:
        return {'kind': 'palette', 'colour': (dest - PAL_BUF[0]) // 2}
    if bbad == 0x22:
        return {'kind': 'palette', 'colour': dest}
    return None


def read_logs(pattern):
    recs = []
    for f in sorted(glob.glob(pattern)):
        for line in open(f):
            if line.startswith('#') or not line.strip():
                continue
            t = line.split()
            if len(t) < 13:
                continue
            v = [int(x, 16) for x in t]
            recs.append((v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7:11], v[11], v[12]))
    return recs


def stable_states(recs):
    """Most common PPU state per game mode (the registers at upload time may
    still hold the previous mode's values; later uploads see the settled ones)."""
    from collections import Counter
    per = {}
    for r in recs:
        per.setdefault(r[9], Counter())[(r[5], r[6], tuple(r[7]), r[8])] += 1
    return {m: c.most_common(1)[0][0] for m, c in per.items()}


def merge_blocks(assets):
    """Join contiguous blocks of the same kind into one asset."""
    out = []
    for e in sorted(assets, key=lambda e: int(e['addr'], 16)):
        if out:
            p = out[-1]
            same = (p['kind'] == e['kind'] and p.get('bpp') == e.get('bpp') and
                    p.get('use') == e.get('use') and p['kind'] in ('tiles', 'tilemap'))
            if same and int(p['addr'], 16) + p['size'] == int(e['addr'], 16):
                p['size'] += e['size']
                p['parts'] = p.get('parts', 1) + 1
                for m in e['modes']:
                    if m not in p['modes']:
                        p['modes'].append(m)
                continue
            if int(p['addr'], 16) + p['size'] > int(e['addr'], 16):
                continue                    # overlaps (re-read of a sub-range)
        out.append(dict(e))
    return out


def cmd_catalog(a):
    recs = read_logs(a.logs)
    st = stable_states(recs)
    assets = {}
    for r in recs:
        if not is_rom(r[0]):
            continue
        bgmode, tadr, sc, obsel = st.get(r[9], (r[5], r[6], tuple(r[7]), r[8]))
        c = classify((r[0], r[1], r[2], r[3], r[4], bgmode, tadr, list(sc), obsel, r[9]))
        if not c:
            continue
        key = '%06X' % r[0]
        e = assets.setdefault(key, dict(c, addr=key, size=r[1], modes=[], vram=r[4]))
        e['size'] = max(e['size'], r[1])
        if r[9] not in e['modes']:
            e['modes'].append(r[9])
    merged = merge_blocks(assets.values())
    out = 'build/%s/assets.json' % a.ver
    json.dump(merged, open(out, 'w'), indent=1)
    kinds = {}
    for e in merged:
        k = e['kind'] + ('/%dbpp/%s' % (e['bpp'], e['use']) if 'bpp' in e else '')
        kinds.setdefault(k, [0, 0])
        kinds[k][0] += 1
        kinds[k][1] += e['size']
    print('%d records, %d ROM assets (%d before merging) -> %s' % (len(recs), len(merged), len(assets), out))
    for k, (n, b) in sorted(kinds.items()):
        print('  %-20s %4d blocks %8d bytes' % (k, n, b))


def snes_rgb(w):
    return ((w & 31) << 3 | (w & 31) >> 2, (w >> 5 & 31) << 3 | (w >> 5 & 31) >> 2,
            (w >> 10 & 31) << 3 | (w >> 10 & 31) >> 2)


def decode_tile(data, bpp):
    px = [0] * 64
    for y in range(8):
        for pl in range(bpp):
            b = data[(pl >> 1) * 16 + y * 2 + (pl & 1)]
            for x in range(8):
                if b >> (7 - x) & 1:
                    px[y * 8 + x] |= 1 << pl
    return px


def encode_tile(px, bpp):
    out = bytearray(bpp * 8)
    for y in range(8):
        for x in range(8):
            v = px[y * 8 + x]
            for pl in range(bpp):
                if v >> pl & 1:
                    out[(pl >> 1) * 16 + y * 2 + (pl & 1)] |= 1 << (7 - x)
    return bytes(out)


def mode_palettes(rom, assets):
    """colour table (256 RGB) per game mode, from the palette loads seen in it."""
    per = {}
    for e in assets:
        if e['kind'] != 'palette':
            continue
        off = lorom_off(int(e['addr'], 16))
        words = [rom[off + i] | rom[off + i + 1] << 8 for i in range(0, e['size'] - 1, 2)]
        for m in e['modes']:
            t = per.setdefault(m, [None] * 256)
            for i, w in enumerate(words):
                c = e['colour'] + i
                if c < 256 and t[c] is None:
                    t[c] = snes_rgb(w)
    return per


def grey(levels):
    return [(i * 255 // (levels - 1),) * 3 for i in range(levels)]


def sheet_palette(per, e):
    bpp = e['bpp']
    levels = 1 << bpp
    first = 128 if e.get('use') == 'obj' else 0
    for m in e['modes']:
        t = per.get(m)
        if not t:
            continue
        cols = t[first:first + levels]
        if len(cols) == levels and sum(c is not None for c in cols) > levels // 2:
            g = grey(levels)
            return [c if c is not None else g[i] for i, c in enumerate(cols)]
    return grey(levels)


def cmd_export(a):
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    assets = json.load(open('build/%s/assets.json' % a.ver))
    per = mode_palettes(rom, assets)
    os.makedirs(os.path.join(a.out, 'gfx'), exist_ok=True)
    os.makedirs(os.path.join(a.out, 'pal'), exist_ok=True)
    ng = npal = 0
    for e in assets:
        off = lorom_off(int(e['addr'], 16))
        if e['kind'] == 'tiles':
            bpp = e['bpp']
            tsize = bpp * 8
            n = e['size'] // tsize
            if n == 0:
                continue
            wt = min(8 if e.get('use') == 'obj' else a.sheet_width, n)
            ht = (n + wt - 1) // wt
            img = Image.new('P', (wt * 8, ht * 8), 0)
            pal = sheet_palette(per, e)
            flat = [v for c in pal for v in c]
            img.putpalette(flat + [0] * (768 - len(flat)))
            pix = img.load()
            for t in range(n):
                px = decode_tile(rom[off + t * tsize: off + (t + 1) * tsize], bpp)
                tx, ty = t % wt, t // wt
                for i, v in enumerate(px):
                    pix[tx * 8 + i % 8, ty * 8 + i // 8] = v
            name = '%s_%dbpp_%s_n%d' % (e['addr'], bpp, e.get('use', 'bg'), n)
            img.save(os.path.join(a.out, 'gfx', name + '.png'), transparency=0)
            with open(os.path.join(a.out, 'gfx', name + '.pal.txt'), 'w') as f:
                f.write('# colours of %s.png, index order\n' % name)
                for c in pal:
                    f.write('%02X%02X%02X\n' % c)
            ng += 1
        elif e['kind'] == 'tilemap':
            os.makedirs(os.path.join(a.out, 'rom'), exist_ok=True)
            with open(os.path.join(a.out, 'rom', '%s_tilemap_bg%d.bin' % (e['addr'], e['bg'] + 1)), 'wb') as f:
                f.write(rom[off:off + e['size']])
        elif e['kind'] == 'palette':
            words = [rom[off + i] | rom[off + i + 1] << 8 for i in range(0, e['size'] - 1, 2)]
            with open(os.path.join(a.out, 'pal', '%s_c%03d.txt' % (e['addr'], e['colour'])), 'w') as f:
                f.write('# %d colours loaded at colour %d (modes %s); RRGGBB\n' % (
                    len(words), e['colour'], ','.join('%02X' % m for m in e['modes'])))
                for w in words:
                    f.write('%02X%02X%02X\n' % snes_rgb(w))
            npal += 1
    print('exported %d tile sheets and %d palettes to %s' % (ng, npal, a.out))


def cmd_check(a):
    """Encode the exported PNGs as the port does and compare with the ROM."""
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    bad = 0
    files = sorted(glob.glob(os.path.join(a.out, 'gfx', '*.png')))
    for f in files:
        name = os.path.basename(f)
        addr = int(name[:6], 16)
        bpp = int(name[name.index('bpp') - 1])
        img = Image.open(f)
        assert img.mode == 'P', name
        pix = img.load()
        wt, ht = img.width // 8, img.height // 8
        data = bytearray()
        for ty in range(ht):
            for tx in range(wt):
                px = [pix[tx * 8 + i % 8, ty * 8 + i // 8] & ((1 << bpp) - 1) for i in range(64)]
                data += encode_tile(px, bpp)
        if '_n' in name:
            n = int(name.split('_n')[1].split('.')[0].split('_')[0])
            data = data[:n * bpp * 8]
        off = lorom_off(addr)
        diff = sum(1 for x, y in zip(data, rom[off:off + len(data)]) if x != y)
        if diff:
            bad += 1
            print('MISMATCH %s: %d bytes differ' % (name, diff))
    print('%d sheets checked, %d mismatches' % (len(files), bad))


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest='cmd', required=True)
    c = sub.add_parser('catalog')
    c.add_argument('ver')
    c.add_argument('logs')
    e = sub.add_parser('export')
    e.add_argument('ver')
    e.add_argument('out')
    e.add_argument('--sheet-width', type=int, default=16)
    k = sub.add_parser('check')
    k.add_argument('ver')
    k.add_argument('out')
    a = ap.parse_args()
    {'catalog': cmd_catalog, 'export': cmd_export, 'check': cmd_check}[a.cmd](a)


if __name__ == '__main__':
    main()
