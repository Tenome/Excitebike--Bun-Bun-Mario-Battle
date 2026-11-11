#!/usr/bin/env python3
"""Compare snesrecomp coverage captures with bsdis.py's static code map.

    python tools/covcmp.py build/s1/code.json cap1.json [cap2.json ...] [--hints-out FILE]

Reports executed instructions that the static pass missed (or decoded with a
different M/X width), grouped into runs, plus the call targets snesrecomp saw
('discoveries'). With --hints-out, writes `code <pc> <mx>` lines for the
start of each missed run, ready to append to config/<ver>/hints.txt.
Mirrors ($00-$3F) are folded onto $80-$BF.
"""
import argparse
import json


def fold(pc):
    b = pc >> 16
    if b < 0x40:
        pc |= 0x800000
    return pc


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('code_json')
    ap.add_argument('caps', nargs='+')
    ap.add_argument('--hints-out')
    a = ap.parse_args()
    code = json.load(open(a.code_json))
    static = {int(k, 16): v for k, v in code['insn'].items()}
    executed = {}
    calls = {}
    for c in a.caps:
        j = json.load(open(c))
        for r in j.get('costs', []):
            if r.get('processor') != 'snes_cpu':
                continue
            pc = fold(int(r['target_pc24'], 16))
            executed.setdefault(pc, set()).add(r['entry_mx'])
        for d in j.get('discoveries', []):
            t = fold(int(d['target_pc24'], 16))
            calls.setdefault(t, set()).add(d['entry_mx'])
    missed = sorted(pc for pc in executed if pc not in static and pc < 0x7E0000 or (pc >= 0x800000 and pc not in static))
    missed = sorted(set(pc for pc in executed if pc not in static))
    ram = [pc for pc in missed if (pc >> 16) in (0x7E, 0x7F) or ((pc >> 16) & 0x7F) < 0x40 and (pc & 0xFFFF) < 0x8000]
    rom_missed = [pc for pc in missed if pc not in ram]
    print('executed instructions: %d   static: %d   executed-but-not-static: %d (ROM %d, RAM %d)' % (
        len(executed), len(static), len(missed), len(rom_missed), len(ram)))
    covered = sum(1 for pc in static if pc in executed)
    print('static instructions executed so far: %d / %d (%.1f%%)' % (covered, len(static), 100.0 * covered / max(1, len(static))))
    runs, cur = [], []
    for pc in rom_missed:
        if cur and pc - cur[-1] > 8:
            runs.append(cur)
            cur = []
        cur.append(pc)
    if cur:
        runs.append(cur)
    for r in runs:
        print('  missed run $%06X-$%06X  %d insns  mx=%s' % (r[0], r[-1], len(r), sorted(executed[r[0]])))
    if ram:
        print('  RAM code: %s' % ' '.join('$%06X' % p for p in ram[:20]))
    newcalls = sorted(t for t in calls if t not in code['funcs'] and ('%06X' % t) not in code['funcs'])
    print('call targets seen at run time but not static funcs: %d' % len(newcalls))
    for t in newcalls[:60]:
        print('  $%06X %s%s' % (t, sorted(calls[t]), '' if t in static else '  (not decoded)'))
    if a.hints_out:
        with open(a.hints_out, 'w') as f:
            for r in runs:
                f.write('code %06X %s\n' % (r[0], sorted(executed[r[0]])[0].lower()))
        print('wrote', a.hints_out)


if __name__ == '__main__':
    main()
