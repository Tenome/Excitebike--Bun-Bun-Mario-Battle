/*
 * Stadium 1 riders and small objects: movement and sprites, hand-written.
 */
#include "hand/hand.h"

/*
 * $80:D069 ClipRiderSprite (RTS). Rider X's 2x2 sprite sits at OAM offset
 * $07 (in Y). Near the screen edges hide one column of it (Y = $F0) or set
 * the X-high bit of the left column, depending on the rider's state $0198,X
 * and screen X $0190,X:
 *   state 2:            x >= $F0 -> hide right column
 *   other non-zero:     x >= $F0 -> hide right column
 *   state 0, x < $E1:   hide both columns
 *   state 0, $E1-$EF:   hide left column, X-high bit on the right column
 *   state 0, x >= $F0:  X-high bit on the left column
 */
void ClipRiderSprite_m1x1(void)
{
    u8 oam = rd8(DP(0x07));
    ldy8(oam);
    u8 state = rd8(ABS(0x0198 + X));
    lda8(state);
    u8 x = rd8(ABS(0x0190 + X));
    if (state != 0) {
        cmp8(state, 0x02);
        if (state == 0x02)
            goto hide_both;
        lda8(x);
        cmp8(x, 0xF0);
        if (fc)
            goto hide_right;
        ret();
        return;
    }
    lda8(x);
    cmp8(x, 0xE1);
    if (!fc)
        goto hide_both;
    cmp8(x, 0xF0);
    if (!fc) {
        lda8(0xF0);
        wr8(ABS(0x1601 + oam), 0xF0);          /* left column off */
        wr8(ABS(0x1609 + oam), 0xF0);
        Y = (u8)(oam >> 2);
        lda8(0x03);
        wr8(ABS(0x1821 + Y), 0x03);            /* right column: large, X bit 8 */
        wr8(ABS(0x1823 + Y), 0x03);
        ret();
        return;
    }
    Y = (u8)(oam >> 2);
    lda8(0x03);
    wr8(ABS(0x1820 + Y), 0x03);                /* left column: large, X bit 8 */
    wr8(ABS(0x1822 + Y), 0x03);
    ret();
    return;
hide_both:
    lda8(0xF0);
    wr8(ABS(0x1601 + oam), 0xF0);
    wr8(ABS(0x1609 + oam), 0xF0);
hide_right:
    lda8(0xF0);
    wr8(ABS(0x1605 + oam), 0xF0);
    wr8(ABS(0x160D + oam), 0xF0);
    ret();
}

/*
 * $80:D34A StepObjectX (RTL). Object X ($1500 table, 16-bit position
 * $1500/$1501 with fraction $1520) moves by its velocity $1510: the high
 * nibble is the signed whole part, the low nibble the fraction (in 16ths).
 * Leaves A = carry out of the fraction + whole part.
 */
void StepObjectX_m1x1(void)
{
    u8 v = rd8(ABS(0x1510 + X));
    wr8(DP(0x01), (u8)(v << 4));               /* fraction */
    u8 whole = lsr8(lsr8(lsr8(lsr8(v))));
    cmp8(whole, 0x08);
    if (whole >= 0x08)
        whole |= 0xF0;                         /* sign-extend the nibble */
    wr8(DP(0x00), whole);
    ldy8(whole & 0x80 ? 0xFF : 0x00);
    wr8(DP(0x02), (u8)Y);                      /* high byte of the step */
    lda8(rd8(ABS(0x1520 + X)));
    fc = 0;
    adc8(rd8(DP(0x01)));
    wr8(ABS(0x1520 + X), (u8)A);
    u8 carry = fc;
    push8(carry);                              /* PHA */
    lda8(rd8(ABS(0x1500 + X)));
    fc = carry;
    adc8(rd8(DP(0x00)));
    wr8(ABS(0x1500 + X), (u8)A);
    lda8(rd8(ABS(0x1501 + X)));
    adc8(rd8(DP(0x02)));
    wr8(ABS(0x1501 + X), (u8)A);
    lda8(pull8());
    fc = 0;
    adc8(rd8(DP(0x00)));
    retl();
}

