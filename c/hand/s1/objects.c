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
static void show_clock(u16 minutes)
{
    rep(0x20);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    X = rd8(ABS(minutes + 2)); nz8((u8)X);
    lda16(rd16(ABSX(0xC6CE)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC70A))); wr16(ABS(0x03D8), A);
    lda16(rd16(ABS(minutes + 1)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC70A))); wr16(ABS(0x03D5), A);
    sep(0x20);
    lda8(rd8(ABS(minutes))); wr8(ABS(0x03D3), (u8)A);
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

void ShowRaceClock_m1x1(void) { show_clock(0x03B1); }

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

void FindObjectSlot_m1x1(void);

/* $81:B6C2 DriftUpObject (RTS): Stadium 4's $81:B96C with Stadium 1's
 * object arrays (6 bytes lower, slot in $039D, Random $81:A1E5). */
void DriftUpObject_m1x1(void)
{
    X = rd8(ABS(0x039D)); nz8((u8)X);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (fz) {
        rep(0x20);
        lda16(rd16(ABSX(0x0319)));
        fc = 0; adc16(rd16(ABSX(0x0279)));
        wr16(ABSX(0x0279), A);
        wr16(ABSX(0x0329), dec16(rd16(ABSX(0x0329))));
        if (fz) {
            lda16(0x0008); wr16(ABSX(0x0329), 0x0008);
            lda16(rd16(ABSX(0x0319)));
            eor16(0xFFFF);
            lda16(inc16(A));
            wr16(ABSX(0x0319), A);
        }
    }
    sep(0x20);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (!fz) { ret(); return; }
    wr8(ABSX(0x0289), dec8(rd8(ABSX(0x0289))));
    lda8(rd8(ABSX(0x0289)));
    cmp8((u8)A, 0x48);
    if (fc) { ret(); return; }
    rep(0x20);
    lda16(0x00F0);
    wr16(ABSX(0x0279), 0x00F0);
    wr16(ABSX(0x0289), 0x00F0);
    sep(0x20);
    wr8(ABSX(0x0309), inc8(rd8(ABSX(0x0309))));
    lda8(0x09);
    push8(0x81); push16(0xB713); FindObjectSlot_m1x1();
    lda8(0x00); wr8(ABSY(0x02E9), 0x00);
    lda8(0x00);
    push8(0x81); push16(0xB71E); FindObjectSlot_m1x1();
    lda8(0x00); wr8(ABSY(0x02E9), 0x00);
    push8(0x81); push16(0xB727); Random_m1x1();
    and8(0x7F);
    fc = 0; adc8(0x80);
    wr8(ABSX(0x0369), (u8)A);
    ret();
}

/* $80:8EA0 (RTS): the select screen's timer: $88:$89 / 60 (the hardware
 * divider) as two digits ($03D3-$03D6, tiles +$50 / +$60), size $22,
 * at VRAM $1A8F. */
void SelectTimerDigits_m1x1(void)
{
    lda8(rd8(DP(0x88))); wr8(ABS(0x4204), (u8)A);
    lda8(rd8(DP(0x89))); wr8(ABS(0x4205), (u8)A);
    lda8(0x3C); wr8(ABS(0x4206), 0x3C);
    wr8(DP(0x00), 0);
    lda8(rd8(ABS(0x4214)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        wr8(DP(0x00), inc8(rd8(DP(0x00))));
        WAIT_TICK();
    }
    fc = 0; adc8(0x50); wr8(ABS(0x03D4), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(0x03D6), (u8)A);
    lda8(rd8(DP(0x00)));
    fc = 0; adc8(0x50); wr8(ABS(0x03D3), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(0x03D5), (u8)A);
    lda8(0x22); wr8(ABS(0x03D2), 0x22);
    rep(0x20);
    lda16(0x1A8F); wr16(DP(0x00), 0x1A8F);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    push16(0x8EF5); VramQueueTiles20_m1x1();
    ret();
}

/* $80:E059 (RTS): the bonus best: the label $80:E0BC at VRAM $194D and
 * the best count $1E88 as two digits (tens blank $FC when both are 0)
 * at $1959. */
void ShowBonusBest_m1x1(void)
{
    rep(0x20);
    lda16(0x194D); wr16(DP(0x00), 0x194D);
    lda16(0xE0BC); wr16(DP(0x02), 0xE0BC);
    sep(0x20);
    push16(0xE069); VramQueueTiles20_m1x1();
    wr8(DP(0x00), 0);
    lda8(rd8(ABS(0x1E88)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        wr8(DP(0x00), inc8(rd8(DP(0x00))));
        WAIT_TICK();
    }
    wr8(DP(0x01), (u8)A);
    ora8(rd8(DP(0x00)));
    if (!fz) {
        lda8(rd8(DP(0x00)));
        fc = 0; adc8(0x50); wr8(ABS(0x03D3), (u8)A);
        fc = 0; adc8(0x10); wr8(ABS(0x03D5), (u8)A);
    } else {
        lda8(0xFC); wr8(ABS(0x03D3), 0xFC); wr8(ABS(0x03D5), 0xFC);
    }
    lda8(rd8(DP(0x01)));
    fc = 0; adc8(0x50); wr8(ABS(0x03D4), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(0x03D6), (u8)A);
    lda8(0x22); wr8(ABS(0x03D2), 0x22);
    rep(0x20);
    lda16(0x1959); wr16(DP(0x00), 0x1959);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    push16(0xE0BA); VramQueueTiles20_m1x1();
    ret();
}

/* $80:EDEF (RTS): the pace bonus: while racing ($8E, not $84, not the
 * special course $0CD7) count frames $0995 and the player's speed into
 * $0996; each 60 the frame count is ranked against $80:E733 (the last
 * slot below in $00, at or above in $01) and with rivals the bonus
 * $80:E73B / $80:E743 goes to $09B0 / $09B1; past $46 frames, restart. */
void PaceBonus_m1x1(void)
{
    lda8(rd8(ABS(0x0CD7)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x8E)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    wr8(ABS(0x0995), inc8(rd8(ABS(0x0995))));
    lda8(rd8(ABS(0x0995)));
    cmp8((u8)A, 0x46);
    if (!fc) {
        lda8(rd8(ABS(0x0180)));
        fc = 0; adc8(rd8(ABS(0x0996)));
        wr8(ABS(0x0996), (u8)A);
        cmp8((u8)A, 0x3C);
        if (!fc) { ret(); return; }
        lda8(rd8(ABS(0x0996)));
        fc = 1; sbc8(0x3C);
        wr8(ABS(0x0996), (u8)A);
        ldx8(0x07);
        do {
            lda8(rd8(ABS(0x0995)));
            cmp8((u8)A, rd8(ABSX(0xE733)));
            if (!fc) wr8(DP(0x00), (u8)X);
            else wr8(DP(0x01), (u8)X);
            X = dec8((u8)X);
        } while (!fz);
        lda8(rd8(DP(0x7D)));
        if (!fz) {
            X = rd8(DP(0x00)); nz8((u8)X);
            lda8(rd8(ABSX(0xE73B))); wr8(ABS(0x09B0), (u8)A);
            lda8(rd8(ABSX(0xE743))); wr8(ABS(0x09B1), (u8)A);
        }
    }
    lda8(0x00);
    wr8(ABS(0x0995), 0);
    wr8(ABS(0x0996), 0);
    ret();
}

void Nop_BD39_m1x1(void);
void MoveObjectX_m1x1(void);
void MoveObjectX8_m1x1(void);

/* One 8x8 hardware multiply, with the wait call ($80:BD39). */
static void mul8_1(u8 a, u8 b, u16 back)
{
    lda8(a); wr8(ABS(0x4202), (u8)A);
    lda8(b); wr8(ABS(0x4203), (u8)A);
    push16(back); Nop_BD39_m1x1();
}

/* [p] * [q] into r..r+3 (direct page) from four 8x8 products. */
static void mul16_1(u8 p, u8 q, u8 r, const u16 back[4])
{
    mul8_1(rd8(DP(p)), rd8(DP(q)), back[0]);
    lda8(rd8(ABS(0x4216))); wr8(DP(r), (u8)A);
    lda8(rd8(ABS(0x4217))); wr8(DP(r + 1), (u8)A);
    mul8_1(rd8(DP(p)), rd8(DP(q + 1)), back[1]);
    lda8(rd8(ABS(0x4216)));
    fc = 0; adc8(rd8(DP(r + 1))); wr8(DP(r + 1), (u8)A);
    lda8(rd8(ABS(0x4217)));
    adc8(0x00); wr8(DP(r + 2), (u8)A);
    mul8_1(rd8(DP(p + 1)), rd8(DP(q)), back[2]);
    lda8(rd8(ABS(0x4216)));
    fc = 0; adc8(rd8(DP(r + 1))); wr8(DP(r + 1), (u8)A);
    lda8(rd8(ABS(0x4217)));
    adc8(rd8(DP(r + 2))); wr8(DP(r + 2), (u8)A);
    mul8_1(rd8(DP(p + 1)), rd8(DP(q + 1)), back[3]);
    lda8(rd8(ABS(0x4216)));
    fc = 0; adc8(rd8(DP(r + 2))); wr8(DP(r + 2), (u8)A);
    lda8(rd8(ABS(0x4217)));
    adc8(0x00); wr8(DP(r + 3), (u8)A);
}

/* $80:BC61 Mul16 (RTL): $00:$01 * $02:$03 -> $04-$07. */
static void mul16_00(void)
{
    static const u16 back[4] = { 0xBC6D, 0xBC84, 0xBCA0, 0xBCBC };
    mul16_1(0x00, 0x02, 0x04, back);
    retl();
}
void Mul16_m1x1(void) { mul16_00(); }
void Mul16_m1x0(void) { mul16_00(); }

/* $80:BCCD Mul16c (RTS): $40:$41 * $42:$43 -> $44-$47. */
void Mul16c_m1x1(void)
{
    static const u16 back[4] = { 0xBCD9, 0xBCF0, 0xBD0C, 0xBD28 };
    mul16_1(0x40, 0x42, 0x44, back);
    ret();
}

/* $80:C877 ShowBestTime (RTS): ShowRaceClock's display for the best time
 * $083A-$083C. */
void ShowBestTime_m1x1(void) { show_clock(0x083A); }

/* $81:9530 (RTS, Y = piece): Stadium 1's confetti piece (Stadium 4's
 * $81:9604 with the arrays at $0100-$0140 and tiles $81:959B / $81:959F). */
void ConfettiStep_m1x1(void)
{
    X = Y; nz8((u8)X);
    wr8(ABSX(0x0120), dec8(rd8(ABSX(0x0120))));
    if (fz) {
        lda8(rd8(ABSX(0x0118)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(0x0118), (u8)A);
        lda8(0x04); wr8(ABSX(0x0120), 0x04);
    }
    push8(0x81); push16(0x9547); MoveObjectX_m1x1();
    push8((u8)X);
    push8(0x81); push16(0x954C); MoveObjectX8_m1x1();
    X = pull8(); nz8((u8)X);
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
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
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(0x1824), 0x02);
    wr8(ABSX(0x0140), inc8(rd8(ABSX(0x0140))));
    lda8(rd8(ABSX(0x0140)));
    if (fz) {
        lda8(0x03); wr8(ABSX(0x0128), 0x03);
        lda8(0x20); wr8(ABSX(0x0120), 0x20);
        wr8(ABSX(0x0118), 0);
    }
    ret();
}

/* $81:96B4 SpawnObject (RTL, Y = type, $00 / $02 = position): Stadium 4's
 * $81:9788 with Stadium 1's object arrays (6 bytes lower) and tables
 * ($81:AABD scripts, $81:AAD1 init routines). */
void SpawnObject_m1x1(void)
{
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    ldx8(0x0E);
    for (;;) {
        lda8(rd8(ABSX(0x02E9)));
        if (fz) break;
        X = dec8((u8)X);
        X = dec8((u8)X);
        if (fn) {
            X = pull8(); nz8((u8)X);
            DB = pull8(); nz8(DB);
            fc = 1;
            retl();
            return;
        }
    }
    lda8((u8)Y);
    wr8(ABSX(0x02F9), (u8)A);
    lda8(0x01); wr8(ABSX(0x02E9), 0x01);
    rep(0x20);
    lda16(Y);
    lda16(asl16(A));
    Y = (u8)A; nz8((u8)Y);
    lda16(rd16(ABSY(0xAABD))); wr16(ABSX(0x0389), A);
    lda16(rd16(DP(0x00))); wr16(ABSX(0x0279), A);
    lda16(rd16(DP(0x02))); wr16(ABSX(0x0289), A);
    static const u16 clr[11] = { 0x0319, 0x0329, 0x0369, 0x0379, 0x0309, 0x0359,
                                 0x0349, 0x0299, 0x02A9, 0x02B9, 0x02C9 };
    for (int i = 0; i < 11; i++) wr16(ABSX(clr[i]), 0);
    sep(0x20);
    wr8(DP(0x0E), (u8)X);
    X = Y; nz8((u8)X);
    push16(0x970F);
    r816_call_pc(0x810000u | rd16(LONGX(0x81AAD1)));
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    fc = 0;
    retl();
}

/* $81:BE4C (RTS): a podium rider walking in: every 8 frames ($0369) it
 * steps ($0319 0/1) one pixel left; at X $90 (or $95 with $0303 = 4) the
 * pose for the player's place and entry ($81:BEAE by $1542 * 8 + place)
 * becomes its state, with a $40 wait. Its frame $0349 from $81:BEAC. */
void PodiumWalkIn_m1x1(void)
{
    X = rd8(ABS(0x039D)); nz8((u8)X);
    lda8(rd8(ABSX(0x0369)));
    if (fz) {
        lda8(rd8(ABSX(0x0319)));
        eor8(0x01);
        wr8(ABSX(0x0319), (u8)A);
        lda8(0x08); wr8(ABSX(0x0369), 0x08);
        wr8(ABSX(0x0279), dec8(rd8(ABSX(0x0279))));
        lda8(rd8(ABS(0x0303)));
        cmp8((u8)A, 0x04);
        int arrived;
        lda8(rd8(ABSX(0x0279)));
        if (fz) { cmp8((u8)A, 0x95); arrived = !fc; }
        else { cmp8((u8)A, 0x90); arrived = !fc; }
        if (arrived) {
            ldy8(0x0E);
            do {
                lda8(rd8(ABSY(0x086E)));
                lda8(dec8((u8)A));
                if (fz) break;
                Y = dec8((u8)Y);
                Y = dec8((u8)Y);
            } while (!fn);
            lda8((u8)Y);
            lda8(lsr8((u8)A));
            wr8(DP(0x00), (u8)A);
            lda8(rd8(ABS(0x1542)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
            fc = 0; adc8(rd8(DP(0x00)));
            wr8(DP(0x00), (u8)A);
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0xBEAE)));
            wr8(ABSX(0x0309), (u8)A);
            lda8(0x40); wr8(ABSX(0x0369), 0x40);
        }
    }
    Y = rd8(ABSX(0x0319)); nz8((u8)Y);
    lda8(rd8(ABSY(0xBEAC)));
    wr8(ABSX(0x0349), (u8)A);
    ret();
}

/* $81:BD6E / $81:BDDB: Stadium 1's frame sequences (Stadium 4's $81:C018 /
 * $81:C085 with the object arrays 6 bytes lower, slot $039D). */
static void frame_sequence1(u8 count, u16 times, u8 spawn, u16 dy,
                            u16 frames, u16 back)
{
    X = rd8(ABS(0x039D)); nz8((u8)X);
    lda8(rd8(ABSX(0x0369)));
    if (fz) {
        lda8(rd8(ABSX(0x0319)));
        lda8(inc8((u8)A));
        cmp8((u8)A, count);
        if (fc) {
            lda8(0x04); wr8(ABSX(0x0309), 0x04);
            lda8(0x08); wr8(ABSX(0x0369), 0x08);
            wr8(ABSX(0x0319), 0);
            ret();
            return;
        }
        wr8(ABSX(0x0319), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSY(times)));
        wr8(ABSX(0x0369), (u8)A);
        lda8(rd8(ABSX(0x0319)));
        cmp8((u8)A, spawn);
        if (fz) {
            push8((u8)Y);
            rep(0x20);
            lda16(rd16(ABSX(0x0279)));
            fc = 1; sbc16(0x0016);
            wr16(DP(0x00), A);
            lda16(rd16(ABSX(0x0289)));
            fc = 1; sbc16(dy);
            wr16(DP(0x02), A);
            sep(0x20);
            ldy8(0x08);
            push8(0x81); push16(back); SpawnObject_m1x1();
            Y = rd8(DP(0x0E)); nz8((u8)Y);
            lda8(0x01); wr8(ABSY(0x0319), 0x01);
            lda8(0x04); wr8(ABSY(0x0329), 0x04);
            Y = pull8(); nz8((u8)Y);
        }
    }
    Y = rd8(ABSX(0x0319)); nz8((u8)Y);
    lda8(rd8(ABSY(frames)));
    wr8(ABSX(0x0349), (u8)A);
    ret();
}

void FrameSequenceA_m1x1(void) { frame_sequence1(0x04, 0xBDD7, 0x02, 0x001C, 0xBDD3, 0xBDAB); }
void FrameSequenceB_m1x1(void) { frame_sequence1(0x06, 0xBE46, 0x03, 0x0018, 0xBE40, 0xBE18); }

/* $81:B731 ReleaseBalloons (RTS): Stadium 4's $81:B9DB with Stadium 1's
 * object arrays and QueueSound / Random. */
void ReleaseBalloons_m1x1(void)
{
    X = rd8(ABS(0x039D)); nz8((u8)X);
    lda8(rd8(ABS(0x086E)));
    cmp8((u8)A, 0x04);
    if (fc) {
        lda8(rd8(ABSX(0x0339)));
        if (!fz) { ret(); return; }
    } else {
        lda8(rd8(DP(0x8A)));
        if (fz) {
            lda8(rd8(ABSX(0x0339)));
            if (fz) {
                lda8(0x2B);
                push8(0x81); push16(0xB750); QueueSound_m1x1();
            }
        }
    }
    rep(0x20);
    wr16(DP(0x8A), inc16(rd16(DP(0x8A))));
    sep(0x20);
    lda8(rd8(DP(0x8B)));
    and8(0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x0369)));
    if (!fz) { ret(); return; }
    wr8(ABSX(0x0339), inc8(rd8(ABSX(0x0339))));
    push8(0x81); push16(0xB768); Random_m1x1();
    and8(0x3F);
    fc = 0; adc8(0x20);
    wr8(ABSX(0x0369), (u8)A);
    ldy8(0x09);
    push8(0x81); push16(0xB776); SpawnObject_m1x1();
    if (fc) { ret(); return; }
    X = rd8(DP(0x0E)); nz8((u8)X);
    push8(0x81); push16(0xB77E); Random_m1x1();
    and8(0x7F);
    fc = 0; adc8(0x40);
    wr8(ABSX(0x0279), (u8)A);
    lda8(0xD0); wr8(ABSX(0x0289), 0xD0);
    lda8(0x48);
    push8(0x81); push16(0xB791); QueueSound_m1x1();
    push8(0x81); push16(0xB795); Random_m1x1();
    and8(0x0F);
    fc = 0; adc8(0xB6);
    wr8(ABSX(0x02A9), (u8)A);
    X = rd8(ABS(0x039D)); nz8((u8)X);
    ret();
}

/* $80:C7DC RaceClockStep (RTS): the race clock at VRAM $1897, its frames
 * $03B3 counting up unless the race is over ($84); then TickRaceClock. */
void RaceClockStep_m1x1(void)
{
    lda8(0x97); wr8(DP(0x00), 0x97);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(rd8(DP(0x84)));
    if (fz) wr8(ABS(0x03B3), inc8(rd8(ABS(0x03B3))));
    TickRaceClock_m1x1();
}

/* $80:C654 RaceTimer (RTS): Stadium 4's $80:C3E1 for Stadium 1 (racing
 * flag $8E, end minutes $80:C6C6, no frame check): the race time runs
 * and is shown; at this race's end minute or after 4 minutes the race
 * ends ($085E cleared, $84 = 2, sound $2D). */
void RaceTimer_m1x1(void)
{
    lda8(rd8(DP(0x8E)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x82)));
    lda8(inc8((u8)A));
    cmp8((u8)A, 0x3C);
    if (fc) {
        lda8(rd8(DP(0x81)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x3C);
        if (fc) { wr8(DP(0x80), inc8(rd8(DP(0x80)))); lda8(0x00); }
        wr8(DP(0x81), (u8)A);
        lda8(0x00);
    }
    wr8(DP(0x82), (u8)A);
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xC6CE)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC70A))); wr16(ABS(0x09B8), A);
    lda16(rd16(DP(0x81)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC70A))); wr16(ABS(0x09B5), A);
    sep(0x20);
    lda8(rd8(DP(0x80))); wr8(ABS(0x09B3), (u8)A);
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x09B3)));
        wr8(ABSX(0x09BB), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    push16(0xC69C); RaceClockStep_m1x1();
    push8((u8)X);
    lda8(rd8(DP(0x7C)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x7D)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xC6C6)));
    X = pull8(); nz8((u8)X);
    cmp8((u8)A, rd8(LONG(0x7FFFFF)));
    if (fc) {
        lda8(rd8(DP(0x80)));
        cmp8((u8)A, 0x04);
        if (!fc) { ret(); return; }
    }
    wr8(ABS(0x085E), 0); wr8(ABS(0x085F), 0);
    lda8(0x02); wr8(DP(0x84), 0x02);
    lda8(0x2D);
    push8(0x80); push16(0xC6C4); QueueSound_m1x1();
    ret();
}

/* $80:D20F (RTL): a coin collected: sound $31, a marker object in a free
 * slot (or the oldest, $1541 + 1) rising from the player ($1500 = $38,
 * $1508 = height $01B0 + $10, velocity $F8 / $E0), and the coin count
 * $1540 (to 5) adds its points ($80:D291 / $80:D298) into $09AF / $09B0. */
void CoinCollected_m1x1(void)
{
    lda8(0x31);
    push8(0x80); push16(0xD214); QueueSound_m1x1();
    wr8(DP(0x00), 0); wr8(DP(0x01), 0);
    ldx8(0x00);
    for (;;) {
        rep(0x20);
        lda16(rd16(ABSX(0x1538)));
        cmp16(A, rd16(DP(0x00)));
        if (fc) { wr16(DP(0x00), A); wr8(DP(0x02), (u8)X); }
        sep(0x20);
        lda8(rd8(ABSX(0x1530)));
        if (fz) break;
        X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x08);
        if (fc) {
            X = rd8(DP(0x02)); nz8((u8)X);
            wr8(ABS(0x1541), inc8(rd8(ABS(0x1541))));
            break;
        }
    }
    wr8(ABSX(0x1538), 0); wr8(ABSX(0x1539), 0);
    lda8(0x01); wr8(ABSX(0x1530), 0x01);
    rep(0x20);
    lda16(0x0038); wr16(ABSX(0x1500), 0x0038);
    lda16(rd16(ABS(0x01B0)));
    fc = 0; adc16(0x0010);
    wr16(ABSX(0x1508), A);
    lda16(0x00F8); wr16(ABSX(0x1510), 0x00F8);
    lda16(0x00E0); wr16(ABSX(0x1518), 0x00E0);
    wr16(ABSX(0x1520), 0); wr16(ABSX(0x1528), 0);
    sep(0x20);
    lda8(rd8(ABS(0x1540)));
    cmp8((u8)A, 0x05);
    if (fc) ldy8(0x05);
    else {
        wr8(ABS(0x1540), inc8(rd8(ABS(0x1540))));
        Y = rd8(ABS(0x1540)); nz8((u8)Y);
    }
    lda8(rd8(ABSY(0xD291)));
    fc = 0; adc8(rd8(ABS(0x09AF)));
    wr8(ABS(0x09AF), (u8)A);
    lda8(rd8(ABSY(0xD298)));
    fc = 0; adc8(rd8(ABS(0x09B0)));
    wr8(ABS(0x09B0), (u8)A);
    retl();
}

/* The end screen's choice box rows ($89:D900 + lo) to VRAM $0300. */
static void choice_box(u8 lo, u16 back)
{
    lda8(lo); wr8(DP(0x02), lo);
    lda8(0xD9); wr8(DP(0x03), 0xD9);
    lda8(0x89); wr8(DP(0x04), 0x89);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(back); VramQueueCopy_m1x1();
}

/* $80:9B4C (RTS): the end screen's choice: after the $88 delay the box
 * ($89:D900); then A: from step 5 ($0454) to the code step $0A with
 * sound $43; otherwise to step $0454, $88 = $20 and the next box
 * ($89:D980). */
void EndScreenChoice_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) {
        u8 t = dec8(rd8(DP(0x88)));
        wr8(DP(0x88), t);
        if (!fz) { ret(); return; }
        choice_box(0x00, 0x9B72);
    }
    lda8(rd8(DP(0x26)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x0454)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x20); wr8(DP(0x88), 0x20);
        lda8(0x43);
        push8(0x80); push16(0x9B89); QueueSound_m1x1();
        lda8(0x0A); wr8(ABS(0x0453), 0x0A);
        ret();
        return;
    }
    lda8(0x43);
    push8(0x80); push16(0x9B9D); QueueSound_m1x1();
    lda8(rd8(ABS(0x0454))); wr8(ABS(0x0453), (u8)A);
    lda8(0x20); wr8(DP(0x88), 0x20);
    choice_box(0x80, 0x9BC6);
    ret();
}

/* $80:DE6A: the bonus count $10 as two digits (tens blank $FC) at VRAM
 * $18B9 from $03D2 (size $22). */
static void bonus_count1(void)
{
    rep(0x20);
    lda16(0x18B9); wr16(DP(0x00), 0x18B9);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x22); wr8(ABS(0x03D2), 0x22);
    ldy8(0x00);
    lda8(rd8(DP(0x10)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    fc = 0; adc8(0x50); wr8(ABS(0x03D4), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(0x03D6), (u8)A);
    lda8((u8)Y);
    fc = 0; adc8(0x50); wr8(ABS(0x03D3), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(0x03D5), (u8)A);
    cmp8((u8)Y, 0x00);
    if (fz) { lda8(0xFC); wr8(ABS(0x03D3), 0xFC); wr8(ABS(0x03D5), 0xFC); }
    push16(0xDEB1); VramQueueTiles20_m1x1();
    ret();
}

/* $80:DE23 (RTS): the bonus result screen starts: $88 = $10, the label
 * $80:DE54 at VRAM $18B6, the tiles $80:DE59 at $0090 ($0F = 8), the
 * next step and a count of 0. */
void BonusResultStart_m1x1(void)
{
    lda8(0x10); wr8(DP(0x88), 0x10);
    rep(0x20);
    lda16(0x18B6); wr16(DP(0x00), 0x18B6);
    lda16(0xDE54); wr16(DP(0x02), 0xDE54);
    sep(0x20);
    push16(0xDE37); VramQueueTiles20_m1x1();
    rep(0x20);
    lda16(0x0090); wr16(DP(0x00), 0x0090);
    lda16(0xDE59); wr16(DP(0x02), 0xDE59);
    sep(0x20);
    lda8(0x08); wr8(DP(0x0F), 0x08);
    push16(0xDE4C); VramQueueTiles_m1x1();
    wr8(DP(0x77), inc8(rd8(DP(0x77))));
    wr8(DP(0x10), 0);
    bonus_count1();
}

/* $80:DEB3 (RTS): the bonus count-up: every 4 frames ($88) the shown
 * count $09C6 steps toward $09C5 with sound $44; at the end, next step,
 * a $40 wait and for a full score ($09C9) the banner $80:DEF2 at $18F4. */
void BonusCountUp_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x09C6)));
    cmp8((u8)A, rd8(ABS(0x09C5)));
    if (!fz) {
        lda8(inc8((u8)A));
        wr8(ABS(0x09C6), (u8)A);
        wr8(DP(0x10), (u8)A);
        lda8(0x44);
        push8(0x80); push16(0xDECA); QueueSound_m1x1();
        lda8(0x04); wr8(DP(0x88), 0x04);
        bonus_count1();
        return;
    }
    wr8(DP(0x77), inc8(rd8(DP(0x77))));
    lda8(0x40); wr8(DP(0x88), 0x40);
    lda8(rd8(ABS(0x09C5)));
    cmp8((u8)A, rd8(ABS(0x09C9)));
    if (!fz) { ret(); return; }
    rep(0x20);
    lda16(0x18F4); wr16(DP(0x00), 0x18F4);
    lda16(0xDEF2); wr16(DP(0x02), 0xDEF2);
    sep(0x20);
    VramQueueTiles20_m1x1();
}

