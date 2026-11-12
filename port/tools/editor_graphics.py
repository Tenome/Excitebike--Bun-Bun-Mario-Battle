#!/usr/bin/env python3
"""Capture the track's real graphics for the course editor.

    python port/tools/editor_graphics.py [--exe <BunbunS4SNESRecomp.exe>] [--out <dir> ...]

Runs the built Gen B executable headless on the course preview ("COURSE
MAP", which scrolls along the track) with test courses loaded as custom
course 4 (Stadium 4 course 1):

  ground     plain ground only (the background every code is cut from)
  calibrate  code $02 every 16 columns (column -> pixel mapping)
  catalog N  eight codes, each followed by a marker post (the preview shows
             only its first ~5700 pixels)

Each run's frames are stitched into a strip using the preview scroll
($9C/$9D, which moves the picture exactly). Where a catalog strip differs
from the ground strip is that code's graphic; it is stored with its offset
from the code's column, so the editor draws it where the game does. The
column codes $00-$3F are shared by both engines, so one set serves both.

Writes editor_graphics.js (window.BUNBUN_GFX) into each --out directory
(default: port/src/embed/genb/ and the editor/ folders beside the built
executables). Everything written is derived from the player's own ROM;
never commit or share it.

Needs Pillow and numpy. Takes a few minutes (nine real-time runs).
"""
import argparse
import base64
import csv
import glob
import io
import os
import re
import shutil
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.dirname(HERE)

GROUND = 0x80
SCRIPT = ('wait 60\npress start 3\nwait 120\npress start 3\nwait 200\npress start 3\n'
          'wait 200\npress start 3\nwait 200\npress a 3\nwait 90\npress a 3\nwait 1700\nquit\n')
FRAMES = (1150, 2420)          # the preview scrolls from about 1250 to 2370
Y0, Y1 = 60, 180               # screen rows kept: the track and what rises above it
SPACING, FIRST = 16, 24


def run(exe, work, name, course):
    d = os.path.join(work, name)
    os.makedirs(d)
    cpath = os.path.join(work, name + '.bin')
    open(cpath, 'wb').write(bytes(course))
    ini = os.path.join(work, name + '.ini')
    open(ini, 'w').write('course.4 = %s\ngame.rivals = final\ngame.flow = menu\n' % cpath)
    script = os.path.join(work, 'preview.txt')
    open(script, 'w').write(SCRIPT)
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy',
               BUNBUN_SETTINGS=ini, BUNBUN_MODS=os.path.join(work, 'nomods'),
               SNESRECOMP_SCREENSHOT_DIR=d,
               SNESRECOMP_SCREENSHOT_FROM=str(FRAMES[0]), SNESRECOMP_SCREENSHOT_TO=str(FRAMES[1]),
               SNESRECOMP_PPU_HEARTBEAT='1', SNESRECOMP_HEARTBEAT_WRAM='009C')
    os.makedirs(env['BUNBUN_MODS'], exist_ok=True)
    log = os.path.join(work, name + '.log')
    with open(log, 'w') as f:
        subprocess.run([exe, '--no-launcher', '--script', script], cwd=os.path.dirname(exe),
                       env=env, stdout=f, stderr=subprocess.STDOUT, timeout=600)
    scroll = {}
    for line in open(log, errors='replace'):
        m = re.search(r'\[ppu\] f(\d+) .*\[009C\]=(\w{4})', line)
        if m:
            scroll[int(m.group(1))] = int(m.group(2), 16)
    rows = list(csv.DictReader(open(os.path.join(d, 'presents.csv'))))
    top = max(scroll.get(int(r['frame']) - 1, 0) for r in rows)
    strip = np.zeros((Y1 - Y0, top + 256, 3), dtype=np.uint8)
    seen = np.zeros(top + 256, dtype=bool)
    for r in rows:
        s = scroll.get(int(r['frame']) - 1)        # the frame shows the previous frame's scroll
        if s is None:
            continue
        im = np.asarray(Image.open(os.path.join(d, 'present_%06d.ppm' % int(r['present']))).convert('RGB'))
        strip[:, s:s + 256] = im[Y0:Y1]
        seen[s:s + 256] = True
    shutil.rmtree(d)                                # the frames are large
    print('  %s: strip %d px' % (name, strip.shape[1]))
    return strip, seen


PRE, POST = 12, 11            # ground columns before / after each marker post


def calibration_course():
    """Posts ($02) alternately 16 and 24 columns apart."""
    b = bytearray([GROUND] * FIRST)
    for k in range(14):
        b += bytes([0x02]) + bytes([GROUND] * (15 if k % 2 == 0 else 23))
    return b + bytes([GROUND] * 40)


def marker_course(codes):
    """A lead marker, then per code: the code, PRE ground, a marker, POST ground."""
    b = bytearray([GROUND] * FIRST) + bytes([0x02]) + bytes([GROUND] * POST)
    for c in codes:
        if c == 0x09:                              # the loader stamps the gate on six columns
            b += bytes([9] * 6) + bytes([GROUND] * (PRE - 5))
        else:
            b += bytes([c]) + bytes([GROUND] * PRE)
        b += bytes([0x02]) + bytes([GROUND] * POST)
    # longer than the preview shows, or it loops the course (lap after lap)
    return b + bytes([GROUND] * max(40, 420 - len(b)))


def diff_mask(strip, ground):
    n = min(strip.shape[1], ground.shape[1])
    return np.abs(strip[:, :n].astype(int) - ground[:, :n].astype(int)).sum(axis=2) > 24


def starts_of(mask, after=300):
    hit = mask.sum(axis=0) > 2
    return [x for x in range(max(1, after), len(hit)) if hit[x] and not hit[x - 1]]


