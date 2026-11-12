/*
 * Stadium 3-4 (Gen B): small leaf routines, hand-written.
 */
#include "hand/hand.h"

void VramQueueTiles20_m1x1(void);

/* $80:DE3D QueueNameplate (RTS): queue the tile list at $80:DE4F at VRAM
 * $18F4 (JMP VramQueueTiles20). */
void QueueNameplate_m1x1(void)
{
    rep(0x20);
    lda16(0x18F4); wr16(DP(0x00), 0x18F4);
    lda16(0xDE4F); wr16(DP(0x02), 0xDE4F);
    sep(0x20);
    VramQueueTiles20_m1x1();
}

/* $80:A650 QueueResultsRow (RTS): unless $84 is set, queue the list at
 * $80:A666 at VRAM $1892 through $80:B38B. */
void QueueResultsRow_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    rep(0x20);
    lda16(0x1892); wr16(DP(0x00), 0x1892);
    lda16(0xA666); wr16(DP(0x02), 0xA666);
    sep(0x20);
    r816_call_pc(0x80B38B);
}

/* $81:C64E FadeApuVolume (RTS): step the sound volume $198A down by 4
 * (while non-zero) and send it to APUIO2. */
void FadeApuVolume_m1x1(void)
{
    lda8(rd8(ABS(0x198A)));
    if (!fz) {
        fc = 1;
        sbc8(0x04);
        wr8(ABS(0x198A), (u8)A);
    }
    lda8(rd8(ABS(0x198A)));
    wr8(ABS(0x2142), (u8)A);
    ret();
}

/* $82:8CC5 ReadStreamByte (M=1, X=0, RTS): A = [$08]; advance the long
 * pointer $08-$0A, wrapping from $FFFF to $8000 of the next bank. */
void ReadStreamByte_m1x0(void)
{
    u32 p = rd16(DP(0x08)) | ((u32)rd8(DP(0x0A)) << 16);
    lda8(rd8(p & 0xFFFFFF));
    X = rd16(DP(0x08)); nz16(X);
    X = inc16(X);
    if (fz) {
        ldx16(0x8000);
        wr8(DP(0x0A), inc8(rd8(DP(0x0A))));
    }
    wr16(DP(0x08), X);
    ret();
}

/* $80:BCF3 AddSpeedToX (RTS): rider X's position high $0186 = speed high
 * $01C6, then fraction $0914 += speed low $01BE with the carry into $0186. */
void AddSpeedToX_m1x1(void)
{
    lda8(rd8(ABSX(0x01C6)));
    wr8(ABSX(0x0186), (u8)A);
    lda8(rd8(ABSX(0x01BE)));
    fc = 0;
    adc8(rd8(ABSX(0x0914)));
    wr8(ABSX(0x0914), (u8)A);
    if (fc)
        wr8(ABSX(0x0186), inc8(rd8(ABSX(0x0186))));
    ret();
}

/* $80:EE8B BlankQueueRowB (RTS): fill words $7F:000E..$7F:002A with tile
 * $00FC. */
void BlankQueueRowB_m1x1(void)
{
    ldx8(0x1E);
    rep(0x20);
    lda16(0x00FC);
    do {
        wr16(LONGX(0x7F000C), 0x00FC);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
    sep(0x20);
    ret();
}

/* $81:98A0 StepAnimTimer (RTS): for object $03A3, count the word timer
 * $031F down; at zero step the frame $030F and reload $00D0. */
void StepAnimTimer_m1x1(void)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    rep(0x20);
    u16 t = dec16(rd16(ABSX(0x031F)));
    wr16(ABSX(0x031F), t);
    if (t == 0) {
        wr16(ABSX(0x030F), inc16(rd16(ABSX(0x030F))));
        lda16(0x00D0);
        wr16(ABSX(0x031F), 0x00D0);
    }
    sep(0x20);
    ret();
}

/* $80:EBBC DispatchBy85B (RTS): tail-jump through the table at $80:DFCF
 * indexed by $85 (pointer kept at $00/$01, X = 0). */
