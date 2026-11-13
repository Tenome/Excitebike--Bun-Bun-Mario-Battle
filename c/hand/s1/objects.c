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
void CrashLand_m1x1(void);

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
        push16(0xDDD5); CrashLand_m1x1();
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

void LanePieceTables_m1x1(void);
void BsxCheckSerial_m1x1(void);
void AnimSourceFromFrame_m1x1(void);
void EmptyHook_m1x1(void);
void VramQueueTiles20_m1x1(void);

/* $80:E8B9 LanePieceAt (RTS, Y = lane, $08 = column): lane Y's row
 * pointer, $07 = lane, the piece's height byte into $10 and A = its type
 * byte ($0180 further on). */
static void lane_piece1(void)
{
    push16(0xE8BB); LanePieceTables_m1x1();
    wr8(DP(0x07), (u8)Y);
    Y = rd8(DP(0x08)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF));
    wr8(DP(0x10), (u8)A);
    push8((u8)Y);
    rep(0x30);
    lda16(Y);
    fc = 0; adc16(0x0180);
    Y = A; nz16(Y);
    sep(0x20);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF));
    sep(0x10);
    Y = pull8(); nz8((u8)Y);
    ret();
}

void LanePieceAt_m1x1(void) { lane_piece1(); }

/* $80:E8B4 RiderPieceAt (RTS, Y = lane): LanePieceAt at rider X's wheel
 * column $0260. */
void RiderPieceAt_m1x1(void)
{
    lda8(rd8(ABSX(0x0260)));
    wr8(DP(0x08), (u8)A);
    lane_piece1();
}

/* $80:E8D5 LanePieceTablesLong (RTL): LanePieceTables with DB = $80. */
void LanePieceTablesLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xE8DC); LanePieceTables_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:99BB Mode0F_EndScreen (RTS): the serial check, then the end-screen
 * step $0453 through $80:99D1 (JMP ($0000)). */
void Mode0F_EndScreen_m1x1(void)
{
    push16(0x99BD); BsxCheckSerial_m1x1();
    lda8(rd8(ABS(0x0453)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x99D1))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(0x99D2))); wr8(DP(0x01), (u8)A);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}

/* $80:DE07 Mode0A (RTS): the bonus course: its animation source, the
 * empty hook, then (once $3A is clear) its step $77 through $80:DE1B. */
void Mode0A_m1x1(void)
{
    push8(0x80); push16(0xDE0A); AnimSourceFromFrame_m1x1();
    push8(0x80); push16(0xDE0E); EmptyHook_m1x1();
    lda8(rd8(DP(0x3A)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x77)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    r816_call_pc(0x800000u | rd16(LONGX(0x80DE1B)));
}

/* $80:C805 ShowRaceClock (RTS): the race time $03B1 (minutes), $03B2
 * (seconds) and $03B3 (frames -> hundredths, $80:C6CE) as two-row digits
 * at $03D2 (from the digit pairs $80:C70A), queued with $0F = $20. */
void ShowRaceClock_m1x1(void)
{
    rep(0x20);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    X = rd8(ABS(0x03B3)); nz8((u8)X);
    lda16(rd16(ABSX(0xC6CE)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC70A))); wr16(ABS(0x03D8), A);
    lda16(rd16(ABS(0x03B2)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC70A))); wr16(ABS(0x03D5), A);
    sep(0x20);
    lda8(rd8(ABS(0x03B1))); wr8(ABS(0x03D3), (u8)A);
    lda8(0x27); wr8(ABS(0x03D2), 0x27);
    rep(0x20);
    lda16(rd16(ABS(0x03D3))); ora16(0x9750); wr16(ABS(0x03D3), A);
    fc = 0; adc16(0x6510); wr16(ABS(0x03DA), A);
    lda16(rd16(ABS(0x03D5))); ora16(0x5050); wr16(ABS(0x03D5), A);
    fc = 0; adc16(0x1010); wr16(ABS(0x03DC), A);
    lda16(rd16(ABS(0x03D7))); ora16(0x5098); wr16(ABS(0x03D7), A);
    fc = 0; adc16(0x1064); wr16(ABS(0x03DE), A);
    sep(0x20);
    lda8(rd8(ABS(0x03D9))); ora8(0x50); wr8(ABS(0x03D9), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(0x03E0), (u8)A);
    VramQueueTiles20_m1x1();
}

/* $80:C7EB TickRaceClock (RTS): at 60 frames ($03B3) carry into the
 * seconds $03B2 and at 60 into the minutes $03B1; then ShowRaceClock. */
void TickRaceClock_m1x1(void)
{
    lda8(rd8(ABS(0x03B3)));
    cmp8((u8)A, 0x3C);
    if (fc) {
        wr8(ABS(0x03B2), inc8(rd8(ABS(0x03B2))));
        lda8(rd8(ABS(0x03B2)));
        cmp8((u8)A, 0x3C);
        if (fc) {
            wr8(ABS(0x03B1), inc8(rd8(ABS(0x03B1))));
            wr8(ABS(0x03B2), 0);
        }
        wr8(ABS(0x03B3), 0);
    }
    ShowRaceClock_m1x1();
}