def post_template(strip, mask, x):
    """The marker post's pixels and mask around its left edge x."""
    w = 12
    return strip[:, x - 2:x + w].astype(int), mask[:, x - 2:x + w]


def find_post(strip, tmpl, lo, hi):
    pix, msk = tmpl
    best, bx = None, None
    for x in range(max(2, lo), min(hi, strip.shape[1] - pix.shape[1] + 2)):
        win = strip[:, x - 2:x - 2 + pix.shape[1]].astype(int)
        d = (np.abs(win - pix).sum(axis=2) * msk).sum()
        if best is None or d < best:
            best, bx = d, x
    return bx, best


def png_data(rgba):
    buf = io.BytesIO()
    Image.fromarray(rgba, 'RGBA').save(buf, 'PNG', optimize=True)
    return 'data:image/png;base64,' + base64.b64encode(buf.getvalue()).decode()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', default=os.path.join(PORT, 'build', 'genb', 'Release', 'BunbunS4SNESRecomp.exe'))
    ap.add_argument('--out', action='append', default=[])
    a = ap.parse_args()
    if not os.path.isfile(a.exe):
        sys.exit('editor_graphics: %s not found (build BunbunS4SNESRecomp first)' % a.exe)
    outs = a.out or [os.path.join(PORT, 'src', 'embed', 'genb')] + sorted(
        set(os.path.dirname(p) for p in glob.glob(os.path.join(PORT, 'build', '**', 'editor', 'course_editor.html'),
                                                  recursive=True)))
    work = tempfile.mkdtemp(prefix='bunbun_gfx_')
    try:
        print('editor_graphics: capturing the course preview (nine runs)')
        ground, _ = run(a.exe, work, 'ground', [GROUND] * 400)
        cal, _ = run(a.exe, work, 'calibrate', calibration_course())
        cm = diff_mask(cal, ground)
        xs = starts_of(cm)[:12]
        gaps = [xs[k + 1] - xs[k] for k in range(len(xs) - 1)]
        s16 = float(np.median(gaps[0::2])); s24 = float(np.median(gaps[1::2]))
        G = (s24 - s16) / 8.0                      # advance of a ground column
        A02 = s16 - 15 * G                         # advance of the marker post
        tmpl = post_template(cal, cm, xs[0])
        print('  ground column %.1f px, post %.1f px' % (G, A02))
        codes, advance = {}, {}
        # $00 ends a course; $30-$3F act like it in the preview and, like
        # $2E-$2F, draw nothing in a race (docs/NOTES.md)
        all_codes = list(range(0x01, 0x30))
        # eight per run: ramps advance the track further than a column, and the
        # preview shows only the first ~5700 pixels
        # $09 (the finish gate) ends the lap: the track loops back to column 0
        # after it, so it goes last, on its own
        all_codes.remove(0x09)
        groups = [all_codes[k:k + 8] for k in range(0, len(all_codes), 8)] + [[0x09]]
        for n, group in enumerate(groups):
            strip, seen = run(a.exe, work, 'catalog%d' % n, marker_course(group))
            m = diff_mask(strip, ground)
            st = starts_of(m)
            if not st:
                continue
            prev, _ = find_post(strip, tmpl, st[0] - 4, st[0] + 4)
            for c in group:
                a_k = prev + A02 + (POST + (6 if c == 0x09 else 1) - 1) * G
                a_k = prev + A02 + POST * G         # the code's own column
                lo_m = int(a_k + PRE * G - 24)
                mk, sad = find_post(strip, tmpl, lo_m, lo_m + 320)
                if (mk is None or mk + 16 > len(seen) or not seen[int(a_k - 4 * G):mk].all()
                        or sad > 40 * tmpl[1].sum()):
                    print('  catalog%d: stopped before code %02X (the preview ended)' % (n, c))
                    break                          # no clean marker: the preview ended
                advance['%02X' % c] = round(mk - a_k - PRE * G, 1)
                lo, hi = int(a_k - 4 * G), int(mk - 3)
                sub = m[:, lo:hi]
                if sub.sum() >= 4:
                    ys, xs2 = np.nonzero(sub)
                    y0, y1, x0, x1 = ys.min(), ys.max() + 1, xs2.min(), xs2.max() + 1
                    rgba = np.zeros((y1 - y0, x1 - x0, 4), dtype=np.uint8)
                    rgba[:, :, :3] = strip[y0:y1, lo + x0:lo + x1]
                    rgba[:, :, 3] = np.where(sub[y0:y1, x0:x1], 255, 0)
                    codes['%02X' % c] = {'dx': round(lo + x0 - a_k, 1), 'dy': int(y0), 'img': png_data(rgba)}
                prev = mk
        P, X0 = G, 0
        g0 = 400
        gw = int(round(32 * G))
        ground_rgba = np.dstack([ground[:, g0:g0 + gw], np.full((Y1 - Y0, gw), 255, np.uint8)])
        js = ('/* Generated by port/tools/editor_graphics.py from the player\'s own ROM.\n'
              ' * Never commit, never share. */\n'
              'window.BUNBUN_GFX = %s;\n')
        import json
        data = {'pxPerCol': round(G, 3), 'postAdvance': round(A02, 3), 'height': Y1 - Y0,
                'groundCols': 32, 'ground': png_data(ground_rgba), 'codes': codes, 'advance': advance}
        text = js % json.dumps(data)
        for o in outs:
            os.makedirs(o, exist_ok=True)
            open(os.path.join(o, 'editor_graphics.js'), 'w', newline='\n').write(text)
            print('  wrote %s' % os.path.join(o, 'editor_graphics.js'))
        print('editor_graphics: %d codes with graphics' % len(codes))
    finally:
        shutil.rmtree(work, ignore_errors=True)


if __name__ == '__main__':
    main()
