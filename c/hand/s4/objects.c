/*
 * Stadium 3-4 (Gen B): jump-contest and bonus object steps, hand-written.
 * Each object ($03A3 selects it) has a state $030F, a step $031F, a random
 * count $032F, a frame $034F, a delay $036F and a counter $037F.
 */
#include "hand/hand.h"

void RandomB_m1x1(void);

/* When the counter is zero: delay `delay`, a random count 1-4, restart the
 * step, next state, frame `next`. Otherwise the frame is `wait` (STZ when
 * zero). `ret` is the JSL return address minus one. */
static void object_advance_random(u8 delay, u8 next, u8 wait, u16 ret)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABSX(0x037F)));
    if (!fz) {
        if (wait) lda8(wait);
        wr8(ABSX(0x034F), wait);
        return;
    }
    lda8(delay); wr8(ABSX(0x036F), delay);
    push8(0x81); push16(ret); RandomB_m1x1();
    and8(0x03);
    lda8(inc8((u8)A));
    wr8(ABSX(0x032F), (u8)A);
    wr8(ABSX(0x031F), 0);
    wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
    lda8(next); wr8(ABSX(0x034F), next);
}

/* $81:BA63, $81:BB97, $81:BCD8, $81:BDF3, $81:BF4B, $81:BBF7
 * ObjectAdvanceRandom* (RTS). */
void ObjectAdvanceRandom20_m1x1(void) { object_advance_random(0x20, 0x01, 0x00, 0xBA73); ret(); }
void ObjectAdvanceRandom10_m1x1(void) { object_advance_random(0x10, 0x01, 0x00, 0xBBA7); ret(); }
void ObjectAdvanceRandom0A_m1x1(void) { object_advance_random(0x0A, 0x01, 0x00, 0xBCE8); ret(); }
void ObjectAdvanceRandom09_m1x1(void) { object_advance_random(0x09, 0x01, 0x00, 0xBE03); ret(); }
void ObjectAdvanceRandom10B_m1x1(void) { object_advance_random(0x10, 0x01, 0x00, 0xBF5B); ret(); }
void ObjectAdvanceRandom16_m1x1(void) { object_advance_random(0x16, 0x09, 0x09, 0xBC07); ret(); }

/* When the delay has run out, step $031F through 1-3 (0 becomes `zero`)
 * and load the delay and frame from the tables at `delays`/`frames`.
 * `tay_first` follows $81:BFCD, which moves the step into Y before the
 * zero test. */
static void object_cycle(u8 zero, u16 delays, u16 frames, int tay_first)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABSX(0x036F)));
    if (!fz) return;
    lda8(rd8(ABSX(0x031F)));
    lda8(inc8((u8)A));
    and8(0x03);
    wr8(ABSX(0x031F), (u8)A);
    if (tay_first) { Y = (u8)A; nz8((u8)Y); }
    if (fz) {
        if (tay_first) lda8(zero); else lda8(inc8((u8)A));
        wr8(ABSX(0x031F), (u8)A);
        if (tay_first) { Y = (u8)A; nz8((u8)Y); }
    }
    if (!tay_first) { Y = (u8)A; nz8((u8)Y); }
    lda8(rd8(ABSY(delays))); wr8(ABSX(0x036F), (u8)A);
    lda8(rd8(ABSY(frames))); wr8(ABSX(0x034F), (u8)A);
}

/* $81:BDAF ObjectCycleA, $81:BFCD ObjectCycleB (RTS). */
void ObjectCycleA_m1x1(void) { object_cycle(0x01, 0xBDD4, 0xBDD8, 0); ret(); }
void ObjectCycleB_m1x1(void) { object_cycle(0x02, 0xBFF4, 0xBFF8, 1); ret(); }

/* $80:D5D9 SetupBonusScreen (RTS): $09C2 = $85 / 4; BG1SC = $1900 = 3,
 * BG2SC = $43, BG12NBA = $55, BG3SC = $1A, BG34NBA = 1. */