/* $80:C12C RivalTarget (RTS, X = rival, Y = speed class): rival X's
 * target speed: 3 after the race ($84); a pacing rival ($80:C17F) on
 * screen page 0 shadows the moving player's speed (from X $40 on, or in
 * the player's row $095B, plain), creeping its high byte up to 4; others
 * $80:BDD7 by class; the low byte $80:BDCF by its random $2F. */
void RivalTarget_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    if (!fz) { lda8(0x03); goto set; }
    lda8(rd8(ABSX(0xC17F)));
    if (!fz) {
        lda8(rd8(ABSX(0x0198)));
        lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
        if (!fc) {
            lda8(rd8(ABS(0x01C0)));
            if (!fz) {
                wr8(ABSX(0x0230), (u8)A);
                lda8(rd8(ABS(0x01B8)));
                wr8(ABSX(0x0228), (u8)A);
                lda8(rd8(ABSX(0x0198)));
                if (!fz) {
                    lda8(rd8(ABSX(0x0190)));
                    cmp8((u8)A, 0x40);
                    if (fc) goto plain;
                    lda8(rd8(ABS(0x0958)));
                    cmp8((u8)A, rd8(ABS(0x095B)));
                    if (fz) { ret(); return; }
                }
                lda8(rd8(ABSX(0x0230)));
                cmp8((u8)A, 0x04);
                if (!fc) wr8(ABSX(0x0230), inc8(rd8(ABSX(0x0230))));
                ret();
                return;
            }
        }
    }
plain:
    lda8(rd8(ABSY(0xBDD7)));
set:
    wr8(ABSX(0x0230), (u8)A);
    lda8(rd8(DPX(0x2F)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBDCF)));
    wr8(ABSX(0x0228), (u8)A);
    ret();
}

void ClearRiderState_m1x1(void);

/* $80:C225 RiderLanding (RTS, X = rider): Stadium 4's $80:BF5C for
 * Stadium 1 (no jump-contest rank; addresses 6 lower, windows $80:BD46 /
 * $80:BD56): an airborne rider low enough lands; a bad angle crashes it,
 * a changed one on the flat bounces it; the fall speed becomes the
 * height less one. */
void RiderLanding_m1x1(void)
{
    lda8(rd8(ABSX(0x01E8)));
    if (fz) { ret(); return; }
    push16(0xC22C); GroundHeightAbove_m1x1();
    sbc8(rd8(ABS(0x09C4)));
    wr8(DP(0x01), (u8)A);
    cmp8((u8)A, rd8(ABSX(0x01B0)));
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x0904)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x08AC)));
    if (fz) { ret(); return; }
    lda8((u8)X);
    if (fz) { lda8(0x21); push8(0x80); push16(0xC249); QueueSound_m1x1(); }
    lda8(0x00);
    wr8(ABSX(0x01E8), 0);
    wr8(ABSX(0x08AC), 0);
    lda8(rd8(ABS(0x09C4)));
    wr8(ABS(0x09C7), (u8)A);
    lda8(rd8(ABSX(0x01C8)));
    if (!fz) goto fall;
    lda8(rd8(DP(0x84)));
    if (!fz) {
        lda8(rd8(ABSX(0x01A0)));
        if (!fz) { ret(); return; }
        lda8(0x06); wr8(ABSX(0x01E0), 0x06);
        ret();
        return;
    }
    Y = rd8(ABSX(0x08F4));
    wr8(ABSX(0x08F4), (u8)A);
    lda8((u8)Y);
    if (!fz) goto fall;
    lda8(rd8(ABSX(0x01A0)));
    Y = rd8(ABSX(0x01C0)); nz8((u8)Y);
    cmp8((u8)Y, 0x02);
    {
        int slow = !fc;
        if (!slow && fz) {
            Y = rd8(ABSX(0x01B8)); nz8((u8)Y);
            cmp8((u8)Y, 0x80);
            slow = !fc;
        }
        if (slow) { fc = 0; adc8(0x08); }
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(0x01E0)));
    cmp8((u8)A, rd8(ABSY(0xBD46)));
    if (fc) {
        cmp8((u8)A, rd8(ABSY(0xBD56)));
        if (!fc) {
            lda8(rd8(ABSX(0x08B4)));
            cmp8((u8)A, rd8(ABSX(0x01E0)));
            if (!fz) {
                lda8(rd8(ABSX(0x01A0)));
                ora8(rd8(ABSX(0x0220)));
                if (fz) {
                    lda8(0x04); wr8(ABSX(0x08CC), 0x04);
                    lda8(lsr8((u8)A)); wr8(ABSX(0x08F4), (u8)A);
                    lda8(lsr8((u8)A)); wr8(ABSX(0x01E8), (u8)A);
                }
            }
            goto fall;
        }
    }
    cmp8((u8)X, 0x00);
    if (fz) { lda8(0x3D); push8(0x80); push16(0xC297); QueueSound_m1x1(); }
    lda8(0x0A); wr8(DPX(0x3D), 0x0A);
    lda8(0xFF); wr8(ABSX(0x01C8), 0xFF);
    lda8((u8)X);
    if (!fz) lda8(0x05);
    fc = 0; adc8(0x24);
    push8(0x80); push16(0xC2AC); QueueSound_m1x1();
    ret();
    return;
fall:
    wr8(DP(0x01), dec8(rd8(DP(0x01))));
    lda8(rd8(DP(0x01)));
    wr8(ABSX(0x01B0), (u8)A);
    ret();
}

/* $80:C1C9 (RTS): when the ball is due ($092C) and rider 7 is off screen
 * (not page 1, page 0 left of $EA), it is cleared for reuse
 * (ClearRiderState). */
void LaunchBall_m1x1(void)
{
    lda8(rd8(ABS(0x092C)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x01DB)));
    ldx8(0x07);
    lda8(rd8(ABSX(0x0198)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    lda8(lsr8((u8)A));
    if (!fc) {
        lda8(rd8(ABSX(0x0190)));
        cmp8((u8)A, 0xEA);
        if (fc) { ret(); return; }
    }
    ClearRiderState_m1x1();
}

void AddSpeedFraction_m1x1(void);
void Tick08CC_m1x1(void);
void ScrollByPlayerSpeed_m1x1(void);
void ScrollRivalsRelative_m1x1(void);
void DrawRiderShadowPair_m1x1(void);

/* $80:BF88 RaceMoveStep (RTS): while racing ($8E) every rider moves,
 * lands and ticks; the scrolls; then the player's speed $0180 adds into
 * the distance ($0994 fraction, $8F), flipping $8D on each carry. */
void RaceMoveStep_m1x1(void)
{
    lda8(rd8(DP(0x8E)));
    if (fz) { ret(); return; }
    ldx8(0x07);
    do {
        push16(0xBF90); AddSpeedFraction_m1x1();
        push16(0xBF93); RiderLanding_m1x1();
        push16(0xBF96); RiderLandCheck_m1x1();
        push16(0xBF99); Tick08CC_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    push16(0xBF9F); ScrollByPlayerSpeed_m1x1();
    push16(0xBFA2); ScrollRivalsRelative_m1x1();
    lda8(rd8(ABS(0x0180)));
    lda8(lsr8((u8)A)); lda8(ror8((u8)A)); lda8(ror8((u8)A)); lda8(ror8((u8)A));
    fc = 0; adc8(rd8(ABS(0x0994)));
    wr8(ABS(0x0994), (u8)A);
    lda8(rd8(ABS(0x0180)));
    adc8(rd8(DP(0x8F)));
    wr8(DP(0x8F), (u8)A);
    if (fc) {
        lda8(rd8(DP(0x8D)));
        eor8(0x01);
        wr8(DP(0x8D), (u8)A);
    }
    ret();
}

/* $80:B015 (RTS, X = rival): rival X retargets its speed (class 1 when
 * its random $2E is exactly $C0, else 0) every 16 pixels of X ($0190 bit
 * 4) once $2E reaches $C0; rider 7 always does. */
void RetargetRival_m1x1(void)
{
    ldy8(0x00);
    cmp8((u8)X, 0x07);
    if (!fz) {
        lda8(rd8(ABSX(0x0190)));
        and8(0x10);
        if (fz) { ret(); return; }
        lda8(rd8(DPX(0x2E)));
        cmp8((u8)A, 0xC0);
        if (!fc) { ret(); return; }
        if (fz) Y = inc8((u8)Y);
    }
    push16(0xB02D); RivalTarget_m1x1();
    ret();
}

/* $80:CDF6 (RTS, X = rider): the wheel spark of a moving rider on a ramp
 * ($08BC), on screen page 1 past X $10 and not in the air: the shadow
 * pair sprites, blinking every 4 frames, tile $0C (or the ramp's tile
 * $80:ACF9; 0 hides it) unless high ($01B0 below $58), attribute | $21,
 * and lowered ($F8) when the speed high byte is 0. */
void WheelSpark_m1x1(void)
{
    lda8(rd8(ABSX(0x08BC)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x01C0)));
    ora8(rd8(ABSX(0x01B8)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x01F8)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x0198)));
    lda8(lsr8((u8)A));
    if (!fc) { ret(); return; }
    lda8(rd8(ABSX(0x0190)));
    cmp8((u8)A, 0x10);
    if (!fc) { ret(); return; }
    push16(0xCE17); DrawRiderShadowPair_m1x1();
    lda8(rd8(DP(0x63)));
    and8(0x04);
    if (!fz) {
        lda8(0xF0); wr8(ABSY(0x16C2), 0xF0);
        lda8(0xE0); wr8(ABSY(0x16C6), 0xE0);
    }
    lda8(rd8(ABSX(0x0958)));
    if (fz) {
        lda8(0x0C); wr8(DP(0x02), 0x0C);
    } else {
        push8((u8)Y);
        Y = rd8(ABSX(0x0210)); nz8((u8)Y);
        lda8(rd8(ABSY(0xACF9)));
        Y = pull8(); nz8((u8)Y);
        wr8(DP(0x02), (u8)A);
        cmp8((u8)A, 0x00);
        if (fz) { lda8(0xF0); wr8(ABSY(0x16C1), 0xF0); wr8(ABSY(0x16C5), 0xF0); }
    }
    lda8(rd8(ABSX(0x01B0)));
    cmp8((u8)A, 0x58);
    if (!fc) lda8(0x0C);
    else lda8(rd8(DP(0x02)));
    ora8(0x21);
    wr8(ABSY(0x16C3), (u8)A);
    wr8(ABSY(0x16C7), (u8)A);
    lda8(rd8(ABSX(0x01C0)));
    if (fz) { lda8(0xF8); wr8(ABSY(0x16C5), 0xF8); }
    ret();
}

/* $80:AF94 PieceFlatish (RTS): C clear when the piece type A is $3B-$3D. */
static void piece_flatish(void)
{
    cmp8((u8)A, 0x3B);
    if (fc) {
        cmp8((u8)A, 0x3E);
        if (!fc) { ret(); return; }
    }
    fc = 1;
    ret();
}
void PieceFlatish_m1x1(void) { piece_flatish(); }

/* $80:AF91 PieceOK (RTS, Y = lane): RiderPieceAt, then PieceFlatish. */
void PieceOK_m1x1(void)
{
    push16(0xAF93); RiderPieceAt_m1x1();
    piece_flatish();
}

/* $80:AF4C / $80:AF71: CPU rider X steers a lane up (lane $08A4 - 1;
 * from lane 1 it tries down) or down (+ 1; from lane 4, up): the lane
 * must be flatish (PieceOK) and clear of a rider ahead (row $0958 -/+ 1);
 * then the steering $01A8 = +1 / -1. */
static void steer_lane1(int up)
{
    for (;;) {
        Y = rd8(ABSX(0x08A4)); nz8((u8)Y);
        if (up) {
            Y = dec8((u8)Y);
            cmp8((u8)Y, 0x01);
            if (fn) { up = 0; continue; }
        } else {
            Y = inc8((u8)Y);
            cmp8((u8)Y, 0x05);
            if (!fn) { up = 1; continue; }
        }
        break;
    }
    push16(up ? 0xAF56 : 0xAF7B); PieceOK_m1x1();
    if (fc) { ret(); return; }
    Y = rd8(ABSX(0x0958));
    Y = up ? inc8((u8)Y) : dec8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    ldy8(0x00);
    push16(up ? 0xAF63 : 0xAF88); RiderAhead_m1x1();
    if (fc) { ret(); return; }
    lda8(up ? 0x01 : 0xFF);
    wr8(ABSX(0x01A8), (u8)A);
    ret();
}

void CpuSteerUp_m1x1(void) { steer_lane1(1); }
void CpuSteerDown_m1x1(void) { steer_lane1(0); }

/* $80:AF6A: down, unless rider X's piece $0210 is $E6 (then up). */
static void steer_by_e6_1(void)
{
    lda8(rd8(ABSX(0x0210)));
    cmp8((u8)A, 0xE6);
    steer_lane1(fz);
}

/* $80:AF07 CpuLaneDecision (RTS, X = rider): on screen page 1, blocked by
 * a rider ahead: let go of the buttons and in the right half steer away
 * (by its pad bit 0 / $0210); otherwise, not steering, at X $80-$EF with
 * its random $2D at $E0 or more: up ($F8 on) or by $0210. */
void CpuLaneDecision_m1x1(void)
{
    lda8(rd8(ABSX(0x0198)));
    lda8(lsr8((u8)A));
    if (!fc) { ret(); return; }
    lda8(rd8(ABSX(0x0958)));
    wr8(DP(0x02), (u8)A);
    ldy8(0x02);
    push16(0xAF16); RiderAhead_m1x1();
    if (fc) {
        push16(0xAF1B); CpuReleaseButtons_m1x1();
        lda8(rd8(ABSX(0x0190)));
        lda8(asl8((u8)A));
        if (!fc) { ret(); return; }
        lda8(rd8(DPX(0x2C)));
        lda8(lsr8((u8)A));
        if (fc) steer_by_e6_1();
        else steer_lane1(1);
        return;
    }
    lda8(rd8(ABSX(0x01A8)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x0198)));
    lda8(lsr8((u8)A));
    if (!fc) { ret(); return; }
    lda8(rd8(ABSX(0x0190)));
    cmp8((u8)A, 0xF0);
    if (fc) { ret(); return; }
    lda8(asl8((u8)A));
    if (!fc) { ret(); return; }
    lda8(rd8(DPX(0x2D)));
    cmp8((u8)A, 0xE0);
    if (!fc) { ret(); return; }
    cmp8((u8)A, 0xF8);
    if (!fc) { steer_by_e6_1(); return; }
    steer_lane1(1);
}

/* $80:B02F CpuAvoidHurdle (RTS, X = rider): the eight pieces ahead
 * ($0884 + 4X): a hurdle ($C0/$C1, $70-$73) sends the rider looking for
 * a flatish lane at that column, up (stopping at a $FA piece) then down
 * (stopping at $E4); a ramp top ($48-$4A) sets $0971 = 9; otherwise
 * $0971 = the piece angle $08B4 (0 while landing). Returns A = hurdles. */
void CpuAvoidHurdle_m1x1(void)
{
    lda8(0x00);
    wr8(DP(0x09), 0); wr8(DP(0x0D), 0);
    lda8((u8)X);
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(DP(0x0A), (u8)A);
    ldy8(0x07);
    for (;;) {
        wr8(DP(0x0B), (u8)Y);
        Y = rd8(DP(0x0A)); nz8((u8)Y);
        lda8(rd8(ABSY(0x0884)));
        int hurdle;
        cmp8((u8)A, 0xC0); hurdle = fz;
        if (!hurdle) { cmp8((u8)A, 0xC1); hurdle = fz; }
        if (!hurdle) {
            cmp8((u8)A, 0x70);
            if (fc) { cmp8((u8)A, 0x74); hurdle = !fc; }
        }
        if (hurdle) goto avoid;
        cmp8((u8)A, 0x48);
        if (fc) {
            cmp8((u8)A, 0x4B);
            if (!fc) { lda8(0x09); goto store; }
        }
        wr8(DP(0x0A), inc8(rd8(DP(0x0A))));
        Y = rd8(DP(0x0B)); nz8((u8)Y);
        Y = dec8((u8)Y);
        if (fn) goto angle;
    }
avoid:
    wr8(DP(0x0D), inc8(rd8(DP(0x0D))));
    lda8(rd8(DP(0x0A)));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    Y = inc8((u8)Y);
    lda8((u8)Y);
    fc = 0; adc8(rd8(ABSX(0x0260)));
    and8(0x3F);
    wr8(DP(0x08), (u8)A);
    lda8(0x40); wr8(DP(0x09), 0x40);
    Y = rd8(ABSX(0x08A4)); nz8((u8)Y);
    for (;;) {
        Y = dec8((u8)Y);
        cmp8((u8)Y, 0x01);
        if (fn) break;
        push16(0xB08F); LanePieceAt_m1x1();
        Y = rd8(DP(0x07)); nz8((u8)Y);
        cmp8((u8)A, 0xFA);
        if (fz) break;
        push16(0xB098); PieceFlatish_m1x1();
        if (!fc) { push16(0xB09D); CpuSteerUp_m1x1(); goto angle; }
    }
    Y = rd8(ABSX(0x08A4)); nz8((u8)Y);
    for (;;) {
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x06);
        if (!fn) goto angle;
        push16(0xB0AB); LanePieceAt_m1x1();
        Y = rd8(DP(0x07)); nz8((u8)Y);
        cmp8((u8)A, 0xE4);
        if (fz) goto angle;
        push16(0xB0B4); PieceFlatish_m1x1();
        if (!fc) { push16(0xB0B9); CpuSteerDown_m1x1(); goto angle; }
    }
angle:
    ldy8(0x00);
    lda8(rd8(ABSX(0x0170)));
    if (fz) { Y = rd8(ABSX(0x08B4)); nz8((u8)Y); }
    lda8((u8)Y);
store:
    wr8(ABSX(0x0971), (u8)A);
    lda8(rd8(DP(0x0D)));
    ret();
}

void CpuSteer_m1x1(void);
void CpuThrottle_m1x1(void);

/* $80:AEC6 CpuRidersStep (RTS): the CPU riders 1-7 on the track ($01D8),
 * not crashed or off it: steer; on the ground also throttle, look for
 * hurdles, and with none (when $092A) retarget and (unless $84) decide
 * on a lane; in the air $0971 follows the piece angle $08B4. */
void CpuRidersStep_m1x1(void)
{
    ldx8(0x01);
    do {
        wr8(DP(0x0F), (u8)X);
        lda8(rd8(ABSX(0x01D8)));
        if (fz) goto next;
        lda8(rd8(ABSX(0x01C8)));
        ora8(rd8(ABSX(0x01D0)));
        if (!fz) goto next;
        push16(0xAED9); CpuSteer_m1x1();
        lda8(rd8(ABSX(0x01E8)));
        if (!fz) {
            lda8(rd8(ABSX(0x08B4)));
            wr8(ABSX(0x0971), (u8)A);
            goto next;
        }
        push16(0xAEE1); CpuThrottle_m1x1();
        push16(0xAEE4); CpuAvoidHurdle_m1x1();
        if (!fz) goto next;
        lda8(rd8(ABS(0x092A)));
        if (fz) goto next;
        push16(0xAEEE); RetargetRival_m1x1();
        lda8(rd8(DP(0x84)));
        if (!fz) goto next;
        push16(0xAEF5); CpuLaneDecision_m1x1();
    next:
        X = rd8(DP(0x0F)); nz8((u8)X);
        X = inc8((u8)X);
        cmp8((u8)X, 0x08);
    } while (!fc);
    ret();
}

void HeightAbove_m1x1(void);

/* $80:ECE4 (RTS): every rider's wheel spark; an airborne rider near the
 * screen edges (page 0 from $F6, page 1 from $F4) also gets a marker
 * sprite ($16A0 + 4X, frame $80:E6EE by $0981, attribute $21). */
void DrawSparksAndMarkers_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        int marker;
        lda8(rd8(ABSX(0x0198)));
        lda8(lsr8((u8)A));
        if (fc) {
            lda8(rd8(ABSX(0x0190)));
            cmp8((u8)A, 0xF4);
            marker = !fc;
        } else {
            lda8(lsr8((u8)A));
            marker = 0;
            if (!fc) {
                lda8(rd8(ABSX(0x0190)));
                cmp8((u8)A, 0xF6);
                marker = fc;
            }
        }
        if (marker) {
            lda8(rd8(ABSX(0x01E8)));
            if (!fz) {
                lda8(0xB6);
                push16(0xED0C); HeightAbove_m1x1();
                wr8(ABSY(0x16A1), (u8)A);
                lda8(rd8(ABSX(0x0190)));
                fc = 0; adc8(0x0A);
                wr8(ABSY(0x16A0), (u8)A);
                wr8(DP(0x01), (u8)Y);
                Y = rd8(ABSX(0x0981)); nz8((u8)Y);
                lda8(rd8(ABSY(0xE6EE)));
                Y = rd8(DP(0x01)); nz8((u8)Y);
                wr8(ABSY(0x16A2), (u8)A);
                push8((u8)Y);
                lda8((u8)Y);
                fc = 0; adc8(0xA0);
                lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
                Y = (u8)A; nz8((u8)Y);
                lda8(0x02); wr8(ABSY(0x1820), 0x02);
                Y = pull8(); nz8((u8)Y);
                lda8(0x21); wr8(ABSY(0x16A3), 0x21);
            }
        }
        push16(0xED3D); WheelSpark_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:C09F PlaceRival (RTS, X = rival): clear it, then Stadium 4's
 * $80:BDDC for Stadium 1 (lanes $80:BDDA, columns $80:BDE8, Random). */
void PlaceRival_m1x1(void)
{
    push16(0xC0A1); ClearRiderState_m1x1();
    push8(0x80); push16(0xC0A5); Random_m1x1();
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBDDA)));
    wr8(ABSX(0x01F0), (u8)A);
    Y = rd8(ABSX(0x0198)); nz8((u8)Y);
    lda8(fz ? 0xA0 : 0x40);
    wr8(ABSX(0x0190), (u8)A);
    lda8(0x06); wr8(ABSX(0x01E0), 0x06);
    lda8(rd8(ABS(0x0260)));
    fc = 0; adc8(rd8(ABSY(0xBDE8)));
    and8(0x3F);
    wr8(ABSX(0x0260), (u8)A);
    lda8(rd8(ABS(0x0188)));
    wr8(ABSX(0x0188), (u8)A);
    lda8((u8)Y);
    if (!fz) {
        ldy8(0x05);
        push16(0xC0DB); RiderPieceAt_m1x1();
        lda8(rd8(DP(0x10)));
        cmp8((u8)A, 0xE4);
        if (!fz) {
            cmp8((u8)A, 0x3B);
            if (!fc) { lda8(0x00); ret(); return; }
            cmp8((u8)A, 0x93);
            if (fc) { lda8(0x00); ret(); return; }
            ldy8(0x02);
            push16(0xC0EE); RiderPieceAt_m1x1();
            cmp8((u8)A, 0xE6);
            if (fz) {
                lda8(rd8(ABSX(0x01F0)));
                cmp8((u8)A, 0x20);
                if (fc) {
                    lda8(0x30); wr8(ABSX(0x01F8), 0x30);
                    lda8(0x01);
                } else lda8(0x03);
                wr8(ABSX(0x0208), (u8)A);
            }
        }
    }
    Y = rd8(ABSX(0x0198)); nz8((u8)Y);
    push16(0xC10D); RivalTarget_m1x1();
    push8(0x80); push16(0xC111); Random_m1x1();
    and8(0x3F);
    if (fz) {
        lda8(rd8(ABSX(0x0198)));
        eor8(0x02);
        lda8(asl8((u8)A));
        wr8(ABSX(0x01C0), (u8)A);
    }
    lda8(0x10); wr8(DP(0x3B), 0x10);
    lda8(0x01); wr8(ABSX(0x01D8), 0x01);
    ret();
}

/* $80:C012 SpawnRivals (RTS): distances, then every rival slot 7-1 not
 * on the track whose distance $085E is $170 or more ahead of the
 * player's $94, or up to $A0 behind it, gets placed (unless $84). */
void SpawnRivals_m1x1(void)
{
    push16(0xC014); UpdateRiderDistances_m1x1();
    ldy8(0x0E);
    ldx8(0x07);
    do {
        lda8(rd8(ABSX(0x01D8)));
        if (fz) {
            rep(0x20);
            lda16(rd16(DP(0x94)));
            fc = 0; adc16(0x0170);
            cmp16(A, rd16(ABSY(0x085E)));
            int place = fc;
            if (!place) {
                lda16(rd16(DP(0x94)));
                fc = 1; sbc16(0x00A0);
                if (!fn) {
                    cmp16(A, rd16(ABSY(0x085E)));
                    place = fc;
                }
            }
            if (place) {
                sep(0x20);
                lda8(rd8(DP(0x84)));
                if (fz) {
                    push8((u8)Y);
                    push16(0xC041); PlaceRival_m1x1();
                    Y = pull8(); nz8((u8)Y);
                }
            }
        }
        sep(0x20);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fz);
    ret();
}

/* $80:BEB7 (RTS, X = rider): a rider walking back onto the track ($08C4)
 * toward its run-out point $090C: every 16 frames (for the player, at
 * once on a new button press, $79) the point moves back; every 8 frames
 * the walk pose $091C steps 1 / 3 / 4 / 0 with sound $23 at 3 and on
 * the wrap. On reaching it (or with no walk), $56,X = 3, the next start
 * step ($01D0) and $099E cleared (rider 7 also clears the ball). */
