/*
 * Routines identical (instruction for instruction) to S4's hand-written
 * ones, carried over by tools/port_hand.py with their return addresses
 * moved. Checked by tools/difftest816.py on s1.
 */
#include "hand/hand.h"

enum {
    P1_HELD_HI = 0x20, P1_PRESS_HI = 0x21, P2_HELD_HI = 0x22, P2_PRESS_HI = 0x23,
    P1_HELD_LO = 0x24, P2_HELD_LO = 0x25, P1_PRESS_LO = 0x26, P2_PRESS_LO = 0x27,
    P1_PREV_HI = 0x28, P2_PREV_HI = 0x29, P1_PREV_LO = 0x2A, P2_PREV_LO = 0x2B,
};
enum { BSX_STATUS = 0x2190, BSX_CTRL = 0x2191, BSX_DATA = 0x2192, BSX_ERR = 0x2193,
       BSX_RESET = 0x2197, BSX_CHANNEL = 0x218E };
enum { MODE_ACTIVE = 0x75, GAME_MODE = 0x76, MODE_STEP = 0x77, MODE_HOLD = 0x50 };
#define ARG_LEN     DP(0x00)    /* 16-bit byte count */
#define ARG_SRC     DP(0x02)    /* 16-bit source address */
#define ARG_SRCBANK DP(0x04)    /* source bank */
#define BS_CLOCK_F0   LONG(0x7FFFF0)
#define BS_CLOCK_F1   LONG(0x7FFFF1)
#define BS_CLOCK_F2   LONG(0x7FFFF2)
#define BS_CLOCK_F3   LONG(0x7FFFF3)
#define BS_CHANNEL    LONG(0x7FFFF7)    /* 16-bit: opened time channel */
#define BS_HOUR       LONG(0x7FFFF9)
#define BS_FRAME      LONG(0x7FFFFA)    /* 16-bit: frames into the minute */
#define BS_SYNCED     LONG(0x7FFFFC)    /* 16-bit: clock set from the stream */
#define BS_MINUTE     LONG(0x7FFFFF)
#define QUEUE_HEAD   LONG(0x7F0000)



/* Step one 5-bit colour field of the word `v` (mask `m`, unit `u`)
 * towards `target`. */
static void fade_field(u16 m, u16 target, u16 u)
{
    lda16(rd16(ABSX(0x1C38)));
    and16(m);
    cmp16(A, target);
    if (fz) return;
    int down = fc;
    lda16(rd16(ABSX(0x1C38)));
    if (down) { fc = 1; sbc16(u); }
    else { fc = 0; adc16(u); }
    wr16(ABSX(0x1C38), A);
}
/* $80:B0AF: AND A into rider X's input $017E. */
static void input_and_a(void)
{
    and8(rd8(ABSX(0x017E)));
    wr8(ABSX(0x017E), (u8)A);
}

/*
 * $81:A17D SinCos (RTL), X = angle: $00 = y offset, $04 = x offset for the
 * radius in $08: y = radius * cos >> 8 / 4 (the ellipse is flat), x = radius
 * * sin >> 8; signs from $81:A527 / $81:A627, magnitudes from $81:A427 /
 * $81:A327 (multiplied with the hardware multiplier, rounded).
 */
