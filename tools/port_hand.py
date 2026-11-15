#!/usr/bin/env python3
"""Carry hand-written routines over to another version when the code is identical.

    python tools/port_hand.py s1 s4 [--out c/hand/s4/from_s1.c]
    python tools/port_hand.py s4 s1 [--operands]   (the other way)
    python tools/port_hand.py s1 s2 / s4 s3        (any pair along s1-s2-s3-s4)

Follows the cross-version function maps (build/xm/s1_s2.txt, s2_s3.txt,
s3_s4.txt) from each routine in c/hand/<src>/hand.txt to its counterpart and
keeps those whose instruction bytes are identical: same operands, so the same
RAM, tables and hardware. Their C is copied with the routine address in the
comment and the pushed return addresses (push16 constants inside the routine)
moved by the address difference; static helpers they use come along. Routines
that call a hand routine the target version does not have are left out.
Writes the C file and prints hand.txt / names.txt lines to add; the copies
still have to pass tools/difftest816.py on the target.
"""
import argparse
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ramname  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def load_map(f, reverse=False):
    m = {}
    for line in open(f):
        t = line.split()
        if len(t) >= 3 and t[0] != '------' and t[1] != '------' and not line.startswith('#'):
            if reverse:
                m[t[1]] = t[0]
            else:
                m[t[0]] = t[1]
    return m


def routine_bytes(ver, f):
    code = json.load(open(os.path.join(ROOT, 'build', ver, 'code.json')))
    rom = open(os.path.join(ROOT, 'orig', ver + '.bs'), 'rb').read()
    ins = {int(k, 16): v for k, v in code['insn'].items()}
    funcs = sorted(int(k, 16) for k in code['funcs'])
    i = funcs.index(f)
    end = funcs[i + 1] if i + 1 < len(funcs) else f + 0x100
    o = lambda a: ((a >> 16) & 0x7F) << 15 | (a & 0x7FFF)
    return b''.join(rom[o(p):o(p) + ins[p][2]] for p in sorted(ins) if f <= p < end), end


def routine_insns(ver, f):
    code = json.load(open(os.path.join(ROOT, 'build', ver, 'code.json')))
    ins = {int(k, 16): v for k, v in code['insn'].items()}
    funcs = sorted(int(k, 16) for k in code['funcs'])
    i = funcs.index(f)
    end = funcs[i + 1] if i + 1 < len(funcs) else f + 0x100
    return [(p, ins[p][0], ins[p][1], ins[p][3]) for p in sorted(ins) if f <= p < end]


def same_target(pa, va, pb, vb, oa, fmap):
    """A jump or call whose target moved with the code: the routine's own
    code shifted by the same amount, or the counterpart of the old target in
    the cross-version map. Hand C calls routines by name, so nothing changes."""
    full = lambda p, v: v if oa.startswith('long') else (p & 0xFF0000) | v
    a, b = full(pa, va), full(pb, vb)
    if b - a == pb - pa:
        return True
    if fmap is not None and fmap.get('%06X' % a) == '%06X' % b:
        return True
    return PCMAP.get(a) == b


PCMAP = {}       # old instruction address -> new, from aligned routines (main)


def operand_map(src_ins, dst_ins, fmap=None):
    """old -> new data addresses when the two routines differ only in
    absolute/long operands (same mnemonics and modes); None otherwise.
    Jump and call targets may differ when they moved with the code (fmap:
    the cross-version function map, old -> new, 6-digit hex)."""
    if len(src_ins) != len(dst_ins):
        return None
    m = {}
    for (pa, ma, oa, va), (pb, mb, ob, vb) in zip(src_ins, dst_ins):
        if ma != mb or oa != ob:
            return None
        if va == vb:
            continue
        if oa in ('abs', 'absx', 'absy', 'long', 'longx', 'iabsx', 'iabs') and isinstance(va, int) and isinstance(vb, int):
            if ma in ('JSR', 'JSL', 'JMP', 'JML') and oa not in ('iabsx', 'iabs'):
                if oa in ('abs', 'long') and same_target(pa, va, pb, vb, oa, fmap):
                    continue
                return None                  # calls elsewhere: not a plain data move
            if m.get(va, vb) != vb:
                return None
            m[va] = vb
        elif oa in ('rel', 'rell'):
            continue                         # branch distances move with the code
        elif oa.startswith('imm') and isinstance(va, int) and isinstance(vb, int) and va > 0xFF:
            # a 16-bit immediate: a table address that moved, or a changed
            # constant; substituted like a data address (difftest checks it)
            if m.get(-va, vb) != vb:
                return None
            m[-va] = vb                      # negative key: this routine only
        else:
            return None
    return m