void SetupBonusScreen_m1x1(void)
{
    lda8(rd8(DP(0x85)));
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    wr8(ABS(0x09C2), (u8)A);
    lda8(0x03); wr8(ABS(0x2107), 0x03); wr8(ABS(0x1900), 0x03);
    lda8(0x43); wr8(ABS(0x2108), 0x43);
    lda8(0x55); wr8(ABS(0x210B), 0x55);
    lda8(0x1A); wr8(ABS(0x2109), 0x1A);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    ret();
}

/* $80:E6AB FillBonusColumns (RTS): for X = 6..0 write the tiles $4C/$4B at
 * $09D3/$09D9 and $18 down rows $0A59, $0B59-$0C99 of column X. */
void FillBonusColumns_m1x1(void)
{
    static const u16 rows[] = { 0x0A59, 0x0B59, 0x0B99, 0x0BD9, 0x0C19, 0x0C59, 0x0C99 };
    ldx8(0x06);
    do {
        lda8(0x4C); wr8(ABSX(0x09D3), 0x4C);
        lda8(0x4B); wr8(ABSX(0x09D9), 0x4B);
        lda8(0x18);
        for (int i = 0; i < 7; i++)
            wr8(ABSX(rows[i]), 0x18);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:DF31 ResultsWait (RTS): leave on a new press ($21 | $26) or when the
 * word timer $88 runs out: clear $0CD7, $1E87 -= 10, mode 9 (JMP
 * $80:8F17). */
void ResultsWait_m1x1(void)
{
    lda8(rd8(DP(0x21)));
    ora8(rd8(DP(0x26)));
    if (fz) {
        rep(0x20);
        u16 t = dec16(rd16(DP(0x88)));
        wr16(DP(0x88), t);
        if (!fz) { sep(0x20); ret(); return; }
    }
    sep(0x20);
    wr8(ABS(0x0CD7), 0);
    lda8(rd8(ABS(0x1E87)));
    fc = 1;
    sbc8(0x0A);
    wr8(ABS(0x1E87), (u8)A);
    lda8(0x09); wr8(DP(0x76), 0x09);
    r816_call_pc(0x808F17);
}

void QueueSoundB_m1x1(void);
void ScrollByPlayerSpeed_m1x1(void);
void FinishModeSetup_m1x1(void);

/* $80:E835 SetJumpRiderSprites (RTS): two sprites at OAM buffer $16C0,Y
 * for rider X: X positions A + $1F and + 8, Y positions from $01B6,X + 8
 * and + 16, tiles $E0 and $F0. */
void SetJumpRiderSprites_m1x1(void)
{
    fc = 0; adc8(0x1F); wr8(ABSY(0x16C0), (u8)A);
    fc = 0; adc8(0x08); wr8(ABSY(0x16C4), (u8)A);
    lda8(rd8(ABSX(0x01B6)));
    fc = 0; adc8(0x08); wr8(ABSY(0x16C5), (u8)A);
    fc = 0; adc8(0x08); wr8(ABSY(0x16C1), (u8)A);
    lda8(0xE0); wr8(ABSY(0x16C2), 0xE0);
    lda8(0xF0); wr8(ABSY(0x16C6), 0xF0);
    ret();
}

/* $80:ECF4 JumpRunUp (RTS): before the take-off ($84 = 0) scroll by the
 * speed (JMP ScrollByPlayerSpeed). After it, $0196 += speed; at $F0 or
 * more land ($84 = $019E = 2, sound $2D), else the speed is zero. */
void JumpRunUp_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    if (fz) { ScrollByPlayerSpeed_m1x1(); return; }
    lda8(rd8(ABS(0x0186)));
    fc = 0;
    adc8(rd8(ABS(0x0196)));
    wr8(ABS(0x0196), (u8)A);
    cmp8((u8)A, 0xF0);
    if (fc) {
        lda8(0x02); wr8(DP(0x84), 0x02); wr8(ABS(0x019E), 0x02);
        lda8(0x2D);
        push8(0x80); push16(0xED15); QueueSoundB_m1x1();
        ret();
        return;
    }
    lda8(0x00); wr8(ABS(0x0186), 0x00);
    ret();
}

/* $82:84C5 ResetSoundState (RTL): $2C = $0F, $1903 = $80, $9A = 0,
 * $1959-$195C = 1-4, $1E8D-$1E90 = 0. */
void ResetSoundState_m1x1(void)
{
    lda8(0x0F); wr8(DP(0x2C), 0x0F);
    lda8(0x80); wr8(ABS(0x1903), 0x80);
    wr8(DP(0x9A), 0);
    lda8(0x01);
    for (u16 a = 0x1959; a <= 0x195C; a++) {
        wr8(ABS(a), (u8)A);
        if (a != 0x195C) lda8(inc8((u8)A));
    }
    for (u16 a = 0x1E8D; a <= 0x1E90; a++)
        wr8(ABS(a), 0);
    retl();
}

/* $80:B5C3 StartBonusMode (RTS): sound 2, clear $1E92-$1E95, timer $88 =
 * $0708, $1991 = $0258, BG3SC $1A, BG34NBA 2, $1905 = 6, then
 * FinishModeSetup. */
void StartBonusMode_m1x1(void)
{
    lda8(0x02);
    push8(0x80); push16(0xB5C8); QueueSoundB_m1x1();
    for (u16 a = 0x1E92; a <= 0x1E95; a++)
        wr8(ABS(a), 0);
    rep(0x20);
    lda16(0x0708); wr16(DP(0x88), 0x0708);
    lda16(0x0258); wr16(ABS(0x1991), 0x0258);
    sep(0x20);
    lda8(0x1A); wr8(ABS(0x2109), 0x1A);
    lda8(0x02); wr8(ABS(0x210C), 0x02);
    lda8(0x06); wr8(ABS(0x1905), 0x06);
    FinishModeSetup_m1x1();
}

/* $80:FB25 StartWheelie (RTS): for the player (X = 0) with $0940, $60,
 * $01EE,X and $0CE0 all clear: $0990 = 8; the first time ($1979 clear)
 * $1979 = $30, sounds $30 and $2D, $AA = 0. Other riders with those flags
 * clear only get $0990 = 8. */
void StartWheelie_m1x1(void)
{
    lda8((u8)X);
    ora8(rd8(ABS(0x0940)));
    ora8(rd8(DP(0x60)));
    ora8(rd8(ABSX(0x01EE)));
    ora8(rd8(ABS(0x0CE0)));
    if (!fz) { ret(); return; }
    lda8(0x08); wr8(ABS(0x0990), 0x08);
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x1979)));
    if (!fz) { ret(); return; }
    lda8(0x30); wr8(ABS(0x1979), 0x30);
    push8(0x80); push16(0xFB49); QueueSoundB_m1x1();
    lda8(0x2D);
    push8(0x80); push16(0xFB4F); QueueSoundB_m1x1();
    wr8(DP(0xAA), 0);
    ret();
}

/* $80:862A StartIntroMode (RTS): from broadcast minute 3 on go straight to
 * mode 0 ($75 = $77 = 0); before it set BG3SC $2A, BG12NBA $44, BG34NBA 1,
 * timer $88 = $0A8C, step $1923 and FinishModeSetup. */
void StartIntroMode_m1x1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x03);
    if (fc) {
        lda8(0x00); wr8(ABS(0x0076), 0x00);
        wr8(DP(0x75), 0);
        wr8(DP(0x77), 0);
        ret();
        return;
    }
    lda8(0x2A); wr8(ABS(0x2109), 0x2A);
    lda8(0x44); wr8(ABS(0x210B), 0x44);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    rep(0x20);
    lda16(0x0A8C); wr16(DP(0x88), 0x0A8C);
    sep(0x20);
    wr8(ABS(0x1923), inc8(rd8(ABS(0x1923))));
    FinishModeSetup_m1x1();
}