void DispatchBy85B_m1x1(void)
{
    lda8(rd8(DP(0x85)));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xDFCF))); wr8(DP(0x00), (u8)A);
    Y = inc8((u8)Y);
    lda8(rd8(ABSY(0xDFCF))); wr8(DP(0x01), (u8)A);
    ldx8(0x00);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}

/* $80:BD0D ScrollByPlayerSpeed (RTS): scroll $9A += player speed $0186
 * (copied to $9C); on carry flip the nametable half $90 and count the page
 * $9D. */
void ScrollByPlayerSpeed_m1x1(void)
{
    lda8(rd8(DP(0x9A)));
    fc = 0;
    adc8(rd8(ABS(0x0186)));
    wr8(DP(0x9A), (u8)A);
    wr8(DP(0x9C), (u8)A);
    if (fc) {
        lda8(rd8(DP(0x90)));
        eor8(0x01);
        wr8(DP(0x90), (u8)A);
        wr8(DP(0x9D), inc8(rd8(DP(0x9D))));
    }
    ret();
}

/* $80:DFB9 EndAtBonusMinute (RTS): at broadcast minute $38 or $1F switch to
 * mode $15 and leave the caller (drop its return, JMP $80:8F17). */
void EndAtBonusMinute_m1x1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x38);
    if (!fz) cmp8((u8)A, 0x1F);
    if (!fz) { ret(); return; }
    lda8(0x15);
    wr8(DP(0x76), 0x15);
    lda8(pull8());
    lda8(pull8());
    r816_call_pc(0x808F17);
}

/* $81:C5B5 ScrollJumpRider (RTS): $0196 += speed $0186 (carry into $019E),
 * $090C -= $0186. */
void ScrollJumpRider_m1x1(void)
{
    lda8(rd8(ABS(0x0186)));
    fc = 0;
    adc8(rd8(ABS(0x0196)));
    if (fc)
        wr8(ABS(0x019E), inc8(rd8(ABS(0x019E))));
    wr8(ABS(0x0196), (u8)A);
    lda8(rd8(ABS(0x090C)));
    fc = 1;
    sbc8(rd8(ABS(0x0186)));
    wr8(ABS(0x090C), (u8)A);
    ret();
}

/* $81:C608 DispatchByDistance (RTS): find the band of the word $1981 in the
 * thresholds at $81:C5FE (five words) and tail-jump through $81:C61F. */
void DispatchByDistance_m1x1(void)
{
    rep(0x20);
    ldx8(0x00);
    lda16(rd16(ABS(0x1981)));
    do {
        cmp16(A, rd16(ABSX(0xC5FE)));
        if (!fc) break;
        X = inc8((u8)X);
        X = inc8((u8)X);
        cmp8((u8)X, 0x0A);
    } while (!fz);
    sep(0x20);
    r816_call_pc(0x810000u | rd16(LONG(0x81C61F + X)));
}

/* $81:C743 SetJumpBanner (RTS): sprite $1610-$1613 = X $38, Y $199C, tile
 * $8E, attribute $2D; $1824 = 2. */
void SetJumpBanner_m1x1(void)
{
    lda8(0x38); wr8(ABS(0x1610), 0x38);
    lda8(rd8(ABS(0x199C))); wr8(ABS(0x1611), (u8)A);
    lda8(0x8E); wr8(ABS(0x1612), 0x8E);
    lda8(0x2D); wr8(ABS(0x1613), 0x2D);
    lda8(0x02); wr8(ABS(0x1824), 0x02);
    ret();
}

/* $80:954E AddBcdDigits (RTS): add the digits $03D0-$03D4 into the decimal
 * number $154A-$154E (one digit per byte, carry into the byte before). */