void SinCos_m1x1(void)
{
    push8((u8)X);
    lda8(rd8(DP(0x08)));
    wr8(ABS(0x4202), (u8)A);
    lda8(rd8(LONG(0x81A308 + X)));
    wr8(ABS(0x4203), (u8)A);
    lda8(rd8(LONG(0x81A408 + X)));
    wr8(DP(0x02), (u8)A);
    wr8(DP(0x03), 0);
    lda8(rd8(ABS(0x4216)));
    lda8(asl8((u8)A));
    lda8(rd8(ABS(0x4217)));
    adc8(0x00);
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x01), 0);
    lda8(rd8(LONG(0x81A208 + X)));
    wr8(ABS(0x4203), (u8)A);
    lda8(rd8(LONG(0x81A508 + X)));
    wr8(DP(0x06), (u8)A);
    wr8(DP(0x07), 0);
    lda8(rd8(ABS(0x4216)));
    lda8(asl8((u8)A));
    lda8(rd8(ABS(0x4217)));
    adc8(0x00);
    wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0);
    X = pull8();
    rep(0x20);
    lda16(rd16(DP(0x00)));
    if (!fn) {
        lda16(lsr16(A));
        lda16(lsr16(A));
    } else {
        fc = 1;
        lda16(ror16(A));
        lda16(ror16(A));
        ora16(0x8000);
    }
    ldy8(rd8(DP(0x02)));
    if (fz) {
        eor16(0xFFFF);
        lda16(inc16(A));
    }
    wr16(DP(0x00), A);
    lda16(rd16(DP(0x04)));
    ldy8(rd8(DP(0x06)));
    if (!fz) {
        eor16(0xFFFF);
        lda16(inc16(A));
    }
    wr16(DP(0x04), A);
    sep(0x20);
    retl();
}
/* $80:CB39 Reach20 (RTS): A + $14 unless A >= $E7 (then A unchanged). */
void Reach20_m1x1(void)
{
    cmp8((u8)A, 0xE7);
    if (!fc)
        adc8(0x14);
    ret();
}
/* $80:CB40 Reach2 (RTS): A + 2 unless A >= $E7. */
void Reach2_m1x1(void)
{
    cmp8((u8)A, 0xE7);
    if (!fc)
        adc8(0x02);
    ret();
}
/* $80:BF29 RiderStartLane (RTS): for rider X with $56,X clear: rider 0
 * waits for $0940 (returns while it is set); others get $3D,X = X * 8 +
 * $1C. Then count $01D6,X up. */
void RiderStartLane_m1x1(void)
{
    lda8(rd8(DPX(0x56)));
    if (!fz) { ret(); return; }
    lda8((u8)X);
    if (fz) {
        lda8(rd8(ABS(0x0940)));
        if (!fz) { ret(); return; }
    } else {
        lda8((u8)X);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        adc8(0x1C);
        wr8(DPX(0x3D), (u8)A);
    }
    wr8(ABSX(0x01D0), inc8(rd8(ABSX(0x01D0))));
    ret();
}
/* $81:9481 StepFallingSprite (RTS, object Y): every 4 frames move it up
 * and left ($0106, $010E) and step its tile $011E; at tile $9F switch it
 * off ($0116 = 0). While racing ($01CE and $84 clear) write it to the OAM
 * buffer $1610 + 2Y, attribute $2D. */
void StepFallingSprite_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (fz) {
        wr8(ABSX(0x0100), dec8(rd8(ABSX(0x0100))));
        wr8(ABSX(0x0108), dec8(rd8(ABSX(0x0108))));
        wr8(ABSX(0x0118), dec8(rd8(ABSX(0x0118))));
        lda8(rd8(ABSX(0x0118)));
        cmp8((u8)A, 0x9F);
        if (fz) { wr8(ABSX(0x0110), 0); ret(); return; }
    }
    lda8(rd8(ABS(0x01C8)));
    ora8(rd8(DP(0x84)));
    if (fz) {
        lda8((u8)X);
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0x0100))); wr8(ABSY(0x1610), (u8)A);
        lda8(rd8(ABSX(0x0108))); wr8(ABSY(0x1611), (u8)A);
        lda8(rd8(ABSX(0x0118))); wr8(ABSY(0x1612), (u8)A);
        lda8(0x2D); wr8(ABSY(0x1613), 0x2D);
    }
    ret();
}
/* $80:CFFA SortRidersByPosition (RTS): $0950-$0957 = rider numbers sorted
 * by their position $01F6 (the player uses $097C when $08C7 is set),
 * largest first (bubble sort through $00-$07). */