void WalkBackOn_m1x1(void)
{
    lda8(rd8(ABSX(0x08C4)));
    if (fz) goto arrived;
    lda8(rd8(DPX(0x45)));
    if (!fz) { ret(); return; }
    lda8((u8)X);
    int step;
    if (fz) {
        Y = rd8(DP(0x79)); nz8((u8)Y);
        lda8(rd8(ABS(0x0178)));
        and8(0xC0);
        if (!fz) {
            wr8(DP(0x79), (u8)A);
            lda8((u8)Y);
            if (fz) { step = 1; goto move; }
        }
        wr8(DP(0x79), (u8)A);
    }
    lda8(rd8(DP(0x63)));
    and8(0x0F);
    step = fz;
move:
    if (step) wr8(ABSX(0x090C), dec8(rd8(ABSX(0x090C))));
    and8(0x07);
    if (fz) {
        lda8(rd8(ABSX(0x091C)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x02);
        if (fz) lda8(inc8((u8)A));
        cmp8((u8)A, 0x05);
        if (fz) {
            lda8(0x23);
            push8(0x80); push16(0xBF01); QueueSound_m1x1();
            lda8(0x00);
            wr8(ABSX(0x091C), 0x00);
        } else {
            cmp8((u8)A, 0x03);
            if (fz) {
                wr8(ABSX(0x091C), (u8)A);
                lda8(0x23);
                push8(0x80); push16(0xBEF9); QueueSound_m1x1();
            } else wr8(ABSX(0x091C), (u8)A);
        }
    }
    lda8(rd8(ABSX(0x090C)));
    fc = 1; sbc8(0x08);
    cmp8((u8)A, rd8(ABSX(0x0190)));
    if (fc) { ret(); return; }
arrived:
    lda8(0x03); wr8(DPX(0x56), 0x03);
    wr8(ABSX(0x01D0), inc8(rd8(ABSX(0x01D0))));
    lda8(0x00); wr8(ABSX(0x099E), 0x00);
    cmp8((u8)X, 0x07);
    if (fz) { wr8(ABS(0x092C), 0); wr8(ABS(0x08C7), 0); }
    ret();
}

void Add32_m1x1(void);

/* $80:DA99 (RTS): the player's track column $0260: a new column in the
 * air adds its coin value $0A93 into $09C5; on the ground the height
 * $09C4 follows $0A13; the piece $09D3 goes to $0210 (below $28 ends the
 * race once: $099C, $84 = 1, sound $2B); a coin $0A53 in the air is
 * collected as piece $45, on the ground (not a $40 piece) taken off the
 * track (scenery rows $3B, $0218); a $84 piece reads $FC on the ground. */
void PlayerTileState_m1x1(void)
{
    X = rd8(ABS(0x0260)); nz8((u8)X);
    lda8(rd8(ABS(0x09C7)));
    ora8(rd8(ABS(0x01E8)));
    if (!fz) {
        lda8(rd8(ABS(0x0260)));
        cmp8((u8)A, rd8(ABS(0x09C8)));
        if (!fz) {
            wr8(ABS(0x09C8), (u8)A);
            lda8(rd8(ABSX(0x0A93)));
            fc = 0; adc8(rd8(ABS(0x09C5)));
            wr8(ABS(0x09C5), (u8)A);
        }
    }
    lda8(rd8(ABS(0x01C8)));
    if (fz) { lda8(rd8(ABSX(0x0A13))); wr8(ABS(0x09C4), (u8)A); }
    lda8(rd8(ABSX(0x09D3)));
    wr8(ABS(0x0210), (u8)A);
    cmp8((u8)A, 0x28);
    if (!fc) {
        lda8(rd8(ABS(0x099C)));
        if (fz) {
            lda8(0x01); wr8(ABS(0x099C), 0x01); wr8(DP(0x84), 0x01);
            lda8(0x2B);
            push8(0x80); push16(0xDADF); QueueSound_m1x1();
        }
        ret();
        return;
    }
    lda8(rd8(ABSX(0x0A53)));
    if (!fz) {
        lda8(rd8(ABS(0x09C7)));
        ora8(rd8(ABS(0x01E8)));
        if (!fz) { lda8(0x45); wr8(ABS(0x0210), 0x45); ret(); return; }
        lda8(rd8(ABSX(0x09D3)));
        and8(0x40);
        if (!fz) { ret(); return; }
        wr8(ABSX(0x0A53), 0);
        lda8(0x3B);
        wr8(ABSX(0x0B53), 0x3B); wr8(ABSX(0x0B93), 0x3B); wr8(ABSX(0x0BD3), 0x3B);
        wr8(ABSX(0x0C13), 0x3B); wr8(ABSX(0x0C53), 0x3B); wr8(ABSX(0x0C93), 0x3B);
        wr8(ABS(0x0218), 0x3B);
    }
    lda8(rd8(ABSX(0x09D3)));
    cmp8((u8)A, 0x84);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x09C7)));
    if (!fz) { ret(); return; }
    lda8(0xFC); wr8(ABS(0x0210), 0xFC);
    ret();
}

/* $80:A231 (RTL): the entry digits $03A6-$03A9 as a number (x1000, x100,
 * x10, +1 via Mul16 and Add32 into $10-$13), packed into the entry
 * record's BCD nibbles ($085A low, $0859, $0858, $0855 bits 4-5). */
void PackEntryNumber_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x10), 0); wr16(DP(0x12), 0);
    static const u16 mul[3] = { 0x03E8, 0x0064, 0x000A };
    static const u16 back[3][2] = { { 0xA249, 0xA24C }, { 0xA261, 0xA264 }, { 0xA279, 0xA27C } };
    for (int i = 0; i < 3; i++) {
        if (i) rep(0x20);
        lda16(rd16(ABS(0x03A6 + i)));
        and16(0x00FF);
        wr16(DP(0x00), A);
        lda16(mul[i]); wr16(DP(0x02), mul[i]);
        sep(0x20);
        push8(0x80); push16(back[i][0]); Mul16_m1x1();
        push16(back[i][1]); Add32_m1x1();
    }
    lda8(rd8(ABS(0x03A9))); wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA28A); Add32_m1x1();
    lda8(rd8(DP(0x10)));
    and8(0x0F);
    ora8(rd8(ABS(0x085A)));
    wr8(ABS(0x085A), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(0x0859), (u8)A);
    lda8(rd8(DP(0x11)));
    and8(0x0F);
    wr8(ABS(0x0858), (u8)A);
    lda8(rd8(DP(0x11)));
    and8(0x30);
    ora8(rd8(ABS(0x0855)));
    wr8(ABS(0x0855), (u8)A);
    retl();
}

/* $81:99CB (RTL): every 8 frames the palette entries from $B1 down
 * (skipping each group's colour 0) step one unit per channel toward the
 * targets $81:9A45 (blue), $81:9A5D (green), $81:9A75 (red). */
void FadePaletteStep_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x07);
    if (fz) {
        X = rd8(DP(0xB1)); nz8((u8)X);
        rep(0x20);
        static const u16 mask[3] = { 0x7C00, 0x03E0, 0x001F };
        static const u16 tgt[3] = { 0x9A45, 0x9A5D, 0x9A75 };
        static const u16 unit[3] = { 0x0400, 0x0020, 0x0001 };
        do {
            lda16((u16)X);
            and16(0x0007);
            if (!fz) {
                for (int c = 0; c < 3; c++) {
                    lda16(rd16(ABSX(0x1C00)));
                    and16(mask[c]);
                    cmp16(A, rd16(ABSX(tgt[c])));
                    if (fz) continue;
                    if (!fc) { lda16(rd16(ABSX(0x1C00))); fc = 0; adc16(unit[c]); }
                    else { lda16(rd16(ABSX(0x1C00))); fc = 1; sbc16(unit[c]); }
                    wr16(ABSX(0x1C00), A);
                }
            }
            X = dec8((u8)X);
            X = dec8((u8)X);
        } while (!fn);
        sep(0x20);
    }
    wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
    retl();
}

/* $80:A1BC (RTL): the record time at $11:572C-$11:572E (minutes x6000,
 * seconds x100, hundredths) as a number into $10-$13, packed as BCD
 * nibbles into $0854-$0857 and $085A bits 4-5 (with $2C bit 1). */
void PackRecordTime_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x10), 0); wr16(DP(0x12), 0);
    static const u16 mul[2] = { 0x1770, 0x0064 };
    static const u16 back[2][2] = { { 0xA1D5, 0xA1D8 }, { 0xA1EE, 0xA1F1 } };
    for (int i = 0; i < 2; i++) {
        if (i) rep(0x20);
        lda16(rd16(LONG(0x11572C + i)));
        and16(0x00FF);
        wr16(DP(0x00), A);
        lda16(mul[i]); wr16(DP(0x02), mul[i]);
        sep(0x20);
        push8(0x80); push16(back[i][0]); Mul16_m1x1();
        push16(back[i][1]); Add32_m1x1();
    }
    lda8(rd8(LONG(0x11572E))); wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA200); Add32_m1x1();
    lda8(rd8(DP(0x11)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(0x0854), (u8)A);
    lda8(rd8(DP(0x11))); and8(0x0F); wr8(ABS(0x0855), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(0x0856), (u8)A);
    lda8(rd8(DP(0x10))); and8(0x0F); wr8(ABS(0x0857), (u8)A);
    lda8(rd8(DP(0x2C)));
    and8(0x02);
    ora8(rd8(DP(0x12)));
    and8(0x03);
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(ABS(0x085A), (u8)A);
    retl();
}

/* $80:9053 (RTL, X = row): a six-sprite text row at OAM $1700: X $68 on
 * in steps of 8, Y $82, tiles $60 + 16X on, attribute $21, small. */
void SpriteTextRow_m1x1(void)
{
    lda8(0x68);
    for (int i = 0; i < 6; i++) {
        if (i) { fc = 0; adc8(0x08); }
        wr8(ABS(0x1700 + 4 * i), (u8)A);
    }
    lda8(0x82);
    for (int i = 0; i < 6; i++) wr8(ABS(0x1701 + 4 * i), 0x82);
    lda8((u8)X);
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    fc = 0; adc8(0x60);
    for (int i = 0; i < 6; i++) {
        if (i) lda8(inc8((u8)A));
        wr8(ABS(0x1702 + 4 * i), (u8)A);
    }
    lda8(0x21);
    for (int i = 0; i < 6; i++) wr8(ABS(0x1703 + 4 * i), 0x21);
    for (int i = 0; i < 6; i++) wr8(ABS(0x1860 + i), 0);
    retl();
}

/* $80:C947 on: the best time at VRAM $18D7, then the tail below. */
static void record_time_tail1(void)
{
    lda8(rd8(DP(0x84)));
    if (fz) { wr8(ABS(0x03B1), 0); wr8(ABS(0x03B2), 0); wr8(ABS(0x03B3), 0); }
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
}

static void show_best_tail1(void)
{
    lda8(0xD7); wr8(DP(0x00), 0xD7);
    lda8(0x18); wr8(DP(0x01), 0x18);
    push16(0xC951); ShowBestTime_m1x1();
    record_time_tail1();
}

/* $80:C8EE RecordCourseTime: the race time $03B1-$03B3 into the stadium's
 * slot $081C + 3 * (stadium - 2); a new best ($083A-$083C) is kept with
 * its stadium $084C and shown, else the best is shown (VRAM $18D7);
 * before the race end the clock restarts. X and Y are kept. */
static void record_course_time(void)
{
    push8((u8)X); push8((u8)Y);
    lda8(rd8(ABS(0x0928)));
    lda8(dec8((u8)A)); lda8(dec8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x00)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(0x03B1))); wr8(ABSX(0x081C), (u8)A);
    lda8(rd8(ABS(0x03B2))); wr8(ABSX(0x081D), (u8)A);
    lda8(rd8(ABS(0x03B3))); wr8(ABSX(0x081E), (u8)A);
    int better = 0;
    lda8(rd8(ABS(0x083A)));
    cmp8((u8)A, rd8(ABS(0x03B1)));
    if (fc) {
        if (!fz) better = 1;
        else {
            lda8(rd8(ABS(0x083B)));
            cmp8((u8)A, rd8(ABS(0x03B2)));
            if (fc) {
                if (!fz) better = 1;
                else {
                    lda8(rd8(ABS(0x083C)));
                    cmp8((u8)A, rd8(ABS(0x03B3)));
                    better = fc && !fz;
                }
            }
        }
    }
    if (better) {
        lda8(rd8(ABS(0x03B1))); wr8(ABS(0x083A), (u8)A);
        lda8(rd8(ABS(0x03B2))); wr8(ABS(0x083B), (u8)A);
        lda8(rd8(ABS(0x03B3))); wr8(ABS(0x083C), (u8)A);
        lda8(rd8(ABS(0x0928)));
        lda8(dec8((u8)A));
        wr8(ABS(0x084C), (u8)A);
        lda8(0xD7); wr8(DP(0x00), 0xD7);
        lda8(0x18); wr8(DP(0x01), 0x18);
        push16(0xC95E); ShowRaceClock_m1x1();
        record_time_tail1();
    } else {
        show_best_tail1();
    }
}

/* (RTL) */
void RecordCourseTime_m1x1(void) { record_course_time(); retl(); }

/* $80:C8E9 (RTL): show the best time at VRAM $18D7 (and before the race
 * end restart the clock), keeping X and Y. */
void ShowCourseBest_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    show_best_tail1();
    retl();
}

void FadePaletteStep_m1x1(void);

/* $81:982B (RTS): object $039D fades the palette in steps: every $C0
 * frames ($88) the fade limit $B1 grows by 8, until $16, then its state
 * steps; each frame FadePaletteStep. */
void FadeInObject_m1x1(void)
{
    X = rd8(ABS(0x039D)); nz8((u8)X);
    lda8(rd8(DP(0x88)));
    if (fz) {
        lda8(rd8(DP(0xB1)));
        cmp8((u8)A, 0x16);
        if (fc) {
            wr8(ABSX(0x0309), inc8(rd8(ABSX(0x0309))));
            X = rd8(ABS(0x039D)); nz8((u8)X);
            ret();
            return;
        }
        fc = 0; adc8(0x08);
        wr8(DP(0xB1), (u8)A);
        lda8(0xC0); wr8(DP(0x88), 0xC0);
    }
    wr8(DP(0x88), dec8(rd8(DP(0x88))));
    push8(0x81); push16(0x984D); FadePaletteStep_m1x1();
    X = rd8(ABS(0x039D)); nz8((u8)X);
    ret();
}

/* $80:D906 (RTS): the bonus label "BONUS" plus the count $AC as two
 * digits ($03D2, size $27, $0F = $20) at VRAM $184D. */
void ShowBonusLabel_m1x1(void)
{
    rep(0x20);
    lda16(0x184D); wr16(DP(0x00), 0x184D);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x27); wr8(ABS(0x03D2), 0x27);
    static const u8 t[5] = { 0x7D, 0x72, 0x76, 0x5E, 0x96 };
    for (int i = 0; i < 5; i++) {
        lda8(t[i]); wr8(ABS(0x03D3 + i), t[i]);
        fc = 0; adc8(0x10); wr8(ABS(0x03DA + i), (u8)A);
    }
    ldy8(0x00);
    lda8(rd8(DP(0xAC)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    fc = 0; adc8(0x50); wr8(ABS(0x03D9), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(0x03E0), (u8)A);
    lda8((u8)Y);
    fc = 0; adc8(0x50); wr8(ABS(0x03D8), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(0x03DF), (u8)A);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xD97C); VramQueueTiles_m1x1();
    ret();
}

/* $80:CD54 RiderPose (RTS, X = rider): A = rider X's pose. Starting or
 * off the track ($01D0 not 0/5): $12 / $16 (walking, $099E) less one on
 * a step ($56,X), | $10. After the race on angle $0A: $0E = 2. Upright
 * on the ground: still ($0E + 1), steering $14 / $15 by $01A8, or
 * blinking $13 every 2 frames; otherwise its angle $01E0. */
void RiderPose_m1x1(void)
{
    lda8(0x00); wr8(DP(0x0E), 0x00);
    lda8(rd8(ABSX(0x01D0)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) {
            ldy8(0x06);
            lda8(rd8(ABSX(0x099E)));
            if (fz) ldy8(0x02);
            lda8((u8)Y);
            Y = rd8(DPX(0x56)); nz8((u8)Y);
            if (!fz) { Y = dec8((u8)Y); lda8((u8)Y); }
            ora8(0x10);
            ret();
            return;
        }
    }
    Y = rd8(DP(0x84)); nz8((u8)Y);
    Y = dec8((u8)Y);
    if (fz) {
        lda8(rd8(ABSX(0x01E0)));
        cmp8((u8)A, 0x0A);
        if (fz) { ldy8(0x02); wr8(DP(0x0E), 0x02); ret(); return; }
    }
    lda8(rd8(ABSX(0x01C8)));
    ora8(rd8(ABSX(0x01E8)));
    if (!fz) goto angle;
    lda8(rd8(ABSX(0x0170)));
    if (!fz) {
        cmp8((u8)A, 0x0D);
        if (!fz) goto angle;
        lda8(rd8(ABSX(0x01F0)));
        cmp8((u8)A, 0x20);
    }
    lda8(rd8(ABSX(0x01B8)));
    ora8(rd8(ABSX(0x01C0)));
    if (fz) {
        wr8(DP(0x0E), inc8(rd8(DP(0x0E))));
        goto angle;
    }
    lda8(rd8(ABSX(0x01E0)));
    cmp8((u8)A, 0x06);
    if (!fz) goto angle;
    ldy8(0x14);
    lda8(rd8(ABSX(0x01A8)));
    if (!fz) {
        if (!fn) Y = inc8((u8)Y);
        lda8((u8)Y);
        ret();
        return;
    }
    lda8(rd8(DP(0x63)));
    and8(0x02);
    if (!fz) { lda8(0x13); ret(); return; }
angle:
    lda8(rd8(ABSX(0x01E0)));
    ret();
}

void RiderShadowY_m1x1(void);

/* $80:C48B: CrashRecovery's rider loop (riders 7-0); entered at $80:C4EF
 * (the lean turn) for the current X by the player's crash steps. */
static void crash_loop1(int enter_turn)
{
    if (enter_turn) goto turn;
    ldx8(0x07);
    for (;;) {
        lda8(rd8(ABSX(0x01C8)));
        if (!fz) {
            ldy8(0x04);
            lda8((u8)X);
            ora8(rd8(ABS(0x0CD7)));
            if (fz) {
                wr8(ABS(0x092D), (u8)A);
                lda8(rd8(DP(0x7F)));
                if (fz) { lda8(0x25); push8(0x80); push16(0xC4A4); QueueSound_m1x1(); }
                lda8(rd8(DP(0x2F)));
                and8(0x03);
                Y = (u8)A; nz8((u8)Y);
            }
            lda8((u8)Y);
            wr8(ABSX(0x0948), (u8)A);
            lda8(rd8(ABSX(0x01C0)));
            if (!fz) goto turn;
            lda8(rd8(ABSX(0x01E8)));
            ora8(rd8(ABSX(0x0170)));
            if (!fz) goto slide;
            wr8(ABSX(0x01B8), (u8)A);
            wr8(ABSX(0x0180), (u8)A);
            wr8(ABSX(0x01C8), (u8)A);
            lda8(0x06); wr8(ABSX(0x01E0), 0x06);
            lda8(rd8(ABSX(0x0190)));
            fc = 0; adc8(0x08);
            wr8(ABSX(0x090C), (u8)A);
            push16(0xC4D4); RiderShadowY_m1x1();
            lda8(0x02); wr8(ABSX(0x091C), 0x02);
            lda8(lsr8((u8)A));
            wr8(ABSX(0x01D0), (u8)A);
            cmp8((u8)X, 0x00);
            if (fz) { lda8(0x3E); push8(0x80); push16(0xC4E7); QueueSound_m1x1(); }
            goto next;
        slide:
            lda8(0x88); wr8(ABSX(0x01B8), 0x88);
        turn:
            lda8(rd8(DPX(0x3D)));
            if (fz) {
                lda8(rd8(ABSX(0x01E0)));
                fc = 0; adc8(rd8(ABSX(0x01C8)));
                and8(0x0F);
                wr8(ABSX(0x01E0), (u8)A);
            }
        }
    next:
        X = dec8((u8)X);
        if (fn) break;
    }
    ret();
}

/* $80:C475 CrashRecovery (RTS): Stadium 4's $80:C1F3 for Stadium 1 (no
 * tile $14 check): every 4 frames each crashed rider picks a lane, gets
 * back up when stopped, slides at $88 while tumbling, else turns. */
void CrashRecovery_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x03);
    cmp8((u8)A, rd8(ABS(0x0998)));
    if (!fz) { ret(); return; }
    wr8(ABS(0x0998), dec8(rd8(ABS(0x0998))));
    lda8(rd8(ABS(0x0998)));
    and8(0x03);
    wr8(ABS(0x0998), (u8)A);
    crash_loop1(0);
}

/* $80:DD0E PlayerCrashStep (RTS): Stadium 4's $80:DBFC for Stadium 1
 * (crashed $01C8, air $01E8, speed $01C0:$01B8, $0170): getting up,
 * sliding (CrashRecovery's lean from $80:C4EF), or ending the tumble
 * with PlayerHalt and the crash landing. */
void PlayerCrashStep_m1x1(void)
{
    lda8(rd8(ABS(0x01C8)));
    if (fz) { ret(); return; }
    lda8(0x00); wr8(ABS(0x092D), 0x00);
    lda8(rd8(DP(0x7F)));
    if (fz) lda8(0x01);
    lda8(rd8(ABS(0x0999)));
    if (!fz) {
        lda8(rd8(ABS(0x01E8)));
        if (fz) {
            lda8(rd8(ABS(0x01C0)));
            if (fz) {
                lda8(rd8(ABS(0x0170)));
                int up = 0;
                if (fz) {
                    lda8(rd8(ABS(0x01E0)));
                    cmp8((u8)A, 0x06);
                    up = fz;
                }
                if (up) {
                    lda8(0x00);
                    wr8(ABS(0x01B8), 0); wr8(ABS(0x01C8), 0); wr8(ABS(0x0999), 0);
                    ret();
                    return;
                }
                lda8(0x88); wr8(ABS(0x01B8), 0x88);
            }
        }
        goto slide;
    }
    lda8(0x00);
    wr8(ABS(0x01B8), 0); wr8(ABS(0x01C0), 0); wr8(ABS(0x0180), 0);
    lda8(rd8(DP(0x3A)));
    if (fz) { lda8(0x20); wr8(DP(0x3A), 0x20); }
    cmp8((u8)A, 0x1C);
    if (fc) { ret(); return; }
    cmp8((u8)A, 0x01);
    if (!fz) goto slide;
    wr8(ABS(0x0999), (u8)A);
    lda8(lsr8((u8)A));
    wr8(ABS(0x09C4), (u8)A);
    lda8(rd8(ABS(0x0170)));
    if (!fz) { ret(); return; }
    push16(0xDD7E); player_halt1();
    push16(0xDDD5); CrashLand_m1x1();
    lda8(rd8(ABS(0x08D4))); wr8(ABS(0x08E4), (u8)A);
    lda8(rd8(ABS(0x08DC))); wr8(ABS(0x08EC), (u8)A);
    ret();
    return;
slide:
    lda8(rd8(DP(0x3B)));
    if (!fz) { ret(); return; }
    lda8(0x02); wr8(DP(0x3B), 0x02);
    crash_loop1(1);
}

void DivideAX_m1x1(void);
void DrawBigSprite_m1x1(void);

/* $80:DB6F (RTS): the bonus time: every 60 frames ($AD) the count $AC
 * goes down; at 0 the run ends ($84 = 2, sound $2D). Then the label. */
void BonusTimerStep_m1x1(void)
{
    wr8(DP(0xAD), dec8(rd8(DP(0xAD))));
    if (fz) {
        lda8(0x3C); wr8(DP(0xAD), 0x3C);
        wr8(DP(0xAC), dec8(rd8(DP(0xAC))));
        if (fz) {
            lda8(0x02); wr8(DP(0x84), 0x02);
            lda8(0x2D);
            push8(0x80); push16(0xDB84); QueueSound_m1x1();
        }
    }
    push16(0xDB87); ShowBonusLabel_m1x1();
    ret();
}

/* $80:CCE1 DrawRider (RTS, X = rider): unless held off the top ($0904
 * with Y $EC+), its pose (RiderPose, $A0) picks the frame ($80:BD6C) and
 * the slot's pose $1961 ($80:CD34; the winner's wave $14 after the race),
 * then the big sprite at (X, height - 8). */
void DrawRider_m1x1(void)
{
    lda8(rd8(ABSX(0x0190))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(0x01B0)));
    fc = 1; sbc8(0x08);
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABSX(0x0904)));
    if (!fz) {
        lda8(rd8(DP(0x01)));
        cmp8((u8)A, 0xEC);
        if (!fc) { ret(); return; }
    }
    push16(0xCCFB); RiderPose_m1x1();
    wr8(DP(0xA0), (u8)A);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    rep(0x20);
    lda16(rd16(ABSY(0xBD6C))); wr16(DP(0x02), A);
    sep(0x20);
    Y = rd8(DP(0xA0)); nz8((u8)Y);
    push8((u8)X);
    X = rd8(DP(0x0D)); nz8((u8)X);
    lda8(rd8(ABSX(0x0950)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSY(0xCD34)));
    wr8(ABSX(0x1961), (u8)A);
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(rd8(ABS(0x01C8)));
        ora8(rd8(ABS(0x01E8)));
        if (fz) {
            lda8(rd8(DP(0x84)));
            cmp8((u8)A, 0x01);
            if (fz) { lda8(0x14); wr8(ABSX(0x1961), 0x14); }
        }
    }
    X = pull8(); nz8((u8)X);
    push16(0xCD32); DrawBigSprite_m1x1();
    ret();
}

/* $80:C604 SpeedGauge (RTS): X = the gauge reading for the player while
 * $092D is set and it is not the CPU's turn: speed ($01C0:$01B8 >> 3)
 * - 2 divided by 2 (4 in the air), at least 1, at most $2F (blinking
 * $2E every 8 frames), + $80 unless a button is held. */
void SpeedGauge_m1x1(void)
{
    ldx8(0x00);
    lda8(rd8(ABS(0x092D)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x7F)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    ldx8(0x02);
    lda8(rd8(ABS(0x01C0))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABS(0x01B8))); wr8(DP(0x01), (u8)A);
    do {
        u8 h = rd8(DP(0x00));
        fc = h & 1; h >>= 1; wr8(DP(0x00), h); nz8(h);
        u8 l = rd8(DP(0x01));
        u8 c = fc; fc = l & 1; l = (u8)((l >> 1) | (c << 7)); wr8(DP(0x01), l); nz8(l);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x02);
    Y = rd8(ABS(0x01E8)); nz8((u8)Y);
    if (!fz) lda8(0x04);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(DP(0x01)));
    fc = 1; sbc8(0x02);
    int one = fn;
    if (!one) {
        push16(0xC636); DivideAX_m1x1();
        lda8((u8)X);
        one = fz;
    }
    if (one) ldx8(0x01);
    cmp8((u8)X, 0x2F);
    if (fc) {
        ldx8(0x2F);
        lda8(rd8(DP(0x63)));
        and8(0x08);
        if (fz) X = dec8((u8)X);
    }
    lda8(rd8(ABS(0x0178)));
    lda8(asl8((u8)A));
    if (fc) { ret(); return; }
    lda8((u8)X);
    ora8(0x80);
    X = (u8)A; nz8((u8)X);
    ret();
}

void ObjectFacing_m1x1(void);
void ApplyVelocityX_m1x1(void);
void ApplyVelocity_m1x1(void);