void AddBcdDigits_m1x1(void)
{
    ldx8(0x04);
    do {
        lda8(rd8(ABSX(0x154A)));
        fc = 0;
        adc8(rd8(ABSX(0x03D0)));
        cmp8((u8)A, 0x0A);
        if (fc) {
            sbc8(0x0A);
            wr8(ABSX(0x1549), inc8(rd8(ABSX(0x1549))));
        }
        wr8(ABSX(0x154A), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* Object $03A3: when its counter $037F is zero, set the delay $036F, clear
 * the timer $031F, step the state $030F and set $034F to `next`; otherwise
 * set $034F to `wait` (STZ when it is zero). */
static void object_advance_or(u8 delay, u8 next, u8 wait)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABSX(0x037F)));
    if (!fz) {
        if (wait) lda8(wait);
        wr8(ABSX(0x034F), wait);
        return;
    }
    lda8(delay); wr8(ABSX(0x036F), delay);
    wr8(ABSX(0x031F), 0);
    wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
    lda8(next); wr8(ABSX(0x034F), next);
}

static void object_advance(u8 delay, u8 next) { object_advance_or(delay, next, 0); }

/* $81:BAC3, $81:BB23, $81:BD92 ObjectAdvance8/20/80 (RTS). */
void ObjectAdvance8_m1x1(void)  { object_advance(0x08, 0x06); ret(); }
void ObjectAdvance20_m1x1(void) { object_advance(0x20, 0x03); ret(); }
void ObjectAdvance80_m1x1(void) { object_advance(0x80, 0x02); ret(); }
/* $81:BED8, $81:BFB0 ObjectAdvance9/C0 (RTS). */
void ObjectAdvance9_m1x1(void)  { object_advance(0x09, 0x03); ret(); }
void ObjectAdvanceC0_m1x1(void) { object_advance(0xC0, 0x04); ret(); }
/* $81:BC60, $81:BD38 ObjectAdvanceOr4/Or5 (RTS): as above, but while the
 * counter runs $034F is 4 or 5. */
void ObjectAdvanceOr4_m1x1(void) { object_advance_or(0x15, 0x03, 0x04); ret(); }
void ObjectAdvanceOr5_m1x1(void) { object_advance_or(0x08, 0x06, 0x05); ret(); }

/* $82:86EB TickTimers (RTL): count the bytes $39+X down to zero, X = $16..0
 * each frame and $29..0 every eleventh frame (divider $38). */
void TickTimers_m1x1(void)
{
    ldx8(0x16);
    u8 d = dec8(rd8(DP(0x38)));
    wr8(DP(0x38), d);
    if (fn) {
        lda8(0x0A); wr8(DP(0x38), 0x0A);
        ldx8(0x29);
    }
    do {
        lda8(rd8(DPX(0x39)));
        if (!fz)
            wr8(DPX(0x39), dec8(rd8(DPX(0x39))));
        X = dec8((u8)X);
    } while (!fn);
    retl();
}

void BsxReadSerialByte_m1x1(void);

/* $80:9879 BsxReadHeader3 (RTL): with BS-X $2199 = $81 read three serial
 * bytes into $00-$02, then set $2199 back to $41. */
void BsxReadHeader3_m1x1(void)
{
    lda8(0x81); wr8(ABS(0x2199), 0x81);
    push8(0x80); push16(0x9883); BsxReadSerialByte_m1x1(); wr8(DP(0x00), (u8)A);
    push8(0x80); push16(0x9889); BsxReadSerialByte_m1x1(); wr8(DP(0x01), (u8)A);
    push8(0x80); push16(0x988F); BsxReadSerialByte_m1x1(); wr8(DP(0x02), (u8)A);
    lda8(0x41); wr8(ABS(0x2199), 0x41);
    retl();
}

/* $80:E74D CheckCourseTime (RTS): in the course's last broadcast minute
 * ($80:E76D by course $7C), once the frame count reaches $0708 set $CA. */
void CheckCourseTime_m1x1(void)
{
    X = rd8(DP(0x7C)); nz8((u8)X);
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, rd8(ABSX(0xE76D)));
    if (fz) {
        rep(0x20);
        lda16(rd16(LONG(0x7FFFFA)));
        cmp16(A, 0x0708);
        if (fc) {
            sep(0x20);
            lda8(0x01); wr8(ABS(0x00CA), 0x01);
        }
    }
    sep(0x20);
    ret();
}

/* $80:F048 StopWhenStill (RTS): when the speed bytes $01BE, $01C6 and $01CE
 * are all zero: $CD = 1, $CF = $20, set bit 0 of $63, $01B6 = $B0. */
