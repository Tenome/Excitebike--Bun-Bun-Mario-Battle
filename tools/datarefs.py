#!/usr/bin/env python3
"""Recover references to ROM data from the code: a first asset map.

    python tools/datarefs.py s1          # writes build/s1/datarefs.txt

Within each straight-line run of instructions (reset at every label), track
immediate values stored to direct page and to DMA source registers:

    LDA #lo / STA $02 ... LDA #bank / STA $04      -> pointer $bank:hilo
    LDA #$hilo (16-bit) / STA $02, LDX #bank / STX $04
    STA $4302 / STA $4304 (DMA A-bus source, absolute or with D=$4300)

Every complete 24-bit value that lands in ROM ($80-$9F:8000-FFFF or the
$C0+ mirror) is reported with the site that formed it. Long-addressed loads
(LDA $8B1234,X) and pointer tables with explicit bank bytes are reported too.
The output groups references by bank so data regions can be carved.
"""
import collections
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bsdis import Rom  # noqa: E402


def main():
    ver = sys.argv[1]
    code = json.load(open('build/%s/code.json' % ver))
    rom = Rom(open('orig/%s.bs' % ver, 'rb').read())
    insn = {int(k, 16): v for k, v in code['insn'].items()}
    labels = set(int(k, 16) for k in code['funcs'])
    # branch targets
    for pc, (mn, mode, size, op) in insn.items():
        if mode == 'rel':
            labels.add((pc & 0xFF0000) | ((pc + 2 + (op - 256 if op & 0x80 else op)) & 0xFFFF))
        elif mode == 'rell':
            labels.add((pc & 0xFF0000) | ((pc + 3 + (op - 0x10000 if op & 0x8000 else op)) & 0xFFFF))
    refs = collections.defaultdict(set)   # rom addr -> set(sites)
    kinds = {}
    A = X = Y = None          # known register values (int) or None
    mem = {}                  # dp/abs address -> known byte
    prev = None
    for pc in sorted(insn):
        mn, mode, size, op = insn[pc]
        if pc in labels or prev is None or (prev + insn[prev][2]) & 0xFFFF != pc & 0xFFFF:
            A = X = Y = None
            mem = {}
        prev = pc
        wide = size == 3  # for immediates
        if mode in ('immm', 'immx') and mn in ('LDA', 'LDX', 'LDY'):
            val = (op, 2 if wide else 1)
            if mn == 'LDA': A = val
            elif mn == 'LDX': X = val
            else: Y = val
            continue
        if mn in ('STA', 'STX', 'STY') and mode in ('dp', 'abs'):
            src = {'STA': A, 'STX': X, 'STY': Y}[mn]
            if src is not None:
                v, n = src
                for i in range(n):
                    mem[(op + i) & 0xFFFF] = (v >> (8 * i)) & 0xFF
            else:
                mem.pop(op, None); mem.pop(op + 1, None)
            # pointer completion check around this address
            for base in (op - 2, op - 1, op):
                b = [mem.get((base + i) & 0xFFFF) for i in range(3)]
                if None not in b:
                    addr = b[0] | b[1] << 8 | b[2] << 16
                    if rom.off(addr) is not None and (addr >> 16) not in (0x00, 0x7E, 0x7F) and (addr & 0xFFFF) >= 0x8000:
                        refs[addr].add(pc)
                        kinds[addr] = 'ptr dp$%02X' % (base & 0xFF) if base < 0x100 else 'ptr $%04X' % base
            continue
        if mode in ('long', 'longx') and mn not in ('JSL', 'JML') and rom.off(op) is not None:
            refs[op].add(pc)
            kinds.setdefault(op, 'long %s' % mode)
            continue
        if mode in ('absx', 'absy') and op >= 0x8000 and mn in ('LDA', 'LDX', 'LDY', 'ADC', 'CMP', 'AND', 'ORA', 'EOR', 'SBC'):
            addr = (pc & 0xFF0000) | op    # DB is usually the program bank ($80)
            refs[addr].add(pc)
            kinds.setdefault(addr, 'table')
            continue
        if mn in ('JSR', 'JSL', 'RTS', 'RTL', 'PLA', 'PLX', 'PLY', 'TAX', 'TAY', 'TXA', 'TYA', 'XBA',
                  'INC', 'DEC', 'INX', 'INY', 'DEX', 'DEY', 'ASL', 'LSR', 'ROL', 'ROR', 'ADC', 'SBC',
                  'AND', 'ORA', 'EOR', 'TCD', 'PLD', 'PLB', 'REP', 'SEP'):
            if mn in ('JSR', 'JSL'):
                A = X = Y = None
            elif mn in ('INX', 'DEX', 'TAX', 'PLX', 'TYX'):
                X = None
            elif mn in ('INY', 'DEY', 'TAY', 'PLY', 'TXY'):
                Y = None
            elif mn not in ('REP', 'SEP', 'PLB', 'TCD', 'PLD'):
                A = None
    by_bank = collections.defaultdict(list)
    for a in sorted(refs):
        by_bank[a >> 16 | 0x80].append(a)
    out = ['# ROM data references recovered from code (%s): %d addresses' % (ver, len(refs)), '']
    for b in sorted(by_bank):
        out.append('## bank $%02X  (%d refs)' % (b, len(by_bank[b])))
        for a in by_bank[b]:
            out.append('  $%06X  %-12s from %s' % (a, kinds[a], ' '.join('$%06X' % s for s in sorted(refs[a])[:5])))
        out.append('')
    open('build/%s/datarefs.txt' % ver, 'w').write('\n'.join(out) + '\n')
    print('build/%s/datarefs.txt: %d refs in %d banks' % (ver, len(refs), len(by_bank)))
    print(' '.join('$%02X:%d' % (b, len(v)) for b, v in sorted(by_bank.items())))


if __name__ == '__main__':
    main()