/* $81:9E61 SteerObjectStep (RTL, Y = flags): Stadium 4's $81:9F3A with
 * Stadium 1's object arrays (6 bytes lower, target $0399 / $039B). */
void SteerObjectStep_m1x1(void)
{
    wr8(DP(0x04), (u8)Y);
    push8(0x81); push16(0x9E66); AimVelocity_m1x1();
    ldy8(0x00);
    lda8(rd8(ABSX(0x0299)));
    if (fn) Y = dec8((u8)Y);
    lda8((u8)Y); wr8(ABSX(0x029A), (u8)A);
    ldy8(0x00);
    lda8(rd8(ABSX(0x02A9)));
    if (fn) Y = dec8((u8)Y);
    lda8((u8)Y); wr8(ABSX(0x02AA), (u8)A);
    rep(0x20);
    wr16(DP(0x04), lsr16(rd16(DP(0x04))));      /* LSR $04 with A 16-bit */
    if (fc) {
        lda16(rd16(ABSX(0x0299))); eor16(0xFFFF); lda16(inc16(A)); wr16(ABSX(0x0299), A);
        lda16(rd16(ABSX(0x02A9))); eor16(0xFFFF); lda16(inc16(A)); wr16(ABSX(0x02A9), A);
    }
    sep(0x20);
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9EA2); ObjectFacing_m1x1(); }
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9EAA); ApplyVelocityX_m1x1(); }
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9EB2); ApplyVelocity_m1x1(); }
    rep(0x20);
    wr16(DP(0x00), 0);
    lda16(rd16(ABSX(0x0299)));
    if (!fz) {
        lda16(rd16(ABSX(0x0279)));
        fc = 1; sbc16(rd16(ABS(0x0399)));
        eor16(rd16(ABSX(0x0299)));
        if (!fn) {
            lda16(rd16(ABS(0x0399))); wr16(ABSX(0x0279), A);
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
    }
    lda16(rd16(ABSX(0x0289)));
    if (!fz) {
        lda16(rd16(ABSX(0x0289)));
        fc = 1; sbc16(rd16(ABS(0x039B)));
        eor16(rd16(ABSX(0x02A9)));
        if (!fn) {
            lda16(rd16(ABS(0x039B))); wr16(ABSX(0x0289), A);
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
    }
    sep(0x20);
    lda8(rd8(DP(0x00)));
    cmp8((u8)A, 0x02);
    fc = fz ? 1 : 0;
    retl();
}

/* $80:AE33 BrakeRider (RTS, X = rider, Y = how hard): slow rider X by
 * $80:B85A,Y unless already below the minimum $80:B865 for its air state
 * (speed high 0); the speed $01C0:$01B8 stops at 0. */
void BrakeRider_m1x1(void)
{
    wr8(DP(0x00), (u8)Y);
    lda8(rd8(ABSX(0x01C0)));
    if (fz) {
        lda8(rd8(ABSX(0x01E8)));
        lda8(lsr8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0x01B8)));
        cmp8((u8)A, rd8(ABSY(0xB865)));
        if (!fc) { ret(); return; }
    }
    Y = rd8(DP(0x00)); nz8((u8)Y);
    lda8(rd8(ABSX(0x01B8)));
    fc = 1; sbc8(rd8(ABSY(0xB85A)));
    wr8(ABSX(0x01B8), (u8)A);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x01C0)));
    if (!fz) { wr8(ABSX(0x01C0), dec8(rd8(ABSX(0x01C0)))); ret(); return; }
    wr8(ABSX(0x01B8), (u8)A);
    ret();
}

void MirrorJumpRider_m1x1(void);

/* $80:EB39 (RTS, X = rider): on a slowing piece (on the ground, unless in
 * jump states 1/2/5...): $08BC = 1, braking 6 (5 holding B), and on a
 * $01 piece the player's $098F is capped at $1200. Y is kept. */
void SlowPieceBrake_m1x1(void)
{
    lda8(rd8(ABSX(0x01E8)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x0208)));
    if (!fz) {
        lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
        if (!fc) { ret(); return; }
    }
    wr8(DP(0x01), (u8)Y);
    ldy8(0x04);
    lda8(rd8(ABSX(0x0178)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    if (fc) Y = dec8((u8)Y);
    lda8(0x01); wr8(ABSX(0x08BC), 0x01);
    Y = inc8((u8)Y); Y = inc8((u8)Y);
    push16(0xEB5C); BrakeRider_m1x1();
    Y = rd8(ABSX(0x0210)); nz8((u8)Y);
    lda8(rd8(ABSY(0xACF9)));
    cmp8((u8)A, 0x01);
    if (fz) {
        cmp8((u8)X, 0x00);
        if (fz) {
            rep(0x20);
            lda16(rd16(ABS(0x098F)));
            cmp16(A, 0x1200);
            if (fc) { lda16(0x1200); wr16(ABS(0x098F), 0x1200); }
        }
        sep(0x20);
    }
    Y = rd8(DP(0x01)); nz8((u8)Y);
    ret();
}

/* $80:CC3D DrawAllRiders (RTS, X = last slot): for slots X-0 the rider in
 * slot $0950 (on the track, $01D8): its palette $04 ($80:BD9A), colour
 * $0B ($80:BDA7 by $0CE0, or by place $1959 and entry $1542 from
 * $80:BDAF, flashing 2/6 for the leader in a race), drawn with DrawRider;
 * the player mirrored when its pose is $16. */
void DrawAllRiders_m1x1(void)
{
    do {
        wr8(DP(0x0D), (u8)X);
        lda8(rd8(ABSX(0xBD9A))); wr8(DP(0x04), (u8)A);
        lda8(rd8(ABS(0x0CE0)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        ora8(rd8(ABSX(0x0950)));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0x0950)));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x01D8)));
        if (!fz) {
            lda8(rd8(ABSY(0xBDA7))); wr8(DP(0x0B), (u8)A);
            lda8(rd8(ABSX(0x1959)));
            if (!fz) {
                cmp8((u8)A, 0x05);
                if (!fc) {
                    push8((u8)X);
                    wr8(DP(0x0B), (u8)X);
                    lda8(rd8(ABS(0x1542)));
                    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
                    fc = 0; adc8(rd8(DP(0x0B)));
                    X = (u8)A; nz8((u8)X);
                    and8(0x07);
                    int flash = fz;
                    if (flash) { lda8(rd8(ABS(0x0CE0))); flash = !fz; }
                    if (flash) { lda8(rd8(DP(0x76))); cmp8((u8)A, 0x03); flash = fz; }
                    if (flash) { lda8(rd8(DP(0x63))); and8(0x02); flash = fz; }
                    if (flash) { lda8(rd8(ABS(0x1923))); flash = fz; }
                    if (flash) {
                        lda8(rd8(ABS(0x1542)));
                        cmp8((u8)A, 0x03);
                        lda8(fz ? 0x02 : 0x06);
                    } else lda8(rd8(ABSX(0xBDAF)));
                    wr8(DP(0x0B), (u8)A);
                    X = pull8(); nz8((u8)X);
                }
            }
            push16(0xCCA2); DrawRider_m1x1();
        }
        cmp8((u8)X, 0x00);
        if (fz) {
            lda8(rd8(DP(0xA0)));
            cmp8((u8)A, 0x16);
            if (fz) { push16(0xCCAF); MirrorJumpRider_m1x1(); }
        }
        X = rd8(DP(0x0D)); nz8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $81:A0B5 (RTL): the engine sound (not during a fade $1923): every
 * rider's distance to the player ($0198:$0190, signed at $10 + 2X,
 * absolute at 2X); the nearest one's distance / 2 (signed) to APU port 1
 * and its speed ($01C0:$01B8 >> 3) to port 3. */
void EngineSound_m1x1(void)
{
    lda8(rd8(ABS(0x1923)));
    if (!fz) { retl(); return; }
    ldx8(0x07);
    ldy8(0x0E);
    lda8(rd8(ABS(0x0190))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABS(0x0198))); wr8(DP(0x01), (u8)A);
    do {
        lda8(rd8(ABSX(0x0198)));
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        lda8(rd8(ABSX(0x0190)));
        rep(0x20);
        fc = 1; sbc16(rd16(DP(0x00)));
        wr16(ABSY(0x0010), A);
        if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
        wr16(ABSY(0x0000), A);
        sep(0x20);
        Y = dec8((u8)Y); Y = dec8((u8)Y);
        X = dec8((u8)X);
    } while (!fz);
    rep(0x20);
    lda16(0x000E); wr16(DP(0x10), 0x000E);
    ldx8(0x0C);
    lda16(rd16(DP(0x0E)));
    do {
        cmp16(A, rd16(DPX(0x00)));
        if (fc) { wr8(DP(0x10), (u8)X); lda16(rd16(DPX(0x00))); }
        X = dec8((u8)X); X = dec8((u8)X);
    } while (!fz);
    sep(0x20);
    X = rd8(DP(0x10)); nz8((u8)X);
    lda8(rd8(DPX(0x00)));
    lda8(lsr8((u8)A));
    Y = rd8(ABSX(0x0011)); nz8((u8)Y);
    if (fn) { eor8(0xFF); lda8(inc8((u8)A)); }
    wr8(ABS(0x2141), (u8)A);
    lda8((u8)X);
    lda8(lsr8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x01C0)));
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    lda8(rd8(ABSX(0x01B8)));
    rep(0x20);
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    sep(0x20);
    wr8(ABS(0x2143), (u8)A);
    retl();
}

void ScrollByPlayerSpeed_m1x1(void);

/* $80:DB44 (RTS): the bonus run: while it runs the screen follows the
 * player; once over ($84) the player rides off by its speed $0180, and
 * past X $F0 the run ends ($84 = $0198 = 2, sound $2D, the label); else
 * the speed is cleared. */
void BonusRunOut_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    if (fz) { ScrollByPlayerSpeed_m1x1(); return; }
    lda8(rd8(ABS(0x0180)));
    fc = 0; adc8(rd8(ABS(0x0190)));
    wr8(ABS(0x0190), (u8)A);
    cmp8((u8)A, 0xF0);
    if (fc) {
        lda8(0x02); wr8(DP(0x84), 0x02); wr8(ABS(0x0198), 0x02);
        lda8(0x2D);
        push8(0x80); push16(0xDB65); QueueSound_m1x1();
        push16(0xDB87); ShowBonusLabel_m1x1();
        ret();
        return;
    }
    lda8(0x00); wr8(ABS(0x0180), 0x00);
    ret();
}

void SegmentNext_m1x1(void);
void SpawnDustPuff_m1x1(void);

/* $80:915C: SegmentNext's tail: APU port 3 silenced, $75 / $77 cleared,
 * fade out ($1923 = $FF). */
static void segment_finish(void)
{
    wr8(ABS(0x2143), 0);
    lda8(0x00);
    wr8(DP(0x75), 0);
    wr8(DP(0x77), 0);
    lda8(0xFF);
    wr8(ABS(0x1923), 0xFF);
    ret();
}

/* $80:AB7F: a dust puff every 16 frames; with $60 odd, $00 = $58,
 * X = 7, Y = $1C. */
static void dust_tail(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x0F);
    if (fz) { push8(0x80); push16(0xAB88); SpawnDustPuff_m1x1(); }
    lda8(rd8(DP(0x60)));
    lda8(lsr8((u8)A));
    if (fc) {
        lda8(0x58); wr8(DP(0x00), 0x58);
        ldx8(0x07);
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
    }
    ret();
}

/* $80:AB3D (RTS): the segment timer $52 (reloaded with $10): at 1 the
 * next segment, and on the special course ($0CD7) the race state $38-$63
 * is cleared, mode 0 with $79 + 1. Otherwise $092D / $0940 / $60 clear,
 * and right after the race ($84 = 1) on even ticks X = $19 (from $12). */
void SegmentTimer_m1x1(void)
{
    lda8(rd8(DP(0x52)));
    if (fz) { lda8(0x10); wr8(DP(0x52), 0x10); }
    cmp8((u8)A, 0x01);
    if (fz) {
        push16(0xAB4B); SegmentNext_m1x1();
        lda8(rd8(ABS(0x0CD7)));
        if (fz) { ret(); return; }
        ldx8(0x2B);
        do {
            wr8(DPX(0x38), 0);
            X = dec8((u8)X);
        } while (!fn);
        wr8(DP(0x76), 0);
        wr8(DP(0x79), inc8(rd8(DP(0x79))));
        segment_finish();
        return;
    }
    lda8(0x00);
    wr8(ABS(0x092D), 0); wr8(ABS(0x0940), 0); wr8(DP(0x60), 0);
    X = rd8(DP(0x84)); nz8((u8)X);
    X = dec8((u8)X);
    if (!fz) { ret(); return; }
    ldx8(0x12);
    lda8(rd8(DP(0x52)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    ldy8(0x06);
    lda8(0xFE);
    do {
        X = inc8((u8)X);
        Y = dec8((u8)Y);
    } while (!fn);
    ret();
}

/* $80:C41D (RTS): the finish ($60 / $0940): at $60 = 8 the race closes
 * ($0940 cleared, sound $2D, $0990 = $08CC = 5, the player's start step
 * $01D0 = 5 if unset); above 8 the stopped player is parked ($01B8 = $C0
 * in a tumble, else 0 with steering $01A8 toward lane $80:BDA2) and the
 * dust tail runs. */
void FinishStep_m1x1(void)
{
    lda8(rd8(DP(0x60)));
    ora8(rd8(ABS(0x0940)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x60)));
    cmp8((u8)A, 0x08);
    if (fz) {
        lda8(0x00); wr8(ABS(0x0940), 0x00);
        lda8(0x2D);
        push8(0x80); push16(0xC461); QueueSound_m1x1();
        lda8(0x05); wr8(ABS(0x0990), 0x05); wr8(ABS(0x08CC), 0x05);
        Y = rd8(ABS(0x01D0)); nz8((u8)Y);
        if (fz) wr8(ABS(0x01D0), (u8)A);
        ret();
        return;
    }
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(0x01C0)));
    ora8(rd8(ABS(0x01C8)));
    ora8(rd8(ABS(0x01D0)));
    if (fz) {
        lda8(rd8(ABS(0x0170)));
        if (!fz) { lda8(0xC0); wr8(ABS(0x01B8), 0xC0); }
        else {
            wr8(ABS(0x01B8), (u8)A);
            ldx8(0x01);
            lda8(rd8(ABS(0x01F0)));
            cmp8((u8)A, rd8(ABS(0xBDA2)));
            if (fz) X = dec8((u8)X);
            wr8(ABS(0x01A8), (u8)X);
        }
    }
    dust_tail();
}

/* $80:E004 CheckEndingMinute (RTS): when $88 runs out: the special course
 * ends ($0CD7 = 0); with tries left ($1E87 - $0A) back to mode 9; else
 * $88 = 1 and from minute $39 to mode $10 with the timers $86-$8D
 * cleared. Each change finishes the segment. */
void CheckEndingMinute_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    wr8(ABS(0x0CD7), 0);
    lda8(rd8(ABS(0x1E87)));
    if (!fn) {
        fc = 1; sbc8(0x0A);
        wr8(ABS(0x1E87), (u8)A);
        if (!fn) {
            lda8(0x09); wr8(DP(0x76), 0x09);
            segment_finish();
            return;
        }
    }
    lda8(0x01); wr8(DP(0x88), 0x01);
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x39);
    if (!fc) { ret(); return; }
    lda8(0x10); wr8(DP(0x76), 0x10);
    rep(0x20);
    wr16(ABS(0x0086), 0); wr16(ABS(0x0088), 0); wr16(ABS(0x008A), 0); wr16(ABS(0x008C), 0);
    sep(0x20);
    segment_finish();
}

/* ---- Stadium 1 start flags and lights: Stadium 4's $80:C2BB / $80:C290
 * with flag table $0258, scroll $92, Y table $80:BDEF, attribute $31,
 * counter $BB and racing flag $8E. ---- */
/* $80:C52E body: the eight start-line flag poles ($0936 - scroll $92, Y
 * from $80:BB07, tile Y = $E1-$E3, attribute $3F; retired off the left edge
 * via $025E), then the countdown beeps: $3C at $B4, $78 or $3C plays sound
 * $26 and counts ($BB); with $54 set, $3C = $B5. */
static void start_flags1(void)
{
    wr8(DP(0x00), (u8)Y);
    ldy8(0x07);
    do {
        lda8(rd8(ABSY(0x0258)));
        if (fz) {
            wr8(DP(0x08), 0);
            lda8(rd8(ABSY(0x0936)));
            fc = 1;
            sbc8(rd8(ABS(0x0092)));
            wr8(DP(0x01), (u8)A);
            cmp8((u8)A, 0xF0);
            if (fc) {
                cmp8((u8)A, 0xF9);
                if (fc) wr8(DP(0x08), inc8(rd8(DP(0x08))));
                lda8(0x01);
                wr8(ABSY(0x0258), 0x01);
            } else {
                lda8((u8)Y);
                lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(ABSY(0xBDEF)));
                wr8(ABSX(0x17A1), (u8)A);
                fc = 0; adc8(0x08);
                wr8(ABSX(0x17A5), (u8)A);
                lda8(rd8(DP(0x00)));
                wr8(ABSX(0x17A2), (u8)A);
                ora8(0x10);
                wr8(ABSX(0x17A6), (u8)A);
                lda8(0x31);
                wr8(ABSX(0x17A3), 0x31); wr8(ABSX(0x17A7), 0x31);
                lda8(rd8(DP(0x01)));
                wr8(ABSX(0x17A0), (u8)A); wr8(ABSX(0x17A4), (u8)A);
                lda8((u8)X);
                lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(DP(0x08)));
                wr8(ABSX(0x1888), (u8)A);
            }
        }
        Y = dec8((u8)Y);
    } while (!fn);
    lda8(rd8(DP(0x54)));
    if (!fz) { lda8(0xB5); wr8(DP(0x3C), 0xB5); return; }
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0xB4);
    if (!fz) { cmp8((u8)A, 0x78); if (!fz) cmp8((u8)A, 0x3C); }
    if (fz) {
        lda8(0x26);
        push8(0x80); push16(0xC5A1); QueueSound_m1x1();
        wr8(DP(0xBB), inc8(rd8(DP(0xBB))));
    }
}

/* $80:C52E DrawStartFlags (RTS, Y = the flag tile). */
void DrawStartFlags_m1x1(void)
{
    start_flags1();
    ret();
}

/* $80:C503 RaceStartLights (RTS): the start of a race. When the lights
 * begin ($3C = 1): unless already running ($8E) count ($BB), set the
 * countdown $8A = $40 and play sound $27 (with the coin turbo also $2D and
 * 1); then $92 = 1, $3C = 2. Before the run ($92 clear) ping the APU every
 * 16 frames of $3C. Then the flags with tile $E1/$E2/$E3 by $3C
 * (DrawStartFlags). */
void RaceStartLights_m1x1(void)
{
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(rd8(DP(0x8E)));
        if (fz) {
            wr8(DP(0xBB), inc8(rd8(DP(0xBB))));
            lda8(0x40); wr8(DP(0x8A), 0x40);
            lda8(0x27);
            push8(0x80); push16(0xC5BA); QueueSound_m1x1();
            lda8(rd8(ABS(0x0CE0)));
            if (!fz) {
                lda8(0x2D);
                push8(0x80); push16(0xC5C5); QueueSound_m1x1();
                lda8(0x01);
                push8(0x80); push16(0xC5CB); QueueSound_m1x1();
            }
        }
        lda8(0x01); wr8(DP(0x8E), 0x01);
        lda8(0x02); wr8(DP(0x3C), 0x02);
    } else {
        Y = rd8(DP(0x8E)); nz8((u8)Y);
        if (!fz) { ret(); return; }
        Y = rd8(DP(0x63)); nz8((u8)Y);
        and8(0x0F);
        if (fz) { ldy8(0x01); wr8(ABS(0x2142), 0x01); }
    }
    ldy8(0xE1);
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0x10);
    if (!fc) {
        ldy8(0xE2);
        cmp8((u8)A, 0x08);
        if (!fc) ldy8(0xE3);
    }
    start_flags1();
    ret();
}

/* $80:A10E (RTL): the five entry-code digits $11:5730-$11:5734 as a
 * number (x10000, x1000, x100, x10, +1 into $10-$13), packed as BCD
 * nibbles into $0850-$0853. */
void PackEntryCode_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x10), 0); wr16(DP(0x12), 0);
    static const u16 mul[4] = { 0x2710, 0x03E8, 0x0064, 0x000A };
    static const u16 back[4][2] = { { 0xA127, 0xA12A }, { 0xA140, 0xA143 },
                                    { 0xA159, 0xA15C }, { 0xA172, 0xA175 } };
    for (int i = 0; i < 4; i++) {
        if (i) rep(0x20);
        lda16(rd16(LONG(0x115730 + i)));
        and16(0x00FF);
        wr16(DP(0x00), A);
        lda16(mul[i]); wr16(DP(0x02), mul[i]);
        sep(0x20);
        push8(0x80); push16(back[i][0]); Mul16_m1x1();
        push16(back[i][1]); Add32_m1x1();
    }
    lda8(rd8(LONG(0x115734))); wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA184); Add32_m1x1();
    lda8(rd8(DP(0x11)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(0x0850), (u8)A);
    lda8(rd8(DP(0x11))); and8(0x0F); wr8(ABS(0x0851), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(0x0852), (u8)A);
    lda8(rd8(DP(0x10))); and8(0x0F); wr8(ABS(0x0853), (u8)A);
    retl();
}

/* $80:98B2 QueueTimeDigits (RTL): the three bytes at [$18] (minutes,
 * seconds, hundredths) as "M'SS"HH" in two rows from $03D3 (blank $0C
 * minute tens, marks $0A / $0B), size $28 from $03D2, at VRAM $00.
 * X and Y are kept. */
void QueueTimeDigits_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    ldx8(0x00); ldy8(0x00);
    lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        X = inc8((u8)X);
    }
    wr8(ABS(0x03D4), (u8)A);
    ora8(0x10); wr8(ABS(0x03DC), (u8)A);
    lda8((u8)X);
    cmp8((u8)A, 0x00);
    if (fz) lda8(0x0C);
    wr8(ABS(0x03D3), (u8)A);
    ora8(0x10); wr8(ABS(0x03DB), (u8)A);
    static const u16 lo[2] = { 0x03D7, 0x03DA };
    for (int i = 0; i < 2; i++) {
        Y = inc8((u8)Y);
        ldx8(0x00);
        lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
        for (;;) {
            cmp8((u8)A, 0x0A);
            if (!fc) break;
            sbc8(0x0A);
            X = inc8((u8)X);
        }
        wr8(ABS(lo[i]), (u8)A);
        ora8(0x10); wr8(ABS(lo[i] + 8), (u8)A);
        lda8((u8)X);
        wr8(ABS(lo[i] - 1), (u8)A);
        ora8(0x10); wr8(ABS(lo[i] + 7), (u8)A);
    }
    lda8(0x0A); wr8(ABS(0x03D5), 0x0A);
    lda8(inc8((u8)A)); wr8(ABS(0x03D8), (u8)A);
    lda8(0x1A); wr8(ABS(0x03DD), 0x1A);
    lda8(inc8((u8)A)); wr8(ABS(0x03E0), (u8)A);
    lda8(0x28); wr8(ABS(0x03D2), 0x28);
    rep(0x20);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push16(0x993E); VramQueueTiles_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}

/* $81:9DC6 (RTS): object $039D's metasprite: frame $0349 of its type's
 * sprite list ($81:AB0D by $02F9; $038A pieces of 4 bytes at $81:AB21:
 * dx, dy, tile, attribute, bit 5 = skip, bit 4 = large), placed at its
 * position $0279 / $0289 with palette bits $0389 into OAM from
 * $1600 + 32 * slot downward. */
void DrawObjectSprites_m1x1(void)
{
    X = rd8(ABS(0x039D)); nz8((u8)X);
    lda8(rd8(ABSX(0x02F9)));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    rep(0x20);
    lda16(rd16(ABSY(0xAB0D))); wr16(DP(0x00), A);
    sep(0x20);
    lda8(rd8(ABSX(0x0349)));
    lda8(inc8((u8)A));
    wr8(ABS(0x4202), (u8)A);
    lda8(rd8(ABSX(0x038A)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(ABS(0x4203), (u8)A);
    rep(0x30);
    lda16(rd16(ABS(0x4216)));
    fc = 1; sbc16(0x0004);
    fc = 0; adc16(rd16(DP(0x00)));
    Y = A; nz16(Y);
    lda16(rd16(ABSX(0x038A))); wr16(DP(0x00), A);
    lda16(rd16(ABSX(0x0279))); wr16(DP(0x04), A);
    lda16(rd16(ABSX(0x0389))); wr16(DP(0x0E), A);
    lda16(rd16(ABSX(0x0289))); wr16(DP(0x06), A);
    lda16(X);
    for (int i = 0; i < 5; i++) lda16(asl16(A));
    X = A; nz16(X);
    sep(0x20);
    do {
        lda8(rd8(ABSY(0xAB24)));
        and8(0x20);
        if (fz) {
            lda8(rd8(ABSY(0xAB21)));
            fc = 0; adc8(rd8(DP(0x04)));
            wr8(ABSX(0x1600), (u8)A);
            lda8(rd8(ABSY(0xAB22)));
            fc = 0; adc8(rd8(DP(0x06)));
            wr8(ABSX(0x1601), (u8)A);
            lda8(rd8(ABSY(0xAB23)));
            wr8(ABSX(0x1602), (u8)A);
            lda8(rd8(ABSY(0xAB24)));
            and8(0xCF);
            ora8(rd8(DP(0x0E)));
            wr8(ABSX(0x1603), (u8)A);
            rep(0x20);
            push16(X);
            lda16(X);
            lda16(lsr16(A)); lda16(lsr16(A));
            X = A; nz16(X);
            sep(0x20);
            lda8(rd8(ABSY(0xAB24)));
            and8(0x10);
            lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
            wr8(ABSX(0x1820), (u8)A);
            X = pull16(); nz16(X);
        }
        for (int i = 0; i < 4; i++) X = dec16(X);
        for (int i = 0; i < 4; i++) Y = dec16(Y);
        wr8(DP(0x00), dec8(rd8(DP(0x00))));
    } while (!fz);
    sep(0x10);
    ret();
}

void StartMode01_m1x1(void);

/* $80:E3C7: leave or act on a demo command Y through the table $80:E0D3 /
 * $80:E0D8 (JMP ($0000)). Returns 1 when the demo should read on
 * ($80:E3A9) after the command (lane/attribute tweaks and the $10 timer
 * set), 0 when the command ended the step. */
static int demo_command1(void)
{
    lda8(rd8(ABSY(0xE0D3))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xE0D8))); wr8(DP(0x01), (u8)A);
    u16 target = rd16(LONG(0x000000));
    switch (target) {
    case 0xE3FB:                                       /* end: mode 1 */
        StartMode01_m1x1();
        return 0;
    case 0xE3F2:
        lda8(0x35);
        fc = 0; adc8(rd8(ABSX(0xE1A5)));
        return 1;
    case 0xE3EE:
        lda8(0x40);
        return 1;
    case 0xE3DF:                                       /* the ball rider */
        ldx8(0x07);
        wr8(ABS(0x092A), 0x07);
        lda8(0x00); wr8(ABSX(0x0198), 0x00);
        ldy8(0x03);
        PlaceRival_m1x1();
        return 0;
    case 0xE3D4:
        lda8(0x10); wr8(ABS(0x099D), 0x10); wr8(ABS(0x0990), 0x10);
        return 1;
    default:
        r816_call_pc(0x800000u | target);
        return 0;
    }
}