void StopWhenStill_m1x1(void)
{
    lda8(rd8(ABS(0x01BE)));
    ora8(rd8(ABS(0x01C6)));
    ora8(rd8(ABS(0x01CE)));
    if (fz) {
        lda8(0x01); wr8(DP(0xCD), 0x01);
        lda8(0x20); wr8(DP(0xCF), 0x20);
        lda8(0x01);
        u8 m = rd8(DP(0x63));
        fz = (m & 0x01) == 0;
        wr8(DP(0x63), m | 0x01);
        lda8(0xB0); wr8(ABS(0x01B6), 0xB0);
    }
    ret();
}

/* $81:C7C3 ClearOnBonusTiles (RTL): for object X on track row $1F with tile
 * $3B-$3D under it, clear $020E,X and $08BC,X. */
void ClearOnBonusTiles_m1x1(void)
{
    lda8(rd8(ABSX(0x021E)));
    cmp8((u8)A, 0x1F);
    if (fz) {
        lda8(rd8(ABSX(0x0216)));
        cmp8((u8)A, 0x3B);
        if (!fz) cmp8((u8)A, 0x3C);
        if (!fz) cmp8((u8)A, 0x3D);
        if (fz) {
            wr8(ABSX(0x020E), 0);
            wr8(ABSX(0x08BC), 0);
        }
    }
    retl();
}

void SetJumpBanner_m1x1(void);

/* $81:C724 StepJumpBanner (RTL): while the banner is on ($199A) count its
 * timer $199B down (raising it, $199C - 1, while the timer is $20 or more)
 * and redraw it; at zero switch it off. */
void StepJumpBanner_m1x1(void)
{
    lda8(rd8(ABS(0x199A)));
    if (fz) { retl(); return; }
    lda8(rd8(ABS(0x199B)));
    if (fz) {
        wr8(ABS(0x199A), 0);
        retl();
        return;
    }
    cmp8((u8)A, 0x20);
    if (fc)
        wr8(ABS(0x199C), dec8(rd8(ABS(0x199C))));
    wr8(ABS(0x199B), dec8(rd8(ABS(0x199B))));
    push16(0xC741); SetJumpBanner_m1x1();
    retl();
}

/* $80:D8EF CheckBonusTime (RTS): in broadcast minute $1E or $37, once the
 * frame count reaches $0708 set $CA. */
void CheckBonusTime_m1x1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x1E);
    if (!fz) cmp8((u8)A, 0x37);
    if (fz) {
        rep(0x20);
        lda16(rd16(LONG(0x7FFFFA)));
        cmp16(A, 0x0708);
        if (fc) {
            sep(0x20);
            lda8(0x01); wr8(ABS(0x00CA), 0x01);
        }
    }
    sep(0x20);
    ret();
}

/* $81:CA05 Hundreds (RTS, entered with M either way): Y = the word $1569
 * divided by 200 (at least 1); A keeps the remainder. */
static void hundreds(void)
{
    rep(0x20);
    ldy8(0x00);
    lda16(rd16(ABS(0x1569)));
    for (;;) {
        cmp16(A, 0x00C8);
        if (!fc) break;
        Y = inc8((u8)Y);
        fc = 1;
        sbc16(0x00C8);
        WAIT_TICK();
    }
    sep(0x20);
    cmp8((u8)Y, 0x00);
    if (fz) Y = inc8((u8)Y);
    ret();
}
void Hundreds_m1x1(void) { hundreds(); }
void Hundreds_m0x1(void) { hundreds(); }

/* $80:9C58 SetupJumpScreen (RTS): $0450 = 8, $1905/$1906 = 2, BGMODE $25,
 * $79 = $75 = 1, $77 = 0, $A2 = 8. */