void SortRidersByPosition_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        wr8(ABSX(0x0950), (u8)A);
        lda8(rd8(ABSX(0x01F0)));
        wr8(DPX(0x00), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(ABS(0x08C7)));
    if (!fz) {
        lda8(rd8(ABS(0x097C)));
        wr8(DP(0x03), (u8)A);
    }
    ldx8(0x07);
    wr8(DP(0x0D), 0x07);
    do {
        ldy8(0x00);
        do {
            X = rd8(ABSY(0x0950)); nz8((u8)X);
            lda8(rd8(DPX(0x00)));
            X = rd8(ABSY(0x0951)); nz8((u8)X);
            cmp8((u8)A, rd8(DPX(0x00)));
            if (fc) {
                lda8(rd8(ABSY(0x0950)));
                wr8(ABSY(0x0951), (u8)A);
                lda8((u8)X);
                wr8(ABSY(0x0950), (u8)A);
            }
            Y = inc8((u8)Y);
            cmp8((u8)Y, rd8(DP(0x0D)));
        } while (!fz);
        u8 n = dec8(rd8(DP(0x0D)));
        wr8(DP(0x0D), n);
    } while (!fz);
    ret();
}
/* $80:C2EF SetRiderLean (RTS): rider X's lean $08B4 from the table at
 * $80:BA92 by its pose $01A6 ($0A for the player standing still on the
 * jump with $84 = 1; 6 while $020E,X is 1). */
void SetRiderLean_m1x1(void)
{
    Y = rd8(ABSX(0x01A0)); nz8((u8)Y);
    lda8(rd8(DP(0x84)));
    cmp8((u8)A, 0x01);
    int table = 1;
    if (fz) {
        lda8((u8)X);
        ora8(rd8(ABS(0x01C8)));
        ora8(rd8(ABS(0x01D0)));
        ora8(rd8(ABS(0x0170)));
        if (fz) { lda8(0x0A); table = 0; }
    }
    if (table) lda8(rd8(ABSY(0xBD65)));
    wr8(ABSX(0x08B4), (u8)A);
    lda8(rd8(ABSX(0x0208)));
    cmp8((u8)A, 0x01);
    if (fz) { lda8(0x06); wr8(ABSX(0x08B4), 0x06); }
    ret();
}
/* $81:9623 DrawSpark (RTS, object Y): sprite at OAM buffer $1610,Y from
 * $0106/$010E, tile and attribute from $81:966F/$81:9673 by the frame
 * $011E (frame 0 once $0146 reaches $F0); every $0126 frames restart the
 * frame ($012E = 1, $0126 = 4). */
void DrawSpark_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(ABSX(0x0100))); wr8(ABSY(0x1610), (u8)A);
    lda8(rd8(ABSX(0x0108))); wr8(ABSY(0x1611), (u8)A);
    lda8(rd8(ABSX(0x0140)));
    cmp8((u8)A, 0xF0);
    if (fc) lda8(0x00);
    else lda8(rd8(ABSX(0x0118)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x81959B))); wr8(ABSY(0x1612), (u8)A);
    lda8(rd8(LONGX(0x81959F))); wr8(ABSY(0x1613), (u8)A);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(0x1824), 0x02);
    u8 t = dec8(rd8(ABSX(0x0120)));
    wr8(ABSX(0x0120), t);
    if (fz) {
        lda8(0x01); wr8(ABSX(0x0128), 0x01);
        lda8(0x04); wr8(ABSX(0x0120), 0x04);
        wr8(ABSX(0x0118), 0);
    }
    ret();
}
/* $81:9362 SpawnDustPuff (RTL): find a free effect slot (6, 4, 2 by
 * $0116) and, unless the dust table at $81:9476 says $80+ for frame $1961,
 * place a puff (tile $A7) at the player ($0196 / $01B6) offset by the
 * tables $81:9476 / $81:9496 minus 3. */