def extract_functions(src):
    """name -> (text with the comment above it, body); static helpers too."""
    out = {}
    # only a comment directly above the function goes with it (not one that
    # runs on into the file's #include lines)
    pat = re.compile(r'^((?:/\*(?:(?!\*/)(?:.|\n))*\*/\n)?)((?:R816_EXPORT )?(?:static )?(?:inline )?(?:void|u8|u16|u32|int) (\w+)\([^)]*\)\n\{\n(?:.|\n)*?\n\})', re.M)
    for m in pat.finditer(src):
        out[m.group(3)] = (m.group(1) + m.group(2), m.group(2))
    # one-line definitions: void Name_m1x1(void) { helper(0x1234); }
    one = re.compile(r'^((?:/\*(?:(?!\*/)(?:.|\n))*\*/\n)?)((?:static )?(?:void|u8|u16|u32|int) (\w+)\([^)]*\)[ \t]*\{[^\n]*\}[ \t]*)$', re.M)
    for m in one.finditer(src):
        if m.group(3) not in out:
            out[m.group(3)] = (m.group(1) + m.group(2), m.group(2))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('src')
    ap.add_argument('dst')
    ap.add_argument('--out')
    ap.add_argument('--operands', action='store_true', help='also routines that differ only in data addresses')
    ap.add_argument('--skip', nargs='*', default=[], help='routines to leave out (e.g. ones difftest cannot run)')
    ap.add_argument('--emit', nargs='*', default=[], help='print these routines (and their helpers) translated to dst, write nothing: a start for hand-written versions of routines that differ')
    a = ap.parse_args()
    order = ['s1', 's2', 's3', 's4']        # build/xm maps join neighbours
    i, j = order.index(a.src), order.index(a.dst)
    if i < j:
        chain = ['%s_%s' % (order[k], order[k + 1]) for k in range(i, j)]
    else:
        chain = ['-%s_%s' % (order[k - 1], order[k]) for k in range(i, j, -1)]
    maps = [load_map(os.path.join(ROOT, 'build', 'xm', c.lstrip('-') + '.txt'), c.startswith('-'))
            for c in chain]
    fmap = {}
    for k in maps[0]:
        d = k
        for mm in maps:
            d = mm.get(d)
            if not d:
                break
        if d:
            fmap[k.upper()] = d.upper()
    # routines the map leaves unpaired: try the address its nearest mapped
    # neighbour's shift gives, and keep it when the instructions line up
    code_src = json.load(open(os.path.join(ROOT, 'build', a.src, 'code.json')))
    code_dst = json.load(open(os.path.join(ROOT, 'build', a.dst, 'code.json')))
    dst_funcs = {k.upper() for k in code_dst['funcs']}
    taken = set(fmap.values())
    mapped = sorted((int(k, 16), int(v, 16)) for k, v in fmap.items())
    for k in sorted(code_src['funcs']):
        if k.upper() in fmap:
            continue
        pc = int(k, 16)
        prev = [(s_, d_) for s_, d_ in mapped if s_ < pc and s_ >> 16 == pc >> 16]
        if not prev:
            continue
        cand = '%06X' % (pc + prev[-1][1] - prev[-1][0])
        if cand not in dst_funcs or cand in taken:
            continue
        try:
            xi, yi = routine_insns(a.src, pc), routine_insns(a.dst, int(cand, 16))
        except (ValueError, KeyError):
            continue
        if len(xi) == len(yi) and all(p[1] == q[1] and p[2] == q[2] for p, q in zip(xi, yi)):
            fmap[k.upper()] = cand
            taken.add(cand)
            maps[0][k.upper()] = cand if len(maps) == 1 else maps[0].get(k.upper())
    pcmap, retmap = PCMAP, {}
    for k in code_src['funcs']:
        d = fmap.get(k.upper())
        if not d:
            continue
        try:
            xi, yi = routine_insns(a.src, int(k, 16)), routine_insns(a.dst, int(d, 16))
        except (ValueError, KeyError):
            continue
        if len(xi) != len(yi) or any(p[1] != q[1] for p, q in zip(xi, yi)):
            continue
        for p, q in zip(xi, yi):
            pcmap[p[0]] = q[0]
            if p[1] in ('JSR', 'JSL'):           # return address: the call's last byte
                retmap[p[0] + (2 if p[1] == 'JSR' else 3)] = q[0] + (2 if q[1] == 'JSR' else 3)
        if xi:
            pcmap[xi[-1][0] + 4] = yi[-1][0] + 4      # rough end: rarely used
    gmap, gbad = {}, set()
    hand = []
    for line in open(os.path.join(ROOT, 'c', 'hand', a.src, 'hand.txt')):
        t = line.split('#')[0].split()
        if len(t) >= 5:
            hand.append(t)
    dst_hand, dst_pcs = set(), set()
    hp = os.path.join(ROOT, 'c', 'hand', a.dst, 'hand.txt')
    mark = '# from %s' % a.src
    for line in open(hp):
        t = line.split('#')[0].split()
        if len(t) >= 5 and mark not in line:
            dst_hand.add(t[4])
            dst_pcs.add(t[0].upper())
    srcs, defines, oneliners, globals_ = {}, [], {}, {}
    ram_src = ramname.load(a.src)       # named RAM operands go back to hex first
    ram_dst = ramname.load(a.dst)
    for fn in sorted(os.listdir(os.path.join(ROOT, 'c', 'hand', a.src))):
        if fn.endswith('.c') and fn not in ('roster_rom.c', 'port_rom.c'):   # the target has its own
            text = open(os.path.join(ROOT, 'c', 'hand', a.src, fn), encoding='utf-8').read()
            if ram_src:
                text = ramname.expand(text, ram_src)
            srcs.update(extract_functions(text))
            for m in re.finditer(r'^(?:static )?(?:const )?(?:unsigned|int|u8|u16|u32|uint\w+|s8|s16)\s+\*?(\w+)(?:\[[^\]]*\])*\s*(?:=\s*(?:\{(?:.|\n)*?\}|[^;{]*))?;', text, re.M):
                globals_.setdefault(m.group(1), m.group(0))
            for m in re.finditer(r'^enum \{(?:.|\n)*?\};', text, re.M):
                if m.group(0) not in defines:
                    defines.append(m.group(0))
            for line in text.splitlines():
                if line.startswith('#define') and line not in defines:
                    defines.append(line)
                m = re.match(r'static inline \w+ \*?(\w+)\([^)]*\)\s*\{.*\}\s*$', line)
                if m:
                    oneliners[m.group(1)] = line
    picked = []
    for t in hand:
        pc, name = t[0], t[4]
        d = pc
        for m in maps:
            d = m.get(d)
            if not d:
                break
        if not d or name in dst_hand or d.upper() in dst_pcs or name in a.skip:
            continue
        try:
            x, _ = routine_bytes(a.src, int(pc, 16))
            y, _ = routine_bytes(a.dst, int(d, 16))
        except (ValueError, KeyError):
            continue
        if x == y:
            picked.append((t, d, {}))
        elif a.operands:
            om = operand_map(routine_insns(a.src, int(pc, 16)), routine_insns(a.dst, int(d, 16)), fmap)
            if om is not None:
                picked.append((t, d, om))
    names_ok = dst_hand | {t[4] for t, _, _ in picked}
    # Address translation for return addresses (push16 constants) and data
    # operands, from every function pair the map aligns instruction for
    # instruction: helpers shared between routines are translated too.
    for t, d, om in picked:
        for o, n in om.items():
            if o < 0:
                continue                     # immediates stay with their routine
            k = (int(t[0], 16) >> 16, o)     # per code bank: tables move per bank
            if gmap.get(k, n) != n:
                gbad.add(k)
            gmap[k] = n
    for o in gbad:
        gmap.pop(o, None)

    def translate(text, bank, delta, lo, hi, own=None):
        """One pass over the hex constants, so nothing is translated twice:
        code addresses in CALL()/r816_call_pc() by the function map;
        push16() return addresses by the instruction map (else the routine's
        own shift); any other constant near the routine that is a return
        address (a JSR/JSL's last byte) by the call map; data operands and
        immediates by the operand map."""
        def one(m):
            pre, hexs = m.group(1), m.group(2)
            v = int(hexs, 16)
            if len(hexs) == 6 and re.search(r'(CALL\(\w+, |r816_call_pc\(|r816_call_native\()$', pre):
                r = fmap.get('%06X' % v)
                if r is None and v in pcmap:
                    r = '%06X' % pcmap[v]
                return pre + '0x' + (r or hexs.upper())
            if len(hexs) == 4 and pre.endswith('push16('):
                r = pcmap.get((bank << 16) | ((v + 1) & 0xFFFF))
                if r is not None:
                    return pre + '0x%04X' % ((r - 1) & 0xFFFF)
                if lo <= v < hi:
                    return pre + '0x%04X' % ((v + delta) & 0xFFFF)
                return m.group(0)
            if len(hexs) == 4:
                r = retmap.get((bank << 16) | v)
                if r is not None and lo - 0x400 <= v < hi + 0x400:
                    return pre + '0x%04X' % (r & 0xFFFF)
                f = fmap.get('%06X' % ((bank << 16) | v))  # a routine's start (return
                if f is not None and lo - 0x400 <= v < hi + 0x400:   # addresses built on it)
                    return pre + '0x%04X' % (int(f, 16) & 0xFFFF)
            n = own.get(-v, own.get(v)) if own else None
            if n is None:
                n = gmap.get((bank, v))
            if n is not None:
                return pre + '0x%0*X' % (6 if v > 0xFFFF else 4, n)
            if len(hexs) == 6:
                # a 24-bit address whose operand was 16-bit (LONGX(0x80DE1B)
                # for JMP ($DE1B,X)): look up its low half in its own bank
                lo16, b24 = v & 0xFFFF, v >> 16
                n = own.get(lo16) if own and b24 == bank else None
                if n is None:
                    n = gmap.get((b24, lo16))
                if n is not None and n <= 0xFFFF:
                    return pre + '0x%06X' % ((v & 0xFF0000) | n)
            return m.group(0)
        return re.sub(r'((?:CALL\(\w+, |r816_call_pc\(|r816_call_native\(|push16\()?)0x0*([0-9A-Fa-f]{4,6})\b', one, text)

    src_names = {t[4] for t in hand}
    # the target has its own roster tables (c/hand/<dst>/roster_rom.c): the
    # roster users come along and include roster.h
    has_roster = any(os.path.exists(os.path.join(ROOT, 'c', 'hand', a.dst, f)) for f in ('roster_rom.c', 'port_rom.c'))

    def static_callees(body):
        """Helpers a body needs: static functions, functions without a
        routine suffix, and internal entry points (Name_mNxN that are not a
        routine of their own in hand.txt)."""
        return [h for h in re.findall(r'\b(\w+)\(', body)
                if h in srcs and (srcs[h][1].startswith('static') or not re.search(r'_m\dx\d$', h)
                                  or re.sub(r'_m\dx\d$', '', h) not in src_names)]

    if a.emit:
        for name in a.emit:
            t = next(t for t in hand if t[4] == name)
            d = fmap.get(t[0].upper(), t[0].upper())
            fname = name + '_m%sx%s' % (t[1], t[2])
            bank, lo = int(t[0], 16) >> 16, int(t[0], 16) & 0xFFFF
            delta = int(d, 16) - int(t[0], 16)
            text = translate(srcs[fname][0], bank, delta, lo, lo + 0x400)
            print(text.replace('$%s:%s' % (t[0][:2], t[0][2:]), '$%s:%s' % (d[:2], d[2:])))
            for h in static_callees(srcs[fname][1]):
                print(translate(srcs[h][0], bank, delta, lo, lo + 0x400))
        return
    out_funcs, helpers, lines_hand, lines_names = [], [], [], []
    helper_bank = {}
    # What each picked routine needs; drop routines whose callees are not
    # available, repeating until nothing more drops (a caller of a dropped
    # routine drops too).
    info, why = {}, {}
    for t, d, om in picked:
        name = t[4]
        fname = name + '_m%sx%s' % (t[1], t[2])
        if fname not in srcs:
            why[name] = 'no C function %s found' % fname
            continue
        text, body = srcs[fname]
        closure, todo = [], static_callees(body)
        while todo:
            h = todo.pop()
            if h not in closure:
                closure.append(h)
                todo += static_callees(srcs[h][1])
        uses = body + ''.join(srcs[h][1] for h in closure)
        if re.search(r'\b(?:roster|port)_\w+\(', uses) and not has_roster:
            why[name] = 'uses the Gen B roster or port hooks'
            continue
        inner = {re.sub(r'_m\dx\d$', '', h) for h in closure}
        info[name] = (closure, set(re.findall(r'\b(\w+)_m\dx\d\b', uses)) - {name} - inner)
    while True:
        ok = dst_hand | set(info)
        bad = {n: c for n, (_, c) in info.items() if any(x not in ok for x in c)}
        if not bad:
            break
        for n, c in bad.items():
            why[n] = 'calls ' + ', '.join(sorted(x for x in c if x not in ok))
            del info[n]
    for n in sorted(why):
        print('skip %s: %s' % (n, why[n]))
    for t, d, om in picked:
        name = t[4]
        if name not in info:
            continue
        text, body = srcs[name + '_m%sx%s' % (t[1], t[2])]
        closure = info[name][0]
        delta = int(d, 16) - int(t[0], 16)
        lo, hi = int(t[0], 16) & 0xFFFF, (int(t[0], 16) & 0xFFFF) + 0x400
        text = translate(text, int(t[0], 16) >> 16, delta, lo, hi, om)
        text = text.replace('$%s:%s' % (t[0][:2], t[0][2:]), '$%s:%s' % (d[:2], d[2:]))
        for h in closure:
            if h not in helpers:
                helpers.append(h)
                helper_bank[h] = (int(t[0], 16) >> 16, delta, lo, hi)
        out_funcs.append(text)
        lines_hand.append('%s  %s %s  %-14s %-22s # from %s %s' % (d, t[1], t[2], t[3], name, a.src, t[0]))
        lines_names.append('%s %-21s # from %s %s' % (d, name, a.src, t[0]))
    helper_text = [translate(srcs[h][0], *helper_bank[h]) for h in helpers]
    every = '\n'.join(out_funcs + helper_text)
    protos = sorted({'void %s(void);' % c for c in re.findall(r'\b(\w+_m\dx\d)\b', every)})
    for h in helpers:
        m = re.match(r'((?:R816_EXPORT )?(?:static )?(?:inline )?\w+ \*?\w+\([^)]*\))', srcs[h][1])
        if m:
            protos.append(m.group(1) + ';')
    head = ['/*', ' * Routines identical (instruction for instruction) to %s\'s hand-written' % a.src.upper(),
            ' * ones, carried over by tools/port_hand.py with their return addresses', ' * moved. Checked by tools/difftest816.py on %s.' % a.dst, ' */',
            '#include "hand/hand.h"']
    for inc in ('roster.h', 'port.h'):          # the target's own hook headers
        if os.path.exists(os.path.join(ROOT, 'c', 'hand', a.dst, inc)):
            head.append('#include "%s"' % inc)
    if ram_dst:
        head.append('#include "ram.h"')
    head.append('')
    used = '\n'.join(out_funcs + [srcs[h][0] for h in helpers])
    inl = [l for n, l in oneliners.items() if re.search(r'\b%s\(' % n, used)]
    gl = [g for n, g in globals_.items() if re.search(r'\b%s\b' % n, every)]
    if ram_dst:   # the target's ram.h names these: drop enums and defines that clash
        defines = [d for d in defines
                   if not set(re.findall(r'\b([A-Z_][A-Z0-9_]*)\s*=', d) + re.findall(r'^#define\s+(\w+)', d)) & set(ram_dst)]
    body = head + defines + [''] + gl + [''] + inl + [''] + protos + [''] + helper_text + [''] + out_funcs
    outp = a.out or os.path.join(ROOT, 'c', 'hand', a.dst, 'from_%s.c' % a.src)
    text = '\n'.join(body) + '\n'
    if ram_dst:
        text = ramname.name_text(text, ram_dst)
    open(outp, 'w', encoding='utf-8', newline='\n').write(text)
    print('wrote %s: %d routines' % (outp, len(out_funcs)))
    for rel, new in ((os.path.join('c', 'hand', a.dst, 'hand.txt'), lines_hand),
                     (os.path.join('config', a.dst, 'names.txt'), lines_names)):
        path = os.path.join(ROOT, rel)
        old = [l for l in open(path, encoding='utf-8').read().splitlines() if mark not in l]
        open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(old + new) + '\n')
        print('updated %s (%d lines from %s)' % (rel, len(new), a.src))
    # fragments of never-returning code (c/hand/<ver>/fragments.txt) whose
    # exported functions came along: their start and end addresses moved
    fsrc = os.path.join(ROOT, 'c', 'hand', a.src, 'fragments.txt')
    if os.path.exists(fsrc):
        out = ['# Fragments of routines that never return, checked by difftest816.py:',
               '# START  END    m x  compare  symbol (carried from %s by tools/port_hand.py)' % a.src]
        for line in open(fsrc):
            t = line.split('#')[0].split()
            if len(t) < 6 or not re.search(r'\b%s\(' % t[5], every):
                continue
            ins_s = {int(k, 16): v for k, v in code_src['insn'].items()}
            ins_d = {int(k, 16): v for k, v in code_dst['insn'].items()}

            def where(x):
                """the fragment address in dst: the instruction map, else the
                same address when the same instruction is there (code outside
                any routine pair, such as the boot after Reset)"""
                if x in PCMAP:
                    return PCMAP[x]
                if x in ins_s and x in ins_d and ins_s[x][0] == ins_d[x][0]:
                    return x
                return None
            st, en = where(int(t[0], 16)), where(int(t[1], 16))
            if st is None or en is None:
                print('fragment %s: no address for %s / %s' % (t[5], t[0], t[1]))
                continue
            rest = line[line.index(t[2], len(t[0]) + len(t[1])):].rstrip()
            out.append('%06X  %06X  %s' % (st, en, rest))
        fdst = os.path.join(ROOT, 'c', 'hand', a.dst, 'fragments.txt')
        if os.path.exists(fdst):              # keep the target's own fragments (# <file>.c)
            out += [l.rstrip() for l in open(fdst)
                    if not l.startswith('#') and re.search(r'#\s*\w+\.c\b', l)]
        open(fdst, 'w', newline='\n').write('\n'.join(out) + '\n')
        print('wrote c/hand/%s/fragments.txt (%d fragments)' % (a.dst, len(out) - 2))


if __name__ == '__main__':
    main()