/* $80:E3A9: the next demo entry ($0CE1): its duration ($80:E0DD) and pad
 * input ($80:E1A5 -> $0178, $098E); entries $F8+ are commands. */
static void demo_next1(void)
{
    for (;;) {
        X = rd8(ABS(0x0CE1)); nz8((u8)X);
        wr8(ABS(0x0CE1), inc8(rd8(ABS(0x0CE1))));
        lda8(rd8(ABSX(0xE0DD)));
        cmp8((u8)A, 0xF8);
        if (!fc) {
            wr8(ABS(0x0CE3), (u8)A);
            lda8(rd8(ABSX(0xE1A5)));
            wr8(ABS(0x0178), (u8)A);
            wr8(ABS(0x098E), (u8)A);
            fc = 0;
            ret();
            return;
        }
        eor8(0xFF);
        Y = (u8)A; nz8((u8)Y);
        if (!demo_command1()) return;
    }
}

/* $80:E377 DemoInputStep (RTS): the attract demo's recorded input:
 * Start with the debug flag ends it; before the run ($8E) the first entry
 * ($0CE1 = 0, or $68 with $81:800C); each entry is held for its duration
 * $0CE3. C clear while it plays. */
void DemoInputStep_m1x1(void)
{
    lda8(rd8(LONG(0x81800E)));
    if (!fz) {
        lda8(rd8(DP(0x21)));
        and8(0x10);
        if (!fz) {
            ldy8(0x00);
            if (demo_command1()) demo_next1();
            return;
        }
    }
    lda8(rd8(DP(0x8E)));
    if (fz) {
        wr8(DP(0x63), 0);
        wr8(ABS(0x0CE3), 0);
        lda8(rd8(LONG(0x81800C)));
        if (!fz) lda8(0x68);
        wr8(ABS(0x0CE1), (u8)A);
        fc = 0;
        ret();
        return;
    }
    lda8(rd8(ABS(0x0CE3)));
    if (!fz) {
        wr8(ABS(0x0CE3), dec8(rd8(ABS(0x0CE3))));
        fc = 0;
        ret();
        return;
    }
    demo_next1();
}

/* $80:E4FC (RTS): from broadcast minute 5:07 on, the blinking "PUSH
 * START" text ($80:E54B + 0/5/... by $BD bit 5) at VRAM $18F1, and Start
 * leaves the demo (APU port 0 = $2D, demo command 0). */
void DemoPushStart_m1x1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x05);
    if (!fc) { ret(); return; }
    if (fz) {
        lda8(rd8(LONG(0x7FFFFB)));
        cmp8((u8)A, 0x07);
        if (!fc) { ret(); return; }
    }
    wr8(DP(0xBD), inc8(rd8(DP(0xBD))));
    lda8(rd8(DP(0xBD)));
    and8(0x20);
    lda8(lsr8((u8)A));
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    fc = 0; adc8(rd8(DP(0x00)));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    fc = 0; adc8(rd8(DP(0x00)));
    rep(0x20);
    and16(0x00FF);
    fc = 0; adc16(0xE54B);
    wr16(DP(0x02), A);
    lda16(0x18F1); wr16(DP(0x00), 0x18F1);
    sep(0x20);
    push16(0xE539); VramQueueTiles20_m1x1();
    lda8(rd8(DP(0x21)));
    and8(0x10);
    if (fz) { ret(); return; }
    lda8(0x2D); wr8(ABS(0x2140), 0x2D);
    ldy8(0x00);
    if (demo_command1()) demo_next1();
}

/* $81:9714 UpdateObjects (RTL): Stadium 4's $81:97E8 with Stadium 1's
 * object arrays (slot $039D; steps $81:AAE5, draws $81:AAF9). */
void UpdateObjects_m1x1(void)
{
    wr8(ABS(0x1958), 0);
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    ldx8(0x0E);
    do {
        wr8(ABS(0x039D), (u8)X);
        lda8(rd8(ABSX(0x02E9)));
        if (!fz) {
            lda8(rd8(ABSX(0x0369)));
            if (!fz) wr8(ABSX(0x0369), dec8(rd8(ABSX(0x0369))));
            lda8(rd8(ABSX(0x0379)));
            if (!fz) wr8(ABSX(0x0379), dec8(rd8(ABSX(0x0379))));
            lda8(rd8(ABSX(0x02F9)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            push8((u8)X);
            push16(0x973E);
            r816_call_pc(0x810000u | rd16(LONGX(0x81AAE5)));
            X = pull8(); nz8((u8)X);
            Y = rd8(ABS(0x039D)); nz8((u8)Y);
            lda8(rd8(ABSY(0x02E9)));
            if (!fz) {
                push16(0x974A);
                r816_call_pc(0x810000u | rd16(LONGX(0x81AAF9)));
            }
        }
        X = rd8(ABS(0x039D)); nz8((u8)X);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:99E7 (RTS): the results list, a line every $20 frames ($88): the
 * time of entry $0452 (3 bytes at $083D) at its VRAM row ($80:9A48),
 * sound $42; after five, the next steps ($0453 = 4 and $0454 = 1, or 2
 * with $085C). */
void EndScreenTimes_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(rd8(ABS(0x0452)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    fc = 0; adc8(rd8(ABS(0x0452)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0x083D))); wr8(ABS(0x03B1), (u8)A);
    lda8(rd8(ABSY(0x083E))); wr8(ABS(0x03B2), (u8)A);
    lda8(rd8(ABSY(0x083F))); wr8(ABS(0x03B3), (u8)A);
    rep(0x20);
    lda16(rd16(ABSX(0x9A48))); wr16(DP(0x00), A);
    lda16(0x03B1); wr16(DP(0x18), 0x03B1);
    sep(0x20);
    lda8(0x00); wr8(DP(0x1A), 0x00);
    push8(0x80); push16(0x9A20); QueueTimeDigits_m1x1();
    lda8(0x42);
    push8(0x80); push16(0x9A26); QueueSound_m1x1();
    wr8(ABS(0x0452), inc8(rd8(ABS(0x0452))));
    lda8(rd8(ABS(0x0452)));
    cmp8((u8)A, 0x05);
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(0x085C)));
    if (fz) {
        lda8(0x04); wr8(ABS(0x0453), 0x04);
        lda8(0x01); wr8(ABS(0x0454), 0x01);
    } else {
        lda8(0x02); wr8(ABS(0x0453), 0x02);
    }
    ret();
}

/* $80:9A87 (RTS): the record entry: after the $88 delay the box
 * ($89:D800); then A: step $0453 = 1, $88 = $20, the next box
 * ($89:D980) and the record time ($11:572C) at VRAM $09D3, sound $43. */
void EndScreenRecord_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) {
        u8 t = dec8(rd8(DP(0x88)));
        wr8(DP(0x88), t);
        if (!fz) { ret(); return; }
        lda8(0x00); wr8(DP(0x02), 0x00);
        lda8(0xD8); wr8(DP(0x03), 0xD8);
        lda8(0x89); wr8(DP(0x04), 0x89);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x9AAD); VramQueueCopy_m1x1();
    }
    lda8(rd8(DP(0x26)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(0x01); wr8(ABS(0x0453), 0x01);
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(0x80); wr8(DP(0x02), 0x80);
    lda8(0xD9); wr8(DP(0x03), 0xD9);
    lda8(0x89); wr8(DP(0x04), 0x89);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x9ADB); VramQueueCopy_m1x1();
    rep(0x20);
    lda16(0x09D3); wr16(DP(0x00), 0x09D3);
    lda16(0x572C); wr16(DP(0x18), 0x572C);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9AF1); QueueTimeDigits_m1x1();
    lda8(0x43);
    push8(0x80); push16(0x9AF7); QueueSound_m1x1();
    ret();
}

void SendSpeedToApu_m1x1(void);
void DispatchBy85_m1x1(void);
void AddScore_m1x1(void);

/* $80:D97E Mode09 (RTS): the bonus course run: engine sound, banners,
 * animation; BG1 / BG2 X from the distance $94 (BG1 flipped by $0100 and
 * $1B21 = 9 once it passed $0500, $AE); the start countdown $88 (sound
 * $26 at $B4 / $78 / $3C, at 0 sound $27 and $8A = $40, counting $BB);
 * then, unless paused ($54) or ended ($84 even: $50 = 2, next segment),
 * the pad drives the player, the nineteen run steps ($85) and the ten
 * extra sprites are hidden. */
void Mode09_m1x1(void)
{
    push16(0xD980); SendSpeedToApu_m1x1();
    push16(0xD983); BonusBannerSprites_m1x1();
    push8(0x80); push16(0xD987); AnimSourceFromFrame_m1x1();
    rep(0x20);
    lda16(rd16(DP(0x94)));
    lda16(lsr16(A));
    wr16(ABS(0x1B02), A);
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    wr16(ABS(0x1B00), A);
    lda16(rd16(ABS(0x00AE)));
    if (!fz) {
        lda16(0x0009); wr16(ABS(0x1B21), 0x0009);
        lda16(rd16(ABS(0x1B02)));
        eor16(0x0100);
        wr16(ABS(0x1B02), A);
    } else {
        lda16(rd16(ABS(0x1B02)));
        cmp16(A, 0x0500);
        if (fc) wr16(ABS(0x00AE), inc16(rd16(ABS(0x00AE))));
    }
    lda16(rd16(DP(0x88)));
    if (!fz) {
        wr16(DP(0x88), dec16(rd16(DP(0x88))));
        lda16(rd16(DP(0x88)));
        int beep;
        cmp16(A, 0x00B4); beep = fz;
        if (!beep) { cmp16(A, 0x0078); beep = fz; }
        if (!beep) { cmp16(A, 0x003C); beep = fz; }
        if (beep) {
            sep(0x20);
            lda8(0x26);
            push8(0x80); push16(0xD9ED); QueueSound_m1x1();
            wr8(DP(0xBB), inc8(rd8(DP(0xBB))));
            ret();
            return;
        }
        cmp16(A, 0x0000);
        if (fz) {
            lda16(0x0040); wr16(DP(0x8A), 0x0040);
            sep(0x20);
            lda8(0x27);
            push8(0x80); push16(0xD9E0); QueueSound_m1x1();
            wr8(DP(0xBB), inc8(rd8(DP(0xBB))));
        }
        sep(0x20);
        ret();
        return;
    }
    sep(0x20);
    wr8(ABS(0x1958), 0);
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) {
        lda8(lsr8((u8)A));
        if (!fc) {
            lda8(0x02); wr8(DP(0x50), 0x02);
            lda8(0x00); wr8(DP(0x85), 0x00); wr8(DP(0x90), 0x00);
            SegmentNext_m1x1();
            return;
        }
    }
    lda8(rd8(DP(0x20)));
    wr8(ABS(0x0178), (u8)A);
    lda8(0x00); wr8(DP(0x85), 0x00);
    do {
        push16(0xDA0C); DispatchBy85_m1x1();
        wr8(DP(0x85), inc8(rd8(DP(0x85))));
        lda8(rd8(DP(0x85)));
        cmp8((u8)A, 0x13);
    } while (!fc);
    ldx8(0x09);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(0xF0); wr8(ABSY(0x16C1), 0xF0);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:DF01 BonusResultStep (RTS): when $88 runs out keep the best count
 * ($1E88), show it; with fewer than 10 tries left ($1E87) the banner
 * ($80:DFF9 at VRAM $1990) and the count box at $1996 ("100000" for a
 * full score $09C9, else the count with blank leading digits), and the
 * points ($09AD = 1 for full, else tens / units in $09AE / $09AF) added
 * to the score. Then $88 = $FF and the next step. */
void BonusResultStep_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x09C5)));
    cmp8((u8)A, rd8(ABS(0x1E88)));
    if (fc) wr8(ABS(0x1E88), (u8)A);
    push16(0xDF15); ShowBonusBest_m1x1();
    lda8(rd8(ABS(0x1E87)));
    int show = 1;
    if (!fn) { cmp8((u8)A, 0x0A); show = !fc; }
    if (show) {
        rep(0x20);
        lda16(0x1990); wr16(DP(0x00), 0x1990);
        lda16(0xDFF9); wr16(DP(0x02), 0xDFF9);
        sep(0x20);
        push16(0xDF32); VramQueueTiles20_m1x1();
        rep(0x20);
        lda16(0x1996); wr16(DP(0x00), 0x1996);
        lda16(0x03D2); wr16(DP(0x02), 0x03D2);
        sep(0x20);
        lda8(0x50);
        wr8(ABS(0x03D3), 0x50);
        wr8(ABS(0x03D3), inc8(rd8(ABS(0x03D3))));
        for (u16 a = 0x03D4; a <= 0x03D7; a++) wr8(ABS(a), 0x50);
        lda8(0x60);
        wr8(ABS(0x03D8), 0x60);
        wr8(ABS(0x03D8), inc8(rd8(ABS(0x03D8))));
        for (u16 a = 0x03D9; a <= 0x03DC; a++) wr8(ABS(a), 0x60);
        lda8(rd8(ABS(0x1E88)));
        cmp8((u8)A, rd8(ABS(0x09C9)));
        if (!fz) {
            lda8(0xFC);
            wr8(ABS(0x03D3), 0xFC); wr8(ABS(0x03D8), 0xFC);
            ldy8(0x00);
            lda8(rd8(ABS(0x1E88)));
            for (;;) {
                cmp8((u8)A, 0x0A);
                if (!fc) break;
                sbc8(0x0A);
                Y = inc8((u8)Y);
                WAIT_TICK();
            }
            fc = 0; adc8(0x50); wr8(ABS(0x03D5), (u8)A);
            fc = 0; adc8(0x10); wr8(ABS(0x03DA), (u8)A);
            lda8((u8)Y);
            fc = 0; adc8(0x50); wr8(ABS(0x03D4), (u8)A);
            fc = 0; adc8(0x10); wr8(ABS(0x03D9), (u8)A);
            lda8(rd8(ABS(0x03D4)));
            cmp8((u8)A, 0x50);
            if (fz) {
                lda8(0xFC);
                wr8(ABS(0x03D4), 0xFC); wr8(ABS(0x03D9), 0xFC);
                lda8(rd8(ABS(0x03D5)));
                cmp8((u8)A, 0x50);
                if (fz) {
                    lda8(0xFC);
                    wr8(ABS(0x03D5), 0xFC); wr8(ABS(0x03D6), 0xFC);
                    wr8(ABS(0x03DA), 0xFC); wr8(ABS(0x03DB), 0xFC);
                }
            }
        }
        lda8(0x25); wr8(ABS(0x03D2), 0x25);
        push16(0xDFCB); VramQueueTiles20_m1x1();
        wr8(DP(0x00), 0);
        lda8(rd8(ABS(0x1E88)));
        cmp8((u8)A, rd8(ABS(0x09C9)));
        if (fz) {
            lda8(0x01); wr8(ABS(0x09AD), 0x01);
        } else {
            for (;;) {
                cmp8((u8)A, 0x0A);
                if (!fc) break;
                sbc8(0x0A);
                wr8(DP(0x00), inc8(rd8(DP(0x00))));
                WAIT_TICK();
            }
            wr8(ABS(0x09AF), (u8)A);
            lda8(rd8(DP(0x00)));
            wr8(ABS(0x09AE), (u8)A);
        }
        push16(0xDFF1); AddScore_m1x1();
    }
    lda8(0xFF); wr8(DP(0x88), 0xFF);
    wr8(DP(0x77), inc8(rd8(DP(0x77))));
    ret();
}

/* $80:CF0D (RTS): the engine temperature: while racing (no turbo $0CE0,
 * not over $84) the heat $0990:$098F rises by $80:BDFF,Y toward the
 * level $80:BE03,Y for the button held (Y from $0178 bits 6-7, A
 * forced before the run) and cools by $0B above it; at $20 it overheats
 * ($60 = $0940 = heat, sounds $2E / $3E); from $1A with A held the
 * warning ($A2, sound $2C) starts, below it stops (sound $2D). Then the
 * gauge: 13 sprites from X $58 + heat ($78 with turbo) at $17F0, tiles
 * $80:CFF2 (+4 with turbo), rows below $70 hidden. */
void TemperatureGauge_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    ora8(rd8(ABS(0x0CE0)));
    u8 top;
    if (!fz) goto off;
    lda8(rd8(ABS(0x0940)));
    if (!fz) {
        lda8(rd8(DP(0x60)));
        lda8(lsr8((u8)A));
        if (fc) goto off;
        top = 0x01;
        goto draw;
    }
    lda8(rd8(ABS(0x0178)));
    and8(0xC0);
    if (!fz) {
        Y = rd8(DP(0x8E)); nz8((u8)Y);
        if (fz) lda8(0x80);
        lda8(asl8((u8)A)); lda8(rol8((u8)A)); lda8(rol8((u8)A));
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(0x0990)));
    cmp8((u8)A, rd8(ABSY(0xBE03)));
    if (!fc) {
        lda8(rd8(ABS(0x098F)));
        fc = 0; adc8(rd8(ABSY(0xBDFF)));
        wr8(ABS(0x098F), (u8)A);
        if (fc) wr8(ABS(0x0990), inc8(rd8(ABS(0x0990))));
    } else if (!fz) {
        lda8(rd8(ABS(0x098F)));
        fc = 1; sbc8(0x0B);
        wr8(ABS(0x098F), (u8)A);
        if (!fc) {
            lda8(rd8(ABS(0x0990)));
            if (!fz) wr8(ABS(0x0990), dec8(rd8(ABS(0x0990))));
        }
    }
    ldx8(0x00);
    lda8(rd8(ABS(0x0990)));
    cmp8((u8)A, 0x20);
    if (fc) {
        wr8(DP(0x60), (u8)A);
        wr8(ABS(0x0940), (u8)A);
        push8((u8)A);
        lda8(0x2E); push8(0x80); push16(0xCF7D); QueueSound_m1x1();
        lda8(0x3E); push8(0x80); push16(0xCF83); QueueSound_m1x1();
        lda8(pull8());
    }
    cmp8((u8)A, 0x1A);
    int warn = 0;
    if (fc) { Y = dec8((u8)Y); warn = fz; }
    if (warn) {
        lda8(rd8(DP(0xA2)));
        if (fz) {
            lda8(rd8(ABS(0x0CE0)));
            if (fz) { lda8(0x2C); push8(0x80); push16(0xCF9A); QueueSound_m1x1(); }
            lda8(0x01); wr8(DP(0xA2), 0x01);
        }
    } else {
        lda8(rd8(DP(0xA2)));
        if (!fz) {
            lda8(0x2D); push8(0x80); push16(0xCFAA); QueueSound_m1x1();
            wr8(DP(0xA2), 0);
        }
    }
off:
    top = 0xFF;
draw:
    lda8(top);
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABS(0x0990)));
    fc = 0; adc8(0x58);
    X = rd8(ABS(0x0CE0)); nz8((u8)X);
    if (!fz) lda8(0x78);
    wr8(DP(0x00), (u8)A);
    ldx8(0x0C);
    lda8(rd8(ABS(0x0CE0)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    do {
        lda8(rd8(DP(0x00)));
        wr8(ABSX(0x17F0), (u8)A);
        fc = 0; adc8(0x08);
        wr8(DP(0x00), (u8)A);
        cmp8((u8)A, 0x70);
        lda8(fc ? 0xC7 : 0xF0);
        wr8(ABSX(0x17F1), (u8)A);
        lda8(rd8(ABSY(0xCFF2)));
        wr8(ABSX(0x17F2), (u8)A);
        lda8(0x21); wr8(ABSX(0x17F3), 0x21);
        Y = inc8((u8)Y);
        X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:9C0D (RTS): the entry-number editor: up / down ($21 bits 3 / 2,
 * sound $33) change digit $0452 of $03A6-$03A9 (wrapping 0-9), left /
 * right move the cursor ($80:9D03); an all-zero number becomes 1 (or 9
 * when decremented on the last digit). The digits at VRAM $0B0E, the
 * cursor sprites ($1600 / $1604, bobbing), and Start moves on with the
 * box $89:DA80. */
void EntryNumberEditor_m1x1(void)
{
    X = rd8(ABS(0x0452)); nz8((u8)X);
    ldy8(0x01);
    lda8(rd8(DP(0x21)));
    and8(0x0C);
    if (!fz) {
        lda8(0x33);
        push8(0x80); push16(0x9C1D); QueueSound_m1x1();
        lda8(rd8(DP(0x21)));
        and8(0x08);
        if (!fz) {
            lda8(rd8(ABSX(0x03A6)));
            lda8(inc8((u8)A));
            wr8(ABSX(0x03A6), (u8)A);
            cmp8((u8)A, 0x0A);
            if (fc) wr8(ABSX(0x03A6), 0);
        } else {
            ldy8(0x09);
            lda8(rd8(ABSX(0x03A6)));
            lda8(dec8((u8)A));
            wr8(ABSX(0x03A6), (u8)A);
            if (fn) { lda8(0x09); wr8(ABSX(0x03A6), 0x09); }
        }
    }
    lda8(rd8(DP(0x21)));
    and8(0x03);
    if (!fz) {
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x9D03)));
        fc = 0; adc8(rd8(ABS(0x0452)));
        and8(0x03);
        wr8(ABS(0x0452), (u8)A);
    }
    lda8(rd8(ABS(0x03A6)));
    ora8(rd8(ABS(0x03A7)));
    ora8(rd8(ABS(0x03A8)));
    ora8(rd8(ABS(0x03A9)));
    if (fz) {
        cmp8((u8)X, 0x03);
        if (!fz) ldy8(0x01);
        wr8(ABS(0x03A9), (u8)Y);
    }
    rep(0x20);
    lda16(rd16(ABS(0x03A6))); ora16(0x1010); wr16(ABS(0x03AA), A);
    lda16(rd16(ABS(0x03A8))); ora16(0x1010); wr16(ABS(0x03AC), A);
    lda16(0x0B0E); wr16(DP(0x00), 0x0B0E);
    lda16(0x03A5); wr16(DP(0x02), 0x03A5);
    sep(0x20);
    lda8(0x24); wr8(ABS(0x03A5), 0x24);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push16(0x9C99); VramQueueTiles_m1x1();
    lda8(rd8(ABS(0x0452)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    fc = 0; adc8(0x70);
    wr8(ABS(0x1600), (u8)A); wr8(ABS(0x1604), (u8)A);
    lda8(rd8(DP(0x63)));
    and8(0x18);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(0x8E);
    fc = 0; adc8(rd8(DP(0x00)));
    wr8(ABS(0x1601), (u8)A);
    lda8(0x78);
    fc = 1; sbc8(rd8(DP(0x00)));
    wr8(ABS(0x1605), (u8)A);
    lda8(0xFE); wr8(ABS(0x1602), 0xFE); wr8(ABS(0x1606), 0xFE);
    lda8(0x21); wr8(ABS(0x1603), 0x21);
    lda8(0xA1); wr8(ABS(0x1607), 0xA1);
    wr8(ABS(0x1820), 0); wr8(ABS(0x1821), 0);
    lda8(rd8(DP(0x21)));
    and8(0x10);
    if (fz) { ret(); return; }
    wr8(ABS(0x0453), inc8(rd8(ABS(0x0453))));
    lda8(0x80); wr8(DP(0x02), 0x80);
    lda8(0xDA); wr8(DP(0x03), 0xDA);
    lda8(0x89); wr8(DP(0x04), 0x89);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x9D01); VramQueueCopy_m1x1();
    ret();
}

/* $80:EBDF UpdateLanes (RTS): every rider's lane $01F0 moves by its
 * steering $01A8 (not while landing, $0170 = $14/$15) and stops on a lane
 * line ($80:E6CD); jump states $0208 settle (a landing crash-lands);
 * rows 8-$39 bound it (the edge flips the steering unless starting or
 * finishing). Then the player's d-pad steers (up $08 / down $04) with
 * sound $22 on a press or every 4 frames. */
void UpdateLanes_m1x1(void)
{
    ldx8(0x07);
    for (;;) {
        lda8(rd8(ABSX(0x0170)));
        cmp8((u8)A, 0x15);
        int move = !fz;
        if (move) { cmp8((u8)A, 0x14); move = !fz; }
        if (move) {
            lda8(rd8(ABSX(0x01F0)));
            fc = 0; adc8(rd8(ABSX(0x01A8)));
            wr8(ABSX(0x01F0), (u8)A);
        }
        ldy8(0x07);
        do {
            lda8(rd8(ABSX(0x01F0)));
            cmp8((u8)A, rd8(ABSY(0xE6CD)));
            if (fz) { lda8(0x00); wr8(ABSX(0x01A8), 0x00); goto next; }
            Y = dec8((u8)Y);
        } while (!fn);
        lda8(rd8(ABSX(0x0208)));
        if (!fz) {
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(rd8(ABSX(0x01F0)));
                cmp8((u8)A, 0x20);
                if (fc) goto rows;
                lda8(0x04); wr8(ABSX(0x0208), 0x04);
                lda8(rd8(ABSX(0x0170)));
                int land = fz;
                if (!land) { cmp8((u8)A, 0x12); land = fz; }
                if (!land) { cmp8((u8)A, 0x10); land = fz; }
                if (!land) { cmp8((u8)A, 0x0D); land = fz; }
                if (!land) goto next;
                lda8(0x00); wr8(ABSX(0x01F8), 0x00);
                push16(0xEC33); CrashLand_m1x1();
                wr8(ABSX(0x0208), dec8(rd8(ABSX(0x0208))));
                if (!fz) goto next;
            }
            cmp8((u8)A, 0x03);
            if (!fz) {
                lda8(rd8(ABSX(0x01F0)));
                cmp8((u8)A, 0x20);
                if (fc) wr8(ABSX(0x01F0), dec8(rd8(ABSX(0x01F0))));
            }
        }
    rows:
        lda8(rd8(ABSX(0x01F0)));
        cmp8((u8)A, 0x08);
        if (!fc) {
            lda8(rd8(ABSX(0x01D0)));
            if (fz) lda8(0x07);
            else {
                lda8(rd8(ABSX(0x01F0)));
                cmp8((u8)A, 0x02);
                if (!fc) lda8(0x01);
            }
            wr8(ABSX(0x01F0), (u8)A);
            cmp8((u8)X, 0x00);
            if (fz) goto flip;
            goto edge;
        }
        cmp8((u8)A, 0x3A);
        if (!fc) goto next;
        lda8(0x39); wr8(ABSX(0x01F0), 0x39);
    edge:
        lda8(rd8(ABSX(0x01D0)));
        ora8(rd8(ABSX(0x0940)));
        if (!fz) goto next;
    flip:
        lda8(rd8(ABSX(0x01A8)));
        if (fz) lda8(0xFF);
        eor8(0xFE);
        wr8(ABSX(0x01A8), (u8)A);
    next:
        X = dec8((u8)X);
        if (fn) break;
    }
    lda8(rd8(DP(0x8E)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x01C8)));
    ora8(rd8(ABS(0x0940)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x01E8)));
    if (!fz) {
        lda8(rd8(ABS(0x08F4)));
        cmp8((u8)A, 0x02);
        if (!fz) { ret(); return; }
        wr8(ABS(0x08F4), inc8(rd8(ABS(0x08F4))));
    }
    lda8(rd8(ABS(0x01D0)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) { ret(); return; }
    }
    Y = rd8(ABS(0x01A8)); nz8((u8)Y);
    lda8(rd8(ABS(0x0178)));
    and8(0x04);
    if (!fz) lda8(0xFF);
    else {
        lda8(rd8(ABS(0x0178)));
        and8(0x08);
        if (fz) { ret(); return; }
        lda8(0x01);
    }
    wr8(ABS(0x01A8), (u8)A);
    lda8(rd8(DP(0x21)));
    and8(0x0C);
    if (fz) {
        lda8(rd8(DP(0x63)));
        and8(0x03);
        if (!fz) { ret(); return; }
    }
    lda8(0x22);
    push8(0x80); push16(0xECE2); QueueSound_m1x1();
    ret();
}