/*
 * $80:D3C1 DrawSmallObject (RTL). Object X as a large sprite at OAM slot
 * X * 2 + 4 (byte offset X * 2 + 4 in the $1600 buffer): position from
 * $1500,X / $1508,X, tile from the 4-frame table $80:D3F3 stepped every 4
 * frames ($63), attribute $2D.
 */
void DrawSmallObject_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x0C);
    Y = lsr8(lsr8((u8)A));
    u8 tile = rd8(ABS(0xD3F3 + Y));
    wr8(DP(0x00), tile);
    SET_A8(asl8((u8)X));
    fc = 0;
    adc8(0x04);
    u8 oam = (u8)A;
    Y = oam;
    wr8(ABS(0x1602 + oam), rd8(DP(0x00)));     /* re-read, as the original does */
    wr8(ABS(0x1600 + oam), rd8(ABS(0x1500 + X)));
    wr8(ABS(0x1601 + oam), rd8(ABS(0x1508 + X)));
    lda8(0x2D);
    wr8(ABS(0x1603 + oam), 0x2D);
    Y = lsr8(lsr8(oam));
    lda8(0x02);
    wr8(ABS(0x1820 + Y), 0x02);                /* large size */
    retl();
}

/*
 * $81:9EF5 ObjectFacing (RTL): $02D9,X = the direction of the larger of the
 * two speed components $0299,X and $02A9,X (signed): 3/2 for $0299 positive
 * / negative, 1/0 for $02A9; unchanged when they are equal. Y is preserved.
 */
void ObjectFacing_m1x1(void)
{
    push8((u8)Y);
    ldy8(0x03);
    lda8(rd8(ABS(0x0299 + X)));
    if (fn) {
        Y = dec8((u8)Y);
        eor8(0xFF);
        lda8(inc8((u8)A));
    }
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x02), (u8)Y);
    ldy8(0x01);
    lda8(rd8(ABS(0x02A9 + X)));
    if (fn) {
        Y = dec8((u8)Y);
        eor8(0xFF);
        lda8(inc8((u8)A));
    }
    cmp8((u8)A, rd8(DP(0x00)));
    if (!fz) {
        if (!fc)
            ldy8(rd8(DP(0x02)));
        lda8((u8)Y);
        wr8(ABS(0x02D9 + X), (u8)A);
    }
    Y = pull8();
    nz8((u8)Y);
    retl();
}

/*
 * $81:932A SpawnPlayerSpark (RTL): start effect slot 0 at the player's
 * position ($0190, $01B0): kind 3 ($0110), 3 frames ($0120), speeds
 * $F8/$D0 ($0138/$0140), and set $09AD = 1.
 */
void SpawnPlayerSpark_m1x1(void)
{
    push8(DB);
    lda8(0x81);
    push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    ldx8(0x00);
    lda8(rd8(ABS(0x0190))); wr8(ABS(0x0100 + X), (u8)A);
    lda8(rd8(ABS(0x01B0))); wr8(ABS(0x0108 + X), (u8)A);
    lda8(0x00); wr8(ABS(0x0118 + X), 0x00);
    lda8(0x03); wr8(ABS(0x0110 + X), 0x03);
    lda8(0xF8); wr8(ABS(0x0138 + X), 0xF8);
    lda8(0xD0); wr8(ABS(0x0140 + X), 0xD0);
    lda8(0x03); wr8(ABS(0x0120 + X), 0x03);
    wr8(ABS(0x0128 + X), 0);
    lda8(0x01); wr8(ABS(0x09AD), 0x01);
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}

/*
 * $81:95EE StepSpark (RTS), effect slot Y: count its frames $0120 down (kind
 * $0110 cleared at zero), rise one pixel every other frame, and draw it as
 * sprite (Y + 4) in the OAM buffer: tile $8E, attribute $2D, large.
 */
