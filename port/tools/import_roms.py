#!/usr/bin/env python3
"""Copy your ROM dumps into the port's data/ folder, once.

    python port/tools/import_roms.py DUMP [DUMP ...] [--to DIR ...]

Each file is identified by its SHA-256 (any name, .bs/.sfc/.smc; a 512-byte
copier header is removed) and copied as data/s1.sfc, data/s3.sfc or
data/s4.sfc beside every built executable, and rom.cfg is pointed at the
executable's own dump so the launcher opens on it (port/build/Release and
port/build/genb/Release by default, or the --to folders). After this the
executables start without being given a ROM, and the originals can move.

The dumps are for your private use only; data/ is never committed or
released.
"""
import argparse
import hashlib
import os
import shutil
import sys

KNOWN = {
    'a22163ddd3d07f01101e885297568d1863db0639e5b1b97fc84dff253dee2089': ('s1.sfc', 'Stadium 1 (Gen A)'),
    '072e053c3fa74fc954b0b1b6e11bdee7219f6323be97abf869f7da3c11ca9247': ('s3.sfc', 'Stadium 3 (Gen B)'),
    'd689aacff0aa4b6357ff23de592980b286464be774f6bcdeea7da07d39de00ea': ('s4.sfc', 'Stadium 4 (Gen B)'),
}
HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_DIRS = [os.path.join(HERE, '..', 'build', 'Release'),
                os.path.join(HERE, '..', 'build', 'genb', 'Release')]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('dumps', nargs='+')
    ap.add_argument('--to', action='append', help='executable folder (repeatable)')
    a = ap.parse_args()
    dirs = a.to or [d for d in DEFAULT_DIRS if os.path.isdir(d)]
    if not dirs:
        sys.exit('no executable folder found; build the port first or pass --to')
    found = 0
    for path in a.dumps:
        data = open(path, 'rb').read()
        if len(data) % 0x8000 == 512:
            data = data[512:]
        h = hashlib.sha256(data).hexdigest()
        if h not in KNOWN:
            print('skip %s: not a supported dump (sha256 %s)' % (path, h))
            continue
        name, what = KNOWN[h]
        for d in dirs:
            out = os.path.join(d, 'data')
            os.makedirs(out, exist_ok=True)
            with open(os.path.join(out, name), 'wb') as f:
                f.write(data)
        print('%s: %s -> data/%s in %d folder(s)' % (path, what, name, len(dirs)))
        # the launcher opens on the dump named in rom.cfg (S1 exe: s1, Gen B exe: s4)
        for d in dirs:
            want = 's4.sfc' if os.path.basename(os.path.normpath(os.path.join(d, '..'))) == 'genb' else 's1.sfc'
            if name == want:
                with open(os.path.join(d, 'rom.cfg'), 'w') as f:
                    f.write(os.path.abspath(os.path.join(d, 'data', name)) + '\n')
        found += 1
    if not found:
        sys.exit(1)


if __name__ == '__main__':
    main()