/* $80:8F2D (RTL): the four select-screen wheels' sprites: a 3x3 block
 * at ($154C, $1550 - $10) in OAM slot $80:903F (by angle quadrant), tiles
 * from $80:9043 (wheel 1 uses set 4 while $1F is clear), attribute
 * $80:9048 | $80:904D (or $28 | ... while turning or off the notch, else
 * with the text row for the selection), all large. */
void DrawSelectWheels_m1x1(void)
{
    ldx8(0x03);
    do {
        lda8(rd8(ABSX(0x154C))); wr8(DP(0x04), (u8)A);
        lda8(rd8(ABSX(0x1550)));
        fc = 1; sbc8(0x10);
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(0x1547)));
        fc = 0; adc8(0x20);
        lda8(rol8((u8)A)); lda8(rol8((u8)A)); lda8(rol8((u8)A));
        and8(0x03);
        push8((u8)X);
        X = (u8)A; nz8((u8)X);
        lda8(rd8(LONGX(0x80903F)));
        X = pull8(); nz8((u8)X);
        Y = (u8)A; nz8((u8)Y);
        push8((u8)X);
        cmp8((u8)X, 0x01);
        if (fz) {
            lda8(rd8(DP(0x1F)));
            if (fz) ldx8(0x04);
        }
        lda8(rd8(DP(0x04)));
        for (int c = 0; c < 3; c++) {
            if (c) { fc = 0; adc8(0x10); }
            wr8(ABSY(0x1600 + 4 * c), (u8)A);
            wr8(ABSY(0x160C + 4 * c), (u8)A);
            wr8(ABSY(0x1618 + 4 * c), (u8)A);
        }
        lda8(rd8(DP(0x00)));
        for (int r = 0; r < 3; r++) {
            if (r) { fc = 0; adc8(0x10); }
            wr8(ABSY(0x1601 + 12 * r), (u8)A);
            wr8(ABSY(0x1605 + 12 * r), (u8)A);
            wr8(ABSY(0x1609 + 12 * r), (u8)A);
        }
        lda8(rd8(LONGX(0x809043)));
        for (int r = 0; r < 3; r++) {
            if (r) { fc = 0; adc8(0x1C); }
            for (int c = 0; c < 3; c++) {
                if (c) { lda8(inc8((u8)A)); lda8(inc8((u8)A)); }
                wr8(ABSY(0x1602 + 12 * r + 4 * c), (u8)A);
            }
        }
        lda8(rd8(DP(0x20)));
        and8(0x03);
        int busy = !fz;
        if (!busy) {
            cmp8((u8)X, 0x04);
            if (fz) { lda8(rd8(ABS(0x1548))); busy = !fz; }
            else { lda8(rd8(ABSX(0x1547))); busy = !fz; }
        }
        if (busy) lda8(0x28);
        else {
            push8(0x80); push16(0x8FEE); SpriteTextRow_m1x1();
            lda8(rd8(LONGX(0x809048)));
        }
        ora8(rd8(LONGX(0x80904D)));
        for (int i = 0; i < 9; i++) wr8(ABSY(0x1603 + 4 * i), (u8)A);
        lda8((u8)Y);
        lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(0x02);
        for (int i = 0; i < 9; i++) wr8(ABSY(0x1820 + i), 0x02);
        X = pull8(); nz8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    retl();
}

/* $80:E411 DemoPadDisplay (RTS): Stadium 4's $80:F3DD with Stadium 1's
 * tables ($80:E4E3 / $80:E4F3 / $80:E4F8), then DemoPushStart. */
void DemoPadDisplay_m1x1(void)
{
    lda8(rd8(ABS(0x098E)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xE4E3)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xE4F3)));
    wr8(DP(0x00), (u8)A);
    ldy8(0x80);
    lda8(0x80);
    wr8(ABSY(0x1600), (u8)A); wr8(ABSY(0x1608), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(0x1604), (u8)A); wr8(ABSY(0x160C), (u8)A);
    lda8(0x18);
    wr8(ABSY(0x1601), (u8)A); wr8(ABSY(0x1605), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(0x1609), (u8)A); wr8(ABSY(0x160D), (u8)A);
    lda8(rd8(DP(0x00)));
    wr8(ABSY(0x1602), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A));
    wr8(ABSY(0x1606), (u8)A);
    fc = 0; adc8(0x1E);
    wr8(ABSY(0x160A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A));
    wr8(ABSY(0x160E), (u8)A);
    lda8(0x26);
    wr8(ABSY(0x1603), 0x26); wr8(ABSY(0x1607), 0x26);
    wr8(ABSY(0x160B), 0x26); wr8(ABSY(0x160F), 0x26);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(0x1820), 0x02); wr8(ABSY(0x1821), 0x02);
    wr8(ABSY(0x1822), 0x02); wr8(ABSY(0x1823), 0x02);
    lda8(rd8(ABS(0x098E)));
    lda8(rol8((u8)A)); lda8(rol8((u8)A)); lda8(rol8((u8)A));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xE4F8)));
    wr8(DP(0x00), (u8)A);
    ldy8(0x70);
    lda8(0xB0);
    wr8(ABSY(0x1600), (u8)A); wr8(ABSY(0x1608), (u8)A);
    fc = 0; adc8(0x18);
    wr8(ABSY(0x1604), (u8)A); wr8(ABSY(0x160C), (u8)A);
    lda8(0x18);
    wr8(ABSY(0x1601), (u8)A); wr8(ABSY(0x1605), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(0x1609), (u8)A); wr8(ABSY(0x160D), (u8)A);
    lda8(rd8(DP(0x00)));
    wr8(ABSY(0x1602), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x1606), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x160A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(0x160E), (u8)A);
    lda8(0x28);
    wr8(ABSY(0x1603), 0x28); wr8(ABSY(0x1607), 0x28);
    wr8(ABSY(0x160B), 0x28); wr8(ABSY(0x160F), 0x28);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(0x1820), 0x02); wr8(ABSY(0x1821), 0x02);
    wr8(ABSY(0x1822), 0x02); wr8(ABSY(0x1823), 0x02);
    push16(0xE4E1); DemoPushStart_m1x1();
    ret();
}

void CoinCollected_m1x1(void);

/* $80:A6B2 (RTL): a collected coin ($AA = type) is erased from the
 * track: its two tile columns (left of the wheel column $0260; one more
 * when the piece there is not $80:A78C,Y) are queued from the blank
 * tiles at $0018 ($80:A79B / $80:A7A0, header $21) at the row's VRAM
 * address ($80:A7A5 + column, wrapping by 32); then CoinCollected, and
 * the fifth coin ($0CDF) starts the turbo ($0CE0 = 1, heat $20, sounds
 * $2D / 1). X is kept. */
void EraseCollectedCoin_m1x1(void)
{
    push8((u8)X);
    lda8(rd8(DP(0xAA)));
    if (fz) { X = pull8(); nz8((u8)X); retl(); return; }
    lda8(0x01); wr8(DP(0x10), 0x01);
    wr8(DP(0x11), 0);
    lda8(rd8(DP(0xAA)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xA791))); wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(ABS(0x0260)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
    Y = rd8(ABS(0x00AA)); nz8((u8)Y);
    cmp8((u8)A, rd8(ABSY(0xA78C)));
    if (!fz) wr8(DP(0x10), inc8(rd8(DP(0x10))));
    for (int pass = 0; pass < 2; pass++) {
        if (pass) {
            wr8(DP(0x10), dec8(rd8(DP(0x10))));
            lda8(rd8(DP(0xAA)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
        }
        rep(0x20);
        lda16(rd16(ABS(0x0260)));
        fc = 1; sbc16(rd16(DP(0x10)));
        and16(0x0020);
        for (int i = 0; i < 5; i++) lda16(asl16(A));
        wr16(DP(0x00), A);
        lda16(rd16(ABS(0x0260)));
        fc = 1; sbc16(rd16(DP(0x10)));
        and16(0x001F);
        fc = 0; adc16(rd16(DP(0x00)));
        fc = 0; adc16(rd16(ABSX(0xA7A5)));
        wr16(DP(0x00), A);
        lda16(0x0018); wr16(DP(0x02), 0x0018);
        sep(0x20);
        X = rd8(DP(0xAA)); nz8((u8)X);
        lda8(0x21); wr8(DP(0x18), 0x21);
        lda8(rd8(ABSX(0xA79B))); wr8(DP(0x19), (u8)A);
        lda8(rd8(ABSX(0xA7A0))); wr8(DP(0x1A), (u8)A);
        lda8(0x08); wr8(DP(0x0F), 0x08);
        push16(pass ? 0xA765 : 0xA71D); VramQueueTiles_m1x1();
    }
    wr8(DP(0xAA), 0);
    wr8(ABS(0x0CDF), inc8(rd8(ABS(0x0CDF))));
    push8(0x80); push16(0xA76E); CoinCollected_m1x1();
    lda8(rd8(ABS(0x0CDF)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x2D);
        push8(0x80); push16(0xA77B); QueueSound_m1x1();
        lda8(0x01); wr8(ABS(0x0CE0), 0x01);
        push8(0x80); push16(0xA784); QueueSound_m1x1();
        lda8(0x20); wr8(ABS(0x0990), 0x20);
    }
    X = pull8(); nz8((u8)X);
    retl();
}

/* $80:A7AF (RTL): a used track item ($AF = type) is redrawn: two tile
 * columns left of the wheel column $0260 (one more when the piece there
 * is $3B / $3C instead of $80:A86F,Y) from the tiles at $0018 ($80:A87E /
 * $80:A883, then $80:A888 / $80:A88D; header $21) at the row's VRAM
 * address ($80:A892 + column). X is kept. */
void RedrawUsedItem_m1x1(void)
{
    push8((u8)X);
    lda8(rd8(DP(0xAF)));
    if (fz) { X = pull8(); nz8((u8)X); retl(); return; }
    wr8(DP(0x10), 0); wr8(DP(0x11), 0);
    lda8(rd8(DP(0xAF)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xA874))); wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(ABS(0x0260)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
    Y = rd8(ABS(0x00AF)); nz8((u8)Y);
    cmp8((u8)A, rd8(ABSY(0xA86F)));
    if (!fz) {
        int two;
        cmp8((u8)A, 0x3C); two = fz;
        if (!two) { cmp8((u8)A, 0x3B); two = fz; }
        if (two) wr8(DP(0x10), inc8(rd8(DP(0x10))));
    }
    wr8(DP(0x10), inc8(rd8(DP(0x10))));
    static const u16 t19[2] = { 0xA87E, 0xA888 };
    static const u16 t1a[2] = { 0xA883, 0xA88D };
    for (int pass = 0; pass < 2; pass++) {
        if (pass) {
            wr8(DP(0x10), dec8(rd8(DP(0x10))));
            lda8(rd8(DP(0xAF)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
        }
        rep(0x20);
        lda16(rd16(ABS(0x0260)));
        fc = 1; sbc16(rd16(DP(0x10)));
        and16(0x0020);
        for (int i = 0; i < 5; i++) lda16(asl16(A));
        wr16(DP(0x00), A);
        lda16(rd16(ABS(0x0260)));
        fc = 1; sbc16(rd16(DP(0x10)));
        and16(0x001F);
        fc = 0; adc16(rd16(DP(0x00)));
        fc = 0; adc16(rd16(ABSX(0xA892)));
        wr16(DP(0x00), A);
        lda16(0x0018); wr16(DP(0x02), 0x0018);
        sep(0x20);
        X = rd8(DP(0xAF)); nz8((u8)X);
        lda8(0x21); wr8(DP(0x18), 0x21);
        lda8(rd8(ABSX(t19[pass]))); wr8(DP(0x19), (u8)A);
        lda8(rd8(ABSX(t1a[pass]))); wr8(DP(0x1A), (u8)A);
        lda8(0x08); wr8(DP(0x0F), 0x08);
        push16(pass ? 0xA86A : 0xA822); VramQueueTiles_m1x1();
    }
    wr8(DP(0xAF), 0);
    X = pull8(); nz8((u8)X);
    retl();
}

void LoadCourse_m1x1(void);

/* $80:E2B6 StartDemo (RTS): the attract demo: entry 7, paused flag $54 =
 * 5, ground $A0, the crowd flags at X $C0 down by $10, the player at lane
 * $0E, column 9, X $38, upright on page 1; $08C4 = $90 = $75 = 1, no
 * fade, BG3 at $19, $1905 = $17; the course and the pad display. */
void StartDemo_m1x1(void)
{
    lda8(0x07); wr8(DP(0x7C), 0x07);
    lda8(0x05); wr8(DP(0x54), 0x05);
    lda8(0xA0); wr8(ABS(0x0CD5), 0xA0);
    ldx8(0x07);
    lda8(0xC0);
    do {
        wr8(ABSX(0x0936), (u8)A);
        fc = 1; sbc8(0x10);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x0E); wr8(ABS(0x01F0), 0x0E);
    lda8(0x08); wr8(ABS(0x0188), 0x08);
    lda8(0x09); wr8(ABS(0x0260), 0x09);
    lda8(0x38); wr8(ABS(0x0190), 0x38);
    lda8(0x06); wr8(ABS(0x01E0), 0x06); wr8(ABS(0x0230), 0x06);
    lda8(0x01); wr8(ABS(0x0198), 0x01); wr8(ABS(0x01D8), 0x01);
    lda8(0x01); wr8(ABS(0x08C4), 0x01); wr8(DP(0x90), 0x01); wr8(DP(0x75), 0x01);
    lda8(0x01); wr8(ABS(0x1923), 0x01);
    lda8(0x19); wr8(ABS(0x2109), 0x19);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    push8(0x80); push16(0xE314); LoadCourse_m1x1();
    push16(0xE317); DemoPadDisplay_m1x1();
    ret();
}

void ClearWords1530_m1x1(void);

/* $80:E8B1 RiderLanePieceAt (RTS): RiderPieceAt in rider X's lane $08A4. */
void RiderLanePieceAt_m1x1(void)
{
    Y = rd8(ABSX(0x08A4)); nz8((u8)Y);
    RiderPieceAt_m1x1();
}

/* $80:B2A1 VramQueueTilesLong (RTL): VramQueueTiles for other banks. */
void VramQueueTilesLong_m1x1(void)
{
    push16(0xB2A3); VramQueueTiles_m1x1();
    retl();
}

/* $80:D3F7 (RTL): clear the coin counts $1540 / $1541 and the marker
 * objects (ClearWords1530, into which it falls). */
void ResetCoinMarkers_m1x1(void)
{
    wr8(ABS(0x1540), 0); wr8(ABS(0x1541), 0);
    ClearWords1530_m1x1();
}

/* $80:8D25 Mode01 (RTS): the player-select screen's step $1E8B through
 * the table $80:8D2B. */
void Mode01_m1x1(void)
{
    X = rd8(ABS(0x1E8B)); nz8((u8)X);
    r816_call_pc(0x800000u | rd16(LONGX(0x808D2B)));
}

/* $80:E9C1 FlattenPlayerPiece (RTS, X = rider): the player on the ground
 * (and $84 clear) turns its piece flat ($1F) and sets the coin type $AA
 * from its lane ($80:E9EC). */
void FlattenPlayerPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x01E8)));
    ora8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    push16(0xE9CE); RiderLanePieceAt_m1x1();
    rep(0x30);
    lda16(Y);
    fc = 0; adc16(0x0180);
    Y = A; nz16(Y);
    sep(0x20);
    lda8(0x1F);
    wr8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF, 0x1F);
    sep(0x10);
    push8((u8)X);
    X = rd8(ABS(0x08A4)); nz8((u8)X);
    lda8(rd8(ABSX(0xE9EC)));
    wr8(DP(0xAA), (u8)A);
    sep(0x10);
    X = pull8(); nz8((u8)X);
    ret();
}

/* $80:B0E1 ReadRiderPieces (RTS): for riders 7-0 the piece under the
 * wheel (type $0218, height $0210) and four heights on ($0884 + 4X). */
void ReadRiderPieces_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        wr8(DP(0x0C), (u8)A);
        lda8(0x04); wr8(DP(0x0B), 0x04);
        push16(0xB0EE); RiderLanePieceAt_m1x1();
        wr8(ABSX(0x0218), (u8)A);
        lda8(rd8(DP(0x10)));
        wr8(ABSX(0x0210), (u8)A);
        for (;;) {
            push8((u8)A);
            Y = inc8((u8)Y);
            lda8((u8)Y);
            and8(0x3F);
            wr8(DP(0x08), (u8)A);
            Y = rd8(DP(0x0C)); nz8((u8)Y);
            lda8(pull8());
            wr8(ABSY(0x0884), (u8)A);
            wr8(DP(0x0C), inc8(rd8(DP(0x0C))));
            u8 n = dec8(rd8(DP(0x0B)));
            wr8(DP(0x0B), n);
            if (fz) break;
            Y = rd8(DP(0x08)); nz8((u8)Y);
            lda8(rd8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF));
        }
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $81:9A8D QueueMapTile (RTL): object X's map tile ($81:9AA6 by row /
 * column, palette $11) as one word at VRAM $1840 + offset. */
void QueueMapTile_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABSX(0x0289)));
    and16(0x00F8);
    lda16(asl16(A)); lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(0x0279)));
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    Y = (u8)A; nz8((u8)Y);
    fc = 0; adc16(0x1840);
    wr16(DP(0x00), A);
    lda16(0x039F); wr16(DP(0x02), 0x039F);
    lda16(rd16(ABSY(0x9AA6)));
    and16(0x00FF);
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    ora16(0x0011);
    wr16(ABS(0x039F), A);
    sep(0x20);
    lda8(0x1C); wr8(DP(0x0F), 0x1C);
    push8(0x81); push16(0x9AC4); VramQueueTilesLong_m1x1();
    retl();
}

/* $80:BE6A WalkToStartLane (RTS, X = rider): Stadium 4's $80:BB82 for
 * Stadium 1 (targets $80:BDA2, ResetCoinMarkers, QueueSound). */
void WalkToStartLane_m1x1(void)
{
    lda8((u8)X);
    Y = (u8)A; nz8((u8)Y);
    if (!fz) {
        lda8(rd8(ABSX(0x01F8)));
        if (!fz) ldy8(0x04);
    }
    lda8(rd8(ABSX(0x01F0)));
    cmp8((u8)A, rd8(ABSY(0xBDA2)));
    if (!fz) {
        ldy8(0x01);
        if (fc) ldy8(0xFF);
        lda8((u8)Y);
        wr8(ABSX(0x01A8), (u8)A);
        ret();
        return;
    }
    lda8(0x00);
    wr8(ABSX(0x091C), 0);
    wr8(ABSX(0x01A8), 0);
    wr8(ABSX(0x01D0), inc8(rd8(ABSX(0x01D0))));
    lda8((u8)X);
    if (fz) {
        Y = rd8(ABS(0x0CE0)); nz8((u8)Y);
        if (!fz) wr8(ABS(0x0990), (u8)A);
        wr8(ABS(0x0CE0), (u8)A);
        wr8(ABS(0x0CDF), (u8)A);
        push8(0x80); push16(0xBEA9); ResetCoinMarkers_m1x1();
        lda8(0x80);
        push8(0x80); push16(0xBEAF); QueueSound_m1x1();
        lda8(0x02);
    }
    lda8(0x08);
    wr8(DPX(0x45), 0x08);
    ret();
}

void VelocityDirection_m1x1(void);

/* $81:9464 (RTL): run the four falling-sprite slots (X = 6, 4, 2, 0) by
 * their step $0110 through the table at $81:9478 (Y = slot). */
void StepSpriteSlots_m1x1(void)
{
    push8((u8)X);
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x0110)));
        lda8(asl8((u8)A));
        push8((u8)X);
        Y = X; nz8((u8)Y);
        X = (u8)A; nz8((u8)X);
        push16(0x9470);
        r816_call_pc(0x810000u | rd16(LONGX(0x819478)));
        X = pull8(); nz8((u8)X);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    X = pull8(); nz8((u8)X);
    retl();
}

/* $81:9778 (RTS): an object following its path ($81:9B66, 4 bytes per
 * point by $0319): SteerObjectStep (Y = $0C, A = $20); at each point the
 * next one, and at the $8000 end mark $0319 = $60 and the state steps.
 * Otherwise its map tile (when $027A is 0) and its direction. */
void FollowObjectPath_m1x1(void)
{
    rep(0x30);
    X = rd16(ABS(0x039D)); nz16(X);
    lda16(rd16(ABSX(0x0319)));
    lda16(asl16(A)); lda16(asl16(A));
    Y = A; nz16(Y);
    lda16(rd16(ABSY(0x9B66)));
    wr16(ABS(0x0399), A);
    lda16(rd16(ABSY(0x9B68)));
    wr16(ABS(0x039B), A);
    sep(0x30);
    ldy8(0x0C);
    lda8(0x20);
    push8(0x81); push16(0x9798); SteerObjectStep_m1x1();
    if (fc) {
        rep(0x30);
        wr16(ABSX(0x0319), inc16(rd16(ABSX(0x0319))));
        lda16(rd16(ABSX(0x0319)));
        lda16(asl16(A)); lda16(asl16(A));
        Y = A; nz16(Y);
        lda16(rd16(ABSY(0x9B66)));
        cmp16(A, 0x8000);
        sep(0x30);
        if (fz) {
            rep(0x20);
            lda16(0x0060);
            wr16(ABSX(0x0319), 0x0060);
            sep(0x20);
            wr8(ABSX(0x0309), inc8(rd8(ABSX(0x0309))));
            ret();
            return;
        }
    }
    lda8(rd8(ABSX(0x027A)));
    if (fz) { push8(0x81); push16(0x97C6); QueueMapTile_m1x1(); }
    push8(0x81); push16(0x97CA); VelocityDirection_m1x1();
    ret();
}

void PlayerOverheatWarn_m1x1(void);

/* The player's piece pointer ($03),Y + $0180 after RiderLanePieceAt. */
static void player_piece1(u16 back)
{
    push16(back); RiderLanePieceAt_m1x1();
    rep(0x30);
    lda16(Y);
    fc = 0; adc16(0x0180);
    Y = A; nz16(Y);
    sep(0x20);
}

static u32 piece_addr1(void)
{
    return (((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF;
}

/* Stadium 1's course map slot: the column $54 back from $A6 (wrapping by
 * $A8) through $7F:6000; X = entry, Y = the player's lane, 8-bit A. */
static void course_map_slot1(void)
{
    rep(0x30);
    lda16(rd16(DP(0xA6)));
    fc = 1; sbc16(0x0054);
    if (fn) { fc = 0; adc16(rd16(DP(0xA8))); }
    X = A; nz16(X);
    lda16(rd16(LONGX(0x7F6000)));
    cmp16(A, rd16(LONGX(0x7F5FFE)));
    if (!fz) lda16(dec16(A));
    X = A; nz16(X);
    lda16(rd16(ABS(0x08A4)));
    and16(0x00FF);
    Y = A; nz16(Y);
    sep(0x20);
}

/* $80:E9F3 UseArrowPiece (RTS, X = rider): for the player on the ground in
 * a lane with an arrow ($80:EA63): sound $32, $1979 = 1, and an arrow
 * piece ($15) becomes used ($12), its course map entry the lane's
 * $80:EA5C and $AF from $80:E9EC. Then PlayerOverheatWarn. (PHX is 8-bit
 * and PLX 16-bit, as in the original.) */
void UseArrowPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(0x01E8)));
        if (fz) {
            Y = rd8(ABS(0x08A4)); nz8((u8)Y);
            lda8(rd8(ABSY(0xEA63)));
            if (!fz) {
                lda8(0x32);
                push8(0x80); push16(0xEA09); QueueSound_m1x1();
                lda8(0x01); wr8(ABS(0x1979), 0x01);
                player_piece1(0xEA11);
                lda8(rd8(piece_addr1()));
                cmp8((u8)A, 0x15);
                if (fz) {
                    lda8(0x12);
                    wr8(piece_addr1(), 0x12);
                    push8((u8)X);
                    course_map_slot1();
                    lda8(rd8(ABSY(0xEA5C)));
                    wr8(LONGX(0x7F4000), (u8)A);
                    lda8(rd8(ABSY(0xE9EC)));
                    wr8(DP(0xAF), (u8)A);
                    X = pull16(); nz16(X);
                }
            }
        }
    }
    sep(0x30);
    PlayerOverheatWarn_m1x1();
}

/* $80:EA6A CollectPickupPiece (RTS, X = rider): the player on the ground
 * gets a spark, and in a lane with a pickup ($80:EACA) sound $47 and the
 * pickup piece ($16) is cleared to flat ($1F), its course map entry $40.
 * (PHX 8-bit, PLX 16-bit, as in the original.) */
void CollectPickupPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(0x01E8)));
        if (fz) {
            push8(0x80); push16(0xEA76); SpawnPlayerSpark_m1x1();
            Y = rd8(ABS(0x08A4)); nz8((u8)Y);
            lda8(rd8(ABSY(0xEACA)));
            if (!fz) {
                lda8(0x47);
                push8(0x80); push16(0xEA84); QueueSound_m1x1();
                player_piece1(0xEA87);
                lda8(rd8(piece_addr1()));
                cmp8((u8)A, 0x16);
                if (fz) {
                    lda8(0x1F);
                    wr8(piece_addr1(), 0x1F);
                    push8((u8)X);
                    course_map_slot1();
                    lda8(0x40);
                    wr8(LONGX(0x7F4000), 0x40);
                    X = pull16(); nz16(X);
                }
            }
        }
    }
    sep(0x30);
    ret();
}

/* $80:916B (RTS): start the player select: two-player mode $1E8B = 2
 * when the header flag $81:800C is set or two of the seven save names
 * ($11:5719) are filled, else 0; the four menu bytes $1547 from $80:91C9
 * (one player) or $80:91C5; next mode, $1923 = 1, BG1SC = 1, pads
 * cleared, Mode01; $88 = $0708 and $7F:4000 = 0. */