/* $80:AF9F RiderAhead (RTS, X = rider, Y = window, $02 = row, $0F = self):
 * C set when another rider on screen page 1 in row $02 sits between
 * rider X's position + $80:B86F,Y and that less $80:B870,Y (riders
 * from $7E up). */
void RiderAhead_m1x1(void)
{
    lda8(rd8(ABSX(0x0190)));
    fc = 0; adc8(rd8(ABSY(0xB86F)));
    wr8(DP(0x00), (u8)A);
    fc = 1; sbc8(rd8(ABSY(0xB870)));
    wr8(DP(0x01), (u8)A);
    Y = rd8(DP(0x7E)); nz8((u8)Y);
    do {
        cmp8((u8)Y, rd8(DP(0x0F)));
        if (!fz) {
            lda8(rd8(ABSY(0x0198)));
            lda8(lsr8((u8)A));
            if (fc) {
                lda8(rd8(ABSY(0x0958)));
                if (!fz) {
                    cmp8((u8)A, rd8(DP(0x02)));
                    if (fz) {
                        lda8(rd8(ABSY(0x0190)));
                        cmp8((u8)A, rd8(DP(0x00)));
                        if (!fc) {
                            cmp8((u8)A, rd8(DP(0x01)));
                            if (fc) { ret(); return; }
                        }
                    }
                }
            }
        }
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x08);
    } while (!fc);
    fc = 0;
    ret();
}

void QueueSound_m1x1(void);

/* $80:ED42 CrossFinish (RTS, X = rider): the player crossing the finish
 * once ($03AF, not $84): $5E = $A4 = $1D; with a finish result $093F
 * sound $4A (1st) or $28, a win counted ($1E87) without rivals and $5E
 * cleared when $60; with none, $84 = 1 and sounds $2B, $2D, $80, $5E
 * cleared. */
void CrossFinish_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x03AF)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    wr8(ABS(0x03AF), inc8(rd8(ABS(0x03AF))));
    lda8(0x1D); wr8(DP(0x5E), 0x1D); wr8(DP(0xA4), 0x1D);
    lda8(rd8(ABS(0x093F)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) { lda8(0x4A); push8(0x80); push16(0xED66); QueueSound_m1x1(); }
        else { lda8(0x28); push8(0x80); push16(0xED6E); QueueSound_m1x1(); }
        lda8(rd8(DP(0x7D)));
        if (fz) wr8(ABS(0x1E87), inc8(rd8(ABS(0x1E87))));
        lda8(rd8(DP(0x60)));
        if (fz) { ret(); return; }
    } else {
        lda8(0x01); wr8(DP(0x84), 0x01);
        lda8(0x2B); push8(0x80); push16(0xED84); QueueSound_m1x1();
        lda8(0x2D); push8(0x80); push16(0xED8A); QueueSound_m1x1();
        lda8(0x80); push8(0x80); push16(0xED90); QueueSound_m1x1();
    }
    lda8(0x00); wr8(DP(0x5E), 0x00);
    ret();
}

/* $80:DA31 (RTS): the bonus banner sprites: for banner $BB (1-based; at
 * 4 only while $8A counts down) four OAM words from $80:DA65 + 8 *
 * ($BB - 1) into $16A0-$16A7, size 2 for both. X is kept. */
