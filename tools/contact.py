#!/usr/bin/env python3
"""Make a contact sheet from snesrecomp present_NNNNNN.ppm dumps.

    python tools/contact.py <shots_dir> out.png [--every N] [--cols 6]
Uses presents.csv for frame labels; only distinct consecutive frames are kept.
"""
import argparse
import csv
import os

from PIL import Image, ImageDraw


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('dir')
    ap.add_argument('out')
    ap.add_argument('--every', type=int, default=1)
    ap.add_argument('--cols', type=int, default=6)
    ap.add_argument('--scale', type=float, default=0.5)
    a = ap.parse_args()
    rows = list(csv.DictReader(open(os.path.join(a.dir, 'presents.csv'))))
    keep, last = [], None
    for i, r in enumerate(rows):
        if r['crc32'] == last:
            continue
        last = r['crc32']
        keep.append(r)
    keep = keep[::a.every]
    imgs = []
    for r in keep:
        p = os.path.join(a.dir, 'present_%06d.ppm' % int(r['present']))
        if os.path.exists(p):
            im = Image.open(p).convert('RGB')
            im = im.resize((int(im.width * a.scale), int(im.height * a.scale)))
            d = ImageDraw.Draw(im)
            d.rectangle([0, 0, 52, 11], fill=(0, 0, 0))
            d.text((2, 0), 'f%s' % r['frame'], fill=(255, 255, 0))
            imgs.append(im)
    if not imgs:
        print('no images')
        return
    w, h = imgs[0].size
    cols = min(a.cols, len(imgs))
    sheet = Image.new('RGB', (cols * w, ((len(imgs) + cols - 1) // cols) * h))
    for i, im in enumerate(imgs):
        sheet.paste(im, ((i % cols) * w, (i // cols) * h))
    sheet.save(a.out)
    print('wrote', a.out, len(imgs), 'frames')


if __name__ == '__main__':
    main()