void PlayerSelectStart_m1x1(void)
{
    lda8(rd8(LONG(0x81800C)));
    int two = !fz;
    if (!two) {
        ldx8(0x06);
        ldy8(0x00);
        wr8(ABS(0x1E8B), 0);
        do {
            lda8(rd8(LONGX(0x115719)));
            if (!fz) Y = inc8((u8)Y);
            X = dec8((u8)X);
        } while (!fn);
        cmp8((u8)Y, 0x02);
        two = fc;
    }
    if (two) { lda8(0x02); wr8(ABS(0x1E8B), 0x02); }
    ldx8(0x03);
    do {
        lda8(rd8(ABS(0x1E8B)));
        if (fz) lda8(rd8(ABSX(0x91C9)));
        else lda8(rd8(ABSX(0x91C5)));
        wr8(ABSX(0x1547), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(0x75), inc8(rd8(DP(0x75))));
    lda8(0x01); wr8(ABS(0x1923), 0x01);
    lda8(0x01); wr8(ABS(0x2107), 0x01);
    wr8(DP(0x20), 0); wr8(DP(0x24), 0);
    push16(0x91B2); Mode01_m1x1();
    rep(0x20);
    lda16(0x0708); wr16(ABS(0x0088), 0x0708);
    lda16(0x0000); wr16(LONG(0x7F4000), 0);
    sep(0x20);
    ret();
}

void sub_80EE5C_m1x1(void);
void sub_80A2B0_m1x1(void);
void MergeBsxFlags_m1x1(void);
void SumBytes0850_m1x1(void);

/* $80:B158 (RTS): the race's per-frame bookkeeping: $7F = frame & 7, the
 * rider pieces, and while racing for real (no $0CD7, $1923 clear, mode
 * below 9) the pace bonus, the score and $80:EE5C. */
void RaceBookkeeping_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x07);
    wr8(DP(0x7F), (u8)A);
    push16(0xB160); ReadRiderPieces_m1x1();
    lda8(rd8(ABS(0x0CD7)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x1923)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x76)));
    cmp8((u8)A, 0x09);
    if (fc) { ret(); return; }
    push16(0xB173); PaceBonus_m1x1();
    push16(0xB176); AddScore_m1x1();
    push16(0xB179); CALL(sub_80EE5C_m1x1, 0x80EE5C, 2);
    ret();
}

/* $80:A0BC (RTL): pack the entry (code, record time, entry number,
 * $80:A2B0, BS-X flags, checksum) and show the twelve packed nibbles
 * $0850 as digit tiles (high and low rows, $2C in front) at VRAM $08B1. */
void ShowEntryPassword_m1x1(void)
{
    push8(0x80); push16(0xA0BF); PackEntryCode_m1x1();
    push8(0x80); push16(0xA0C3); PackRecordTime_m1x1();
    push8(0x80); push16(0xA0C7); PackEntryNumber_m1x1();
    push8(0x80); push16(0xA0CB); CALL(sub_80A2B0_m1x1, 0x80A2B0, 3);
    push8(0x80); push16(0xA0CF); MergeBsxFlags_m1x1();
    push8(0x80); push16(0xA0D3); SumBytes0850_m1x1();
    ldx8(0x0B);
    do {
        lda8(rd8(ABSX(0x0850)));
        and8(0x30);
        lda8(asl8((u8)A));
        fc = 0; adc8(0x20);
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(0x0850)));
        and8(0x0F);
        ora8(rd8(DP(0x00)));
        wr8(ABSX(0x03D3), (u8)A);
        ora8(0x10);
        wr8(ABSX(0x03DF), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x2C); wr8(ABS(0x03D2), 0x2C);
    rep(0x20);
    lda16(0x08B1); wr16(DP(0x00), 0x08B1);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push16(0xA10C); VramQueueTiles_m1x1();
    retl();
}

void DmaKick_m0x1(void);
void ClearTilemaps_m1x1(void);

/* $80:A4FE FinalSchedule (RTS): with a final ($084C) the record clock
 * $083A-$083C goes to $03B1, the clock ticks against the final's target
 * ($80:972B), and on even frames fourteen line-buffer entries before
 * $7F:0000's end become $24; $88 = 1. */
void FinalSchedule_m1x1(void)
{
    lda8(rd8(ABS(0x084C)));
    if (!fz) {
        lda8(rd8(ABS(0x083A))); wr8(ABS(0x03B1), (u8)A);
        lda8(rd8(ABS(0x083B))); wr8(ABS(0x03B2), (u8)A);
        lda8(rd8(ABS(0x083C))); wr8(ABS(0x03B3), (u8)A);
        lda8(rd8(ABS(0x084C)));
        lda8(dec8((u8)A));
        lda8(asl8((u8)A));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x972B))); wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(0x972C))); wr8(DP(0x01), (u8)A);
        push16(0xA527); TickRaceClock_m1x1();
        lda8(rd8(DP(0x63)));
        and8(0x01);
        if (fz) {
            lda8(rd8(LONG(0x7F0000)));
            fc = 1; sbc8(0x27);
            X = (u8)A; nz8((u8)X);
            lda8(0x24);
            static const u8 off[14] = { 0,2,4,6,8,0x0A,0x0C,0x1A,0x1C,0x1E,0x20,0x22,0x24,0x26 };
            for (int i = 0; i < 14; i++) wr8(LONGX(0x7F0000 + off[i]), 0x24);
        }
    }
    lda8(0x01); wr8(ABS(0x0088), 0x01);
    ret();
}

/* $81:A620 SetHdmaPreset (RTL, A = preset): HDMA off and its state
 * ($190F, $1907-$190C) cleared, $190D = $20. Preset $FF stops there.
 * Otherwise the preset's three line tables ($81:A6EB/$A77D/$A80F from
 * $81:A608, length $81:A614) go to $0695/$0715/$0795 and its three value
 * tables ($81:A8A1/$A8E1/$A921 from $81:A60E, length $81:A61A) to
 * $0455/$0515/$05D5; channels 2-4 point at the line tables (mode $40 to
 * $2132), $0815 = 1, $190F = $1C, $1C00 cleared and $1915 + 1. */
void SetHdmaPreset_m1x1(void)
{
    wr8(ABS(0x420C), 0);
    wr8(ABS(0x190F), 0);
    wr8(ABS(0x190A), 0); wr8(ABS(0x190B), 0);
    wr8(ABS(0x1907), 0); wr8(ABS(0x1908), 0);
    wr8(ABS(0x1909), 0); wr8(ABS(0x190C), 0);
    ldx8(0x20); wr8(ABS(0x190D), 0x20);
    cmp8((u8)A, 0xFF);
    if (fz) { retl(); return; }
    push8(DB);
    push8(0x81);
    DB = pull8(); nz8(DB);
    rep(0x30);
    wr16(DP(0x00), A);
    and16(0x00FF);
    lda16(asl16(A));
    Y = A; nz16(Y);
    push16(Y);
    lda16(rd16(ABSY(0xA614))); wr16(DP(0x02), A);
    X = rd16(ABSY(0xA608)); nz16(X);
    Y = 0; nz16(Y);
    sep(0x20);
    do {
        lda8(rd8(ABSX(0xA6EB))); wr8(ABSY(0x0695), (u8)A);
        lda8(rd8(ABSX(0xA77D))); wr8(ABSY(0x0715), (u8)A);
        lda8(rd8(ABSX(0xA80F))); wr8(ABSY(0x0795), (u8)A);
        X = inc16(X);
        Y = inc16(Y);
        cmp16(Y, rd16(DP(0x02)));
    } while (!fz);
    rep(0x20);
    Y = pull16(); nz16(Y);
    lda16(rd16(ABSY(0xA61A))); wr16(DP(0x02), A);
    X = rd16(ABSY(0xA60E)); nz16(X);
    Y = 0; nz16(Y);
    sep(0x20);
    do {
        lda8(rd8(ABSX(0xA8A1))); wr8(ABSY(0x0455), (u8)A);
        lda8(rd8(ABSX(0xA8E1))); wr8(ABSY(0x0515), (u8)A);
        lda8(rd8(ABSX(0xA921))); wr8(ABSY(0x05D5), (u8)A);
        X = inc16(X);
        Y = inc16(Y);
        cmp16(Y, rd16(DP(0x02)));
    } while (!fz);
    X = 0x3240; nz16(X);
    wr16(ABS(0x4320), 0x3240); wr16(ABS(0x4330), 0x3240); wr16(ABS(0x4340), 0x3240);
    X = 0x0695; nz16(X); wr16(ABS(0x4322), 0x0695);
    X = 0x0715; nz16(X); wr16(ABS(0x4332), 0x0715);
    X = 0x0795; nz16(X); wr16(ABS(0x4342), 0x0795);
    lda8(0x00);
    wr8(ABS(0x4324), 0); wr8(ABS(0x4334), 0); wr8(ABS(0x4344), 0);
    wr8(ABS(0x4327), 0); wr8(ABS(0x4337), 0); wr8(ABS(0x4347), 0);
    lda8(0x01); wr8(ABS(0x0815), 0x01);
    lda8(0x1C); wr8(ABS(0x190F), 0x1C);
    sep(0x10);
    wr8(ABS(0x1C00), 0); wr8(ABS(0x1C01), 0);
    wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:B538 (RTS): the four riders' palette words $1E12-$1E18 (cleared in
 * modes without them, $80:B5F5, or with $1923): per rider Y = 3-0 its
 * colour byte $1961 (the player's $15 swapped for $80:B658 by its
 * $091C), bit 0 to the high byte, bits 7-1 doubled into the high byte,
 * plus the stadium row $80:B618; a cycling flash ($1E0E: every other
 * frame $1E0A = $80:B638 + $8000, $1E10 = $4F00 + phase); and the
 * stadium's four bytes $80:B608 to $1543-$1546. */
void RiderPaletteWords_m1x1(void)
{
    lda8(rd8(ABS(0x1923)));
    int clear = !fz;
    if (!clear) {
        X = rd8(DP(0x76)); nz8((u8)X);
        lda8(rd8(ABSX(0xB5F5)));
        clear = fz;
    }
    if (clear) {
        rep(0x20);
        wr16(ABS(0x1E12), 0); wr16(ABS(0x1E14), 0);
        wr16(ABS(0x1E16), 0); wr16(ABS(0x1E18), 0);
        sep(0x20);
        ret();
        return;
    }
    rep(0x20);
    ldx8(0x06);
    ldy8(0x03);
    do {
        lda16(rd16(ABSY(0x1961)));
        wr16(DP(0x0E), A);
        cmp8((u8)Y, 0x00);
        if (fz) {
            and16(0x00FF);
            cmp16(A, 0x0015);
            if (fz) {
                lda16(rd16(ABSY(0x091C)));
                push8((u8)X);
                X = (u8)A; nz8((u8)X);
                lda16(rd16(LONGX(0x80B658)));
                X = pull8(); nz8((u8)X);
                wr16(DP(0x0E), A);
            }
        }
        and16(0x0001);
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        wr16(DP(0x00), A);
        wr16(ABS(0x000C), 0);
        wr8(DP(0x0C), (u8)X);
        lda16(rd16(ABS(0x1542)));
        and16(0x00FF);
        lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
        fc = 0; adc16(rd16(DP(0x0C)));
        X = (u8)A; nz8((u8)X);
        lda16(rd16(DP(0x0E)));
        and16(0x00FE);
        lda16(asl16(A));
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        fc = 0; adc16(rd16(DP(0x00)));
        fc = 0; adc16(rd16(ABSX(0xB618)));
        X = rd8(DP(0x0C)); nz8((u8)X);
        wr16(ABSX(0x1E12), A);
        X = dec8((u8)X);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
    } while (!fn);
    wr16(ABS(0x1E10), 0);
    wr16(ABS(0x1E0E), inc16(rd16(ABS(0x1E0E))));
    lda16(rd16(ABS(0x1E0E)));
    and16(0x0001);
    if (fz) {
        lda16(rd16(ABS(0x1E0E)));
        and16(0x001E);
        X = (u8)A; nz8((u8)X);
        lda16(rd16(ABSX(0xB638)));
        fc = 0; adc16(0x8000);
        wr16(ABS(0x1E0A), A);
        lda16(X);
        and16(0x0006);
        lda16(lsr16(A));
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        fc = 0; adc16(0x4F00);
        wr16(ABS(0x1E10), A);
    }
    sep(0x20);
    lda8(rd8(ABS(0x1542)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xB608))); wr8(ABS(0x1543), (u8)A);
    lda8(rd8(ABSX(0xB609))); wr8(ABS(0x1544), (u8)A);
    lda8(rd8(ABSX(0xB60A))); wr8(ABS(0x1545), (u8)A);
    lda8(rd8(ABSX(0xB60B))); wr8(ABS(0x1546), (u8)A);
    ret();
}

void DrawAllRiders_m1x1(void);

/* $80:D813 (RTS): set up Stadium 1's jump contest: the lane rows ($09D3
 * ramps $FC, three empty rows, six scenery rows $3B); the streamer start;
 * the player at column 9, X $30, angle 6 ($90 = 2, $1905 = $17, HDMA
 * preset 1), drawn once at ground $B0 then $B8; HDMA channels 7/6
 * ($80:D8ED -> $210F, $80:D8F4 -> $2108); $88 = $012C, $1B00/$1B02
 * cleared, $1B20 = 9, $1B21 = 8 and $190F = $DC. */
void SetupJumpContest_m1x1(void)
{
    ldx8(0x3F);
    do {
        lda8(0xFC); wr8(ABSX(0x09D3), 0xFC);
        lda8(0x00);
        wr8(ABSX(0x0A13), 0); wr8(ABSX(0x0A53), 0); wr8(ABSX(0x0A93), 0);
        lda8(0x3B);
        wr8(ABSX(0x0B53), 0x3B); wr8(ABSX(0x0B93), 0x3B); wr8(ABSX(0x0BD3), 0x3B);
        wr8(ABSX(0x0C13), 0x3B); wr8(ABSX(0x0C53), 0x3B); wr8(ABSX(0x0C93), 0x3B);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x34); wr8(ABS(0x0270), 0x34);
    lda8(0x0A); wr8(ABS(0x0273), 0x0A);
    lda8(0x05); wr8(ABS(0x0272), 0x05);
    lda8(0x74); wr8(ABS(0x0271), 0x74);
    lda8(0x08); wr8(ABS(0x0188), 0x08);
    lda8(0x09); wr8(ABS(0x0260), 0x09);
    lda8(0x30); wr8(ABS(0x0190), 0x30);
    lda8(0x06); wr8(ABS(0x01E0), 0x06);
    lda8(0x01);
    wr8(ABS(0x0198), 0x01); wr8(ABS(0x01D8), 0x01); wr8(ABS(0x0958), 0x01);
    lda8(asl8((u8)A));
    wr8(DP(0x90), (u8)A);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    lda8(0x01);
    push8(0x80); push16(0xD87C); SetHdmaPreset_m1x1();
    ldx8(0x00);
    lda8(0xB0); wr8(ABS(0x0CD5), 0xB0); wr8(ABS(0x01B0), 0xB0);
    push16(0xD889); DrawAllRiders_m1x1();
    lda8(0xB8); wr8(ABS(0x0CD5), 0xB8); wr8(ABS(0x01B0), 0xB8);
    static const u8 ch[12][2] = {
        {0x70,0x42},{0x71,0x0F},{0x72,0xED},{0x73,0xD8},{0x74,0x80},{0x77,0x00},
        {0x60,0x40},{0x61,0x08},{0x62,0xF4},{0x63,0xD8},{0x64,0x80},{0x67,0x00} };
    for (int i = 0; i < 12; i++) { lda8(ch[i][1]); wr8(ABS(0x4300 | ch[i][0]), ch[i][1]); }
    rep(0x20);
    lda16(0x012C); wr16(DP(0x88), 0x012C);
    wr16(ABS(0x1B00), 0); wr16(ABS(0x1B02), 0);
    sep(0x20);
    lda8(0x09); wr8(ABS(0x1B20), 0x09);
    lda8(0x08); wr8(ABS(0x1B21), 0x08);
    lda8(0xDC); wr8(ABS(0x190F), 0xDC);
    ret();
}

/* Stadium 1's rider jump steps ($80:E90D-$80:E9B1), run by the piece
 * interpreter's table $80:E701. */

/* $80:E975 (RTS, X = rider): $01A0 = $0981 = A, then A = $0248 - $0238. */
static void set_rider_pair1(void)
{
    wr8(ABSX(0x01A0), (u8)A);
    wr8(ABSX(0x0981), (u8)A);
    lda8(rd8(ABSX(0x0248)));
    fc = 1; sbc8(rd8(ABSX(0x0238)));
    ret();
}

void SetRiderPair_m1x1(void) { set_rider_pair1(); }

/* $80:E929 (RTS): jump height $01F8 and its base $0268 = A. */
static void set_jump_base1(void)
{
    wr8(ABSX(0x01F8), (u8)A);
    wr8(ABSX(0x0268), (u8)A);
    ret();
}

void SetJumpBase_m1x1(void) { set_jump_base1(); }

/* $80:E94C (RTS): jump height $01F8 = $0268 - A. */
static void height_below1(void)
{
    wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(0x0268)));
    fc = 1; sbc8(rd8(DP(0x00)));
    wr8(ABSX(0x01F8), (u8)A);
    ret();
}

void HeightBelow_m1x1(void) { height_below1(); }

static void pair1(u8 v, u16 back)
{
    lda8(v);
    push16(back); set_rider_pair1();
}

/* $80:E91E (RTS): $0240 = $80, SetRiderPair(5), the jump base at half. */
void RiderKickUp_m1x1(void)
{
    lda8(0x80); wr8(ABSX(0x0240), 0x80);
    pair1(0x05, 0xE927);
    lda8(lsr8((u8)A));
    set_jump_base1();
}

/* $80:E930 (RTS): SetRiderPair(2), the height half the pair below. */
void RiderHalfDrop_m1x1(void)
{
    pair1(0x02, 0xE934);
    lda8(lsr8((u8)A));
    height_below1();
}

/* $80:E939 (RTS): SetRiderPair(6), the jump base there, $0240 = $60. */
void RiderHighJump_m1x1(void)
{
    pair1(0x06, 0xE93D);
    push16(0xE940); set_jump_base1();
    lda8(0x60); wr8(ABSX(0x0240), 0x60);
    ret();
}

/* $80:E947 (RTS): SetRiderPair(1), then HeightBelow. */
void RiderPairHeight_m1x1(void)
{
    pair1(0x01, 0xE94B);
    height_below1();
}

/* $80:E958 (RTS): SetRiderPair(4), the jump base at twice the pair
 * (+$10 more when $0200 is set), $0240 = $40. */
void RiderJumpBase_m1x1(void)
{
    pair1(0x04, 0xE95C);
    lda8(asl8((u8)A));
    push16(0xE960); set_jump_base1();
    lda8(rd8(ABSX(0x0200)));
    if (!fz) {
        lda8(rd8(ABSX(0x01F8)));
        fc = 0; adc8(0x10);
        push16(0xE96E); set_jump_base1();
    }
    lda8(0x40); wr8(ABSX(0x0240), 0x40);
    ret();
}

/* $80:E983 (RTS): unless in jump state 1, SetRiderPair(3) and the height
 * twice the pair below; in state 4 it crash-lands into state 2 ($08AC
 * too). */
void RiderDropStep_m1x1(void)
{
    lda8(rd8(ABSX(0x0208)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) { ret(); return; }
    }
    pair1(0x03, 0xE990);
    lda8(asl8((u8)A));
    push16(0xE994); height_below1();
    lda8(rd8(ABSX(0x0208)));
    cmp8((u8)A, 0x04);
    if (!fz) { ret(); return; }
    push16(0xE99E); CrashLand_m1x1();
    lda8(0x02);
    wr8(ABSX(0x0208), 0x02);
    wr8(ABSX(0x08AC), 0x02);
    ret();
}

/* $80:E9A8 (RTS): jump state $0208 and $0200 cleared. */
void ClearJumpState_m1x1(void)
{
    lda8(0x00);
    wr8(ABSX(0x0208), 0);
    wr8(ABSX(0x0200), 0);
    ret();
}

/* $80:E9B1 (RTS): jump state 1, or 2 when the rider's lane $01F0 is
 * below $20. */
void StartJumpState_m1x1(void)
{
    lda8(0x01); wr8(ABSX(0x0208), 0x01);
    lda8(rd8(ABSX(0x01F0)));
    cmp8((u8)A, 0x20);
    if (!fc) wr8(ABSX(0x0208), inc8(rd8(ABSX(0x0208))));
    ret();
}

/* $80:E90F (RTS, X = last rider): riders X-0 on the ground get their
 * height $01B0 from GroundHeightAbove. */
void RiderGroundHeightsFrom_m1x1(void)
{
    do {
        lda8(rd8(ABSX(0x01E8)));
        if (fz) {
            push16(0xE916); GroundHeightAbove_m1x1();
            wr8(ABSX(0x01B0), (u8)A);
        }
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:E90D (RTS): the same for all eight riders. */
void RiderGroundHeights_m1x1(void)
{
    ldx8(0x07);
    RiderGroundHeightsFrom_m1x1();
}

/* $80:E834 (RTS, A = action): jump through the rider piece action table
 * $80:E701. Not registered on its own: difftest feeds it out-of-range
 * actions, which read past the table. */
static void piece_action1(void)
{
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xE701))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xE702))); wr8(DP(0x01), (u8)A);
    if (!r816_call_pc(0x800000u | rd16(LONG(0x000000))))
        r816_unknown_target(0x80E840, rd16(LONG(0x000000)));
}

/* ($0A),Y in bank DB. */
static u8 script_byte1(void)
{
    return rd8((((u32)DB << 16) + rd16(DP(0x0A)) + Y) & 0xFFFFFF);
}

/* $80:E7AC (RTS, X = rider): the rider's piece script. Its distance
 * $0248 advances by the speed $0180. With no script running ($0170 = 0)
 * a piece type $0218 below $18 starts script type + 1 (a $18+ piece in
 * jump state 3 in lanes 0-2 starts $0C + 1; otherwise the player's $1979
 * clears); $0250 = 0 and $0248 = $0188 - 1. Then the script ($80:E574 /
 * $80:E58B by script) runs pairs (distance, action) up to $0248: an
 * action with a $80:E700 bit sets the rider's angle ($01E0 = low nibble,
 * $01A0 from $80:E6F5) when it is on the ground and not crashed; one with
 * a $80:E6FF bit sets $0220 / $0238; others go through PieceAction. At
 * the $FF end the script stops ($0170, $01A0 cleared, $01F8 cleared on
 * the ground outside jump state 1, state 2 -> 3) and $08BC clears. */
void RunRiderPieces_m1x1(void)
{
    lda8(rd8(ABSX(0x0180)));
    fc = 0; adc8(rd8(ABSX(0x0248)));
    wr8(ABSX(0x0248), (u8)A);
    cmp8((u8)X, 0x00);
    if (fz) lda8(rd8(ABSX(0x0170)));
    lda8(rd8(ABSX(0x0170)));
    if (fz) {
        lda8(rd8(ABSX(0x0218)));
        cmp8((u8)A, 0x18);
        if (fc) {
            int start = 0;
            lda8(rd8(ABSX(0x0208)));
            cmp8((u8)A, 0x03);
            if (fz) {
                lda8(rd8(ABSX(0x08A4)));
                cmp8((u8)A, 0x03);
                if (!fc) {
                    lda8(0x0C);
                    wr8(ABSX(0x0218), 0x0C);
                    start = 1;
                }
            }
            if (!start) {
                cmp8((u8)X, 0x00);
                if (fz) wr8(ABS(0x1979), 0);
                ret();
                return;
            }
        }
        wr8(ABSX(0x0170), (u8)A);
        wr8(ABSX(0x0170), inc8(rd8(ABSX(0x0170))));
        lda8(0x00);
        wr8(ABSX(0x0250), 0);
        lda8(rd8(ABSX(0x0188)));
        wr8(ABSX(0x0248), (u8)A);
        wr8(ABSX(0x0248), dec8(rd8(ABSX(0x0248))));
    }
    for (;;) {
        /* L80E7FA */
        Y = rd8(ABSX(0x0170)); nz8((u8)Y);
        lda8(rd8(ABSY(0xE574))); wr8(DP(0x0A), (u8)A);
        lda8(rd8(ABSY(0xE58B))); wr8(DP(0x0B), (u8)A);
        Y = rd8(ABSX(0x0250)); nz8((u8)Y);
        lda8(script_byte1());
        cmp8((u8)A, 0xFF);
        if (fz) break;
        wr8(DP(0x0F), (u8)A);
        cmp8((u8)A, rd8(ABSX(0x0248)));
        if (!fz && fc) { ret(); return; }
        Y = inc8((u8)Y);
        lda8(script_byte1());
        bit8(rd8(ABS(0xE700)), 0);
        if (!fz) {
            /* L80E843 */
            lda8(rd8(ABSX(0x01E8)));
            ora8(rd8(ABSX(0x01C8)));
            if (fz) {
                lda8(script_byte1());
                and8(0x0F);
                wr8(DP(0x00), (u8)A);
                lda8(rd8(ABSX(0x0208)));
                cmp8((u8)A, 0x01);
                if (!fz) {
                    lda8(rd8(DP(0x00)));
                    wr8(ABSX(0x01E0), (u8)A);
                    lda8(rd8(ABSX(0x0170)));
                    cmp8((u8)A, 0x03);
                    if (!fz) {
                        lda8(rd8(ABSX(0x01E0)));
                        fc = 1; sbc8(0x02);
                        Y = (u8)A; nz8((u8)Y);
                        lda8(rd8(ABSY(0xE6F5)));
                        wr8(ABSX(0x01A0), (u8)A);
                    }
                }
            }
        } else {
            bit8(rd8(ABS(0xE6FF)), 0);
            if (fz) {
                push16(0xE82B); piece_action1();
                wr8(ABSX(0x0250), inc8(rd8(ABSX(0x0250))));
                wr8(ABSX(0x0250), inc8(rd8(ABSX(0x0250))));
                if (!fz) continue;
                piece_action1();
                return;
            }
            /* L80E8A5 */
            and8(0x0F);
            wr8(ABSX(0x0220), (u8)A);
            lda8(rd8(DP(0x0F)));
            wr8(ABSX(0x0238), (u8)A);
            if (fn) { RiderLanePieceAt_m1x1(); return; }
        }
        /* L80E871 */
        wr8(ABSX(0x0250), inc8(rd8(ABSX(0x0250))));
        wr8(ABSX(0x0250), inc8(rd8(ABSX(0x0250))));
        if (fz) break;
    }
    /* L80E87C */
    lda8(0x00);
    wr8(ABSX(0x0170), 0);
    wr8(ABSX(0x01A0), 0);
    lda8(rd8(ABSX(0x0200)));
    if (fz) {
        lda8(rd8(ABSX(0x0208)));
        cmp8((u8)A, 0x01);
        if (!fz) {
            lda8(0x00);
            wr8(ABSX(0x01F8), 0);
            lda8(rd8(ABSX(0x0208)));
            cmp8((u8)A, 0x02);
            if (fz) wr8(ABSX(0x0208), inc8(rd8(ABSX(0x0208))));
        }
    }
    lda8(0x00);
    wr8(ABSX(0x08BC), 0);
    ret();
}

void BlankQueueRow_m1x1(void);
void UpdateObjects_m1x1(void);

/* $80:EDB6 (RTS): A + 1, wrapping at $40. */
void NextColumn_m1x1(void)
{
    fc = 0; adc8(0x01);
    cmp8((u8)A, 0x40);
    if (fz) lda8(0x00);
    ret();
}

/* $80:EDB2 (RTL): NextColumn for other banks. */
void NextColumnLong_m1x1(void)
{
    push16(0xEDB4); NextColumn_m1x1();
    retl();
}