static void banner_sprites(u16 table)
{
    lda8(rd8(DP(0xBB)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x04);
    if (fz) {
        lda8(rd8(DP(0x8A)));
        if (fz) { ret(); return; }
        wr8(DP(0x8A), dec8(rd8(DP(0x8A))));
    }
    push8((u8)X);
    lda8(rd8(DP(0xBB)));
    lda8(dec8((u8)A));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    ldy8(0xA0);
    rep(0x20);
    do {
        lda16(rd16(ABSX(table)));
        wr16(ABSY(0x1600), A);
        X = inc8((u8)X); X = inc8((u8)X);
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        cmp8((u8)Y, 0xA8);
    } while (!fc);
    ldy8(0x28);
    lda16(0x0202); wr16(ABSY(0x1820), 0x0202);
    sep(0x20);
    X = pull8(); nz8((u8)X);
    ret();
}

void BonusBannerSprites_m1x1(void) { banner_sprites(0xDA65); }

/* $80:AA78 (RTS): BonusBannerSprites' twin for the race banners
 * (table $80:AAAC). */
void RaceBannerSprites_m1x1(void) { banner_sprites(0xAAAC); }

void VramQueueTiles_m1x1(void);
void VramQueueCopy_m1x1(void);
void FadeSkyColours_m1x1(void);
void Random_m1x1(void);

/* $80:EE68 (RTS): the six score digits $0CE5 as two rows at $0CEC /
 * $0CF2 (+$50 / +$60; leading zeros blank $0F except the last), size
 * $26, queued from $0CEB (VRAM address in $00). */
void QueueScoreDigits_m1x1(void)
{
    ldx8(0x00);
    Y = X; nz8((u8)Y);
    do {
        lda8(rd8(ABSX(0x0CE5)));
        int blank = 0;
        if (fz) {
            cmp8((u8)Y, 0x00);
            if (fz) { cmp8((u8)X, 0x05); blank = !fz; }
        }
        if (blank) lda8(0x0F);
        else { Y = inc8((u8)Y); fc = 0; adc8(0x50); }
        wr8(ABSX(0x0CEC), (u8)A);
        fc = 0; adc8(0x10);
        wr8(ABSX(0x0CF2), (u8)A);
        X = inc8((u8)X);
        cmp8((u8)X, 0x06);
    } while (!fc);
    lda8(0x26); wr8(ABS(0x0CEB), 0x26);
    lda8(0xEB); wr8(DP(0x02), 0xEB);
    lda8(0x0C); wr8(DP(0x03), 0x0C);
    VramQueueTiles_m1x1();
}

/* The end screen's box: 2 rows of $20 from $89:DA00 to VRAM $0300. */
static void end_box(u16 back)
{
    lda8(0x00); wr8(DP(0x02), 0x00);
    lda8(0xDA); wr8(DP(0x03), 0xDA);
    lda8(0x89); wr8(DP(0x04), 0x89);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(back); VramQueueCopy_m1x1();
}

/* $80:9D07 (RTS): on the end screen's prompt a press of A (bit 7): with
 * the new press ($26) the next step ($1923 - 1, $0453 + 1, sound $45);
 * held from before ($21) the box is redrawn and the step goes back. */
void EndScreenPrompt_m1x1(void)
{
    lda8(rd8(DP(0x26)));
    ora8(rd8(DP(0x21)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x26)));
    if (!fz) {
        wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
        wr8(ABS(0x0453), inc8(rd8(ABS(0x0453))));
        lda8(0x45);
        push8(0x80); push16(0x9D1E); QueueSound_m1x1();
        ret();
        return;
    }
    end_box(0x9D3E);
    wr8(ABS(0x0453), dec8(rd8(ABS(0x0453))));
    ret();
}

/* $80:9BC8 (RTS): the end screen's scroll-in: BG X $9A moves 2 a frame;
 * at $0140 sound $2D, the next step, the box, $03A9 = 1 and $0452
 * cleared. */
void EndScreenScrollIn_m1x1(void)
{
    rep(0x20);
    wr16(ABS(0x009A), inc16(rd16(ABS(0x009A))));
    wr16(ABS(0x009A), inc16(rd16(ABS(0x009A))));
    lda16(rd16(ABS(0x009A)));
    cmp16(A, 0x0140);
    if (fc) {
        sep(0x20);
        lda8(0x2D);
        push8(0x80); push16(0x9BDF); QueueSound_m1x1();
        wr8(ABS(0x0453), inc8(rd8(ABS(0x0453))));
        end_box(0x9C01);
        lda8(0x01); wr8(ABS(0x03A9), 0x01);
        wr8(ABS(0x0452), 0);
    }
    sep(0x20);
    ret();
}

/* $81:97E2 (RTS): the sky fade object ($039D): the sky colours fade, and
 * when its word timer $0319 runs out its state steps, the palette
 * entries $1C02-$1C16 become $6D69, and $9D + 1, $88 = $C0, $B1 = 6. */
void SkyFadeObject_m1x1(void)
{
    push8(0x81); push16(0x97E5); FadeSkyColours_m1x1();
    X = rd8(ABS(0x039D)); nz8((u8)X);
    rep(0x20);
    wr16(ABSX(0x0319), dec16(rd16(ABSX(0x0319))));
    if (fz) {
        wr16(ABSX(0x0309), inc16(rd16(ABSX(0x0309))));
        rep(0x20);
        lda16(0x6D69);
        for (u16 a = 0x1C02; a <= 0x1C16; a += 2) wr16(ABS(a), 0x6D69);
        sep(0x20);
        wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
        wr8(DP(0x9D), inc8(rd8(DP(0x9D))));
        lda8(0xC0); wr8(DP(0x88), 0xC0);
        lda8(0x06); wr8(DP(0xB1), 0x06);
    }
    sep(0x20);
    ret();
}

/* $81:BBE4 (RTS): object $039D's idle animation: when its delay $0369 is
 * out the step $0319 cycles 0-3 (delay and frame from $81:BC26 /
 * $81:BC2A); after $0329 rounds it goes to state 2 (delay $0A, frame 6,
 * counter $40-$7F). */
