#!/usr/bin/env python3
"""Write an objdiff-format progress report (progress/<ver>/report.json) for ByteMatch.

    python tools/report.py s1 [--c] [--no-verify] [--cc65 DIR]

Assembly (default): one unit per code bank ($80, $81, $82 for Gen B) with one
item per routine of the static code map (tools/bsdis.py), and one data unit
per ROM bank with the bytes that are not code. The source is ca65 assembly
that rebuilds the dump byte for byte, so every item is 100 % once
tools/build.py reports a match; the report is not written otherwise.

C (--c): progress of the decompilation into C (s1, s4): a routine is
  100 %  hand-written readable C (c/hand/<ver>/hand.txt), checked by
         tools/difftest816.py against the original on everything its callers
         can observe;
   50 %  mechanically lifted C (tools/lift816.py) that passes the difftest
         (not listed as failing in build/<ver>/difftest.txt);
    0 %  otherwise.

The report holds routine names, addresses, sizes and percentages only: no
game bytes. The ROMs cannot be on GitHub, so reports are generated locally
and committed; .github/workflows/report.yml publishes them as the artifacts
<version>_report that ByteMatch reads from CI runs on master.
"""
import argparse
import bisect
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)


def measures(code, data, units, complete_units):
    tc = sum(i['size'] for i in code)
    mc = sum(i['size'] for i in code if i['pct'] >= 100)
    td = sum(i['size'] for i in data)
    md = sum(i['size'] for i in data if i['pct'] >= 100)
    fz = sum(i['size'] * i['pct'] for i in code) / tc if tc else 0.0
    nf = len(code)
    mf = sum(1 for i in code if i['pct'] >= 100)
    cc = tc if code and mc == tc else 0
    cd = td if data and md == td else 0
    pct = lambda a, b: round(100.0 * a / b, 4) if b else 0.0
    return {'fuzzy_match_percent': round(fz, 4), 'total_code': str(tc), 'matched_code': str(mc),
            'matched_code_percent': pct(mc, tc), 'total_data': str(td), 'matched_data': str(md),
            'matched_data_percent': pct(md, td), 'total_functions': nf, 'matched_functions': mf,
            'matched_functions_percent': pct(mf, nf), 'complete_code': str(cc),
            'complete_code_percent': pct(cc, tc), 'complete_data': str(cd),
            'complete_data_percent': pct(cd, td), 'total_units': units, 'complete_units': complete_units}


def lorom_off(a):
    return ((a >> 16) & 0x7F) << 15 | (a & 0x7FFF)


