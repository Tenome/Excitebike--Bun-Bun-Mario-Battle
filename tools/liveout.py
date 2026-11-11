#!/usr/bin/env python3
"""Which registers/flags do callers read after a routine returns?

    python tools/liveout.py s1 DmaFillZero [more names or pc24 ...]

Interprocedural liveness over the static code map:
  live_in(f)  = registers f reads before writing them (walking its body;
                a call adds the callee's live_in; RTS/RTL adds live_out(f))
  live_out(f) = union over f's call sites of what the continuation reads
                (for a tail dispatch, live_out of the dispatching routine)
A walk stops on a path once a register is written (dead) or read (live).
Recursion cycles are resolved conservatively (everything live). The result
is what a hand-written replacement must reproduce
(tools/difftest816.py compare spec, c/hand/<ver>/hand.txt).
"""
import bisect
import json
import os
import sys

REGS = ('A', 'X', 'Y', 'C', 'Z', 'N', 'V')
ALL = frozenset(REGS)

UD = {
    'LDA': ('', 'ANZ'), 'LDX': ('', 'XNZ'), 'LDY': ('', 'YNZ'),
    'STA': ('A', ''), 'STX': ('X', ''), 'STY': ('Y', ''), 'STZ': ('', ''),
    'ADC': ('AC', 'ACNZV'), 'SBC': ('AC', 'ACNZV'),
    'AND': ('A', 'ANZ'), 'ORA': ('A', 'ANZ'), 'EOR': ('A', 'ANZ'),
    'CMP': ('A', 'CNZ'), 'CPX': ('X', 'CNZ'), 'CPY': ('Y', 'CNZ'),
    'BIT': ('A', 'NZV'), 'TSB': ('A', 'Z'), 'TRB': ('A', 'Z'),
    'ASL': ('', 'CNZ'), 'LSR': ('', 'CNZ'), 'ROL': ('C', 'CNZ'), 'ROR': ('C', 'CNZ'),
    'INC': ('', 'NZ'), 'DEC': ('', 'NZ'),
    'INX': ('X', 'XNZ'), 'INY': ('Y', 'YNZ'), 'DEX': ('X', 'XNZ'), 'DEY': ('Y', 'YNZ'),
    'TAX': ('A', 'XNZ'), 'TAY': ('A', 'YNZ'), 'TXA': ('X', 'ANZ'), 'TYA': ('Y', 'ANZ'),
    'TXY': ('X', 'YNZ'), 'TYX': ('Y', 'XNZ'), 'TSX': ('', 'XNZ'), 'TXS': ('X', ''),
    'TCD': ('A', 'NZ'), 'TDC': ('', 'ANZ'), 'TCS': ('A', ''), 'TSC': ('', 'ANZ'),
    'XBA': ('A', 'ANZ'), 'PHA': ('A', ''), 'PHX': ('X', ''), 'PHY': ('Y', ''),
    'PLA': ('', 'ANZ'), 'PLX': ('', 'XNZ'), 'PLY': ('', 'YNZ'), 'PLB': ('', 'NZ'), 'PLD': ('', 'NZ'),
    'PHP': ('CZNV', ''), 'PLP': ('', 'CZNV'),
    'CLC': ('', 'C'), 'SEC': ('', 'C'), 'CLV': ('', 'V'),
    'BPL': ('N', ''), 'BMI': ('N', ''), 'BVC': ('V', ''), 'BVS': ('V', ''),
    'BCC': ('C', ''), 'BCS': ('C', ''), 'BNE': ('Z', ''), 'BEQ': ('Z', ''),
    'XCE': ('C', 'C'), 'MVN': ('AXY', 'AXY'), 'MVP': ('AXY', 'AXY'),
}
INDEX_USE = {'dpx': 'X', 'absx': 'X', 'longx': 'X', 'idpx': 'X', 'iabsx': 'X',
             'dpy': 'Y', 'absy': 'Y', 'idpy': 'Y', 'ildpy': 'Y', 'isry': 'Y'}
BR = {'BPL', 'BMI', 'BVC', 'BVS', 'BCC', 'BCS', 'BNE', 'BEQ'}


