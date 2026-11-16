/*
 * Stadium 1 riders and small objects: movement and sprites, hand-written.
 */
#include "hand/hand.h"
#include "ram.h"
#include "port.h"

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
static void clip_rider(void)
{
    u8 oam = rd8(DP(0x07));
    ldy8(oam);
    u8 state = rd8(ABS(RIDER_PAGE + X));
    lda8(state);
    u8 x = rd8(ABS(RIDER_X + X));
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
        wr8(ABS(OAM_BUF + 0x01 + oam), 0xF0);          /* left column off */
        wr8(ABS(OAM_BUF + 0x09 + oam), 0xF0);
        Y = (u8)(oam >> 2);
        lda8(0x03);
        wr8(ABS(OAM_SIZE_BUF + 0x01 + Y), 0x03);            /* right column: large, X bit 8 */
        wr8(ABS(OAM_SIZE_BUF + 0x03 + Y), 0x03);
        ret();
        return;
    }
    Y = (u8)(oam >> 2);
    lda8(0x03);
    wr8(ABS(OAM_SIZE_BUF + Y), 0x03);                /* left column: large, X bit 8 */
    wr8(ABS(OAM_SIZE_BUF + 0x02 + Y), 0x03);
    ret();
    return;
hide_both:
    lda8(0xF0);
    wr8(ABS(OAM_BUF + 0x01 + oam), 0xF0);
    wr8(ABS(OAM_BUF + 0x09 + oam), 0xF0);
hide_right:
    lda8(0xF0);
    wr8(ABS(OAM_BUF + 0x05 + oam), 0xF0);
    wr8(ABS(OAM_BUF + 0x0D + oam), 0xF0);
    ret();
}
void ClipRiderSprite_m1x1(void)
{
    clip_rider();
    port_ws_clip_rider();            /* port.h: the port's widescreen margins */
}

/*
 * $80:D34A StepObjectX (RTL). Object X ($1500 table, 16-bit position
 * $1500/$1501 with fraction $1520) moves by its velocity $1510: the high
 * nibble is the signed whole part, the low nibble the fraction (in 16ths).
 * Leaves A = carry out of the fraction + whole part.
 */
void StepObjectX_m1x1(void)
{
    u8 v = rd8(ABS(MARKER_VX + X));
    wr8(DP(0x01), (u8)(v << 4));               /* fraction */
    u8 whole = lsr8(lsr8(lsr8(lsr8(v))));
    cmp8(whole, 0x08);
    if (whole >= 0x08)
        whole |= 0xF0;                         /* sign-extend the nibble */
    wr8(DP(0x00), whole);
    ldy8(whole & 0x80 ? 0xFF : 0x00);
    wr8(DP(0x02), (u8)Y);                      /* high byte of the step */
    lda8(rd8(ABS(MARKER_FRAC + X)));
    fc = 0;
    adc8(rd8(DP(0x01)));
    wr8(ABS(MARKER_FRAC + X), (u8)A);
    u8 carry = fc;
    push8(carry);                              /* PHA */
    lda8(rd8(ABS(MARKER_X + X)));
    fc = carry;
    adc8(rd8(DP(0x00)));
    wr8(ABS(MARKER_X + X), (u8)A);
    lda8(rd8(ABS(MARKER_X + 0x01 + X)));
    adc8(rd8(DP(0x02)));
    wr8(ABS(MARKER_X + 0x01 + X), (u8)A);
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
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x0C);
    Y = lsr8(lsr8((u8)A));
    u8 tile = rd8(ABS(0xD3F3 + Y));
    wr8(DP(0x00), tile);
    SET_A8(asl8((u8)X));
    fc = 0;
    adc8(0x04);
    u8 oam = (u8)A;
    Y = oam;
    wr8(ABS(OAM_BUF + 0x02 + oam), rd8(DP(0x00)));     /* re-read, as the original does */
    wr8(ABS(OAM_BUF + oam), rd8(ABS(MARKER_X + X)));
    wr8(ABS(OAM_BUF + 0x01 + oam), rd8(ABS(MARKER_Y + X)));
    lda8(0x2D);
    wr8(ABS(OAM_BUF + 0x03 + oam), 0x2D);
    Y = lsr8(lsr8(oam));
    lda8(0x02);
    wr8(ABS(OAM_SIZE_BUF + Y), 0x02);                /* large size */
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
    lda8(rd8(ABS(OBJ_VX + X)));
    if (fn) {
        Y = dec8((u8)Y);
        eor8(0xFF);
        lda8(inc8((u8)A));
    }
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x02), (u8)Y);
    ldy8(0x01);
    lda8(rd8(ABS(OBJ_VY + X)));
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
    lda8(rd8(ABS(RIDER_X))); wr8(ABS(EFFECT_X + X), (u8)A);
    lda8(rd8(ABS(RIDER_Y))); wr8(ABS(EFFECT_Y + X), (u8)A);
    lda8(0x00); wr8(ABS(EFFECT_FRAME + X), 0x00);
    lda8(0x03); wr8(ABS(EFFECT_KIND + X), 0x03);
    lda8(0xF8); wr8(ABS(EFFECT_VX + X), 0xF8);
    lda8(0xD0); wr8(ABS(EFFECT_VY + X), 0xD0);
    lda8(0x03); wr8(ABS(EFFECT_TIMER + X), 0x03);
    wr8(ABS(EFFECT_STEP + X), 0);
    lda8(0x01); wr8(ABS(SCORE_ADD + 0x01), 0x01);
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
    u8 t = dec8(rd8(ABS(EFFECT_TIMER + X)));
    wr8(ABS(EFFECT_TIMER + X), t);
    if (t == 0)
        wr8(ABS(EFFECT_KIND + X), 0);
    lda8(rd8(ABS(EFFECT_TIMER + X)));
    lda8(lsr8((u8)A));
    if (fc)
        wr8(ABS(EFFECT_Y + X), dec8(rd8(ABS(EFFECT_Y + X))));
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(EFFECT_X + X))); wr8(ABS(OAM_BUF + 0x10 + Y), (u8)A);
    lda8(rd8(ABS(EFFECT_Y + X))); wr8(ABS(OAM_BUF + 0x11 + Y), (u8)A);
    lda8(0x8E); wr8(ABS(OAM_BUF + 0x12 + Y), 0x8E);
    lda8(0x2D); wr8(ABS(OAM_BUF + 0x13 + Y), 0x2D);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABS(OAM_SIZE_BUF + 0x04 + Y), 0x02);
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
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    cmp8((u8)A, rd8(ABSX(RIDER_TARGET_HI)));
    if (!fc) goto press;
    if (fz) {
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        cmp8((u8)A, rd8(ABSX(RIDER_TARGET_LO)));
        if (!fc) goto press;
    }
    CpuReleaseButtons_m1x1();
    return;
press:
    lda8((u8)Y);
    ora8(rd8(ABSX(RIDER_INPUT)));
    wr8(ABSX(RIDER_INPUT), (u8)A);
    ret();
}

/* $80:AFF2 CpuReleaseDpad (RTS): input $0178 & $F0. */
void CpuReleaseDpad_m1x1(void)
{
    lda8(0xF0);
    and8(rd8(ABSX(RIDER_INPUT)));
    wr8(ABSX(RIDER_INPUT), (u8)A);
    ret();
}

/* $80:AFFB CpuSteer (RTS): release the d-pad, then press down (1) or up
 * (2) toward the target lane angle $0971 against $01E0 (none when equal
 * or no target). */
void CpuSteer_m1x1(void)
{
    push16(0xAFFD); CpuReleaseDpad_m1x1();
    ldy8(0x01);
    lda8(rd8(ABSX(RIDER_WANT_LEAN)));
    if (fz) { ret(); return; }
    cmp8((u8)A, rd8(ABSX(RIDER_LEAN)));
    if (fz) { ret(); return; }
    if (fc) Y = inc8((u8)Y);
    lda8((u8)Y);
    ora8(rd8(ABSX(RIDER_INPUT)));
    wr8(ABSX(RIDER_INPUT), (u8)A);
    ret();
}

/* $81:9F7A AimVelocity (RTL, A = steps): object X's velocity toward its
 * target from AimRatio ($02A9 vertical, $0299 horizontal). */
void AimVelocity_m1x1(void)
{
    push8(0x81); push16(0x9F7D); AimRatio_m1x1();
    lda8(rd8(DP(0x00))); wr8(ABSX(OBJ_VY), (u8)A);
    lda8(rd8(DP(0x0A))); wr8(ABSX(OBJ_VX), (u8)A);
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
    wr8(ABS(RIDER_SPEED_LO), 0); wr8(ABS(RIDER_SPEED_HI), 0); wr8(ABS(PLAYER_GROUND), 0);
    wr8(ABS(PLAYER_LIFT), 0); wr8(ABS(RIDER_STEP), 0); wr8(ABS(RIDER_SLOW), 0);
    ret();
}

void PlayerHalt_m1x1(void) { player_halt1(); }

/* $80:DDA7 PlayerStop (RTS): crashed flags $01C8 = $0999 = $FF, then
 * PlayerHalt. */
void PlayerStop_m1x1(void)
{
    lda8(0xFF);
    wr8(ABS(RIDER_BUMPED), 0xFF);
    wr8(ABS(PLAYER_TUMBLE), 0xFF);
    player_halt1();
}

/* $80:DD82 PlayerColumnCheck (RTS): as Stadium 4's $80:DC70, with
 * Stadium 1's addresses (wheel column $0260, air $01E8, fall speed
 * $01B0; CrashLand is $80:C37B). */
void PlayerColumnCheck_m1x1(void)
{
    Y = rd8(ABS(RIDER_COLUMN)); nz8((u8)Y);
    lda8(rd8(ABS(RIDER_AIR)));
    if (!fz) {
        lda8(rd8(ABS(0x0904)));
        if (!fz) { ret(); return; }
        lda8(rd8(ABSY(TRACK_ROWS + 0x40)));
        Y = inc8((u8)Y);
        cmp8((u8)A, rd8(ABSY(TRACK_ROWS + 0x40)));
        if (fz) { ret(); return; }
        push16(0xDD9A); GroundHeightAbove_m1x1();
        fc = 0; adc8(0x08);
        fc = 1; sbc8(rd8(ABSY(TRACK_ROWS + 0x40)));
        cmp8((u8)A, rd8(ABS(RIDER_Y)));
        if (fc) { ret(); return; }
        PlayerStop_m1x1();
        return;
    }
    lda8(rd8(ABS(PLAYER_LIFT)));
    if (!fz) {
        cmp8((u8)A, rd8(ABSY(TRACK_ROWS + 0x40)));
        int stop;
        if (fz) {
            Y = inc8((u8)Y);
            cmp8((u8)A, rd8(ABSY(TRACK_ROWS + 0x40)));
            stop = !fc;
            if (!stop) goto ramp;
        } else stop = !fc;
        if (stop) { push16(0xDDD2); PlayerStop_m1x1(); }
        push16(0xDDD5); CrashLand_m1x1();
        lda8(rd8(ABS(RIDER_BOUNCE_LO))); wr8(ABS(RIDER_HEIGHT_LO), (u8)A);
        lda8(rd8(ABS(RIDER_BOUNCE_HI))); wr8(ABS(RIDER_HEIGHT_HI), (u8)A);
        ret();
        return;
    }
ramp:
    ldx8(0x01);
    lda8(rd8(ABSY(TRACK_ROWS)));
    cmp8((u8)A, 0xFC);
    if (!fz) ldx8(0x00);
    wr8(ABS(RIDER_BAND), (u8)X);
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
    wr8(ABSX(MARKER_VY), inc8(rd8(ABSX(MARKER_VY))));
    push8(0x80); push16(0xD32F); DrawSmallObject_m1x1();
    lda8(rd8(ABSX(MARKER_Y)));
    cmp8((u8)A, 0xC0);
    if (fc) {
        wr8(ABSX(MARKER_ON), 0);
        lda8(rd8(ABS(MARKER_COUNT)));
        cmp8((u8)A, 0x05);
        if (!fc) wr8(ABS(MARKER_COUNT), inc8(rd8(ABS(MARKER_COUNT))));
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
    lda8(rd8(ABSX(RIDER_COLUMN)));
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
    lda8(rd8(ABS(END_STEP)));
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
    lda8(rd8(DP(TUMBLE_TIMER)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(MODE_STEP)));
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
    lda16(rd16(ABSX(0xC70A))); wr16(ABS(DIGIT_ROW + 0x05), A);
    lda16(rd16(ABS(minutes + 1)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC70A))); wr16(ABS(DIGIT_ROW + 0x02), A);
    sep(0x20);
    lda8(rd8(ABS(minutes))); wr8(ABS(DIGIT_ROW), (u8)A);
    lda8(0x27); wr8(ABS(DIGIT_BUF), 0x27);
    rep(0x20);
    lda16(rd16(ABS(DIGIT_ROW))); ora16(0x9750); wr16(ABS(DIGIT_ROW), A);
    fc = 0; adc16(0x6510); wr16(ABS(DIGIT_ROW + 0x07), A);
    lda16(rd16(ABS(DIGIT_ROW + 0x02))); ora16(0x5050); wr16(ABS(DIGIT_ROW + 0x02), A);
    fc = 0; adc16(0x1010); wr16(ABS(DIGIT_ROW + 0x09), A);
    lda16(rd16(ABS(DIGIT_ROW + 0x04))); ora16(0x5098); wr16(ABS(DIGIT_ROW + 0x04), A);
    fc = 0; adc16(0x1064); wr16(ABS(DIGIT_ROW + 0x0B), A);
    sep(0x20);
    lda8(rd8(ABS(DIGIT_ROW + 0x06))); ora8(0x50); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x0D), (u8)A);
    VramQueueTiles20_m1x1();
}

void ShowRaceClock_m1x1(void) { show_clock(0x03B1); }

/* $80:C7EB TickRaceClock (RTS): at 60 frames ($03B3) carry into the
 * seconds $03B2 and at 60 into the minutes $03B1; then ShowRaceClock. */
void TickRaceClock_m1x1(void)
{
    lda8(rd8(ABS(RACE_FRAMES)));
    cmp8((u8)A, 0x3C);
    if (fc) {
        wr8(ABS(RACE_SEC), inc8(rd8(ABS(RACE_SEC))));
        lda8(rd8(ABS(RACE_SEC)));
        cmp8((u8)A, 0x3C);
        if (fc) {
            wr8(ABS(RACE_MIN), inc8(rd8(ABS(RACE_MIN))));
            wr8(ABS(RACE_SEC), 0);
        }
        wr8(ABS(RACE_FRAMES), 0);
    }
    ShowRaceClock_m1x1();
}

/* $80:AF9F RiderAhead (RTS, X = rider, Y = window, $02 = row, $0F = self):
 * C set when another rider on screen page 1 in row $02 sits between
 * rider X's position + $80:B86F,Y and that less $80:B870,Y (riders
 * from $7E up). */
void RiderAhead_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_X)));
    fc = 0; adc8(rd8(ABSY(0xB86F)));
    wr8(DP(0x00), (u8)A);
    fc = 1; sbc8(rd8(ABSY(0xB870)));
    wr8(DP(0x01), (u8)A);
    Y = rd8(DP(FIRST_RIVAL)); nz8((u8)Y);
    do {
        cmp8((u8)Y, rd8(DP(0x0F)));
        if (!fz) {
            lda8(rd8(ABSY(RIDER_PAGE)));
            lda8(lsr8((u8)A));
            if (fc) {
                lda8(rd8(ABSY(RIDER_BAND)));
                if (!fz) {
                    cmp8((u8)A, rd8(DP(0x02)));
                    if (fz) {
                        lda8(rd8(ABSY(RIDER_X)));
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
    lda8(rd8(ABS(FINISH_CROSSED)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    wr8(ABS(FINISH_CROSSED), inc8(rd8(ABS(FINISH_CROSSED))));
    lda8(0x1D); wr8(DP(FLASH_TIMER), 0x1D); wr8(DP(0xA4), 0x1D);
    lda8(rd8(ABS(LAPS_LEFT)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) { lda8(0x4A); push8(0x80); push16(0xED66); QueueSound_m1x1(); }
        else { lda8(0x28); push8(0x80); push16(0xED6E); QueueSound_m1x1(); }
        lda8(rd8(DP(FINAL_RACE)));
        if (fz) wr8(ABS(BONUS_TRIES), inc8(rd8(ABS(BONUS_TRIES))));
        lda8(rd8(DP(OVERHEATED)));
        if (fz) { ret(); return; }
    } else {
        lda8(0x01); wr8(DP(RACE_END), 0x01);
        lda8(0x2B); push8(0x80); push16(0xED84); QueueSound_m1x1();
        lda8(0x2D); push8(0x80); push16(0xED8A); QueueSound_m1x1();
        lda8(0x80); push8(0x80); push16(0xED90); QueueSound_m1x1();
    }
    lda8(0x00); wr8(DP(FLASH_TIMER), 0x00);
    ret();
}

/* $80:DA31 (RTS): the bonus banner sprites: for banner $BB (1-based; at
 * 4 only while $8A counts down) four OAM words from $80:DA65 + 8 *
 * ($BB - 1) into $16A0-$16A7, size 2 for both. X is kept. */
static void banner_sprites(u16 table)
{
    lda8(rd8(DP(COUNTDOWN)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x04);
    if (fz) {
        lda8(rd8(DP(TIMER_A)));
        if (fz) { ret(); return; }
        wr8(DP(TIMER_A), dec8(rd8(DP(TIMER_A))));
    }
    push8((u8)X);
    lda8(rd8(DP(COUNTDOWN)));
    lda8(dec8((u8)A));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    ldy8(0xA0);
    rep(0x20);
    do {
        lda16(rd16(ABSX(table)));
        wr16(ABSY(OAM_BUF), A);
        X = inc8((u8)X); X = inc8((u8)X);
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        cmp8((u8)Y, 0xA8);
    } while (!fc);
    ldy8(0x28);
    lda16(0x0202); wr16(ABSY(OAM_SIZE_BUF), 0x0202);
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
        lda8(rd8(ABSX(SCORE_DIGITS)));
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
    lda8(rd8(DP(P1_PRESS_LO)));
    ora8(rd8(DP(P1_PRESS_HI)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(DP(P1_PRESS_LO)));
    if (!fz) {
        wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
        wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
        lda8(0x45);
        push8(0x80); push16(0x9D1E); QueueSound_m1x1();
        ret();
        return;
    }
    end_box(0x9D3E);
    wr8(ABS(END_STEP), dec8(rd8(ABS(END_STEP))));
    ret();
}

/* $80:9BC8 (RTS): the end screen's scroll-in: BG X $9A moves 2 a frame;
 * at $0140 sound $2D, the next step, the box, $03A9 = 1 and $0452
 * cleared. */
void EndScreenScrollIn_m1x1(void)
{
    rep(0x20);
    wr16(ABS(END_SCROLL), inc16(rd16(ABS(END_SCROLL))));
    wr16(ABS(END_SCROLL), inc16(rd16(ABS(END_SCROLL))));
    lda16(rd16(ABS(END_SCROLL)));
    cmp16(A, 0x0140);
    if (fc) {
        sep(0x20);
        lda8(0x2D);
        push8(0x80); push16(0x9BDF); QueueSound_m1x1();
        wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
        end_box(0x9C01);
        lda8(0x01); wr8(ABS(ENTRY_DIGITS + 0x03), 0x01);
        wr8(ABS(ENTRY_CURSOR), 0);
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
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    rep(0x20);
    wr16(ABSX(OBJ_STEP), dec16(rd16(ABSX(OBJ_STEP))));
    if (fz) {
        wr16(ABSX(OBJ_STATE), inc16(rd16(ABSX(OBJ_STATE))));
        rep(0x20);
        lda16(0x6D69);
        for (u16 a = 0x1C02; a <= 0x1C16; a += 2) wr16(ABS(a), 0x6D69);
        sep(0x20);
        wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
        wr8(DP(0x9D), inc8(rd8(DP(0x9D))));
        lda8(0xC0); wr8(DP(MODE_TIMER), 0xC0);
        lda8(0x06); wr8(DP(FADE_FROM), 0x06);
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
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(OBJ_STEP)));
    lda8(inc8((u8)A));
    and8(0x03);
    wr8(ABSX(OBJ_STEP), (u8)A);
    Y = (u8)A; nz8((u8)Y);
    if (fz) {
        wr8(ABSX(OBJ_RANDOM), dec8(rd8(ABSX(OBJ_RANDOM))));
        if (fz) {
            lda8(0x02); wr8(ABSX(OBJ_STATE), 0x02);
            lda8(0x0A); wr8(ABSX(OBJ_DELAY), 0x0A);
            lda8(0x06); wr8(ABSX(OBJ_FRAME), 0x06);
            push8(0x81); push16(0xBC0F); Random_m1x1();
            and8(0x3F);
            fc = 0; adc8(0x40);
            wr8(ABSX(OBJ_COUNTER), (u8)A);
            ret();
            return;
        }
    }
    lda8(rd8(ABSY(0xBC26))); wr8(ABSX(OBJ_DELAY), (u8)A);
    lda8(rd8(ABSY(0xBC2A))); wr8(ABSX(OBJ_FRAME), (u8)A);
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
            lda8(rd8(LONGX(VQ0_DATA)));
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
        wr8(ABSY(DIGIT_ROW), (u8)A);
        ora8(0x10);
        wr8(ABSY(DIGIT_ROW + 0x06), (u8)A);
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x05);
    } while (!fc);
    wr8(ABS(DIGIT_ROW + 0x05), 0);
    lda8(0x10); wr8(ABS(DIGIT_ROW + 0x0B), 0x10);
    lda8(0x26); wr8(ABS(DIGIT_BUF), 0x26);
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
    X = rd8(ABS(ROW_TYPE)); nz8((u8)X);
    lda8(rd8(ABSX(0xD4E2))); wr8(DP(0x09), (u8)A);
    lda8(rd8(ABSX(0xD4EB))); wr8(DP(0x0A), (u8)A);
    ldy8(0x00); wr8(DP(0x0E), 0);
    do {
        Y = rd8(DP(0x0E)); nz8((u8)Y);
        push16(0xDCA2); LanePieceTables_m1x1();
        Y = rd8(ABS(BUILD_COLUMN)); nz8((u8)Y);
        lda8(rd8((((u32)DB << 16) + rd16(DP(0x09)) + Y) & 0xFFFFFF));
        wr8(ABS(BUILD_COLUMN), inc8(rd8(ABS(BUILD_COLUMN))));
        Y = rd8(DP(0x0F)); nz8((u8)Y);
        wr8((((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF, (u8)A);
        wr8(DP(0x10), (u8)A);
        wr8(DP(0x0E), inc8(rd8(DP(0x0E))));
        lda8(rd8(DP(0x0E)));
        cmp8((u8)A, 0x03);
        if (fz) {
            lda8(rd8(DP(0x10)));
            if (fz) lda8(0x3B);
            wr8(ABSY(TRACK_ROWS + 0x180), (u8)A); wr8(ABSY(TRACK_ROWS + 0x1C0), (u8)A);
            wr8(ABSY(TRACK_ROWS + 0x200), (u8)A); wr8(ABSY(TRACK_ROWS + 0x240), (u8)A);
            wr8(ABSY(TRACK_ROWS + 0x280), (u8)A); wr8(ABSY(TRACK_ROWS + 0x2C0), (u8)A);
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
        lda8(rd8(ABSX(MARKER_ON)));
        if (!fz) {
            rep(0x20);
            wr16(ABSX(MARKER_AGE), inc16(rd16(ABSX(MARKER_AGE))));
            sep(0x20);
            push8(0x80); push16(0xD2B0); MarkerObjectStep_m1x1();
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    push8(0x80); push16(0xD2B8); DrawCoinIcons_m1x1();
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x07);
    if (!fz) { retl(); return; }
    push8((u8)X);
    ldy8(0x00);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x38);
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    do {
        lda16(rd16(LONGX(0x80D2DF)));
        wr16(ABSY(PALETTE_BUF + 0x08), A);
        X = inc8((u8)X); X = inc8((u8)X);
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        cmp8((u8)Y, 0x08);
    } while (!fc);
    sep(0x20);
    X = pull8(); nz8((u8)X);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
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
    lda8(rd8(ABSX(EFFECT_X))); wr8(ABSY(OAM_BUF + 0x10), (u8)A);
    lda8(rd8(ABSX(EFFECT_Y))); wr8(ABSY(OAM_BUF + 0x11), (u8)A);
    lda8(rd8(ABSX(EFFECT_FRAME)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x8195EB))); wr8(ABSY(OAM_BUF + 0x12), (u8)A);
    lda8(0x2D); wr8(ABSY(OAM_BUF + 0x13), 0x2D);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(OAM_SIZE_BUF + 0x04), 0x02);
    wr8(ABSX(EFFECT_TIMER), dec8(rd8(ABSX(EFFECT_TIMER))));
    if (!fz) { ret(); return; }
    lda8(0x04); wr8(ABSX(EFFECT_TIMER), 0x04);
    wr8(ABSX(EFFECT_FRAME), inc8(rd8(ABSX(EFFECT_FRAME))));
    lda8(rd8(ABSX(EFFECT_FRAME)));
    cmp8((u8)A, 0x03);
    if (!fc) { ret(); return; }
    wr8(ABSX(EFFECT_STEP), inc8(rd8(ABSX(EFFECT_STEP))));
    lda8(0x40); wr8(ABSX(EFFECT_TIMER), 0x40);
    ret();
}

/* $80:9A52 EndScreenScore (RTS): when $88 runs out, the five digits at
 * $00:0CE5 at VRAM $0975, $88 = $20 and the next end-screen steps
 * ($0453 = 4 and $0454 = 5, or 3 when $085D is set). */
void EndScreenScore_m1x1(void)
{
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    rep(0x20);
    lda16(0x0CE5); wr16(DP(0x18), 0x0CE5);
    lda16(0x0975); wr16(DP(0x00), 0x0975);
    sep(0x20);
    lda8(0x00); wr8(DP(0x1A), 0x00);
    push8(0x80); push16(0x9A6B); QueueFiveDigits_m1x1();
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(rd8(ABS(NEW_SCORE_RECORD)));
    if (fz) {
        lda8(0x04); wr8(ABS(END_STEP), 0x04);
        lda8(0x05); wr8(ABS(END_NEXT_STEP), 0x05);
    } else {
        lda8(0x03); wr8(ABS(END_STEP), 0x03);
    }
    ret();
}

/* $80:9AF9 EndScreenCode (RTS): after the $88 delay the code box (2 rows
 * of $20 from $89:D880 at $0300); then A ($26 bit 7): $88 = $20, sound
 * $43, step $0453 = $0A and the five digits at $11:5730 to VRAM $0A15. */
void EndScreenCode_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        u8 t = dec8(rd8(DP(MODE_TIMER)));
        wr8(DP(MODE_TIMER), t);
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
    lda8(rd8(DP(P1_PRESS_LO)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(0x43);
    push8(0x80); push16(0x9B2F); QueueSound_m1x1();
    lda8(0x0A); wr8(ABS(END_STEP), 0x0A);
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
    lda8(0x02); wr8(ABSX(RIDER_AIR), 0x02);
    lda8(0x0F); wr8(ABSX(RIDER_HEIGHT_LO), 0x0F);
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    fc = 0; adc8(0xAF);
    wr8(ABSX(RIDER_BOUNCE_LO), (u8)A);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    adc8(0x00);
    wr8(ABSX(RIDER_BOUNCE_HI), (u8)A);
    lda8(rd8(ABSX(RIDER_SLOW)));
    cmp8((u8)A, 0x02);
    if (fz) {
        u8 hi = rd8(ABSX(RIDER_BOUNCE_HI));
        fc = hi & 1; hi >>= 1; wr8(ABSX(RIDER_BOUNCE_HI), hi); nz8(hi);
        u8 lo = rd8(ABSX(RIDER_BOUNCE_LO));
        u8 c = fc; fc = lo & 1; lo = (u8)((lo >> 1) | (c << 7)); wr8(ABSX(RIDER_BOUNCE_LO), lo); nz8(lo);
    }
    lda8((u8)X);
    if (!fz) { ret(); return; }
    wr8(ABS(PLAYER_LIFT), (u8)A);
    lda8(0x20);
    push8(0x80); push16(0xC3E6); QueueSound_m1x1();
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    eor8(0x30);
    if (fz) lda8(0x08);
    ret();
}

/* $80:AC9C RiderOnRamp (RTS): Stadium 4's $80:ACD6 with Stadium 1's
 * addresses (piece $0210, table $80:ACF9, air $0208, fall $01B0). */
void RiderOnRamp_m1x1(void)
{
    Y = rd8(ABSX(RIDER_TILE_Y)); nz8((u8)Y);
    lda8(rd8(ABSY(0xACF9)));
    if (fz) {
        lda8(0x03);
        cmp8((u8)A, rd8(ABSX(RIDER_JUMP)));
        int ramp = 0;
        if (fz) {
            lda8(rd8(ABSX(RIDER_BAND)));
            cmp8((u8)A, 0x03);
            ramp = fc;
        }
        if (!ramp) {
            lda8(rd8(ABSX(RIDER_BAND)));
            if (!fz) {
                lda8(0x01);
                cmp8((u8)A, rd8(ABSX(RIDER_ON_RAMP)));
                if (fz) { ret(); return; }
                lda8(lsr8((u8)A));
                wr8(ABSX(RIDER_ON_RAMP), (u8)A);
                ret();
                return;
            }
        }
    }
    lda8(0x01);
    cmp8((u8)A, rd8(ABSX(RIDER_ON_RAMP)));
    if (!fz) {
        push8((u8)A);
        lda8(rd8(DP(GAME_MODE)));
        cmp8((u8)A, 0x09);
        if (fz) {
            lda8(rd8(ABSY(0xACF9)));
            if (fn) {
                lda8(rd8(ABS(RIDER_ON_RAMP)));
                if (fz) {
                    lda8(rd8(ABS(RIDER_Y)));
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
        wr8(ABSX(RIDER_ON_RAMP), (u8)A);
    }
    lda8((u8)X);
    if (fz) {
        lda8(rd8(ABS(RIDER_SPEED_LO)));
        ora8(rd8(ABS(RIDER_SPEED_HI)));
    }
    ret();
}

void GroundHeightAbove_m1x1(void);

/* $80:CB47 BallHit (RTS): Stadium 4's $80:C997 with Stadium 1's
 * addresses: the player running into the ball stops (A = 1). */
void BallHit_m1x1(void)
{
    lda8(rd8(ABS(RIDER_TIMER)));
    ora8(rd8(ABS(OVERHEAT_LOCK)));
    if (!fz) { ret(); return; }
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(RIDER_PAGE + 0x03)));
    lda8(lsr8((u8)A));
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(BALL_OUT)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_LANE)));
    fc = 1; sbc8(0x03);
    cmp8((u8)A, rd8(ABS(BALL_LANE)));
    if (fc) { ret(); return; }
    adc8(0x07);
    cmp8((u8)A, rd8(ABS(BALL_LANE)));
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(RIDER_X)));
    cmp8((u8)A, rd8(ABS(RIDER_RUNOUT + 0x03)));
    if (fc) { ret(); return; }
    adc8(0x12);
    cmp8((u8)A, rd8(ABS(RIDER_RUNOUT + 0x03)));
    if (!fc) { ret(); return; }
    push16(0xCB7E); GroundHeightAbove_m1x1();
    fc = 1; sbc8(0x08);
    cmp8((u8)A, rd8(ABS(RIDER_Y)));
    if (fc) { ret(); return; }
    ldy8(0xFF); wr8(ABS(RIDER_BUMPED), 0xFF);
    Y = inc8((u8)Y); wr8(ABS(RIDER_STEER + 0x03), (u8)Y);
    Y = inc8((u8)Y); wr8(ABS(RIDER_TIMER + 0x03), (u8)Y);
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
    lda8(rd8(ABSX(SELECT_WHEELS)));
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
    lda8(rd8(ABS(SELECT_STEP)));
    if (!fz) {
        lda8(rd8(ABSX(SELECT_WHEELS)));
        and8(0x3F);
        notch = fz;
    } else {
        lda8(rd8(ABS(SELECT_WHEELS)));
        notch = fz;
        if (!notch) { cmp8((u8)A, 0x56); notch = fz; }
        if (!notch) { cmp8((u8)A, 0xAA); notch = fz; }
    }
    if (!notch) { retl(); return; }
    lda8(rd8(ABS(SELECT_TURN)));
    if (fz) { retl(); return; }
    lda8(rd8(ABS(SELECT_TURN)));
    if (fz) { retl(); return; }
    lda8(rd8(DP(P1_HELD_HI)));
    and8(0x03);
    if (!fz) { retl(); return; }
    lda8(0x41);
    push8(0x81); push16(0xA178); QueueSound_m1x1();
    wr8(ABS(SELECT_TURN), 0);
    retl();
}

/* $80:C04B (RTS): during a race with rivals ($7D, $8E, not $84) each
 * rider's distance $085E: riders on the track from their position
 * ($0198:$0190 + $94 - $0100), the others advanced by their speed
 * ($0158 fraction + $0D01). */
void UpdateRiderDistances_m1x1(void)
{
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    rep(0x20);
    ldy8(0x0E);
    ldx8(0x07);
    do {
        lda16(rd16(ABSX(RIDER_ACTIVE)));
        and16(0x00FF);
        if (!fz) {
            lda16(rd16(ABSX(RIDER_X))); wr16(DP(0x00), A);
            lda16(rd16(ABSX(RIDER_PAGE))); wr16(DP(0x01), A);
            lda16(rd16(DP(0x00)));
            fc = 0; adc16(rd16(DP(SCROLL_X)));
            fc = 1; sbc16(0x0100);
            wr16(ABSY(RIDER_DIST), A);
        } else {
            lda16(rd16(ABSY(0x0158)));
            fc = 0; adc16(rd16(ABSY(RIDER_PACE)));
            push16(A);
            and16(0x00FF);
            wr16(ABSY(0x0158), A);
            lda16(pull16());
            A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
            and16(0x00FF);
            fc = 0; adc16(rd16(ABSY(RIDER_DIST)));
            wr16(ABSY(RIDER_DIST), A);
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
    lda8(rd8(ABSX(RIDER_INPUT)));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBD42)));
    wr8(ABSX(RIDER_FALL_SPEED), (u8)A);
    lda8(rd8(ABSX(RIDER_Y)));
    wr8(DP(0x01), (u8)A);
    push16(0xC3A4); RiderHeightStep_m1x1();
    wr8(ABSX(RIDER_Y), (u8)A);
    lda8(rd8(ABSX(RIDER_PIECE_ACTION)));
    if (fz) { lda8(0x01); wr8(ABSX(RIDER_LANDED), 0x01); }
}

/* $80:C37E: A is the crash speed ($08EC); the crash starts (StartJump),
 * then the steps. */
static void crash_from1(void)
{
    wr8(ABSX(RIDER_HEIGHT_HI), (u8)A);
    push16(0xC383); StartJump_m1x1();
    crash_tail1();
}

/* $80:C37B CrashLand (RTS): crash at rider X's speed $01C0. */
void CrashLand_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    crash_from1();
    ret();
}

/* $80:C34C RiderLandCheck (RTS): rider X in air state 2 continues the
 * crash steps unless the player presses right ($0178 bit 0, $7F
 * clear); any other nonzero state crashes it at speed 0. */
void RiderLandCheck_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x02);
    if (!fz) { lda8(0x00); crash_from1(); ret(); return; }
    lda8(rd8(ABSX(RIDER_INPUT)));
    lda8(lsr8((u8)A));
    if (fc) {
        lda8(rd8(DP(RIDER_TURN)));
        if (fz) { ret(); return; }
    }
    crash_tail1();
    ret();
}

/* $80:EB0C (RTS, X = rider): on the ground the speed $01C0:$01B8 grows
 * by $0180, then a crash at speed 0. */
void SpeedBumpCrash_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_SPEED_LO)));
    fc = 0; adc8(0x80);
    wr8(ABS(RIDER_SPEED_LO), (u8)A);
    lda8(rd8(ABS(RIDER_SPEED_HI)));
    adc8(0x01);
    wr8(ABS(RIDER_SPEED_HI), (u8)A);
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
    lda8(rd8(DP(FRAME_COUNT)));
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
    lda8(rd8(ABS(SELECT_TURN)));
    int turning = !fz;
    if (!turning) {
        lda8(rd8(DP(P1_HELD_HI)));
        and8(0x03);
        if (!fz) {
            X = (u8)A; nz8((u8)X);
            lda8(rd8(ABSX(0x8F29)));
            wr8(ABS(SELECT_TURN), (u8)A);
            turning = 1;
        }
    }
    if (turning) {
        lda8(rd8(DP(FRAME_COUNT)));
        and8(0x03);
        if (fz) { lda8(0x40); push8(0x80); push16(0x8F14); QueueSound_m1x1(); }
    }
    ldx8(0x03);
    do {
        lda8(rd8(ABSX(SELECT_WHEELS)));
        fc = 0; adc8(rd8(ABS(SELECT_TURN)));
        wr8(ABSX(SELECT_WHEELS), (u8)A);
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
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    ldy8(0x00);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x02);
    if (!fc) {
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        cmp8((u8)A, rd8(ABSX(RIDER_SLOW_AT)));
        if (!fc) Y = inc8((u8)Y);
    }
    lda8((u8)Y);
    wr8(ABSX(RIDER_SLOW), (u8)A);
    lda8(rd8(ABSX(RIDER_LANDING)));
    cmp8((u8)A, 0x02);
    if (fz) {
        lda8(0x00);
        wr8(ABS(PLAYER_GROUND), 0); wr8(ABS(PLAYER_LIFT), 0);
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
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (fz) {
        rep(0x20);
        lda16(rd16(ABSX(OBJ_STEP)));
        fc = 0; adc16(rd16(ABSX(OBJ_X)));
        wr16(ABSX(OBJ_X), A);
        wr16(ABSX(OBJ_RANDOM), dec16(rd16(ABSX(OBJ_RANDOM))));
        if (fz) {
            lda16(0x0008); wr16(ABSX(OBJ_RANDOM), 0x0008);
            lda16(rd16(ABSX(OBJ_STEP)));
            eor16(0xFFFF);
            lda16(inc16(A));
            wr16(ABSX(OBJ_STEP), A);
        }
    }
    sep(0x20);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (!fz) { ret(); return; }
    wr8(ABSX(OBJ_Y), dec8(rd8(ABSX(OBJ_Y))));
    lda8(rd8(ABSX(OBJ_Y)));
    cmp8((u8)A, 0x48);
    if (fc) { ret(); return; }
    rep(0x20);
    lda16(0x00F0);
    wr16(ABSX(OBJ_X), 0x00F0);
    wr16(ABSX(OBJ_Y), 0x00F0);
    sep(0x20);
    wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
    lda8(0x09);
    push8(0x81); push16(0xB713); FindObjectSlot_m1x1();
    lda8(0x00); wr8(ABSY(OBJ_ACTIVE), 0x00);
    lda8(0x00);
    push8(0x81); push16(0xB71E); FindObjectSlot_m1x1();
    lda8(0x00); wr8(ABSY(OBJ_ACTIVE), 0x00);
    push8(0x81); push16(0xB727); Random_m1x1();
    and8(0x7F);
    fc = 0; adc8(0x80);
    wr8(ABSX(OBJ_DELAY), (u8)A);
    ret();
}