void SpawnDustPuff_m1x1(void)
{
    push8(DB);
    lda8(0x81);
    push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x0110)));
        if (fz) {
            Y = rd8(ABS(0x1961)); nz8((u8)Y);
            lda8(rd8(ABSY(0x93A2)));
            if (!fn) {
                fc = 1; sbc8(0x03);
                fc = 0; adc8(rd8(ABS(0x0190)));
                wr8(ABSX(0x0100), (u8)A);
                lda8(rd8(ABSY(0x93C2)));
                fc = 1; sbc8(0x03);
                fc = 0; adc8(rd8(ABS(0x01B0)));
                wr8(ABSX(0x0108), (u8)A);
                lda8(0xA7); wr8(ABSX(0x0118), 0xA7);
                lda8(0x01); wr8(ABSX(0x0110), 0x01);
            }
            X = pull8(); nz8((u8)X);
            DB = pull8(); nz8(DB);
            retl();
            return;
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/* $81:9670 MoveObjectX (RTL): add the signed velocity nibbles of $013E,X
 * (low nibble as fraction, high nibble as whole pixels) to the 8.8
 * position $014E/$0106-$0107. Returns A = carry + whole step. */
void MoveObjectX_m1x1(void)
{
    lda8(rd8(ABSX(0x0138)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABSX(0x0138)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    cmp8((u8)A, 0x08);
    if (fc) ora8(0xF0);
    wr8(DP(0x00), (u8)A);
    ldy8(0x00);
    cmp8((u8)A, 0x00);
    if (fn) Y = dec8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    lda8(rd8(ABSX(0x0148)));
    fc = 0;
    adc8(rd8(DP(0x01)));
    wr8(ABSX(0x0148), (u8)A);
    lda8(0x00);
    lda8(rol8((u8)A));
    push8((u8)A);
    lda8(ror8((u8)A));
    lda8(rd8(ABSX(0x0100)));
    adc8(rd8(DP(0x00)));
    wr8(ABSX(0x0100), (u8)A);
    lda8(rd8(ABSX(0x0101)));
    adc8(rd8(DP(0x02)));
    wr8(ABSX(0x0101), (u8)A);
    lda8(pull8());
    fc = 0;
    adc8(rd8(DP(0x00)));
    retl();
}
/* $81:94BE DrawDustPair (RTS, object Y): scroll the pair left with the
 * player's speed ($0106 and $012E -= $0186); every 4 frames step the
 * tile $011E down, ending at $AF ($0116 = 0). While racing write both
 * sprites (OAM buffer $1610 + 2Y and $17E0 + 2Y, attribute $2D). */
void DrawDustPair_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(ABSX(0x0100)));
    fc = 1; sbc8(rd8(ABS(0x0180)));
    wr8(ABSX(0x0100), (u8)A);
    lda8(rd8(ABSX(0x0128)));
    fc = 1; sbc8(rd8(ABS(0x0180)));
    wr8(ABSX(0x0128), (u8)A);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (fz) {
        wr8(ABSX(0x0118), dec8(rd8(ABSX(0x0118))));
        lda8(rd8(ABSX(0x0118)));
        cmp8((u8)A, 0xAF);
        if (fz) { wr8(ABSX(0x0110), 0); ret(); return; }
    }
    lda8(rd8(ABS(0x01C8)));
    ora8(rd8(DP(0x84)));
    if (fz) {
        lda8((u8)X);
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0x0100))); wr8(ABSY(0x1610), (u8)A);
        lda8(rd8(ABSX(0x0108))); wr8(ABSY(0x1611), (u8)A);
        lda8(rd8(ABSX(0x0118))); wr8(ABSY(0x1612), (u8)A);
        lda8(0x2D); wr8(ABSY(0x1613), 0x2D);
        lda8(rd8(ABSX(0x0128))); wr8(ABSY(0x17E0), (u8)A);
        lda8(rd8(ABSX(0x0130))); wr8(ABSY(0x17E1), (u8)A);
        lda8(rd8(ABSX(0x0118))); wr8(ABSY(0x17E2), (u8)A);
        lda8(0x2D); wr8(ABSY(0x17E3), 0x2D);
    }
    ret();
}
/* $81:9957 FadeSkyColours (RTL): every 8 frames move the four colours at
 * $1C38-$1C3F one step towards $6C00 / $0160 / $0009 (blue, green, red)
 * and flag the palette upload ($1915). */
void FadeSkyColours_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x07);
    if (fz) {
        ldx8(0x06);
        rep(0x20);
        do {
            fade_field(0x7C00, 0x6C00, 0x0400);
            fade_field(0x03E0, 0x0160, 0x0020);
            fade_field(0x001F, 0x0009, 0x0001);
            X = dec8((u8)X);
            X = dec8((u8)X);
        } while (!fn);
        sep(0x20);
    }
    wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
    retl();
}
/* $80:B17B DrawBigSprite (RTS): a 2x2 block of 16x16 sprites at OAM
 * buffer $1600 + $04 from X $00 / Y $01; tiles from $80:B296 for X < 4
 * (attribute bit 0 cleared) or from $02 (+2 each), attribute
 * ($0B << 1) | $20 | $03; all four large. */