void SetupJumpScreen_m1x1(void)
{
    lda8(0x08); wr8(ABS(0x0450), 0x08);
    lda8(0x02); wr8(ABS(0x1905), 0x02); wr8(ABS(0x1906), 0x02);
    lda8(0x25); wr8(ABS(0x2105), 0x25);
    lda8(0x01); wr8(DP(0x79), 0x01); wr8(DP(0x75), 0x01);
    lda8(lsr8((u8)A)); wr8(DP(0x77), (u8)A);
    lda8(0x08); wr8(ABS(0x00A2), 0x08);
    ret();
}

/* $80:BC41 RiderStartLane (RTS): for rider X with $56,X clear: rider 0
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
    wr8(ABSX(0x01D6), inc8(rd8(ABSX(0x01D6))));
    ret();
}

/* $80:DA0E, $80:ECD8 ScrollBySpeed16 (RTS): fraction $0994 += speed $0186
 * * 16 (the carry out of the shifts goes in), carry into $93; when $93
 * wraps flip the half $91. */
static void scroll_by_speed16(void)
{
    lda8(rd8(ABS(0x0186)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    adc8(rd8(ABS(0x0994)));
    wr8(ABS(0x0994), (u8)A);
    lda8(rd8(DP(0x93)));
    adc8(0x00);
    wr8(DP(0x93), (u8)A);
    if (fc) {
        lda8(rd8(DP(0x91)));
        eor8(0x01);
        wr8(DP(0x91), (u8)A);
    }
    ret();
}
void ScrollBySpeed16_m1x1(void)  { scroll_by_speed16(); }
void ScrollBySpeed16B_m1x1(void) { scroll_by_speed16(); }

/* $80:DF55 SetBgTileBase (RTL): $0086 = ($63 & $30) << 5 + $E000. */
void SetBgTileBase_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(0x63)));
    and16(0x0030);
    lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
    fc = 0; adc16(0x2000);
    fc = 0; adc16(0xC000);
    wr16(ABS(0x0086), A);
    sep(0x20);
    retl();
}

/* $80:E4A9 SetupCourseScreen (RTS): $09C2 = $85 / 4; BG1SC = $1900 = 3,
 * BG2SC = $43, BG12NBA = $55, $01FE = $30, $1958 = 0. */
void SetupCourseScreen_m1x1(void)
{
    lda8(rd8(DP(0x85)));
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    wr8(ABS(0x09C2), (u8)A);
    lda8(0x03); wr8(ABS(0x2107), 0x03); wr8(ABS(0x1900), 0x03);
    lda8(0x43); wr8(ABS(0x2108), 0x43);
    lda8(0x55); wr8(ABS(0x210B), 0x55);
    lda8(0x30); wr8(ABS(0x01FE), 0x30);
    wr8(ABS(0x1958), 0);
    ret();
}

/* $81:B929 StepObjectScript (RTS): for object $03A3, when its delay $036F
 * is zero step $031F; past 7 clear $02EF, else load $034F and the next
 * delay from the tables at $81:B94D and $81:B955. */
void StepObjectScript_m1x1(void)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABSX(0x036F)));
    if (!fz) { ret(); return; }
    wr8(ABSX(0x031F), inc8(rd8(ABSX(0x031F))));
    lda8(rd8(ABSX(0x031F)));
    cmp8((u8)A, 0x08);
    if (fc) { wr8(ABSX(0x02EF), 0); ret(); return; }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xB94D))); wr8(ABSX(0x034F), (u8)A);
    lda8(rd8(ABSY(0xB955))); wr8(ABSX(0x036F), (u8)A);
    ret();
}

/* $82:8CD4 ReadStreamWord (M=1, X=0, RTS): two stream bytes, the first in
 * the high byte of A (same pointer rules as ReadStreamByte). */
static void stream_step(void)
{
    u32 p = rd16(DP(0x08)) | ((u32)rd8(DP(0x0A)) << 16);
    lda8(rd8(p & 0xFFFFFF));
    X = inc16(X);
    if (fz) {
        ldx16(0x8000);
        wr8(DP(0x0A), inc8(rd8(DP(0x0A))));
    }
    wr16(DP(0x08), X);
}
void ReadStreamWord_m1x0(void)
{
    X = rd16(DP(0x08)); nz16(X);
    stream_step();
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    stream_step();
    ret();
}