/* $80:8EA0 (RTS): the select screen's timer: $88:$89 / 60 (the hardware
 * divider) as two digits ($03D3-$03D6, tiles +$50 / +$60), size $22,
 * at VRAM $1A8F. */
void SelectTimerDigits_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER))); wr8(ABS(WRDIVL), (u8)A);
    lda8(rd8(DP(0x89))); wr8(ABS(WRDIVH), (u8)A);
    lda8(0x3C); wr8(ABS(WRDIVB), 0x3C);
    wr8(DP(0x00), 0);
    lda8(rd8(ABS(RDDIVL)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        wr8(DP(0x00), inc8(rd8(DP(0x00))));
        WAIT_TICK();
    }
    fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
    lda8(rd8(DP(0x00)));
    fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    lda8(0x22); wr8(ABS(DIGIT_BUF), 0x22);
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
    lda8(rd8(ABS(BONUS_BEST)));
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
        fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW), (u8)A);
        fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    } else {
        lda8(0xFC); wr8(ABS(DIGIT_ROW), 0xFC); wr8(ABS(DIGIT_ROW + 0x02), 0xFC);
    }
    lda8(rd8(DP(0x01)));
    fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
    lda8(0x22); wr8(ABS(DIGIT_BUF), 0x22);
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
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    wr8(ABS(PACE_FRAMES), inc8(rd8(ABS(PACE_FRAMES))));
    lda8(rd8(ABS(PACE_FRAMES)));
    cmp8((u8)A, 0x46);
    if (!fc) {
        lda8(rd8(ABS(RIDER_STEP)));
        fc = 0; adc8(rd8(ABS(PACE_DIST)));
        wr8(ABS(PACE_DIST), (u8)A);
        cmp8((u8)A, 0x3C);
        if (!fc) { ret(); return; }
        lda8(rd8(ABS(PACE_DIST)));
        fc = 1; sbc8(0x3C);
        wr8(ABS(PACE_DIST), (u8)A);
        ldx8(0x07);
        do {
            lda8(rd8(ABS(PACE_FRAMES)));
            cmp8((u8)A, rd8(ABSX(0xE733)));
            if (!fc) wr8(DP(0x00), (u8)X);
            else wr8(DP(0x01), (u8)X);
            X = dec8((u8)X);
        } while (!fz);
        lda8(rd8(DP(FINAL_RACE)));
        if (!fz) {
            X = rd8(DP(0x00)); nz8((u8)X);
            lda8(rd8(ABSX(0xE73B))); wr8(ABS(SCORE_ADD + 0x04), (u8)A);
            lda8(rd8(ABSX(0xE743))); wr8(ABS(SCORE_ADD + 0x05), (u8)A);
        }
    }
    lda8(0x00);
    wr8(ABS(PACE_FRAMES), 0);
    wr8(ABS(PACE_DIST), 0);
    ret();
}

void Nop_BD39_m1x1(void);
void MoveObjectX_m1x1(void);
void MoveObjectX8_m1x1(void);

/* One 8x8 hardware multiply, with the wait call ($80:BD39). */
static void mul8_1(u8 a, u8 b, u16 back)
{
    lda8(a); wr8(ABS(WRMPYA), (u8)A);
    lda8(b); wr8(ABS(WRMPYB), (u8)A);
    push16(back); Nop_BD39_m1x1();
}

/* [p] * [q] into r..r+3 (direct page) from four 8x8 products. */
static void mul16_1(u8 p, u8 q, u8 r, const u16 back[4])
{
    mul8_1(rd8(DP(p)), rd8(DP(q)), back[0]);
    lda8(rd8(ABS(RDMPYL))); wr8(DP(r), (u8)A);
    lda8(rd8(ABS(RDMPYH))); wr8(DP(r + 1), (u8)A);
    mul8_1(rd8(DP(p)), rd8(DP(q + 1)), back[1]);
    lda8(rd8(ABS(RDMPYL)));
    fc = 0; adc8(rd8(DP(r + 1))); wr8(DP(r + 1), (u8)A);
    lda8(rd8(ABS(RDMPYH)));
    adc8(0x00); wr8(DP(r + 2), (u8)A);
    mul8_1(rd8(DP(p + 1)), rd8(DP(q)), back[2]);
    lda8(rd8(ABS(RDMPYL)));
    fc = 0; adc8(rd8(DP(r + 1))); wr8(DP(r + 1), (u8)A);
    lda8(rd8(ABS(RDMPYH)));
    adc8(rd8(DP(r + 2))); wr8(DP(r + 2), (u8)A);
    mul8_1(rd8(DP(p + 1)), rd8(DP(q + 1)), back[3]);
    lda8(rd8(ABS(RDMPYL)));
    fc = 0; adc8(rd8(DP(r + 2))); wr8(DP(r + 2), (u8)A);
    lda8(rd8(ABS(RDMPYH)));
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
    wr8(ABSX(EFFECT_TIMER), dec8(rd8(ABSX(EFFECT_TIMER))));
    if (fz) {
        lda8(rd8(ABSX(EFFECT_FRAME)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(EFFECT_FRAME), (u8)A);
        lda8(0x04); wr8(ABSX(EFFECT_TIMER), 0x04);
    }
    push8(0x81); push16(0x9547); MoveObjectX_m1x1();
    push8((u8)X);
    push8(0x81); push16(0x954C); MoveObjectX8_m1x1();
    X = pull8(); nz8((u8)X);
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(EFFECT_X))); wr8(ABSY(OAM_BUF + 0x10), (u8)A);
    lda8(rd8(ABSX(EFFECT_Y))); wr8(ABSY(OAM_BUF + 0x11), (u8)A);
    lda8(rd8(ABSX(EFFECT_VY)));
    cmp8((u8)A, 0xF0);
    if (fc) lda8(0x00);
    else lda8(rd8(ABSX(EFFECT_FRAME)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x81959B))); wr8(ABSY(OAM_BUF + 0x12), (u8)A);
    lda8(rd8(LONGX(0x81959F))); wr8(ABSY(OAM_BUF + 0x13), (u8)A);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(OAM_SIZE_BUF + 0x04), 0x02);
    wr8(ABSX(EFFECT_VY), inc8(rd8(ABSX(EFFECT_VY))));
    lda8(rd8(ABSX(EFFECT_VY)));
    if (fz) {
        lda8(0x03); wr8(ABSX(EFFECT_STEP), 0x03);
        lda8(0x20); wr8(ABSX(EFFECT_TIMER), 0x20);
        wr8(ABSX(EFFECT_FRAME), 0);
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
        lda8(rd8(ABSX(OBJ_ACTIVE)));
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
    wr8(ABSX(OBJ_TYPE), (u8)A);
    lda8(0x01); wr8(ABSX(OBJ_ACTIVE), 0x01);
    rep(0x20);
    lda16(Y);
    lda16(asl16(A));
    Y = (u8)A; nz8((u8)Y);
    lda16(rd16(ABSY(0xAABD))); wr16(ABSX(OBJ_SCRIPT), A);
    lda16(rd16(DP(0x00))); wr16(ABSX(OBJ_X), A);
    lda16(rd16(DP(0x02))); wr16(ABSX(OBJ_Y), A);
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
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (fz) {
        lda8(rd8(ABSX(OBJ_STEP)));
        eor8(0x01);
        wr8(ABSX(OBJ_STEP), (u8)A);
        lda8(0x08); wr8(ABSX(OBJ_DELAY), 0x08);
        wr8(ABSX(OBJ_X), dec8(rd8(ABSX(OBJ_X))));
        lda8(rd8(ABS(OBJ_TYPE + 0x0A)));
        cmp8((u8)A, 0x04);
        int arrived;
        lda8(rd8(ABSX(OBJ_X)));
        if (fz) { cmp8((u8)A, 0x95); arrived = !fc; }
        else { cmp8((u8)A, 0x90); arrived = !fc; }
        if (arrived) {
            ldy8(0x0E);
            do {
                lda8(rd8(ABSY(PLAYER_PLACE)));
                lda8(dec8((u8)A));
                if (fz) break;
                Y = dec8((u8)Y);
                Y = dec8((u8)Y);
            } while (!fn);
            lda8((u8)Y);
            lda8(lsr8((u8)A));
            wr8(DP(0x00), (u8)A);
            lda8(rd8(ABS(PLAYER_CHAR)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
            fc = 0; adc8(rd8(DP(0x00)));
            wr8(DP(0x00), (u8)A);
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0xBEAE)));
            wr8(ABSX(OBJ_STATE), (u8)A);
            lda8(0x40); wr8(ABSX(OBJ_DELAY), 0x40);
        }
    }
    Y = rd8(ABSX(OBJ_STEP)); nz8((u8)Y);
    lda8(rd8(ABSY(0xBEAC)));
    wr8(ABSX(OBJ_FRAME), (u8)A);
    ret();
}

/* $81:BD6E / $81:BDDB: Stadium 1's frame sequences (Stadium 4's $81:C018 /
 * $81:C085 with the object arrays 6 bytes lower, slot $039D). */
static void frame_sequence1(u8 count, u16 times, u8 spawn, u16 dy,
                            u16 frames, u16 back)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (fz) {
        lda8(rd8(ABSX(OBJ_STEP)));
        lda8(inc8((u8)A));
        cmp8((u8)A, count);
        if (fc) {
            lda8(0x04); wr8(ABSX(OBJ_STATE), 0x04);
            lda8(0x08); wr8(ABSX(OBJ_DELAY), 0x08);
            wr8(ABSX(OBJ_STEP), 0);
            ret();
            return;
        }
        wr8(ABSX(OBJ_STEP), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSY(times)));
        wr8(ABSX(OBJ_DELAY), (u8)A);
        lda8(rd8(ABSX(OBJ_STEP)));
        cmp8((u8)A, spawn);
        if (fz) {
            push8((u8)Y);
            rep(0x20);
            lda16(rd16(ABSX(OBJ_X)));
            fc = 1; sbc16(0x0016);
            wr16(DP(0x00), A);
            lda16(rd16(ABSX(OBJ_Y)));
            fc = 1; sbc16(dy);
            wr16(DP(0x02), A);
            sep(0x20);
            ldy8(0x08);
            push8(0x81); push16(back); SpawnObject_m1x1();
            Y = rd8(DP(0x0E)); nz8((u8)Y);
            lda8(0x01); wr8(ABSY(OBJ_STEP), 0x01);
            lda8(0x04); wr8(ABSY(OBJ_RANDOM), 0x04);
            Y = pull8(); nz8((u8)Y);
        }
    }
    Y = rd8(ABSX(OBJ_STEP)); nz8((u8)Y);
    lda8(rd8(ABSY(frames)));
    wr8(ABSX(OBJ_FRAME), (u8)A);
    ret();
}

void FrameSequenceA_m1x1(void) { frame_sequence1(0x04, 0xBDD7, 0x02, 0x001C, 0xBDD3, 0xBDAB); }
void FrameSequenceB_m1x1(void) { frame_sequence1(0x06, 0xBE46, 0x03, 0x0018, 0xBE40, 0xBE18); }

/* $81:B731 ReleaseBalloons (RTS): Stadium 4's $81:B9DB with Stadium 1's
 * object arrays and QueueSound / Random. */
void ReleaseBalloons_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABS(PLAYER_PLACE)));
    cmp8((u8)A, 0x04);
    if (fc) {
        lda8(rd8(ABSX(OBJ_RELEASED)));
        if (!fz) { ret(); return; }
    } else {
        lda8(rd8(DP(TIMER_A)));
        if (fz) {
            lda8(rd8(ABSX(OBJ_RELEASED)));
            if (fz) {
                lda8(0x2B);
                push8(0x81); push16(0xB750); QueueSound_m1x1();
            }
        }
    }
    rep(0x20);
    wr16(DP(TIMER_A), inc16(rd16(DP(TIMER_A))));
    sep(0x20);
    lda8(rd8(DP(0x8B)));
    and8(0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(OBJ_DELAY)));
    if (!fz) { ret(); return; }
    wr8(ABSX(OBJ_RELEASED), inc8(rd8(ABSX(OBJ_RELEASED))));
    push8(0x81); push16(0xB768); Random_m1x1();
    and8(0x3F);
    fc = 0; adc8(0x20);
    wr8(ABSX(OBJ_DELAY), (u8)A);
    ldy8(0x09);
    push8(0x81); push16(0xB776); SpawnObject_m1x1();
    if (fc) { ret(); return; }
    X = rd8(DP(0x0E)); nz8((u8)X);
    push8(0x81); push16(0xB77E); Random_m1x1();
    and8(0x7F);
    fc = 0; adc8(0x40);
    wr8(ABSX(OBJ_X), (u8)A);
    lda8(0xD0); wr8(ABSX(OBJ_Y), 0xD0);
    lda8(0x48);
    push8(0x81); push16(0xB791); QueueSound_m1x1();
    push8(0x81); push16(0xB795); Random_m1x1();
    and8(0x0F);
    fc = 0; adc8(0xB6);
    wr8(ABSX(OBJ_VY), (u8)A);
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    ret();
}

/* $80:C7DC RaceClockStep (RTS): the race clock at VRAM $1897, its frames
 * $03B3 counting up unless the race is over ($84); then TickRaceClock. */
void RaceClockStep_m1x1(void)
{
    lda8(0x97); wr8(DP(0x00), 0x97);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(rd8(DP(RACE_END)));
    if (fz) wr8(ABS(RACE_FRAMES), inc8(rd8(ABS(RACE_FRAMES))));
    TickRaceClock_m1x1();
}

/* $80:C654 RaceTimer (RTS): Stadium 4's $80:C3E1 for Stadium 1 (racing
 * flag $8E, end minutes $80:C6C6, no frame check): the race time runs
 * and is shown; at this race's end minute or after 4 minutes the race
 * ends ($085E cleared, $84 = 2, sound $2D). */
void RaceTimer_m1x1(void)
{
    lda8(rd8(DP(RACE_STARTED)));
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
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(FINAL_RACE)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xC6C6)));
    X = pull8(); nz8((u8)X);
    cmp8((u8)A, rd8(LONG(BCAST_MINUTE)));
    if (fc) {
        lda8(rd8(DP(0x80)));
        cmp8((u8)A, 0x04);
        if (!fc) { ret(); return; }
    }
    wr8(ABS(RIDER_DIST), 0); wr8(ABS(RIDER_DIST + 0x01), 0);
    lda8(0x02); wr8(DP(RACE_END), 0x02);
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
        lda16(rd16(ABSX(MARKER_AGE)));
        cmp16(A, rd16(DP(0x00)));
        if (fc) { wr16(DP(0x00), A); wr8(DP(0x02), (u8)X); }
        sep(0x20);
        lda8(rd8(ABSX(MARKER_ON)));
        if (fz) break;
        X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x08);
        if (fc) {
            X = rd8(DP(0x02)); nz8((u8)X);
            wr8(ABS(MARKER_COUNT), inc8(rd8(ABS(MARKER_COUNT))));
            break;
        }
    }
    wr8(ABSX(MARKER_AGE), 0); wr8(ABSX(MARKER_AGE + 0x01), 0);
    lda8(0x01); wr8(ABSX(MARKER_ON), 0x01);
    rep(0x20);
    lda16(0x0038); wr16(ABSX(MARKER_X), 0x0038);
    lda16(rd16(ABS(RIDER_Y)));
    fc = 0; adc16(0x0010);
    wr16(ABSX(MARKER_Y), A);
    lda16(0x00F8); wr16(ABSX(MARKER_VX), 0x00F8);
    lda16(0x00E0); wr16(ABSX(MARKER_VY), 0x00E0);
    wr16(ABSX(MARKER_FRAC), 0); wr16(ABSX(0x1528), 0);
    sep(0x20);
    lda8(rd8(ABS(COIN_COMBO)));
    cmp8((u8)A, 0x05);
    if (fc) ldy8(0x05);
    else {
        wr8(ABS(COIN_COMBO), inc8(rd8(ABS(COIN_COMBO))));
        Y = rd8(ABS(COIN_COMBO)); nz8((u8)Y);
    }
    lda8(rd8(ABSY(0xD291)));
    fc = 0; adc8(rd8(ABS(SCORE_ADD + 0x03)));
    wr8(ABS(SCORE_ADD + 0x03), (u8)A);
    lda8(rd8(ABSY(0xD298)));
    fc = 0; adc8(rd8(ABS(SCORE_ADD + 0x04)));
    wr8(ABS(SCORE_ADD + 0x04), (u8)A);
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
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        u8 t = dec8(rd8(DP(MODE_TIMER)));
        wr8(DP(MODE_TIMER), t);
        if (!fz) { ret(); return; }
        choice_box(0x00, 0x9B72);
    }
    lda8(rd8(DP(P1_PRESS_LO)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(END_NEXT_STEP)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
        lda8(0x43);
        push8(0x80); push16(0x9B89); QueueSound_m1x1();
        lda8(0x0A); wr8(ABS(END_STEP), 0x0A);
        ret();
        return;
    }
    lda8(0x43);
    push8(0x80); push16(0x9B9D); QueueSound_m1x1();
    lda8(rd8(ABS(END_NEXT_STEP))); wr8(ABS(END_STEP), (u8)A);
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
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
    lda8(0x22); wr8(ABS(DIGIT_BUF), 0x22);
    ldy8(0x00);
    lda8(rd8(DP(0x10)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
    lda8((u8)Y);
    fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    cmp8((u8)Y, 0x00);
    if (fz) { lda8(0xFC); wr8(ABS(DIGIT_ROW), 0xFC); wr8(ABS(DIGIT_ROW + 0x02), 0xFC); }
    push16(0xDEB1); VramQueueTiles20_m1x1();
    ret();
}

/* $80:DE23 (RTS): the bonus result screen starts: $88 = $10, the label
 * $80:DE54 at VRAM $18B6, the tiles $80:DE59 at $0090 ($0F = 8), the
 * next step and a count of 0. */
void BonusResultStart_m1x1(void)
{
    lda8(0x10); wr8(DP(MODE_TIMER), 0x10);
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
    wr8(DP(MODE_STEP), inc8(rd8(DP(MODE_STEP))));
    wr8(DP(0x10), 0);
    bonus_count1();
}

/* $80:DEB3 (RTS): the bonus count-up: every 4 frames ($88) the shown
 * count $09C6 steps toward $09C5 with sound $44; at the end, next step,
 * a $40 wait and for a full score ($09C9) the banner $80:DEF2 at $18F4. */
void BonusCountUp_m1x1(void)
{
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(BONUS_SHOWN)));
    cmp8((u8)A, rd8(ABS(BONUS_RESULT)));
    if (!fz) {
        lda8(inc8((u8)A));
        wr8(ABS(BONUS_SHOWN), (u8)A);
        wr8(DP(0x10), (u8)A);
        lda8(0x44);
        push8(0x80); push16(0xDECA); QueueSound_m1x1();
        lda8(0x04); wr8(DP(MODE_TIMER), 0x04);
        bonus_count1();
        return;
    }
    wr8(DP(MODE_STEP), inc8(rd8(DP(MODE_STEP))));
    lda8(0x40); wr8(DP(MODE_TIMER), 0x40);
    lda8(rd8(ABS(BONUS_RESULT)));
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
    lda8(rd8(DP(RACE_END)));
    if (!fz) { lda8(0x03); goto set; }
    lda8(rd8(ABSX(0xC17F)));
    if (!fz) {
        lda8(rd8(ABSX(RIDER_PAGE)));
        lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
        if (!fc) {
            lda8(rd8(ABS(RIDER_SPEED_HI)));
            if (!fz) {
                wr8(ABSX(RIDER_TARGET_HI), (u8)A);
                lda8(rd8(ABS(RIDER_SPEED_LO)));
                wr8(ABSX(RIDER_TARGET_LO), (u8)A);
                lda8(rd8(ABSX(RIDER_PAGE)));
                if (!fz) {
                    lda8(rd8(ABSX(RIDER_X)));
                    cmp8((u8)A, 0x40);
                    if (fc) goto plain;
                    lda8(rd8(ABS(RIDER_BAND)));
                    cmp8((u8)A, rd8(ABS(RIDER_BAND + 0x03)));
                    if (fz) { ret(); return; }
                }
                lda8(rd8(ABSX(RIDER_TARGET_HI)));
                cmp8((u8)A, 0x04);
                if (!fc) wr8(ABSX(RIDER_TARGET_HI), inc8(rd8(ABSX(RIDER_TARGET_HI))));
                ret();
                return;
            }
        }
    }
plain:
    lda8(rd8(ABSY(0xBDD7)));
set:
    wr8(ABSX(RIDER_TARGET_HI), (u8)A);
    lda8(rd8(DPX(0x2F)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBDCF)));
    wr8(ABSX(RIDER_TARGET_LO), (u8)A);
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
    lda8(rd8(ABSX(RIDER_AIR)));
    if (fz) { ret(); return; }
    push16(0xC22C); GroundHeightAbove_m1x1();
    sbc8(rd8(ABS(PLAYER_GROUND)));
    wr8(DP(0x01), (u8)A);
    cmp8((u8)A, rd8(ABSX(RIDER_Y)));
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x0904)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_LANDED)));
    if (fz) { ret(); return; }
    lda8((u8)X);
    if (fz) { lda8(0x21); push8(0x80); push16(0xC249); QueueSound_m1x1(); }
    lda8(0x00);
    wr8(ABSX(RIDER_AIR), 0);
    wr8(ABSX(RIDER_LANDED), 0);
    lda8(rd8(ABS(PLAYER_GROUND)));
    wr8(ABS(PLAYER_LIFT), (u8)A);
    lda8(rd8(ABSX(RIDER_BUMPED)));
    if (!fz) goto fall;
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        lda8(rd8(ABSX(RIDER_POSE)));
        if (!fz) { ret(); return; }
        lda8(0x06); wr8(ABSX(RIDER_LEAN), 0x06);
        ret();
        return;
    }
    Y = rd8(ABSX(RIDER_SLOW));
    wr8(ABSX(RIDER_SLOW), (u8)A);
    lda8((u8)Y);
    if (!fz) goto fall;
    lda8(rd8(ABSX(RIDER_POSE)));
    Y = rd8(ABSX(RIDER_SPEED_HI)); nz8((u8)Y);
    cmp8((u8)Y, 0x02);
    {
        int slow = !fc;
        if (!slow && fz) {
            Y = rd8(ABSX(RIDER_SPEED_LO)); nz8((u8)Y);
            cmp8((u8)Y, 0x80);
            slow = !fc;
        }
        if (slow) { fc = 0; adc8(0x08); }
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, rd8(ABSY(0xBD46)));
    if (fc) {
        cmp8((u8)A, rd8(ABSY(0xBD56)));
        if (!fc) {
            lda8(rd8(ABSX(RIDER_PIECE_LEAN)));
            cmp8((u8)A, rd8(ABSX(RIDER_LEAN)));
            if (!fz) {
                lda8(rd8(ABSX(RIDER_POSE)));
                ora8(rd8(ABSX(RIDER_PIECE_ACTION)));
                if (fz) {
                    lda8(0x04); wr8(ABSX(RIDER_COUNTDOWN), 0x04);
                    lda8(lsr8((u8)A)); wr8(ABSX(RIDER_SLOW), (u8)A);
                    lda8(lsr8((u8)A)); wr8(ABSX(RIDER_AIR), (u8)A);
                }
            }
            goto fall;
        }
    }
    cmp8((u8)X, 0x00);
    if (fz) { lda8(0x3D); push8(0x80); push16(0xC297); QueueSound_m1x1(); }
    lda8(0x0A); wr8(DPX(RIDER_HOLD), 0x0A);
    lda8(0xFF); wr8(ABSX(RIDER_BUMPED), 0xFF);
    lda8((u8)X);
    if (!fz) lda8(0x05);
    fc = 0; adc8(0x24);
    push8(0x80); push16(0xC2AC); QueueSound_m1x1();
    ret();
    return;
fall:
    wr8(DP(0x01), dec8(rd8(DP(0x01))));
    lda8(rd8(DP(0x01)));
    wr8(ABSX(RIDER_Y), (u8)A);
    ret();
}

/* $80:C1C9 (RTS): when the ball is due ($092C) and rider 7 is off screen
 * (not page 1, page 0 left of $EA), it is cleared for reuse
 * (ClearRiderState). */
void LaunchBall_m1x1(void)
{
    lda8(rd8(ABS(BALL_DUE)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_ACTIVE + 0x03)));
    ldx8(0x07);
    lda8(rd8(ABSX(RIDER_PAGE)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    lda8(lsr8((u8)A));
    if (!fc) {
        lda8(rd8(ABSX(RIDER_X)));
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
    lda8(rd8(DP(RACE_STARTED)));
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
    lda8(rd8(ABS(RIDER_STEP)));
    lda8(lsr8((u8)A)); lda8(ror8((u8)A)); lda8(ror8((u8)A)); lda8(ror8((u8)A));
    fc = 0; adc8(rd8(ABS(SCROLL_FRAC)));
    wr8(ABS(SCROLL_FRAC), (u8)A);
    lda8(rd8(ABS(RIDER_STEP)));
    adc8(rd8(DP(FAR_SCROLL)));
    wr8(DP(FAR_SCROLL), (u8)A);
    if (fc) {
        lda8(rd8(DP(FAR_HALF)));
        eor8(0x01);
        wr8(DP(FAR_HALF), (u8)A);
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
        lda8(rd8(ABSX(RIDER_X)));
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
    lda8(rd8(ABSX(RIDER_ON_RAMP)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    ora8(rd8(ABSX(RIDER_SPEED_LO)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_HEIGHT)));
    if (!fz) { ret(); return; }
    int margin = port_ws_spark_margin();     /* port.h: a rider in the widescreen margins */
    if (margin) {
        lda8(rd8(ABSX(RIDER_X)));
    } else {
        lda8(rd8(ABSX(RIDER_PAGE)));
        lda8(lsr8((u8)A));
        if (!fc) { ret(); return; }
        lda8(rd8(ABSX(RIDER_X)));
        cmp8((u8)A, 0x10);
        if (!fc) { ret(); return; }
    }
    push16(0xCE17); DrawRiderShadowPair_m1x1();
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x04);
    if (!fz) {
        lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC2), 0xF0);
        lda8(0xE0); wr8(ABSY(OAM_BUF + 0xC6), 0xE0);
    }
    lda8(rd8(ABSX(RIDER_BAND)));
    if (fz) {
        lda8(0x0C); wr8(DP(0x02), 0x0C);
    } else {
        push8((u8)Y);
        Y = rd8(ABSX(RIDER_TILE_Y)); nz8((u8)Y);
        lda8(rd8(ABSY(0xACF9)));
        Y = pull8(); nz8((u8)Y);
        wr8(DP(0x02), (u8)A);
        cmp8((u8)A, 0x00);
        if (fz) { lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC1), 0xF0); wr8(ABSY(OAM_BUF + 0xC5), 0xF0); }
    }
    lda8(rd8(ABSX(RIDER_Y)));
    cmp8((u8)A, 0x58);
    if (!fc) lda8(0x0C);
    else lda8(rd8(DP(0x02)));
    ora8(0x21);
    wr8(ABSY(OAM_BUF + 0xC3), (u8)A);
    wr8(ABSY(OAM_BUF + 0xC7), (u8)A);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) { lda8(0xF8); wr8(ABSY(OAM_BUF + 0xC5), 0xF8); }
    if (margin) port_ws_spark_fix();
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
        Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
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
    Y = rd8(ABSX(RIDER_BAND));
    Y = up ? inc8((u8)Y) : dec8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    ldy8(0x00);
    push16(up ? 0xAF63 : 0xAF88); RiderAhead_m1x1();
    if (fc) { ret(); return; }
    lda8(up ? 0x01 : 0xFF);
    wr8(ABSX(RIDER_STEER), (u8)A);
    ret();
}

void CpuSteerUp_m1x1(void) { steer_lane1(1); }
void CpuSteerDown_m1x1(void) { steer_lane1(0); }

/* $80:AF6A: down, unless rider X's piece $0210 is $E6 (then up). */
static void steer_by_e6_1(void)
{
    lda8(rd8(ABSX(RIDER_TILE_Y)));
    cmp8((u8)A, 0xE6);
    steer_lane1(fz);
}

/* $80:AF07 CpuLaneDecision (RTS, X = rider): on screen page 1, blocked by
 * a rider ahead: let go of the buttons and in the right half steer away
 * (by its pad bit 0 / $0210); otherwise, not steering, at X $80-$EF with
 * its random $2D at $E0 or more: up ($F8 on) or by $0210. */
void CpuLaneDecision_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_PAGE)));
    lda8(lsr8((u8)A));
    if (!fc) { ret(); return; }
    lda8(rd8(ABSX(RIDER_BAND)));
    wr8(DP(0x02), (u8)A);
    ldy8(0x02);
    push16(0xAF16); RiderAhead_m1x1();
    if (fc) {
        push16(0xAF1B); CpuReleaseButtons_m1x1();
        lda8(rd8(ABSX(RIDER_X)));
        lda8(asl8((u8)A));
        if (!fc) { ret(); return; }
        lda8(rd8(DPX(RANDOM_LFSR)));
        lda8(lsr8((u8)A));
        if (fc) steer_by_e6_1();
        else steer_lane1(1);
        return;
    }
    lda8(rd8(ABSX(RIDER_STEER)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_PAGE)));
    lda8(lsr8((u8)A));
    if (!fc) { ret(); return; }
    lda8(rd8(ABSX(RIDER_X)));
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
        lda8(rd8(ABSY(PIECES_AHEAD)));
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
    fc = 0; adc8(rd8(ABSX(RIDER_COLUMN)));
    and8(0x3F);
    wr8(DP(0x08), (u8)A);
    lda8(0x40); wr8(DP(0x09), 0x40);
    Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
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
    Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
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
    lda8(rd8(ABSX(RIDER_SCRIPT)));
    if (fz) { Y = rd8(ABSX(RIDER_PIECE_LEAN)); nz8((u8)Y); }
    lda8((u8)Y);