void DrawBigSprite_m1x1(void)
{
    push8((u8)Y);
    Y = rd8(DP(0x04)); nz8((u8)Y);
    lda8(rd8(DP(0x00)));
    wr8(ABSY(0x1600), (u8)A); wr8(ABSY(0x1608), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(0x1604), (u8)A); wr8(ABSY(0x160C), (u8)A);
    lda8(rd8(DP(0x01)));
    wr8(ABSY(0x1601), (u8)A); wr8(ABSY(0x1605), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(0x1609), (u8)A); wr8(ABSY(0x160D), (u8)A);
    cmp8((u8)X, 0x04);
    if (!fc) {
        wr8(DP(0x03), 0);
        lda8(rd8(ABSX(0xB1E6)));
    } else {
        lda8(rd8(DP(0x02)));
    }
    wr8(ABSY(0x1602), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x1606), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x160A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x160E), (u8)A);
    lda8(rd8(DP(0x0B)));
    lda8(asl8((u8)A));
    ora8(0x20);
    ora8(rd8(DP(0x03)));
    wr8(ABSY(0x1603), (u8)A); wr8(ABSY(0x1607), (u8)A);
    wr8(ABSY(0x160B), (u8)A); wr8(ABSY(0x160F), (u8)A);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(0x1820), 0x02); wr8(ABSY(0x1821), 0x02);
    wr8(ABSY(0x1822), 0x02); wr8(ABSY(0x1823), 0x02);
    Y = pull8(); nz8((u8)Y);
    ret();
}
/* $81:93E2 SpawnExhaust (RTL): while the turbo is on ($0CE0) and the
 * exhaust table $81:9518 allows frame $1961, every 4 frames put a pair of
 * puffs (tile $B2) into a free slot (6, 4, 2) behind the player at random
 * offsets from $2C-$2F. */
void SpawnExhaust_m1x1(void)
{
    push8(DB);
    lda8(0x81);
    push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    lda8(rd8(ABS(0x0CE0)));
    if (fz) goto out;
    Y = rd8(ABS(0x1961)); nz8((u8)Y);
    lda8(rd8(ABSY(0x9444)));
    if (fz) goto out;
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (!fz) goto out;
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x0110)));
        if (fz) {
            lda8(rd8(DP(0x2C))); and8(0x07); fc = 0; adc8(0x38);
            wr8(ABSX(0x0100), (u8)A);
            lda8(rd8(DP(0x2D))); and8(0x0F); fc = 0; adc8(rd8(ABS(0x01B0))); sbc8(0x02);
            wr8(ABSX(0x0108), (u8)A);
            lda8(rd8(DP(0x2E))); and8(0x07); fc = 0; adc8(0x3C);
            wr8(ABSX(0x0128), (u8)A);
            lda8(rd8(DP(0x2F))); and8(0x0F); fc = 0; adc8(rd8(ABS(0x01B0))); sbc8(0x02);
            wr8(ABSX(0x0130), (u8)A);
            lda8(0xB2); wr8(ABSX(0x0118), 0xB2);
            lda8(0x02); wr8(ABSX(0x0110), 0x02);
            goto out;
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
out:
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/* $80:D198 MirrorJumpRider (RTS): copy the player's four sprites (OAM
 * buffer $1600 + $04) to $1700 + $04 as the shadow at X $090C, Y $90 -
 * $0979 (all large), then make the originals tiles $C8-$CE with
 * attribute bit 0 set. */
void MirrorJumpRider_m1x1(void)
{
    Y = rd8(ABS(0x0004)); nz8((u8)Y);
    ldx8(0x10);
    do {
        lda8(rd8(ABSY(0x1600)));
        wr8(ABSY(0x1700), (u8)A);
        Y = inc8((u8)Y);
        X = dec8((u8)X);
    } while (!fz);
    Y = rd8(ABS(0x0004)); nz8((u8)Y);
    lda8(rd8(ABS(0x090C)));
    wr8(ABSY(0x1700), (u8)A); wr8(ABSY(0x1708), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(0x1704), (u8)A); wr8(ABSY(0x170C), (u8)A);
    lda8(0x90);
    fc = 1; sbc8(rd8(ABS(0x0979)));
    wr8(ABSY(0x1701), (u8)A); wr8(ABSY(0x1705), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(0x1709), (u8)A); wr8(ABSY(0x170D), (u8)A);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    fc = 0; adc8(0x40);
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(0x1820), 0x02); wr8(ABSY(0x1821), 0x02);
    wr8(ABSY(0x1822), 0x02); wr8(ABSY(0x1823), 0x02);
    Y = rd8(ABS(0x0004)); nz8((u8)Y);
    lda8(rd8(ABSY(0x1603)));
    ora8(0x01);
    wr8(ABSY(0x1603), (u8)A); wr8(ABSY(0x1607), (u8)A);
    wr8(ABSY(0x160B), (u8)A); wr8(ABSY(0x160F), (u8)A);
    lda8(0xC8); wr8(ABSY(0x1602), 0xC8);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x1606), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x160A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x160E), (u8)A);
    ret();
}
/* $81:9C98 VelocityDirection (RTL): the 8-way direction ($02DF,X, 0-7)
 * of the velocity $029F/$02AF,X from its signs (table $81:9DE0) and which
 * component dominates. */
