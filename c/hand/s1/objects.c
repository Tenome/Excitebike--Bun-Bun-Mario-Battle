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
