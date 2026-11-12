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
 * clear $034F. */
static void object_advance(u8 delay, u8 next)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABSX(0x037F)));
    if (!fz) { wr8(ABSX(0x034F), 0); return; }
    lda8(delay); wr8(ABSX(0x036F), delay);
    wr8(ABSX(0x031F), 0);
    wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
    lda8(next); wr8(ABSX(0x034F), next);
}

/* $81:BAC3, $81:BB23, $81:BD92 ObjectAdvance8/20/80 (RTS). */
void ObjectAdvance8_m1x1(void)  { object_advance(0x08, 0x06); ret(); }
void ObjectAdvance20_m1x1(void) { object_advance(0x20, 0x03); ret(); }
void ObjectAdvance80_m1x1(void) { object_advance(0x80, 0x02); ret(); }