void VelocityDirection_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x04), 0);
    wr16(DP(0x06), 0);
    lda16(rd16(ABSX(0x0299)));
    if (fn) {
        eor16(0xFFFF);
        lda16(inc16(A));
        wr16(DP(0x04), inc16(rd16(DP(0x04))));
    }
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(0x02A9)));
    if (fn) {
        eor16(0xFFFF);
        lda16(inc16(A));
        wr16(DP(0x06), inc16(rd16(DP(0x06))));
    }
    wr16(DP(0x02), A);
    lda16(rd16(DP(0x04)));
    fc = 0;
    adc16(rd16(DP(0x06)));
    lda16(lsr16(A));
    if (fc) {
        lda16(rd16(DP(0x00)));
        push16(A);
        lda16(rd16(DP(0x02)));
        wr16(DP(0x00), A);
        lda16(pull16());
        wr16(DP(0x02), A);
    }
    lda16(rd16(DP(0x00)));
    cmp16(A, 0x0020);
    if (!fz) {
        for (int i = 0; i < 6; i++) lda16(lsr16(A));
    } else {
        lda16(rd16(DP(0x02)));
        cmp16(A, 0x0020);
        int one = fz;
        if (!one) {
            for (int i = 0; i < 6; i++) lda16(lsr16(A));
            one = fc;
        }
        lda16(one ? 0x0001 : 0x0002);
    }
    wr16(DP(0x08), A);
    lda16(rd16(DP(0x04)));
    lda16(asl16(A));
    ora16(rd16(DP(0x06)));
    Y = (u8)A; nz8((u8)Y);
    lda16(rd16(ABSY(0x9D07)));
    fc = 0;
    adc16(rd16(DP(0x08)));
    and16(0x0007);
    wr16(ABSX(0x02D9), A);
    sep(0x20);
    retl();
}
/* $81:9D0B DrawBigBall (RTS): a 2x2 ball sprite (tiles $DD/$DE/$ED/$EE,
 * attribute $2C) at OAM buffer $17F0 for object $03A3, from its 16-bit
 * position $027F - 10 / $028F + 6; near the left edge (X high byte $FF)
 * or right edge it is clipped by hiding columns and setting the size/X9
 * bits $189C-$189F. */
