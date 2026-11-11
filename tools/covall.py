#!/usr/bin/env python3
"""Run covcmp over every capture of a version, skipping sessions that ran wild.

    python tools/covall.py s4 'E:/.../cov/s4m' ['E:/.../cov/s4' ...]
A session is skipped when it executed outside the version's code banks
(for example the deliberate jump into the BS-X BIOS at the end of a broadcast).
"""
import glob
import json
import os
import subprocess
import sys

ver, dirs = sys.argv[1], sys.argv[2:]
code = json.load(open('build/%s/code.json' % ver))
banks = {int(k, 16) >> 16 for k in code['funcs']}
good, bad = [], []
for d in dirs:
    for c in glob.glob(os.path.join(d, '*', 'cap.json')):
        try:
            j = json.load(open(c))
        except Exception:
            continue
        pcs = [int(r['target_pc24'], 16) for r in j['costs'] if r.get('processor') == 'snes_cpu']
        if any(((p >> 16) | 0x80) not in banks for p in pcs):
            bad.append(c)
        else:
            good.append(c)
print('captures: %d used, %d skipped %s' % (len(good), len(bad), [os.path.basename(os.path.dirname(b)) for b in bad]))
here = os.path.dirname(os.path.abspath(__file__))
r = subprocess.run([sys.executable, os.path.join(here, 'covcmp.py'), 'build/%s/code.json' % ver] + good + sys.argv[0:0],
                   capture_output=True, text=True)
print(r.stdout)