store:
    wr8(ABSX(RIDER_WANT_LEAN), (u8)A);
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
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (fz) goto next;
        lda8(rd8(ABSX(RIDER_BUMPED)));
        ora8(rd8(ABSX(RIDER_TIMER)));
        if (!fz) goto next;
        push16(0xAED9); CpuSteer_m1x1();
        lda8(rd8(ABSX(RIDER_AIR)));
        if (!fz) {
            lda8(rd8(ABSX(RIDER_PIECE_LEAN)));
            wr8(ABSX(RIDER_WANT_LEAN), (u8)A);
            goto next;
        }
        push16(0xAEE1); CpuThrottle_m1x1();
        push16(0xAEE4); CpuAvoidHurdle_m1x1();
        if (!fz) goto next;
        lda8(rd8(ABS(RIDER_IN_MIDDLE)));
        if (fz) goto next;
        push16(0xAEEE); RetargetRival_m1x1();
        lda8(rd8(DP(RACE_END)));
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
        lda8(rd8(ABSX(RIDER_PAGE)));
        lda8(lsr8((u8)A));
        if (fc) {
            lda8(rd8(ABSX(RIDER_X)));
            cmp8((u8)A, 0xF4);
            marker = !fc;
        } else {
            lda8(lsr8((u8)A));
            marker = 0;
            if (!fc) {
                lda8(rd8(ABSX(RIDER_X)));
                cmp8((u8)A, 0xF6);
                marker = fc;
            }
        }
        if (marker) {
            lda8(rd8(ABSX(RIDER_AIR)));
            if (!fz) {
                lda8(0xB6);
                push16(0xED0C); HeightAbove_m1x1();
                wr8(ABSY(OAM_BUF + 0xA1), (u8)A);
                lda8(rd8(ABSX(RIDER_X)));
                fc = 0; adc8(0x0A);
                wr8(ABSY(OAM_BUF + 0xA0), (u8)A);
                wr8(DP(0x01), (u8)Y);
                Y = rd8(ABSX(RIDER_ACTION_POSE)); nz8((u8)Y);
                lda8(rd8(ABSY(0xE6EE)));
                Y = rd8(DP(0x01)); nz8((u8)Y);
                wr8(ABSY(OAM_BUF + 0xA2), (u8)A);
                push8((u8)Y);
                lda8((u8)Y);
                fc = 0; adc8(0xA0);
                lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
                Y = (u8)A; nz8((u8)Y);
                lda8(0x02); wr8(ABSY(OAM_SIZE_BUF), 0x02);
                Y = pull8(); nz8((u8)Y);
                lda8(0x21); wr8(ABSY(OAM_BUF + 0xA3), 0x21);
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
    wr8(ABSX(RIDER_LANE), (u8)A);
    Y = rd8(ABSX(RIDER_PAGE)); nz8((u8)Y);
    lda8(fz ? 0xA0 : 0x40);
    wr8(ABSX(RIDER_X), (u8)A);
    lda8(0x06); wr8(ABSX(RIDER_LEAN), 0x06);
    lda8(rd8(ABS(RIDER_COLUMN)));
    fc = 0; adc8(rd8(ABSY(0xBDE8)));
    and8(0x3F);
    wr8(ABSX(RIDER_COLUMN), (u8)A);
    lda8(rd8(ABS(RIDER_WHEEL)));
    wr8(ABSX(RIDER_WHEEL), (u8)A);
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
                lda8(rd8(ABSX(RIDER_LANE)));
                cmp8((u8)A, 0x20);
                if (fc) {
                    lda8(0x30); wr8(ABSX(RIDER_HEIGHT), 0x30);
                    lda8(0x01);
                } else lda8(0x03);
                wr8(ABSX(RIDER_JUMP), (u8)A);
            }
        }
    }
    Y = rd8(ABSX(RIDER_PAGE)); nz8((u8)Y);
    push16(0xC10D); RivalTarget_m1x1();
    push8(0x80); push16(0xC111); Random_m1x1();
    and8(0x3F);
    if (fz) {
        lda8(rd8(ABSX(RIDER_PAGE)));
        eor8(0x02);
        lda8(asl8((u8)A));
        wr8(ABSX(RIDER_SPEED_HI), (u8)A);
    }
    lda8(0x10); wr8(DP(0x3B), 0x10);
    lda8(0x01); wr8(ABSX(RIDER_ACTIVE), 0x01);
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
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (fz) {
            rep(0x20);
            lda16(rd16(DP(SCROLL_X)));
            fc = 0; adc16(0x0170);
            cmp16(A, rd16(ABSY(RIDER_DIST)));
            int place = fc;
            if (!place) {
                lda16(rd16(DP(SCROLL_X)));
                fc = 1; sbc16(0x00A0);
                if (!fn) {
                    cmp16(A, rd16(ABSY(RIDER_DIST)));
                    place = fc;
                }
            }
            if (place) {
                sep(0x20);
                lda8(rd8(DP(RACE_END)));
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
    lda8(rd8(ABSX(RIDER_FINISHED)));
    if (fz) goto arrived;
    lda8(rd8(DPX(RIDER_LEAN_TIMER)));
    if (!fz) { ret(); return; }
    lda8((u8)X);
    int step;
    if (fz) {
        Y = rd8(DP(TURN_LATCH)); nz8((u8)Y);
        lda8(rd8(ABS(RIDER_INPUT)));
        and8(0xC0);
        if (!fz) {
            wr8(DP(TURN_LATCH), (u8)A);
            lda8((u8)Y);
            if (fz) { step = 1; goto move; }
        }
        wr8(DP(TURN_LATCH), (u8)A);
    }
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x0F);
    step = fz;
move:
    if (step) wr8(ABSX(RIDER_RUNOUT), dec8(rd8(ABSX(RIDER_RUNOUT))));
    and8(0x07);
    if (fz) {
        lda8(rd8(ABSX(FINISH_POSE)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x02);
        if (fz) lda8(inc8((u8)A));
        cmp8((u8)A, 0x05);
        if (fz) {
            lda8(0x23);
            push8(0x80); push16(0xBF01); QueueSound_m1x1();
            lda8(0x00);
            wr8(ABSX(FINISH_POSE), 0x00);
        } else {
            cmp8((u8)A, 0x03);
            if (fz) {
                wr8(ABSX(FINISH_POSE), (u8)A);
                lda8(0x23);
                push8(0x80); push16(0xBEF9); QueueSound_m1x1();
            } else wr8(ABSX(FINISH_POSE), (u8)A);
        }
    }
    lda8(rd8(ABSX(RIDER_RUNOUT)));
    fc = 1; sbc8(0x08);
    cmp8((u8)A, rd8(ABSX(RIDER_X)));
    if (fc) { ret(); return; }
arrived:
    lda8(0x03); wr8(DPX(RIDER_DONE), 0x03);
    wr8(ABSX(RIDER_TIMER), inc8(rd8(ABSX(RIDER_TIMER))));
    lda8(0x00); wr8(ABSX(RIDER_WAVING), 0x00);
    cmp8((u8)X, 0x07);
    if (fz) { wr8(ABS(BALL_DUE), 0); wr8(ABS(BALL_OUT), 0); }
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
    X = rd8(ABS(RIDER_COLUMN)); nz8((u8)X);
    lda8(rd8(ABS(PLAYER_LIFT)));
    ora8(rd8(ABS(RIDER_AIR)));
    if (!fz) {
        lda8(rd8(ABS(RIDER_COLUMN)));
        cmp8((u8)A, rd8(ABS(0x09C8)));
        if (!fz) {
            wr8(ABS(0x09C8), (u8)A);
            lda8(rd8(ABSX(TRACK_ROWS + 0xC0)));
            fc = 0; adc8(rd8(ABS(BONUS_RESULT)));
            wr8(ABS(BONUS_RESULT), (u8)A);
        }
    }
    lda8(rd8(ABS(RIDER_BUMPED)));
    if (fz) { lda8(rd8(ABSX(TRACK_ROWS + 0x40))); wr8(ABS(PLAYER_GROUND), (u8)A); }
    lda8(rd8(ABSX(TRACK_ROWS)));
    wr8(ABS(RIDER_TILE_Y), (u8)A);
    cmp8((u8)A, 0x28);
    if (!fc) {
        lda8(rd8(ABS(COURSE_ENDED)));
        if (fz) {
            lda8(0x01); wr8(ABS(COURSE_ENDED), 0x01); wr8(DP(RACE_END), 0x01);
            lda8(0x2B);
            push8(0x80); push16(0xDADF); QueueSound_m1x1();
        }
        ret();
        return;
    }
    lda8(rd8(ABSX(TRACK_ROWS + 0x80)));
    if (!fz) {
        lda8(rd8(ABS(PLAYER_LIFT)));
        ora8(rd8(ABS(RIDER_AIR)));
        if (!fz) { lda8(0x45); wr8(ABS(RIDER_TILE_Y), 0x45); ret(); return; }
        lda8(rd8(ABSX(TRACK_ROWS)));
        and8(0x40);
        if (!fz) { ret(); return; }
        wr8(ABSX(TRACK_ROWS + 0x80), 0);
        lda8(0x3B);
        wr8(ABSX(TRACK_ROWS + 0x180), 0x3B); wr8(ABSX(TRACK_ROWS + 0x1C0), 0x3B); wr8(ABSX(TRACK_ROWS + 0x200), 0x3B);
        wr8(ABSX(TRACK_ROWS + 0x240), 0x3B); wr8(ABSX(TRACK_ROWS + 0x280), 0x3B); wr8(ABSX(TRACK_ROWS + 0x2C0), 0x3B);
        wr8(ABS(RIDER_TILE), 0x3B);
    }
    lda8(rd8(ABSX(TRACK_ROWS)));
    cmp8((u8)A, 0x84);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(PLAYER_LIFT)));
    if (!fz) { ret(); return; }
    lda8(0xFC); wr8(ABS(RIDER_TILE_Y), 0xFC);
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
        lda16(rd16(ABS(ENTRY_DIGITS + i)));
        and16(0x00FF);
        wr16(DP(0x00), A);
        lda16(mul[i]); wr16(DP(0x02), mul[i]);
        sep(0x20);
        push8(0x80); push16(back[i][0]); Mul16_m1x1();
        push16(back[i][1]); Add32_m1x1();
    }
    lda8(rd8(ABS(ENTRY_DIGITS + 0x03))); wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA28A); Add32_m1x1();
    lda8(rd8(DP(0x10)));
    and8(0x0F);
    ora8(rd8(ABS(ENTRY_CODE + 0x0A)));
    wr8(ABS(ENTRY_CODE + 0x0A), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x09), (u8)A);
    lda8(rd8(DP(0x11)));
    and8(0x0F);
    wr8(ABS(ENTRY_CODE + 0x08), (u8)A);
    lda8(rd8(DP(0x11)));
    and8(0x30);
    ora8(rd8(ABS(ENTRY_CODE + 0x05)));
    wr8(ABS(ENTRY_CODE + 0x05), (u8)A);
    retl();
}

/* $81:99CB (RTL): every 8 frames the palette entries from $B1 down
 * (skipping each group's colour 0) step one unit per channel toward the
 * targets $81:9A45 (blue), $81:9A5D (green), $81:9A75 (red). */
void FadePaletteStep_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x07);
    if (fz) {
        X = rd8(DP(FADE_FROM)); nz8((u8)X);
        rep(0x20);
        static const u16 mask[3] = { 0x7C00, 0x03E0, 0x001F };
        static const u16 tgt[3] = { 0x9A45, 0x9A5D, 0x9A75 };
        static const u16 unit[3] = { 0x0400, 0x0020, 0x0001 };
        do {
            lda16((u16)X);
            and16(0x0007);
            if (!fz) {
                for (int c = 0; c < 3; c++) {
                    lda16(rd16(ABSX(PALETTE_BUF)));
                    and16(mask[c]);
                    cmp16(A, rd16(ABSX(tgt[c])));
                    if (fz) continue;
                    if (!fc) { lda16(rd16(ABSX(PALETTE_BUF))); fc = 0; adc16(unit[c]); }
                    else { lda16(rd16(ABSX(PALETTE_BUF))); fc = 1; sbc16(unit[c]); }
                    wr16(ABSX(PALETTE_BUF), A);
                }
            }
            X = dec8((u8)X);
            X = dec8((u8)X);
        } while (!fn);
        sep(0x20);
    }
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
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
        lda16(rd16(LONG(SAVE_BEST_TIME + i)));
        and16(0x00FF);
        wr16(DP(0x00), A);
        lda16(mul[i]); wr16(DP(0x02), mul[i]);
        sep(0x20);
        push8(0x80); push16(back[i][0]); Mul16_m1x1();
        push16(back[i][1]); Add32_m1x1();
    }
    lda8(rd8(LONG(SAVE_BEST_FRAC))); wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA200); Add32_m1x1();
    lda8(rd8(DP(0x11)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x04), (u8)A);
    lda8(rd8(DP(0x11))); and8(0x0F); wr8(ABS(ENTRY_CODE + 0x05), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x06), (u8)A);
    lda8(rd8(DP(0x10))); and8(0x0F); wr8(ABS(ENTRY_CODE + 0x07), (u8)A);
    lda8(rd8(DP(RANDOM_LFSR)));
    and8(0x02);
    ora8(rd8(DP(0x12)));
    and8(0x03);
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x0A), (u8)A);
    retl();
}

/* $80:9053 (RTL, X = row): a six-sprite text row at OAM $1700: X $68 on
 * in steps of 8, Y $82, tiles $60 + 16X on, attribute $21, small. */
void SpriteTextRow_m1x1(void)
{
    lda8(0x68);
    for (int i = 0; i < 6; i++) {
        if (i) { fc = 0; adc8(0x08); }
        wr8(ABS(OAM_BUF + 0x100 + 4 * i), (u8)A);
    }
    lda8(0x82);
    for (int i = 0; i < 6; i++) wr8(ABS(OAM_BUF + 0x101 + 4 * i), 0x82);
    lda8((u8)X);
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    fc = 0; adc8(0x60);
    for (int i = 0; i < 6; i++) {
        if (i) lda8(inc8((u8)A));
        wr8(ABS(OAM_BUF + 0x102 + 4 * i), (u8)A);
    }
    lda8(0x21);
    for (int i = 0; i < 6; i++) wr8(ABS(OAM_BUF + 0x103 + 4 * i), 0x21);
    for (int i = 0; i < 6; i++) wr8(ABS(OAM_SIZE_BUF + 0x40 + i), 0);
    retl();
}

/* $80:C947 on: the best time at VRAM $18D7, then the tail below. */
static void record_time_tail1(void)
{
    lda8(rd8(DP(RACE_END)));
    if (fz) { wr8(ABS(RACE_MIN), 0); wr8(ABS(RACE_SEC), 0); wr8(ABS(RACE_FRAMES), 0); }
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
    lda8(rd8(ABS(LAP)));
    lda8(dec8((u8)A)); lda8(dec8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x00)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(RACE_MIN))); wr8(ABSX(COURSE_TIMES), (u8)A);
    lda8(rd8(ABS(RACE_SEC))); wr8(ABSX(COURSE_TIMES + 0x01), (u8)A);
    lda8(rd8(ABS(RACE_FRAMES))); wr8(ABSX(COURSE_TIMES + 0x02), (u8)A);
    int better = 0;
    lda8(rd8(ABS(RECORD_TIME)));
    cmp8((u8)A, rd8(ABS(RACE_MIN)));
    if (fc) {
        if (!fz) better = 1;
        else {
            lda8(rd8(ABS(RECORD_TIME + 0x01)));
            cmp8((u8)A, rd8(ABS(RACE_SEC)));
            if (fc) {
                if (!fz) better = 1;
                else {
                    lda8(rd8(ABS(RECORD_TIME + 0x02)));
                    cmp8((u8)A, rd8(ABS(RACE_FRAMES)));
                    better = fc && !fz;
                }
            }
        }
    }
    if (better) {
        lda8(rd8(ABS(RACE_MIN))); wr8(ABS(RECORD_TIME), (u8)A);
        lda8(rd8(ABS(RACE_SEC))); wr8(ABS(RECORD_TIME + 0x01), (u8)A);
        lda8(rd8(ABS(RACE_FRAMES))); wr8(ABS(RECORD_TIME + 0x02), (u8)A);
        lda8(rd8(ABS(LAP)));
        lda8(dec8((u8)A));
        wr8(ABS(RECORD_COURSE), (u8)A);
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
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(DP(MODE_TIMER)));
    if (fz) {
        lda8(rd8(DP(FADE_FROM)));
        cmp8((u8)A, 0x16);
        if (fc) {
            wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
            X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
            ret();
            return;
        }
        fc = 0; adc8(0x08);
        wr8(DP(FADE_FROM), (u8)A);
        lda8(0xC0); wr8(DP(MODE_TIMER), 0xC0);
    }
    wr8(DP(MODE_TIMER), dec8(rd8(DP(MODE_TIMER))));
    push8(0x81); push16(0x984D); FadePaletteStep_m1x1();
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
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
    lda8(0x27); wr8(ABS(DIGIT_BUF), 0x27);
    static const u8 t[5] = { 0x7D, 0x72, 0x76, 0x5E, 0x96 };
    for (int i = 0; i < 5; i++) {
        lda8(t[i]); wr8(ABS(DIGIT_ROW + i), t[i]);
        fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x07 + i), (u8)A);
    }
    ldy8(0x00);
    lda8(rd8(DP(BONUS_SECONDS)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x0D), (u8)A);
    lda8((u8)Y);
    fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x0C), (u8)A);
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
    lda8(rd8(ABSX(RIDER_TIMER)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) {
            ldy8(0x06);
            lda8(rd8(ABSX(RIDER_WAVING)));
            if (fz) ldy8(0x02);
            lda8((u8)Y);
            Y = rd8(DPX(RIDER_DONE)); nz8((u8)Y);
            if (!fz) { Y = dec8((u8)Y); lda8((u8)Y); }
            ora8(0x10);
            ret();
            return;
        }
    }
    Y = rd8(DP(RACE_END)); nz8((u8)Y);
    Y = dec8((u8)Y);
    if (fz) {
        lda8(rd8(ABSX(RIDER_LEAN)));
        cmp8((u8)A, 0x0A);
        if (fz) { ldy8(0x02); wr8(DP(0x0E), 0x02); ret(); return; }
    }
    lda8(rd8(ABSX(RIDER_BUMPED)));
    ora8(rd8(ABSX(RIDER_AIR)));
    if (!fz) goto angle;
    lda8(rd8(ABSX(RIDER_SCRIPT)));
    if (!fz) {
        cmp8((u8)A, 0x0D);
        if (!fz) goto angle;
        lda8(rd8(ABSX(RIDER_LANE)));
        cmp8((u8)A, 0x20);
    }
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    ora8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) {
        wr8(DP(0x0E), inc8(rd8(DP(0x0E))));
        goto angle;
    }
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, 0x06);
    if (!fz) goto angle;
    ldy8(0x14);
    lda8(rd8(ABSX(RIDER_STEER)));
    if (!fz) {
        if (!fn) Y = inc8((u8)Y);
        lda8((u8)Y);
        ret();
        return;
    }
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x02);
    if (!fz) { lda8(0x13); ret(); return; }