void IdleCycleObject_m1x1(void)
{
    X = rd8(ABS(0x039D)); nz8((u8)X);
    lda8(rd8(ABSX(0x0369)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x0319)));
    lda8(inc8((u8)A));
    and8(0x03);
    wr8(ABSX(0x0319), (u8)A);
    Y = (u8)A; nz8((u8)Y);
    if (fz) {
        wr8(ABSX(0x0329), dec8(rd8(ABSX(0x0329))));
        if (fz) {
            lda8(0x02); wr8(ABSX(0x0309), 0x02);
            lda8(0x0A); wr8(ABSX(0x0369), 0x0A);
            lda8(0x06); wr8(ABSX(0x0349), 0x06);
            push8(0x81); push16(0xBC0F); Random_m1x1();
            and8(0x3F);
            fc = 0; adc8(0x40);
            wr8(ABSX(0x0379), (u8)A);
            ret();
            return;
        }
    }
    lda8(rd8(ABSY(0xBC26))); wr8(ABSX(0x0369), (u8)A);
    lda8(rd8(ABSY(0xBC2A))); wr8(ABSX(0x0349), (u8)A);
    ret();
}

void LanePieceTablesLong_m1x1(void);
void DrawCoinIcons_m1x1(void);

/* $81:8BE2 FillLaneColumn (RTL): as Stadium 4's $81:8BEB (column $0F of
 * the six lane rows from the course buffer, entry $1A down, skipping
 * $FC ramp tops). */
void FillLaneColumn_m1x1(void)
{
    ldx8(0x05);
    lda8(0x1A); wr8(DP(0x0C), 0x1A);
    do {
        Y = X; nz8((u8)Y);
        push8(0x81); push16(0x8BEC); LanePieceTablesLong_m1x1();
        wr8(DP(0x0A), (u8)X);
        for (;;) {
            X = rd8(DP(0x0C)); nz8((u8)X);
            lda8(rd8(LONGX(0x7F000E)));
            cmp8((u8)A, 0xFC);
            if (!fz) break;
            wr8(DP(0x0C), dec8(rd8(DP(0x0C))));
            wr8(DP(0x0C), dec8(rd8(DP(0x0C))));
            lda8(rd8(DP(0x0C)));
            if (fn) break;
        }
        Y = rd8(DP(0x0F)); nz8((u8)Y);
        wr8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF, (u8)A);
        rep(0x30);
        lda16(Y);
        fc = 0; adc16(0x0180);
        Y = A; nz16(Y);
        sep(0x20);
        lda8(rd8(LONGX(0x7F200E)));
        wr8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF, (u8)A);
        sep(0x10);
        wr8(DP(0x0C), dec8(rd8(DP(0x0C))));
        wr8(DP(0x0C), dec8(rd8(DP(0x0C))));
        X = rd8(DP(0x0A)); nz8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    retl();
}

/* $80:9942 QueueFiveDigits (RTL): the five digits at [$18] (leading zeros
 * blank $0C) as two rows at $03D3 / $03D9 (+$10, trailing 0 / $10), size
 * $26 from $03D2, queued at VRAM $00. X and Y are kept. */
void QueueFiveDigits_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    ldx8(0x00); ldy8(0x00);
    do {
        lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
        int blank = 0;
        if (fz) { cmp8((u8)X, 0x00); blank = fz; }
        if (blank) lda8(0x0C);
        else X = inc8((u8)X);
        wr8(ABSY(0x03D3), (u8)A);
        ora8(0x10);
        wr8(ABSY(0x03D9), (u8)A);
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x05);
    } while (!fc);
    wr8(ABS(0x03D8), 0);
    lda8(0x10); wr8(ABS(0x03DE), 0x10);
    lda8(0x26); wr8(ABS(0x03D2), 0x26);
    rep(0x20);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push16(0x997E); VramQueueTiles_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}

/* $80:DC8D CourseColumn (RTS): as Stadium 4's $80:DB7B (course $0277's
 * pointer tables $80:D4E2 / $80:D4EB). */
void CourseColumn_m1x1(void)
{
    X = rd8(ABS(0x0277)); nz8((u8)X);
    lda8(rd8(ABSX(0xD4E2))); wr8(DP(0x09), (u8)A);
    lda8(rd8(ABSX(0xD4EB))); wr8(DP(0x0A), (u8)A);
    ldy8(0x00); wr8(DP(0x0E), 0);
    do {
        Y = rd8(DP(0x0E)); nz8((u8)Y);
        push16(0xDCA2); LanePieceTables_m1x1();
        Y = rd8(ABS(0x09C3)); nz8((u8)Y);
        lda8(rd8((((u32)DB << 16) + rd16(DP(0x09)) + Y) & 0xFFFFFF));
        wr8(ABS(0x09C3), inc8(rd8(ABS(0x09C3))));
        Y = rd8(DP(0x0F)); nz8((u8)Y);
        wr8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF, (u8)A);
        wr8(DP(0x10), (u8)A);
        wr8(DP(0x0E), inc8(rd8(DP(0x0E))));
        lda8(rd8(DP(0x0E)));
        cmp8((u8)A, 0x03);
        if (fz) {
            lda8(rd8(DP(0x10)));
            if (fz) lda8(0x3B);
            wr8(ABSY(0x0B53), (u8)A); wr8(ABSY(0x0B93), (u8)A);
            wr8(ABSY(0x0BD3), (u8)A); wr8(ABSY(0x0C13), (u8)A);
            wr8(ABSY(0x0C53), (u8)A); wr8(ABSY(0x0C93), (u8)A);
            lda8(rd8(DP(0x0E)));
        }
        cmp8((u8)A, 0x04);
    } while (!fz);
    ret();
}