def load_names(ver):
    names = {}
    p = os.path.join(ROOT, 'config', ver, 'names.txt')
    if os.path.exists(p):
        for line in open(p, encoding='utf-8', errors='replace'):
            t = line.split('#')[0].split()
            if len(t) >= 2:
                names[int(t[0], 16)] = t[1]
    return names


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('--c', action='store_true')
    ap.add_argument('--no-verify', action='store_true')
    ap.add_argument('--cc65')
    a = ap.parse_args()
    os.chdir(ROOT)
    if not a.c and not a.no_verify:
        cmd = [sys.executable, os.path.join(HERE, 'build.py'), a.ver] + (['--cc65', a.cc65] if a.cc65 else [])
        r = subprocess.run(cmd, capture_output=True, text=True)
        out = (r.stdout + r.stderr).strip()
        print(out.splitlines()[-1] if out else '')
        if r.returncode != 0 or 'differ' in out.lower():
            sys.exit('the rebuild does not match orig/%s.bs; not writing a report' % a.ver)
    code = json.load(open('build/%s/code.json' % a.ver))
    insn = {int(k, 16): v for k, v in code['insn'].items()}
    funcs = sorted(int(k, 16) for k in code['funcs'])
    names = load_names(a.ver)
    size = {}
    code_bytes = set()
    for pc, v in insn.items():
        n = v[2]
        i = bisect.bisect_right(funcs, pc) - 1
        if i < 0:
            continue
        f = funcs[i]
        size[f] = size.get(f, 0) + n
        for k in range(n):
            code_bytes.add(lorom_off((pc & 0xFF0000) | ((pc + k) & 0xFFFF)))

    pct_of = lambda f: 100.0
    if a.c:
        hand, failing = set(), set()
        hp = os.path.join(ROOT, 'c', 'hand', a.ver, 'hand.txt')
        for line in open(hp):
            t = line.split('#')[0].split()
            if len(t) >= 5:
                hand.add(int(t[0], 16))
        byname = {v: k for k, v in names.items()}
        dp = os.path.join(ROOT, 'build', a.ver, 'difftest.txt')
        if os.path.exists(dp):
            for line in open(dp):
                m = re.match(r'FAIL\s+(\w+?)_m\dx\d\b', line)
                if m:
                    n = m.group(1)
                    f = byname.get(n) or (int(n[4:], 16) if n.startswith('sub_') else None)
                    if f is not None:
                        failing.add(f)
        pct_of = lambda f: 100.0 if f in hand else (0.0 if f in failing else 50.0)

    units, all_code, all_data, ndone = [], [], [], 0
    banks = sorted({f >> 16 for f in funcs})
    for b in banks:
        fi = [{'name': names.get(f, 'sub_%06X' % f), 'size': size.get(f, 0), 'pct': pct_of(f), 'addr': f}
              for f in funcs if f >> 16 == b and size.get(f, 0)]
        units.append(('main/bank%02X' % b, fi, [], 'c/hand/%s' % a.ver if a.c else 'build/%s/asm/bank%02X.s' % (a.ver, b)))
    if not a.c:
        romsize = os.path.getsize('orig/%s.bs' % a.ver)
        for bank in range(romsize // 0x8000):
            n = sum(1 for o in range(bank * 0x8000, (bank + 1) * 0x8000) if o not in code_bytes)
            if n:
                di = [{'name': 'Bank%02X_data' % (0x80 + bank), 'size': n, 'pct': 100.0, 'addr': (0x80 + bank) << 16 | 0x8000}]
                units.append(('data/bank%02X' % (0x80 + bank), [], di, 'build/%s/asm' % a.ver))

    def item(i):
        return {'name': i['name'], 'size': str(i['size']), 'fuzzy_match_percent': round(i['pct'], 4),
                'metadata': {'virtual_address': str(i['addr'])}}

    out_units = []
    for name, fi, di, src in units:
        done = all(i['pct'] >= 100 for i in fi + di)
        m = measures(fi, di, 1, 1 if done else 0)
        secs = []
        if fi:
            secs.append({'name': '.text', 'size': str(sum(i['size'] for i in fi)), 'fuzzy_match_percent': m['fuzzy_match_percent']})
        if di:
            secs.append({'name': '.data', 'size': str(sum(i['size'] for i in di)), 'fuzzy_match_percent': m['matched_data_percent']})
        out_units.append({'name': name, 'measures': m, 'sections': secs, 'functions': [item(i) for i in fi],
                          'metadata': {'complete': done, 'source_path': src}})
        all_code += fi
        all_data += di
        ndone += 1 if done else 0
    report = {'measures': measures(all_code, all_data, len(out_units), ndone), 'units': out_units,
              'version': 2, 'categories': []}
    outp = os.path.join(ROOT, 'progress', a.ver + ('_c' if a.c else ''), 'report.json')
    os.makedirs(os.path.dirname(outp), exist_ok=True)
    with open(outp, 'w', encoding='utf-8', newline='\n') as fh:
        json.dump(report, fh, indent=1)
    m = report['measures']
    print('report %s: %d/%d routines, %s of %s code bytes (fuzzy %.1f %%), %s data bytes, %d units' % (
        os.path.relpath(outp, ROOT), m['matched_functions'], m['total_functions'], m['matched_code'],
        m['total_code'], m['fuzzy_match_percent'], m['total_data'], len(out_units)))


if __name__ == '__main__':
    main()
