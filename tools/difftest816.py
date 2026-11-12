#!/usr/bin/env python3
"""Differential test: lifted C (c/auto/<ver>) against the original 65816 code.

    python tools/difftest816.py s1 [--only NAME_OR_PC] [--cases 20] [--snapshots 'glob']

For every lifted function variant: start from a WRAM snapshot (frame dumps of
real play when given, else random bytes) with random registers and random
hardware read values, run the original routine in tools/cpu816.py and the
lifted C in a DLL, and compare all of WRAM, A/X/Y/S/D/DB/P and the ordered
hardware writes. Cases where the original does not return within the step
limit (it waits on hardware) are skipped on both sides.
"""
import argparse
import ctypes
import glob
import json
import os
import random
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from cpu816 import CPU, Stop  # noqa: E402

ROOT = os.path.dirname(HERE)


def build_dll(ver):
    out = os.path.join(ROOT, 'build', ver, 'lifted.dll')
    srcs = [os.path.join(ROOT, 'c', 'runtime816.c'), os.path.join(ROOT, 'c', 'difftest_harness.c'),
            os.path.join(ROOT, 'c', 'auto', ver, 'lifted.c')]
    srcs += sorted(glob.glob(os.path.join(ROOT, 'c', 'hand', ver, '*.c')))
    newest = max(os.path.getmtime(s) for s in srcs + glob.glob(os.path.join(ROOT, 'c', '*.h')) + glob.glob(os.path.join(ROOT, 'c', 'hand', ver, '*.h')))
    if not os.path.exists(out) or os.path.getmtime(out) < newest:
        cmd = ['clang', '-O1', '-w', '-shared', '-I', os.path.join(ROOT, 'c'),
               '-I', os.path.join(ROOT, 'c', 'auto', ver), '-o', out] + srcs
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode:
            sys.exit(r.stdout + r.stderr)
    return out