/* $80:EDC0 (RTL): AddScore for other banks. */
void AddScoreLong_m1x1(void)
{
    push16(0xEDC2); AddScore_m1x1();
    retl();
}

/* $80:EE58 (RTL): QueueScoreDigits for other banks. */
void QueueScoreDigitsLong_m1x1(void)
{
    push16(0xEE5A); QueueScoreDigits_m1x1();
    retl();
}

/* $80:DD03 (RTS, X = row type): the row's piece list ($05) from
 * $80:D4D0 / $80:D4D9. */
void CourseRowList_m1x1(void)
{
    lda8(rd8(ABSX(0xD4D0))); wr8(DP(0x05), (u8)A);
    lda8(rd8(ABSX(0xD4D9))); wr8(DP(0x06), (u8)A);
    ret();
}

/* $80:DCEA (entered with M either way): from Y + 2 on, the list bytes
 * ($05),Y as tiles $08xx into the course row buffer $7F:000E + X up to
 * X = $1E. */
static void course_row_tiles1(void)
{
    do {
        Y = inc8((u8)Y);
        Y = inc8((u8)Y);
        rep(0x20);
        lda16(rd16((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        and16(0x00FF);
        ora16(0x0800);
        wr16(LONGX(0x7F000E), A);
        X = inc8((u8)X);
        X = inc8((u8)X);
        cmp8((u8)X, 0x1E);
    } while (!fz);
    sep(0x20);
    ret();
}

static u8 course_list1(u32 dp)
{
    return rd8((((u32)DB << 16) + rd16(DP(dp)) + Y) & 0xFFFFFF);
}

/* $80:DB89 (RTS): build the course one column ahead. The course script
 * ($07 from $80:D445 / $80:D447 by the header flag $81:800C) starts with
 * the scenery byte $09C9. Each time the builder column $0273 reaches the
 * player column $0260: the next script entry (0 = stop, 9 = loop to
 * entry 1 as row type 1, $80+ = that many empty rows, else a row type
 * $0277) and its row list ($80:DD03) give the column pieces; a column
 * is blanked, filled ($80:DCEA) and queued for VRAM at $0271 - $20 (row
 * pointer $0270, column $0271 wrapping $80 -> $60 with $0272 ^ 4), and
 * the builder column steps. */
void BuildCourseColumn_m1x1(void)
{
    lda8(rd8(LONG(0x81800C)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xD445))); wr8(DP(0x07), (u8)A);
    lda8(rd8(ABSX(0xD447))); wr8(DP(0x08), (u8)A);
    Y = rd8(ABS(0x0275)); nz8((u8)Y);
    if (fz) {
        lda8(course_list1(0x07));
        wr8(ABS(0x09C9), (u8)A);
        Y = inc8((u8)Y);
        wr8(ABS(0x0275), (u8)Y);
    }
    lda8(rd8(ABS(0x0273)));
    cmp8((u8)A, rd8(ABS(0x0260)));
    if (!fz) { ret(); return; }
    int step;   /* 0 = next entry, 1 = counting empty rows, 2 = same row */
    lda8(rd8(ABS(0x0274)));
    if (!fz) step = 1;
    else {
        lda8(rd8(ABS(0x0278)));
        step = fz ? 0 : 2;
    }
    for (;;) {
        if (step == 0) {
            Y = rd8(ABS(0x0275)); nz8((u8)Y);
            wr8(ABS(0x0275), inc8(rd8(ABS(0x0275))));
            lda8(course_list1(0x07));
            if (fz) { ret(); return; }
            cmp8((u8)A, 0x09);
            if (fz) {
                lda8(0x01);
                wr8(ABS(0x0275), 0x01);
            }
            cmp8((u8)A, 0x80);
            if (fc) {
                and8(0x7F);
                wr8(ABS(0x0274), (u8)A);
                step = 1;
            } else {
                wr8(ABS(0x0277), (u8)A);
                step = 2;
            }
        }
        if (step == 1) {
            wr8(ABS(0x0274), dec8(rd8(ABS(0x0274))));
            lda8(0x00);
            wr8(ABS(0x09C3), 0);
            wr8(ABS(0x0278), 0);
            wr8(ABS(0x0277), 0);
        }
        /* L80DBE5 */
        X = rd8(ABS(0x0277)); nz8((u8)X);
        push16(0xDBEA); CourseRowList_m1x1();
        Y = rd8(ABS(0x0278)); nz8((u8)Y);
        lda8(course_list1(0x05));
        if (!fz) break;
        wr8(ABS(0x0278), (u8)A);
        wr8(ABS(0x09C3), (u8)A);
        step = 0;
    }
    push16(0xDBF7); BlankQueueRow_m1x1();
    lda8(course_list1(0x05));
    X = (u8)A; nz8((u8)X);
    push16(0xDBFD); course_row_tiles1();
    Y = inc8((u8)Y);
    Y = inc8((u8)Y);
    wr8(ABS(0x0278), (u8)Y);
    lda8(0x81); wr8(LONG(0x7F0004), 0x81);
    lda8(0x01); wr8(LONG(0x7F0005), 0x01);
    lda8(0x18); wr8(LONG(0x7F0006), 0x18);
    lda8(rd8(LONG(0x7F0000)));
    fc = 0; adc8(0x0C);
    wr8(LONG(0x7F0007), (u8)A);
    lda8(0x00); wr8(LONG(0x7F0008), 0);
    lda8(0x7F); wr8(LONG(0x7F0009), 0x7F);
    rep(0x20);
    lda16(rd16(LONG(0x7F0000)));
    fc = 0; adc16(0x002A);
    wr16(LONG(0x7F0000), A);
    wr16(LONG(0x7F000C), A);
    lda16(rd16(ABS(0x0271)));
    fc = 1; sbc16(0x0020);
    wr16(LONG(0x7F0002), A);
    lda16(0x001E); wr16(LONG(0x7F000A), 0x001E);
    sep(0x20);
    lda8(rd8(ABS(0x0270)));
    wr8(DP(0x0F), (u8)A);
    push16(0xDC59); CourseColumn_m1x1();
    lda8(rd8(ABS(0x0270)));
    push16(0xDC5F); NextColumn_m1x1();
    wr8(ABS(0x0270), (u8)A);
    wr8(ABS(0x0271), inc8(rd8(ABS(0x0271))));
    lda8(rd8(ABS(0x0271)));
    cmp8((u8)A, 0x80);
    if (fz) {
        lda8(0x60); wr8(ABS(0x0271), 0x60);
        lda8(rd8(ABS(0x0272)));
        eor8(0x04);
        wr8(ABS(0x0272), (u8)A);
    }
    lda8(rd8(ABS(0x0260)));
    push16(0xDC7F); NextColumn_m1x1();
    wr8(ABS(0x0273), (u8)A);
    ret();
}

/* $80:A576: the free run results wait: objects update; once the timer
 * $7F:FFFF reaches the stage $80:A5A8 value the next stage starts (mode
 * 2, $7C + 1, $7D = 0), or after stage 3 the end (mode 9, $1E87 - 10). */
static void free_run_wait1(void)
{
    push8(0x80); push16(0xA579); UpdateObjects_m1x1();
    X = rd8(DP(0x7C)); nz8((u8)X);
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, rd8(ABSX(0xA5A8)));
    if (!fc) { ret(); return; }
    wr8(DP(0x75), 0);
    wr8(DP(0x77), 0);
    wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    cmp8((u8)X, 0x03);
    if (!fc) {
        wr8(DP(0x7D), 0);
        wr8(DP(0x7C), inc8(rd8(DP(0x7C))));
        lda8(0x02); wr8(DP(0x76), 0x02);
    } else {
        lda8(0x09); wr8(DP(0x76), 0x09);
        lda8(rd8(ABS(0x1E87)));
        fc = 1; sbc8(0x0A);
        wr8(ABS(0x1E87), (u8)A);
    }
    ret();
}

/* $80:9698 Mode04_FreeRunSchedule (RTS): between free-run stages. After
 * the stage ($7D set) the results wait. Otherwise, until the timer
 * $7F:FFFF passes the stage $80:9727 value (then back to mode 2 with $7D
 * set), every $20 frames ($88) the next of the ten record rows ($0452)
 * shows: its time ($081C + 3 * row) to $03B1 with sound $42 and the
 * clock against $80:972B (an empty row skips ahead to the last), sound
 * $43 at the tenth; from row 10 on, FinalSchedule. */
void Mode04_FreeRunSchedule_m1x1(void)
{
    lda8(rd8(DP(0x7D)));
    if (!fz) { free_run_wait1(); return; }
    X = rd8(DP(0x7C)); nz8((u8)X);
    lda8(rd8(ABSX(0x9727)));
    cmp8((u8)A, rd8(LONG(0x7FFFFF)));
    if (!fc) {
        lda8(0x02); wr8(DP(0x76), 0x02);
        wr8(DP(0x75), 0);
        wr8(DP(0x77), 0);
        wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
        wr8(DP(0x7D), inc8(rd8(DP(0x7D))));
        ret();
        return;
    }
    wr8(ABS(0x0088), dec8(rd8(ABS(0x0088))));
    if (!fz) { ret(); return; }
    lda8(0x20); wr8(ABS(0x0088), 0x20);
    lda8(rd8(ABS(0x0452)));
    cmp8((u8)A, 0x0A);
    if (fc) { push16(0x96C0); FinalSchedule_m1x1(); ret(); return; }
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    fc = 0; adc8(rd8(ABS(0x0452)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0x081C)));
    cmp8((u8)A, 0xFF);
    if (!fz) {
        wr8(ABS(0x03B1), (u8)A);
        lda8(rd8(ABSY(0x081D))); wr8(ABS(0x03B2), (u8)A);
        lda8(rd8(ABSY(0x081E))); wr8(ABS(0x03B3), (u8)A);
        lda8(0x42);
        push8(0x80); push16(0x96F2); QueueSound_m1x1();
        rep(0x20);
        lda16(rd16(ABSX(0x972B)));
        wr16(DP(0x00), A);
        sep(0x20);
        push16(0x96FE); TickRaceClock_m1x1();
    } else {
        lda8(rd8(ABS(0x0452)));
        if (fz) {
            lda8(0x0A); wr8(ABS(0x0452), 0x0A);
            lda8(0x01); wr8(ABS(0x0088), 0x01);
            ret();
            return;
        }
        lda8(0x09); wr8(ABS(0x0452), 0x09);
    }
    wr8(ABS(0x0452), inc8(rd8(ABS(0x0452))));
    lda8(rd8(ABS(0x0452)));
    cmp8((u8)A, 0x0A);
    if (fz) {
        lda8(0x43);
        push8(0x80); push16(0x9725); QueueSound_m1x1();
    }
    ret();
}

/* $81:9852 (RTS): Stadium 1's results bonus count-down object. Every 4
 * frames ($88) the bonus $B2 counts down by one into the score (sound
 * $44, the count at VRAM $1CB6, $09AF = 1, the score digits). At 0 it
 * hands the podium on: the player place (the slot where $086E is 1)
 * plus 8 * $154F picks the type-7 object state 3 (timer $10) and the
 * type from $81:9936 goes to state 4 with long timers; its own state
 * steps. */
void ResultsBonusCount_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) goto tick;
    lda8(rd8(DP(0xB2)));
    if (fz) {
        ldy8(0x0E);
        do {
            lda8(rd8(ABSY(0x086E)));
            lda8(dec8((u8)A));
            if (fz) break;
            Y = dec8((u8)Y);
            Y = dec8((u8)Y);
        } while (!fn);
        lda8((u8)Y);
        lda8(lsr8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABS(0x1542)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(0x03); wr8(DP(0x01), 0x03);
        lda8(0x07);
        push8(0x81); push16(0x9882); FindObjectSlot_m1x1();
        lda8(rd8(DP(0x01))); wr8(ABSY(0x0309), (u8)A);
        lda8(0x10); wr8(ABSY(0x0369), 0x10);
        X = rd8(ABS(0x039D)); nz8((u8)X);
        wr8(ABSX(0x0309), inc8(rd8(ABSX(0x0309))));
        Y = rd8(DP(0x00)); nz8((u8)Y);
        lda8(rd8(ABSY(0x9936)));
        push8(0x81); push16(0x989B); FindObjectSlot_m1x1();
        lda8(0x04); wr8(ABSY(0x0309), 0x04);
        lda8(0xFF); wr8(ABSY(0x0369), 0xFF);
        lda8(0xFF); wr8(ABSY(0x0379), 0xFF);
        goto tick;
    }
    wr8(DP(0xB2), dec8(rd8(DP(0xB2))));
    lda8(0x44);
    push8(0x81); push16(0x98B5); QueueSound_m1x1();
    ldy8(0x00);
    lda8(rd8(DP(0xB2)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
    }
    wr8(DP(0x00), (u8)A);
    ora8(0x50); wr8(ABS(0x03D5), (u8)A);
    eor8(0x30); wr8(ABS(0x03DA), (u8)A);
    cmp8((u8)Y, 0x00);
    if (!fz) {
        lda8((u8)Y);
        ora8(0x50); wr8(ABS(0x03D4), (u8)A);
        eor8(0x30); wr8(ABS(0x03D9), (u8)A);
    } else {
        lda8(0xFC); wr8(ABS(0x03D4), 0xFC); wr8(ABS(0x03D9), 0xFC);
    }
    lda8((u8)Y);
    ora8(rd8(DP(0x00)));
    if (fz) {
        lda8(0xFC);
        wr8(ABS(0x03D5), 0xFC); wr8(ABS(0x03D6), 0xFC);
        wr8(ABS(0x03DA), 0xFC); wr8(ABS(0x03DB), 0xFC);
    }
    lda8(0xFC); wr8(ABS(0x03D3), 0xFC); wr8(ABS(0x03D8), 0xFC);
    lda8(0x04); wr8(DP(0x88), 0x04);
    rep(0x20);
    lda16(0x1CB6); wr16(DP(0x00), 0x1CB6);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x991C); VramQueueTilesLong_m1x1();
    lda8(0x01); wr8(ABS(0x09AF), 0x01);
    push8(0x81); push16(0x9925); AddScoreLong_m1x1();
    wr8(DP(0x00), dec8(rd8(DP(0x00))));
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x992F); QueueScoreDigitsLong_m1x1();
tick:
    wr8(DP(0x88), dec8(rd8(DP(0x88))));
    X = rd8(ABS(0x039D)); nz8((u8)X);
    ret();
}

void SortRidersByPosition_m1x1(void);
void LoadCourse_m1x1(void);
void ClearWords1530_m1x1(void);
void MarkerObjects_m1x1(void);
void DrawStartFlags_m1x1(void);
void Random_m1x1(void);

/* $80:CC38 (RTS): sort the riders by position, then draw all eight. */
void SortAndDrawRiders_m1x1(void)
{
    push16(0xCC3A); SortRidersByPosition_m1x1();
    ldx8(0x07);
    DrawAllRiders_m1x1();
}

/* $80:A89C RacePlaceRiders (RTS): set up a Stadium 1 race: $1923 = 1, next step, pause
 * timer $54 (5 when clear), ground $A0; the eight riders' $0936 ($C0 down
 * by $10), $098B = $FF, $90 = $08C4 = 1, $098C = 2; the riders (only the
 * player in a free run, $7D clear) placed from $80:B839/$B841/$B849 at
 * angle 6 with $0228 from their pads; one CPU rider ($2C's low bits, 2
 * when 0) starts at half lean; ground heights, the riders drawn; a free
 * run's best time reset to 4:00:00; the best time and clock; the course
 * loaded; scores cleared (free run), $0990 = $20 with $0CE0; the record
 * rows cleared; marker objects and start flags ($E1); $9E = 4; and the
 * eight CPU colour pairs $0D01 ($80:A9AA) shuffled 16 times. */
void RacePlaceRiders_m1x1(void)
{
    lda8(0x01); wr8(ABS(0x1923), 0x01);
    wr8(DP(0x75), inc8(rd8(DP(0x75))));
    lda8(rd8(DP(0x54)));
    if (fz) lda8(0x05);
    wr8(DP(0x54), (u8)A);
    lda8(0xA0); wr8(ABS(0x0CD5), 0xA0);
    ldx8(0x07);
    lda8(0xC0);
    do {
        wr8(ABSX(0x0936), (u8)A);
        fc = 1; sbc8(0x10);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0xFF); wr8(ABS(0x098B), 0xFF);
    ldx8(0x01);
    wr8(DP(0x90), 0x01);
    wr8(ABS(0x08C4), 0x01);
    X = inc8((u8)X);
    wr8(ABS(0x098C), (u8)X);
    X = rd8(DP(0x7D)); nz8((u8)X);
    if (!fz) ldx8(0x07);
    do {
        lda8(rd8(ABSX(0xB841))); wr8(ABSX(0x01F0), (u8)A);
        lda8(0x08); wr8(ABSX(0x0188), 0x08);
        lda8(rd8(ABSX(0xB839))); wr8(ABSX(0x0260), (u8)A);
        lda8(rd8(ABSX(0xB849))); wr8(ABSX(0x0190), (u8)A);
        lda8(0x06); wr8(ABSX(0x01E0), 0x06); wr8(ABSX(0x0230), 0x06);
        lda8(0x01); wr8(ABSX(0x0198), 0x01); wr8(ABSX(0x01D8), 0x01);
        lda8(rd8(DPX(0x2D)));
        lda8(lsr8((u8)A));
        wr8(ABSX(0x0228), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(DPX(0x2D)));
    and8(0x03);
    if (fz) lda8(0x02);
    X = (u8)A; nz8((u8)X);
    wr8(ABSX(0x0230), lsr8(rd8(ABSX(0x0230))));
    push16(0xA911); RiderGroundHeights_m1x1();
    push16(0xA914); SortAndDrawRiders_m1x1();
    lda8(rd8(DP(0x7D)));
    if (fz) {
        lda8(0x04); wr8(ABS(0x083A), 0x04);
        wr8(ABS(0x083B), 0);
        wr8(ABS(0x083C), 0);
    }
    wr8(ABS(0x084C), 0);
    push8(0x80); push16(0xA92A); ShowCourseBest_m1x1();
    lda8(0x97); wr8(DP(0x00), 0x97);
    lda8(0x18); wr8(DP(0x01), 0x18);
    push16(0xA935); ShowRaceClock_m1x1();
    push8(0x80); push16(0xA939); LoadCourse_m1x1();
    push8(0x80); push16(0xA93D); ClearWords1530_m1x1();
    lda8(rd8(DP(0x7D)));
    if (fz) {
        wr8(ABS(0x0CE0), 0); wr8(ABS(0x0CDF), 0);
        wr8(ABS(0x1540), 0); wr8(ABS(0x1541), 0);
    }
    lda8(rd8(ABS(0x0CE0)));
    if (!fz) { lda8(0x20); wr8(ABS(0x0990), 0x20); }
    ldx8(0x1D);
    lda8(0xFF);
    do {
        wr8(ABSX(0x081C), 0xFF);
        X = dec8((u8)X);
    } while (!fn);
    wr8(ABS(0x190D), 0);
    push8(0x80); push16(0xA968); MarkerObjects_m1x1();
    ldy8(0xE1);
    push16(0xA96D); DrawStartFlags_m1x1();
    lda8(0x04); wr8(ABS(0x009E), 0x04);
    ldx8(0x0E);
    do {
        lda8(rd8(ABSX(0xA9AA))); wr8(ABSX(0x0D01), (u8)A);
        lda8(rd8(ABSX(0xA9AB))); wr8(ABSX(0x0D02), (u8)A);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x10); wr8(DP(0x00), 0x10);
    do {
        push8(0x80); push16(0xA98C); Random_m1x1();
        and8(0x0E);
        Y = (u8)A; nz8((u8)Y);
        push8(0x80); push16(0xA993); Random_m1x1();
        and8(0x0E);
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x0D01)));
        push8((u8)A);
        lda8(rd8(ABSY(0x0D01)));
        wr8(ABSX(0x0D01), (u8)A);
        lda8(pull8());
        wr8(ABSY(0x0D01), (u8)A);
        wr8(DP(0x00), dec8(rd8(DP(0x00))));
    } while (!fz);
    ret();
}

void RiderOnRamp_m1x1(void);
void AccelerateRider_m1x1(void);
void BrakeRider_m1x1(void);

/* $80:AE2F (RTS, X = rider, Y = how hard): BrakeRider on the rider whose
 * turn it is ($7F). */
void BrakeOnTurn_m1x1(void)
{
    cmp8((u8)X, rd8(DP(0x7F)));
    if (!fz) { ret(); return; }
    BrakeRider_m1x1();
}

/* $80:C31B: lean rider X one step toward angle A, every Y frames ($45,X). */
static void lean_toward1(void)
{
    wr8(DP(0x00), (u8)A);
    lda8(rd8(DPX(0x45)));
    if (!fz) { ret(); return; }
    lda8((u8)Y);
    wr8(DPX(0x45), (u8)A);
    lda8(rd8(ABSX(0x01E0)));
    cmp8((u8)A, rd8(DP(0x00)));
    if (fz) { ret(); return; }
    if (fc) {
        wr8(ABSX(0x01E0), dec8(rd8(ABSX(0x01E0))));
        wr8(ABSX(0x01E0), dec8(rd8(ABSX(0x01E0))));
    }
    wr8(ABSX(0x01E0), inc8(rd8(ABSX(0x01E0))));
    ret();
}

/* $80:AE62: the d-pad leans rider X (every $80:B86D frames by air state):
 * up ($00 bit 0) leans back to $80:B861; otherwise forward to $80:B863,
 * and holding a button past it on the ground keeps tipping until angle
 * $0D flips it ($01C8 = 1, $3D,X = $0A, sound $24 / $29). */
static void lean_by_pad1(void)
{
    lda8(rd8(DPX(0x45)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x01E8)));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xB86D)));
    wr8(DPX(0x45), (u8)A);
    {
        u8 v = rd8(DP(0x00));
        fc = v & 1;
        v >>= 1;
        wr8(DP(0x00), v);
        nz8(v);
    }
    if (fc) {
        lda8(rd8(ABSX(0x01E0)));
        cmp8((u8)A, rd8(ABSY(0xB861)));
        if (fz) { ret(); return; }
        if (!fc) { wr8(ABSX(0x01E0), inc8(rd8(ABSX(0x01E0)))); ret(); return; }
        wr8(ABSX(0x01E0), dec8(rd8(ABSX(0x01E0))));
        ret();
        return;
    }
    lda8(rd8(ABSX(0x08F4)));
    and8(0x02);
    wr8(ABSX(0x08F4), (u8)A);
    lda8(rd8(ABSX(0x01E0)));
    cmp8((u8)A, rd8(ABSY(0xB863)));
    if (!fc) { wr8(ABSX(0x01E0), inc8(rd8(ABSX(0x01E0)))); ret(); return; }
    lda8(rd8(ABSX(0x0178)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x01E8)));
    if (!fz) { ret(); return; }
    wr8(ABSX(0x01E0), inc8(rd8(ABSX(0x01E0))));
    lda8(0x0D); wr8(DPX(0x45), 0x0D);
    lda8(rd8(ABSX(0x01E0)));
    cmp8((u8)A, 0x0D);
    if (!fc) { ret(); return; }
    lda8(0x01); wr8(ABSX(0x01C8), 0x01);
    lda8(0x0A); wr8(DPX(0x3D), 0x0A);
    lda8((u8)X);
    if (!fz) lda8(0x05);
    fc = 0; adc8(0x24);
    push8(0x80); push16(0xAEC4); QueueSound_m1x1();
    ret();
}

/* $80:ABD7 RiderThrottle (RTS, X = rider): rider X's throttle and lean
 * from its pad $0178. Not crashed or held, on the ground with a button
 * down (A/B: $C0) it accelerates up to the top speed $80:B86A / $80:B867
 * (by button and, for the player, $84), with sound $3F when the player
 * starts from still; otherwise it brakes (BrakeOnTurn, Y = how hard).
 * Then, not crashed: in the air or fast enough ($A0) the d-pad leans it;
 * on the ground it settles toward the piece angle $08B4. */
void RiderThrottle_m1x1(void)
{
    lda8(rd8(ABSX(0x0178)));
    and8(0x03);
    wr8(DP(0x0A), (u8)A);
    ldy8(0x00);
    lda8(rd8(ABSX(0x08CC)));
    ora8(rd8(ABSX(0x01C8)));
    if (!fz) goto brake;
    lda8(rd8(ABSX(0x01D0)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) goto brake;
    }
    lda8((u8)X);
    if (fz) {
        ldy8(0x02);
        lda8(rd8(DP(0x84)));
        if (!fz) goto top;
        ldy8(0x00);
    }
    lda8(rd8(ABSX(0x01E8)));
    if (!fz) {
        ldy8(0x04);
        lda8(rd8(DP(0x0A)));
        if (fz) goto brake;
        Y = (u8)A; nz8((u8)Y);
        goto harder;
    }
    lda8(rd8(ABSX(0x0178)));
    and8(0xC0);
    if (fz) goto brake;
    wr8(DP(0x00), (u8)X);
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(0x01B8)));
        ora8(rd8(ABSX(0x01C0)));
        if (fz) {
            lda8(rd8(DP(0x21)));
            and8(0xC0);
            if (!fz) { lda8(0x3F); push8(0x80); push16(0xAC21); QueueSound_m1x1(); }
        }
    }
    lda8(rd8(ABSX(0x0178)));
    lda8(asl8((u8)A));
    if (!fc) wr8(DP(0x00), inc8(rd8(DP(0x00))));
    push16(0xAC2C); RiderOnRamp_m1x1();
    if (!fz) {
        lda8(rd8(ABSX(0x0208)));
        if (fz) {
            lda8(rd8(ABSX(0x01C0)));
            if (!fz) goto brake;
        }
    }
    Y = rd8(DP(0x00)); nz8((u8)Y);
    lda8((u8)X);
    if (!fz) goto accel;
top:
    lda8(rd8(ABSX(0x01C0)));
    cmp8((u8)A, rd8(ABSY(0xB86A)));
    if (!fc) goto accel;
    if (!fz) goto harder;
    lda8(rd8(ABSX(0x01B8)));
    cmp8((u8)A, rd8(ABSY(0xB867)));
    if (fz) goto after;
    if (fc) goto harder;
accel:
    push16(0xAC54); AccelerateRider_m1x1();
    goto after;
harder:
    Y = inc8((u8)Y);
brake:
    push16(0xAC62); BrakeOnTurn_m1x1();
after:
    lda8(rd8(ABSX(0x01C8)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x01E8)));
    int pad = !fz;
    if (!pad) {
        lda8(rd8(ABSX(0x0170)));
        ora8(rd8(DP(0x84)));
        if (fz) {
            lda8(rd8(ABSX(0x01C0)));
            pad = !fz;
            if (!pad) {
                lda8(rd8(ABSX(0x01B8)));
                cmp8((u8)A, 0xA0);
                pad = fc;
            }
        }
    }
    if (pad) {
        lda8(rd8(DP(0x0A)));
        if (!fz) {
            wr8(DP(0x00), (u8)A);
            lean_by_pad1();
            return;
        }
    }
    lda8(rd8(ABSX(0x01E8)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x08B4)));
    cmp8((u8)A, rd8(ABSX(0x01E0)));
    if (fz) { ret(); return; }
    ldy8(0x05);
    lean_toward1();
}