void StepSpark_m1x1(void)
{
    X = Y; nz8((u8)X);
    u8 t = dec8(rd8(ABS(0x0120 + X)));
    wr8(ABS(0x0120 + X), t);
    if (t == 0)
        wr8(ABS(0x0110 + X), 0);
    lda8(rd8(ABS(0x0120 + X)));
    lda8(lsr8((u8)A));
    if (fc)
        wr8(ABS(0x0108 + X), dec8(rd8(ABS(0x0108 + X))));
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(0x0100 + X))); wr8(ABS(0x1610 + Y), (u8)A);
    lda8(rd8(ABS(0x0108 + X))); wr8(ABS(0x1611 + Y), (u8)A);
    lda8(0x8E); wr8(ABS(0x1612 + Y), 0x8E);
    lda8(0x2D); wr8(ABS(0x1613 + Y), 0x2D);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABS(0x1824 + Y), 0x02);
    ret();
}

/* ---- Stadium 1 versions of Stadium 4 routines (rider arrays sit 6 bytes
 * lower: $0178 input, $01B8/$01C0 speed, $0228/$0230 target speed). ---- */

void CpuReleaseButtons_m1x1(void);
void AimRatio_m1x1(void);

/* $80:AFD5 CpuThrottle (RTS): CPU rider X below its target speed
 * ($0228/$0230) holds A ($80); at or above it lets go of the buttons. */
void CpuThrottle_m1x1(void)
{
    ldy8(0x80);
    lda8(rd8(ABSX(0x01C0)));
    cmp8((u8)A, rd8(ABSX(0x0230)));
    if (!fc) goto press;
    if (fz) {
        lda8(rd8(ABSX(0x01B8)));
        cmp8((u8)A, rd8(ABSX(0x0228)));
        if (!fc) goto press;
    }
    CpuReleaseButtons_m1x1();
    return;
press:
    lda8((u8)Y);
    ora8(rd8(ABSX(0x0178)));
    wr8(ABSX(0x0178), (u8)A);
    ret();
}

/* $80:AFF2 CpuReleaseDpad (RTS): input $0178 & $F0. */
void CpuReleaseDpad_m1x1(void)
{
    lda8(0xF0);
    and8(rd8(ABSX(0x0178)));
    wr8(ABSX(0x0178), (u8)A);
    ret();
}

/* $80:AFFB CpuSteer (RTS): release the d-pad, then press down (1) or up
 * (2) toward the target lane angle $0971 against $01E0 (none when equal
 * or no target). */
void CpuSteer_m1x1(void)
{
    push16(0xAFFD); CpuReleaseDpad_m1x1();
    ldy8(0x01);
    lda8(rd8(ABSX(0x0971)));
    if (fz) { ret(); return; }
    cmp8((u8)A, rd8(ABSX(0x01E0)));
    if (fz) { ret(); return; }
    if (fc) Y = inc8((u8)Y);
    lda8((u8)Y);
    ora8(rd8(ABSX(0x0178)));
    wr8(ABSX(0x0178), (u8)A);
    ret();
}

/* $81:9F7A AimVelocity (RTL, A = steps): object X's velocity toward its
 * target from AimRatio ($02A9 vertical, $0299 horizontal). */
void AimVelocity_m1x1(void)
{
    push8(0x81); push16(0x9F7D); AimRatio_m1x1();
    lda8(rd8(DP(0x00))); wr8(ABSX(0x02A9), (u8)A);
    lda8(rd8(DP(0x0A))); wr8(ABSX(0x0299), (u8)A);
    retl();
}

void GroundHeightAbove_m1x1(void);
void StepObjectX_m1x1(void);
void StepObjectX8_m1x1(void);
void DrawSmallObject_m1x1(void);
void sub_80C37B_m1x1(void);