def returns_long(code, entry):
    """Does the routine end in RTL? (decides JSR vs JSL call frame)"""
    insn = code['insn']
    seen, work = set(), [entry]
    while work:
        pc = work.pop()
        if pc in seen or len(seen) > 4000:
            continue
        seen.add(pc)
        k = '%06X' % pc
        if k not in insn:
            continue
        mn, mode, size, op = insn[k]
        if mn == 'RTL':
            return True
        if mn == 'RTS':
            return False
        nxt = (pc & 0xFF0000) | ((pc + size) & 0xFFFF)
        if mode == 'rel':
            work.append((pc & 0xFF0000) | ((nxt + (op - 256 if op & 0x80 else op)) & 0xFFFF))
        if mn in ('BRA', 'BRL', 'JMP', 'JML'):
            if mn == 'JMP' and mode == 'abs':
                work.append((pc & 0xFF0000) | op)
            continue
        work.append(nxt)
    return False


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('ver')
    ap.add_argument('--only')
    ap.add_argument('--cases', type=int, default=12)
    ap.add_argument('--snapshots', default=None)
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--steps', type=int, default=20000)
    ap.add_argument('--verbose', action='store_true')
    ap.add_argument('--poke', action='append', default=[], help='ADDR=HEX: set WRAM byte(s) before each case (DP addresses get D added)')
    ap.add_argument('--dump-fail', action='store_true', help='save the first failing case per function')
    ap.add_argument('--hand-only', action='store_true', help='test only the hand-written variants')
    a = ap.parse_args()
    os.chdir(ROOT)
    code = json.load(open('build/%s/code.json' % a.ver))
    rom = open('orig/%s.bs' % a.ver, 'rb').read()
    dll = ctypes.CDLL(build_dll(a.ver))
    dll.h_name.restype = ctypes.c_char_p
    dll.h_wram.restype = ctypes.POINTER(ctypes.c_uint8)
    dll.h_hw_table.restype = ctypes.POINTER(ctypes.c_uint8)
    rombuf = ctypes.create_string_buffer(rom, len(rom))
    dll.h_set_rom(rombuf, len(rom))
    wram_p = dll.h_wram()
    hwt_p = dll.h_hw_table()
    snaps = []
    if not a.snapshots and os.path.isdir(os.path.join('build', 'snapshots', a.ver)):
        a.snapshots = os.path.join('build', 'snapshots', a.ver, '*.bin')
    if a.snapshots:
        files = sorted(glob.glob(a.snapshots))
        step = max(1, len(files) // 64)
        for f in files[::step][:64]:
            b = open(f, 'rb').read()
            if len(b) == 0x20000:
                snaps.append(b)
    hand_mode = {}
    hp = os.path.join('c', 'hand', a.ver, 'hand.txt')
    if os.path.exists(hp):
        for line in open(hp):
            t = line.split('#')[0].split()
            if len(t) >= 4:
                hand_mode[(int(t[0], 16), int(t[1]), int(t[2]))] = t[3]
    rnd = random.Random(a.seed)
    n = dll.h_count()
    totals = {'pass': 0, 'fail': 0, 'skip': 0}
    failing = []
    for i in range(n):
        m, x = ctypes.c_uint8(), ctypes.c_uint8()
        pc = dll.h_entry(i, ctypes.byref(m), ctypes.byref(x))
        name = dll.h_name(i).decode()
        if a.only and a.only.upper() not in (name.upper(), '%06X' % pc) and not name.upper().startswith(a.only.upper()):
            continue
        long_call = returns_long(code, pc)
        mode_cmp = hand_mode.get((pc, m.value, x.value), 'full')
        if a.only is None and a.hand_only and (pc, m.value, x.value) not in hand_mode:
            continue
        res = {'pass': 0, 'fail': 0, 'skip': 0}
        first = None
        for case in range(a.cases):
            base = bytearray(snaps[rnd.randrange(len(snaps))]) if snaps else bytearray(rnd.getrandbits(8) for _ in range(0x20000))
            hw = bytearray(rnd.getrandbits(8) for _ in range(0x4000))
            regs = [rnd.getrandbits(16), rnd.getrandbits(16), rnd.getrandbits(16), 0x1FF0, 0x0000,
                    rnd.choice([0x80, 0x80, 0x00, 0x7E]), 0]
            if rnd.random() < 0.15:
                regs[4] = rnd.choice([0x4300, 0x1B00, 0x0100])
            p = rnd.getrandbits(8) & ~0x30 | (0x20 if m.value else 0) | (0x10 if x.value else 0)
            p &= ~0x08  # decimal mode off unless the routine sets it
            if x.value:
                regs[1] &= 0xFF; regs[2] &= 0xFF
            for pk in a.poke:
                ad, _, hx = pk.partition('=')
                ad = int(ad, 16)
                if ad < 0x100:
                    ad = (ad + regs[4]) & 0xFFFF
                for k in range(len(hx) // 2):
                    base[ad + k] = int(hx[2 * k:2 * k + 2], 16)
            # original
            cpu = CPU(rom)
            cpu.wram[:] = base
            cpu.hw_values = {0x2000 + k: hw[k] for k in range(0x4000)}
            cpu.A, cpu.X, cpu.Y, cpu.S, cpu.D, cpu.DB = regs[:6]
            cpu.P = p
            try:
                cpu.call(pc, m=bool(m.value), x=bool(x.value), long=long_call, max_steps=a.steps)
            except Stop:
                res['skip'] += 1
                continue
            except (IndexError, KeyError, ValueError):
                res['skip'] += 1
                continue
            # lifted
            ctypes.memmove(wram_p, bytes(base), 0x20000)
            ctypes.memmove(hwt_p, bytes(hw), 0x4000)
            dll.h_set_regs(regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], p)
            rc = dll.h_run(i, 1 if long_call else 0, a.steps * 4)
            if rc:
                res['skip' if rc == 1 else 'fail'] += 1
                if rc == 2 and first is None:
                    uv = ctypes.c_uint32()
                    us = dll.h_unknown(ctypes.byref(uv))
                    first = 'lifted code reached an unknown target at $%06X (value $%X)' % (us, uv.value)
                continue
            outw = ctypes.string_at(wram_p, 0x20000)
            out = (ctypes.c_uint16 * 7)()
            dll.h_get_regs(out)
            la, lv = ctypes.POINTER(ctypes.c_uint32)(), ctypes.POINTER(ctypes.c_uint8)()
            nlog = dll.h_hwlog(ctypes.byref(la), ctypes.byref(lv))
            lw = [(la[k], lv[k]) for k in range(nlog)]
            ow = []
            for w in cpu.hw_writes:
                if w[0] == 'open':
                    ow.append((0x80000000 | w[1], w[2]))
                else:
                    ow.append(w)
            problems = []
            if outw != bytes(cpu.wram):
                diffs = [k for k in range(0x20000) if outw[k] != cpu.wram[k]]
                problems.append('wram %d bytes differ, first $%05X orig %02X lifted %02X' % (
                    len(diffs), diffs[0], cpu.wram[diffs[0]], outw[diffs[0]]))
            mask_ax = 0xFFFF
            want = [cpu.A, cpu.X, cpu.Y, cpu.S, cpu.D, cpu.DB, cpu.P & 0xFF]
            got = list(out)
            for nm, w_, g_ in zip(('A', 'X', 'Y', 'S', 'D', 'DB', 'P'), want, got):
                base_mode, _, extra = mode_cmp.partition('+')
                keep = set(extra.split(',')) if extra else set()
                if base_mode == 'mem' and nm in ('A', 'X', 'Y') and nm not in keep:
                    continue
                if base_mode == 'mem' and nm == 'P':
                    flagbits = sum(b for f, b in (('C', 1), ('Z', 2), ('V', 0x40), ('N', 0x80)) if f in keep)
                    if (w_ & flagbits) != (g_ & flagbits):
                        problems.append('flags orig %02X lifted %02X' % (w_, g_))
                    continue
                if (w_ & mask_ax) != (g_ & mask_ax):
                    problems.append('%s orig %04X lifted %04X' % (nm, w_, g_))
            if lw != ow:
                k = next((k for k in range(min(len(lw), len(ow))) if lw[k] != ow[k]), min(len(lw), len(ow)))
                problems.append('hw writes differ at #%d (orig %d, lifted %d writes)' % (k, len(ow), len(lw)))
                if os.environ.get('DIFFTEST_SHOW_HW'):
                    print('   orig  ', ' '.join('%06X=%02X' % w for w in ow[max(0, k - 3):k + 4]))
                    print('   lifted', ' '.join('%06X=%02X' % w for w in lw[max(0, k - 3):k + 4]))
            if problems:
                res['fail'] += 1
                if first is None:
                    first = '; '.join(problems)
                    if a.dump_fail:
                        dd = os.path.join('build', a.ver, 'fail_%s' % name)
                        os.makedirs(dd, exist_ok=True)
                        open(os.path.join(dd, 'wram_in.bin'), 'wb').write(bytes(base))
                        open(os.path.join(dd, 'hw.bin'), 'wb').write(bytes(hw))
                        open(os.path.join(dd, 'wram_orig.bin'), 'wb').write(bytes(cpu.wram))
                        open(os.path.join(dd, 'wram_lifted.bin'), 'wb').write(outw)
                        json.dump({'regs': regs, 'p': p, 'long': long_call, 'pc': pc,
                                   'm': m.value, 'x': x.value}, open(os.path.join(dd, 'case.json'), 'w'))
            else:
                res['pass'] += 1
        for k in totals:
            totals[k] += res[k]
        status = 'FAIL' if res['fail'] else ('ok' if res['pass'] else 'skip')
        if res['fail']:
            failing.append(name)
        if a.verbose or res['fail'] or a.only:
            print('%-4s %-36s pass %2d fail %2d skip %2d  %s' % (status, name, res['pass'], res['fail'], res['skip'], first or ''))
    print('TOTAL pass %d fail %d skip %d; failing functions: %d' % (totals['pass'], totals['fail'], totals['skip'], len(failing)))


if __name__ == '__main__':
    main()
