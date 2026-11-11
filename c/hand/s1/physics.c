/*
 * Stadium 1: rider position and scroll arithmetic. Hand-written; ADC/SBC go
 * through the machine model so carries chain exactly as in the original.
 */
#include "hand/hand.h"

/*
 * $80:BF74 RiderShadowY (RTS): $0979,X = ($090C,X - $0190,X - 8) * 2
 * + $01F0,X + 4, with the original's carry chaining (the second SBC and ADC
 * use the carry of the step before).
 */
void RiderShadowY_m1x1(void)
{
    lda8(rd8(ABS(0x090C + X)));
    fc = 1;
    sbc8(rd8(ABS(0x0190 + X)));
    sbc8(0x08);
    lda8(asl8((u8)A));
    fc = 0;
    adc8(rd8(ABS(0x01F0 + X)));
    adc8(0x04);
    wr8(ABS(0x0979 + X), (u8)A);
    ret();
}

/*
 * $80:BFD0 ScrollByPlayerSpeed (RTS): add the player's whole-pixel speed
 * $0180 to the BG scroll $92 (copied to $94); on a carry flip the tilemap
 * half $8C and bump the high byte $95.
 */
void ScrollByPlayerSpeed_m1x1(void)
{
    lda8(rd8(DP(0x92)));
    fc = 0;
    adc8(rd8(ABS(0x0180)));
    wr8(DP(0x92), (u8)A);
    wr8(DP(0x94), (u8)A);
    if (!fc) { ret(); return; }
    lda8(rd8(DP(0x8C)));
    eor8(0x01);
    wr8(DP(0x8C), (u8)A);
    wr8(DP(0x95), inc8(rd8(DP(0x95))));
    ret();
}

/*
 * $80:A1A6 Add32 (RTS): the 32-bit value at $10-$13 += $04-$07. The high
 * word adds the low word's carry first ($0000 + $12 + C), then $06.
 */
void Add32_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(0x04)));
    fc = 0;
    adc16(rd16(DP(0x10)));
    wr16(DP(0x10), A);
    lda16(0x0000);
    adc16(rd16(DP(0x12)));
    fc = 0;
    adc16(rd16(DP(0x06)));
    wr16(DP(0x12), A);
    sep(0x20);
    ret();
}