/* $80:DDAF PlayerHalt (RTS): the player's speed and jump state cleared. */
static void player_halt1(void)
{
    lda8(0x00);
    wr8(ABS(0x01B8), 0); wr8(ABS(0x01C0), 0); wr8(ABS(0x09C4), 0);
    wr8(ABS(0x09C7), 0); wr8(ABS(0x0180), 0); wr8(ABS(0x08F4), 0);
    ret();
}

void PlayerHalt_m1x1(void) { player_halt1(); }

/* $80:DDA7 PlayerStop (RTS): crashed flags $01C8 = $0999 = $FF, then
 * PlayerHalt. */
void PlayerStop_m1x1(void)
{
    lda8(0xFF);
    wr8(ABS(0x01C8), 0xFF);
    wr8(ABS(0x0999), 0xFF);
    player_halt1();
}

/* $80:DD82 PlayerColumnCheck (RTS): as Stadium 4's $80:DC70, with
 * Stadium 1's addresses (wheel column $0260, air $01E8, fall speed
 * $01B0; CrashLand is $80:C37B). */
void PlayerColumnCheck_m1x1(void)
{
    Y = rd8(ABS(0x0260)); nz8((u8)Y);
    lda8(rd8(ABS(0x01E8)));
    if (!fz) {
        lda8(rd8(ABS(0x0904)));
        if (!fz) { ret(); return; }
        lda8(rd8(ABSY(0x0A13)));
        Y = inc8((u8)Y);
        cmp8((u8)A, rd8(ABSY(0x0A13)));
        if (fz) { ret(); return; }
        push16(0xDD9A); GroundHeightAbove_m1x1();
        fc = 0; adc8(0x08);
        fc = 1; sbc8(rd8(ABSY(0x0A13)));
        cmp8((u8)A, rd8(ABS(0x01B0)));
        if (fc) { ret(); return; }
        PlayerStop_m1x1();
        return;
    }
    lda8(rd8(ABS(0x09C7)));
    if (!fz) {
        cmp8((u8)A, rd8(ABSY(0x0A13)));
        int stop;
        if (fz) {
            Y = inc8((u8)Y);
            cmp8((u8)A, rd8(ABSY(0x0A13)));
            stop = !fc;
            if (!stop) goto ramp;
        } else stop = !fc;
        if (stop) { push16(0xDDD2); PlayerStop_m1x1(); }
        push16(0xDDD5); CALL(sub_80C37B_m1x1, 0x80C37B, 2);
        lda8(rd8(ABS(0x08D4))); wr8(ABS(0x08E4), (u8)A);
        lda8(rd8(ABS(0x08DC))); wr8(ABS(0x08EC), (u8)A);
        ret();
        return;
    }
ramp:
    ldx8(0x01);
    lda8(rd8(ABSY(0x09D3)));
    cmp8((u8)A, 0xFC);
    if (!fz) ldx8(0x00);
    wr8(ABS(0x0958), (u8)X);
    ret();
}

/* $80:D31F MarkerObjectStep (RTL, X = object): move object X and X + 8,
 * count its timer $1518 up and draw it; once its Y $1508 reaches $C0,
 * clear $1530 and count $1541 up to 5. */
void MarkerObjectStep_m1x1(void)
{
    push8(0x80); push16(0xD322); StepObjectX_m1x1();
    push8((u8)X);
    push8(0x80); push16(0xD327); StepObjectX8_m1x1();
    X = pull8(); nz8((u8)X);
    wr8(ABSX(0x1518), inc8(rd8(ABSX(0x1518))));
    push8(0x80); push16(0xD32F); DrawSmallObject_m1x1();
    lda8(rd8(ABSX(0x1508)));
    cmp8((u8)A, 0xC0);
    if (fc) {
        wr8(ABSX(0x1530), 0);
        lda8(rd8(ABS(0x1541)));
        cmp8((u8)A, 0x05);
        if (!fc) wr8(ABS(0x1541), inc8(rd8(ABS(0x1541))));
    }
    retl();
}