angle:
    lda8(rd8(ABSX(RIDER_LEAN)));
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
        lda8(rd8(ABSX(RIDER_BUMPED)));
        if (!fz) {
            ldy8(0x04);
            lda8((u8)X);
            ora8(rd8(ABS(ATTRACT_DEMO)));
            if (fz) {
                wr8(ABS(GAUGE_ON), (u8)A);
                lda8(rd8(DP(RIDER_TURN)));
                if (fz) { lda8(0x25); push8(0x80); push16(0xC4A4); QueueSound_m1x1(); }
                lda8(rd8(DP(0x2F)));
                and8(0x03);
                Y = (u8)A; nz8((u8)Y);
            }
            lda8((u8)Y);
            wr8(ABSX(0x0948), (u8)A);
            lda8(rd8(ABSX(RIDER_SPEED_HI)));
            if (!fz) goto turn;
            lda8(rd8(ABSX(RIDER_AIR)));
            ora8(rd8(ABSX(RIDER_SCRIPT)));
            if (!fz) goto slide;
            wr8(ABSX(RIDER_SPEED_LO), (u8)A);
            wr8(ABSX(RIDER_STEP), (u8)A);
            wr8(ABSX(RIDER_BUMPED), (u8)A);
            lda8(0x06); wr8(ABSX(RIDER_LEAN), 0x06);
            lda8(rd8(ABSX(RIDER_X)));
            fc = 0; adc8(0x08);
            wr8(ABSX(RIDER_RUNOUT), (u8)A);
            push16(0xC4D4); RiderShadowY_m1x1();
            lda8(0x02); wr8(ABSX(FINISH_POSE), 0x02);
            lda8(lsr8((u8)A));
            wr8(ABSX(RIDER_TIMER), (u8)A);
            cmp8((u8)X, 0x00);
            if (fz) { lda8(0x3E); push8(0x80); push16(0xC4E7); QueueSound_m1x1(); }
            goto next;
        slide:
            lda8(0x88); wr8(ABSX(RIDER_SPEED_LO), 0x88);
        turn:
            lda8(rd8(DPX(RIDER_HOLD)));
            if (fz) {
                lda8(rd8(ABSX(RIDER_LEAN)));
                fc = 0; adc8(rd8(ABSX(RIDER_BUMPED)));
                and8(0x0F);
                wr8(ABSX(RIDER_LEAN), (u8)A);
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
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    cmp8((u8)A, rd8(ABS(CRASH_PHASE)));
    if (!fz) { ret(); return; }
    wr8(ABS(CRASH_PHASE), dec8(rd8(ABS(CRASH_PHASE))));
    lda8(rd8(ABS(CRASH_PHASE)));
    and8(0x03);
    wr8(ABS(CRASH_PHASE), (u8)A);
    crash_loop1(0);
}

/* $80:DD0E PlayerCrashStep (RTS): Stadium 4's $80:DBFC for Stadium 1
 * (crashed $01C8, air $01E8, speed $01C0:$01B8, $0170): getting up,
 * sliding (CrashRecovery's lean from $80:C4EF), or ending the tumble
 * with PlayerHalt and the crash landing. */
void PlayerCrashStep_m1x1(void)
{
    lda8(rd8(ABS(RIDER_BUMPED)));
    if (fz) { ret(); return; }
    lda8(0x00); wr8(ABS(GAUGE_ON), 0x00);
    lda8(rd8(DP(RIDER_TURN)));
    if (fz) lda8(0x01);
    lda8(rd8(ABS(PLAYER_TUMBLE)));
    if (!fz) {
        lda8(rd8(ABS(RIDER_AIR)));
        if (fz) {
            lda8(rd8(ABS(RIDER_SPEED_HI)));
            if (fz) {
                lda8(rd8(ABS(RIDER_SCRIPT)));
                int up = 0;
                if (fz) {
                    lda8(rd8(ABS(RIDER_LEAN)));
                    cmp8((u8)A, 0x06);
                    up = fz;
                }
                if (up) {
                    lda8(0x00);
                    wr8(ABS(RIDER_SPEED_LO), 0); wr8(ABS(RIDER_BUMPED), 0); wr8(ABS(PLAYER_TUMBLE), 0);
                    ret();
                    return;
                }
                lda8(0x88); wr8(ABS(RIDER_SPEED_LO), 0x88);
            }
        }
        goto slide;
    }
    lda8(0x00);
    wr8(ABS(RIDER_SPEED_LO), 0); wr8(ABS(RIDER_SPEED_HI), 0); wr8(ABS(RIDER_STEP), 0);
    lda8(rd8(DP(TUMBLE_TIMER)));
    if (fz) { lda8(0x20); wr8(DP(TUMBLE_TIMER), 0x20); }
    cmp8((u8)A, 0x1C);
    if (fc) { ret(); return; }
    cmp8((u8)A, 0x01);
    if (!fz) goto slide;
    wr8(ABS(PLAYER_TUMBLE), (u8)A);
    lda8(lsr8((u8)A));
    wr8(ABS(PLAYER_GROUND), (u8)A);
    lda8(rd8(ABS(RIDER_SCRIPT)));
    if (!fz) { ret(); return; }
    push16(0xDD7E); player_halt1();
    push16(0xDDD5); CrashLand_m1x1();
    lda8(rd8(ABS(RIDER_BOUNCE_LO))); wr8(ABS(RIDER_HEIGHT_LO), (u8)A);
    lda8(rd8(ABS(RIDER_BOUNCE_HI))); wr8(ABS(RIDER_HEIGHT_HI), (u8)A);
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
    wr8(DP(BONUS_TICK), dec8(rd8(DP(BONUS_TICK))));
    if (fz) {
        lda8(0x3C); wr8(DP(BONUS_TICK), 0x3C);
        wr8(DP(BONUS_SECONDS), dec8(rd8(DP(BONUS_SECONDS))));
        if (fz) {
            lda8(0x02); wr8(DP(RACE_END), 0x02);
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
    lda8(rd8(ABSX(RIDER_X))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(RIDER_Y)));
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
    lda8(rd8(ABSX(RANK_ORDER)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSY(0xCD34)));
    wr8(ABSX(SPRITE_FRAME), (u8)A);
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(rd8(ABS(RIDER_BUMPED)));
        ora8(rd8(ABS(RIDER_AIR)));
        if (fz) {
            lda8(rd8(DP(RACE_END)));
            cmp8((u8)A, 0x01);
            if (fz) { lda8(0x14); wr8(ABSX(SPRITE_FRAME), 0x14); }
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
    lda8(rd8(ABS(GAUGE_ON)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(RIDER_TURN)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    ldx8(0x02);
    lda8(rd8(ABS(RIDER_SPEED_HI))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABS(RIDER_SPEED_LO))); wr8(DP(0x01), (u8)A);
    do {
        u8 h = rd8(DP(0x00));
        fc = h & 1; h >>= 1; wr8(DP(0x00), h); nz8(h);
        u8 l = rd8(DP(0x01));
        u8 c = fc; fc = l & 1; l = (u8)((l >> 1) | (c << 7)); wr8(DP(0x01), l); nz8(l);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x02);
    Y = rd8(ABS(RIDER_AIR)); nz8((u8)Y);
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
        lda8(rd8(DP(FRAME_COUNT)));
        and8(0x08);
        if (fz) X = dec8((u8)X);
    }
    lda8(rd8(ABS(RIDER_INPUT)));
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
    lda8(rd8(ABSX(OBJ_VX)));
    if (fn) Y = dec8((u8)Y);
    lda8((u8)Y); wr8(ABSX(OBJ_VX + 0x01), (u8)A);
    ldy8(0x00);
    lda8(rd8(ABSX(OBJ_VY)));
    if (fn) Y = dec8((u8)Y);
    lda8((u8)Y); wr8(ABSX(OBJ_VY + 0x01), (u8)A);
    rep(0x20);
    wr16(DP(0x04), lsr16(rd16(DP(0x04))));      /* LSR $04 with A 16-bit */
    if (fc) {
        lda16(rd16(ABSX(OBJ_VX))); eor16(0xFFFF); lda16(inc16(A)); wr16(ABSX(OBJ_VX), A);
        lda16(rd16(ABSX(OBJ_VY))); eor16(0xFFFF); lda16(inc16(A)); wr16(ABSX(OBJ_VY), A);
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
    lda16(rd16(ABSX(OBJ_VX)));
    if (!fz) {
        lda16(rd16(ABSX(OBJ_X)));
        fc = 1; sbc16(rd16(ABS(OBJ_TARGET_X)));
        eor16(rd16(ABSX(OBJ_VX)));
        if (!fn) {
            lda16(rd16(ABS(OBJ_TARGET_X))); wr16(ABSX(OBJ_X), A);
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
    }
    lda16(rd16(ABSX(OBJ_Y)));
    if (!fz) {
        lda16(rd16(ABSX(OBJ_Y)));
        fc = 1; sbc16(rd16(ABS(OBJ_TARGET_Y)));
        eor16(rd16(ABSX(OBJ_VY)));
        if (!fn) {
            lda16(rd16(ABS(OBJ_TARGET_Y))); wr16(ABSX(OBJ_Y), A);
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
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (fz) {
        lda8(rd8(ABSX(RIDER_AIR)));
        lda8(lsr8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        cmp8((u8)A, rd8(ABSY(0xB865)));
        if (!fc) { ret(); return; }
    }
    Y = rd8(DP(0x00)); nz8((u8)Y);
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    fc = 1; sbc8(rd8(ABSY(0xB85A)));
    wr8(ABSX(RIDER_SPEED_LO), (u8)A);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    if (!fz) { wr8(ABSX(RIDER_SPEED_HI), dec8(rd8(ABSX(RIDER_SPEED_HI)))); ret(); return; }
    wr8(ABSX(RIDER_SPEED_LO), (u8)A);
    ret();
}

void MirrorJumpRider_m1x1(void);

/* $80:EB39 (RTS, X = rider): on a slowing piece (on the ground, unless in
 * jump states 1/2/5...): $08BC = 1, braking 6 (5 holding B), and on a
 * $01 piece the player's $098F is capped at $1200. Y is kept. */
void SlowPieceBrake_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_JUMP)));
    if (!fz) {
        lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
        if (!fc) { ret(); return; }
    }
    wr8(DP(0x01), (u8)Y);
    ldy8(0x04);
    lda8(rd8(ABSX(RIDER_INPUT)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    if (fc) Y = dec8((u8)Y);
    lda8(0x01); wr8(ABSX(RIDER_ON_RAMP), 0x01);
    Y = inc8((u8)Y); Y = inc8((u8)Y);
    push16(0xEB5C); BrakeRider_m1x1();
    Y = rd8(ABSX(RIDER_TILE_Y)); nz8((u8)Y);
    lda8(rd8(ABSY(0xACF9)));
    cmp8((u8)A, 0x01);
    if (fz) {
        cmp8((u8)X, 0x00);
        if (fz) {
            rep(0x20);
            lda16(rd16(ABS(TURBO_HEAT_FRAC)));
            cmp16(A, 0x1200);
            if (fc) { lda16(0x1200); wr16(ABS(TURBO_HEAT_FRAC), 0x1200); }
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
        lda8(rd8(ABS(TURBO_ON)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        ora8(rd8(ABSX(RANK_ORDER)));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(RANK_ORDER)));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (!fz) {
            lda8(rd8(ABSY(0xBDA7))); wr8(DP(0x0B), (u8)A);
            lda8(rd8(ABSX(0x1959)));
            if (!fz) {
                cmp8((u8)A, 0x05);
                if (!fc) {
                    push8((u8)X);
                    wr8(DP(0x0B), (u8)X);
                    lda8(rd8(ABS(PLAYER_CHAR)));
                    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
                    fc = 0; adc8(rd8(DP(0x0B)));
                    X = (u8)A; nz8((u8)X);
                    and8(0x07);
                    int flash = fz;
                    if (flash) { lda8(rd8(ABS(TURBO_ON))); flash = !fz; }
                    if (flash) { lda8(rd8(DP(GAME_MODE))); cmp8((u8)A, 0x03); flash = fz; }
                    if (flash) { lda8(rd8(DP(FRAME_COUNT))); and8(0x02); flash = fz; }
                    if (flash) { lda8(rd8(ABS(FADE_STATE))); flash = fz; }
                    if (flash) {
                        lda8(rd8(ABS(PLAYER_CHAR)));
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
    lda8(rd8(ABS(FADE_STATE)));
    if (!fz) { retl(); return; }
    ldx8(0x07);
    ldy8(0x0E);
    lda8(rd8(ABS(RIDER_X))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABS(RIDER_PAGE))); wr8(DP(0x01), (u8)A);
    do {
        lda8(rd8(ABSX(RIDER_PAGE)));
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        lda8(rd8(ABSX(RIDER_X)));
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
    wr8(ABS(APUIO1), (u8)A);
    lda8((u8)X);
    lda8(lsr8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
    rep(0x20);
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    sep(0x20);
    wr8(ABS(APUIO3), (u8)A);
    retl();
}

void ScrollByPlayerSpeed_m1x1(void);

/* $80:DB44 (RTS): the bonus run: while it runs the screen follows the
 * player; once over ($84) the player rides off by its speed $0180, and
 * past X $F0 the run ends ($84 = $0198 = 2, sound $2D, the label); else
 * the speed is cleared. */
void BonusRunOut_m1x1(void)
{
    lda8(rd8(DP(RACE_END)));
    if (fz) { ScrollByPlayerSpeed_m1x1(); return; }
    lda8(rd8(ABS(RIDER_STEP)));
    fc = 0; adc8(rd8(ABS(RIDER_X)));
    wr8(ABS(RIDER_X), (u8)A);
    cmp8((u8)A, 0xF0);
    if (fc) {
        lda8(0x02); wr8(DP(RACE_END), 0x02); wr8(ABS(RIDER_PAGE), 0x02);
        lda8(0x2D);
        push8(0x80); push16(0xDB65); QueueSound_m1x1();
        push16(0xDB87); ShowBonusLabel_m1x1();
        ret();
        return;
    }
    lda8(0x00); wr8(ABS(RIDER_STEP), 0x00);
    ret();
}

void SegmentNext_m1x1(void);
void SpawnDustPuff_m1x1(void);

/* $80:915C: SegmentNext's tail: APU port 3 silenced, $75 / $77 cleared,
 * fade out ($1923 = $FF). */
static void segment_finish(void)
{
    wr8(ABS(APUIO3), 0);
    lda8(0x00);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    lda8(0xFF);
    wr8(ABS(FADE_STATE), 0xFF);
    ret();
}

/* $80:AB7F: a dust puff every 16 frames; with $60 odd, $00 = $58,
 * X = 7, Y = $1C. */
static void dust_tail(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x0F);
    if (fz) { push8(0x80); push16(0xAB88); SpawnDustPuff_m1x1(); }
    lda8(rd8(DP(OVERHEATED)));
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
        lda8(rd8(ABS(ATTRACT_DEMO)));
        if (fz) { ret(); return; }
        ldx8(0x2B);
        do {
            wr8(DPX(SLOW_TICK), 0);
            X = dec8((u8)X);
        } while (!fn);
        wr8(DP(GAME_MODE), 0);
        wr8(DP(TURN_LATCH), inc8(rd8(DP(TURN_LATCH))));
        segment_finish();
        return;
    }
    lda8(0x00);
    wr8(ABS(GAUGE_ON), 0); wr8(ABS(OVERHEAT_LOCK), 0); wr8(DP(OVERHEATED), 0);
    X = rd8(DP(RACE_END)); nz8((u8)X);
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
    lda8(rd8(DP(OVERHEATED)));
    ora8(rd8(ABS(OVERHEAT_LOCK)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(OVERHEATED)));
    cmp8((u8)A, 0x08);
    if (fz) {
        lda8(0x00); wr8(ABS(OVERHEAT_LOCK), 0x00);
        lda8(0x2D);
        push8(0x80); push16(0xC461); QueueSound_m1x1();
        lda8(0x05); wr8(ABS(ENGINE_HEAT), 0x05); wr8(ABS(RIDER_COUNTDOWN), 0x05);
        Y = rd8(ABS(RIDER_TIMER)); nz8((u8)Y);
        if (fz) wr8(ABS(RIDER_TIMER), (u8)A);
        ret();
        return;
    }
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(RIDER_SPEED_HI)));
    ora8(rd8(ABS(RIDER_BUMPED)));
    ora8(rd8(ABS(RIDER_TIMER)));
    if (fz) {
        lda8(rd8(ABS(RIDER_SCRIPT)));
        if (!fz) { lda8(0xC0); wr8(ABS(RIDER_SPEED_LO), 0xC0); }
        else {
            wr8(ABS(RIDER_SPEED_LO), (u8)A);
            ldx8(0x01);
            lda8(rd8(ABS(RIDER_LANE)));
            cmp8((u8)A, rd8(ABS(0xBDA2)));
            if (fz) X = dec8((u8)X);
            wr8(ABS(RIDER_STEER), (u8)X);
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
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    wr8(ABS(ATTRACT_DEMO), 0);
    lda8(rd8(ABS(BONUS_TRIES)));
    if (!fn) {
        fc = 1; sbc8(0x0A);
        wr8(ABS(BONUS_TRIES), (u8)A);
        if (!fn) {
            lda8(0x09); wr8(DP(GAME_MODE), 0x09);
            segment_finish();
            return;
        }
    }
    lda8(0x01); wr8(DP(MODE_TIMER), 0x01);
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x39);
    if (!fc) { ret(); return; }
    lda8(0x10); wr8(DP(GAME_MODE), 0x10);
    rep(0x20);
    wr16(ABS(UPLOAD_SRC), 0); wr16(ABS(MODE_TIMER), 0); wr16(ABS(TIMER_A), 0); wr16(ABS(SCROLL_HALF), 0);
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
            lda8(rd8(ABSY(START_POLES)));
            fc = 1;
            sbc8(rd8(ABS(BG_SCROLL)));
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
                wr8(ABSX(OAM_BUF + 0x1A1), (u8)A);
                fc = 0; adc8(0x08);
                wr8(ABSX(OAM_BUF + 0x1A5), (u8)A);
                lda8(rd8(DP(0x00)));
                wr8(ABSX(OAM_BUF + 0x1A2), (u8)A);
                ora8(0x10);
                wr8(ABSX(OAM_BUF + 0x1A6), (u8)A);
                lda8(0x31);
                wr8(ABSX(OAM_BUF + 0x1A3), 0x31); wr8(ABSX(OAM_BUF + 0x1A7), 0x31);
                lda8(rd8(DP(0x01)));
                wr8(ABSX(OAM_BUF + 0x1A0), (u8)A); wr8(ABSX(OAM_BUF + 0x1A4), (u8)A);
                lda8((u8)X);
                lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(DP(0x08)));
                wr8(ABSX(OAM_SIZE_BUF + 0x68), (u8)A);
            }
        }
        Y = dec8((u8)Y);
    } while (!fn);
    lda8(rd8(DP(0x54)));
    if (!fz) { lda8(0xB5); wr8(DP(START_STATE), 0xB5); return; }
    lda8(rd8(DP(START_STATE)));
    cmp8((u8)A, 0xB4);
    if (!fz) { cmp8((u8)A, 0x78); if (!fz) cmp8((u8)A, 0x3C); }
    if (fz) {
        lda8(0x26);
        push8(0x80); push16(0xC5A1); QueueSound_m1x1();
        wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
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
    lda8(rd8(DP(START_STATE)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(rd8(DP(RACE_STARTED)));
        if (fz) {
            wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
            lda8(0x40); wr8(DP(TIMER_A), 0x40);
            lda8(0x27);
            push8(0x80); push16(0xC5BA); QueueSound_m1x1();
            lda8(rd8(ABS(TURBO_ON)));
            if (!fz) {
                lda8(0x2D);
                push8(0x80); push16(0xC5C5); QueueSound_m1x1();
                lda8(0x01);
                push8(0x80); push16(0xC5CB); QueueSound_m1x1();
            }
        }
        lda8(0x01); wr8(DP(RACE_STARTED), 0x01);
        lda8(0x02); wr8(DP(START_STATE), 0x02);
    } else {
        Y = rd8(DP(RACE_STARTED)); nz8((u8)Y);
        if (!fz) { ret(); return; }
        Y = rd8(DP(FRAME_COUNT)); nz8((u8)Y);
        and8(0x0F);
        if (fz) { ldy8(0x01); wr8(ABS(APUIO2), 0x01); }
    }
    ldy8(0xE1);
    lda8(rd8(DP(START_STATE)));
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
        lda16(rd16(LONG(SAVE_BEST_SCORE + i)));
        and16(0x00FF);
        wr16(DP(0x00), A);
        lda16(mul[i]); wr16(DP(0x02), mul[i]);
        sep(0x20);
        push8(0x80); push16(back[i][0]); Mul16_m1x1();
        push16(back[i][1]); Add32_m1x1();
    }
    lda8(rd8(LONG(SAVE_BEST_SCORE + 0x04))); wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA184); Add32_m1x1();
    lda8(rd8(DP(0x11)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(ENTRY_CODE), (u8)A);
    lda8(rd8(DP(0x11))); and8(0x0F); wr8(ABS(ENTRY_CODE + 0x01), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x02), (u8)A);
    lda8(rd8(DP(0x10))); and8(0x0F); wr8(ABS(ENTRY_CODE + 0x03), (u8)A);
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
    wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x09), (u8)A);
    lda8((u8)X);
    cmp8((u8)A, 0x00);
    if (fz) lda8(0x0C);
    wr8(ABS(DIGIT_ROW), (u8)A);
    ora8(0x10); wr8(ABS(DIGIT_ROW + 0x08), (u8)A);
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
    lda8(0x0A); wr8(ABS(DIGIT_ROW + 0x02), 0x0A);
    lda8(inc8((u8)A)); wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
    lda8(0x1A); wr8(ABS(DIGIT_ROW + 0x0A), 0x1A);
    lda8(inc8((u8)A)); wr8(ABS(DIGIT_ROW + 0x0D), (u8)A);
    lda8(0x28); wr8(ABS(DIGIT_BUF), 0x28);
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
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    lda8(rd8(ABSX(OBJ_TYPE)));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    rep(0x20);
    lda16(rd16(ABSY(0xAB0D))); wr16(DP(0x00), A);
    sep(0x20);
    lda8(rd8(ABSX(OBJ_FRAME)));
    lda8(inc8((u8)A));
    wr8(ABS(WRMPYA), (u8)A);
    lda8(rd8(ABSX(OBJ_SCRIPT + 0x01)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(ABS(WRMPYB), (u8)A);
    rep(0x30);
    lda16(rd16(ABS(RDMPYL)));
    fc = 1; sbc16(0x0004);
    fc = 0; adc16(rd16(DP(0x00)));
    Y = A; nz16(Y);
    lda16(rd16(ABSX(OBJ_SCRIPT + 0x01))); wr16(DP(0x00), A);
    lda16(rd16(ABSX(OBJ_X))); wr16(DP(0x04), A);
    lda16(rd16(ABSX(OBJ_SCRIPT))); wr16(DP(0x0E), A);
    lda16(rd16(ABSX(OBJ_Y))); wr16(DP(0x06), A);
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
            wr8(ABSX(OAM_BUF), (u8)A);
            lda8(rd8(ABSY(0xAB22)));
            fc = 0; adc8(rd8(DP(0x06)));
            wr8(ABSX(OAM_BUF + 0x01), (u8)A);
            lda8(rd8(ABSY(0xAB23)));
            wr8(ABSX(OAM_BUF + 0x02), (u8)A);
            lda8(rd8(ABSY(0xAB24)));
            and8(0xCF);
            ora8(rd8(DP(0x0E)));
            wr8(ABSX(OAM_BUF + 0x03), (u8)A);
            rep(0x20);
            push16(X);
            lda16(X);
            lda16(lsr16(A)); lda16(lsr16(A));
            X = A; nz16(X);
            sep(0x20);
            lda8(rd8(ABSY(0xAB24)));
            and8(0x10);
            lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
            wr8(ABSX(OAM_SIZE_BUF), (u8)A);
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
        wr8(ABS(RIDER_IN_MIDDLE), 0x07);
        lda8(0x00); wr8(ABSX(RIDER_PAGE), 0x00);
        ldy8(0x03);
        PlaceRival_m1x1();
        return 0;
    case 0xE3D4:
        lda8(0x10); wr8(ABS(0x099D), 0x10); wr8(ABS(ENGINE_HEAT), 0x10);
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
        X = rd8(ABS(DEMO_INDEX)); nz8((u8)X);
        wr8(ABS(DEMO_INDEX), inc8(rd8(ABS(DEMO_INDEX))));
        lda8(rd8(ABSX(0xE0DD)));
        cmp8((u8)A, 0xF8);
        if (!fc) {
            wr8(ABS(DEMO_TIME), (u8)A);
            lda8(rd8(ABSX(0xE1A5)));
            wr8(ABS(RIDER_INPUT), (u8)A);
            wr8(ABS(DEMO_PAD), (u8)A);
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
    lda8(rd8(LONG(DEBUG_ENABLED)));
    if (!fz) {
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x10);
        if (!fz) {
            ldy8(0x00);
            if (demo_command1()) demo_next1();
            return;
        }
    }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) {
        wr8(DP(FRAME_COUNT), 0);
        wr8(ABS(DEMO_TIME), 0);
        lda8(rd8(LONG(STADIUM_SELECT)));
        if (!fz) lda8(0x68);
        wr8(ABS(DEMO_INDEX), (u8)A);
        fc = 0;
        ret();
        return;
    }
    lda8(rd8(ABS(DEMO_TIME)));
    if (!fz) {
        wr8(ABS(DEMO_TIME), dec8(rd8(ABS(DEMO_TIME))));
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
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, 0x05);
    if (!fc) { ret(); return; }
    if (fz) {
        lda8(rd8(LONG(BCAST_FRAME_HI)));
        cmp8((u8)A, 0x07);
        if (!fc) { ret(); return; }
    }
    wr8(DP(TITLE_BLINK), inc8(rd8(DP(TITLE_BLINK))));
    lda8(rd8(DP(TITLE_BLINK)));
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
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x10);
    if (fz) { ret(); return; }
    lda8(0x2D); wr8(ABS(APUIO0), 0x2D);
    ldy8(0x00);
    if (demo_command1()) demo_next1();
}

/* $81:9714 UpdateObjects (RTL): Stadium 4's $81:97E8 with Stadium 1's
 * object arrays (slot $039D; steps $81:AAE5, draws $81:AAF9). */
void UpdateObjects_m1x1(void)
{
    wr8(ABS(SPRITES_FROZEN), 0);
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    ldx8(0x0E);
    do {
        wr8(ABS(OBJ_CURRENT), (u8)X);
        lda8(rd8(ABSX(OBJ_ACTIVE)));
        if (!fz) {
            lda8(rd8(ABSX(OBJ_DELAY)));
            if (!fz) wr8(ABSX(OBJ_DELAY), dec8(rd8(ABSX(OBJ_DELAY))));
            lda8(rd8(ABSX(OBJ_COUNTER)));
            if (!fz) wr8(ABSX(OBJ_COUNTER), dec8(rd8(ABSX(OBJ_COUNTER))));
            lda8(rd8(ABSX(OBJ_TYPE)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            push8((u8)X);
            push16(0x973E);
            r816_call_pc(0x810000u | rd16(LONGX(0x81AAE5)));
            X = pull8(); nz8((u8)X);
            Y = rd8(ABS(OBJ_CURRENT)); nz8((u8)Y);
            lda8(rd8(ABSY(OBJ_ACTIVE)));
            if (!fz) {
                push16(0x974A);
                r816_call_pc(0x810000u | rd16(LONGX(0x81AAF9)));
            }
        }
        X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
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
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(rd8(ABS(ENTRY_CURSOR)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    fc = 0; adc8(rd8(ABS(ENTRY_CURSOR)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(COURSE_BEST))); wr8(ABS(RACE_MIN), (u8)A);
    lda8(rd8(ABSY(COURSE_BEST + 0x01))); wr8(ABS(RACE_SEC), (u8)A);
    lda8(rd8(ABSY(COURSE_BEST + 0x02))); wr8(ABS(RACE_FRAMES), (u8)A);
    rep(0x20);
    lda16(rd16(ABSX(0x9A48))); wr16(DP(0x00), A);
    lda16(0x03B1); wr16(DP(0x18), 0x03B1);
    sep(0x20);
    lda8(0x00); wr8(DP(0x1A), 0x00);
    push8(0x80); push16(0x9A20); QueueTimeDigits_m1x1();
    lda8(0x42);
    push8(0x80); push16(0x9A26); QueueSound_m1x1();
    wr8(ABS(ENTRY_CURSOR), inc8(rd8(ABS(ENTRY_CURSOR))));
    lda8(rd8(ABS(ENTRY_CURSOR)));
    cmp8((u8)A, 0x05);
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(NEW_TIME_RECORD)));
    if (fz) {
        lda8(0x04); wr8(ABS(END_STEP), 0x04);
        lda8(0x01); wr8(ABS(END_NEXT_STEP), 0x01);
    } else {
        lda8(0x02); wr8(ABS(END_STEP), 0x02);
    }
    ret();
}

/* $80:9A87 (RTS): the record entry: after the $88 delay the box
 * ($89:D800); then A: step $0453 = 1, $88 = $20, the next box
 * ($89:D980) and the record time ($11:572C) at VRAM $09D3, sound $43. */
void EndScreenRecord_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        u8 t = dec8(rd8(DP(MODE_TIMER)));
        wr8(DP(MODE_TIMER), t);
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
    lda8(rd8(DP(P1_PRESS_LO)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(0x01); wr8(ABS(END_STEP), 0x01);
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
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
    lda16(rd16(DP(SCROLL_X)));
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
    lda16(rd16(DP(MODE_TIMER)));
    if (!fz) {
        wr16(DP(MODE_TIMER), dec16(rd16(DP(MODE_TIMER))));
        lda16(rd16(DP(MODE_TIMER)));
        int beep;
        cmp16(A, 0x00B4); beep = fz;
        if (!beep) { cmp16(A, 0x0078); beep = fz; }
        if (!beep) { cmp16(A, 0x003C); beep = fz; }
        if (beep) {
            sep(0x20);
            lda8(0x26);
            push8(0x80); push16(0xD9ED); QueueSound_m1x1();
            wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
            ret();
            return;
        }
        cmp16(A, 0x0000);
        if (fz) {
            lda16(0x0040); wr16(DP(TIMER_A), 0x0040);
            sep(0x20);
            lda8(0x27);
            push8(0x80); push16(0xD9E0); QueueSound_m1x1();
            wr8(DP(COUNTDOWN), inc8(rd8(DP(COUNTDOWN))));
        }
        sep(0x20);
        ret();
        return;
    }
    sep(0x20);
    wr8(ABS(SPRITES_FROZEN), 0);
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        lda8(lsr8((u8)A));
        if (!fc) {
            lda8(0x02); wr8(DP(MODE_HOLD), 0x02);
            lda8(0x00); wr8(DP(SUB_STEP), 0x00); wr8(DP(VSCROLL_ZOOM), 0x00);
            SegmentNext_m1x1();
            return;
        }
    }
    lda8(rd8(DP(P1_HELD_HI)));
    wr8(ABS(RIDER_INPUT), (u8)A);
    lda8(0x00); wr8(DP(SUB_STEP), 0x00);
    do {
        push16(0xDA0C); DispatchBy85_m1x1();
        wr8(DP(SUB_STEP), inc8(rd8(DP(SUB_STEP))));
        lda8(rd8(DP(SUB_STEP)));
        cmp8((u8)A, 0x13);
    } while (!fc);
    ldx8(0x09);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(0xF0); wr8(ABSY(OAM_BUF + 0xC1), 0xF0);
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
    u8 t = dec8(rd8(DP(MODE_TIMER)));
    wr8(DP(MODE_TIMER), t);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(BONUS_RESULT)));
    cmp8((u8)A, rd8(ABS(BONUS_BEST)));
    if (fc) wr8(ABS(BONUS_BEST), (u8)A);
    push16(0xDF15); ShowBonusBest_m1x1();
    lda8(rd8(ABS(BONUS_TRIES)));
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
        wr8(ABS(DIGIT_ROW), 0x50);
        wr8(ABS(DIGIT_ROW), inc8(rd8(ABS(DIGIT_ROW))));
        for (u16 a = 0x03D4; a <= 0x03D7; a++) wr8(ABS(a), 0x50);
        lda8(0x60);
        wr8(ABS(DIGIT_ROW + 0x05), 0x60);
        wr8(ABS(DIGIT_ROW + 0x05), inc8(rd8(ABS(DIGIT_ROW + 0x05))));
        for (u16 a = 0x03D9; a <= 0x03DC; a++) wr8(ABS(a), 0x60);
        lda8(rd8(ABS(BONUS_BEST)));
        cmp8((u8)A, rd8(ABS(0x09C9)));
        if (!fz) {
            lda8(0xFC);
            wr8(ABS(DIGIT_ROW), 0xFC); wr8(ABS(DIGIT_ROW + 0x05), 0xFC);
            ldy8(0x00);
            lda8(rd8(ABS(BONUS_BEST)));
            for (;;) {
                cmp8((u8)A, 0x0A);
                if (!fc) break;
                sbc8(0x0A);
                Y = inc8((u8)Y);
                WAIT_TICK();
            }
            fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
            fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x07), (u8)A);
            lda8((u8)Y);
            fc = 0; adc8(0x50); wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
            fc = 0; adc8(0x10); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
            lda8(rd8(ABS(DIGIT_ROW + 0x01)));
            cmp8((u8)A, 0x50);
            if (fz) {
                lda8(0xFC);
                wr8(ABS(DIGIT_ROW + 0x01), 0xFC); wr8(ABS(DIGIT_ROW + 0x06), 0xFC);
                lda8(rd8(ABS(DIGIT_ROW + 0x02)));
                cmp8((u8)A, 0x50);
                if (fz) {
                    lda8(0xFC);
                    wr8(ABS(DIGIT_ROW + 0x02), 0xFC); wr8(ABS(DIGIT_ROW + 0x03), 0xFC);
                    wr8(ABS(DIGIT_ROW + 0x07), 0xFC); wr8(ABS(DIGIT_ROW + 0x08), 0xFC);
                }
            }
        }
        lda8(0x25); wr8(ABS(DIGIT_BUF), 0x25);
        push16(0xDFCB); VramQueueTiles20_m1x1();
        wr8(DP(0x00), 0);
        lda8(rd8(ABS(BONUS_BEST)));
        cmp8((u8)A, rd8(ABS(0x09C9)));
        if (fz) {
            lda8(0x01); wr8(ABS(SCORE_ADD + 0x01), 0x01);
        } else {
            for (;;) {
                cmp8((u8)A, 0x0A);
                if (!fc) break;
                sbc8(0x0A);
                wr8(DP(0x00), inc8(rd8(DP(0x00))));
                WAIT_TICK();
            }
            wr8(ABS(SCORE_ADD + 0x03), (u8)A);
            lda8(rd8(DP(0x00)));
            wr8(ABS(SCORE_ADD + 0x02), (u8)A);
        }
        push16(0xDFF1); AddScore_m1x1();
    }
    lda8(0xFF); wr8(DP(MODE_TIMER), 0xFF);
    wr8(DP(MODE_STEP), inc8(rd8(DP(MODE_STEP))));
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
    lda8(rd8(DP(RACE_END)));
    ora8(rd8(ABS(TURBO_ON)));
    u8 top;
    if (!fz) goto off;
    lda8(rd8(ABS(OVERHEAT_LOCK)));
    if (!fz) {
        lda8(rd8(DP(OVERHEATED)));
        lda8(lsr8((u8)A));
        if (fc) goto off;
        top = 0x01;
        goto draw;
    }
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0xC0);
    if (!fz) {
        Y = rd8(DP(RACE_STARTED)); nz8((u8)Y);
        if (fz) lda8(0x80);
        lda8(asl8((u8)A)); lda8(rol8((u8)A)); lda8(rol8((u8)A));
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(ENGINE_HEAT)));
    cmp8((u8)A, rd8(ABSY(0xBE03)));
    if (!fc) {
        lda8(rd8(ABS(TURBO_HEAT_FRAC)));
        fc = 0; adc8(rd8(ABSY(0xBDFF)));
        wr8(ABS(TURBO_HEAT_FRAC), (u8)A);
        if (fc) wr8(ABS(ENGINE_HEAT), inc8(rd8(ABS(ENGINE_HEAT))));
    } else if (!fz) {
        lda8(rd8(ABS(TURBO_HEAT_FRAC)));
        fc = 1; sbc8(0x0B);
        wr8(ABS(TURBO_HEAT_FRAC), (u8)A);
        if (!fc) {
            lda8(rd8(ABS(ENGINE_HEAT)));
            if (!fz) wr8(ABS(ENGINE_HEAT), dec8(rd8(ABS(ENGINE_HEAT))));
        }
    }
    ldx8(0x00);
    lda8(rd8(ABS(ENGINE_HEAT)));
    cmp8((u8)A, 0x20);
    if (fc) {
        wr8(DP(OVERHEATED), (u8)A);
        wr8(ABS(OVERHEAT_LOCK), (u8)A);
        push8((u8)A);
        lda8(0x2E); push8(0x80); push16(0xCF7D); QueueSound_m1x1();
        lda8(0x3E); push8(0x80); push16(0xCF83); QueueSound_m1x1();
        lda8(pull8());
    }
    cmp8((u8)A, 0x1A);
    int warn = 0;
    if (fc) { Y = dec8((u8)Y); warn = fz; }
    if (warn) {
        lda8(rd8(DP(TURBO_WARN)));
        if (fz) {
            lda8(rd8(ABS(TURBO_ON)));
            if (fz) { lda8(0x2C); push8(0x80); push16(0xCF9A); QueueSound_m1x1(); }
            lda8(0x01); wr8(DP(TURBO_WARN), 0x01);
        }
    } else {
        lda8(rd8(DP(TURBO_WARN)));
        if (!fz) {
            lda8(0x2D); push8(0x80); push16(0xCFAA); QueueSound_m1x1();
            wr8(DP(TURBO_WARN), 0);
        }
    }
off:
    top = 0xFF;
draw:
    lda8(top);
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABS(ENGINE_HEAT)));
    fc = 0; adc8(0x58);
    X = rd8(ABS(TURBO_ON)); nz8((u8)X);
    if (!fz) lda8(0x78);
    wr8(DP(0x00), (u8)A);
    ldx8(0x0C);
    lda8(rd8(ABS(TURBO_ON)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    do {
        lda8(rd8(DP(0x00)));
        wr8(ABSX(OAM_BUF + 0x1F0), (u8)A);
        fc = 0; adc8(0x08);
        wr8(DP(0x00), (u8)A);
        cmp8((u8)A, 0x70);
        lda8(fc ? 0xC7 : 0xF0);
        wr8(ABSX(OAM_BUF + 0x1F1), (u8)A);
        lda8(rd8(ABSY(0xCFF2)));
        wr8(ABSX(OAM_BUF + 0x1F2), (u8)A);
        lda8(0x21); wr8(ABSX(OAM_BUF + 0x1F3), 0x21);
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
    X = rd8(ABS(ENTRY_CURSOR)); nz8((u8)X);
    ldy8(0x01);
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x0C);
    if (!fz) {
        lda8(0x33);
        push8(0x80); push16(0x9C1D); QueueSound_m1x1();
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x08);
        if (!fz) {
            lda8(rd8(ABSX(ENTRY_DIGITS)));
            lda8(inc8((u8)A));
            wr8(ABSX(ENTRY_DIGITS), (u8)A);
            cmp8((u8)A, 0x0A);
            if (fc) wr8(ABSX(ENTRY_DIGITS), 0);
        } else {
            ldy8(0x09);
            lda8(rd8(ABSX(ENTRY_DIGITS)));
            lda8(dec8((u8)A));
            wr8(ABSX(ENTRY_DIGITS), (u8)A);
            if (fn) { lda8(0x09); wr8(ABSX(ENTRY_DIGITS), 0x09); }
        }
    }
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x03);
    if (!fz) {
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x9D03)));
        fc = 0; adc8(rd8(ABS(ENTRY_CURSOR)));
        and8(0x03);
        wr8(ABS(ENTRY_CURSOR), (u8)A);
    }
    lda8(rd8(ABS(ENTRY_DIGITS)));
    ora8(rd8(ABS(ENTRY_DIGITS + 0x01)));
    ora8(rd8(ABS(ENTRY_DIGITS + 0x02)));
    ora8(rd8(ABS(ENTRY_DIGITS + 0x03)));
    if (fz) {
        cmp8((u8)X, 0x03);
        if (!fz) ldy8(0x01);
        wr8(ABS(ENTRY_DIGITS + 0x03), (u8)Y);
    }
    rep(0x20);
    lda16(rd16(ABS(ENTRY_DIGITS))); ora16(0x1010); wr16(ABS(0x03AA), A);
    lda16(rd16(ABS(ENTRY_DIGITS + 0x02))); ora16(0x1010); wr16(ABS(0x03AC), A);
    lda16(0x0B0E); wr16(DP(0x00), 0x0B0E);
    lda16(0x03A5); wr16(DP(0x02), 0x03A5);
    sep(0x20);
    lda8(0x24); wr8(ABS(0x03A5), 0x24);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push16(0x9C99); VramQueueTiles_m1x1();
    lda8(rd8(ABS(ENTRY_CURSOR)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    fc = 0; adc8(0x70);
    wr8(ABS(OAM_BUF), (u8)A); wr8(ABS(OAM_BUF + 0x04), (u8)A);
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x18);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(0x8E);
    fc = 0; adc8(rd8(DP(0x00)));
    wr8(ABS(OAM_BUF + 0x01), (u8)A);
    lda8(0x78);
    fc = 1; sbc8(rd8(DP(0x00)));
    wr8(ABS(OAM_BUF + 0x05), (u8)A);
    lda8(0xFE); wr8(ABS(OAM_BUF + 0x02), 0xFE); wr8(ABS(OAM_BUF + 0x06), 0xFE);
    lda8(0x21); wr8(ABS(OAM_BUF + 0x03), 0x21);
    lda8(0xA1); wr8(ABS(OAM_BUF + 0x07), 0xA1);
    wr8(ABS(OAM_SIZE_BUF), 0); wr8(ABS(OAM_SIZE_BUF + 0x01), 0);
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x10);
    if (fz) { ret(); return; }
    wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
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
        lda8(rd8(ABSX(RIDER_SCRIPT)));
        cmp8((u8)A, 0x15);
        int move = !fz;
        if (move) { cmp8((u8)A, 0x14); move = !fz; }
        if (move) {
            lda8(rd8(ABSX(RIDER_LANE)));
            fc = 0; adc8(rd8(ABSX(RIDER_STEER)));
            wr8(ABSX(RIDER_LANE), (u8)A);
        }
        ldy8(0x07);
        do {
            lda8(rd8(ABSX(RIDER_LANE)));
            cmp8((u8)A, rd8(ABSY(0xE6CD)));
            if (fz) { lda8(0x00); wr8(ABSX(RIDER_STEER), 0x00); goto next; }
            Y = dec8((u8)Y);
        } while (!fn);
        lda8(rd8(ABSX(RIDER_JUMP)));
        if (!fz) {
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(rd8(ABSX(RIDER_LANE)));
                cmp8((u8)A, 0x20);
                if (fc) goto rows;
                lda8(0x04); wr8(ABSX(RIDER_JUMP), 0x04);
                lda8(rd8(ABSX(RIDER_SCRIPT)));
                int land = fz;
                if (!land) { cmp8((u8)A, 0x12); land = fz; }
                if (!land) { cmp8((u8)A, 0x10); land = fz; }
                if (!land) { cmp8((u8)A, 0x0D); land = fz; }
                if (!land) goto next;
                lda8(0x00); wr8(ABSX(RIDER_HEIGHT), 0x00);
                push16(0xEC33); CrashLand_m1x1();
                wr8(ABSX(RIDER_JUMP), dec8(rd8(ABSX(RIDER_JUMP))));
                if (!fz) goto next;
            }
            cmp8((u8)A, 0x03);
            if (!fz) {
                lda8(rd8(ABSX(RIDER_LANE)));
                cmp8((u8)A, 0x20);
                if (fc) wr8(ABSX(RIDER_LANE), dec8(rd8(ABSX(RIDER_LANE))));
            }
        }
    rows:
        lda8(rd8(ABSX(RIDER_LANE)));
        cmp8((u8)A, 0x08);
        if (!fc) {
            lda8(rd8(ABSX(RIDER_TIMER)));
            if (fz) lda8(0x07);
            else {
                lda8(rd8(ABSX(RIDER_LANE)));
                cmp8((u8)A, 0x02);
                if (!fc) lda8(0x01);
            }
            wr8(ABSX(RIDER_LANE), (u8)A);
            cmp8((u8)X, 0x00);
            if (fz) goto flip;
            goto edge;
        }
        cmp8((u8)A, 0x3A);
        if (!fc) goto next;
        lda8(0x39); wr8(ABSX(RIDER_LANE), 0x39);
    edge:
        lda8(rd8(ABSX(RIDER_TIMER)));
        ora8(rd8(ABSX(OVERHEAT_LOCK)));
        if (!fz) goto next;
    flip:
        lda8(rd8(ABSX(RIDER_STEER)));
        if (fz) lda8(0xFF);
        eor8(0xFE);
        wr8(ABSX(RIDER_STEER), (u8)A);
    next:
        X = dec8((u8)X);
        if (fn) break;
    }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_BUMPED)));
    ora8(rd8(ABS(OVERHEAT_LOCK)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_AIR)));
    if (!fz) {
        lda8(rd8(ABS(RIDER_SLOW)));
        cmp8((u8)A, 0x02);
        if (!fz) { ret(); return; }
        wr8(ABS(RIDER_SLOW), inc8(rd8(ABS(RIDER_SLOW))));
    }
    lda8(rd8(ABS(RIDER_TIMER)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) { ret(); return; }
    }
    Y = rd8(ABS(RIDER_STEER)); nz8((u8)Y);
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0x04);
    if (!fz) lda8(0xFF);
    else {
        lda8(rd8(ABS(RIDER_INPUT)));
        and8(0x08);
        if (fz) { ret(); return; }
        lda8(0x01);
    }
    wr8(ABS(RIDER_STEER), (u8)A);
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x0C);
    if (fz) {
        lda8(rd8(DP(FRAME_COUNT)));
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
        lda8(rd8(ABSX(SELECT_WHEELS)));
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
            wr8(ABSY(OAM_BUF + 4 * c), (u8)A);
            wr8(ABSY(OAM_BUF + 0x0C + 4 * c), (u8)A);
            wr8(ABSY(OAM_BUF + 0x18 + 4 * c), (u8)A);
        }
        lda8(rd8(DP(0x00)));
        for (int r = 0; r < 3; r++) {
            if (r) { fc = 0; adc8(0x10); }
            wr8(ABSY(OAM_BUF + 0x01 + 12 * r), (u8)A);
            wr8(ABSY(OAM_BUF + 0x05 + 12 * r), (u8)A);
            wr8(ABSY(OAM_BUF + 0x09 + 12 * r), (u8)A);
        }
        lda8(rd8(LONGX(0x809043)));
        for (int r = 0; r < 3; r++) {
            if (r) { fc = 0; adc8(0x1C); }
            for (int c = 0; c < 3; c++) {
                if (c) { lda8(inc8((u8)A)); lda8(inc8((u8)A)); }
                wr8(ABSY(OAM_BUF + 0x02 + 12 * r + 4 * c), (u8)A);
            }
        }
        lda8(rd8(DP(P1_HELD_HI)));
        and8(0x03);
        int busy = !fz;
        if (!busy) {
            cmp8((u8)X, 0x04);
            if (fz) { lda8(rd8(ABS(SELECT_WHEELS + 0x01))); busy = !fz; }
            else { lda8(rd8(ABSX(SELECT_WHEELS))); busy = !fz; }
        }
        if (busy) lda8(0x28);
        else {
            push8(0x80); push16(0x8FEE); SpriteTextRow_m1x1();
            lda8(rd8(LONGX(0x809048)));
        }
        ora8(rd8(LONGX(0x80904D)));
        for (int i = 0; i < 9; i++) wr8(ABSY(OAM_BUF + 0x03 + 4 * i), (u8)A);
        lda8((u8)Y);
        lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(0x02);
        for (int i = 0; i < 9; i++) wr8(ABSY(OAM_SIZE_BUF + i), 0x02);
        X = pull8(); nz8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    retl();
}

/* $80:E411 DemoPadDisplay (RTS): Stadium 4's $80:F3DD with Stadium 1's
 * tables ($80:E4E3 / $80:E4F3 / $80:E4F8), then DemoPushStart. */