/* $80:D29F (RTL): the marker objects (even slots 6-0, $1530: word timer
 * $1538 up, MarkerObjectStep), the coin icons, and every 8 frames the
 * cycling palette entries $1C08-$1C0F from $80:D2DF ($63 & $38). */
void MarkerObjects_m1x1(void)
{
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x1530)));
        if (!fz) {
            rep(0x20);
            wr16(ABSX(0x1538), inc16(rd16(ABSX(0x1538))));
            sep(0x20);
            push8(0x80); push16(0xD2B0); MarkerObjectStep_m1x1();
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    push8(0x80); push16(0xD2B8); DrawCoinIcons_m1x1();
    lda8(rd8(DP(0x63)));
    and8(0x07);
    if (!fz) { retl(); return; }
    push8((u8)X);
    ldy8(0x00);
    lda8(rd8(DP(0x63)));
    and8(0x38);
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    do {
        lda16(rd16(LONGX(0x80D2DF)));
        wr16(ABSY(0x1C08), A);
        X = inc8((u8)X); X = inc8((u8)X);
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        cmp8((u8)Y, 0x08);
    } while (!fc);
    sep(0x20);
    X = pull8(); nz8((u8)X);
    wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
    retl();
}

/* $81:95A3 (RTS, Y = piece): a spark piece's sprite ($1610 + 2Y:
 * position $0100 / $0108, tile $81:95EB by frame $0118, attribute $2D,
 * size 2); every 4 frames ($0120) the frame steps, and after frame 2 the
 * state $0128 steps with a $40 wait. */
void SparkPieceStep_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(0x0100))); wr8(ABSY(0x1610), (u8)A);
    lda8(rd8(ABSX(0x0108))); wr8(ABSY(0x1611), (u8)A);
    lda8(rd8(ABSX(0x0118)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x8195EB))); wr8(ABSY(0x1612), (u8)A);
    lda8(0x2D); wr8(ABSY(0x1613), 0x2D);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(0x1824), 0x02);
    wr8(ABSX(0x0120), dec8(rd8(ABSX(0x0120))));
    if (!fz) { ret(); return; }
    lda8(0x04); wr8(ABSX(0x0120), 0x04);
    wr8(ABSX(0x0118), inc8(rd8(ABSX(0x0118))));
    lda8(rd8(ABSX(0x0118)));
    cmp8((u8)A, 0x03);
    if (!fc) { ret(); return; }
    wr8(ABSX(0x0128), inc8(rd8(ABSX(0x0128))));
    lda8(0x40); wr8(ABSX(0x0120), 0x40);
    ret();
}

/* $80:9A52 EndScreenScore (RTS): when $88 runs out, the five digits at
 * $00:0CE5 at VRAM $0975, $88 = $20 and the next end-screen steps
 * ($0453 = 4 and $0454 = 5, or 3 when $085D is set). */
void EndScreenScore_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    rep(0x20);
    lda16(0x0CE5); wr16(DP(0x18), 0x0CE5);
    lda16(0x0975); wr16(DP(0x00), 0x0975);
    sep(0x20);
    lda8(0x00); wr8(DP(0x1A), 0x00);
    push8(0x80); push16(0x9A6B); QueueFiveDigits_m1x1();
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(rd8(ABS(0x085D)));
    if (fz) {
        lda8(0x04); wr8(ABS(0x0453), 0x04);
        lda8(0x05); wr8(ABS(0x0454), 0x05);
    } else {
        lda8(0x03); wr8(ABS(0x0453), 0x03);
    }
    ret();
}

/* $80:9AF9 EndScreenCode (RTS): after the $88 delay the code box (2 rows
 * of $20 from $89:D880 at $0300); then A ($26 bit 7): $88 = $20, sound
 * $43, step $0453 = $0A and the five digits at $11:5730 to VRAM $0A15. */
