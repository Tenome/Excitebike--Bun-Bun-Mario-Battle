/* runtime816.c - state and the larger helpers for lifted 65816 code. */
#include "runtime816.h"

u16 A, X, Y, S = 0x1FF0, D;
u8 DB = 0x80, PB = 0x80;
u8 fn, fz, fc, fv, fd, fi, fm = 1, fx = 1;
u8 wram[0x20000];
const u8 *rom;
u32 rom_size = 1;
int r816_unresolved;

void adc8(u8 v)
{
    u8 a = (u8)A;
    if (fd) {
        int lo = (a & 0xF) + (v & 0xF) + fc;
        if (lo > 9) lo += 6;
        int hi = (a >> 4) + (v >> 4) + (lo > 0xF);
        fv = ((~(a ^ v) & (a ^ (u8)(hi << 4))) & 0x80) != 0;
        if (hi > 9) hi += 6;
        fc = hi > 0xF;
        lda8((u8)((hi << 4) | (lo & 0xF)));
    } else {
        unsigned r = a + v + fc;
        fv = ((~(a ^ v) & (a ^ r)) & 0x80) != 0;
        fc = r > 0xFF;
        lda8((u8)r);
    }
}

void adc16(u16 v)
{
    u16 a = A;
    if (fd) {
        unsigned r = 0, carry = fc;
        for (int sh = 0; sh < 16; sh += 4) {
            unsigned d = ((a >> sh) & 0xF) + ((v >> sh) & 0xF) + carry;
            carry = d > 9;
            if (carry) d += 6;
            r |= (d & 0xF) << sh;
        }
        fv = ((~(a ^ v) & (a ^ r)) & 0x8000) != 0;
        fc = (u8)carry;
        lda16((u16)r);
    } else {
        unsigned r = a + v + fc;
        fv = ((~(a ^ v) & (a ^ r)) & 0x8000) != 0;
        fc = r > 0xFFFF;
        lda16((u16)r);
    }
}

void sbc8(u8 v)
{
    if (fd) {
        u8 a = (u8)A;
        int borrow = !fc;
        unsigned r = 0;
        for (int i = 0; i < 2; i++) {
            int d = ((a >> (4 * i)) & 0xF) - ((v >> (4 * i)) & 0xF) - borrow;
            borrow = d < 0;
            if (borrow) d += 10;
            r |= (unsigned)(d & 0xF) << (4 * i);
        }
        u8 bin = (u8)(a - v - !fc);
        fv = (((a ^ v) & (a ^ bin)) & 0x80) != 0;
        fc = !borrow;
        lda8((u8)r);
    } else {
        adc8((u8)~v);
    }
}

void sbc16(u16 v)
{
    if (fd) {
        u16 a = A;
        int borrow = !fc;
        unsigned r = 0;
        for (int i = 0; i < 4; i++) {
            int d = ((a >> (4 * i)) & 0xF) - ((v >> (4 * i)) & 0xF) - borrow;
            borrow = d < 0;
            if (borrow) d += 10;
            r |= (unsigned)(d & 0xF) << (4 * i);
        }
        u16 bin = (u16)(a - v - !fc);
        fv = (((a ^ v) & (a ^ bin)) & 0x8000) != 0;
        fc = !borrow;
        lda16((u16)r);
    } else {
        adc16((u16)~v);
    }
}

void mvn(u8 dst, u8 src)
{
    DB = dst;
    do {
        wr8(((u32)dst << 16) | Y, rd8(((u32)src << 16) | X));
        if (fx) { X = (u8)(X + 1); Y = (u8)(Y + 1); } else { X++; Y++; }
        A--;
    } while (A != 0xFFFF);
}

void mvp(u8 dst, u8 src)
{
    DB = dst;
    do {
        wr8(((u32)dst << 16) | Y, rd8(((u32)src << 16) | X));
        if (fx) { X = (u8)(X - 1); Y = (u8)(Y - 1); } else { X--; Y--; }
        A--;
    } while (A != 0xFFFF);
}

void r816_unknown_target(u32 site, u32 value)
{
    (void)site; (void)value;
    r816_unresolved = 1;
}