void DemoPadDisplay_m1x1(void)
{
    lda8(rd8(ABS(DEMO_PAD)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xE4E3)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xE4F3)));
    wr8(DP(0x00), (u8)A);
    ldy8(0x80);
    lda8(0x80);
    wr8(ABSY(OAM_BUF), (u8)A); wr8(ABSY(OAM_BUF + 0x08), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x04), (u8)A); wr8(ABSY(OAM_BUF + 0x0C), (u8)A);
    lda8(0x18);
    wr8(ABSY(OAM_BUF + 0x01), (u8)A); wr8(ABSY(OAM_BUF + 0x05), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x09), (u8)A); wr8(ABSY(OAM_BUF + 0x0D), (u8)A);
    lda8(rd8(DP(0x00)));
    wr8(ABSY(OAM_BUF + 0x02), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A));
    wr8(ABSY(OAM_BUF + 0x06), (u8)A);
    fc = 0; adc8(0x1E);
    wr8(ABSY(OAM_BUF + 0x0A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A));
    wr8(ABSY(OAM_BUF + 0x0E), (u8)A);
    lda8(0x26);
    wr8(ABSY(OAM_BUF + 0x03), 0x26); wr8(ABSY(OAM_BUF + 0x07), 0x26);
    wr8(ABSY(OAM_BUF + 0x0B), 0x26); wr8(ABSY(OAM_BUF + 0x0F), 0x26);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(OAM_SIZE_BUF), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x01), 0x02);
    wr8(ABSY(OAM_SIZE_BUF + 0x02), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x03), 0x02);
    lda8(rd8(ABS(DEMO_PAD)));
    lda8(rol8((u8)A)); lda8(rol8((u8)A)); lda8(rol8((u8)A));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xE4F8)));
    wr8(DP(0x00), (u8)A);
    ldy8(0x70);
    lda8(0xB0);
    wr8(ABSY(OAM_BUF), (u8)A); wr8(ABSY(OAM_BUF + 0x08), (u8)A);
    fc = 0; adc8(0x18);
    wr8(ABSY(OAM_BUF + 0x04), (u8)A); wr8(ABSY(OAM_BUF + 0x0C), (u8)A);
    lda8(0x18);
    wr8(ABSY(OAM_BUF + 0x01), (u8)A); wr8(ABSY(OAM_BUF + 0x05), (u8)A);
    fc = 0; adc8(0x10);
    wr8(ABSY(OAM_BUF + 0x09), (u8)A); wr8(ABSY(OAM_BUF + 0x0D), (u8)A);
    lda8(rd8(DP(0x00)));
    wr8(ABSY(OAM_BUF + 0x02), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x06), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x0A), (u8)A);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABSY(OAM_BUF + 0x0E), (u8)A);
    lda8(0x28);
    wr8(ABSY(OAM_BUF + 0x03), 0x28); wr8(ABSY(OAM_BUF + 0x07), 0x28);
    wr8(ABSY(OAM_BUF + 0x0B), 0x28); wr8(ABSY(OAM_BUF + 0x0F), 0x28);
    lda8((u8)Y);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABSY(OAM_SIZE_BUF), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x01), 0x02);
    wr8(ABSY(OAM_SIZE_BUF + 0x02), 0x02); wr8(ABSY(OAM_SIZE_BUF + 0x03), 0x02);
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
    lda8(rd8(DP(COIN_TAKEN)));
    if (fz) { X = pull8(); nz8((u8)X); retl(); return; }
    lda8(0x01); wr8(DP(0x10), 0x01);
    wr8(DP(0x11), 0);
    lda8(rd8(DP(COIN_TAKEN)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xA791))); wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(ABS(RIDER_COLUMN)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
    Y = rd8(ABS(COIN_TAKEN)); nz8((u8)Y);
    cmp8((u8)A, rd8(ABSY(0xA78C)));
    if (!fz) wr8(DP(0x10), inc8(rd8(DP(0x10))));
    for (int pass = 0; pass < 2; pass++) {
        if (pass) {
            wr8(DP(0x10), dec8(rd8(DP(0x10))));
            lda8(rd8(DP(COIN_TAKEN)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
        }
        rep(0x20);
        lda16(rd16(ABS(RIDER_COLUMN)));
        fc = 1; sbc16(rd16(DP(0x10)));
        and16(0x0020);
        for (int i = 0; i < 5; i++) lda16(asl16(A));
        wr16(DP(0x00), A);
        lda16(rd16(ABS(RIDER_COLUMN)));
        fc = 1; sbc16(rd16(DP(0x10)));
        and16(0x001F);
        fc = 0; adc16(rd16(DP(0x00)));
        fc = 0; adc16(rd16(ABSX(0xA7A5)));
        wr16(DP(0x00), A);
        lda16(0x0018); wr16(DP(0x02), 0x0018);
        sep(0x20);
        X = rd8(DP(COIN_TAKEN)); nz8((u8)X);
        lda8(0x21); wr8(DP(0x18), 0x21);
        lda8(rd8(ABSX(0xA79B))); wr8(DP(0x19), (u8)A);
        lda8(rd8(ABSX(0xA7A0))); wr8(DP(0x1A), (u8)A);
        lda8(0x08); wr8(DP(0x0F), 0x08);
        push16(pass ? 0xA765 : 0xA71D); VramQueueTiles_m1x1();
    }
    wr8(DP(COIN_TAKEN), 0);
    wr8(ABS(COIN_COUNT), inc8(rd8(ABS(COIN_COUNT))));
    push8(0x80); push16(0xA76E); CoinCollected_m1x1();
    lda8(rd8(ABS(COIN_COUNT)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x2D);
        push8(0x80); push16(0xA77B); QueueSound_m1x1();
        lda8(0x01); wr8(ABS(TURBO_ON), 0x01);
        push8(0x80); push16(0xA784); QueueSound_m1x1();
        lda8(0x20); wr8(ABS(ENGINE_HEAT), 0x20);
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
    lda8(rd8(DP(CLOCK_MINUTE)));
    if (fz) { X = pull8(); nz8((u8)X); retl(); return; }
    wr8(DP(0x10), 0); wr8(DP(0x11), 0);
    lda8(rd8(DP(CLOCK_MINUTE)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xA874))); wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(ABS(RIDER_COLUMN)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
    Y = rd8(ABS(CLOCK_MINUTE)); nz8((u8)Y);
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
            lda8(rd8(DP(CLOCK_MINUTE)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
        }
        rep(0x20);
        lda16(rd16(ABS(RIDER_COLUMN)));
        fc = 1; sbc16(rd16(DP(0x10)));
        and16(0x0020);
        for (int i = 0; i < 5; i++) lda16(asl16(A));
        wr16(DP(0x00), A);
        lda16(rd16(ABS(RIDER_COLUMN)));
        fc = 1; sbc16(rd16(DP(0x10)));
        and16(0x001F);
        fc = 0; adc16(rd16(DP(0x00)));
        fc = 0; adc16(rd16(ABSX(0xA892)));
        wr16(DP(0x00), A);
        lda16(0x0018); wr16(DP(0x02), 0x0018);
        sep(0x20);
        X = rd8(DP(CLOCK_MINUTE)); nz8((u8)X);
        lda8(0x21); wr8(DP(0x18), 0x21);
        lda8(rd8(ABSX(t19[pass]))); wr8(DP(0x19), (u8)A);
        lda8(rd8(ABSX(t1a[pass]))); wr8(DP(0x1A), (u8)A);
        lda8(0x08); wr8(DP(0x0F), 0x08);
        push16(pass ? 0xA86A : 0xA822); VramQueueTiles_m1x1();
    }
    wr8(DP(CLOCK_MINUTE), 0);
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
    lda8(0x07); wr8(DP(COURSE_SLOT), 0x07);
    lda8(0x05); wr8(DP(0x54), 0x05);
    lda8(0xA0); wr8(ABS(GROUND_LINE), 0xA0);
    ldx8(0x07);
    lda8(0xC0);
    do {
        wr8(ABSX(START_POLES), (u8)A);
        fc = 1; sbc8(0x10);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x0E); wr8(ABS(RIDER_LANE), 0x0E);
    lda8(0x08); wr8(ABS(RIDER_WHEEL), 0x08);
    lda8(0x09); wr8(ABS(RIDER_COLUMN), 0x09);
    lda8(0x38); wr8(ABS(RIDER_X), 0x38);
    lda8(0x06); wr8(ABS(RIDER_LEAN), 0x06); wr8(ABS(RIDER_TARGET_HI), 0x06);
    lda8(0x01); wr8(ABS(RIDER_PAGE), 0x01); wr8(ABS(RIDER_ACTIVE), 0x01);
    lda8(0x01); wr8(ABS(RIDER_FINISHED), 0x01); wr8(DP(VSCROLL_ZOOM), 0x01); wr8(DP(MODE_ACTIVE), 0x01);
    lda8(0x01); wr8(ABS(FADE_STATE), 0x01);
    lda8(0x19); wr8(ABS(BG3SC), 0x19);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    push8(0x80); push16(0xE314); LoadCourse_m1x1();
    push16(0xE317); DemoPadDisplay_m1x1();
    ret();
}

void ClearWords1530_m1x1(void);

/* $80:E8B1 RiderLanePieceAt (RTS): RiderPieceAt in rider X's lane $08A4. */
void RiderLanePieceAt_m1x1(void)
{
    Y = rd8(ABSX(RIDER_LANE_NUM)); nz8((u8)Y);
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
    wr8(ABS(COIN_COMBO), 0); wr8(ABS(MARKER_COUNT), 0);
    ClearWords1530_m1x1();
}

/* $80:8D25 Mode01 (RTS): the player-select screen's step $1E8B through
 * the table $80:8D2B. */
void Mode01_m1x1(void)
{
    X = rd8(ABS(SELECT_STEP)); nz8((u8)X);
    r816_call_pc(0x800000u | rd16(LONGX(0x808D2B)));
}

/* $80:E9C1 FlattenPlayerPiece (RTS, X = rider): the player on the ground
 * (and $84 clear) turns its piece flat ($1F) and sets the coin type $AA
 * from its lane ($80:E9EC). */
void FlattenPlayerPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_AIR)));
    ora8(rd8(DP(RACE_END)));
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
    X = rd8(ABS(RIDER_LANE_NUM)); nz8((u8)X);
    lda8(rd8(ABSX(0xE9EC)));
    wr8(DP(COIN_TAKEN), (u8)A);
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
        wr8(ABSX(RIDER_TILE), (u8)A);
        lda8(rd8(DP(0x10)));
        wr8(ABSX(RIDER_TILE_Y), (u8)A);
        for (;;) {
            push8((u8)A);
            Y = inc8((u8)Y);
            lda8((u8)Y);
            and8(0x3F);
            wr8(DP(0x08), (u8)A);
            Y = rd8(DP(0x0C)); nz8((u8)Y);
            lda8(pull8());
            wr8(ABSY(PIECES_AHEAD), (u8)A);
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
    lda16(rd16(ABSX(OBJ_Y)));
    and16(0x00F8);
    lda16(asl16(A)); lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(OBJ_X)));
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
        lda8(rd8(ABSX(RIDER_HEIGHT)));
        if (!fz) ldy8(0x04);
    }
    lda8(rd8(ABSX(RIDER_LANE)));
    cmp8((u8)A, rd8(ABSY(0xBDA2)));
    if (!fz) {
        ldy8(0x01);
        if (fc) ldy8(0xFF);
        lda8((u8)Y);
        wr8(ABSX(RIDER_STEER), (u8)A);
        ret();
        return;
    }
    lda8(0x00);
    wr8(ABSX(FINISH_POSE), 0);
    wr8(ABSX(RIDER_STEER), 0);
    wr8(ABSX(RIDER_TIMER), inc8(rd8(ABSX(RIDER_TIMER))));
    lda8((u8)X);
    if (fz) {
        Y = rd8(ABS(TURBO_ON)); nz8((u8)Y);
        if (!fz) wr8(ABS(ENGINE_HEAT), (u8)A);
        wr8(ABS(TURBO_ON), (u8)A);
        wr8(ABS(COIN_COUNT), (u8)A);
        push8(0x80); push16(0xBEA9); ResetCoinMarkers_m1x1();
        lda8(0x80);
        push8(0x80); push16(0xBEAF); QueueSound_m1x1();
        lda8(0x02);
    }
    lda8(0x08);
    wr8(DPX(RIDER_LEAN_TIMER), 0x08);
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
        lda8(rd8(ABSX(EFFECT_KIND)));
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
    X = rd16(ABS(OBJ_CURRENT)); nz16(X);
    lda16(rd16(ABSX(OBJ_STEP)));
    lda16(asl16(A)); lda16(asl16(A));
    Y = A; nz16(Y);
    lda16(rd16(ABSY(0x9B66)));
    wr16(ABS(OBJ_TARGET_X), A);
    lda16(rd16(ABSY(0x9B68)));
    wr16(ABS(OBJ_TARGET_Y), A);
    sep(0x30);
    ldy8(0x0C);
    lda8(0x20);
    push8(0x81); push16(0x9798); SteerObjectStep_m1x1();
    if (fc) {
        rep(0x30);
        wr16(ABSX(OBJ_STEP), inc16(rd16(ABSX(OBJ_STEP))));
        lda16(rd16(ABSX(OBJ_STEP)));
        lda16(asl16(A)); lda16(asl16(A));
        Y = A; nz16(Y);
        lda16(rd16(ABSY(0x9B66)));
        cmp16(A, 0x8000);
        sep(0x30);
        if (fz) {
            rep(0x20);
            lda16(0x0060);
            wr16(ABSX(OBJ_STEP), 0x0060);
            sep(0x20);
            wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
            ret();
            return;
        }
    }
    lda8(rd8(ABSX(OBJ_X + 0x01)));
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
    lda16(rd16(DP(COURSE_COLUMN)));
    fc = 1; sbc16(0x0054);
    if (fn) { fc = 0; adc16(rd16(DP(0xA8))); }
    X = A; nz16(X);
    lda16(rd16(LONGX(TRACK_COL_INDEX)));
    cmp16(A, rd16(LONGX(TRACK_BUF + 0x1FFE)));
    if (!fz) lda16(dec16(A));
    X = A; nz16(X);
    lda16(rd16(ABS(RIDER_LANE_NUM)));
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
        lda8(rd8(ABSX(RIDER_AIR)));
        if (fz) {
            Y = rd8(ABS(RIDER_LANE_NUM)); nz8((u8)Y);
            lda8(rd8(ABSY(0xEA63)));
            if (!fz) {
                lda8(0x32);
                push8(0x80); push16(0xEA09); QueueSound_m1x1();
                lda8(0x01); wr8(ABS(WHEELIE_LOCK), 0x01);
                player_piece1(0xEA11);
                lda8(rd8(piece_addr1()));
                cmp8((u8)A, 0x15);
                if (fz) {
                    lda8(0x12);
                    wr8(piece_addr1(), 0x12);
                    push8((u8)X);
                    course_map_slot1();
                    lda8(rd8(ABSY(0xEA5C)));
                    wr8(LONGX(TRACK_BUF), (u8)A);
                    lda8(rd8(ABSY(0xE9EC)));
                    wr8(DP(CLOCK_MINUTE), (u8)A);
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
        lda8(rd8(ABSX(RIDER_AIR)));
        if (fz) {
            push8(0x80); push16(0xEA76); SpawnPlayerSpark_m1x1();
            Y = rd8(ABS(RIDER_LANE_NUM)); nz8((u8)Y);
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
                    wr8(LONGX(TRACK_BUF), 0x40);
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
    lda8(rd8(LONG(STADIUM_SELECT)));
    int two = !fz;
    if (!two) {
        ldx8(0x06);
        ldy8(0x00);
        wr8(ABS(SELECT_STEP), 0);
        do {
            lda8(rd8(LONGX(0x115719)));
            if (!fz) Y = inc8((u8)Y);
            X = dec8((u8)X);
        } while (!fn);
        cmp8((u8)Y, 0x02);
        two = fc;
    }
    if (two) { lda8(0x02); wr8(ABS(SELECT_STEP), 0x02); }
    ldx8(0x03);
    do {
        lda8(rd8(ABS(SELECT_STEP)));
        if (fz) lda8(rd8(ABSX(0x91C9)));
        else lda8(rd8(ABSX(0x91C5)));
        wr8(ABSX(SELECT_WHEELS), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    lda8(0x01); wr8(ABS(FADE_STATE), 0x01);
    lda8(0x01); wr8(ABS(BG1SC), 0x01);
    wr8(DP(P1_HELD_HI), 0); wr8(DP(P1_HELD_LO), 0);
    push16(0x91B2); Mode01_m1x1();
    rep(0x20);
    lda16(0x0708); wr16(ABS(MODE_TIMER), 0x0708);
    lda16(0x0000); wr16(LONG(TRACK_BUF), 0);
    sep(0x20);
    ret();
}

void QueueScoreDigitsHud_m1x1(void);
void LoadSavedEntry_m1x1(void);
void MergeBsxFlags_m1x1(void);
void SumBytes0850_m1x1(void);

/* $80:B158 (RTS): the race's per-frame bookkeeping: $7F = frame & 7, the
 * rider pieces, and while racing for real (no $0CD7, $1923 clear, mode
 * below 9) the pace bonus, the score and $80:EE5C. */
void RaceBookkeeping_m1x1(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x07);
    wr8(DP(RIDER_TURN), (u8)A);
    push16(0xB160); ReadRiderPieces_m1x1();
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(FADE_STATE)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(GAME_MODE)));
    cmp8((u8)A, 0x09);
    if (fc) { ret(); return; }
    push16(0xB173); PaceBonus_m1x1();
    push16(0xB176); AddScore_m1x1();
    push16(0xB179); CALL(QueueScoreDigitsHud_m1x1, 0x80EE5C, 2);
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
    push8(0x80); push16(0xA0CB); CALL(LoadSavedEntry_m1x1, 0x80A2B0, 3);
    push8(0x80); push16(0xA0CF); MergeBsxFlags_m1x1();
    push8(0x80); push16(0xA0D3); SumBytes0850_m1x1();
    ldx8(0x0B);
    do {
        lda8(rd8(ABSX(ENTRY_CODE)));
        and8(0x30);
        lda8(asl8((u8)A));
        fc = 0; adc8(0x20);
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(ENTRY_CODE)));
        and8(0x0F);
        ora8(rd8(DP(0x00)));
        wr8(ABSX(DIGIT_ROW), (u8)A);
        ora8(0x10);
        wr8(ABSX(DIGIT_ROW + 0x0C), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x2C); wr8(ABS(DIGIT_BUF), 0x2C);
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
    lda8(rd8(ABS(RECORD_COURSE)));
    if (!fz) {
        lda8(rd8(ABS(RECORD_TIME))); wr8(ABS(RACE_MIN), (u8)A);
        lda8(rd8(ABS(RECORD_TIME + 0x01))); wr8(ABS(RACE_SEC), (u8)A);
        lda8(rd8(ABS(RECORD_TIME + 0x02))); wr8(ABS(RACE_FRAMES), (u8)A);
        lda8(rd8(ABS(RECORD_COURSE)));
        lda8(dec8((u8)A));
        lda8(asl8((u8)A));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x972B))); wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(0x972C))); wr8(DP(0x01), (u8)A);
        push16(0xA527); TickRaceClock_m1x1();
        lda8(rd8(DP(FRAME_COUNT)));
        and8(0x01);
        if (fz) {
            lda8(rd8(LONG(VQ_END)));
            fc = 1; sbc8(0x27);
            X = (u8)A; nz8((u8)X);
            lda8(0x24);
            static const u8 off[14] = { 0,2,4,6,8,0x0A,0x0C,0x1A,0x1C,0x1E,0x20,0x22,0x24,0x26 };
            for (int i = 0; i < 14; i++) wr8(LONGX(VQ_VMADD + off[i]), 0x24);
        }
    }
    lda8(0x01); wr8(ABS(MODE_TIMER), 0x01);
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
    wr8(ABS(HDMAEN), 0);
    wr8(ABS(SH_HDMAEN), 0);
    wr8(ABS(SH_TMW), 0); wr8(ABS(SH_TSW), 0);
    wr8(ABS(0x1907), 0); wr8(ABS(0x1908), 0);
    wr8(ABS(0x1909), 0); wr8(ABS(SH_CGWSEL), 0);
    ldx8(0x20); wr8(ABS(SH_CGADSUB), 0x20);
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
    wr16(ABS(DMAP2), 0x3240); wr16(ABS(DMAP3), 0x3240); wr16(ABS(DMAP4), 0x3240);
    X = 0x0695; nz16(X); wr16(ABS(A1T2L), 0x0695);
    X = 0x0715; nz16(X); wr16(ABS(A1T3L), 0x0715);
    X = 0x0795; nz16(X); wr16(ABS(A1T4L), 0x0795);
    lda8(0x00);
    wr8(ABS(A1B2), 0); wr8(ABS(A1B3), 0); wr8(ABS(A1B4), 0);
    wr8(ABS(DASB2), 0); wr8(ABS(DASB3), 0); wr8(ABS(DASB4), 0);
    lda8(0x01); wr8(ABS(0x0815), 0x01);
    lda8(0x1C); wr8(ABS(SH_HDMAEN), 0x1C);
    sep(0x10);
    wr8(ABS(PALETTE_BUF), 0); wr8(ABS(PALETTE_BUF + 0x01), 0);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
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
    lda8(rd8(ABS(FADE_STATE)));
    int clear = !fz;
    if (!clear) {
        X = rd8(DP(GAME_MODE)); nz8((u8)X);
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
        lda16(rd16(ABSY(SPRITE_FRAME)));
        wr16(DP(0x0E), A);
        cmp8((u8)Y, 0x00);
        if (fz) {
            and16(0x00FF);
            cmp16(A, 0x0015);
            if (fz) {
                lda16(rd16(ABSY(FINISH_POSE)));
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
        lda16(rd16(ABS(PLAYER_CHAR)));
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
    wr16(ABS(ANIM_VRAM), 0);
    wr16(ABS(ANIM_STRIP), inc16(rd16(ABS(ANIM_STRIP))));
    lda16(rd16(ABS(ANIM_STRIP)));
    and16(0x0001);
    if (fz) {
        lda16(rd16(ABS(ANIM_STRIP)));
        and16(0x001E);
        X = (u8)A; nz8((u8)X);
        lda16(rd16(ABSX(0xB638)));
        fc = 0; adc16(0x8000);
        wr16(ABS(ANIM_SRC), A);
        lda16(X);
        and16(0x0006);
        lda16(lsr16(A));
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        fc = 0; adc16(0x4F00);
        wr16(ABS(ANIM_VRAM), A);
    }
    sep(0x20);
    lda8(rd8(ABS(PLAYER_CHAR)));
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
        lda8(0xFC); wr8(ABSX(TRACK_ROWS), 0xFC);
        lda8(0x00);
        wr8(ABSX(TRACK_ROWS + 0x40), 0); wr8(ABSX(TRACK_ROWS + 0x80), 0); wr8(ABSX(TRACK_ROWS + 0xC0), 0);
        lda8(0x3B);
        wr8(ABSX(TRACK_ROWS + 0x180), 0x3B); wr8(ABSX(TRACK_ROWS + 0x1C0), 0x3B); wr8(ABSX(TRACK_ROWS + 0x200), 0x3B);
        wr8(ABSX(TRACK_ROWS + 0x240), 0x3B); wr8(ABSX(TRACK_ROWS + 0x280), 0x3B); wr8(ABSX(TRACK_ROWS + 0x2C0), 0x3B);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x34); wr8(ABS(TRACK_ROW_PTR), 0x34);
    lda8(0x0A); wr8(ABS(BUILD_COL), 0x0A);
    lda8(0x05); wr8(ABS(TRACK_VRAM_HALF), 0x05);
    lda8(0x74); wr8(ABS(TRACK_VRAM_COL), 0x74);
    lda8(0x08); wr8(ABS(RIDER_WHEEL), 0x08);
    lda8(0x09); wr8(ABS(RIDER_COLUMN), 0x09);
    lda8(0x30); wr8(ABS(RIDER_X), 0x30);
    lda8(0x06); wr8(ABS(RIDER_LEAN), 0x06);
    lda8(0x01);
    wr8(ABS(RIDER_PAGE), 0x01); wr8(ABS(RIDER_ACTIVE), 0x01); wr8(ABS(RIDER_BAND), 0x01);
    lda8(asl8((u8)A));
    wr8(DP(VSCROLL_ZOOM), (u8)A);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    lda8(0x01);
    push8(0x80); push16(0xD87C); SetHdmaPreset_m1x1();
    ldx8(0x00);
    lda8(0xB0); wr8(ABS(GROUND_LINE), 0xB0); wr8(ABS(RIDER_Y), 0xB0);
    push16(0xD889); DrawAllRiders_m1x1();
    lda8(0xB8); wr8(ABS(GROUND_LINE), 0xB8); wr8(ABS(RIDER_Y), 0xB8);
    static const u8 ch[12][2] = {
        {0x70,0x42},{0x71,0x0F},{0x72,0xED},{0x73,0xD8},{0x74,0x80},{0x77,0x00},
        {0x60,0x40},{0x61,0x08},{0x62,0xF4},{0x63,0xD8},{0x64,0x80},{0x67,0x00} };
    for (int i = 0; i < 12; i++) { lda8(ch[i][1]); wr8(ABS(0x4300 | ch[i][0]), ch[i][1]); }
    rep(0x20);
    lda16(0x012C); wr16(DP(MODE_TIMER), 0x012C);
    wr16(ABS(0x1B00), 0); wr16(ABS(0x1B02), 0);
    sep(0x20);
    lda8(0x09); wr8(ABS(0x1B20), 0x09);
    lda8(0x08); wr8(ABS(0x1B21), 0x08);
    lda8(0xDC); wr8(ABS(SH_HDMAEN), 0xDC);
    ret();
}

/* Stadium 1's rider jump steps ($80:E90D-$80:E9B1), run by the piece
 * interpreter's table $80:E701. */

/* $80:E975 (RTS, X = rider): $01A0 = $0981 = A, then A = $0248 - $0238. */
static void set_rider_pair1(void)
{
    wr8(ABSX(RIDER_POSE), (u8)A);
    wr8(ABSX(RIDER_ACTION_POSE), (u8)A);
    lda8(rd8(ABSX(RIDER_DISTANCE)));
    fc = 1; sbc8(rd8(ABSX(0x0238)));
    ret();
}

void SetRiderPair_m1x1(void) { set_rider_pair1(); }

/* $80:E929 (RTS): jump height $01F8 and its base $0268 = A. */
static void set_jump_base1(void)
{
    wr8(ABSX(RIDER_HEIGHT), (u8)A);
    wr8(ABSX(RIDER_JUMP_BASE), (u8)A);
    ret();
}

void SetJumpBase_m1x1(void) { set_jump_base1(); }

/* $80:E94C (RTS): jump height $01F8 = $0268 - A. */
static void height_below1(void)
{
    wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(RIDER_JUMP_BASE)));
    fc = 1; sbc8(rd8(DP(0x00)));
    wr8(ABSX(RIDER_HEIGHT), (u8)A);
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
    lda8(0x80); wr8(ABSX(RIDER_SLOW_AT), 0x80);
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
    lda8(0x60); wr8(ABSX(RIDER_SLOW_AT), 0x60);
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
    lda8(rd8(ABSX(RIDER_LANDING)));
    if (!fz) {
        lda8(rd8(ABSX(RIDER_HEIGHT)));
        fc = 0; adc8(0x10);
        push16(0xE96E); set_jump_base1();
    }
    lda8(0x40); wr8(ABSX(RIDER_SLOW_AT), 0x40);
    ret();
}

/* $80:E983 (RTS): unless in jump state 1, SetRiderPair(3) and the height
 * twice the pair below; in state 4 it crash-lands into state 2 ($08AC
 * too). */
void RiderDropStep_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_JUMP)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) { ret(); return; }
    }
    pair1(0x03, 0xE990);
    lda8(asl8((u8)A));
    push16(0xE994); height_below1();
    lda8(rd8(ABSX(RIDER_JUMP)));
    cmp8((u8)A, 0x04);
    if (!fz) { ret(); return; }
    push16(0xE99E); CrashLand_m1x1();
    lda8(0x02);
    wr8(ABSX(RIDER_JUMP), 0x02);
    wr8(ABSX(RIDER_LANDED), 0x02);
    ret();
}

/* $80:E9A8 (RTS): jump state $0208 and $0200 cleared. */
void ClearJumpState_m1x1(void)
{
    lda8(0x00);
    wr8(ABSX(RIDER_JUMP), 0);
    wr8(ABSX(RIDER_LANDING), 0);
    ret();
}

/* $80:E9B1 (RTS): jump state 1, or 2 when the rider's lane $01F0 is
 * below $20. */
void StartJumpState_m1x1(void)
{
    lda8(0x01); wr8(ABSX(RIDER_JUMP), 0x01);
    lda8(rd8(ABSX(RIDER_LANE)));
    cmp8((u8)A, 0x20);
    if (!fc) wr8(ABSX(RIDER_JUMP), inc8(rd8(ABSX(RIDER_JUMP))));
    ret();
}

/* $80:E90F (RTS, X = last rider): riders X-0 on the ground get their
 * height $01B0 from GroundHeightAbove. */