void EndScreenCode_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) {
        u8 t = dec8(rd8(DP(0x88)));
        wr8(DP(0x88), t);
        if (!fz) { ret(); return; }
        lda8(0x80); wr8(DP(0x02), 0x80);
        lda8(0xD8); wr8(DP(0x03), 0xD8);
        lda8(0x89); wr8(DP(0x04), 0x89);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x9B1F); VramQueueCopy_m1x1();
    }
    lda8(rd8(DP(0x26)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(0x43);
    push8(0x80); push16(0x9B2F); QueueSound_m1x1();
    lda8(0x0A); wr8(ABS(0x0453), 0x0A);
    rep(0x20);
    lda16(0x0A15); wr16(DP(0x00), 0x0A15);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9B4A); QueueFiveDigits_m1x1();
    ret();
}

/* $80:C3B3 StartJump (RTS, X = rider): rider X takes off: air state 2,
 * height step $0F, the take-off speed ($01C0:$01B8 + $AF, halved when
 * $08F4 = 2) into $08DC:$08D4. The player also clears $09C7, plays
 * sound $20 and returns A = speed high * 16 ^ $30 (8 when that is 0). */
void StartJump_m1x1(void)
{
    lda8(0x02); wr8(ABSX(0x01E8), 0x02);
    lda8(0x0F); wr8(ABSX(0x08E4), 0x0F);
    lda8(rd8(ABSX(0x01B8)));
    fc = 0; adc8(0xAF);
    wr8(ABSX(0x08D4), (u8)A);
    lda8(rd8(ABSX(0x01C0)));
    adc8(0x00);
    wr8(ABSX(0x08DC), (u8)A);
    lda8(rd8(ABSX(0x08F4)));
    cmp8((u8)A, 0x02);
    if (fz) {
        u8 hi = rd8(ABSX(0x08DC));
        fc = hi & 1; hi >>= 1; wr8(ABSX(0x08DC), hi); nz8(hi);
        u8 lo = rd8(ABSX(0x08D4));
        u8 c = fc; fc = lo & 1; lo = (u8)((lo >> 1) | (c << 7)); wr8(ABSX(0x08D4), lo); nz8(lo);
    }
    lda8((u8)X);
    if (!fz) { ret(); return; }
    wr8(ABS(0x09C7), (u8)A);
    lda8(0x20);
    push8(0x80); push16(0xC3E6); QueueSound_m1x1();
    lda8(rd8(ABSX(0x01C0)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    eor8(0x30);
    if (fz) lda8(0x08);
    ret();
}

/* $80:AC9C RiderOnRamp (RTS): Stadium 4's $80:ACD6 with Stadium 1's
 * addresses (piece $0210, table $80:ACF9, air $0208, fall $01B0). */
void RiderOnRamp_m1x1(void)
{
    Y = rd8(ABSX(0x0210)); nz8((u8)Y);
    lda8(rd8(ABSY(0xACF9)));
    if (fz) {
        lda8(0x03);
        cmp8((u8)A, rd8(ABSX(0x0208)));
        int ramp = 0;
        if (fz) {
            lda8(rd8(ABSX(0x0958)));
            cmp8((u8)A, 0x03);
            ramp = fc;
        }
        if (!ramp) {
            lda8(rd8(ABSX(0x0958)));
            if (!fz) {
                lda8(0x01);
                cmp8((u8)A, rd8(ABSX(0x08BC)));
                if (fz) { ret(); return; }
                lda8(lsr8((u8)A));
                wr8(ABSX(0x08BC), (u8)A);
                ret();
                return;
            }
        }
    }
    lda8(0x01);
    cmp8((u8)A, rd8(ABSX(0x08BC)));
    if (!fz) {
        push8((u8)A);
        lda8(rd8(DP(0x76)));
        cmp8((u8)A, 0x09);
        if (fz) {
            lda8(rd8(ABSY(0xACF9)));
            if (fn) {
                lda8(rd8(ABS(0x08BC)));
                if (fz) {
                    lda8(rd8(ABS(0x01B0)));
                    cmp8((u8)A, 0xB0);
                    if (fz) {
                        lda8(0x46);
                        push8(0x80); push16(0xACDB); QueueSound_m1x1();
                    }
                }
            }
        }
        lda8(pull8());
        lda8(asl8((u8)A));
        wr8(ABSX(0x08BC), (u8)A);
    }
    lda8((u8)X);
    if (fz) {
        lda8(rd8(ABS(0x01B8)));
        ora8(rd8(ABS(0x01C0)));
    }
    ret();
}

void GroundHeightAbove_m1x1(void);

/* $80:CB47 BallHit (RTS): Stadium 4's $80:C997 with Stadium 1's
 * addresses: the player running into the ball stops (A = 1). */
void BallHit_m1x1(void)
{
    lda8(rd8(ABS(0x01D0)));
    ora8(rd8(ABS(0x0940)));
    if (!fz) { ret(); return; }
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(0x019B)));
    lda8(lsr8((u8)A));
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(0x08C7)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x01F0)));
    fc = 1; sbc8(0x03);
    cmp8((u8)A, rd8(ABS(0x097C)));
    if (fc) { ret(); return; }
    adc8(0x07);
    cmp8((u8)A, rd8(ABS(0x097C)));
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(0x0190)));
    cmp8((u8)A, rd8(ABS(0x090F)));
    if (fc) { ret(); return; }
    adc8(0x12);
    cmp8((u8)A, rd8(ABS(0x090F)));
    if (!fc) { ret(); return; }
    push16(0xCB7E); GroundHeightAbove_m1x1();
    fc = 1; sbc8(0x08);
    cmp8((u8)A, rd8(ABS(0x01B0)));
    if (fc) { ret(); return; }
    ldy8(0xFF); wr8(ABS(0x01C8), 0xFF);
    Y = inc8((u8)Y); wr8(ABS(0x01AB), (u8)Y);
    Y = inc8((u8)Y); wr8(ABS(0x01D3), (u8)Y);
    Y = inc8((u8)Y); wr8(ABS(0x091F), (u8)Y);
    lda8(0x01);
    ret();
}