void DrawBigBall_m1x1(void)
{
    rep(0x20);
    X = rd8(ABS(0x039D)); nz8((u8)X);
    lda16(rd16(ABSX(0x0279)));
    fc = 0; adc16(0xFFF6);
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(0x0289)));
    fc = 0; adc16(0x0006);
    wr16(DP(0x02), A);
    sep(0x20);
    lda8(0xDD); wr8(ABS(0x17F2), 0xDD);
    lda8(inc8((u8)A)); wr8(ABS(0x17F6), (u8)A);
    lda8(0xED); wr8(ABS(0x17FA), 0xED);
    lda8(inc8((u8)A)); wr8(ABS(0x17FE), (u8)A);
    lda8(0x2C);
    wr8(ABS(0x17F3), 0x2C); wr8(ABS(0x17F7), 0x2C);
    wr8(ABS(0x17FB), 0x2C); wr8(ABS(0x17FF), 0x2C);
    lda8(rd8(DP(0x00)));
    wr8(ABS(0x17F0), (u8)A); wr8(ABS(0x17F8), (u8)A);
    fc = 0; adc8(0x08);
    wr8(ABS(0x17F4), (u8)A); wr8(ABS(0x17FC), (u8)A);
    lda8(rd8(DP(0x02)));
    wr8(ABS(0x17F1), (u8)A); wr8(ABS(0x17F5), (u8)A);
    fc = 0; adc8(0x08);
    wr8(ABS(0x17F9), (u8)A); wr8(ABS(0x17FD), (u8)A);
    lda8(0x02); wr8(DP(0x04), 0x02);
    lda8(rd8(DP(0x01)));
    if (fz) {
        lda8(rd8(DP(0x00)));
        cmp8((u8)A, 0xF0);
        if (fc) {
            lda8(0xF0);
            wr8(ABS(0x17F5), 0xF0); wr8(ABS(0x17FD), 0xF0);
            lda8(rd8(DP(0x04)));
            wr8(ABS(0x189C), (u8)A); wr8(ABS(0x189E), (u8)A);
            ret();
            return;
        }
        goto all4;
    }
    lda8(inc8((u8)A));
    if (fz) {
        lda8(rd8(DP(0x00)));
        cmp8((u8)A, 0xE9);
        if (fc) {
            wr8(DP(0x04), inc8(rd8(DP(0x04))));
            cmp8((u8)A, 0xF1);
            if (fc) goto all4;
            lda8(0xF0);
            wr8(ABS(0x17F1), 0xF0); wr8(ABS(0x17F9), 0xF0);
            lda8(rd8(DP(0x04)));
            wr8(ABS(0x189D), (u8)A); wr8(ABS(0x189F), (u8)A);
            ret();
            return;
        }
    }
    lda8(0xF0);
    wr8(ABS(0x17F1), 0xF0); wr8(ABS(0x17F5), 0xF0);
    wr8(ABS(0x17F9), 0xF0); wr8(ABS(0x17FD), 0xF0);
    ret();
    return;
all4:
    lda8(rd8(DP(0x04)));
    wr8(ABS(0x189C), (u8)A); wr8(ABS(0x189D), (u8)A);
    wr8(ABS(0x189E), (u8)A); wr8(ABS(0x189F), (u8)A);
    ret();
}
/* $81:8C33 CopyTrackColumn (RTL): the track streamer's column: from the
 * piece words at ($05) + Y + 2, ... each tile (& $07FF | $0800, palette 2)
 * into the VRAM queue $7F:000C + X and its height bits (high byte >> 3)
 * into $7F:200C + X, until X = $1E. */
