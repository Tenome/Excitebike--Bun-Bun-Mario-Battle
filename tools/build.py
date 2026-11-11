#!/usr/bin/env python3
"""Disassemble, reassemble and byte-compare one version.

    python tools/build.py s1 [--cc65 DIR]

Runs bsdis.py -> mkasm.py -> ca65/ld65 into build/<ver>/asm and compares the
result with orig/<ver>.bs. Prints the first differences if any.
"""
import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
DEFAULT_CC65 = r'E:\Claude - NES Decomp Disassembly\tools\bin\cc65\bin'


def run(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        sys.stdout.write(r.stdout + r.stderr)
        raise SystemExit('failed: %s' % ' '.join(cmd))
    return r.stdout


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('--cc65', default=os.environ.get('CC65_BIN', DEFAULT_CC65))
    a = ap.parse_args()
    os.chdir(ROOT)
    rom = os.path.join('orig', a.ver + '.bs')
    out = os.path.join('build', a.ver)
    asm = os.path.join(out, 'asm')
    run([sys.executable, os.path.join(HERE, 'bsdis.py'), rom, '--out', out])
    run([sys.executable, os.path.join(HERE, 'mkasm.py'), rom, os.path.join(out, 'code.json'), asm,
         '--names', os.path.join('config', a.ver, 'names.txt')])
    sys.path.insert(0, HERE)
    from bsdis import HWREG
    with open(os.path.join(asm, 'hwregs.inc'), 'w', newline='\n') as f:
        for k in sorted(HWREG):
            f.write('%s = $%04X\n' % (HWREG[k], k))
    ca65 = os.path.join(a.cc65, 'ca65')
    ld65 = os.path.join(a.cc65, 'ld65')
    objs = []
    for fn in sorted(os.listdir(asm)):
        if fn.startswith('bank') and fn.endswith('.s'):
            o = os.path.join(asm, fn[:-2] + '.o')
            run([ca65, '-I', asm, os.path.join(asm, fn), '-o', o])
            objs.append(o)
    built = os.path.join(out, a.ver + '_built.bs')
    run([ld65, '-C', os.path.join(asm, 'link.cfg'), '-o', built] + objs)
    x, y = open(rom, 'rb').read(), open(built, 'rb').read()
    if x == y:
        print('%s: MATCH (%d bytes)' % (a.ver, len(x)))
        return
    diffs = [i for i in range(min(len(x), len(y))) if x[i] != y[i]]
    print('%s: MISMATCH size %d vs %d, %d differing bytes' % (a.ver, len(x), len(y), len(diffs)))
    for i in diffs[:20]:
        print('  file $%05X  ($%02X:%04X)  orig %02X built %02X' % (i, 0x80 | (i >> 15), 0x8000 | (i & 0x7FFF), x[i], y[i]))
    sys.exit(1)


if __name__ == '__main__':
    main()