void RiderGroundHeightsFrom_m1x1(void)
{
    do {
        lda8(rd8(ABSX(RIDER_AIR)));
        if (fz) {
            push16(0xE916); GroundHeightAbove_m1x1();
            wr8(ABSX(RIDER_Y), (u8)A);
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

/* $80:E834 PieceAction (RTS, A = action 0-24): jump through the rider
 * piece action table $80:E701. Checked with difftest --set-a for each
 * action (higher numbers read past the table). */
void PieceAction_m1x1(void)
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
    lda8(rd8(ABSX(RIDER_STEP)));
    fc = 0; adc8(rd8(ABSX(RIDER_DISTANCE)));
    wr8(ABSX(RIDER_DISTANCE), (u8)A);
    cmp8((u8)X, 0x00);
    if (fz) lda8(rd8(ABSX(RIDER_SCRIPT)));
    lda8(rd8(ABSX(RIDER_SCRIPT)));
    if (fz) {
        lda8(rd8(ABSX(RIDER_TILE)));
        cmp8((u8)A, 0x18);
        if (fc) {
            int start = 0;
            lda8(rd8(ABSX(RIDER_JUMP)));
            cmp8((u8)A, 0x03);
            if (fz) {
                lda8(rd8(ABSX(RIDER_LANE_NUM)));
                cmp8((u8)A, 0x03);
                if (!fc) {
                    lda8(0x0C);
                    wr8(ABSX(RIDER_TILE), 0x0C);
                    start = 1;
                }
            }
            if (!start) {
                cmp8((u8)X, 0x00);
                if (fz) wr8(ABS(WHEELIE_LOCK), 0);
                ret();
                return;
            }
        }
        wr8(ABSX(RIDER_SCRIPT), (u8)A);
        wr8(ABSX(RIDER_SCRIPT), inc8(rd8(ABSX(RIDER_SCRIPT))));
        lda8(0x00);
        wr8(ABSX(RIDER_PIECE_POS), 0);
        lda8(rd8(ABSX(RIDER_WHEEL)));
        wr8(ABSX(RIDER_DISTANCE), (u8)A);
        wr8(ABSX(RIDER_DISTANCE), dec8(rd8(ABSX(RIDER_DISTANCE))));
    }
    for (;;) {
        /* L80E7FA */
        Y = rd8(ABSX(RIDER_SCRIPT)); nz8((u8)Y);
        lda8(rd8(ABSY(0xE574))); wr8(DP(0x0A), (u8)A);
        lda8(rd8(ABSY(0xE58B))); wr8(DP(0x0B), (u8)A);
        Y = rd8(ABSX(RIDER_PIECE_POS)); nz8((u8)Y);
        lda8(script_byte1());
        cmp8((u8)A, 0xFF);
        if (fz) break;
        wr8(DP(0x0F), (u8)A);
        cmp8((u8)A, rd8(ABSX(RIDER_DISTANCE)));
        if (!fz && fc) { ret(); return; }
        Y = inc8((u8)Y);
        lda8(script_byte1());
        bit8(rd8(ABS(0xE700)), 0);
        if (!fz) {
            /* L80E843 */
            lda8(rd8(ABSX(RIDER_AIR)));
            ora8(rd8(ABSX(RIDER_BUMPED)));
            if (fz) {
                lda8(script_byte1());
                and8(0x0F);
                wr8(DP(0x00), (u8)A);
                lda8(rd8(ABSX(RIDER_JUMP)));
                cmp8((u8)A, 0x01);
                if (!fz) {
                    lda8(rd8(DP(0x00)));
                    wr8(ABSX(RIDER_LEAN), (u8)A);
                    lda8(rd8(ABSX(RIDER_SCRIPT)));
                    cmp8((u8)A, 0x03);
                    if (!fz) {
                        lda8(rd8(ABSX(RIDER_LEAN)));
                        fc = 1; sbc8(0x02);
                        Y = (u8)A; nz8((u8)Y);
                        lda8(rd8(ABSY(0xE6F5)));
                        wr8(ABSX(RIDER_POSE), (u8)A);
                    }
                }
            }
        } else {
            bit8(rd8(ABS(0xE6FF)), 0);
            if (fz) {
                push16(0xE82B); PieceAction_m1x1();
                wr8(ABSX(RIDER_PIECE_POS), inc8(rd8(ABSX(RIDER_PIECE_POS))));
                wr8(ABSX(RIDER_PIECE_POS), inc8(rd8(ABSX(RIDER_PIECE_POS))));
                if (!fz) continue;
                PieceAction_m1x1();
                return;
            }
            /* L80E8A5 */
            and8(0x0F);
            wr8(ABSX(RIDER_PIECE_ACTION), (u8)A);
            lda8(rd8(DP(0x0F)));
            wr8(ABSX(0x0238), (u8)A);
            if (fn) { RiderLanePieceAt_m1x1(); return; }
        }
        /* L80E871 */
        wr8(ABSX(RIDER_PIECE_POS), inc8(rd8(ABSX(RIDER_PIECE_POS))));
        wr8(ABSX(RIDER_PIECE_POS), inc8(rd8(ABSX(RIDER_PIECE_POS))));
        if (fz) break;
    }
    /* L80E87C */
    lda8(0x00);
    wr8(ABSX(RIDER_SCRIPT), 0);
    wr8(ABSX(RIDER_POSE), 0);
    lda8(rd8(ABSX(RIDER_LANDING)));
    if (fz) {
        lda8(rd8(ABSX(RIDER_JUMP)));
        cmp8((u8)A, 0x01);
        if (!fz) {
            lda8(0x00);
            wr8(ABSX(RIDER_HEIGHT), 0);
            lda8(rd8(ABSX(RIDER_JUMP)));
            cmp8((u8)A, 0x02);
            if (fz) wr8(ABSX(RIDER_JUMP), inc8(rd8(ABSX(RIDER_JUMP))));
        }
    }
    lda8(0x00);
    wr8(ABSX(RIDER_ON_RAMP), 0);
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
void CourseRowTiles_m1x1(void)
{
    do {
        WAIT_TICK();
        Y = inc8((u8)Y);
        Y = inc8((u8)Y);
        rep(0x20);
        lda16(rd16((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        and16(0x00FF);
        ora16(0x0800);
        wr16(LONGX(VQ0_DATA), A);
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
    lda8(rd8(LONG(STADIUM_SELECT)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xD445))); wr8(DP(0x07), (u8)A);
    lda8(rd8(ABSX(0xD447))); wr8(DP(0x08), (u8)A);
    Y = rd8(ABS(TRACK_READ_POS)); nz8((u8)Y);
    if (fz) {
        lda8(course_list1(0x07));
        wr8(ABS(0x09C9), (u8)A);
        Y = inc8((u8)Y);
        wr8(ABS(TRACK_READ_POS), (u8)Y);
    }
    lda8(rd8(ABS(BUILD_COL)));
    cmp8((u8)A, rd8(ABS(RIDER_COLUMN)));
    if (!fz) { ret(); return; }
    int step;   /* 0 = next entry, 1 = counting empty rows, 2 = same row */
    lda8(rd8(ABS(BUILD_EMPTY)));
    if (!fz) step = 1;
    else {
        lda8(rd8(ABS(ROW_OFFSET)));
        step = fz ? 0 : 2;
    }
    for (;;) {
        WAIT_TICK();
        if (step == 0) {
            Y = rd8(ABS(TRACK_READ_POS)); nz8((u8)Y);
            wr8(ABS(TRACK_READ_POS), inc8(rd8(ABS(TRACK_READ_POS))));
            lda8(course_list1(0x07));
            if (fz) { ret(); return; }
            cmp8((u8)A, 0x09);
            if (fz) {
                lda8(0x01);
                wr8(ABS(TRACK_READ_POS), 0x01);
            }
            cmp8((u8)A, 0x80);
            if (fc) {
                and8(0x7F);
                wr8(ABS(BUILD_EMPTY), (u8)A);
                step = 1;
            } else {
                wr8(ABS(ROW_TYPE), (u8)A);
                step = 2;
            }
        }
        if (step == 1) {
            wr8(ABS(BUILD_EMPTY), dec8(rd8(ABS(BUILD_EMPTY))));
            lda8(0x00);
            wr8(ABS(BUILD_COLUMN), 0);
            wr8(ABS(ROW_OFFSET), 0);
            wr8(ABS(ROW_TYPE), 0);
        }
        /* L80DBE5 */
        X = rd8(ABS(ROW_TYPE)); nz8((u8)X);
        push16(0xDBEA); CourseRowList_m1x1();
        Y = rd8(ABS(ROW_OFFSET)); nz8((u8)Y);
        lda8(course_list1(0x05));
        if (!fz) break;
        wr8(ABS(ROW_OFFSET), (u8)A);
        wr8(ABS(BUILD_COLUMN), (u8)A);
        step = 0;
    }
    push16(0xDBF7); BlankQueueRow_m1x1();
    lda8(course_list1(0x05));
    X = (u8)A; nz8((u8)X);
    push16(0xDBFD); CourseRowTiles_m1x1();
    Y = inc8((u8)Y);
    Y = inc8((u8)Y);
    wr8(ABS(ROW_OFFSET), (u8)Y);
    lda8(0x81); wr8(LONG(VQ0_VMAIN), 0x81);
    lda8(0x01); wr8(LONG(VQ0_DMAP), 0x01);
    lda8(0x18); wr8(LONG(VQ0_BBAD), 0x18);
    lda8(rd8(LONG(VQ_END)));
    fc = 0; adc8(0x0C);
    wr8(LONG(VQ0_SRC), (u8)A);
    lda8(0x00); wr8(LONG(VQ0_SRC_HI), 0);
    lda8(0x7F); wr8(LONG(VQ0_SRC_BANK), 0x7F);
    rep(0x20);
    lda16(rd16(LONG(VQ_END)));
    fc = 0; adc16(0x002A);
    wr16(LONG(VQ_END), A);
    wr16(LONG(VQ0_NEXT), A);
    lda16(rd16(ABS(TRACK_VRAM_COL)));
    fc = 1; sbc16(0x0020);
    wr16(LONG(VQ0_VMADD), A);
    lda16(0x001E); wr16(LONG(VQ0_SIZE), 0x001E);
    sep(0x20);
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    wr8(DP(0x0F), (u8)A);
    push16(0xDC59); CourseColumn_m1x1();
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    push16(0xDC5F); NextColumn_m1x1();
    wr8(ABS(TRACK_ROW_PTR), (u8)A);
    wr8(ABS(TRACK_VRAM_COL), inc8(rd8(ABS(TRACK_VRAM_COL))));
    lda8(rd8(ABS(TRACK_VRAM_COL)));
    cmp8((u8)A, 0x80);
    if (fz) {
        lda8(0x60); wr8(ABS(TRACK_VRAM_COL), 0x60);
        lda8(rd8(ABS(TRACK_VRAM_HALF)));
        eor8(0x04);
        wr8(ABS(TRACK_VRAM_HALF), (u8)A);
    }
    lda8(rd8(ABS(RIDER_COLUMN)));
    push16(0xDC7F); NextColumn_m1x1();
    wr8(ABS(BUILD_COL), (u8)A);
    ret();
}

/* $80:A576: the free run results wait: objects update; once the timer
 * $7F:FFFF reaches the stage $80:A5A8 value the next stage starts (mode
 * 2, $7C + 1, $7D = 0), or after stage 3 the end (mode 9, $1E87 - 10). */
static void free_run_wait1(void)
{
    push8(0x80); push16(0xA579); UpdateObjects_m1x1();
    X = rd8(DP(COURSE_SLOT)); nz8((u8)X);
    lda8(rd8(LONG(BCAST_MINUTE)));
    cmp8((u8)A, rd8(ABSX(0xA5A8)));
    if (!fc) { ret(); return; }
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    cmp8((u8)X, 0x03);
    if (!fc) {
        wr8(DP(FINAL_RACE), 0);
        wr8(DP(COURSE_SLOT), inc8(rd8(DP(COURSE_SLOT))));
        lda8(0x02); wr8(DP(GAME_MODE), 0x02);
    } else {
        lda8(0x09); wr8(DP(GAME_MODE), 0x09);
        lda8(rd8(ABS(BONUS_TRIES)));
        fc = 1; sbc8(0x0A);
        wr8(ABS(BONUS_TRIES), (u8)A);
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
    lda8(rd8(DP(FINAL_RACE)));
    if (!fz) { free_run_wait1(); return; }
    X = rd8(DP(COURSE_SLOT)); nz8((u8)X);
    lda8(rd8(ABSX(0x9727)));
    cmp8((u8)A, rd8(LONG(BCAST_MINUTE)));
    if (!fc) {
        lda8(0x02); wr8(DP(GAME_MODE), 0x02);
        wr8(DP(MODE_ACTIVE), 0);
        wr8(DP(MODE_STEP), 0);
        wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
        wr8(DP(FINAL_RACE), inc8(rd8(DP(FINAL_RACE))));
        ret();
        return;
    }
    wr8(ABS(MODE_TIMER), dec8(rd8(ABS(MODE_TIMER))));
    if (!fz) { ret(); return; }
    lda8(0x20); wr8(ABS(MODE_TIMER), 0x20);
    lda8(rd8(ABS(ENTRY_CURSOR)));
    cmp8((u8)A, 0x0A);
    if (fc) { push16(0x96C0); FinalSchedule_m1x1(); ret(); return; }
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    fc = 0; adc8(rd8(ABS(ENTRY_CURSOR)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(COURSE_TIMES)));
    cmp8((u8)A, 0xFF);
    if (!fz) {
        wr8(ABS(RACE_MIN), (u8)A);
        lda8(rd8(ABSY(COURSE_TIMES + 0x01))); wr8(ABS(RACE_SEC), (u8)A);
        lda8(rd8(ABSY(COURSE_TIMES + 0x02))); wr8(ABS(RACE_FRAMES), (u8)A);
        lda8(0x42);
        push8(0x80); push16(0x96F2); QueueSound_m1x1();
        rep(0x20);
        lda16(rd16(ABSX(0x972B)));
        wr16(DP(0x00), A);
        sep(0x20);
        push16(0x96FE); TickRaceClock_m1x1();
    } else {
        lda8(rd8(ABS(ENTRY_CURSOR)));
        if (fz) {
            lda8(0x0A); wr8(ABS(ENTRY_CURSOR), 0x0A);
            lda8(0x01); wr8(ABS(MODE_TIMER), 0x01);
            ret();
            return;
        }
        lda8(0x09); wr8(ABS(ENTRY_CURSOR), 0x09);
    }
    wr8(ABS(ENTRY_CURSOR), inc8(rd8(ABS(ENTRY_CURSOR))));
    lda8(rd8(ABS(ENTRY_CURSOR)));
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
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) goto tick;
    lda8(rd8(DP(PLACE_BONUS)));
    if (fz) {
        ldy8(0x0E);
        do {
            lda8(rd8(ABSY(PLAYER_PLACE)));
            lda8(dec8((u8)A));
            if (fz) break;
            Y = dec8((u8)Y);
            Y = dec8((u8)Y);
        } while (!fn);
        lda8((u8)Y);
        lda8(lsr8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABS(PLAYER_CHAR)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(0x03); wr8(DP(0x01), 0x03);
        lda8(0x07);
        push8(0x81); push16(0x9882); FindObjectSlot_m1x1();
        lda8(rd8(DP(0x01))); wr8(ABSY(OBJ_STATE), (u8)A);
        lda8(0x10); wr8(ABSY(OBJ_DELAY), 0x10);
        X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
        wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
        Y = rd8(DP(0x00)); nz8((u8)Y);
        lda8(rd8(ABSY(0x9936)));
        push8(0x81); push16(0x989B); FindObjectSlot_m1x1();
        lda8(0x04); wr8(ABSY(OBJ_STATE), 0x04);
        lda8(0xFF); wr8(ABSY(OBJ_DELAY), 0xFF);
        lda8(0xFF); wr8(ABSY(OBJ_COUNTER), 0xFF);
        goto tick;
    }
    wr8(DP(PLACE_BONUS), dec8(rd8(DP(PLACE_BONUS))));
    lda8(0x44);
    push8(0x81); push16(0x98B5); QueueSound_m1x1();
    ldy8(0x00);
    lda8(rd8(DP(PLACE_BONUS)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
    }
    wr8(DP(0x00), (u8)A);
    ora8(0x50); wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
    eor8(0x30); wr8(ABS(DIGIT_ROW + 0x07), (u8)A);
    cmp8((u8)Y, 0x00);
    if (!fz) {
        lda8((u8)Y);
        ora8(0x50); wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
        eor8(0x30); wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
    } else {
        lda8(0xFC); wr8(ABS(DIGIT_ROW + 0x01), 0xFC); wr8(ABS(DIGIT_ROW + 0x06), 0xFC);
    }
    lda8((u8)Y);
    ora8(rd8(DP(0x00)));
    if (fz) {
        lda8(0xFC);
        wr8(ABS(DIGIT_ROW + 0x02), 0xFC); wr8(ABS(DIGIT_ROW + 0x03), 0xFC);
        wr8(ABS(DIGIT_ROW + 0x07), 0xFC); wr8(ABS(DIGIT_ROW + 0x08), 0xFC);
    }
    lda8(0xFC); wr8(ABS(DIGIT_ROW), 0xFC); wr8(ABS(DIGIT_ROW + 0x05), 0xFC);
    lda8(0x04); wr8(DP(MODE_TIMER), 0x04);
    rep(0x20);
    lda16(0x1CB6); wr16(DP(0x00), 0x1CB6);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x991C); VramQueueTilesLong_m1x1();
    lda8(0x01); wr8(ABS(SCORE_ADD + 0x03), 0x01);
    push8(0x81); push16(0x9925); AddScoreLong_m1x1();
    wr8(DP(0x00), dec8(rd8(DP(0x00))));
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x992F); QueueScoreDigitsLong_m1x1();
tick:
    wr8(DP(MODE_TIMER), dec8(rd8(DP(MODE_TIMER))));
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
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
    lda8(0x01); wr8(ABS(FADE_STATE), 0x01);
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    lda8(rd8(DP(0x54)));
    if (fz) lda8(0x05);
    wr8(DP(0x54), (u8)A);
    lda8(0xA0); wr8(ABS(GROUND_LINE), 0xA0);
    ldx8(0x07);
    lda8(0xC0);
    do {
        wr8(ABSX(START_POLES), (u8)A);
        fc = 1; sbc8(0x10);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0xFF); wr8(ABS(0x098B), 0xFF);
    ldx8(0x01);
    wr8(DP(VSCROLL_ZOOM), 0x01);
    wr8(ABS(RIDER_FINISHED), 0x01);
    X = inc8((u8)X);
    wr8(ABS(0x098C), (u8)X);
    X = rd8(DP(FINAL_RACE)); nz8((u8)X);
    if (!fz) ldx8(0x07);
    do {
        lda8(rd8(ABSX(0xB841))); wr8(ABSX(RIDER_LANE), (u8)A);
        lda8(0x08); wr8(ABSX(RIDER_WHEEL), 0x08);
        lda8(rd8(ABSX(0xB839))); wr8(ABSX(RIDER_COLUMN), (u8)A);
        lda8(rd8(ABSX(0xB849))); wr8(ABSX(RIDER_X), (u8)A);
        lda8(0x06); wr8(ABSX(RIDER_LEAN), 0x06); wr8(ABSX(RIDER_TARGET_HI), 0x06);
        lda8(0x01); wr8(ABSX(RIDER_PAGE), 0x01); wr8(ABSX(RIDER_ACTIVE), 0x01);
        lda8(rd8(DPX(0x2D)));
        lda8(lsr8((u8)A));
        wr8(ABSX(RIDER_TARGET_LO), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(DPX(0x2D)));
    and8(0x03);
    if (fz) lda8(0x02);
    X = (u8)A; nz8((u8)X);
    wr8(ABSX(RIDER_TARGET_HI), lsr8(rd8(ABSX(RIDER_TARGET_HI))));
    push16(0xA911); RiderGroundHeights_m1x1();
    push16(0xA914); SortAndDrawRiders_m1x1();
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) {
        lda8(0x04); wr8(ABS(RECORD_TIME), 0x04);
        wr8(ABS(RECORD_TIME + 0x01), 0);
        wr8(ABS(RECORD_TIME + 0x02), 0);
    }
    wr8(ABS(RECORD_COURSE), 0);
    push8(0x80); push16(0xA92A); ShowCourseBest_m1x1();
    lda8(0x97); wr8(DP(0x00), 0x97);
    lda8(0x18); wr8(DP(0x01), 0x18);
    push16(0xA935); ShowRaceClock_m1x1();
    push8(0x80); push16(0xA939); LoadCourse_m1x1();
    push8(0x80); push16(0xA93D); ClearWords1530_m1x1();
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) {
        wr8(ABS(TURBO_ON), 0); wr8(ABS(COIN_COUNT), 0);
        wr8(ABS(COIN_COMBO), 0); wr8(ABS(MARKER_COUNT), 0);
    }
    lda8(rd8(ABS(TURBO_ON)));
    if (!fz) { lda8(0x20); wr8(ABS(ENGINE_HEAT), 0x20); }
    ldx8(0x1D);
    lda8(0xFF);
    do {
        wr8(ABSX(COURSE_TIMES), 0xFF);
        X = dec8((u8)X);
    } while (!fn);
    wr8(ABS(SH_CGADSUB), 0);
    push8(0x80); push16(0xA968); MarkerObjects_m1x1();
    ldy8(0xE1);
    push16(0xA96D); DrawStartFlags_m1x1();
    lda8(0x04); wr8(ABS(PIC_SCROLL), 0x04);
    ldx8(0x0E);
    do {
        lda8(rd8(ABSX(0xA9AA))); wr8(ABSX(RIDER_PACE), (u8)A);
        lda8(rd8(ABSX(0xA9AB))); wr8(ABSX(RIDER_PACE + 0x01), (u8)A);
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
        lda8(rd8(ABSX(RIDER_PACE)));
        push8((u8)A);
        lda8(rd8(ABSY(RIDER_PACE)));
        wr8(ABSX(RIDER_PACE), (u8)A);
        lda8(pull8());
        wr8(ABSY(RIDER_PACE), (u8)A);
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
    cmp8((u8)X, rd8(DP(RIDER_TURN)));
    if (!fz) { ret(); return; }
    BrakeRider_m1x1();
}

/* $80:C31B: lean rider X one step toward angle A, every Y frames ($45,X). */
static void lean_toward1(void)
{
    wr8(DP(0x00), (u8)A);
    lda8(rd8(DPX(RIDER_LEAN_TIMER)));
    if (!fz) { ret(); return; }
    lda8((u8)Y);
    wr8(DPX(RIDER_LEAN_TIMER), (u8)A);
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, rd8(DP(0x00)));
    if (fz) { ret(); return; }
    if (fc) {
        wr8(ABSX(RIDER_LEAN), dec8(rd8(ABSX(RIDER_LEAN))));
        wr8(ABSX(RIDER_LEAN), dec8(rd8(ABSX(RIDER_LEAN))));
    }
    wr8(ABSX(RIDER_LEAN), inc8(rd8(ABSX(RIDER_LEAN))));
    ret();
}

/* $80:AE62: the d-pad leans rider X (every $80:B86D frames by air state):
 * up ($00 bit 0) leans back to $80:B861; otherwise forward to $80:B863,
 * and holding a button past it on the ground keeps tipping until angle
 * $0D flips it ($01C8 = 1, $3D,X = $0A, sound $24 / $29). */
static void lean_by_pad1(void)
{
    lda8(rd8(DPX(RIDER_LEAN_TIMER)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_AIR)));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xB86D)));
    wr8(DPX(RIDER_LEAN_TIMER), (u8)A);
    {
        u8 v = rd8(DP(0x00));
        fc = v & 1;
        v >>= 1;
        wr8(DP(0x00), v);
        nz8(v);
    }
    if (fc) {
        lda8(rd8(ABSX(RIDER_LEAN)));
        cmp8((u8)A, rd8(ABSY(0xB861)));
        if (fz) { ret(); return; }
        if (!fc) { wr8(ABSX(RIDER_LEAN), inc8(rd8(ABSX(RIDER_LEAN)))); ret(); return; }
        wr8(ABSX(RIDER_LEAN), dec8(rd8(ABSX(RIDER_LEAN))));
        ret();
        return;
    }
    lda8(rd8(ABSX(RIDER_SLOW)));
    and8(0x02);
    wr8(ABSX(RIDER_SLOW), (u8)A);
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, rd8(ABSY(0xB863)));
    if (!fc) { wr8(ABSX(RIDER_LEAN), inc8(rd8(ABSX(RIDER_LEAN)))); ret(); return; }
    lda8(rd8(ABSX(RIDER_INPUT)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    wr8(ABSX(RIDER_LEAN), inc8(rd8(ABSX(RIDER_LEAN))));
    lda8(0x0D); wr8(DPX(RIDER_LEAN_TIMER), 0x0D);
    lda8(rd8(ABSX(RIDER_LEAN)));
    cmp8((u8)A, 0x0D);
    if (!fc) { ret(); return; }
    lda8(0x01); wr8(ABSX(RIDER_BUMPED), 0x01);
    lda8(0x0A); wr8(DPX(RIDER_HOLD), 0x0A);
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
    lda8(rd8(ABSX(RIDER_INPUT)));
    and8(0x03);
    wr8(DP(0x0A), (u8)A);
    ldy8(0x00);
    lda8(rd8(ABSX(RIDER_COUNTDOWN)));
    ora8(rd8(ABSX(RIDER_BUMPED)));
    if (!fz) goto brake;
    lda8(rd8(ABSX(RIDER_TIMER)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) goto brake;
    }
    lda8((u8)X);
    if (fz) {
        ldy8(0x02);
        lda8(rd8(DP(RACE_END)));
        if (!fz) goto top;
        ldy8(0x00);
    }
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) {
        ldy8(0x04);
        lda8(rd8(DP(0x0A)));
        if (fz) goto brake;
        Y = (u8)A; nz8((u8)Y);
        goto harder;
    }
    lda8(rd8(ABSX(RIDER_INPUT)));
    and8(0xC0);
    if (fz) goto brake;
    wr8(DP(0x00), (u8)X);
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(RIDER_SPEED_LO)));
        ora8(rd8(ABSX(RIDER_SPEED_HI)));
        if (fz) {
            lda8(rd8(DP(P1_PRESS_HI)));
            and8(0xC0);
            if (!fz) { lda8(0x3F); push8(0x80); push16(0xAC21); QueueSound_m1x1(); }
        }
    }
    lda8(rd8(ABSX(RIDER_INPUT)));
    lda8(asl8((u8)A));
    if (!fc) wr8(DP(0x00), inc8(rd8(DP(0x00))));
    push16(0xAC2C); RiderOnRamp_m1x1();
    if (!fz) {
        lda8(rd8(ABSX(RIDER_JUMP)));
        if (fz) {
            lda8(rd8(ABSX(RIDER_SPEED_HI)));
            if (!fz) goto brake;
        }
    }
    Y = rd8(DP(0x00)); nz8((u8)Y);
    lda8((u8)X);
    if (!fz) goto accel;
top:
    lda8(rd8(ABSX(RIDER_SPEED_HI)));
    cmp8((u8)A, rd8(ABSY(0xB86A)));
    if (!fc) goto accel;
    if (!fz) goto harder;
    lda8(rd8(ABSX(RIDER_SPEED_LO)));
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
    lda8(rd8(ABSX(RIDER_BUMPED)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_AIR)));
    int pad = !fz;
    if (!pad) {
        lda8(rd8(ABSX(RIDER_SCRIPT)));
        ora8(rd8(DP(RACE_END)));
        if (fz) {
            lda8(rd8(ABSX(RIDER_SPEED_HI)));
            pad = !fz;
            if (!pad) {
                lda8(rd8(ABSX(RIDER_SPEED_LO)));
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
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_PIECE_LEAN)));
    cmp8((u8)A, rd8(ABSX(RIDER_LEAN)));
    if (fz) { ret(); return; }
    ldy8(0x05);
    lean_toward1();
}

void Reach20_m1x1(void);
void Reach2_m1x1(void);
void CoinCollected_m1x1(void);

/* JSL QueueSound from bank $80 with the return address `back`. */
static void sound80_1(u8 cmd, u16 back)
{
    lda8(cmd);
    push8(0x80); push16(back); QueueSound_m1x1();
}

/* Crash sound for rider r: $24, or $29 ($24 + 5) for a rival. */
static void crash_sound1(u8 r, u16 back)
{
    lda8(r);
    if (!fz) lda8(0x05);
    fc = 0; adc8(0x24);
    push8(0x80); push16(back); QueueSound_m1x1();
}

/* $80:C9C4 RiderCollisions (RTS): Stadium 1's version of the Stadium 4
 * routine: every pair of riders X < Y that are both racing ($01D8), on
 * the ground ($0198 bit 0), in the same lane band ($0958, non-zero), not
 * already bumped ($01C8; X also not in $0940) and close enough (height
 * $01B0 within $14 when either is in the air; X position $0190 within 2
 * for a bump of both or $14 for one): rider X alone falls back (crash
 * sound, $01C8,X = $FF, sound $3D for the player), or both bump: for the
 * player's bump with speed a coin is won (CoinCollected, $0CDF counts to
 * 5: sounds $2D and $01, $0CE0 = 1, $0990 = $20); rider Y gets $01C8 =
 * $FF, its voice ($80:CB19 by coins $1542 and rider) and the crash
 * sound. $5F = $10 when the player and rider 7 meet. */
void RiderCollisions_m1x1(void)
{
    ldx8(0x00);
    ldy8(0x01);
    for (;;) {
        /* L80C9C8 */
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (fz) goto next_x;
        for (;;) {
            /* L80C9D0 */
            lda8(rd8(ABSY(RIDER_ACTIVE)));
            if (fz) goto next_y;
            lda8(rd8(ABSX(RIDER_PAGE)));
            lda8(lsr8((u8)A));
            if (!fc) goto next_x;
            lda8(rd8(ABSY(RIDER_PAGE)));
            lda8(lsr8((u8)A));
            if (!fc) goto next_y;
            lda8(rd8(ABSX(RIDER_BAND)));
            if (fz) goto next_x;
            cmp8((u8)A, rd8(ABSY(RIDER_BAND)));
            if (!fz) goto next_y;
            lda8(rd8(ABSX(RIDER_BUMPED)));
            ora8(rd8(ABSX(OVERHEAT_LOCK)));
            if (!fz) goto next_x;
            lda8(rd8(ABSY(RIDER_BUMPED)));
            if (!fz) goto next_y;
            lda8(rd8(ABSX(RIDER_AIR)));
            if (fz) {
                lda8(rd8(ABSY(RIDER_AIR)));
                if (fz) goto side;
            }
            lda8(rd8(ABSX(RIDER_Y)));
            cmp8((u8)A, rd8(ABSY(RIDER_Y)));
            if (!fc) {
                push16(0xCA0C); Reach20_m1x1();
                cmp8((u8)A, rd8(ABSY(RIDER_Y)));
                if (fc) goto side;
                goto next_y;
            }
            lda8(rd8(ABSY(RIDER_Y)));
            push16(0xCA1A); Reach20_m1x1();
            cmp8((u8)A, rd8(ABSX(RIDER_Y)));
            if (!fc) goto next_y;
side:
            lda8(rd8(ABSX(RIDER_X)));
            cmp8((u8)A, rd8(ABSY(RIDER_X)));
            if (!fc) {
                push16(0xCA2A); Reach2_m1x1();
                cmp8((u8)A, rd8(ABSY(RIDER_X)));
                if (fc) goto bump_both;
                lda8(rd8(ABSX(RIDER_X)));
                push16(0xCA35); Reach20_m1x1();
                cmp8((u8)A, rd8(ABSY(RIDER_X)));
                if (fc) goto rear_bump;
                goto next_y;
            }
            lda8(rd8(ABSY(RIDER_X)));
            push16(0xCA42); Reach2_m1x1();
            cmp8((u8)A, rd8(ABSX(RIDER_X)));
            if (fc) goto bump_both;
            lda8(rd8(ABSY(RIDER_X)));
            push16(0xCA4D); Reach20_m1x1();
            cmp8((u8)A, rd8(ABSX(RIDER_X)));
            if (fc) goto meet;
            goto next_y;

rear_bump:
            crash_sound1((u8)X, 0xCA61);
            cmp8((u8)X, 0x00);
            if (fz) sound80_1(0x3D, 0xCA6B);
            lda8(0xFF);
            wr8(ABSX(RIDER_BUMPED), 0xFF);
            goto after_bump;

bump_both:
            crash_sound1((u8)X, 0xCA7F);
            lda8(0xFF);
            wr8(ABSX(RIDER_BUMPED), 0xFF);
meet:
            cmp8((u8)X, 0x00);
            if (fz) {
                cmp8((u8)Y, 0x07);
                if (fz) { lda8(0x10); wr8(DP(0x5F), 0x10); }
            }
            lda8(rd8(ABS(RIDER_BUMPED)));
            if (fz) {
                cmp8((u8)X, 0x00);
                if (fz) {
                    lda8(rd8(ABS(RIDER_SPEED_LO)));
                    ora8(rd8(ABS(RIDER_SPEED_HI)));
                    if (!fz) {
                        wr8(ABS(COIN_COUNT), inc8(rd8(ABS(COIN_COUNT))));
                        sound80_1(0x29, 0xCAAA);
                        push8((u8)Y);
                        push8((u8)X);
                        push8(0x80); push16(0xCAB0); CoinCollected_m1x1();
                        X = pull8(); nz8((u8)X);
                        Y = pull8(); nz8((u8)Y);
                        lda8(rd8(ABS(COIN_COUNT)));
                        cmp8((u8)A, 0x05);
                        if (fz) {
                            sound80_1(0x2D, 0xCAC8);
                            lda8(0x01);
                            wr8(ABS(TURBO_ON), 0x01);
                            push8(0x80); push16(0xCAD1); QueueSound_m1x1();
                            lda8(0x20);
                            wr8(ABS(ENGINE_HEAT), 0x20);
                        } else if (fc) {
                            lda8(0x06);
                            wr8(ABS(COIN_COUNT), 0x06);
                        }
                    }
                }
            }
            lda8(0xFF);
            wr8(ABSY(RIDER_BUMPED), 0xFF);
            push8((u8)Y);
            wr8(ABS(0x000F), (u8)Y);
            lda8(rd8(ABS(PLAYER_CHAR)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
            fc = 0; adc8(rd8(DP(0x0F)));
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0xCB19)));
            if (!fz) { push8(0x80); push16(0xCAF2); QueueSound_m1x1(); }
            Y = pull8(); nz8((u8)Y);
            crash_sound1((u8)Y, 0xCAFF);
after_bump:
            cmp8((u8)X, 0x00);
            if (fz) lda8(0x01);
next_y:
            Y = inc8((u8)Y);
            cmp8((u8)Y, 0x08);
            if (fz) break;
        }
next_x:
        X = inc8((u8)X);
        lda8((u8)X);
        Y = (u8)A; nz8((u8)Y);
        Y = inc8((u8)Y);
        cmp8((u8)X, 0x07);
        if (fz) { ret(); return; }
    }
}

void BlankQueueRowPal2_m1x1(void);
void CopyTrackColumn_m1x1(void);
void FillLaneColumn_m1x1(void);

/* $81:8A42 (RTL, X = row type): the row's piece list ($05) from
 * $81:8010 / $81:8040 (DB = $81). */
void RaceRowList_m1x1(void)
{
    lda8(rd8(ABSX(0x8010))); wr8(DP(0x05), (u8)A);
    lda8(rd8(ABSX(0x8040))); wr8(DP(0x06), (u8)A);
    retl();
}

/* $81:8A4D (RTL): load the race course one column ahead (DB = $81). The
 * expanded course $7F:4000 ([$07]) holds the lap count at entry 0 (doubled
 * in a race, $7D clear; $0928 + 1, $A6 cleared) and then row entries: with
 * a $81:8070 bit a row type from $81:8A30 by the low nibble, else the low
 * six bits; types $30 and up set the finish/variant flag $092C. Each time
 * the builder column $0273 reaches the player column $0260 the row list
 * ($81:8A42) gives the next column: blanked, its entry recorded at
 * $7F:6000 + $A6, filled (CopyTrackColumn), on the finish row ($09) the
 * lap number ($81:8A3D by $0929, then $0928) at $7F:0026, and queued for
 * VRAM at $0271 - $20 (FillLaneColumn; $0271 wraps at $20 with $0272 ^ 4).
 * At the end of a finish row the next lap starts ($03AF cleared, $0928 +
 * 1, $093F - 1, $098A = 1 on the last; the course restarts, $A8 = $A6). */
void LoadRaceColumn_m1x1(void)
{
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(0x00); wr8(DP(0x07), 0x00);
    lda8(0x40); wr8(DP(0x08), 0x40);
    lda8(0x7F); wr8(DP(0x09), 0x7F);
    lda8(rd8(ABS(BUILD_COL)));
    cmp8((u8)A, rd8(ABS(RIDER_COLUMN)));
    if (!fz) goto out;
    lda8(rd8(ABS(ROW_OFFSET)));
    if (!fz) goto column;
next_entry:
    WAIT_TICK();
    rep(0x10);
    Y = rd16(ABS(TRACK_READ_POS)); nz16(Y);
    if (fz) {
        lda8(rd8(ILDPY(0x07)));
        wr8(ABS(LAPS_LEFT), (u8)A);
        wr8(ABS(LAP), inc8(rd8(ABS(LAP))));
        wr8(DP(COURSE_COLUMN), 0);
        wr8(DP(0xA7), 0);
        lda8(rd8(DP(FINAL_RACE)));
        if (fz) wr8(ABS(LAPS_LEFT), asl8(rd8(ABS(LAPS_LEFT))));
    }
read_entry:
    rep(0x20);
    wr16(ABS(TRACK_READ_POS), inc16(rd16(ABS(TRACK_READ_POS))));
    sep(0x20);
    if (fx) { Y = rd8(ABS(TRACK_READ_POS)); nz8((u8)Y); }
    else { Y = rd16(ABS(TRACK_READ_POS)); nz16(Y); }
    lda8(rd8(ILDPY(0x07)));
    if (fz) goto out;
    sep(0x10);
    bit8(rd8(ABS(0x8070)), 0);
    if (!fz) {
        and8(0x0F);
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x8A30)));
        wr8(ABS(ROW_TYPE), (u8)A);
    } else {
        sep(0x10);
        ldx8(0x01);
        and8(0x3F);
        wr8(ABS(ROW_TYPE), (u8)A);
    }
    cmp8((u8)A, 0x30);
    if (fc) {
        int dex = 1;
        if (fz) {
            lda8(rd8(ABS(0x098A)));
            dex = !fz;
        }
        if (dex) X = dec8((u8)X);
        wr8(ABS(BALL_DUE), (u8)X);
        if (!fn) goto next_entry;
    }
column:
    X = rd8(ABS(ROW_TYPE)); nz8((u8)X);
    push8(0x81); push16(0x8AD0); RaceRowList_m1x1();
    Y = rd8(ABS(ROW_OFFSET)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
    if (fz) {
        wr8(ABS(ROW_OFFSET), (u8)A);
        lda8(rd8(ABS(ROW_TYPE)));
        cmp8((u8)A, 0x09);
        if (!fz) goto next_entry;
        wr8(ABS(FINISH_CROSSED), 0);
        wr8(ABS(LAP), inc8(rd8(ABS(LAP))));
        wr8(ABS(LAPS_LEFT), dec8(rd8(ABS(LAPS_LEFT))));
        if (fz) { lda8(0x01); wr8(ABS(0x098A), 0x01); }
        lda8(0x00);
        wr8(ABS(TRACK_READ_POS), 0);
        wr8(ABS(0x0276), 0);
        wr8(ABS(LAP_SIGN), 0);
        rep(0x20);
        lda16(rd16(DP(COURSE_COLUMN)));
        wr16(DP(0xA8), A);
        wr16(DP(COURSE_COLUMN), 0);
        sep(0x20);
        goto read_entry;
    }
    push8(0x81); push16(0x8ADE); BlankQueueRowPal2_m1x1();
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
    push8((u8)A);
    rep(0x30);
    X = rd16(DP(COURSE_COLUMN)); nz16(X);
    lda16(rd16(ABS(TRACK_READ_POS)));
    wr16(LONGX(TRACK_COL_INDEX), A);
    wr16(DP(COURSE_COLUMN), inc16(rd16(DP(COURSE_COLUMN))));
    wr16(DP(COURSE_COLUMN), inc16(rd16(DP(COURSE_COLUMN))));
    sep(0x30);
    X = pull8(); nz8((u8)X);
    push8(0x81); push16(0x8AF7); CopyTrackColumn_m1x1();
    Y = inc8((u8)Y);
    Y = inc8((u8)Y);
    wr8(ABS(ROW_OFFSET), (u8)Y);
    lda8(rd8(ABS(ROW_TYPE)));
    cmp8((u8)A, 0x09);
    if (fz) {
        lda8(rd8(ABS(LAPS_LEFT)));
        cmp8((u8)A, 0x01);
        if (!fz) {
            cmp8((u8)Y, 0x4A);
            if (fc) {
                cmp8((u8)Y, 0xB0);
                if (!fc) {
                    X = rd8(ABS(LAP_SIGN)); nz8((u8)X);
                    lda8(rd8(ABSX(0x8A3D)));
                    wr8(LONG(VQ0_DATA + 0x18), (u8)A);
                    wr8(ABS(LAP_SIGN), inc8(rd8(ABS(LAP_SIGN))));
                    cmp8((u8)X, 0x05);
                    if (fz) {
                        lda8(rd8(ABS(LAP)));
                        wr8(LONG(VQ0_DATA + 0x18), (u8)A);
                    }
                }
            }
        }
    }
    lda8(0x81); wr8(LONG(VQ0_VMAIN), 0x81);
    lda8(0x01); wr8(LONG(VQ0_DMAP), 0x01);
    lda8(0x18); wr8(LONG(VQ0_BBAD), 0x18);
    lda8(0x02);
    fc = 0; adc8(0x0C);
    wr8(LONG(VQ0_SRC), (u8)A);
    lda8(0x00); wr8(LONG(VQ0_SRC_HI), 0);
    lda8(0x7F); wr8(LONG(VQ0_SRC_BANK), 0x7F);
    rep(0x20);
    lda16(0x0002);
    fc = 0; adc16(0x0028);
    wr16(LONG(VQ_END), A);
    wr16(LONG(VQ0_NEXT), A);
    lda16(rd16(ABS(TRACK_VRAM_COL)));
    fc = 1; sbc16(0x0020);
    wr16(LONG(VQ0_VMADD), A);
    lda16(0x001C); wr16(LONG(VQ0_SIZE), 0x001C);
    sep(0x20);
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    wr8(DP(0x0F), (u8)A);
    push8(0x81); push16(0x8B7F); FillLaneColumn_m1x1();
    lda8(rd8(ABS(TRACK_ROW_PTR)));
    push8(0x81); push16(0x8B86); NextColumnLong_m1x1();
    wr8(ABS(TRACK_ROW_PTR), (u8)A);
    wr8(ABS(TRACK_VRAM_COL), inc8(rd8(ABS(TRACK_VRAM_COL))));
    lda8(rd8(ABS(TRACK_VRAM_COL)));
    cmp8((u8)A, 0x20);
    if (fz) {
        lda8(0x00); wr8(ABS(TRACK_VRAM_COL), 0);
        lda8(rd8(ABS(TRACK_VRAM_HALF)));
        eor8(0x04);
        wr8(ABS(TRACK_VRAM_HALF), (u8)A);
    }
    lda8(rd8(ABS(RIDER_COLUMN)));
    push8(0x81); push16(0x8BA7); NextColumnLong_m1x1();
    wr8(ABS(BUILD_COL), (u8)A);
    DB = pull8(); nz8(DB);
    retl();
    return;
out:
    sep(0x10);
    DB = pull8(); nz8(DB);
    retl();
}

void VramQueueBlock_m1x1(void);

/* $80:A5AC (RTS): the character name block ($80:A5F9 + 29 * $1542) to
 * VRAM $1C2D. */
void DrawRacerNameLow_m1x1(void)
{
    rep(0x20);
    lda16(0x1C2D); wr16(DP(0x00), 0x1C2D);
    sep(0x20);
    lda8(rd8(ABS(PLAYER_CHAR)));
    wr8(ABS(WRMPYA), (u8)A);
    lda8(0x1D);
    wr8(ABS(WRMPYB), 0x1D);
    rep(0x20);
    A = rd16(ABS(RDMPYL));
    fc = 0; adc16(0xA5F9);
    wr16(DP(0x02), A);
    sep(0x20);
    VramQueueBlock_m1x1();
}

void RiderShadowY_m1x1(void);
void AnimateWheels_m1x1(void);
void TemperatureGauge_m1x1(void);

/* $80:ABA0 (RTS, X = last rider): riders X-0 that are racing ($01D8) get
 * their throttle; a crashed, held ($0940) or otherwise busy one (state
 * $01D0 not 5) has its pad $0178 cleared first. Then, unpaused ($54) with
 * the player holding A/B and $092D, $84 clear, $092D = 1. */
void ThrottleFrom_m1x1(void)
{
    do {
        lda8(rd8(ABSX(RIDER_ACTIVE)));
        if (!fz) {
            lda8(rd8(ABSX(RIDER_TIMER)));
            cmp8((u8)A, 0x05);
            if (!fz) {
                ora8(rd8(ABSX(RIDER_BUMPED)));
                ora8(rd8(ABSX(OVERHEAT_LOCK)));
                if (!fz) { lda8(0x00); wr8(ABSX(RIDER_INPUT), 0); }
            }
            push16(0xABBB); RiderThrottle_m1x1();
        }
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(GAUGE_ON)));
    ora8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    lda8(0x01); wr8(ABS(GAUGE_ON), 0x01);
    ret();
}

/* $80:AB9A (RTS): while racing ($8E), ThrottleFrom for all eight riders
 * (else only its closing check). */
void ThrottleAll_m1x1(void)
{
    lda8(rd8(DP(RACE_STARTED)));
    if (!fz) {
        ldx8(0x07);
        ThrottleFrom_m1x1();
        return;
    }
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(GAUGE_ON)));
    ora8(rd8(DP(RACE_END)));
    if (!fz) { ret(); return; }
    lda8(0x01); wr8(ABS(GAUGE_ON), 0x01);
    ret();
}

/* $80:BE2E (RTS, X = rider): the rider's state step $01D0 through the
 * table $80:BE07 / $80:BE0D. */
void RiderStateStep_m1x1(void)
{
    Y = rd8(ABSX(RIDER_TIMER)); nz8((u8)Y);
    lda8(rd8(ABSY(0xBE07))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xBE0D))); wr8(DP(0x01), (u8)A);
    if (!r816_call_pc(0x800000u | rd16(LONG(0x000000))))
        r816_unknown_target(0x80BE3B, rd16(LONG(0x000000)));
}