void SinCos_m1x1(void);

/* $81:A126 (RTL, X = wheel): a select-screen wheel at angle $1547,X on a
 * radius $40 around ($68, $50) into $154C / $1550; for wheel 0 a click
 * (sound $41, $154B cleared) at the notches ($00 / $56 / $AA, or every
 * $40 with $1E8B) while $154B is set and no d-pad is held. */
void SelectWheelPosition_m1x1(void)
{
    push8((u8)X);
    lda8(0x40); wr8(DP(0x08), 0x40);
    lda8(rd8(ABSX(0x1547)));
    X = (u8)A; nz8((u8)X);
    push8(0x81); push16(0xA132); SinCos_m1x1();
    X = pull8(); nz8((u8)X);
    lda8(0x68);
    fc = 0; adc8(rd8(DP(0x04)));
    wr8(ABSX(0x154C), (u8)A);
    lda8(0x50);
    fc = 0; adc8(rd8(DP(0x00)));
    wr8(ABSX(0x1550), (u8)A);
    cmp8((u8)X, 0x00);
    if (!fz) { retl(); return; }
    int notch;
    lda8(rd8(ABS(0x1E8B)));
    if (!fz) {
        lda8(rd8(ABSX(0x1547)));
        and8(0x3F);
        notch = fz;
    } else {
        lda8(rd8(ABS(0x1547)));
        notch = fz;
        if (!notch) { cmp8((u8)A, 0x56); notch = fz; }
        if (!notch) { cmp8((u8)A, 0xAA); notch = fz; }
    }
    if (!notch) { retl(); return; }
    lda8(rd8(ABS(0x154B)));
    if (fz) { retl(); return; }
    lda8(rd8(ABS(0x154B)));
    if (fz) { retl(); return; }
    lda8(rd8(DP(0x20)));
    and8(0x03);
    if (!fz) { retl(); return; }
    lda8(0x41);
    push8(0x81); push16(0xA178); QueueSound_m1x1();
    wr8(ABS(0x154B), 0);
    retl();
}

/* $80:C04B (RTS): during a race with rivals ($7D, $8E, not $84) each
 * rider's distance $085E: riders on the track from their position
 * ($0198:$0190 + $94 - $0100), the others advanced by their speed
 * ($0158 fraction + $0D01). */
void UpdateRiderDistances_m1x1(void)
{
    lda8(rd8(DP(0x7D)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x8E)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    rep(0x20);
    ldy8(0x0E);
    ldx8(0x07);
    do {
        lda16(rd16(ABSX(0x01D8)));
        and16(0x00FF);
        if (!fz) {
            lda16(rd16(ABSX(0x0190))); wr16(DP(0x00), A);
            lda16(rd16(ABSX(0x0198))); wr16(DP(0x01), A);
            lda16(rd16(DP(0x00)));
            fc = 0; adc16(rd16(DP(0x94)));
            fc = 1; sbc16(0x0100);
            wr16(ABSY(0x085E), A);
        } else {
            lda16(rd16(ABSY(0x0158)));
            fc = 0; adc16(rd16(ABSY(0x0D01)));
            push16(A);
            and16(0x00FF);
            wr16(ABSY(0x0158), A);
            lda16(pull16());
            A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
            and16(0x00FF);
            fc = 0; adc16(rd16(ABSY(0x085E)));
            wr16(ABSY(0x085E), A);
        }
        X = dec8((u8)X);
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fn);
    sep(0x20);
    ret();
}

void RiderHeightStep_m1x1(void);
void StartJump_m1x1(void);

/* $80:C391: the crash steps: rider X's pad picks the tumble step
 * ($80:BD42 -> $08FC), the height moves (RiderHeightStep from $01B0)
 * and, off the ramp ($0220 = 0), $08AC = 1. */
static void crash_tail1(void)
{
    lda8(rd8(ABSX(0x0178)));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBD42)));
    wr8(ABSX(0x08FC), (u8)A);
    lda8(rd8(ABSX(0x01B0)));
    wr8(DP(0x01), (u8)A);
    push16(0xC3A4); RiderHeightStep_m1x1();
    wr8(ABSX(0x01B0), (u8)A);
    lda8(rd8(ABSX(0x0220)));
    if (fz) { lda8(0x01); wr8(ABSX(0x08AC), 0x01); }
}

