#!/usr/bin/env python3
"""A small, exact 65816 interpreter (native mode) for differential testing.

Memory model (LoROM cartridge, no PPU):
  $7E:0000-$7F:FFFF          WRAM (128 KB); $00-$3F/$80-$BF:0000-1FFF mirror it
  $00-$3F/$80-$BF:2000-5FFF  hardware registers: writes are logged in order,
                             reads come from `hw_read(addr)` (default: 0, or a
                             per-register value table)
  ROM                         LoROM, read-only
Everything else reads 0 and ignores writes (logged as 'open').

Use:
    cpu = CPU(rom_bytes)
    cpu.wram[...] = ...
    cpu.call(0x808539, m=0, x=1, A=..., X=..., Y=...)   # runs until the routine returns
    cpu.hw_writes  -> [(addr, value), ...]
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from op65816 import OPS, operand_size  # noqa: E402

C, Z, I, D_, X_, M_, V, N = 1, 2, 4, 8, 16, 32, 64, 128


class Stop(Exception):
    pass


class CPU:
    def __init__(self, rom):
        self.rom = rom
        self.wram = bytearray(0x20000)
        self.hw_writes = []
        self.hw_values = {}
        self.A = self.X = self.Y = 0
        self.S = 0x1FF0
        self.D = 0
        self.DB = 0x80
        self.PB = 0x80
        self.PC = 0
        self.P = M_ | X_
        self.steps = 0
        self.max_steps = 2_000_000

    # ---- memory -----------------------------------------------------------
    def read8(self, a):
        a &= 0xFFFFFF
        b, o = a >> 16, a & 0xFFFF
        if b in (0x7E, 0x7F):
            return self.wram[((b - 0x7E) << 16) | o]
        lb = b & 0x7F
        if lb < 0x40:
            if o < 0x2000:
                return self.wram[o]
            if o < 0x6000:
                return self.hw_read(o)
            if o < 0x8000:
                return 0
            off = (lb << 15) | (o & 0x7FFF)
            return self.rom[off % len(self.rom)]
        if lb >= 0x40 and o >= 0x8000:
            off = ((lb & 0x3F) << 15) | (o & 0x7FFF)
            return self.rom[off % len(self.rom)]
        return 0

    def hw_read(self, o):
        v = self.hw_values.get(o, 0)
        if callable(v):
            return v() & 0xFF
        return v & 0xFF

    def write8(self, a, v):
        a &= 0xFFFFFF
        v &= 0xFF
        b, o = a >> 16, a & 0xFFFF
        if b in (0x7E, 0x7F):
            self.wram[((b - 0x7E) << 16) | o] = v
            return
        lb = b & 0x7F
        if lb < 0x40 and o < 0x2000:
            self.wram[o] = v
        elif lb < 0x40 and o < 0x6000:
            self.hw_writes.append((o, v))
        else:
            self.hw_writes.append(('open', a, v))

    def read16(self, a, wrap_bank=False):
        lo = self.read8(a)
        a2 = (a & 0xFF0000) | ((a + 1) & 0xFFFF) if wrap_bank else (a + 1)
        return lo | self.read8(a2) << 8

    def write16(self, a, v, wrap_bank=False):
        self.write8(a, v)
        a2 = (a & 0xFF0000) | ((a + 1) & 0xFFFF) if wrap_bank else (a + 1)
        self.write8(a2, v >> 8)

    # ---- helpers -----------------------------------------------------------
    @property
    def m(self):
        return bool(self.P & M_)

    @property
    def x(self):
        return bool(self.P & X_)

    def setnz(self, v, wide):
        if wide:
            v &= 0xFFFF
            self.P = (self.P & ~(N | Z)) | (N if v & 0x8000 else 0) | (Z if v == 0 else 0)
        else:
            v &= 0xFF
            self.P = (self.P & ~(N | Z)) | (N if v & 0x80 else 0) | (Z if v == 0 else 0)

    def getA(self):
        return self.A & 0xFF if self.m else self.A & 0xFFFF

    def setA(self, v):
        if self.m:
            self.A = (self.A & 0xFF00) | (v & 0xFF)
        else:
            self.A = v & 0xFFFF

    def push8(self, v):
        self.write8(self.S, v)
        self.S = (self.S - 1) & 0xFFFF

    def pull8(self):
        self.S = (self.S + 1) & 0xFFFF
        return self.read8(self.S)

    def push16(self, v):
        self.push8(v >> 8)
        self.push8(v)

    def pull16(self):
        lo = self.pull8()
        return lo | self.pull8() << 8

    def fetch(self, n):
        v = 0
        for i in range(n):
            v |= self.read8((self.PB << 16) | ((self.PC + i) & 0xFFFF)) << (8 * i)
        self.PC = (self.PC + n) & 0xFFFF
        return v

    # ---- effective address -------------------------------------------------
    def ea(self, mode, op):
        D, X, Y, DB = self.D, self.X, self.Y, self.DB
        if mode == 'dp':
            return (D + op) & 0xFFFF
        if mode == 'dpx':
            return (D + op + X) & 0xFFFF
        if mode == 'dpy':
            return (D + op + Y) & 0xFFFF
        if mode == 'idp':
            return (DB << 16) + self.read16((D + op) & 0xFFFF)
        if mode == 'idpx':
            return (DB << 16) + self.read16((D + op + X) & 0xFFFF)
        if mode == 'idpy':
            return ((DB << 16) + self.read16((D + op) & 0xFFFF) + Y) & 0xFFFFFF
        if mode == 'ildp':
            p = (D + op) & 0xFFFF
            return self.read16(p) | self.read8((p + 2) & 0xFFFF) << 16
        if mode == 'ildpy':
            p = (D + op) & 0xFFFF
            return ((self.read16(p) | self.read8((p + 2) & 0xFFFF) << 16) + Y) & 0xFFFFFF
        if mode == 'sr':
            return (self.S + op) & 0xFFFF
        if mode == 'isry':
            return ((DB << 16) + self.read16((self.S + op) & 0xFFFF) + Y) & 0xFFFFFF
        if mode == 'abs':
            return (DB << 16) | op
        if mode == 'absx':
            return ((DB << 16) + op + X) & 0xFFFFFF
        if mode == 'absy':
            return ((DB << 16) + op + Y) & 0xFFFFFF
        if mode == 'long':
            return op
        if mode == 'longx':
            return (op + X) & 0xFFFFFF
        raise ValueError(mode)

    def rd(self, mode, op, wide):
        if mode in ('immm', 'immx'):
            return op
        a = self.ea(mode, op)
        return self.read16(a) if wide else self.read8(a)

    def wr(self, mode, op, v, wide):
        a = self.ea(mode, op)
        if wide:
            self.write16(a, v)
        else:
            self.write8(a, v)

    # ---- arithmetic --------------------------------------------------------
    def adc(self, v):
        wide = not self.m
        a = self.getA()
        c = self.P & C
        if self.P & D_:
            if wide:
                r = 0
                carry = c
                for sh in (0, 4, 8, 12):
                    d = ((a >> sh) & 0xF) + ((v >> sh) & 0xF) + carry
                    carry = 1 if d > 9 else 0
                    if carry:
                        d += 6
                    r |= (d & 0xF) << sh
                res = r
                ov = (~(a ^ v) & (a ^ res) & 0x8000) != 0
                self.P = (self.P & ~(C | V)) | (C if carry else 0) | (V if ov else 0)
            else:
                lo = (a & 0xF) + (v & 0xF) + c
                if lo > 9:
                    lo += 6
                hi = (a >> 4) + (v >> 4) + (1 if lo > 0xF else 0)
                ov = (~(a ^ v) & (a ^ (hi << 4)) & 0x80) != 0
                if hi > 9:
                    hi += 6
                res = ((hi << 4) | (lo & 0xF)) & 0xFF
                self.P = (self.P & ~(C | V)) | (C if hi > 0xF else 0) | (V if ov else 0)
        else:
            mask, sign = (0xFFFF, 0x8000) if wide else (0xFF, 0x80)
            r = a + v + c
            res = r & mask
            ov = (~(a ^ v) & (a ^ res) & sign) != 0
            self.P = (self.P & ~(C | V)) | (C if r > mask else 0) | (V if ov else 0)
        self.setA(res)
        self.setnz(res, wide)

    def sbc(self, v):
        wide = not self.m
        if self.P & D_:
            a = self.getA()
            c = self.P & C
            mask = 0xFFFF if wide else 0xFF
            digits = 4 if wide else 2
            r, borrow = 0, 1 - c
            for i in range(digits):
                sh = 4 * i
                d = ((a >> sh) & 0xF) - ((v >> sh) & 0xF) - borrow
                borrow = 1 if d < 0 else 0
                if borrow:
                    d += 10
                r |= (d & 0xF) << sh
            sign = 0x8000 if wide else 0x80
            bin_r = (a - v - (1 - c)) & mask
            ov = ((a ^ v) & (a ^ bin_r) & sign) != 0
            self.P = (self.P & ~(C | V)) | (0 if borrow else C) | (V if ov else 0)
            self.setA(r)
            self.setnz(r, wide)
        else:
            self.adc(v ^ (0xFFFF if wide else 0xFF))

    def cmp(self, reg, v, wide):
        mask = 0xFFFF if wide else 0xFF
        r = (reg & mask) - (v & mask)
        self.P = (self.P & ~C) | (C if r >= 0 else 0)
        self.setnz(r, wide)

    # ---- execution ---------------------------------------------------------
    def call(self, pc24, m=True, x=True, A=None, X=None, Y=None, long=False, max_steps=None):
        """Call a routine as JSR (long=False) or JSL and run until it returns."""
        if A is not None: self.A = A & 0xFFFF
        if X is not None: self.X = X & 0xFFFF
        if Y is not None: self.Y = Y & 0xFFFF
        self.P = (self.P & ~(M_ | X_)) | (M_ if m else 0) | (X_ if x else 0)
        if self.x:
            self.X &= 0xFF; self.Y &= 0xFF
        base_s = self.S
        self.PB = pc24 >> 16
        if long:
            self.push8(0xFF)
        self.push16(0xFFFF - 1 + 1 - 1)  # return address $xx:FFFF-1 (sentinel)
        self.PC = pc24 & 0xFFFF
        limit = max_steps or self.max_steps
        while True:
            if self.S == base_s and self.PC == 0xFFFF:
                return
            self.step()
            if self.steps > limit:
                raise Stop('step limit at $%02X:%04X' % (self.PB, self.PC))

    def step(self):
        self.steps += 1
        pc0 = (self.PB << 16) | self.PC
        opc = self.fetch(1)
        mn, mode = OPS[opc]
        m, x = self.m, self.x
        size = operand_size(mode, m, x)
        op = self.fetch(size) if size else 0
        wa, wx = not m, not x
        P = self.P
        if mn in ('LDA', 'LDX', 'LDY'):
            wide = wa if mn == 'LDA' else wx
            v = self.rd(mode, op, wide)
            if mn == 'LDA': self.setA(v)
            elif mn == 'LDX': self.X = v
            else: self.Y = v
            self.setnz(v, wide)
        elif mn in ('STA', 'STX', 'STY', 'STZ'):
            wide = wa if mn in ('STA', 'STZ') else wx
            v = {'STA': self.A, 'STX': self.X, 'STY': self.Y, 'STZ': 0}[mn]
            self.wr(mode, op, v, wide)
        elif mn in ('ORA', 'AND', 'EOR', 'ADC', 'SBC', 'CMP'):
            v = self.rd(mode, op, wa)
            if mn == 'ADC': self.adc(v)
            elif mn == 'SBC': self.sbc(v)
            elif mn == 'CMP': self.cmp(self.A, v, wa)
            else:
                a = self.getA()
                r = a | v if mn == 'ORA' else (a & v if mn == 'AND' else a ^ v)
                self.setA(r); self.setnz(r, wa)
        elif mn in ('CPX', 'CPY'):
            v = self.rd(mode, op, wx)
            self.cmp(self.X if mn == 'CPX' else self.Y, v, wx)
        elif mn == 'BIT':
            v = self.rd(mode, op, wa)
            a = self.getA()
            z = (a & v) == 0
            self.P = (self.P & ~Z) | (Z if z else 0)
            if mode != 'immm':
                sign = 0x8000 if wa else 0x80
                self.P = (self.P & ~(N | V)) | (N if v & sign else 0) | (V if v & (sign >> 1) else 0)
        elif mn in ('TSB', 'TRB'):
            v = self.rd(mode, op, wa)
            a = self.getA()
            self.P = (self.P & ~Z) | (Z if (a & v) == 0 else 0)
            self.wr(mode, op, (v | a) if mn == 'TSB' else (v & ~a), wa)
        elif mn in ('ASL', 'LSR', 'ROL', 'ROR', 'INC', 'DEC'):
            wide = wa
            mask, sign = (0xFFFF, 0x8000) if wide else (0xFF, 0x80)
            v = self.getA() if mode == 'acc' else self.rd(mode, op, wide)
            c = self.P & C
            if mn == 'ASL': nc, r = v & sign, (v << 1) & mask
            elif mn == 'LSR': nc, r = v & 1, v >> 1
            elif mn == 'ROL': nc, r = v & sign, ((v << 1) | c) & mask
            elif mn == 'ROR': nc, r = v & 1, (v >> 1) | (sign if c else 0)
            elif mn == 'INC': nc, r = None, (v + 1) & mask
            else: nc, r = None, (v - 1) & mask
            if nc is not None:
                self.P = (self.P & ~C) | (C if nc else 0)
            if mode == 'acc': self.setA(r)
            else: self.wr(mode, op, r, wide)
            self.setnz(r, wide)
        elif mn in ('INX', 'INY', 'DEX', 'DEY'):
            mask = 0xFFFF if wx else 0xFF
            if mn[2] == 'X':
                self.X = (self.X + (1 if mn[0] == 'I' else -1)) & mask; self.setnz(self.X, wx)
            else:
                self.Y = (self.Y + (1 if mn[0] == 'I' else -1)) & mask; self.setnz(self.Y, wx)
        elif mn in ('TAX', 'TAY', 'TXA', 'TYA', 'TXY', 'TYX', 'TSX', 'TXS', 'TCD', 'TDC', 'TCS', 'TSC'):
            if mn == 'TAX': self.X = self.A & (0xFFFF if wx else 0xFF); self.setnz(self.X, wx)
            elif mn == 'TAY': self.Y = self.A & (0xFFFF if wx else 0xFF); self.setnz(self.Y, wx)
            elif mn == 'TXA': self.setA(self.X); self.setnz(self.X, wa)
            elif mn == 'TYA': self.setA(self.Y); self.setnz(self.Y, wa)
            elif mn == 'TXY': self.Y = self.X; self.setnz(self.Y, wx)
            elif mn == 'TYX': self.X = self.Y; self.setnz(self.X, wx)
            elif mn == 'TSX': self.X = self.S & (0xFFFF if wx else 0xFF); self.setnz(self.X, wx)
            elif mn == 'TXS': self.S = self.X
            elif mn == 'TCD': self.D = self.A; self.setnz(self.D, True)
            elif mn == 'TDC': self.A = self.D; self.setnz(self.A, True)
            elif mn == 'TCS': self.S = self.A
            elif mn == 'TSC': self.A = self.S; self.setnz(self.A, True)
        elif mn == 'XBA':
            self.A = ((self.A >> 8) | (self.A << 8)) & 0xFFFF
            self.setnz(self.A & 0xFF, False)
        elif mn in ('PHA', 'PHX', 'PHY', 'PHB', 'PHD', 'PHK', 'PHP'):
            if mn == 'PHA': (self.push16 if wa else self.push8)(self.A)
            elif mn == 'PHX': (self.push16 if wx else self.push8)(self.X)
            elif mn == 'PHY': (self.push16 if wx else self.push8)(self.Y)
            elif mn == 'PHB': self.push8(self.DB)
            elif mn == 'PHD': self.push16(self.D)
            elif mn == 'PHK': self.push8(self.PB)
            else: self.push8(self.P)
        elif mn in ('PLA', 'PLX', 'PLY', 'PLB', 'PLD', 'PLP'):
            if mn == 'PLA':
                v = self.pull16() if wa else self.pull8(); self.setA(v); self.setnz(v, wa)
            elif mn == 'PLX':
                self.X = self.pull16() if wx else self.pull8(); self.setnz(self.X, wx)
            elif mn == 'PLY':
                self.Y = self.pull16() if wx else self.pull8(); self.setnz(self.Y, wx)
            elif mn == 'PLB':
                self.DB = self.pull8(); self.setnz(self.DB, False)
            elif mn == 'PLD':
                self.D = self.pull16(); self.setnz(self.D, True)
            else:
                self.P = self.pull8()
                if self.x: self.X &= 0xFF; self.Y &= 0xFF
        elif mn == 'PEA':
            self.push16(op)
        elif mn == 'PEI':
            self.push16(self.read16((self.D + op) & 0xFFFF))
        elif mn == 'PER':
            self.push16((self.PC + (op - 0x10000 if op & 0x8000 else op)) & 0xFFFF)
        elif mn == 'REP':
            self.P &= ~op
        elif mn == 'SEP':
            self.P |= op
            if self.x: self.X &= 0xFF; self.Y &= 0xFF
        elif mn in ('CLC', 'SEC', 'CLI', 'SEI', 'CLD', 'SED', 'CLV'):
            bit = {'CLC': C, 'SEC': C, 'CLI': I, 'SEI': I, 'CLD': D_, 'SED': D_, 'CLV': V}[mn]
            if mn.startswith('SE'): self.P |= bit
            else: self.P &= ~bit
        elif mn == 'XCE':
            pass
        elif mn in ('BPL', 'BMI', 'BVC', 'BVS', 'BCC', 'BCS', 'BNE', 'BEQ', 'BRA'):
            cond = {'BPL': not P & N, 'BMI': P & N, 'BVC': not P & V, 'BVS': P & V,
                    'BCC': not P & C, 'BCS': P & C, 'BNE': not P & Z, 'BEQ': P & Z, 'BRA': True}[mn]
            if cond:
                self.PC = (self.PC + (op - 256 if op & 0x80 else op)) & 0xFFFF
        elif mn == 'BRL':
            self.PC = (self.PC + (op - 0x10000 if op & 0x8000 else op)) & 0xFFFF
        elif mn == 'JMP':
            if mode == 'abs': self.PC = op
            elif mode == 'iabs': self.PC = self.read16(op)
            elif mode == 'iabsx': self.PC = self.read16((self.PB << 16) | ((op + self.X) & 0xFFFF))
        elif mn == 'JML':
            if mode == 'long': self.PB, self.PC = op >> 16, op & 0xFFFF
            else:
                t = self.read16(op) | self.read8(op + 2) << 16
                self.PB, self.PC = t >> 16, t & 0xFFFF
        elif mn == 'JSR':
            self.push16((self.PC - 1) & 0xFFFF)
            if mode == 'abs': self.PC = op
            else: self.PC = self.read16((self.PB << 16) | ((op + self.X) & 0xFFFF))
        elif mn == 'JSL':
            self.push8(self.PB)
            self.push16((self.PC - 1) & 0xFFFF)
            self.PB, self.PC = op >> 16, op & 0xFFFF
        elif mn == 'RTS':
            self.PC = (self.pull16() + 1) & 0xFFFF
        elif mn == 'RTL':
            self.PC = (self.pull16() + 1) & 0xFFFF
            self.PB = self.pull8()
        elif mn == 'RTI':
            self.P = self.pull8()
            self.PC = self.pull16()
            self.PB = self.pull8()
        elif mn in ('MVN', 'MVP'):
            dst, src = op & 0xFF, op >> 8
            self.DB = dst
            while True:
                self.write8((dst << 16) | self.Y, self.read8((src << 16) | self.X))
                d = 1 if mn == 'MVN' else -1
                mask = 0xFFFF if wx else 0xFF
                self.X = (self.X + d) & mask
                self.Y = (self.Y + d) & mask
                self.A = (self.A - 1) & 0xFFFF
                if self.A == 0xFFFF:
                    break
        elif mn == 'NOP':
            pass
        elif mn in ('WAI', 'STP', 'BRK', 'COP', 'WDM'):
            raise Stop('%s at $%06X' % (mn, pc0))
        else:
            raise Stop('unimplemented %s at $%06X' % (mn, pc0))