/* $80:BE22 (RTS): for riders 7-0 the state step and the shadow height. */
void RiderStepAll_m1x1(void)
{
    ldx8(0x07);
    do {
        push16(0xBE26); RiderStateStep_m1x1();
        push16(0xBE29); RiderShadowY_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:E773 (RTS): AnimateWheels for all eight riders. */
void AnimateAllWheels_m1x1(void)
{
    ldx8(0x07);
    AnimateWheels_m1x1();
}

/* $80:E7A4 (RTS): the piece scripts of riders 7-1, then the player's. */
void RunAllRiderPieces_m1x1(void)
{
    ldx8(0x07);
    do {
        push16(0xE7A8); RunRiderPieces_m1x1();
        X = dec8((u8)X);
    } while (!fz);
    RunRiderPieces_m1x1();
}

/* $80:EB80 (RTS): for riders 7-0 with a pending piece action $0220, run
 * it (PieceAction). */
void RunPieceActions_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8(rd8(ABSX(RIDER_PIECE_ACTION)));
        if (!fz) { push16(0xEB89); PieceAction_m1x1(); }
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:E374 (RTS): TemperatureGauge. */
void TemperatureGaugeJmp_m1x1(void)
{
    TemperatureGauge_m1x1();
}

void SendSpeedToApu_m1x1(void);
void SetRiderLean_m1x1(void);
void RiderLaneBand_m1x1(void);
void SegmentTimer_m1x1(void);
void RaceClockStep_m1x1(void);
void FinishStep_m1x1(void);
void SpawnRivals_m1x1(void);
void UpdatePlayerPlace_m1x1(void);
void LaunchBall_m1x1(void);
void RaceTimer_m1x1(void);
void ItemFlashState_m1x1(void);
void CrashRecovery_m1x1(void);
void UpdateLanes_m1x1(void);
void DrawSparksAndMarkers_m1x1(void);
void BallOrGauge_m1x1(void);
void ClearMidScreenRiders_m1x1(void);
void RaceMoveStep_m1x1(void);
void RaceStartLights_m1x1(void);
void RaceBannerSprites_m1x1(void);
void CpuRidersStep_m1x1(void);
void DrawPlace_m1x1(void);
void DrawLap_m1x1(void);
void EraseCollectedCoin_m1x1(void);
void RedrawUsedItem_m1x1(void);
void SpawnExhaust_m1x1(void);
void EngineSound_m1x1(void);
void DemoInputStep_m1x1(void);
void DemoPadDisplay_m1x1(void);

/* $80:A9BF / $80:E32C: lean and lane band for riders 7-0. */
static void lean_and_bands1(u16 back1, u16 back2)
{
    ldx8(0x07);
    do {
        push16(back1); SetRiderLean_m1x1();
        push16(back2); RiderLaneBand_m1x1();
        X = dec8((u8)X);
    } while (!fn);
}

/* $80:A9BA Mode03_Race (RTS): one race frame. Engine pitch; leans and
 * lane bands; the player's pad (unless the attract demo, $0CD7); rider
 * state steps, piece scripts and actions. After the finish ($84) the
 * segment timer and clock; else the finish check, a free run's rivals,
 * place and ball, the race timer, item flash and rider collisions. Then
 * crash recovery, throttles, lanes, ground heights, sparks, the course
 * column, ball or gauge, the riders sorted and drawn, the move step,
 * start lights, banners, gauge, wheels, CPU riders, place; at a lap
 * ($A4) the course time and lap (and after the finish the label
 * $80:AA6D at $1891); coin and item redraws, markers, exhaust, the
 * sprite slots and engine sound. */
void Mode03_Race_m1x1(void)
{
    push16(0xA9BC); SendSpeedToApu_m1x1();
    lean_and_bands1(0xA9C1, 0xA9C4);
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (fz) { lda8(rd8(DP(P1_HELD_HI))); wr8(ABS(RIDER_INPUT), (u8)A); }
    push16(0xA9D4); RiderStepAll_m1x1();
    push16(0xA9D7); RunAllRiderPieces_m1x1();
    push16(0xA9DA); RunPieceActions_m1x1();
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        push16(0xA9E1); SegmentTimer_m1x1();
        push16(0xA9E4); RaceClockStep_m1x1();
    } else {
        push16(0xA9EA); FinishStep_m1x1();
        lda8(rd8(DP(FINAL_RACE)));
        if (!fz) {
            push16(0xA9F1); SpawnRivals_m1x1();
            push16(0xA9F4); UpdatePlayerPlace_m1x1();
            push16(0xA9F7); LaunchBall_m1x1();
        }
        push16(0xA9FA); RaceTimer_m1x1();
        push16(0xA9FD); ItemFlashState_m1x1();
        push16(0xAA00); RiderCollisions_m1x1();
    }
    push16(0xAA03); CrashRecovery_m1x1();
    push16(0xAA06); ThrottleAll_m1x1();
    push16(0xAA09); UpdateLanes_m1x1();
    push16(0xAA0C); RiderGroundHeights_m1x1();
    push16(0xAA0F); DrawSparksAndMarkers_m1x1();
    push8(0x80); push16(0xAA13); LoadRaceColumn_m1x1();
    push16(0xAA16); BallOrGauge_m1x1();
    push16(0xAA19); SortAndDrawRiders_m1x1();
    push16(0xAA1C); ClearMidScreenRiders_m1x1();
    push16(0xAA1F); RaceMoveStep_m1x1();
    push16(0xAA22); RaceStartLights_m1x1();
    push16(0xAA25); RaceBannerSprites_m1x1();
    push16(0xAA28); TemperatureGauge_m1x1();
    push16(0xAA2B); AnimateAllWheels_m1x1();
    push16(0xAA2E); CpuRidersStep_m1x1();
    push16(0xAA31); DrawPlace_m1x1();
    lda8(rd8(DP(0xA4)));
    if (!fz) {
        wr8(DP(0xA4), 0);
        push8(0x80); push16(0xAA3B); RecordCourseTime_m1x1();
        push16(0xAA3E); DrawLap_m1x1();
        lda8(rd8(DP(RACE_END)));
        if (!fz) {
            rep(0x20);
            lda16(0x1891); wr16(DP(0x00), 0x1891);
            lda16(0xAA6D); wr16(DP(0x02), 0xAA6D);
            sep(0x20);
            push16(0xAA53); VramQueueTiles20_m1x1();
        }
    }
    push8(0x80); push16(0xAA57); EraseCollectedCoin_m1x1();
    push8(0x80); push16(0xAA5B); RedrawUsedItem_m1x1();
    push8(0x80); push16(0xAA5F); MarkerObjects_m1x1();
    push8(0x80); push16(0xAA63); SpawnExhaust_m1x1();
    push8(0x80); push16(0xAA67); StepSpriteSlots_m1x1();
    push8(0x80); push16(0xAA6B); EngineSound_m1x1();
    ret();
}

/* $80:E319 Mode0C (RTS): one attract-demo frame: engine pitch, the CPU
 * pads $2C-$33 released, the demo input (carry set: skip to the end
 * steps); else the race frame's rider steps without the player's pad,
 * timers or rewards. Then start lights, gauge, wheels, CPU riders, the
 * pad display and the sprite slots. */
void Mode0C_m1x1(void)
{
    push16(0xE31B); SendSpeedToApu_m1x1();
    ldx8(0x07);
    lda8(0xFF);
    do {
        wr8(DPX(RANDOM_LFSR), 0xFF);
        X = dec8((u8)X);
    } while (!fn);
    push16(0xE327); DemoInputStep_m1x1();
    if (!fc) {
        lean_and_bands1(0xE32E, 0xE331);
        push16(0xE337); RiderStepAll_m1x1();
        push16(0xE33A); RunAllRiderPieces_m1x1();
        push16(0xE33D); RunPieceActions_m1x1();
        push16(0xE340); FinishStep_m1x1();
        push16(0xE343); RiderCollisions_m1x1();
        push16(0xE346); CrashRecovery_m1x1();
        push16(0xE349); ThrottleAll_m1x1();
        push16(0xE34C); UpdateLanes_m1x1();
        push16(0xE34F); RiderGroundHeights_m1x1();
        push16(0xE352); DrawSparksAndMarkers_m1x1();
        push8(0x80); push16(0xE356); LoadRaceColumn_m1x1();
        push16(0xE359); SortAndDrawRiders_m1x1();
        push16(0xE35C); ClearMidScreenRiders_m1x1();
        push16(0xE35F); RaceMoveStep_m1x1();
    }
    push16(0xE362); RaceStartLights_m1x1();
    push16(0xE365); TemperatureGaugeJmp_m1x1();
    push16(0xE368); AnimateAllWheels_m1x1();
    push16(0xE36B); CpuRidersStep_m1x1();
    push16(0xE36E); DemoPadDisplay_m1x1();
    push8(0x80); push16(0xE372); StepSpriteSlots_m1x1();
    ret();
}

void SetHdmaPreset_m1x1(void);
void ComputePlaces_m1x1(void);
void SpawnObject_m1x1(void);
void TickRaceClock_m1x1(void);
void DrawText95C4_m1x1(void);
void QueueScoreDigitsLong_m1x1(void);
void DrawRacerNameLow_m1x1(void);

/* One VRAM DMA through DmaKick (DP = $4300): $0800 bytes from bank X
 * offset A to the word address vaddr. */
static void results_dma1(u16 vaddr, u16 src, u8 bank, u16 back)
{
    lda16(vaddr); wr16(ABS(VMADDL), vaddr);
    lda16(0x0800); wr16(DP(0x05), 0x0800);
    lda16(src);
    ldx8(bank);
    ldy8(0x01);
    push16(back); DmaKick_m0x1();
}

/* Spawn object type Y at ($00, $02). */
static void results_spawn1(u16 x, u16 y, u8 type, u16 back)
{
    rep(0x20);
    lda16(x); wr16(DP(0x00), x);
    lda16(y); wr16(DP(0x02), y);
    sep(0x20);
    ldy8(type);
    push8(0x80); push16(back); SpawnObject_m1x1();
}

/* Find the rider slot (X, even, from 14 down) whose place $086E is p. */
static void find_place1(u8 p)
{
    ldx8(0x0E);
    do {
        lda8(rd8(ABSX(PLAYER_PLACE)));
        cmp8((u8)A, p);
        if (fz) return;
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
}

/* The podium rider for slot X: object $80:9638 + 16 * $1542 + X. */
static void podium_rider1(u16 x, u16 y, u16 back)
{
    rep(0x20);
    lda16(x); wr16(DP(0x00), x);
    lda16(y); wr16(DP(0x02), y);
    sep(0x20);
    lda8(rd8(ABS(PLAYER_CHAR)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(DP(0x04), (u8)A);
    lda8((u8)X);
    fc = 0; adc8(rd8(DP(0x04)));
    X = (u8)A; nz8((u8)X);
    Y = rd8(ABSX(0x9638)); nz8((u8)Y);
    push8(0x80); push16(back); SpawnObject_m1x1();
}

/* $80:92A0 (RTS): after a race the record $083A-$083C is kept for the
 * course ($083D + 3 * $7C). In a race ($7D clear), with NMI off, the
 * tilemaps cleared and three $0800-byte VRAM loads ($89:B000 to $0000,
 * $88:D000 to $0800, $89:B800 to $1800), the course label ($80:938C +
 * 5 * $7C) at $1893, NMI back, $88 = $40, $0452 cleared, $9E = $FC, $9C
 * = 4 and HDMA preset 0. In a free run, HDMA preset 0, sound $03 and the
 * results screen with NMI off: places, the podium objects (the flag, the
 * first three riders from $80:9638 by $1542, the trophy or the plain
 * stand for places below 4 with palette $80:95D9, the cup), the objects
 * updated, the time $083A copied to $03B1 and shown at $1C74, the place
 * badge at $1CAC (else the text at $1CA4), for places 1-7 the place
 * points ($80:95DD / $95E5, bonus $B2 from $95ED) at $1CB6, the score
 * digits at $1CF5, the racer name; then NMI back on. */
void RaceResultsSetup_m1x1(void)
{
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(COURSE_SLOT)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(RECORD_TIME))); wr8(ABSX(COURSE_BEST), (u8)A);
    lda8(rd8(ABS(RECORD_TIME + 0x01))); wr8(ABSX(COURSE_BEST + 0x01), (u8)A);
    lda8(rd8(ABS(RECORD_TIME + 0x02))); wr8(ABSX(COURSE_BEST + 0x02), (u8)A);
    lda8(rd8(DP(FINAL_RACE)));
    if (fz) {
        wr8(ABS(NMITIMEN), 0);
        push8(0x80); push16(0x92FF); ClearTilemaps_m1x1();
        rep(0x20);
        push16(D);
        lda16(0x4300);
        D = A;
        ldy8(0x80); wr8(ABS(VMAIN), 0x80);
        lda16(0x1801); wr16(DP(0x00), 0x1801);
        results_dma1(0x0000, 0xB000, 0x89, 0x9325);
        results_dma1(0x0800, 0xD000, 0x88, 0x933A);
        results_dma1(0x1800, 0xB800, 0x89, 0x934F);
        D = pull16(); nz16(D);
        sep(0x20);
        lda8(rd8(DP(COURSE_SLOT)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(COURSE_SLOT)));
        rep(0x20);
        and16(0x00FF);
        fc = 0; adc16(0x938C);
        wr16(DP(0x02), A);
        lda16(0x1893); wr16(DP(0x00), 0x1893);
        sep(0x20);
        push16(0x936E); VramQueueTiles20_m1x1();
        lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
        lda8(0x40); wr8(ABS(MODE_TIMER), 0x40);
        wr8(ABS(ENTRY_CURSOR), 0);
        lda8(0xFC); wr8(DP(PIC_SCROLL), 0xFC);
        lda8(0x04); wr8(DP(0x9C), 0x04);
        lda8(0x00);
        push8(0x80); push16(0x938A); SetHdmaPreset_m1x1();
        ret();
        return;
    }
    lda8(0x00);
    push8(0x80); push16(0x92EF); SetHdmaPreset_m1x1();
    lda8(0x03);
    push8(0x80); push16(0x92F5); QueueSound_m1x1();
    wr8(ABS(NMITIMEN), 0);
    push16(0x93A5); ComputePlaces_m1x1();
    results_spawn1(0xFFC0, 0x0018, 0x00, 0x93B9);
    results_spawn1(0x0078, 0x00C0, 0x09, 0x93CD);
    X = rd8(DP(0x0E)); nz8((u8)X);
    lda8(0x02); wr8(ABSX(OBJ_STATE), 0x02);
    find_place1(0x01);
    podium_rider1(0x0080, 0x008B, 0x9404);
    find_place1(0x02);
    podium_rider1(0x0054, 0x00B0, 0x9434);
    lda8(rd8(ABS(PLAYER_PLACE)));
    cmp8((u8)A, 0x04);
    if (fc) {
        rep(0x20);
        lda16(0x00E8); wr16(DP(0x00), 0x00E8);
        lda16(0x00D8); wr16(DP(0x02), 0x00D8);
        sep(0x20);
        Y = rd8(ABS(PLAYER_CHAR)); nz8((u8)Y);
        Y = inc8((u8)Y);
        push8(0x80); push16(0x9451); SpawnObject_m1x1();
        X = rd8(DP(0x0E)); nz8((u8)X);
        lda8(0x02); wr8(ABSX(OBJ_STATE), 0x02);
        Y = rd8(ABS(PLAYER_CHAR)); nz8((u8)Y);
        lda8(rd8(ABSY(0x95D9))); wr8(ABSX(OBJ_FRAME), (u8)A);
    } else {
        results_spawn1(0x0078, 0x00C0, 0x09, 0x9477);
        X = rd8(DP(0x0E)); nz8((u8)X);
        lda8(0x02); wr8(ABSX(OBJ_STATE), 0x02);
    }
    find_place1(0x03);
    podium_rider1(0x00B8, 0x00B8, 0x94AE);
    results_spawn1(0x00A0, 0x008B, 0x07, 0x94C2);
    push8(0x80); push16(0x94C6); UpdateObjects_m1x1();
    wr8(ABS(SPRITES_FROZEN), inc8(rd8(ABS(SPRITES_FROZEN))));
    lda8(rd8(ABS(RECORD_TIME))); wr8(ABS(RACE_MIN), (u8)A);
    lda8(rd8(ABS(RECORD_TIME + 0x01))); wr8(ABS(RACE_SEC), (u8)A);
    lda8(rd8(ABS(RECORD_TIME + 0x02))); wr8(ABS(RACE_FRAMES), (u8)A);
    rep(0x20);
    lda16(0x1C74); wr16(DP(0x00), 0x1C74);
    sep(0x20);
    push16(0x94E7); TickRaceClock_m1x1();
    lda8(rd8(ABS(RIDER_DIST)));
    ora8(rd8(ABS(RIDER_DIST + 0x01)));
    if (fz) {
        push16(0x94F2); DrawText95C4_m1x1();
    } else {
        lda8(0x22); wr8(ABS(DIGIT_BUF), 0x22);
        lda8(rd8(ABS(PLAYER_PLACE)));
        lda8(dec8((u8)A));
        lda8(asl8((u8)A));
        ora8(0x30);
        wr8(ABS(DIGIT_ROW), (u8)A);
        lda8(inc8((u8)A));
        wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
        fc = 0; adc8(0x0F);
        wr8(ABS(DIGIT_ROW + 0x02), (u8)A);
        lda8(inc8((u8)A));
        wr8(ABS(DIGIT_ROW + 0x03), (u8)A);
        rep(0x20);
        lda16(0x1CAC); wr16(DP(0x00), 0x1CAC);
        lda16(0x03D2); wr16(DP(0x02), 0x03D2);
        sep(0x20);
        lda8(0x24); wr8(DP(0x0F), 0x24);
        push16(0x9526); VramQueueTiles_m1x1();
    }
    X = rd8(ABS(PLAYER_PLACE)); nz8((u8)X);
    X = dec8((u8)X);
    cmp8((u8)X, 0x07);
    if (!fc) {
        lda8(rd8(ABSX(0x95DD)));
        ora8(0x50);
        wr8(ABS(DIGIT_ROW), (u8)A);
        fc = 0; adc8(0x10);
        wr8(ABS(DIGIT_ROW + 0x05), (u8)A);
        lda8(rd8(ABSX(0x95E5)));
        ora8(0x50);
        wr8(ABS(DIGIT_ROW + 0x01), (u8)A);
        fc = 0; adc8(0x10);
        wr8(ABS(DIGIT_ROW + 0x06), (u8)A);
        lda8(rd8(ABSX(0x95ED))); wr8(DP(PLACE_BONUS), (u8)A);
        lda8(0x25); wr8(ABS(DIGIT_BUF), 0x25);
        lda8(0x50);
        wr8(ABS(DIGIT_ROW + 0x02), 0x50); wr8(ABS(DIGIT_ROW + 0x03), 0x50); wr8(ABS(DIGIT_ROW + 0x04), 0x50);
        fc = 0; adc8(0x10);
        wr8(ABS(DIGIT_ROW + 0x07), (u8)A); wr8(ABS(DIGIT_ROW + 0x08), (u8)A); wr8(ABS(DIGIT_ROW + 0x09), (u8)A);
        lda8(rd8(ABSX(0x95DD)));
        if (fz) {
            lda8(0xFC);
            wr8(ABS(DIGIT_ROW), 0xFC); wr8(ABS(DIGIT_ROW + 0x05), 0xFC);
        }
        rep(0x20);
        lda16(0x1CB6); wr16(DP(0x00), 0x1CB6);
        lda16(0x03D2); wr16(DP(0x02), 0x03D2);
        sep(0x20);
        lda8(0x28); wr8(DP(0x0F), 0x28);
        push16(0x958D); VramQueueTiles_m1x1();
    }
    rep(0x20);
    lda16(0x1CF5); wr16(DP(0x00), 0x1CF5);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x80); push16(0x95A3); QueueScoreDigitsLong_m1x1();
    push16(0x95A6); DrawRacerNameLow_m1x1();
    lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
    ret();
}

void DmaCopyToWram_m0x1(void);

/* $80:B8D8 / $80:B8F5: one table copy (length already in $00) from
 * $88:(base + 2 * $100 * table[$76]) to WRAM $00:dst through
 * DmaCopyToWram. */
static void scene_table_copy1(u16 table, u16 base, u16 dst, u16 back)
{
    Y = rd8(DP(GAME_MODE)); nz8((u8)Y);
    lda16(rd16(ABSY(table)));
    and16(0x00FF);
    A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);       /* XBA */
    lda16(asl16(A));
    fc = 0; adc16(base);
    wr16(DP(0x02), A);
    ldy8(0x88); wr8(DP(0x04), 0x88);
    lda16(dst);
    ldy8(0x00);
    push8(0x80); push16(back); DmaCopyToWram_m0x1();
}

/* $80:B873 (RTS): load scene $76 with NMI and HDMA off: $1556 from
 * $80:B940, the tilemaps cleared, then the VRAM DMA list $80:B9D8 (7-byte
 * entries: VRAM address, length, source, bank; $FFFF ends) chosen by
 * $80:B9B2, two $0100-byte tables from bank $88 ($80:B98C / $B99F) to
 * $1C00 / $1D00, $1915 + 1, $190F and $1E0C from $80:B979 / $B966, $1B00-
 * $1B0F and $1B20-$1B2F cleared, NMI back on; X and Y kept. */
void LoadScene_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    X = rd8(DP(GAME_MODE)); nz8((u8)X);
    lda8(rd8(ABSX(0xB940))); wr8(ABS(CLOCK_ON), (u8)A);
    wr8(ABS(NMITIMEN), 0);
    wr8(ABS(SH_HDMAEN), 0);
    wr8(ABS(HDMAEN), 0);
    push8(0x80); push16(0xB889); ClearTilemaps_m1x1();
    rep(0x30);
    push16(D);
    lda16(0x4300);
    D = A;
    Y = 0x0080; nz16(Y);
    wr16(ABS(VMAIN), 0x0080);
    lda16(0x1801); wr16(DP(0x00), 0x1801);
    lda16(rd16(ABS(GAME_MODE)));
    and16(0x00FF);
    lda16(asl16(A));
    X = A; nz16(X);
    Y = rd16(ABSX(0xB9B2)); nz16(Y);
    for (;;) {
        WAIT_TICK();
        lda16(rd16(ABSY(0xB9D8)));
        cmp16(A, 0xFFFF);
        if (fz) break;
        wr16(ABS(VMADDL), A);
        lda16(rd16(ABSY(0xB9DA))); wr16(DP(0x05), A);
        lda16(rd16(ABSY(0xB9DC)));
        X = rd16(ABSY(0xB9DE)); nz16(X);
        push16(Y);
        sep(0x10);
        ldy8(0x01);
        push16(0xB8C4); DmaKick_m0x1();
        rep(0x10);
        Y = pull16(); nz16(Y);
        lda16(Y);
        fc = 0; adc16(0x0007);
        Y = A; nz16(Y);
    }
    sep(0x10);
    D = pull16(); nz16(D);
    lda16(0x0100); wr16(DP(0x00), 0x0100);
    scene_table_copy1(0xB98C, 0xA000, 0x1C00, 0xB8F4);
    scene_table_copy1(0xB99F, 0xA100, 0x1D00, 0xB911);
    wr16(ABS(PALETTE_DIRTY), inc16(rd16(ABS(PALETTE_DIRTY))));
    sep(0x20);
    Y = rd8(DP(GAME_MODE)); nz8((u8)Y);
    lda8(rd8(ABSY(0xB953)));
    lda8(rd8(ABSY(0xB979))); wr8(ABS(SH_HDMAEN), (u8)A);
    lda8(rd8(ABSY(0xB966))); wr8(ABS(ANIM_UPLOAD_ON), (u8)A);
    ldx8(0x0F);
    do {
        wr8(ABSX(0x1B00), 0);
        wr8(ABSX(0x1B20), 0);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    ret();
}

/* $80:A3AC (RTS, M=1 X=0): the 48-bit number $0000/$0002/$0004 made from
 * the 11 decimal digits at [$1C] (acc = acc * 10 + digit, the last digit
 * only added); P and DB kept. */
void DigitsToNumber_m1x0(void)
{
    push8(getP());
    push8(DB);
    push8(0x80); DB = pull8(); nz8(DB);
    sep(0x10);
    rep(0x20);
    wr16(ABS(0x0000), 0);
    wr16(ABS(0x0002), 0);
    wr16(ABS(0x0004), 0);
    ldy8(0x00);
    for (;;) {
        lda16(rd16(ILDPY(0x1C)));
        and16(0x00FF);
        fc = 0; adc16(rd16(ABS(0x0000)));
        wr16(ABS(0x0000), A);
        if (fc) {
            u16 v = inc16(rd16(ABS(0x0002))); wr16(ABS(0x0002), v);
            if (fz) wr16(ABS(0x0004), inc16(rd16(ABS(0x0004))));
        }
        cmp8((u8)Y, 0x0B);
        if (fz) break;
        lda16(rd16(ABS(0x0004))); push16(A);
        lda16(rd16(ABS(0x0002))); push16(A);
        lda16(rd16(ABS(0x0000))); push16(A);
        for (int k = 0; k < 2; k++) {
            wr16(ABS(0x0000), asl16(rd16(ABS(0x0000))));
            wr16(ABS(0x0002), rol16(rd16(ABS(0x0002))));
            wr16(ABS(0x0004), rol16(rd16(ABS(0x0004))));
        }
        A = pull16(); nz16(A);
        fc = 0; adc16(rd16(ABS(0x0000))); wr16(ABS(0x0000), A);
        A = pull16(); nz16(A);
        adc16(rd16(ABS(0x0002))); wr16(ABS(0x0002), A);
        A = pull16(); nz16(A);
        adc16(rd16(ABS(0x0004))); wr16(ABS(0x0004), A);
        wr16(ABS(0x0000), asl16(rd16(ABS(0x0000))));
        wr16(ABS(0x0002), rol16(rd16(ABS(0x0002))));
        wr16(ABS(0x0004), rol16(rd16(ABS(0x0004))));
        Y = inc8((u8)Y);
    }
    DB = pull8(); nz8(DB);
    setP(pull8());
    ret();
}

void GrowBar087E_m1x1(void);
void StepColourCycle_m1x1(void);
void BsxCheckSerial_m1x1(void);
void VramQueueCopy_m1x1(void);

/* $80:9FE0 / $80:9DEC: leave for the BS-X BIOS: interrupts and NMI off,
 * the BS-X memory mapping reset, the sound CPU back to its loader, JML
 * $10:5C3C. The port never runs this (port/src/gen_stubs.c replaces both
 * routines' exits). */
static void bsx_exit1(u32 site)
{
    if (port_bsx_exit(site))            /* the port's exit instead (port.h) */
        return;
    fi = 1;
    wr8(ABS(NMITIMEN), 0);
    lda8(0x00);
    wr8(LONG(0x035000), 0);
    wr8(LONG(0x045000), 0);
    wr8(LONG(0x075000), 0);
    wr8(LONG(0x085000), 0);
    lda8(0x80); wr8(LONG(0x0E5000), 0x80);
    lda8(0xFE); wr8(ABS(APUIO0), 0xFE);
    if (!r816_call_pc(0x105C3C))
        r816_unknown_target(site, 0x105C3C);
}

/* $80:9DE0 EndScreenStart (RTS): on a new Start press, with $81:800C set
 * go to mode $12 ($75 / $77 cleared, $1923 - 1); clear, leave for the
 * BIOS. */
void EndScreenStart_m1x1(void)
{
    if (port_end_screen_start())        /* the port restarts the hour (port.h) */
        return;
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x10);
    if (fz) { ret(); return; }
    lda8(rd8(LONG(STADIUM_SELECT)));
    if (fz) { bsx_exit1(0x809E0D); return; }
    lda8(0x12); wr8(DP(GAME_MODE), 0x12);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    ret();
}

/* $80:9F68 Mode12_AfterEnd (RTS): the closing credits: the BS-X serial check;
 * while $88 counts down the fade ($1903 = 15 - ($88 - 1) / 16, colour math
 * $1C00 = 9); every fourth frame the bar and colour cycle and the scroll
 * $9E + 1 (at $0700: HDMA off, $88 = $FF, $1905 = 5, $1903 = 0). Below
 * $0780 the next credits rows ($8F:BB00 + 8 * ($9E & ~7), $0220 bytes) go to
 * VRAM $3000 + 4 * (($9E + $E0) & $1F8); at $0780 the scroll stops, and
 * after $01C0 frames ($8A) Start leaves for the BIOS. */
void Mode12_AfterEnd_m1x1(void)
{
    push16(0x9F6A); BsxCheckSerial_m1x1();
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        wr8(DP(MODE_TIMER), dec8(rd8(DP(MODE_TIMER))));
        lda8(dec8((u8)A));
        lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(0x0F);
        fc = 1; sbc8(rd8(DP(0x00)));
        wr8(ABS(SH_INIDISP), (u8)A);
        lda8(0x09); wr8(ABS(PALETTE_BUF), 0x09);
        wr8(ABS(PALETTE_BUF + 0x01), 0);
        wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
        lda8(0xFF); wr8(ABS(COLOUR_CYCLE), 0xFF);
    }
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x03);
    if (fz) {
        push16(0x9F98); GrowBar087E_m1x1();
        push16(0x9F9B); StepColourCycle_m1x1();
        rep(0x20);
        wr16(DP(PIC_SCROLL), inc16(rd16(DP(PIC_SCROLL))));
        lda16(rd16(DP(PIC_SCROLL)));
        cmp16(A, 0x0700);
        if (fz) {
            sep(0x20);
            wr8(ABS(SH_HDMAEN), 0);
            lda8(0xFF); wr8(DP(MODE_TIMER), 0xFF);
            lda8(0x05); wr8(ABS(SH_TM), 0x05);
            wr8(ABS(SH_INIDISP), 0);
        }
    }
    rep(0x20);
    lda16(rd16(DP(END_FRAMES)));
    if (fz) {
        lda16(rd16(DP(PIC_SCROLL)));
        cmp16(A, 0x0780);
        if (fn || !fc) {
            lda16(rd16(DP(PIC_SCROLL)));
            and16(0xFFF8);
            lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
            fc = 0; adc16(0xBB00);
            wr16(DP(0x02), A);
            sep(0x20);
            lda8(0x8F); wr8(DP(0x04), 0x8F);
            rep(0x20);
            lda16(0x0220); wr16(DP(0x10), 0x0220);
            lda16(rd16(ABS(PIC_SCROLL)));
            fc = 0; adc16(0x00E0);
            and16(0x01F8);
            lda16(asl16(A)); lda16(asl16(A));
            fc = 0; adc16(0x3000);
            wr16(DP(0x00), A);
            sep(0x20);
            push16(0xA03D); VramQueueCopy_m1x1();
            ret();
            return;
        }
        wr16(DP(END_FRAMES), inc16(rd16(DP(END_FRAMES))));
        lda16(0x01C0); wr16(DP(TIMER_A), 0x01C0);
    }
    lda16(0x0780); wr16(DP(PIC_SCROLL), 0x0780);
    lda16(rd16(DP(TIMER_A)));
    if (!fz) {
        wr16(DP(TIMER_A), dec16(rd16(DP(TIMER_A))));
    } else {
        lda16(rd16(DP(P1_PRESS_HI)));
        and16(0x0010);
        if (!fz) {
            sep(0x20);
            bsx_exit1(0x80A003);
            return;
        }
    }
    sep(0x20);
    ret();
}

/* $80:B79B (RTL): the save checksum: $10:5004 = the 16-bit sum of the
 * bytes $10:5000-$5FFF, $11:5000-$5FFF and $12:5000-$5FFF, $10:5002 its
 * complement (both cleared to 0 / $FFFF first). */
void SaveChecksum_m1x1(void)
{
    rep(0x30);
    lda16(0x0000);
    wr16(LONG(SAVE_SUM), 0);
    A = dec16(A);
    wr16(LONG(SAVE_SUM_INV), A);
    wr16(DP(0x00), 0);
    X = 0x0FFF; nz16(X);
    do {
        WAIT_TICK();
        lda16(rd16(LONGX(0x105000))); and16(0x00FF); wr16(DP(0x02), A);
        lda16(rd16(LONGX(0x115000))); and16(0x00FF); wr16(DP(0x04), A);
        lda16(rd16(LONGX(0x125000))); and16(0x00FF);
        fc = 0; adc16(rd16(DP(0x00)));
        fc = 0; adc16(rd16(DP(0x02)));
        fc = 0; adc16(rd16(DP(0x04)));
        wr16(DP(0x00), A);
        X = dec16(X);
    } while (!fn);
    wr16(LONG(SAVE_SUM), A);
    eor16(0xFFFF);
    wr16(LONG(SAVE_SUM_INV), A);
    sep(0x30);
    retl();
}

/* $80:B702 (RTL): check the save header at $11:5700 against the ROM
 * title ($00:FFC0-$FFCF), the stadium flag $81:800C and the broadcast id
 * $7F:FFF0; on a mismatch clear $11:5700-$57FF, copy the title, and set
 * the starting values ($11:5720/23/26/29 = 4, $11:572C = $10). Then record
 * the broadcast id, date ($7F:FFF2, $7F:FFF9), the stadium flag and this
 * weekday's entry ($80:98AA by $7F:FFF3 & 7) and redo the checksum. */
void SaveHeaderCheck_m1x1(void)
{
    int same = 1;
    ldx8(0x0F);
    do {
        lda8(rd8(LONGX(ROM_TITLE)));
        cmp8((u8)A, rd8(LONGX(SAVE_TITLE)));
        if (!fz) { same = 0; break; }
        X = dec8((u8)X);
    } while (!fn);
    if (same) {
        lda8(rd8(LONG(SAVE_STADIUM)));
        cmp8((u8)A, rd8(LONG(STADIUM_SELECT)));
        if (!fz) same = 0;
    }
    if (same) {
        rep(0x20);
        lda16(rd16(LONG(BSX_CLOCK_0)));
        cmp16(A, rd16(LONG(SAVE_CLOCK)));
        if (!fz) same = 0;
    }
    if (!same) {
        rep(0x30);
        X = 0x00FE; nz16(X);
        lda16(0x0000);
        do {
            wr16(LONGX(SAVE_TITLE), 0);
            X = dec16(X); X = dec16(X);
        } while (!fn);
        sep(0x10);
        ldx8(0x0E);
        do {
            lda16(rd16(LONGX(ROM_TITLE)));
            wr16(LONGX(SAVE_TITLE), A);
            X = dec8((u8)X); X = dec8((u8)X);
        } while (!fn);
        lda16(0x0004);
        wr16(LONG(SAVE_TIMES), 4); wr16(LONG(SAVE_TIMES + 0x03), 4);
        wr16(LONG(SAVE_TIMES + 0x06), 4); wr16(LONG(SAVE_TIMES + 0x09), 4);
        lda16(0x0010); wr16(LONG(SAVE_BEST_TIME), 0x0010);
    }
    lda16(rd16(LONG(BSX_CLOCK_0))); wr16(LONG(SAVE_CLOCK), A);
    lda16(rd16(LONG(BSX_CLOCK_2))); wr16(LONG(SAVE_CLOCK_HI), A);
    lda16(rd16(LONG(BCAST_HOUR))); and16(0x00FF); wr16(LONG(SAVE_HOUR), A);
    lda16(rd16(LONG(STADIUM_SELECT))); wr16(LONG(SAVE_STADIUM), A);
    sep(0x20);
    lda8(rd8(LONG(BSX_CLOCK_3)));
    and8(0x07);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x98AA))); wr8(LONGX(SAVE_DAYS), (u8)A);
    push8(0x80); push16(0xB797); SaveChecksum_m1x1();
    sep(0x30);
    retl();
}

void QueueTimeDigits_m1x1(void);
void QueueFiveDigits_m1x1(void);