/* $80:C37E: A is the crash speed ($08EC); the crash starts (StartJump),
 * then the steps. */
static void crash_from1(void)
{
    wr8(ABSX(0x08EC), (u8)A);
    push16(0xC383); StartJump_m1x1();
    crash_tail1();
}

/* $80:C37B CrashLand (RTS): crash at rider X's speed $01C0. */
void CrashLand_m1x1(void)
{
    lda8(rd8(ABSX(0x01C0)));
    crash_from1();
    ret();
}

/* $80:C34C RiderLandCheck (RTS): rider X in air state 2 continues the
 * crash steps unless the player presses right ($0178 bit 0, $7F
 * clear); any other nonzero state crashes it at speed 0. */
void RiderLandCheck_m1x1(void)
{
    lda8(rd8(ABSX(0x01E8)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x02);
    if (!fz) { lda8(0x00); crash_from1(); ret(); return; }
    lda8(rd8(ABSX(0x0178)));
    lda8(lsr8((u8)A));
    if (fc) {
        lda8(rd8(DP(0x7F)));
        if (fz) { ret(); return; }
    }
    crash_tail1();
    ret();
}

/* $80:EB0C (RTS, X = rider): on the ground the speed $01C0:$01B8 grows
 * by $0180, then a crash at speed 0. */
void SpeedBumpCrash_m1x1(void)
{
    lda8(rd8(ABSX(0x01E8)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x01B8)));
    fc = 0; adc8(0x80);
    wr8(ABS(0x01B8), (u8)A);
    lda8(rd8(ABS(0x01C0)));
    adc8(0x01);
    wr8(ABS(0x01C0), (u8)A);
    lda8(0x00);
    crash_from1();
    ret();
}

void BallHit_m1x1(void);
void FlagRidersAtLine_m1x1(void);

/* $80:AB2A (RTS): on even frames the ball check; on frames 1 mod 4 the
 * line flags and the speed gauge ($80:C604). */
void BallOrGauge_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    lda8(lsr8((u8)A));
    if (!fc) {
        push16(0xAB31); BallHit_m1x1();
        ret();
        return;
    }
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    push16(0xAB38); FlagRidersAtLine_m1x1();
    r816_call_pc(0x80C604);
}

void SelectWheelPosition_m1x1(void);

/* $80:8EF7 (RTL): the select screen's wheels turn: a d-pad press sets the
 * turn step $154B ($80:8F29), with sound $40 every 4 frames while it
 * turns; the four wheel angles $1547 advance by it and are placed. */
void TurnSelectWheels_m1x1(void)
{
    lda8(rd8(ABS(0x154B)));
    int turning = !fz;
    if (!turning) {
        lda8(rd8(DP(0x20)));
        and8(0x03);
        if (!fz) {
            X = (u8)A; nz8((u8)X);
            lda8(rd8(ABSX(0x8F29)));
            wr8(ABS(0x154B), (u8)A);
            turning = 1;
        }
    }
    if (turning) {
        lda8(rd8(DP(0x63)));
        and8(0x03);
        if (fz) { lda8(0x40); push8(0x80); push16(0x8F14); QueueSound_m1x1(); }
    }
    ldx8(0x03);
    do {
        lda8(rd8(ABSX(0x1547)));
        fc = 0; adc8(rd8(ABS(0x154B)));
        wr8(ABSX(0x1547), (u8)A);
        push8(0x80); push16(0x8F24); SelectWheelPosition_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    retl();
}

/* $80:EB8E (RTS, X = rider): a slow-down piece: on the ground and moving,
 * $08F4 = 1 when slower than $0200 (speed high below 2 and low below
 * $0240), else 0; then a crash at speed 0, or on a $0200 = 2 piece the
 * jump state cleared ($09C4 / $09C7) and CrashLand. */
void SlowPieceStep_m1x1(void)
{
    lda8(rd8(ABSX(0x01E8)));
    if (!fz) { ret(); return; }
    ldy8(0x00);
    lda8(rd8(ABSX(0x01C0)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x02);
    if (!fc) {
        lda8(rd8(ABSX(0x01B8)));
        cmp8((u8)A, rd8(ABSX(0x0240)));
        if (!fc) Y = inc8((u8)Y);
    }
    lda8((u8)Y);
    wr8(ABSX(0x08F4), (u8)A);
    lda8(rd8(ABSX(0x0200)));
    cmp8((u8)A, 0x02);
    if (fz) {
        lda8(0x00);
        wr8(ABS(0x09C4), 0); wr8(ABS(0x09C7), 0);
        CrashLand_m1x1();
        return;
    }
    lda8(0x00);
    crash_from1();
    ret();
}