class Live:
    def __init__(self, code):
        self.insn = {int(k, 16): v for k, v in code['insn'].items()}
        self.funcs = sorted(int(k, 16) for k in code['funcs'])
        self.fset = set(self.funcs)
        self.jt = {int(k, 16): [int(x, 16) for x in v['targets']] for k, v in code.get('jt', {}).items()}
        self.sites = {}
        for pc, (mn, mode, size, op) in self.insn.items():
            if mn in ('JSR', 'JSL') and mode in ('abs', 'long'):
                t = (pc & 0xFF0000) | op if mode == 'abs' else op
                self.sites.setdefault(t, []).append(pc)
            if mode in ('iabsx', 'iabs'):
                for t in self.jt.get(pc, []):
                    self.sites.setdefault(t, []).append(pc)
        self.memo_in, self.memo_out = {}, {}
        self.busy = set()

    def owner(self, pc):
        i = bisect.bisect_right(self.funcs, pc) - 1
        return self.funcs[i] if i >= 0 else pc

    def call_target(self, pc):
        mn, mode, size, op = self.insn[pc]
        if mn in ('JSR', 'JSL') and mode in ('abs', 'long'):
            return [(pc & 0xFF0000) | op if mode == 'abs' else op]
        if mn == 'JSR' and mode == 'iabsx':
            return self.jt.get(pc, [])
        return None

    def walk(self, start, ctx, at_return=True):
        """Registers read before written on some path from start (inside ctx).
        at_return: add live_out(ctx) at RTS/RTL (False when walking a callee
        for its call site: the code after the call is walked by the caller)."""
        live = set()
        work = [(start, frozenset())]
        seen = set()
        while work:
            pc, decided = work.pop()
            if (pc, decided) in seen or len(seen) > 30000:
                continue
            seen.add((pc, decided))
            if pc not in self.insn:
                live |= ALL - decided
                continue
            mn, mode, size, op = self.insn[pc]
            nxt = (pc & 0xFF0000) | ((pc + size) & 0xFFFF)
            u, d = UD.get(mn, ('', ''))
            if mn in ('REP', 'SEP'):
                u, d = '', ''.join(f for f, bit in (('C', 1), ('Z', 2), ('V', 0x40), ('N', 0x80)) if op & bit)
            if mn in ('ASL', 'LSR', 'ROL', 'ROR', 'INC', 'DEC') and mode == 'acc':
                u, d = u + 'A', d + 'A'
            u = set(u) | set(INDEX_USE.get(mode, ''))
            tg = self.call_target(pc)
            if tg is not None:
                for t in tg:
                    live |= self.live_in(t) - decided
                live |= u - decided
                decided = decided | frozenset(u)
                work.append((nxt, decided))
                continue
            if mn in ('RTS', 'RTL'):
                live |= ((self.live_out(ctx) if at_return else frozenset()) | u) - decided
                continue
            if mn in ('RTI', 'STP', 'WAI', 'BRK', 'COP'):
                continue
            if mode in ('iabs', 'iabsx', 'ilabs') or (mn in ('JMP', 'JML') and mode in ('abs', 'long')
                                                      and ((pc & 0xFF0000) | op if mode == 'abs' else op) in self.fset):
                # tail transfer: the target's live-in (it returns to our caller)
                targets = self.jt.get(pc, []) if mode in ('iabs', 'iabsx') else \
                    ([(pc & 0xFF0000) | op if mode == 'abs' else op] if mode in ('abs', 'long') else [])
                if not targets:
                    live |= ALL - decided
                for t in targets:
                    live |= (self.live_in(t) | u | (self.live_out(ctx) if at_return else frozenset())) - decided
                continue
            live |= u - decided
            decided = decided | frozenset(u) | frozenset(d)
            if decided >= ALL:
                continue
            if mn in BR:
                t = (pc & 0xFF0000) | ((nxt + (op - 256 if op & 0x80 else op)) & 0xFFFF)
                if t in self.fset and t != ctx:
                    live |= self.live_in(t) - decided
                else:
                    work.append((t, decided))
                work.append((nxt, decided))
            elif mn in ('BRA', 'BRL'):
                rel = (op - 256 if op & 0x80 else op) if mn == 'BRA' else (op - 0x10000 if op & 0x8000 else op)
                work.append(((pc & 0xFF0000) | ((nxt + rel) & 0xFFFF), decided))
            elif mn == 'JMP' and mode == 'abs':
                work.append(((pc & 0xFF0000) | op, decided))
            else:
                if nxt in self.fset and nxt != ctx:   # falls into the next routine
                    live |= self.live_in(nxt) - decided
                else:
                    work.append((nxt, decided))
        return frozenset(live)

    def live_in(self, f):
        if f in self.memo_in:
            return self.memo_in[f]
        key = ('in', f)
        if key in self.busy:
            return ALL
        self.busy.add(key)
        r = self.walk(f, f, at_return=False)
        self.busy.discard(key)
        self.memo_in[f] = r
        return r

    def live_out(self, f):
        if f in self.memo_out:
            return self.memo_out[f]
        key = ('out', f)
        if key in self.busy:
            return ALL
        self.busy.add(key)
        live = set()
        sites = self.sites.get(f, [])
        if not sites:
            live = set(ALL)          # entry point / vector: unknown callers
        for s in sites:
            mn, mode, size, op = self.insn[s]
            if mn == 'JMP':           # tail dispatch: our caller is the dispatcher's caller
                live |= self.live_out(self.owner(s))
            else:
                cont = (s & 0xFF0000) | ((s + size) & 0xFFFF)
                live |= self.walk(cont, self.owner(s))
        self.busy.discard(key)
        r = frozenset(live)
        self.memo_out[f] = r
        return r


def main():
    ver, targets = sys.argv[1], sys.argv[2:]
    code = json.load(open('build/%s/code.json' % ver))
    names = {}
    np = 'config/%s/names.txt' % ver
    if os.path.exists(np):
        for line in open(np):
            t = line.split('#')[0].split()
            if len(t) >= 2:
                names[t[1]] = int(t[0], 16)
    lv = Live(code)
    for tname in targets:
        f = names.get(tname) or int(tname.replace("sub_", ""), 16)
        out = lv.live_out(f)
        print('%-22s sites %3d  live-out: %s' % (tname, len(lv.sites.get(f, [])),
                                                ','.join(r for r in REGS if r in out) or '-'))


if __name__ == '__main__':
    main()
