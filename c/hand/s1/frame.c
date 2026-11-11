/*
 * Stadium 1 per-frame housekeeping, hand-written (called from the main loop).
 */
#include "hand/hand.h"

/*
 * $80:B4AE StepLfsr (M=1, X=1, RTS). One step of the 96-bit shift register
 * in $2C-$37 (a random source): the new top bit is bit 1 of $2C XOR bit 1 of
 * $2D, shifted in from the left across all twelve bytes. $0000 is used as
 * scratch (absolute, so it lands at DB:0000).
 */
void StepLfsr_m1x1(void)
{
    u8 a = rd8(DP(0x2C)) & 0x02;
    wr8(ABS(0x0000), a);
    a = (u8)((rd8(DP(0x2D)) & 0x02) ^ rd8(ABS(0x0000)));
    lda8(a);
    fc = a != 0;
    for (int i = 0x2C; i <= 0x37; i++)
        wr8(DP(i), ror8(rd8(DP(i))));
    ret();
}

/*
 * $80:B226 ClearOamBuffer (M=1, X=1, RTS). Unless $1958 is set, hides all
 * 128 sprites of the OAM buffer at $1600 (Y = $F0) and clears the size/X-high
 * entries $1821-$189F ($1820 itself is left alone: the loop stops at Y = 0).
 */
void ClearOamBuffer_m1x1(void)
{
    lda8(rd8(ABS(0x1958)));
    if ((u8)A != 0) {
        ret();
        return;
    }
    u8 x = 0;
    do {
        for (int quarter = 0; quarter < 4; quarter++)
            for (int k = 0; k < 8; k++)
                wr8(ABS(0x1601 + quarter * 0x80 + k * 4 + x), 0xF0);
        SET_A8(x); fc = 0;
        adc8(0x20);                                /* next 8 sprites per quarter */
        x = (u8)A;
        ldx8(x);
    } while (!(x & 0x80));
    lda8(0);
    for (u8 y = 0x7F; y != 0; y--)
        wr8(ABS(0x1820 + y), 0);
    ldy8(0);
    ret();
}

/*
 * $80:85A6 PackOamHighTable (M=1, X=1, RTS). With the direct page at $1800,
 * packs the per-sprite 2-bit entries ($1820 + n: size bit and X bit 8) four
 * to a byte into the OAM high table $1800-$181F, from the last group down.
 */
void PackOamHighTable_m1x1(void)
{
    push16(D);
    A = 0x1800; D = 0x1800;
    u8 y = 0x1C;
    do {
        u8 x = (u8)(y << 2);
        X = x;
        for (int part = 0; part < 4; part++) {
            u8 base = (u8)(x + part * 4);
            u8 v = rd8(DP((u8)(base + 0x23)));
            v = asl8(asl8(v)) | rd8(DP((u8)(base + 0x22)));
            v = asl8(asl8(v)) | rd8(DP((u8)(base + 0x21)));
            v = asl8(asl8(v)) | rd8(DP((u8)(base + 0x20)));
            lda8(v);
            wr8(ABS(0x1800 + y + part), v);
        }
        y = (u8)(y - 4);
        ldy8(y);
    } while (!(y & 0x80));
    D = pull16();
    nz16(D);
    ret();
}