void CopyTrackColumn_m1x1(void)
{
    do {
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        rep(0x20);
        lda16(rd16((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        push16(A);
        and16(0x07FF);
        ora16(0x0800);
        wr16(LONGX(0x7F000C), A);
        lda16(pull16());
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        and16(0x00FF);
        lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
        wr16(LONGX(0x7F200C), A);
        X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x1E);
    } while (!fz);
    sep(0x20);
    retl();
}
/* $80:AFE9 CpuReleaseButtons (RTS): a moving CPU rider lets go of the
 * buttons (input $017E & $0F). */
void CpuReleaseButtons_m1x1(void)
{
    lda8(rd8(ABSX(0x01C0)));
    if (fz) { ret(); return; }
    lda8(0x0F);
    input_and_a();
    ret();
}
/* $81:9F8C AimRatio (RTL, A = steps): for object X, the direction to the
 * target ($039F/$03A1 vs $027F/$028F) as a step pair: the larger axis gets
 * A, the smaller its share (Bresenham over A steps), signed by direction,
 * into $00 (vertical) and $0A (horizontal). A = 0: $00 = 0. */
void AimRatio_m1x1(void)
{
    wr8(DP(0x0A), (u8)A);
    cmp8((u8)A, 0x00);
    if (fz) { wr8(DP(0x00), 0); retl(); return; }
    push8((u8)X);
    push8((u8)Y);
    rep(0x20);
    ldy8(0x00);
    lda16(rd16(ABS(0x039B)));
    fc = 1; sbc16(rd16(ABSX(0x0289)));
    wr16(DP(0x12), A);
    if (fn) Y = inc8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    lda16(rd16(DP(0x12)));
    if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
    wr16(DP(0x10), A);
    ldy8(0x00);
    lda16(rd16(ABS(0x0399)));
    fc = 1; sbc16(rd16(ABSX(0x0279)));
    wr16(DP(0x16), A);
    if (fn) Y = inc8((u8)Y);
    rep(0x20);
    wr8(DP(0x03), (u8)Y);
    lda16(rd16(DP(0x16)));
    if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
    wr16(DP(0x14), A);
    ldy8(0x00);
    cmp16(A, rd16(DP(0x10)));
    if (!fc) {
        Y = inc8((u8)Y);
        push16(A);
        lda16(rd16(DP(0x10)));
        wr16(DP(0x14), A);
        lda16(pull16());
        wr16(DP(0x10), A);
    }
    wr16(DP(0x1E), 0);
    wr16(DP(0x00), 0);
    X = rd8(DP(0x0A)); nz8((u8)X);
    do {
        lda16(rd16(DP(0x1E)));
        fc = 0; adc16(rd16(DP(0x10)));
        cmp16(A, rd16(DP(0x14)));
        if (fc) {
            sbc16(rd16(DP(0x14)));
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
        wr16(DP(0x1E), A);
        X = dec8((u8)X);
        WAIT_TICK();
    } while (!fz);
    sep(0x20);
    lda8((u8)Y);
    if (!fz) {
        lda8(rd8(DP(0x00)));
        push8((u8)A);
        lda8(rd8(DP(0x0A)));
        wr8(DP(0x00), (u8)A);
        lda8(pull8());
        wr8(DP(0x0A), (u8)A);
    }
    lda8(rd8(DP(0x00)));
    Y = rd8(DP(0x02)); nz8((u8)Y);
    if (!fz) { eor8(0xFF); lda8(inc8((u8)A)); wr8(DP(0x00), (u8)A); }
    lda8(rd8(DP(0x0A)));
    Y = rd8(DP(0x03)); nz8((u8)Y);
    if (!fz) { eor8(0xFF); lda8(inc8((u8)A)); wr8(DP(0x0A), (u8)A); }
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}
/* $80:BE3E RiderRunOut (RTS): rider X past the finish ($08C4) and not
 * stopped ($01FE) waves ($099E = 1) and on even frames rolls on ($090C +
 * 1) until it is $80:BB30 (by lane $0948) past its column; then, or when
 * not running out, its finish state advances ($01D6 + 1) and the wave
 * timer $45 = 8. */
void RiderRunOut_m1x1(void)
{
    lda8(rd8(ABSX(0x08C4)));
    int done = fz;
    if (!done) {
        lda8(rd8(ABSX(0x01F8)));
        done = !fz;
    }
    if (!done) {
        lda8(0x01); wr8(ABSX(0x099E), 0x01);
        lda8(rd8(DP(0x63)));
        lda8(lsr8((u8)A));
        if (fc) { ret(); return; }
        wr8(ABSX(0x090C), inc8(rd8(ABSX(0x090C))));
        Y = rd8(ABSX(0x0948)); nz8((u8)Y);
        lda8(rd8(ABSX(0x0190)));
        fc = 0; adc8(rd8(ABSY(0xBE18)));
        cmp8((u8)A, rd8(ABSX(0x090C)));
        if (fc) { ret(); return; }
    }
    u8 v = inc8(rd8(ABSX(0x01D0)));
    wr8(ABSX(0x01D0), v);
    if (!fz) { lda8(0x08); wr8(DPX(0x45), 0x08); }
    ret();
}