/* $80:97FF (RTL): keep the broadcast's records in the save: a total time
 * $084D-$084F below the saved best ($11:572C) replaces it with the course
 * times $083D-$0848 ($11:5720) and sets $085C; a score $0CE5-$0CE9 above
 * the saved one ($11:5730) replaces it and sets $085D. Then the broadcast
 * id, date, stadium flag and weekday entry, and the checksum. */
void SaveBestRecords_m1x1(void)
{
    wr8(ABS(NEW_TIME_RECORD), 0);
    wr8(ABS(NEW_SCORE_RECORD), 0);
    int better = 0;
    lda8(rd8(ABS(TOTAL_TIME)));
    cmp8((u8)A, rd8(LONG(SAVE_BEST_TIME)));
    if (!fz) better = !fc;
    else {
        lda8(rd8(ABS(TOTAL_TIME + 0x01)));
        cmp8((u8)A, rd8(LONG(SAVE_BEST_SEC)));
        if (!fz) better = !fc;
        else {
            lda8(rd8(ABS(TOTAL_TIME + 0x02)));
            cmp8((u8)A, rd8(LONG(SAVE_BEST_FRAC)));
            better = !fc;
        }
    }
    if (better) {
        lda8(rd8(ABS(TOTAL_TIME))); wr8(LONG(SAVE_BEST_TIME), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME + 0x01))); wr8(LONG(SAVE_BEST_SEC), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME + 0x02))); wr8(LONG(SAVE_BEST_FRAC), (u8)A);
        wr8(ABS(NEW_TIME_RECORD), inc8(rd8(ABS(NEW_TIME_RECORD))));
        ldx8(0x0B);
        do {
            lda8(rd8(ABSX(COURSE_BEST))); wr8(LONGX(SAVE_TIMES), (u8)A);
            X = dec8((u8)X);
        } while (!fn);
    }
    ldx8(0x00);
    for (;;) {
        lda8(rd8(LONGX(SAVE_BEST_SCORE)));
        cmp8((u8)A, rd8(ABSX(SCORE_DIGITS)));
        if (!fc) {
            ldx8(0x04);
            do {
                lda8(rd8(ABSX(SCORE_DIGITS))); wr8(LONGX(SAVE_BEST_SCORE), (u8)A);
                X = dec8((u8)X);
            } while (!fn);
            wr8(ABS(NEW_SCORE_RECORD), inc8(rd8(ABS(NEW_SCORE_RECORD))));
            break;
        }
        if (!fz) break;
        X = inc8((u8)X);
        cmp8((u8)X, 0x05);
        if (fc) break;
    }
    lda8(rd8(LONG(BSX_CLOCK_0))); wr8(LONG(SAVE_CLOCK), (u8)A);
    lda8(rd8(LONG(BSX_CLOCK_1))); wr8(LONG(0x115711), (u8)A);
    lda8(rd8(LONG(BSX_CLOCK_2))); wr8(LONG(SAVE_CLOCK_HI), (u8)A);
    lda8(rd8(LONG(BCAST_HOUR))); wr8(LONG(SAVE_STADIUM), (u8)A);
    lda8(rd8(LONG(STADIUM_SELECT))); wr8(LONG(SAVE_STADIUM), (u8)A);
    lda8(rd8(LONG(BSX_CLOCK_3)));
    and8(0x07);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x98AA))); wr8(LONGX(SAVE_DAYS), (u8)A);
    push8(0x80); push16(0x98A8); SaveChecksum_m1x1();
    retl();
}

/* $80:973F (RTS): the broadcast totals screen: BG3 / tile bases, the
 * saved best total ($11:572C) at $09D3 and best score ($11:5730) at $0A15;
 * the four course times $083D-$0848 rounded through $80:C6CE and summed
 * (minutes, seconds, hundredths) into $084D-$084F and $0849-$084B; the
 * records saved; $1905 = $17, $1923 + 1, $75 + 1, $0452/$0453 cleared, $88
 * = $20, $9A = $D8, $190D cleared. */
void BroadcastTotals_m1x1(void)
{
    lda8(0x19); wr8(ABS(BG3SC), 0x19);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    rep(0x20);
    lda16(0x09D3); wr16(DP(0x00), 0x09D3);
    lda16(0x572C); wr16(DP(0x18), 0x572C);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9763); QueueTimeDigits_m1x1();
    rep(0x20);
    lda16(0x0A15); wr16(DP(0x00), 0x0A15);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9779); QueueFiveDigits_m1x1();
    wr8(ABS(TOTAL_TIME), 0);
    wr8(ABS(TOTAL_TIME + 0x01), 0);
    wr8(ABS(TOTAL_TIME + 0x02), 0);
    ldx8(0x00);
    do {
        Y = rd8(ABSX(COURSE_BEST + 0x02)); nz8((u8)Y);
        lda8(rd8(ABSY(0xC6CE))); wr8(ABSX(COURSE_BEST + 0x02), (u8)A);
        fc = 0; adc8(rd8(ABS(TOTAL_TIME + 0x02)));
        wr8(ABS(TOTAL_TIME + 0x02), (u8)A);
        cmp8((u8)A, 0x64);
        if (fc) {
            sbc8(0x64);
            wr8(ABS(TOTAL_TIME + 0x02), (u8)A);
            u8 v = inc8(rd8(ABS(TOTAL_TIME + 0x01))); wr8(ABS(TOTAL_TIME + 0x01), v);
            if (fz) wr8(ABS(TOTAL_TIME), inc8(rd8(ABS(TOTAL_TIME))));
        }
        lda8(rd8(ABSX(COURSE_BEST + 0x01)));
        fc = 0; adc8(rd8(ABS(TOTAL_TIME + 0x01)));
        wr8(ABS(TOTAL_TIME + 0x01), (u8)A);
        cmp8((u8)A, 0x3C);
        if (fc) {
            sbc8(0x3C);
            wr8(ABS(TOTAL_TIME + 0x01), (u8)A);
            wr8(ABS(TOTAL_TIME), inc8(rd8(ABS(TOTAL_TIME))));
        }
        lda8(rd8(ABSX(COURSE_BEST)));
        fc = 0; adc8(rd8(ABS(TOTAL_TIME)));
        wr8(ABS(TOTAL_TIME), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME))); wr8(ABS(0x0849), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME + 0x01))); wr8(ABS(0x084A), (u8)A);
        lda8(rd8(ABS(TOTAL_TIME + 0x02))); wr8(ABS(0x084B), (u8)A);
        X = inc8((u8)X); X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x0C);
    } while (!fc);
    push8(0x80); push16(0x97E2); SaveBestRecords_m1x1();
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    wr8(ABS(END_STEP), 0);
    wr8(ABS(ENTRY_CURSOR), 0);
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(0xD8); wr8(DP(END_SCROLL), 0xD8);
    wr8(ABS(SH_CGADSUB), 0);
    ret();
}

/* $80:91CD (RTS): new session: the save header check, the attract state
 * $0CD3-$0CF2 and $7A-$85 cleared, $79 = $7E = 1. */
void StartSession_m1x1(void)
{
    push8(0x80); push16(0x91D0); SaveHeaderCheck_m1x1();
    ldy8(0x1F);
    lda8(0x00);
    do {
        wr8(ABSY(0x0CD3), 0);
        Y = dec8((u8)Y);
    } while (!fn);
    ldy8(0x0B);
    do {
        wr8(ABSY(0x007A), 0);
        Y = dec8((u8)Y);
    } while (!fn);
    wr8(DP(COURSE_SLOT), 0);
    wr8(ABS(0x007B), 0);
    lda8(0x01);
    wr8(DP(TURN_LATCH), 0x01);
    wr8(DP(FIRST_RIVAL), 0x01);
    lda8(0x80);
    lda8(0x0F);
    ret();
}

void ShowEntryPassword_m1x1(void);

/* $80:9D43 (RTS): the entry screen: tilemaps cleared, the font page
 * ($89:C800, $0800 bytes) to VRAM $0800, $0453 and $1923 + 1, $9A/$9B
 * cleared; the saved best total ($11:572C) at $08F5 and best score
 * ($11:5730) at $0936; the row at $03A5 (palette 1) at $0978; the entry
 * password; with $81:800C set the row $80:9DDD at $09EB. */
void DrawEntryScreen_m1x1(void)
{
    push8(0x80); push16(0x9D46); ClearTilemaps_m1x1();
    rep(0x20);
    push16(D);
    lda16(0x4300);
    D = A;
    ldy8(0x80); wr8(ABS(VMAIN), 0x80);
    lda16(0x1801); wr16(DP(0x00), 0x1801);
    lda16(0x0800); wr16(ABS(VMADDL), 0x0800);
    lda16(0x0800); wr16(DP(0x05), 0x0800);
    lda16(0xC800);
    ldx8(0x89);
    ldy8(0x01);
    push16(0x9D6C); DmaKick_m0x1();
    D = pull16(); nz16(D);
    sep(0x20);
    wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    wr8(ABS(END_SCROLL), 0);
    wr8(ABS(0x009B), 0);
    rep(0x20);
    lda16(0x08F5); wr16(DP(0x00), 0x08F5);
    lda16(0x572C); wr16(DP(0x18), 0x572C);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9D91); QueueTimeDigits_m1x1();
    rep(0x20);
    lda16(0x0936); wr16(DP(0x00), 0x0936);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9DA7); QueueFiveDigits_m1x1();
    rep(0x20);
    lda16(0x0978); wr16(DP(0x00), 0x0978);
    lda16(0x03A5); wr16(DP(0x02), 0x03A5);
    sep(0x20);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push16(0x9DBC); VramQueueTiles_m1x1();
    push8(0x80); push16(0x9DC0); ShowEntryPassword_m1x1();
    lda8(rd8(LONG(STADIUM_SELECT)));
    if (!fz) {
        rep(0x20);
        lda16(0x09EB); wr16(DP(0x00), 0x09EB);
        lda16(0x9DDD); wr16(DP(0x02), 0x9DDD);
        sep(0x20);
        lda8(0x01); wr8(DP(0x0F), 0x01);
        push16(0x9DDB); VramQueueTiles20_m1x1();
    }
    ret();
}

void BsxFindFile_m0x0(void);

/* $81:FA81 BsxProbeReceiver (RTL, M=1 X=0): is a satellite receiver
 * there? $2194 = 1; $2188 must read back $AA and $55 as written; then
 * $2188/$2189/$218E/$218F are cleared and Z is set if they all read 0
 * (no receiver: Z clear). */
void BsxProbeReceiver_m1x0(void)
{
    lda8(0x01); wr8(ABS(0x2194), 0x01);
    lda8(0xAA); wr8(ABS(0x2188), 0xAA);
    cmp8((u8)A, rd8(ABS(0x2188)));
    if (!fz) { retl(); return; }
    lda8(0x55); wr8(ABS(0x2188), 0x55);
    cmp8((u8)A, rd8(ABS(0x2188)));
    if (!fz) { retl(); return; }
    wr8(ABS(0x2188), 0);
    wr8(ABS(0x2189), 0);
    wr8(ABS(0x218E), 0);
    wr8(ABS(0x218F), 0);
    lda8(rd8(ABS(0x2188)));
    ora8(rd8(ABS(0x2189)));
    ora8(rd8(ABS(0x218E)));
    ora8(rd8(ABS(0x218F)));
    retl();
}

/* $81:FA68 BsxDetect (RTL): $7F:FFE1 = 1 when a receiver answers the
 * probe, else 0; P kept. */
void BsxDetect_m1x1(void)
{
    push8(getP());
    sep(0x20);
    rep(0x10);
    lda8(0x00);
    wr8(LONG(NO_RECEIVER), 0);
    push8(0x81); push16(0xFA76); BsxProbeReceiver_m1x0();
    if (!fz) {
        lda8(0x01);
        wr8(LONG(NO_RECEIVER), 0x01);
    }
    setP(pull8());
    retl();
}

/* $81:F80C BsxOpenTimeChannel (RTL): the frame counter $7F:FFFA cleared;
 * look up the time file (type $0101, $13D3 / $13D5 = $0800); found: its
 * channel $13FE to $7F:FFF7 and $218E, the stream armed ($2191 / $2192),
 * carry clear; not found: $7F:FFF7 = $5533, $218E = $AAEE, carry set. P
 * otherwise kept. */
void BsxOpenTimeChannel_m1x1(void)
{
    push8(getP());
    rep(0x30);
    lda16(0x0000);
    wr16(LONG(BCAST_FRAME), 0);
    lda16(0x0101); wr16(ABS(0x13D3), 0x0101);
    lda16(0x0800); wr16(ABS(0x13D5), 0x0800);
    push8(0x81); push16(0xF825); BsxFindFile_m0x0();
    if (!fc) {
        lda16(rd16(ABS(0x13FE)));
        wr16(LONG(BS_TIME_CHAN), A);
        wr16(ABS(0x218E), A);
        lda16(0x0001);
        wr16(ABS(0x2191), 0x0001);
        wr16(ABS(0x2192), 0x0001);
        setP(pull8());
        fc = 0;
        retl();
        return;
    }
    rep(0x20);
    lda16(0x5533); wr16(LONG(BS_TIME_CHAN), 0x5533);
    lda16(0xAAEE); wr16(ABS(0x218E), 0xAAEE);
    setP(pull8());
    fc = 1;
    retl();
}

void Mul16_m1x0(void);

/* $80:A387 EepromSendCommand (RTS, M=1 X=0): select the serial EEPROM
 * ($2198: chip select $C0, clock bit 0) and clock out the command byte A,
 * high bit first (data bit 7). */
void EepromSendCommand_m1x0(void)
{
    push8((u8)A);
    wr8(ABS(0x2198), 0);
    lda8(0xC0); wr8(ABS(0x2198), 0xC0);
    ora8(0x01); wr8(ABS(0x2198), (u8)A);
    A = (A & 0xFF00) | pull8(); nz8((u8)A);
    Y = 0x0008; nz16(Y);
    do {
        push8((u8)A);
        and8(0x80);
        ora8(0x40);
        wr8(ABS(0x2198), (u8)A);
        ora8(0x01);
        wr8(ABS(0x2198), (u8)A);
        A = (A & 0xFF00) | pull8(); nz8((u8)A);
        lda8(asl8((u8)A));
        Y = dec16(Y);
    } while (!fz);
    ret();
}

/* $80:A34B EepromReadWords (RTS, M=1 X=0): read X 16-bit words from EEPROM
 * address A into Y ($00:Y onwards, DB): the read command A | $80, then 16
 * clocked bits per word (bit 7 of $2198), high bit first. */
void EepromReadWords_m1x0(void)
{
    push16(Y);
    ora8(0x80);
    push16(0xA350); EepromSendCommand_m1x0();
    lda8(0x40); wr8(ABS(0x2198), 0x40);
    Y = pull16(); nz16(Y);
    rep(0x20);
    do {
        push16(Y);
        Y = 0x0010; nz16(Y);
        lda16(0x0000);
        do {
            push16(A);
            sep(0x20);
            lda8(0x41); wr8(ABS(0x2198), 0x41);
            lda8(0x40); wr8(ABS(0x2198), 0x40);
            lda8(rd8(ABS(0x2198)));
            lda8(asl8((u8)A));
            rep(0x20);
            A = pull16(); nz16(A);
            lda16(rol16(A));
            Y = dec16(Y);
        } while (!fz);
        Y = pull16(); nz16(Y);
        wr16(ABSY(0x0000), A);
        Y = inc16(Y); Y = inc16(Y);
        X = dec16(X);
    } while (!fz);
    sep(0x20);
    wr8(ABS(0x2198), 0);
    ret();
}

/* $80:A2B0 (RTL): read the saved entry record from EEPROM (13 words to
 * $1E89); its 11 digits as a number ($0000-$0005), the three BCD digits
 * $1E9B / $1E9D / $1E9F as a value in $10 (hundreds, tens, ones), and that
 * value's bit pairs merged into the course flags $0850-$0854. */
void LoadSavedEntry_m1x1(void)
{
    sep(0x20);
    rep(0x10);
    lda8(0x00);
    Y = 0x1E89; nz16(Y);
    X = 0x000D; nz16(X);
    push16(0xA2BE); EepromReadWords_m1x0();
    X = 0x1E89; nz16(X);
    wr16(DP(0x1C), 0x1E89);
    lda8(0x00); wr8(DP(0x1E), 0);
    push16(0xA2CA); DigitsToNumber_m1x0();
    wr8(DP(0x10), 0);
    wr8(DP(0x11), 0);
    rep(0x20);
    lda16(rd16(ABS(0x1E9B))); and16(0x000F); wr16(DP(0x00), A);
    lda16(0x0064); wr16(DP(0x02), 0x0064);
    sep(0x20);
    push8(0x80); push16(0xA2E3); Mul16_m1x0();
    rep(0x20);
    lda16(rd16(DP(0x04))); wr16(DP(0x10), A);
    sep(0x20);
    rep(0x20);
    lda16(rd16(ABS(0x1E9D))); and16(0x000F); wr16(DP(0x00), A);
    lda16(0x000A); wr16(DP(0x02), 0x000A);
    sep(0x20);
    push8(0x80); push16(0xA300); Mul16_m1x0();
    rep(0x20);
    lda16(rd16(DP(0x04)));
    fc = 0; adc16(rd16(DP(0x10)));
    wr16(DP(0x10), A);
    sep(0x20);
    rep(0x20);
    lda16(rd16(ABS(0x1E9F))); and16(0x000F);
    fc = 0; adc16(rd16(DP(0x10)));
    wr16(DP(0x10), A);
    sep(0x30);
    lda8(rd8(DP(0x10))); and8(0x03);
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x04), tsb8(rd8(ABS(ENTRY_CODE + 0x04))));
    lda8(rd8(DP(0x10))); and8(0x0C);
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x03), tsb8(rd8(ABS(ENTRY_CODE + 0x03))));
    lda8(rd8(DP(0x10))); and8(0x30);
    wr8(ABS(ENTRY_CODE + 0x02), tsb8(rd8(ABS(ENTRY_CODE + 0x02))));
    lda8(rd8(DP(0x10))); and8(0xC0);
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(ENTRY_CODE + 0x01), tsb8(rd8(ABS(ENTRY_CODE + 0x01))));
    lda8(rd8(DP(0x11))); and8(0x03);
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(ABS(ENTRY_CODE), tsb8(rd8(ABS(ENTRY_CODE))));
    retl();
}

void Mul16c_m1x1(void);
void FinishModeSetup_m1x1(void);
void ShowBonusLabel_m1x1(void);
void QueueScoreDigits_m1x1(void);
void SetJumpBase_m1x1(void);
void PlaceRival_m1x1(void);
void CrashLand_m1x1(void);
void EmptyHook_m1x1(void);

/* Empty routines (RTS): $80:BE2D, $80:DDF8, $81:9956, $81:B67E. */
void EmptyBE2D_m1x1(void) { ret(); }
void EmptyDDF8_m1x1(void) { ret(); }
void Empty9956_m1x1(void) { ret(); }
void EmptyB67E_m1x1(void) { ret(); }

/* $80:B6A9 ModeNop (RTS). */
void ModeNop_m1x1(void) { ret(); }

/* $80:8AFD (RTS): next mode step ($75 + 1). */
void ModeStepNext_m1x1(void)
{
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    ret();
}

/* $80:8D0C (RTS): the (empty) hook, sound $03, then FinishModeSetup. */
void StartModeWithSound_m1x1(void)
{
    push8(0x80); push16(0x8D0F); EmptyHook_m1x1();
    lda8(0x03);
    push8(0x80); push16(0x8D15); QueueSound_m1x1();
    FinishModeSetup_m1x1();
}

/* $81:BD67 (RTS): clear the palette $0349 of object $039D. */
void ClearObjectPalette_m1x1(void)
{
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    wr8(ABSX(OBJ_FRAME), 0);
    ret();
}

/* $80:B21D ScreenOff (RTS): screen off ($1903 shadow). */
void ScreenOff_m1x1(void)
{
    lda8(0x80);
    wr8(ABS(INIDISP), 0x80);
    wr8(ABS(SH_INIDISP), 0x80);
    ret();
}

/* $81:9520 (RTS by tail): run the step $0128,Y of object Y through the
 * table $81:9528 (one entry, $81:9530). */
void ObjectStepDispatch_m1x1(void)
{
    lda8(rd8(ABSY(EFFECT_STEP)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    u16 t = rd16(LONG(0x819528 + X));
    if (!r816_call_pc(0x810000 | t))
        r816_unknown_target(0x819525, t);
}

/* $80:9E96: leave the ending for mode $0F ($75 / $77 cleared, $1923 - 1,
 * $88 = $20). */
static void ending_done1(void)
{
    lda8(0x0F); wr8(DP(GAME_MODE), 0x0F);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    rep(0x20);
    lda16(0x0020); wr16(DP(MODE_TIMER), 0x0020);
    sep(0x20);
    ret();
}

/* $80:9E6E Mode10_Ending (RTS): count $88 down, then mode $0F. */
void Mode10_Ending_m1x1(void)
{
    rep(0x20);
    wr16(DP(MODE_TIMER), dec16(rd16(DP(MODE_TIMER))));
    sep(0x20);
    if (fz) { ending_done1(); return; }
    ret();
}

/* $80:D7B2 (RTS): $09C2 = $85 / 4. */
void SetBonusRow_m1x1(void)
{
    lda8(rd8(DP(SUB_STEP)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(0x09C2), (u8)A);
    ret();
}

/* $80:D8FB (RTS): $1958 + 1, $AC = $1E, $AD = 1, then the bonus label. */
void StartBonusLabel_m1x1(void)
{
    wr8(ABS(SPRITES_FROZEN), inc8(rd8(ABS(SPRITES_FROZEN))));
    lda8(0x1E); wr8(DP(BONUS_SECONDS), 0x1E);
    lda8(0x01); wr8(DP(BONUS_TICK), 0x01);
    ShowBonusLabel_m1x1();
}

/* $80:E2AB (RTS): BG mode 9, BG1 map at $0000 (BG1SC = 1). */
void SetRaceBgMode_m1x1(void)
{
    lda8(0x09); wr8(ABS(BGMODE), 0x09);
    lda8(0x01); wr8(ABS(BG1SC), 0x01);
    ret();
}

/* $80:EBD3 (RTS, X = rider): clear $0220,X, $01A0,X and $0981,X. */
void ClearRiderAction_m1x1(void)
{
    lda8(0x00);
    wr8(ABSX(RIDER_PIECE_ACTION), 0);
    wr8(ABSX(RIDER_POSE), 0);
    wr8(ABSX(RIDER_ACTION_POSE), 0);
    ret();
}

/* $80:BC55 (RTS, M=0): A * $40 through Mul16c ($42 = A), the middle word
 * $45 back in A. */
void ScaleIrqLine_m0x1(void)
{
    wr16(DP(0x42), A);
    sep(0x20);
    push16(0xBC5B); Mul16c_m1x1();
    rep(0x20);
    lda16(rd16(DP(RIDER_LEAN_TIMER)));
    ret();
}

/* $80:DDF9 (RTS): HDMA preset 2, $3A = $0CD7 = $20. */
void StartDemoHdma_m1x1(void)
{
    lda8(0x02);
    push8(0x80); push16(0xDDFE); SetHdmaPreset_m1x1();
    lda8(0x20); wr8(DP(TUMBLE_TIMER), 0x20);
    wr8(ABS(ATTRACT_DEMO), 0x20);
    ret();
}

/* $80:E29D (RTS): course 0, race, $7E cleared, attract demo $0CD7 = $0F. */
void ResetDemoCourse_m1x1(void)
{
    lda8(0x00);
    wr8(DP(COURSE_SLOT), 0); wr8(DP(FINAL_RACE), 0); wr8(DP(FIRST_RIVAL), 0);
    lda8(0x0F); wr8(ABS(ATTRACT_DEMO), 0x0F);
    ret();
}

/* $80:EE5C (RTS): the score digits at VRAM $1B38, palette $20. */
void QueueScoreDigitsHud_m1x1(void)
{
    lda8(0x38); wr8(DP(0x00), 0x38);
    lda8(0x1B); wr8(DP(0x01), 0x1B);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    QueueScoreDigits_m1x1();
}

/* $80:E3DF (RTS by tail): place rival 7 ($092A = 7, $0198,7 cleared) on
 * lane 3. */
void PlaceLastRival_m1x1(void)
{
    ldx8(0x07);
    wr8(ABS(RIDER_IN_MIDDLE), 0x07);
    lda8(0x00); wr8(ABSX(RIDER_PAGE), 0);
    ldy8(0x03);
    PlaceRival_m1x1();
}

/* $80:EBC1 (RTS by tail, X = rider): clear $09C4 / $09C7, then land. */
void ResetAndLand_m1x1(void)
{
    lda8(0x00);
    wr8(ABS(PLAYER_GROUND), 0);
    wr8(ABS(PLAYER_LIFT), 0);
    CrashLand_m1x1();
}

/* $80:EB25 / $80:EB2F / $80:EB34 (RTS by tail, X = rider): jump base $30
 * or $10 (SetJumpBase); the first also sets $0200,X = 2 and $0240,X = 1. */
void JumpBase30_m1x1(void) { lda8(0x30); SetJumpBase_m1x1(); }
void JumpBase10_m1x1(void) { lda8(0x10); SetJumpBase_m1x1(); }
void StartJumpBase30_m1x1(void)
{
    lda8(0x02); wr8(ABSX(RIDER_LANDING), 0x02);
    lda8(0x01); wr8(ABSX(RIDER_SLOW_AT), 0x01);
    JumpBase30_m1x1();
}

/* Demo exits that continue with the next entry: $80:E3D4 ($099D = $0990
 * = $10), $80:E3EE and $80:E3F2 (A = $35 + $80:E1A5,X, unused). */
void DemoExitBoost_m1x1(void)
{
    lda8(0x10);
    wr8(ABS(0x099D), 0x10);
    wr8(ABS(ENGINE_HEAT), 0x10);
    demo_next1();
}
void DemoExitSkip40_m1x1(void)
{
    lda8(0x40);
    demo_next1();
}
void DemoExitSkip35_m1x1(void)
{
    lda8(0x35);
    fc = 0; adc8(rd8(ABSX(0xE1A5)));
    demo_next1();
}

/* $80:EAFF (RTS, X = rider): $0200,X = 1. */
void SetJumpState1_m1x1(void)
{
    lda8(0x01);
    wr8(ABSX(RIDER_LANDING), 0x01);
    ret();
}

/* $80:EB04 (RTS, X = rider): on the ground a bump crash: unless already
 * crashed ($01C8), the player gets sound $3B, and the speed $01C0 grows
 * (to 4, then $01B8 = $C0); the crash starts at speed 0. */
void RiderBumpCrash_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_AIR)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(RIDER_BUMPED)));
    if (fz) {
        cmp8((u8)X, 0x00);
        if (fz) {
            lda8(0x3B);
            push8(0x80); push16(0xC367); QueueSound_m1x1();
        }
        lda8(rd8(ABSX(RIDER_SPEED_HI)));
        cmp8((u8)A, 0x04);
        if (!fc) wr8(ABSX(RIDER_SPEED_HI), inc8(rd8(ABSX(RIDER_SPEED_HI))));
        else { lda8(0xC0); wr8(ABSX(RIDER_SPEED_LO), 0xC0); }
    }
    lda8(0x00);
    crash_from1();
    ret();
}

void ClearJumpState_m1x1(void);

/* $80:EBB5 (RTS, X = rider): by the jump state $0208: 1 = landed ($01F8
 * cleared, then ResetAndLand), 3 = the jump flags cleared ($80:E9A8). */
void LandingStep_m1x1(void)
{
    lda8(rd8(ABSX(RIDER_JUMP)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(0x00); wr8(ABSX(RIDER_HEIGHT), 0);
        ResetAndLand_m1x1();
        return;
    }
    cmp8((u8)A, 0x03);
    if (!fz) { ret(); return; }
    ClearJumpState_m1x1();
}

/* Interrupt return: P, then PC and PB from the interrupt frame. */
static void rti1(void) { setP(pull8()); pull16(); pull8(); }

/* $80:FFA0 (RTI): the empty handler for COP, BRK and ABORT. */
void EmptyVector_m1x1(void) { rti1(); }

/* $80:FFA9 -> $80:BBDA Irq (RTI): on the H/V timer IRQ with $91 set, the
 * scroll $94 as multiplier, the HDMA line table at $1B00 from fixed
 * factors (stadium perspective), $1B20 / $1B22 = factor $A0 and $94. */
void IrqVector_m1x1(void)
{
    static const struct { u16 f; u8 dp; u16 ret; } k[8] = {
        { 0x0120, 0x0E, 0xBBFF }, { 0x00E0, 0x0C, 0xBC07 }, { 0x00D0, 0x0A, 0xBC0F },
        { 0x00C0, 0x08, 0xBC17 }, { 0x00B0, 0x06, 0xBC1F }, { 0x00A0, 0x04, 0xBC27 },
        { 0x00A0, 0x02, 0xBC2F }, { 0x0040, 0x00, 0xBC37 },
    };
    rep(0x30);
    push16(A); push16(X); push16(Y); push16(D);
    lda16(0x1B00); D = A; nz16(D);
    push8(DB); push8(0x80); DB = pull8(); nz8(DB);
    sep(0x30);
    lda8(rd8(ABS(TIMEUP)));
    if (fn) {
        lda8(rd8(ABS(0x0091)));
        if (!fz) {
            rep(0x20);
            lda16(rd16(ABS(SCROLL_X)));
            wr16(DP(0x40), A);
            for (int i = 0; i < 8; i++) {
                lda16(k[i].f);
                push16(k[i].ret); ScaleIrqLine_m0x1();
                wr16(DP(k[i].dp), A);
            }
            wr16(DP(0x10), 0);
            lda16(0x00A0);
            push16(0xBC41); ScaleIrqLine_m0x1();
            wr16(DP(0x20), A);
            lda16(rd16(ABS(SCROLL_X)));
            wr16(DP(0x22), A);
            wr16(DP(0x24), 0);
            sep(0x20);
        }
    }
    rep(0x30);
    DB = pull8(); nz8(DB);
    D = pull16(); nz16(D);
    Y = pull16(); nz16(Y);
    X = pull16(); nz16(X);
    A = pull16(); nz16(A);
    rti1();
}

void NmiFixedUploads_m1x1(void);
void NmiFlushVramQueue_m1x1(void);

/* One sound command channel in NMI: while a command $6D+n is pending, wait
 * for APU port n to echo it, then step the ring index $69+n and clear the
 * pending byte; with none pending, send the next one from the ring
 * $1400 + $40n (cleared after taking). */
static void nmi_sound_channel1(u8 n)
{
    lda8(rd8(DP(SND_PENDING + n)));
    if (!fz) {
        cmp8((u8)A, rd8(ABS(APUIO0 + n)));
        if (!fz) return;
        lda8(rd8(DP(SND_READ + n)));
        lda8(inc8((u8)A));
        and8(0x3F);
        wr8(DP(SND_READ + n), (u8)A);
        lda8(0x00);
    } else {
        X = rd8(DP(SND_READ + n)); nz8((u8)X);
        lda8(rd8(ABSX(SND_RING + 0x40 * n)));
        if (fz) return;
        wr8(ABSX(SND_RING + 0x40 * n), 0);
    }
    wr8(DP(SND_PENDING + n), (u8)A);
    wr8(ABS(APUIO0 + n), (u8)A);
}

/* $80:FFA1 -> $80:85FF Nmi (RTI): forced blank and HDMA off; unless the
 * frame was already serviced ($081A), the fixed uploads, the VRAM queue
 * and the BG1-BG3 scroll (BG1 V scroll wobbles with $8C bit 1 unless $90);
 * NMI/IRQ enable ($91 adds the V timer), screen, layer, HDMA and colour
 * math registers from their shadows $1903-$190F; the four sound command
 * channels (channel 3 spins until its port echoes); $081A + 1. */
void NmiVector_m1x1(void)
{
    rep(0x30);
    push16(A); push16(X); push16(Y); push16(D);
    lda16(0x0000); D = A; nz16(D);
    push8(DB); push8(0x80); DB = pull8(); nz8(DB);
    lda16(0x0006); wr16(ABS(VTIMEL), A);
    sep(0x30);
    lda8(rd8(ABS(RDNMI)));
    lda8(0x80); wr8(ABS(INIDISP), 0x80);
    wr8(ABS(HDMAEN), 0);
    lda8(rd8(ABS(NMI_FRAME_READY)));
    if (fz) {
        push16(0x862A); NmiFixedUploads_m1x1();
        push8(0x80); push16(0x862E); NmiFlushVramQueue_m1x1();
        lda8(rd8(DP(0x92))); wr8(ABS(BG1HOFS), (u8)A);
        lda8(rd8(DP(0x8C))); wr8(ABS(BG1HOFS), (u8)A);
        lda8(rd8(DP(0x93))); wr8(ABS(BG1VOFS), (u8)A);
        if (rd8(DP(0x90))) {
            wr8(ABS(BG1VOFS), 0);
        } else {
            lda8(rd8(DP(0x8C)));
            and8(0x02);
            lda8(lsr8((u8)A));
            wr8(ABS(BG1VOFS), (u8)A);
        }
        for (int r = 0; r < 4; r++) {
            lda8(rd8(DP(0x98 + 2 * r))); wr8(ABS(BG2HOFS + r), (u8)A);
            lda8(rd8(DP(0x99 + 2 * r))); wr8(ABS(BG2HOFS + r), (u8)A);
        }
    }
    lda8(rd8(DP(0x91)) ? 0xB1 : 0x81);
    wr8(ABS(NMITIMEN), (u8)A);
    wr8(ABS(SH_NMITIMEN), (u8)A);
    lda8(rd8(ABS(SH_INIDISP))); wr8(ABS(INIDISP), (u8)A);
    lda8(rd8(ABS(SH_TM))); wr8(ABS(TM), (u8)A);
    lda8(rd8(ABS(SH_TS))); wr8(ABS(TS), (u8)A);
    lda8(rd8(ABS(SH_HDMAEN))); wr8(ABS(HDMAEN), (u8)A);
    lda8(rd8(ABS(SH_TMW))); wr8(ABS(TMW), (u8)A);
    lda8(rd8(ABS(SH_TSW))); wr8(ABS(TSW), (u8)A);
    lda8(rd8(ABS(SH_CGWSEL))); wr8(ABS(CGWSEL), (u8)A);
    lda8(rd8(ABS(SH_CGADSUB))); wr8(ABS(CGADSUB), (u8)A);
    sep(0x30);
    nmi_sound_channel1(0);
    nmi_sound_channel1(1);
    nmi_sound_channel1(2);
    for (;;) {                              /* channel 3: BNE back to LDA $70 */
        lda8(rd8(DP(SND_PENDING + 3)));
        if (fz) {
            X = rd8(DP(SND_READ + 3)); nz8((u8)X);
            lda8(rd8(ABSX(SND_RING + 0xC0)));
            if (fz) break;
            wr8(ABSX(SND_RING + 0xC0), 0);
        } else {
            cmp8((u8)A, rd8(ABS(APUIO3)));
            if (!fz) continue;
            lda8(rd8(DP(SND_READ + 3)));
            lda8(inc8((u8)A));
            and8(0x3F);
            wr8(DP(SND_READ + 3), (u8)A);
            lda8(0x00);
        }
        wr8(DP(SND_PENDING + 3), (u8)A);
        wr8(ABS(APUIO3), (u8)A);
        break;
    }
    wr8(ABS(NMI_FRAME_READY), inc8(rd8(ABS(NMI_FRAME_READY))));
    rep(0x30);
    DB = pull8(); nz8(DB);
    D = pull16(); nz16(D);
    Y = pull16(); nz16(Y);
    X = pull16(); nz16(X);
    A = pull16(); nz16(A);
    rti1();
}
