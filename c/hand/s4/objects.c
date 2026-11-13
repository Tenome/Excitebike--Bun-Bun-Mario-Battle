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

void VramQueueTiles20_m1x1(void);
void VramQueueTiles_m1x1(void);

/* $80:CF32 RankPlayer (RTS): while racing ($086E set, $84 clear, $92 set)
 * the place $086E = 8 minus the riders the word $085E beats among
 * $0860-$086C. */
void RankPlayer_m1x1(void)
{
    lda8(rd8(ABS(0x086E)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x92)));
    if (fz) { ret(); return; }
    rep(0x20);
    lda16(0x0008); wr16(ABS(0x086E), 0x0008);
    ldy8(0x0E);
    lda16(rd16(ABS(0x085E)));
    do {
        cmp16(A, rd16(ABSY(0x085E)));
        if (fc && !fz)
            wr16(ABS(0x086E), dec16(rd16(ABS(0x086E))));
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fz);
    sep(0x20);
    ret();
}

/* $81:C178 ObjectWalkCycle (RTS): every 4 frames step $031F through 0-3
 * and count $027F; at $A0 reset the state ($030F = 0, delay $40). The frame
 * $034F comes from the table at $81:C1AA. */
void ObjectWalkCycle_m1x1(void)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABSX(0x036F)));
    if (fz) {
        lda8(rd8(ABSX(0x031F)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(0x031F), (u8)A);
        lda8(0x04); wr8(ABSX(0x036F), 0x04);
        wr8(ABSX(0x027F), inc8(rd8(ABSX(0x027F))));
        lda8(rd8(ABSX(0x027F)));
        cmp8((u8)A, 0xA0);
        if (fc) {
            wr8(ABSX(0x030F), 0);
            lda8(0x40); wr8(ABSX(0x036F), 0x40);
        }
    }
    Y = rd8(ABSX(0x031F)); nz8((u8)Y);
    lda8(rd8(ABSY(0xC1AA)));
    wr8(ABSX(0x034F), (u8)A);
    ret();
}

/* $82:8BB4 LoadStadiumPalette (RTL): copy 32 bytes from the Stadium's
 * table ($82:8BB0 indexed by $81:800C) to $1D20 and flag $1915. */
void LoadStadiumPalette_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82);
    DB = pull8(); nz8(DB);
    rep(0x20);
    lda16(rd16(LONG(0x81800C)));
    lda16(asl16(A));
    X = A; nz16(X);
    lda16(rd16(ABSX(0x8BB0)));
    wr16(DP(0x00), A);
    ldy8(0x1E);
    do {
        lda16(rd16(((u32)DB << 16) + ((rd16(DP(0x00)) + Y) & 0xFFFF)));
        wr16(ABSY(0x1D20), A);
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fn);
    sep(0x20);
    wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:A39C CountCourseBits (RTL): $085A = the number of set bits in the
 * low six bits of $0850-$0859, modulo 64. */
void CountCourseBits_m1x1(void)
{
    wr8(DP(0x00), 0);
    ldx8(0x09);
    do {
        ldy8(0x05);
        lda8(rd8(ABSX(0x0850)));
        wr8(DP(0x01), (u8)A);
        do {
            lda8(rd8(DP(0x01)));
            lda8(lsr8((u8)A));
            wr8(DP(0x01), (u8)A);
            lda8(rol8((u8)A));
            and8(0x01);
            fc = 0;
            adc8(rd8(DP(0x00)));
            wr8(DP(0x00), (u8)A);
            Y = dec8((u8)Y);
        } while (!fn);
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(DP(0x00)));
    and8(0x3F);
    wr8(ABS(0x085A), (u8)A);
    retl();
}

/* $80:B76A StartJumpMode (RTS): sound 2, clear the jump scores and flags
 * ($1578, $156B, $0CE0, words $1567, $1569, $156D), timer $88 = $0708,
 * $1905 = $13, BG3SC $1A, BG34NBA 2. */
void StartJumpMode_m1x1(void)
{
    lda8(0x02);
    push8(0x80); push16(0xB76F); QueueSoundB_m1x1();
    wr8(ABS(0x1578), 0);
    wr8(ABS(0x156B), 0);
    wr8(ABS(0x0CE0), 0);
    rep(0x20);
    wr16(ABS(0x1567), 0);
    wr16(ABS(0x1569), 0);
    wr16(ABS(0x156D), 0);
    lda16(0x0708); wr16(DP(0x88), 0x0708);
    sep(0x20);
    lda8(0x15); wr8(ABS(0x1905), 0x15);
    lda8(0x13); wr8(ABS(0x1905), 0x13);
    lda8(0x1A); wr8(ABS(0x2109), 0x1A);
    lda8(0x02); wr8(ABS(0x210C), 0x02);
    ret();
}

/* $80:DD67 ShowResultsBanner (RTS): $88 = $10, queue the lists at $80:DD98
 * (VRAM $18B6) and $80:DD9D (VRAM $0090, attribute 8), step $77, $10 = 0,
 * then JMP $80:DDAE. */
void ShowResultsBanner_m1x1(void)
{
    lda8(0x10); wr8(DP(0x88), 0x10);
    rep(0x20);
    lda16(0x18B6); wr16(DP(0x00), 0x18B6);
    lda16(0xDD98); wr16(DP(0x02), 0xDD98);
    sep(0x20);
    push16(0xDD7B); VramQueueTiles20_m1x1();
    rep(0x20);
    lda16(0x0090); wr16(DP(0x00), 0x0090);
    lda16(0xDD9D); wr16(DP(0x02), 0xDD9D);
    sep(0x20);
    lda8(0x08); wr8(DP(0x0F), 0x08);
    push16(0xDD90); VramQueueTiles_m1x1();
    wr8(DP(0x77), inc8(rd8(DP(0x77))));
    wr8(DP(0x10), 0);
    r816_call_pc(0x80DDAE);
}

/* A = RandomB() & $3F + $40 (`ret`: JSL return address minus one). */
static void random_rest(u16 ret)
{
    push8(0x81); push16(ret); RandomB_m1x1();
    and8(0x3F);
    fc = 0;
    adc8(0x40);
}

/* Put the object to rest: state `state` (STZ when zero) and a random pause
 * $037F of $40-$7F frames. */
static void object_rest(u8 state, u16 ret)
{
    if (state) lda8(state);
    wr8(ABSX(0x030F), state);
    random_rest(ret);
    wr8(ABSX(0x037F), (u8)A);
}

/* Load the delay and frame for step Y. */
static void object_frame(u16 delays, u16 frames)
{
    lda8(rd8(ABSY(delays))); wr8(ABSX(0x036F), (u8)A);
    lda8(rd8(ABSY(frames))); wr8(ABSX(0x034F), (u8)A);
}

/* Selects the object; returns 0 while its delay runs. */
static int object_ready(void)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABSX(0x036F)));
    return fz;
}

/* Flip the step between 0 and 1 (`mask` 1), or count it 0-3 (`mask` 3).
 * Each time it comes back to 0 count $032F down; at zero rest in `state`. */
static void object_repeat(u8 mask, u8 state, u16 delays, u16 frames, u16 ret)
{
    if (!object_ready()) return;
    lda8(rd8(ABSX(0x031F)));
    if (mask == 1) eor8(0x01);
    else { lda8(inc8((u8)A)); and8(mask); }
    wr8(ABSX(0x031F), (u8)A);
    Y = (u8)A; nz8((u8)Y);
    if (fz) {
        u8 n = dec8(rd8(ABSX(0x032F)));
        wr8(ABSX(0x032F), n);
        if (fz) { object_rest(state, ret); return; }
    }
    object_frame(delays, frames);
}

/* $81:BA8A, $81:BBBE, $81:BCFF, $81:BE1A ObjectBlink*, $81:BF72 and
 * $81:BC20 ObjectRepeat4* (RTS). */
void ObjectBlinkA_m1x1(void) { object_repeat(1, 0, 0xBABF, 0xBAC1, 0xBAA8); ret(); }
void ObjectBlinkB_m1x1(void) { object_repeat(1, 0, 0xBBF3, 0xBBF5, 0xBBDC); ret(); }
void ObjectBlinkC_m1x1(void) { object_repeat(1, 0, 0xBD34, 0xBD36, 0xBD1D); ret(); }
void ObjectBlinkD_m1x1(void) { object_repeat(1, 0, 0xBE4F, 0xBE51, 0xBE38); ret(); }
void ObjectRepeat4A_m1x1(void) { object_repeat(3, 0, 0xBFA8, 0xBFAC, 0xBF91); ret(); }
void ObjectRepeat4B_m1x1(void) { object_repeat(3, 2, 0xBC58, 0xBC5C, 0xBC41); ret(); }

/* $81:BD57 ObjectRun4 (RTS): count the step 0-3; at 0 rest in state 2. */
void ObjectRun4_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(0x031F)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(0x031F), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        if (fz) object_rest(2, 0xBD73);
        else object_frame(0xBD8A, 0xBD8E);
    }
    ret();
}

/* $81:BAE0 ObjectRun6 (RTS): count the step 0-5; at 0 rest in state 2. */
void ObjectRun6_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(0x031F)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x06);
        if (fc) lda8(0x00);
        wr8(ABSX(0x031F), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        if (fz) object_rest(2, 0xBB00);
        else object_frame(0xBB17, 0xBB1D);
    }
    ret();
}

/* $81:BEF5 ObjectRunOnce6 (RTS): step 1-5 through the tables; at 6 rest in
 * state 4 (the step stays at 6). */
void ObjectRunOnce6_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(0x031F)));
        lda8(inc8((u8)A));
        wr8(ABSX(0x031F), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        cmp8((u8)A, 0x06);
        if (fc) object_rest(4, 0xBF11);
        else object_frame(0xBF28, 0xBF2E);
    }
    ret();
}

/* $81:BC7F ObjectLoop7 (RTS): step 1-6 through the tables; at 7 back to
 * step 0 with a random pause of $40-$7F frames as its delay. */
void ObjectLoop7_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(0x031F)));
        lda8(inc8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        wr8(ABSX(0x031F), (u8)A);
        cmp8((u8)A, 0x07);
        if (fc) {
            ldy8(0x00);
            wr8(ABSX(0x031F), 0);
            random_rest(0xBC9B);
            wr8(ABSX(0x036F), (u8)A);
        } else {
            lda8(rd8(ABSY(0xBCB3))); wr8(ABSX(0x036F), (u8)A);
        }
        lda8(rd8(ABSY(0xBCBA))); wr8(ABSX(0x034F), (u8)A);
    }
    ret();
}

/* $81:BB40 ObjectLoop5 (RTS): step 1-4 through the tables; at 5 back to
 * step 1 with the delay from the table plus a random 0-$3F. */
void ObjectLoop5_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(0x031F)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x05);
        if (fc) {
            lda8(0x01); wr8(ABSX(0x031F), 0x01);
            Y = 0x01; nz8(1);
            push8(0x81); push16(0xBB59); RandomB_m1x1();
            and8(0x3F);
            fc = 0;
            adc8(rd8(ABSY(0xBB76)));
            wr8(ABSX(0x036F), (u8)A);
        } else {
            wr8(ABSX(0x031F), (u8)A);
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0xBB76))); wr8(ABSX(0x036F), (u8)A);
        }
        lda8(rd8(ABSY(0xBB7B))); wr8(ABSX(0x034F), (u8)A);
    }
    ret();
}

/* $81:BE53 ObjectFlicker (RTS): every 10 frames flip the frame bit; when
 * the pause $037F is over start the next state (delay 9, random count 1-4,
 * frame 8). */
void ObjectFlicker_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(0x034F)));
        eor8(0x01);
        wr8(ABSX(0x034F), (u8)A);
        lda8(0x0A); wr8(ABSX(0x036F), 0x0A);
    }
    lda8(rd8(ABSX(0x037F)));
    if (fz) {
        lda8(0x0A); wr8(ABSX(0x036F), 0x0A);
        lda8(0x09); wr8(ABSX(0x036F), 0x09);
        push8(0x81); push16(0xBE7A); RandomB_m1x1();
        and8(0x03);
        lda8(inc8((u8)A));
        wr8(ABSX(0x032F), (u8)A);
        wr8(ABSX(0x031F), 0);
        wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
        lda8(0x08); wr8(ABSX(0x034F), 0x08);
    }
    ret();
}

/* Wheel animation for riders X..0: while moving, count $018E down by the
 * speed $0186 and on each wrap (+8) step the wheel frame $0266 (0-63);
 * clear $0246. */
static void spin_wheels(void)
{
    do {
        lda8(rd8(ABSX(0x0186)));
        if (!fz) {
            lda8(rd8(ABSX(0x018E)));
            fc = 1;
            sbc8(rd8(ABSX(0x0186)));
            if (!fz && !fn) {
                wr8(ABSX(0x018E), (u8)A);
            } else {
                fc = 0;
                adc8(0x08);
                wr8(ABSX(0x018E), (u8)A);
                wr8(ABSX(0x0266), inc8(rd8(ABSX(0x0266))));
                lda8(rd8(ABSX(0x0266)));
                and8(0x3F);
                wr8(ABSX(0x0266), (u8)A);
            }
        }
        lda8(0x00);
        wr8(ABSX(0x0246), 0x00);
        X = dec8((u8)X);
    } while (!fn);
}

/* $80:F757 SpinWheels (RTS, riders X..0), $81:C579 SpinWheels8 (RTS,
 * riders 7..0). */
void SpinWheels_m1x1(void)  { spin_wheels(); ret(); }
void SpinWheels8_m1x1(void) { ldx8(0x07); spin_wheels(); ret(); }

/* $80:B736 EndJumpMode (RTS): once the frame count reaches $0708, before
 * broadcast minute $28 go to the course preview (mode $18, course 2),
 * from it on to mode $10; $86/$87 = 0 (JMP $80:8F17). */
void EndJumpMode_m1x1(void)
{
    rep(0x20);
    lda16(rd16(LONG(0x7FFFFA)));
    cmp16(A, 0x0708);
    sep(0x20);
    if (!fc) { ret(); return; }
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x28);
    if (!fc) {
        lda8(0x18); wr8(DP(0x76), 0x18);
        wr8(DP(0x86), 0);
        wr8(DP(0x87), 0);
        lda8(0x02); wr8(DP(0x7C), 0x02);
        wr8(DP(0x7D), 0);
    } else {
        wr8(DP(0x86), 0);
        wr8(DP(0x87), 0);
        lda8(0x10); wr8(DP(0x76), 0x10);
    }
    r816_call_pc(0x808F17);
}

/* $80:FF0C QueueBonusDigits (RTS): six digits $0CE5-$0CEA as a 6x2 tile
 * block at $0CEB (leading zeros blank $FF, the last digit always shown;
 * the lower row is digit | $10), size $26, then VramQueueTiles. */
void QueueBonusDigits_m1x1(void)
{
    ldx8(0x00);
    Y = X; nz8((u8)Y);
    do {
        lda8(rd8(ABSX(0x0CE5)));
        if (fz && Y == 0 && X != 5) {
            lda8(0xFF);
        } else {
            Y = inc8((u8)Y);
            lda8(rd8(ABSX(0x0CE5)));
        }
        wr8(ABSX(0x0CEC), (u8)A);
        ora8(0x10);
        wr8(ABSX(0x0CF2), (u8)A);
        X = inc8((u8)X);
        cmp8((u8)X, 0x06);
    } while (!fc);
    lda8(0x26); wr8(ABS(0x0CEB), 0x26);
    lda8(0xEB); wr8(DP(0x02), 0xEB);
    lda8(0x0C); wr8(DP(0x03), 0x0C);
    VramQueueTiles_m1x1();
}

/* $82:811D SetupHdmaChannels (RTL): HDMA channel 7 to register $0F (BG2
 * horizontal scroll) from the table at $82:815A and channel 6 to $0D (BG1)
 * from $82:8176, both mode $42 (indirect, two writes once), data bank 0. */
void SetupHdmaChannels_m1x1(void)
{
    lda8(0x42); wr8(ABS(0x4370), 0x42);
    lda8(0x0F); wr8(ABS(0x4371), 0x0F);
    lda8(0x5A); wr8(ABS(0x4372), 0x5A);
    lda8(0x81); wr8(ABS(0x4373), 0x81);
    lda8(0x82); wr8(ABS(0x4374), 0x82);
    lda8(0x00); wr8(ABS(0x4377), 0x00);
    lda8(0x42); wr8(ABS(0x4360), 0x42);
    lda8(0x0D); wr8(ABS(0x4361), 0x0D);
    lda8(0x76); wr8(ABS(0x4362), 0x76);
    lda8(0x81); wr8(ABS(0x4363), 0x81);
    lda8(0x82); wr8(ABS(0x4364), 0x82);
    lda8(0x00); wr8(ABS(0x4367), 0x00);
    retl();
}

/* $80:B059 RiderAhead (RTS): is another rider (from $7E up to 7, not $0F)
 * in the air state 1 ($019E), in lane $02 ($0958) and between
 * $0196,X + table[Y] - table[Y+1] and $0196,X + table[Y] (tables at
 * $80:B88B)? C = 1 with Y = that rider, else C = 0. */
void RiderAhead_m1x1(void)
{
    lda8(rd8(ABSX(0x0196)));
    fc = 0;
    adc8(rd8(ABSY(0xB88B)));
    wr8(DP(0x00), (u8)A);
    fc = 1;
    sbc8(rd8(ABSY(0xB88C)));
    wr8(DP(0x01), (u8)A);
    Y = rd8(DP(0x7E)); nz8((u8)Y);
    do {
        cmp8((u8)Y, rd8(DP(0x0F)));
        if (fz) goto next;
        lda8(rd8(ABSY(0x019E)));
        cmp8((u8)A, 0x01);
        if (!fz) goto next;
        lda8(rd8(ABSY(0x0958)));
        if (fz) goto next;
        cmp8((u8)A, rd8(DP(0x02)));
        if (!fz) goto next;
        lda8(rd8(ABSY(0x0196)));
        cmp8((u8)A, rd8(DP(0x00)));
        if (fc) goto next;
        cmp8((u8)A, rd8(DP(0x01)));
        if (fc) { ret(); return; }
    next:
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x08);
    } while (!fc);
    fc = 0;
    ret();
}

/* $80:D1E6 DrawRiderMarker (RTL): sprite X + 1 (OAM buffer $1600) at
 * $1500,X / $1508,X with the blinking tile from $80:D218 (by $63 bits 2-3),
 * attribute $21, size bit 2. */
void DrawRiderMarker_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x0C);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xD218)));
    wr8(DP(0x00), (u8)A);
    lda8((u8)X);
    lda8(asl8((u8)A));
    fc = 0;
    adc8(0x04);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(DP(0x00))); wr8(ABSY(0x1602), (u8)A);
    lda8(rd8(ABSX(0x1500))); wr8(ABSY(0x1600), (u8)A);
    lda8(rd8(ABSX(0x1508))); wr8(ABSY(0x1601), (u8)A);
    lda8(0x21); wr8(ABSY(0x1603), 0x21);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(0x1820), 0x02);
    retl();
}

/* $81:9555 StepFallingSprite (RTS, object Y): every 4 frames move it up
 * and left ($0106, $010E) and step its tile $011E; at tile $9F switch it
 * off ($0116 = 0). While racing ($01CE and $84 clear) write it to the OAM
 * buffer $1610 + 2Y, attribute $2D. */
void StepFallingSprite_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (fz) {
        wr8(ABSX(0x0106), dec8(rd8(ABSX(0x0106))));
        wr8(ABSX(0x010E), dec8(rd8(ABSX(0x010E))));
        wr8(ABSX(0x011E), dec8(rd8(ABSX(0x011E))));
        lda8(rd8(ABSX(0x011E)));
        cmp8((u8)A, 0x9F);
        if (fz) { wr8(ABSX(0x0116), 0); ret(); return; }
    }
    lda8(rd8(ABS(0x01CE)));
    ora8(rd8(DP(0x84)));
    if (fz) {
        lda8((u8)X);
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0x0106))); wr8(ABSY(0x1610), (u8)A);
        lda8(rd8(ABSX(0x010E))); wr8(ABSY(0x1611), (u8)A);
        lda8(rd8(ABSX(0x011E))); wr8(ABSY(0x1612), (u8)A);
        lda8(0x2D); wr8(ABSY(0x1613), 0x2D);
    }
    ret();
}

/* $80:EB83 DrawCountdownLights (RTS): while $C3 is on (until the timer
 * $8E runs out) draw six sprites in a row at X $50 + 16n, Y $50, tiles
 * from $80:EBB6, attribute $31. */
void DrawCountdownLights_m1x1(void)
{
    lda8(rd8(DP(0xC3)));
    if (fz) { ret(); return; }
    u8 t = dec8(rd8(DP(0x8E)));
    wr8(DP(0x8E), t);
    if (fz) wr8(DP(0xC3), 0);
    push8((u8)X);
    ldx8(0x05);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        fc = 0;
        adc8(0x50);
        wr8(ABSY(0x1600), (u8)A);
        lda8(0x50); wr8(ABSY(0x1601), 0x50);
        lda8(rd8(ABSX(0xEBB6))); wr8(ABSY(0x1602), (u8)A);
        lda8(0x31); wr8(ABSY(0x1603), 0x31);
        lda8(0x02); wr8(ABSX(0x1820), 0x02);
        X = dec8((u8)X);
    } while (!fn);
    X = pull8(); nz8((u8)X);
    ret();
}

/* $81:BE8E ObjectRepeat4Rest (RTS): count the step 0-3; each time it
 * wraps count $032F down; at zero rest in state 2 with delay 10 and
 * frame 6. */
void ObjectRepeat4Rest_m1x1(void)
{
    if (object_ready()) {
        lda8(rd8(ABSX(0x031F)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(0x031F), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        int rest = 0;
        if (fz) {
            u8 n = dec8(rd8(ABSX(0x032F)));
            wr8(ABSX(0x032F), n);
            rest = fz;
        }
        if (rest) {
            lda8(0x02); wr8(ABSX(0x030F), 0x02);
            lda8(0x0A); wr8(ABSX(0x036F), 0x0A);
            lda8(0x06); wr8(ABSX(0x034F), 0x06);
            random_rest(0xBEB9);
            wr8(ABSX(0x037F), (u8)A);
        } else {
            object_frame(0xBED0, 0xBED4);
        }
    }
    ret();
}

/* $80:CE1F SortRidersByPosition (RTS): $0950-$0957 = rider numbers sorted
 * by their position $01F6 (the player uses $097C when $08C7 is set),
 * largest first (bubble sort through $00-$07). */
void SortRidersByPosition_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        wr8(ABSX(0x0950), (u8)A);
        lda8(rd8(ABSX(0x01F6)));
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

/* $80:DDAE QueueResultsCount (RTS): the number $10 as two digits (tens
 * blank when zero) in a 2x2 block at $03CF (size $22), queued at VRAM
 * $18B9 (VramQueueTiles20). */
void QueueResultsCount_m1x1(void)
{
    rep(0x20);
    lda16(0x18B9); wr16(DP(0x00), 0x18B9);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x22); wr8(ABS(0x03CF), 0x22);
    ldy8(0x00);
    lda8(rd8(DP(0x10)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(0x03D1), (u8)A);
    ora8(0x10);
    wr8(ABS(0x03D3), (u8)A);
    lda8((u8)Y);
    wr8(ABS(0x03D0), (u8)A);
    ora8(0x10);
    wr8(ABS(0x03D2), (u8)A);
    cmp8((u8)Y, 0x00);
    if (fz) {
        lda8(0xFF);
        wr8(ABS(0x03D0), 0xFF);
        wr8(ABS(0x03D2), 0xFF);
    }
    push16(0xDDED); VramQueueTiles20_m1x1();
    ret();
}

void AddScore_m1x1(void);

/* Countdown sign: while $C3 (1-4) is on, copy its 8-byte sprite set from
 * `table` + ($C3 - 1) * 8 to OAM buffer $16A0-$16A7 (both large). At 4
 * it stays up until the timer $8A runs out. */
static void draw_countdown_sign(u16 table)
{
    lda8(rd8(DP(0xC3)));
    if (fz) return;
    cmp8((u8)A, 0x04);
    if (fz) {
        lda8(rd8(DP(0x8A)));
        if (fz) return;
        wr8(DP(0x8A), dec8(rd8(DP(0x8A))));
    }
    push8((u8)X);
    lda8(rd8(DP(0xC3)));
    lda8(dec8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
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
    lda16(0x0202);
    wr16(ABSY(0x1820), 0x0202);
    sep(0x20);
    X = pull8(); nz8((u8)X);
}

/* $80:AA94, $80:D914 DrawCountdownSign/B (RTS): tables $80:AAC8, $80:D948. */
void DrawCountdownSign_m1x1(void)  { draw_countdown_sign(0xAAC8); ret(); }
void DrawCountdownSignB_m1x1(void) { draw_countdown_sign(0xD948); ret(); }

/* $80:C131 StartCrash (RTS): rider X falls (state $01EE = 2, timer $08E4
 * = $0F) with a bounce speed $08D4/$08DC = its speed + $AF (halved when
 * $08F4 is 2). For the player also clear $09C7 and play sound $20. */
void StartCrash_m1x1(void)
{
    lda8(0x02); wr8(ABSX(0x01EE), 0x02);
    lda8(0x0F); wr8(ABSX(0x08E4), 0x0F);
    lda8(rd8(ABSX(0x01BE)));
    fc = 0;
    adc8(0xAF);
    wr8(ABSX(0x08D4), (u8)A);
    lda8(rd8(ABSX(0x01C6)));
    adc8(0x00);
    wr8(ABSX(0x08DC), (u8)A);
    lda8(rd8(ABSX(0x08F4)));
    cmp8((u8)A, 0x02);
    if (fz) {
        wr8(ABSX(0x08DC), lsr8(rd8(ABSX(0x08DC))));
        wr8(ABSX(0x08D4), ror8(rd8(ABSX(0x08D4))));
    }
    lda8((u8)X);
    if (!fz) { ret(); return; }
    wr8(ABS(0x09C7), (u8)A);
    lda8(0x20);
    push8(0x80); push16(0xC164); QueueSoundB_m1x1();
    lda8(rd8(ABSX(0x01C6)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    eor8(0x30);
    if (fz) lda8(0x08);
    ret();
}

/* $80:DCE6 AddBonusPoints (RTS): when the result $09C5 (forced 0 when
 * $1990 >= 2) equals the target $09C9 add 1 to $1E92 ($09AD = 1); else
 * split it into tens $09AE / units $09AF and add them to $1E93 / $1E94.
 * Then AddScore. */
void AddBonusPoints_m1x1(void)
{
    lda8(rd8(ABS(0x1990)));
    lda8(lsr8((u8)A));
    if (!fz) wr8(ABS(0x09C5), 0);
    wr8(DP(0x00), 0);
    lda8(rd8(ABS(0x09C5)));
    cmp8((u8)A, rd8(ABS(0x09C9)));
    if (fz) {
        lda8(0x01); wr8(ABS(0x09AD), 0x01);
        fc = 0;
        adc8(rd8(ABS(0x1E92)));
        wr8(ABS(0x1E92), (u8)A);
    } else {
        for (;;) {
            cmp8((u8)A, 0x0A);
            if (!fc) break;
            sbc8(0x0A);
            wr8(DP(0x00), inc8(rd8(DP(0x00))));
            WAIT_TICK();
        }
        wr8(ABS(0x09AF), (u8)A);
        fc = 0;
        adc8(rd8(ABS(0x1E94)));
        wr8(ABS(0x1E94), (u8)A);
        lda8(rd8(DP(0x00)));
        wr8(ABS(0x09AE), (u8)A);
        fc = 0;
        adc8(rd8(ABS(0x1E93)));
        wr8(ABS(0x1E93), (u8)A);
    }
    push16(0xDD29); AddScore_m1x1();
    ret();
}

/* $80:C057 SetRiderLean (RTS): rider X's lean $08B4 from the table at
 * $80:BA92 by its pose $01A6 ($0A for the player standing still on the
 * jump with $84 = 1; 6 while $020E,X is 1). */
void SetRiderLean_m1x1(void)
{
    Y = rd8(ABSX(0x01A6)); nz8((u8)Y);
    lda8(rd8(DP(0x84)));
    cmp8((u8)A, 0x01);
    int table = 1;
    if (fz) {
        lda8((u8)X);
        ora8(rd8(ABS(0x01CE)));
        ora8(rd8(ABS(0x01D6)));
        ora8(rd8(ABS(0x0176)));
        if (fz) { lda8(0x0A); table = 0; }
    }
    if (table) lda8(rd8(ABSY(0xBA92)));
    wr8(ABSX(0x08B4), (u8)A);
    lda8(rd8(ABSX(0x020E)));
    cmp8((u8)A, 0x01);
    if (fz) { lda8(0x06); wr8(ABSX(0x08B4), 0x06); }
    ret();
}

/* $80:CF9D QueuePlaceBadge (RTS): the place $086E as a 2x2 block at $0CF8
 * (digit tiles $60 + 2(place - 1), suffix tiles from $80:CFE2), size $24,
 * then VramQueueTiles. */
void QueuePlaceBadge_m1x1(void)
{
    lda8(rd8(ABS(0x086E)));
    lda8(dec8((u8)A));
    lda8(asl8((u8)A));
    push8((u8)A);
    fc = 0;
    adc8(0x60);
    wr8(ABS(0x0CF9), (u8)A);
    lda8(inc8((u8)A));
    wr8(ABS(0x0CFA), (u8)A);
    fc = 0;
    adc8(0x0F);
    wr8(ABS(0x0CFD), (u8)A);
    lda8(inc8((u8)A));
    wr8(ABS(0x0CFE), (u8)A);
    lda8(pull8());
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xCFE2))); wr8(ABS(0x0CFB), (u8)A);
    lda8(rd8(ABSX(0xCFE3))); wr8(ABS(0x0CFC), (u8)A);
    lda8(rd8(ABSX(0xCFE4))); wr8(ABS(0x0CFF), (u8)A);
    lda8(rd8(ABSX(0xCFE5))); wr8(ABS(0x0D00), (u8)A);
    lda8(0x24); wr8(ABS(0x0CF8), 0x24);
    lda8(0xF8); wr8(DP(0x02), 0xF8);
    lda8(0x0C); wr8(DP(0x03), 0x0C);
    VramQueueTiles_m1x1();
}

/* $80:F1D6 SetJumpScroll (RTL): from the player's height $01B6 and
 * 2 - $0904: below $0130 the vertical scroll $9E = value - $30, $A2 =
 * (value - $0130) / 2 + $8100 and $9B = $9E low; otherwise $9E = $A2 =
 * $0100, $9B = 0. */
void SetJumpScroll_m1x1(void)
{
    lda8(rd8(ABS(0x0904)));
    eor8(0xFF);
    lda8(inc8((u8)A));
    lda8(inc8((u8)A));
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABS(0x01B6)));
    wr8(DP(0x00), (u8)A);
    rep(0x20);
    lda16(rd16(DP(0x00)));
    cmp16(A, 0x0130);
    if (fc) {
        lda16(0x0100);
        wr16(DP(0x9E), 0x0100);
        wr16(DP(0xA2), 0x0100);
        sep(0x20);
        wr8(DP(0x9B), 0);
        retl();
        return;
    }
    fc = 1;
    sbc16(0x0130);
    push16(A);
    fc = 0;
    adc16(0x0100);
    wr16(DP(0x9E), A);
    lda16(pull16());
    lda16(lsr16(A));
    fc = 0;
    adc16(0x8100);
    wr16(DP(0xA2), A);
    sep(0x20);
    lda8(rd8(DP(0x9E)));
    wr8(DP(0x9B), (u8)A);
    retl();
}

/* $81:96F7 DrawSpark (RTS, object Y): sprite at OAM buffer $1610,Y from
 * $0106/$010E, tile and attribute from $81:966F/$81:9673 by the frame
 * $011E (frame 0 once $0146 reaches $F0); every $0126 frames restart the
 * frame ($012E = 1, $0126 = 4). */
void DrawSpark_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(ABSX(0x0106))); wr8(ABSY(0x1610), (u8)A);
    lda8(rd8(ABSX(0x010E))); wr8(ABSY(0x1611), (u8)A);
    lda8(rd8(ABSX(0x0146)));
    cmp8((u8)A, 0xF0);
    if (fc) lda8(0x00);
    else lda8(rd8(ABSX(0x011E)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x81966F))); wr8(ABSY(0x1612), (u8)A);
    lda8(rd8(LONGX(0x819673))); wr8(ABSY(0x1613), (u8)A);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(0x1824), 0x02);
    u8 t = dec8(rd8(ABSX(0x0126)));
    wr8(ABSX(0x0126), t);
    if (fz) {
        lda8(0x01); wr8(ABSX(0x012E), 0x01);
        lda8(0x04); wr8(ABSX(0x0126), 0x04);
        wr8(ABSX(0x011E), 0);
    }
    ret();
}

void SetupHdmaChannels_m1x1(void);

/* $80:9D37 StartPracticeMode (RTS): BG1SC = $1900 = 1, BG2SC 9, $1905 =
 * $17, $86/$87 = 0, the HDMA scroll channels, BG3SC $2B, BG12NBA $44,
 * BG34NBA 1, step $1923 and $75, timer $88 = $0708, $190D = $E3. */
void StartPracticeMode_m1x1(void)
{
    lda8(0x01); wr8(ABS(0x2107), 0x01); wr8(ABS(0x1900), 0x01);
    lda8(0x09); wr8(ABS(0x2108), 0x09);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    wr8(DP(0x86), 0);
    wr8(DP(0x87), 0);
    push8(0x80); push16(0x9D50); SetupHdmaChannels_m1x1();
    lda8(0x2B); wr8(ABS(0x2109), 0x2B);
    lda8(0x44); wr8(ABS(0x210B), 0x44);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    wr8(ABS(0x1923), inc8(rd8(ABS(0x1923))));
    wr8(DP(0x75), inc8(rd8(DP(0x75))));
    rep(0x20);
    lda16(0x0708); wr16(DP(0x88), 0x0708);
    sep(0x20);
    lda8(0xE3); wr8(ABS(0x190D), 0xE3);
    ret();
}

/* $80:E771 DrawJumpRank (RTS): in jump state 2 ($C7) show the rank sprite
 * (tile from $80:E7BB by $156B, negative ranks as 0) at X $38: it rises
 * from Y $A0 to $80, then stays for $40 frames ($8E). */
void DrawJumpRank_m1x1(void)
{
    lda8(rd8(DP(0xC7)));
    cmp8((u8)A, 0x02);
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0xD2)));
    if (fz) {
        lda8(rd8(ABS(0x156B)));
        if (fn) wr8(ABS(0x156B), 0);
        wr8(DP(0xD2), inc8(rd8(DP(0xD2))));
        lda8(0xA0); wr8(DP(0xD3), 0xA0);
        lda8(0x40); wr8(DP(0x8E), 0x40);
    }
    lda8(rd8(DP(0xD3)));
    cmp8((u8)A, 0x80);
    if (fc) {
        wr8(DP(0xD3), dec8(rd8(DP(0xD3))));
    } else {
        lda8(rd8(DP(0x8E)));
        if (fz) { ret(); return; }
        wr8(DP(0x8E), dec8(rd8(DP(0x8E))));
    }
    lda8(0x38); wr8(ABS(0x1700), 0x38);
    lda8(rd8(DP(0xD3))); wr8(ABS(0x1701), (u8)A);
    Y = rd8(ABS(0x156B)); nz8((u8)Y);
    lda8(rd8(ABSY(0xE7BB))); wr8(ABS(0x1702), (u8)A);
    lda8(0x21); wr8(ABS(0x1703), 0x21);
    lda8(0x02); wr8(ABS(0x1860), 0x02);
    ret();
}

/* $82:8698 StepFade (RTL): brightness $1903 low nibble += fade step
 * $1923, clamped at $0F (ending the fade). Below zero the screen is dark:
 * reset the scroll, sound and race flags, play sound $80, $1903 = $80. */
void StepFade_m1x1(void)
{
    lda8(rd8(ABS(0x1903)));
    and8(0x0F);
    fc = 0;
    adc8(rd8(ABS(0x1923)));
    if (fn) {
        wr8(DP(0x9A), 0); wr8(DP(0x9B), 0); wr8(DP(0x9C), 0); wr8(DP(0x9D), 0);
        wr8(ABS(0x1958), 0);
        wr8(ABS(0x1E0C), 0);
        lda8(0x80);
        push8(0x82); push16(0x86BE); QueueSoundB_m1x1();
        lda8(0x01); wr8(ABS(0x1E84), 0x01);
        lda8(0x80); wr8(ABS(0x1903), 0x80);
        wr8(ABS(0x2142), 0);
        for (u16 a = 0x1961; a <= 0x1964; a++)
            wr8(ABS(a), 0);
        wr8(ABS(0x00CA), 0);
        wr8(ABS(0x0086), 0);
        wr8(ABS(0x0087), 0);
        wr8(ABS(0x1E0C), 0);
    } else {
        cmp8((u8)A, 0x0F);
        if (!fc) { wr8(ABS(0x1903), (u8)A); retl(); return; }
        lda8(0x0F);
    }
    wr8(ABS(0x1923), 0);
    wr8(ABS(0x1903), (u8)A);
    retl();
}

/* $80:B7A0 Mode17_BonusIntro (RTS): count the word timer $88 down (sound
 * $34 at $06CC); below that zoom $A6 up by 4 to $0100, then set colour
 * math ($190D $41, COLDATA $E0) and wait for a press. At zero or on a
 * press leave the mode (JMP $80:8F15); otherwise $1905 = $17. */
void Mode17_BonusIntro_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(0x0088)));
    lda16(dec16(A));
    wr16(ABS(0x0088), A);
    if (fz) goto leave;
    cmp16(A, 0x06CC);
    if (fz) {
        sep(0x20);
        lda8(0x34);
        push8(0x80); push16(0xB7BB); QueueSoundB_m1x1();
    } else if (fc) {
        goto keep;
    }
    rep(0x20);
    lda16(rd16(DP(0xA6)));
    cmp16(A, 0x0100);
    if (fz) {
        sep(0x20);
        lda8(0x41); wr8(ABS(0x190D), 0x41);
        lda8(0xE0); wr8(ABS(0x2132), 0xE0);
        lda8(rd8(DP(0x21)));
        ora8(rd8(DP(0x26)));
        if (!fz) goto leave;
        ret();
        return;
    }
    fc = 0;
    adc16(0x0004);
    wr16(DP(0xA6), A);
keep:
    sep(0x20);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    ret();
    return;
leave:
    sep(0x20);
    r816_call_pc(0x808F15);
}

/* $80:BC5B RiderAtStart (RTS): rider X at the start line ($3D,X clear):
 * riders other than the player are flagged as started ($01AE); once a
 * rider moves (or is off its start column $80:BACF) it is flagged too,
 * and riders from column 8 on (the player always) clear $01D6. A rival
 * not in the air also gets $199E = 1 and a random boost from $80:F6AB. */
void RiderAtStart_m1x1(void)
{
    lda8(rd8(DPX(0x3D)));
    if (!fz) { ret(); return; }
    lda8((u8)X);
    if (!fz) { lda8(0x01); wr8(ABSX(0x01AE), 0x01); }
    lda8(rd8(ABSX(0x01C6)));
    ora8(rd8(ABSX(0x01AE)));
    if (fz) {
        lda8(rd8(ABSX(0x01F6)));
        cmp8((u8)A, rd8(ABSX(0xBACF)));
        if (fz) { ret(); return; }
    }
    lda8(0x01); wr8(ABSX(0x01AE), 0x01);
    lda8((u8)X);
    if (!fz) {
        lda8(rd8(ABSX(0x01F6)));
        cmp8((u8)A, 0x08);
        if (!fc) { ret(); return; }
    }
    lda8(0x00); wr8(ABSX(0x01D6), 0x00);
    cmp8((u8)X, 0x00);
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x019E)));
    cmp8((u8)A, 0x01);
    if (fz) { ret(); return; }
    lda8(0x01); wr8(ABSX(0x199E), 0x01);
    push8(0x80); push16(0xBC9E); RandomB_m1x1();
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF6AB))); wr8(ABSX(0x19A6), (u8)A);
    ret();
}

/* $81:9677 DrawStar (RTS, object Y): sprite at OAM buffer $1610 + 2Y
 * from $0106/$010E, tile from $81:96BF by frame $011E, attribute $2B;
 * every 4 frames step the frame, and after frame 2 move on ($012E + 1)
 * with a $40-frame wait. */
void DrawStar_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(0x0106))); wr8(ABSY(0x1610), (u8)A);
    lda8(rd8(ABSX(0x010E))); wr8(ABSY(0x1611), (u8)A);
    lda8(rd8(ABSX(0x011E)));
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x8196BF))); wr8(ABSY(0x1612), (u8)A);
    lda8(0x2B); wr8(ABSY(0x1613), 0x2B);
    X = pull8(); nz8((u8)X);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(0x1824), 0x02);
    u8 t = dec8(rd8(ABSX(0x0126)));
    wr8(ABSX(0x0126), t);
    if (fz) {
        lda8(0x04); wr8(ABSX(0x0126), 0x04);
        wr8(ABSX(0x011E), inc8(rd8(ABSX(0x011E))));
        lda8(rd8(ABSX(0x011E)));
        cmp8((u8)A, 0x03);
        if (fc) {
            wr8(ABSX(0x012E), inc8(rd8(ABSX(0x012E))));
            lda8(0x40); wr8(ABSX(0x0126), 0x40);
        }
    }
    ret();
}

/* $81:9436 SpawnDustPuff (RTL): find a free effect slot (6, 4, 2 by
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
        lda8(rd8(ABSX(0x0116)));
        if (fz) {
            Y = rd8(ABS(0x1961)); nz8((u8)Y);
            lda8(rd8(ABSY(0x9476)));
            if (!fn) {
                fc = 1; sbc8(0x03);
                fc = 0; adc8(rd8(ABS(0x0196)));
                wr8(ABSX(0x0106), (u8)A);
                lda8(rd8(ABSY(0x9496)));
                fc = 1; sbc8(0x03);
                fc = 0; adc8(rd8(ABS(0x01B6)));
                wr8(ABSX(0x010E), (u8)A);
                lda8(0xA7); wr8(ABSX(0x011E), 0xA7);
                lda8(0x01); wr8(ABSX(0x0116), 0x01);
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

/* $80:8FA6 ClearRaceState (RTS): zero $0884-$09B1, $09B2-$0CD3, $80-$84
 * and $197A-$19FF, with sound $2D in between. */
void ClearRaceState_m1x1(void)
{
    rep(0x10);
    for (X = 0x0884; X < 0x09B2; X++)
        wr8(DPX(0x00), 0);
    cmp16(X, 0x09B2);
    sep(0x10);
    lda8(0x09); wr8(DP(0x01), 0x09);
    lda8(0xB2); wr8(DP(0x00), 0xB2);
    rep(0x10);
    lda8(0x00);
    for (int y = 0x0321; y >= 0; y--)
        wr8((((u32)DB << 16) + rd16(DP(0x00)) + y) & 0xFFFFFF, 0);
    Y = 0xFFFF; nz16(Y);
    sep(0x10);
    ldx8(0x03);
    do {
        wr8(DPX(0x80), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(0x84), (u8)A);
    lda8(0x2D);
    push8(0x80); push16(0x8FD9); QueueSoundB_m1x1();
    rep(0x10);
    lda8(0x7A); wr8(DP(0x00), 0x7A);
    lda8(0x19); wr8(DP(0x01), 0x19);
    lda8(0x00);
    for (int y = 0x0085; y >= 0; y--)
        wr8((((u32)DB << 16) + rd16(DP(0x00)) + y) & 0xFFFFFF, 0);
    Y = 0xFFFF; nz16(Y);
    sep(0x10);
    ret();
}

/* Two decimal digits of `v`: the units ORed into `lo`, the tens into
 * `hi`. */
static void or_digits(u8 v, u16 hi, u16 lo)
{
    ldy8(0x00);
    lda8(v);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    ora8(rd8(ABS(lo)));
    wr8(ABS(lo), (u8)A);
    lda8((u8)Y);
    ora8(rd8(ABS(hi)));
    wr8(ABS(hi), (u8)A);
}

/* $80:A129 ReadBroadcastDate (RTL): merge the three BS-X bytes at
 * $11:572C-$11:572E as two-digit pairs into $0854-$0859. */
void ReadBroadcastDate_m1x1(void)
{
    or_digits(rd8(LONG(0x11572C)), 0x0854, 0x0855);
    or_digits(rd8(LONG(0x11572D)), 0x0856, 0x0857);
    or_digits(rd8(LONG(0x11572E)), 0x0858, 0x0859);
    retl();
}

/* $81:9744 MoveObjectX (RTL): add the signed velocity nibbles of $013E,X
 * (low nibble as fraction, high nibble as whole pixels) to the 8.8
 * position $014E/$0106-$0107. Returns A = carry + whole step. */
void MoveObjectX_m1x1(void)
{
    lda8(rd8(ABSX(0x013E)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABSX(0x013E)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    cmp8((u8)A, 0x08);
    if (fc) ora8(0xF0);
    wr8(DP(0x00), (u8)A);
    ldy8(0x00);
    cmp8((u8)A, 0x00);
    if (fn) Y = dec8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    lda8(rd8(ABSX(0x014E)));
    fc = 0;
    adc8(rd8(DP(0x01)));
    wr8(ABSX(0x014E), (u8)A);
    lda8(0x00);
    lda8(rol8((u8)A));
    push8((u8)A);
    lda8(ror8((u8)A));
    lda8(rd8(ABSX(0x0106)));
    adc8(rd8(DP(0x00)));
    wr8(ABSX(0x0106), (u8)A);
    lda8(rd8(ABSX(0x0107)));
    adc8(rd8(DP(0x02)));
    wr8(ABSX(0x0107), (u8)A);
    lda8(pull8());
    fc = 0;
    adc8(rd8(DP(0x00)));
    retl();
}

/* Time display "M'SS''FF" at $03CF (size $27) from minutes `m`, seconds
 * `s` and frames `f`: digits via the tables $80:C470/$80:C4AC, separator
 * tiles $47/$48 and the lower row (+$10). `last_add` picks the final
 * operation for the last lower tile (ORA or ADC $10). */
static void build_time(u16 m, u16 s, u16 f, int last_add)
{
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    X = rd8(ABS(f)); nz8((u8)X);
    lda16(rd16(ABSX(0xC470)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC4AC))); wr16(ABS(0x03D5), A);
    lda16(rd16(ABS(s)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC4AC))); wr16(ABS(0x03D2), A);
    sep(0x20);
    lda8(rd8(ABS(m))); wr8(ABS(0x03D0), (u8)A);
    lda8(0x27); wr8(ABS(0x03CF), 0x27);
    rep(0x20);
    lda16(rd16(ABS(0x03D0)));
    and16(0x00FF);
    ora16(0x4700);
    wr16(ABS(0x03D0), A);
    ora16(0xFF10);
    wr16(ABS(0x03D7), A);
    lda16(rd16(ABS(0x03D2)));
    ora16(0x1010);
    wr16(ABS(0x03D9), A);
    lda16(rd16(ABS(0x03D4)));
    and16(0xFF00);
    ora16(0x0048);
    wr16(ABS(0x03D4), A);
    ora16(0x10FF);
    wr16(ABS(0x03DB), A);
    sep(0x20);
    lda8(rd8(ABS(0x03D6)));
    if (last_add) { fc = 0; adc8(0x10); } else ora8(0x10);
    wr8(ABS(0x03DD), (u8)A);
}

/* $80:C5B0 QueueRaceTime (RTS): the race clock $03AE-$03B0, then
 * VramQueueTiles. $80:C623 QueueBestTime (RTS): the record $083A-$083C,
 * then VramQueueTiles20. */
void QueueRaceTime_m1x1(void) { build_time(0x03AE, 0x03AF, 0x03B0, 0); VramQueueTiles_m1x1(); }
void QueueBestTime_m1x1(void) { build_time(0x083A, 0x083B, 0x083C, 1); VramQueueTiles20_m1x1(); }

/* $80:AE33 AccelerateRider (RTS): the player's ($7F) speed $01BE/$01C6
 * gains the step from $80:B864 (or $80:B86D in a race below page 2),
 * capped at $80:B883/$80:B886 for rider 0. A rider with $019E negative
 * is set to speed $0480 ($0580 when the player has not crashed). */
void AccelerateRider_m1x1(void)
{
    cmp8((u8)X, rd8(DP(0x7F)));
    if (fz) {
        lda8(rd8(DP(0x76)));
        cmp8((u8)A, 0x03);
        int alt = 0;
        if (fz) {
            lda8(rd8(DP(0x9D)));
            cmp8((u8)A, 0x02);
            alt = !fc;
        }
        lda8(rd8(ABSY(alt ? 0xB86D : 0xB864)));
        fc = 0;
        adc8(rd8(ABSX(0x01BE)));
        wr8(ABSX(0x01BE), (u8)A);
        if (fc) wr8(ABSX(0x01C6), inc8(rd8(ABSX(0x01C6))));
        lda8((u8)X);
        if (fz) {
            lda8(rd8(ABSX(0x01C6)));
            cmp8((u8)A, rd8(ABSY(0xB886)));
            int cap = 0;
            if (fz) {
                lda8(rd8(ABSX(0x01BE)));
                cmp8((u8)A, rd8(ABSY(0xB883)));
                cap = fc;
            } else {
                cap = fc;
            }
            if (cap) {
                lda8(rd8(ABSY(0xB883))); wr8(ABSX(0x01BE), (u8)A);
                lda8(rd8(ABSY(0xB886))); wr8(ABSX(0x01C6), (u8)A);
            }
        }
    }
    lda8(rd8(ABSX(0x019E)));
    if (fn) {
        lda8(rd8(ABS(0x01EE)));
        lda8(fz ? 0x05 : 0x04);
        wr8(ABSX(0x01C6), (u8)A);
        lda8(0x80); wr8(ABSX(0x01BE), 0x80);
    }
    ret();
}

/* $80:BD88 UpdateRiderDistances (RTS): in a race ($7D, $92 set, $84
 * clear) the word distances $085E + 2X for riders 7..0: on-screen riders
 * ($01DE) from their position $0196/$019E + scroll $9C - $0100; others
 * add their speed $0D01 to the fraction $015E and the carry. */
void UpdateRiderDistances_m1x1(void)
{
    lda8(rd8(DP(0x7D)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x92)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    rep(0x20);
    ldy8(0x0E);
    ldx8(0x07);
    do {
        lda16(rd16(ABSX(0x01DE)));
        and16(0x00FF);
        if (!fz) {
            lda16(rd16(ABSX(0x0196))); wr16(DP(0x00), A);
            lda16(rd16(ABSX(0x019E))); wr16(DP(0x01), A);
            lda16(rd16(DP(0x00)));
            fc = 0; adc16(rd16(DP(0x9C)));
            fc = 1; sbc16(0x0100);
            wr16(ABSY(0x085E), A);
        } else {
            lda16(rd16(ABSY(0x015E)));
            fc = 0; adc16(rd16(ABSY(0x0D01)));
            push16(A);
            and16(0x00FF);
            wr16(ABSY(0x015E), A);
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

void QueueSoundB_m0x1(void);
void Hundreds_m1x1(void);

/* $81:9592 DrawDustPair (RTS, object Y): scroll the pair left with the
 * player's speed ($0106 and $012E -= $0186); every 4 frames step the
 * tile $011E down, ending at $AF ($0116 = 0). While racing write both
 * sprites (OAM buffer $1610 + 2Y and $17E0 + 2Y, attribute $2D). */
void DrawDustPair_m1x1(void)
{
    X = Y; nz8((u8)X);
    lda8(rd8(ABSX(0x0106)));
    fc = 1; sbc8(rd8(ABS(0x0186)));
    wr8(ABSX(0x0106), (u8)A);
    lda8(rd8(ABSX(0x012E)));
    fc = 1; sbc8(rd8(ABS(0x0186)));
    wr8(ABSX(0x012E), (u8)A);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (fz) {
        wr8(ABSX(0x011E), dec8(rd8(ABSX(0x011E))));
        lda8(rd8(ABSX(0x011E)));
        cmp8((u8)A, 0xAF);
        if (fz) { wr8(ABSX(0x0116), 0); ret(); return; }
    }
    lda8(rd8(ABS(0x01CE)));
    ora8(rd8(DP(0x84)));
    if (fz) {
        lda8((u8)X);
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0x0106))); wr8(ABSY(0x1610), (u8)A);
        lda8(rd8(ABSX(0x010E))); wr8(ABSY(0x1611), (u8)A);
        lda8(rd8(ABSX(0x011E))); wr8(ABSY(0x1612), (u8)A);
        lda8(0x2D); wr8(ABSY(0x1613), 0x2D);
        lda8(rd8(ABSX(0x012E))); wr8(ABSY(0x17E0), (u8)A);
        lda8(rd8(ABSX(0x0136))); wr8(ABSY(0x17E1), (u8)A);
        lda8(rd8(ABSX(0x011E))); wr8(ABSY(0x17E2), (u8)A);
        lda8(0x2D); wr8(ABSY(0x17E3), 0x2D);
    }
    ret();
}

/* $80:ACD6 RiderOnRamp (RTS): from the track piece $0216,X (table
 * $80:AD33) decide whether rider X is on a ramp ($08BC = 2) or not
 * ($08BC = 0, only from 1). Landing the player on a big ramp in the bonus
 * mode at height $B0 plays sound $46. For the player Z = not moving. */
void RiderOnRamp_m1x1(void)
{
    Y = rd8(ABSX(0x0216)); nz8((u8)Y);
    lda8(rd8(ABSY(0xAD33)));
    if (fz) {
        lda8(0x03);
        cmp8((u8)A, rd8(ABSX(0x020E)));
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
            lda8(rd8(ABSY(0xAD33)));
            if (fn) {
                lda8(rd8(ABS(0x08BC)));
                if (fz) {
                    lda8(rd8(ABS(0x01B6)));
                    cmp8((u8)A, 0xB0);
                    if (fz) {
                        lda8(0x46);
                        push8(0x80); push16(0xAD15); QueueSoundB_m1x1();
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
        lda8(rd8(ABS(0x01BE)));
        ora8(rd8(ABS(0x01C6)));
    }
    ret();
}

/* $81:C9A7 BuildJumpScore (RTL): shift the score digits $03CF-$03DB right
 * by three, then put the jump count (Hundreds of $1569) with its label
 * tiles $4B/$4C and $5B/$5C in front; size $29. Zero distance blanks it. */
void BuildJumpScore_m1x1(void)
{
    ldy8(0x05);
    ldx8(0x0C);
    do {
        lda8(rd8(ABSX(0x03CF)));
        wr8(ABSX(0x03D2), (u8)A);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
    } while (!fn);
    push16(0xC9B7); Hundreds_m1x1();
    ldx8(0x07);
    lda8(0x4B); wr8(ABSX(0x03CF), 0x4B);
    lda8(inc8((u8)A)); wr8(ABSX(0x03D0), (u8)A);
    lda8((u8)Y); wr8(ABSX(0x03D1), (u8)A);
    ldx8(0x10);
    lda8(0x5B); wr8(ABSX(0x03CF), 0x5B);
    lda8(inc8((u8)A)); wr8(ABSX(0x03D0), (u8)A);
    lda8((u8)Y);
    fc = 0; adc8(0x10);
    wr8(ABSX(0x03D1), (u8)A);
    lda8(0x29); wr8(ABS(0x03CF), 0x29);
    rep(0x20);
    lda16(rd16(ABS(0x1569)));
    if (fz) {
        ldx8(0xFF);
        wr8(ABS(0x03D5), 0xFF); wr8(ABS(0x03D6), 0xFF); wr8(ABS(0x03D7), 0xFF);
        wr8(ABS(0x03DE), 0xFF); wr8(ABS(0x03DF), 0xFF); wr8(ABS(0x03E0), 0xFF);
        X = inc8((u8)X);
        wr8(ABS(0x03D8), (u8)X);
        ldx8(0x10);
        wr8(ABS(0x03E1), 0x10);
    }
    sep(0x20);
    retl();
}

/* $80:B5F8 Mode09_BonusIntro (RTS): a pre-delay $1991 first; then, once
 * the broadcast minute reaches the table $80:B5F6 (by course $7C / 2),
 * go to mode 9 ($1923 - 1). Otherwise count $88 down (sound $34 at $06CC,
 * nothing above it) and zoom $A6 to $0100, then set colour math ($190D
 * $42, COLDATA $E0). */
void Mode09_BonusIntro_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(0x1991)));
    if (!fz) {
        wr16(ABS(0x1991), dec16(rd16(ABS(0x1991))));
    } else {
        sep(0x20);
        lda8(rd8(DP(0x7C)));
        lda8(lsr8((u8)A));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(LONG(0x7FFFFF)));
        cmp8((u8)A, rd8(ABSX(0xB5F6)));
        if (fc) {
            lda8(0x09); wr8(DP(0x76), 0x09);
            wr8(DP(0x75), 0);
            wr8(DP(0x77), 0);
            wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
            sep(0x20);
            ret();
            return;
        }
    }
    rep(0x20);
    lda16(rd16(ABS(0x0088)));
    lda16(dec16(A));
    wr16(ABS(0x0088), A);
    if (fz) goto done;
    cmp16(A, 0x06CC);
    if (fz) {
        lda16(0x0034);
        push8(0x80); push16(0xB62D); QueueSoundB_m0x1();
    } else if (fc) {
        goto done;
    }
    lda16(rd16(DP(0xA6)));
    cmp16(A, 0x0100);
    if (fz) {
        sep(0x20);
        lda8(0x42); wr8(ABS(0x190D), 0x42);
        lda8(0xE0); wr8(ABS(0x2132), 0xE0);
    } else {
        fc = 0;
        adc16(0x0004);
        wr16(DP(0xA6), A);
    }
done:
    sep(0x20);
    ret();
}

/* $80:FE93 TrackRiderPace (RTS): while racing, count frames $0995 and the
 * distance $0996 (+ speed); every $3C units of distance pick the pace band
 * from $80:F715 (frames, for X = 7..1) and in a race set the AI targets
 * $09B0/$09B1 from $80:F71D/$80:F725. Restart both after $46 frames. */
void TrackRiderPace_m1x1(void)
{
    lda8(rd8(ABS(0x0CD7)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x92)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    wr8(ABS(0x0995), inc8(rd8(ABS(0x0995))));
    lda8(rd8(ABS(0x0995)));
    cmp8((u8)A, 0x46);
    if (fc) {
        lda8(0x00);
        wr8(ABS(0x0995), 0);
        wr8(ABS(0x0996), 0);
        ret();
        return;
    }
    lda8(rd8(ABS(0x0186)));
    fc = 0;
    adc8(rd8(ABS(0x0996)));
    wr8(ABS(0x0996), (u8)A);
    cmp8((u8)A, 0x3C);
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(0x0996)));
    fc = 1;
    sbc8(0x3C);
    wr8(ABS(0x0996), (u8)A);
    ldx8(0x07);
    do {
        lda8(rd8(ABS(0x0995)));
        cmp8((u8)A, rd8(ABSX(0xF715)));
        if (!fc) wr8(DP(0x00), (u8)X);
        else wr8(DP(0x01), (u8)X);
        X = dec8((u8)X);
    } while (!fz);
    lda8(rd8(DP(0x7D)));
    if (!fz) {
        X = rd8(DP(0x00)); nz8((u8)X);
        lda8(rd8(ABSX(0xF71D))); wr8(ABS(0x09B0), (u8)A);
        lda8(rd8(ABSX(0xF725))); wr8(ABS(0x09B1), (u8)A);
    }
    lda8(0x00);
    wr8(ABS(0x0995), 0);
    wr8(ABS(0x0996), 0);
    ret();
}

/* $82:8782 HideAllSprites (RTL): unless $1958 is set, move all 128
 * sprites of the OAM buffer off screen (Y $F0) and clear the size table
 * $1821-$189F. */
void HideAllSprites_m1x1(void)
{
    static const u16 rows[] = { 0x1601, 0x1681, 0x1701, 0x1781 };
    lda8(rd8(ABS(0x1958)));
    if (!fz) { retl(); return; }
    ldx8(0x00);
    do {
        lda8(0xF0);
        for (int r = 0; r < 4; r++)
            for (int k = 0; k < 8; k++)
                wr8(ABSX(rows[r] + 4 * k), 0xF0);
        lda8((u8)X);
        fc = 0;
        adc8(0x20);
        X = (u8)A; nz8((u8)X);
    } while (!fn);
    lda8(0x00);
    ldy8(0x7F);
    do {
        wr8(ABSY(0x1820), 0);
        Y = dec8((u8)Y);
    } while (!fz);
    retl();
}

void SetJumpRiderSprites_m1x1(void);

/* $81:C75E CheckLaneTile (RTL): for rider X without a lane flag ($01AE),
 * by its lane mode $08A4 (0, 2, 3, 5) look for the marker tiles at
 * $0887 + 4X or under it ($0216,X): $65/$6A or $67/$68 give $01AE = $FF,
 * $35/$6A or $37/$68 give 1. */
void CheckLaneTile_m1x1(void)
{
    lda8(rd8(ABSX(0x01AE)));
    if (!fz) { retl(); return; }
    lda8((u8)X);
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    u8 mode = rd8(ABSX(0x08A4));
    lda8(mode);
    u8 tile, under, value;
    if (fz) { cmp8(mode, 0x02); tile = 0x65; under = 0x6A; value = 0xFF; }
    else {
        cmp8(mode, 0x02);
        if (fz) { tile = 0x67; under = 0x68; value = 0xFF; }
        else {
            cmp8(mode, 0x03);
            if (fz) { tile = 0x35; under = 0x6A; value = 0x01; }
            else {
                cmp8(mode, 0x05);
                if (!fz) { retl(); return; }
                tile = 0x37; under = 0x68; value = 0x01;
            }
        }
    }
    lda8(rd8(ABSY(0x0887)));
    cmp8((u8)A, tile);
    if (!fz) {
        lda8(rd8(ABSX(0x0216)));
        cmp8((u8)A, under);
        if (!fz) { retl(); return; }
    }
    lda8(value);
    wr8(ABSX(0x01AE), value);
    retl();
}

/* $80:E7C1 DrawJumpRiderSprites (RTS): for a moving rider X on a ramp
 * ($08BC) in the air state 1 past column $10: its two sprites
 * (SetJumpRiderSprites), blinking every 4 frames, palette from the track
 * piece (or $0A off the track, $0E above height $58), attribute | $61,
 * and the lower one hidden while the speed high byte is zero. */
void DrawJumpRiderSprites_m1x1(void)
{
    lda8(rd8(ABSX(0x08BC)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x01C6)));
    ora8(rd8(ABSX(0x01BE)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x01FE)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x019E)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x0196)));
    cmp8((u8)A, 0x10);
    if (!fc) { ret(); return; }
    push16(0xE7E3); SetJumpRiderSprites_m1x1();
    lda8(rd8(DP(0x63)));
    and8(0x04);
    if (!fz) {
        lda8(0xF0); wr8(ABSY(0x16C2), 0xF0);
        lda8(0xE0); wr8(ABSY(0x16C6), 0xE0);
    }
    lda8(rd8(ABSX(0x0958)));
    if (fz) {
        lda8(0x0A); wr8(DP(0x02), 0x0A);
    } else {
        push8((u8)Y);
        Y = rd8(ABSX(0x0216)); nz8((u8)Y);
        lda8(rd8(ABSY(0xAD33)));
        Y = pull8(); nz8((u8)Y);
        wr8(DP(0x02), (u8)A);
        cmp8((u8)A, 0x00);
        if (fz) {
            lda8(0xF0);
            wr8(ABSY(0x16C1), 0xF0);
            wr8(ABSY(0x16C5), 0xF0);
        }
    }
    lda8(rd8(ABSX(0x01B6)));
    cmp8((u8)A, 0x58);
    if (!fc) lda8(0x0E);
    else lda8(rd8(DP(0x02)));
    ora8(0x61);
    wr8(ABSY(0x16C3), (u8)A);
    wr8(ABSY(0x16C7), (u8)A);
    lda8(rd8(ABSX(0x01C6)));
    if (fz) { lda8(0xF8); wr8(ABSY(0x16C5), 0xF8); }
    ret();
}

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

/* $81:9A27 FadeSkyColours (RTL): every 8 frames move the four colours at
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

/* $80:B22B DrawBigSprite (RTS): a 2x2 block of 16x16 sprites at OAM
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
        lda8(rd8(ABSX(0xB296)));
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

/* $80:D7AB QueueBonusCount (RTS): "x NN" style counter at $03CF (size
 * $27, label tiles $2D $22 $26 $0E $46 and their lower halves), the
 * number $B4 as two digits, queued at VRAM $184D with attribute $20. */
void QueueBonusCount_m1x1(void)
{
    static const u8 label[5] = { 0x2D, 0x22, 0x26, 0x0E, 0x46 };
    rep(0x20);
    lda16(0x184D); wr16(DP(0x00), 0x184D);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x27); wr8(ABS(0x03CF), 0x27);
    for (int i = 0; i < 5; i++) {
        lda8(label[i]); wr8(ABS(0x03D0 + i), label[i]);
        fc = 0; adc8(0x10);
        wr8(ABS(0x03D7 + i), (u8)A);
    }
    ldy8(0x00);
    lda8(rd8(DP(0xB4)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(0x03D6), (u8)A);
    ora8(0x10); wr8(ABS(0x03DD), (u8)A);
    lda8((u8)Y);
    wr8(ABS(0x03D5), (u8)A);
    ora8(0x10); wr8(ABS(0x03DC), (u8)A);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xD819); VramQueueTiles_m1x1();
    ret();
}

/* $81:94B6 SpawnExhaust (RTL): while the turbo is on ($0CE0) and the
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
    lda8(rd8(ABSY(0x9518)));
    if (fz) goto out;
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (!fz) goto out;
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x0116)));
        if (fz) {
            lda8(rd8(DP(0x2C))); and8(0x07); fc = 0; adc8(0x38);
            wr8(ABSX(0x0106), (u8)A);
            lda8(rd8(DP(0x2D))); and8(0x0F); fc = 0; adc8(rd8(ABS(0x01B6))); sbc8(0x02);
            wr8(ABSX(0x010E), (u8)A);
            lda8(rd8(DP(0x2E))); and8(0x07); fc = 0; adc8(0x3C);
            wr8(ABSX(0x012E), (u8)A);
            lda8(rd8(DP(0x2F))); and8(0x0F); fc = 0; adc8(rd8(ABS(0x01B6))); sbc8(0x02);
            wr8(ABSX(0x0136), (u8)A);
            lda8(0xB2); wr8(ABSX(0x011E), 0xB2);
            lda8(0x02); wr8(ABSX(0x0116), 0x02);
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

/* $80:BBCF RiderAtFinish (RTS): rider X crossing the finish ($08C4):
 * slow the run-out ($090C - 1 every 16 frames, every frame for the player
 * on a new turn press) and every 8 frames step the wave pose $091C
 * (0, 1, 3, 4; sound $23 at 3 and when it wraps). Once past $0196 + 8 the
 * rider stops: $56 = 3, $01D6 + 1, $099E = 0; rider 7 also clears $092C
 * and $08C7. */
void RiderAtFinish_m1x1(void)
{
    lda8(rd8(ABSX(0x08C4)));
    if (fz) goto stop;
    lda8(rd8(DPX(0x45)));
    if (!fz) { ret(); return; }
    lda8((u8)X);
    if (fz) {
        Y = rd8(DP(0x79)); nz8((u8)Y);
        lda8(rd8(ABS(0x017E)));
        and8(0xC0);
        if (!fz) {
            wr8(DP(0x79), (u8)A);
            lda8((u8)Y);
            if (fz) {
                wr8(ABSX(0x090C), dec8(rd8(ABSX(0x090C))));
                goto pose;
            }
        }
        wr8(DP(0x79), (u8)A);
    }
    lda8(rd8(DP(0x63)));
    and8(0x0F);
    if (fz) wr8(ABSX(0x090C), dec8(rd8(ABSX(0x090C))));
pose:
    and8(0x07);
    if (fz) {
        lda8(rd8(ABSX(0x091C)));
        lda8(inc8((u8)A));
        cmp8((u8)A, 0x02);
        if (fz) lda8(inc8((u8)A));
        cmp8((u8)A, 0x05);
        if (fz) {
            lda8(0x23);
            push8(0x80); push16(0xBC19); QueueSoundB_m1x1();
            lda8(0x00);
            wr8(ABSX(0x091C), 0x00);
        } else {
            cmp8((u8)A, 0x03);
            if (fz) {
                wr8(ABSX(0x091C), (u8)A);
                lda8(0x23);
                push8(0x80); push16(0xBC11); QueueSoundB_m1x1();
            } else {
                wr8(ABSX(0x091C), (u8)A);
            }
        }
    }
    lda8(rd8(ABSX(0x090C)));
    fc = 1;
    sbc8(0x08);
    cmp8((u8)A, rd8(ABSX(0x0196)));
    if (fc) { ret(); return; }
stop:
    lda8(0x03); wr8(DPX(0x56), 0x03);
    wr8(ABSX(0x01D6), inc8(rd8(ABSX(0x01D6))));
    lda8(0x00); wr8(ABSX(0x099E), 0x00);
    cmp8((u8)X, 0x07);
    if (fz) {
        wr8(ABS(0x092C), 0);
        wr8(ABS(0x08C7), 0);
    }
    ret();
}

/* $80:F15F BuildScoreDigits (M=0, RTS): the word A as five decimal digits
 * at $03D0-$03D4 (leading zeros blank $FF), the lower row (+$10) at
 * $03D6-$03DA, $03D5 = 0, $03DB = $10, size $26 at $03CF. */
void BuildScoreDigits_m0x1(void)
{
    static const u16 pow10[3] = { 10000, 1000, 100 };
    for (int i = 0; i < 3; i++) {
        ldy8(0x00);
        for (;;) {
            cmp16(A, pow10[i]);
            if (!fc) break;
            sbc16(pow10[i]);
            Y = inc8((u8)Y);
            WAIT_TICK();
        }
        wr8(ABS(0x03D0 + i), (u8)Y);
    }
    ldy8(0x00);
    sep(0x20);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(0x03D3), (u8)Y);
    wr8(ABS(0x03D4), (u8)A);
    wr8(ABS(0x03D5), 0);
    lda8(0x10); wr8(ABS(0x03DB), 0x10);
    ldy8(0x00);
    wr8(DP(0x00), 0);
    do {
        lda8(rd8(ABSY(0x03D0)));
        if (fz && rd8(DP(0x00)) == 0) {
            lda8(rd8(DP(0x00)));
            lda8(0xFF);
            wr8(ABSY(0x03D0), 0xFF);
            wr8(ABSY(0x03D6), 0xFF);
        } else {
            lda8(rd8(ABSY(0x03D0)));
            ora8(0x10);
            wr8(ABSY(0x03D6), (u8)A);
            wr8(DP(0x00), inc8(rd8(DP(0x00))));
        }
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x05);
    } while (!fc);
    lda8(0x26); wr8(ABS(0x03CF), 0x26);
    ret();
}

/* Step one colour field of the word $1C00,X towards the target word at
 * `targets`,X (mask `m`, unit `u`). */
static void fade_to(u16 m, u16 targets, u16 u)
{
    lda16(rd16(ABSX(0x1C00)));
    and16(m);
    cmp16(A, rd16(ABSX(targets)));
    if (fz) return;
    int down = fc;
    lda16(rd16(ABSX(0x1C00)));
    if (down) { fc = 1; sbc16(u); }
    else { fc = 0; adc16(u); }
    wr16(ABSX(0x1C00), A);
}

/* $81:9A9B FadePalette (RTL): every 8 frames move the colours $1C00 +
 * $B9 down to 0 (skipping each palette's colour 0) one step towards the
 * targets at $81:9B15 / $81:9B2D / $81:9B45 (blue, green, red), and flag
 * the palette upload ($1915). */
void FadePalette_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x07);
    if (fz) {
        X = rd8(DP(0xB9)); nz8((u8)X);
        rep(0x20);
        do {
            lda16((u16)X);
            and16(0x0007);
            if (!fz) {
                fade_to(0x7C00, 0x9B15, 0x0400);
                fade_to(0x03E0, 0x9B2D, 0x0020);
                fade_to(0x001F, 0x9B45, 0x0001);
            }
            X = dec8((u8)X);
            X = dec8((u8)X);
        } while (!fn);
        sep(0x20);
    }
    wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
    retl();
}

/* $80:D013 MirrorJumpRider (RTS): copy the player's four sprites (OAM
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

/* $82:8701 PlaceMinuteMarker (RTL): in a race (mode 3, $0CD7 clear) when
 * the broadcast minute changes ($BF) to one of the eight in $82:8773,
 * find the next free stretch of the track buffer $7F:4000 after the
 * player ($027B + 3; a column $80 not preceded by tiles $17/$19/$1B/$03/
 * $04, wrapping at 9) and drop the item from $82:877B by lane mode $08A4
 * there; clear $B8. */
void PlaceMinuteMarker_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82);
    DB = pull8(); nz8(DB);
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, rd8(ABS(0x00BF)));
    if (fz) goto out;
    lda8(rd8(ABS(0x0CD7)));
    if (!fz) goto out;
    lda8(rd8(DP(0x76)));
    cmp8((u8)A, 0x03);
    if (!fz) goto out;
    ldx8(0x07);
    lda8(rd8(LONG(0x7FFFFF)));
    for (;;) {
        cmp8((u8)A, rd8(ABSX(0x8773)));
        if (fz) break;
        X = dec8((u8)X);
        if (fn) { DB = pull8(); nz8(DB); retl(); return; }
    }
    rep(0x10);
    X = rd16(ABS(0x027B)); nz16(X);
    X = inc16(X); X = inc16(X); X = inc16(X);
    for (;;) {
        lda8(rd8(LONGX(0x7F4000)));
        cmp8((u8)A, 0x09);
        if (fz) { X = 0x0000; nz16(X); }
        X = inc16(X);
        cmp8((u8)A, 0x80);
        WAIT_TICK();
        if (!fz) continue;
        lda8(rd8(LONGX(0x7F3FFE)));
        cmp8((u8)A, 0x17); if (fz) continue;
        cmp8((u8)A, 0x19); if (fz) continue;
        cmp8((u8)A, 0x1B); if (fz) continue;
        cmp8((u8)A, 0x03); if (fz) continue;
        cmp8((u8)A, 0x04); if (fz) continue;
        break;
    }
    rep(0x20);
    lda16(rd16(ABS(0x08A4)));
    and16(0x00FF);
    Y = A; nz16(Y);
    sep(0x20);
    lda8(rd8(ABSY(0x877B)));
    wr8(LONGX(0x7F3FFF), (u8)A);
    wr8(ABS(0x00B8), 0);
out:
    sep(0x10);
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:CB78 RiderPose (RTS): returns A = the sprite pose for rider X
 * ($0E = extra flags): a waving pose | $10 at the finish or start
 * ($01D6), the crash and fall poses $13-$15, or its lean $01E6. */
void RiderPose_m1x1(void)
{
    wr8(DP(0x0E), 0);
    lda8(rd8(ABSX(0x01D6)));
    int special = fz;
    if (!fz) { cmp8((u8)A, 0x05); special = fz; }
    if (!special) {
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
    Y = rd8(DP(0x84)); nz8((u8)Y);
    Y = dec8((u8)Y);
    if (fz) {
        lda8(rd8(ABSX(0x01E6)));
        cmp8((u8)A, 0x0A);
        if (fz) { ldy8(0x02); wr8(DP(0x0E), 0x02); ret(); return; }
    }
    lda8(rd8(ABSX(0x01CE)));
    ora8(rd8(ABSX(0x01EE)));
    if (!fz) goto lean;
    lda8(rd8(ABSX(0x0176)));
    if (!fz) {
        cmp8((u8)A, 0x0D);
        if (!fz) goto lean;
        lda8(rd8(ABSX(0x01F6)));
        cmp8((u8)A, 0x20);
    }
    lda8(rd8(ABSX(0x01BE)));
    ora8(rd8(ABSX(0x01C6)));
    if (fz) {
        wr8(DP(0x0E), inc8(rd8(DP(0x0E))));
        goto lean;
    }
    lda8(rd8(ABSX(0x01E6)));
    cmp8((u8)A, 0x06);
    if (!fz) goto lean;
    ldy8(0x14);
    lda8(rd8(ABSX(0x01AE)));
    if (!fz) {
        if (fn) Y = inc8((u8)Y);
        lda8((u8)Y);
        ret();
        return;
    }
    lda8(rd8(DP(0x63)));
    and8(0x02);
    if (!fz) { lda8(0x13); ret(); return; }
lean:
    lda8(rd8(ABSX(0x01E6)));
    ret();
}

/* $80:D97C ReadJumpTrack (RTS): for the player's wheel column $0266:
 * count landing points ($0A93 at each new column while airborne or
 * crashed) into $09C5, take the height $0A13 while on the ground, and the
 * tile $09D3 into $0216. Tiles below $28 end the course once ($099C, $84,
 * $1990 = 1, sound $2B); tile flags $0A53 turn the column into $3B tiles
 * (or $45 while airborne); tile $84 becomes $FC on the ground. */
void ReadJumpTrack_m1x1(void)
{
    X = rd8(ABS(0x0266)); nz8((u8)X);
    lda8(rd8(ABS(0x09C7)));
    ora8(rd8(ABS(0x01EE)));
    if (!fz) {
        lda8(rd8(ABS(0x0266)));
        cmp8((u8)A, rd8(ABS(0x09C8)));
        if (!fz) {
            wr8(ABS(0x09C8), (u8)A);
            lda8(rd8(ABSX(0x0A93)));
            fc = 0; adc8(rd8(ABS(0x09C5)));
            wr8(ABS(0x09C5), (u8)A);
        }
    }
    lda8(rd8(ABS(0x01CE)));
    if (fz) {
        lda8(rd8(ABSX(0x0A13)));
        wr8(ABS(0x09C4), (u8)A);
    }
    lda8(rd8(ABSX(0x09D3)));
    wr8(ABS(0x0216), (u8)A);
    cmp8((u8)A, 0x28);
    if (!fc) {
        lda8(rd8(ABS(0x099C)));
        if (fz) {
            lda8(0x01);
            wr8(ABS(0x099C), 0x01);
            wr8(DP(0x84), 0x01);
            wr8(ABS(0x1990), 0x01);
            lda8(0x2B);
            push8(0x80); push16(0xD9C5); QueueSoundB_m1x1();
        }
        ret();
        return;
    }
    lda8(rd8(ABSX(0x0A53)));
    if (!fz) {
        lda8(rd8(ABS(0x09C7)));
        ora8(rd8(ABS(0x01EE)));
        if (!fz) {
            lda8(0x45); wr8(ABS(0x0216), 0x45);
            ret();
            return;
        }
        lda8(rd8(ABSX(0x09D3)));
        and8(0x40);
        if (!fz) { ret(); return; }
        wr8(ABSX(0x0A53), 0);
        lda8(0x3B);
        wr8(ABSX(0x0B53), 0x3B); wr8(ABSX(0x0B93), 0x3B); wr8(ABSX(0x0BD3), 0x3B);
        wr8(ABSX(0x0C13), 0x3B); wr8(ABSX(0x0C53), 0x3B); wr8(ABSX(0x0C93), 0x3B);
        wr8(ABS(0x021E), 0x3B);
    }
    lda8(rd8(ABSX(0x09D3)));
    cmp8((u8)A, 0x84);
    if (fz) {
        lda8(rd8(ABS(0x09C7)));
        if (fz) { lda8(0xFC); wr8(ABS(0x0216), 0xFC); }
    }
    ret();
}

void QueueRaceTime_m1x1(void);
void QueueBestTime_m1x1(void);

/* $81:9D71 VelocityDirection (RTL): the 8-way direction ($02DF,X, 0-7)
 * of the velocity $029F/$02AF,X from its signs (table $81:9DE0) and which
 * component dominates. */
void VelocityDirection_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x04), 0);
    wr16(DP(0x06), 0);
    lda16(rd16(ABSX(0x029F)));
    if (fn) {
        eor16(0xFFFF);
        lda16(inc16(A));
        wr16(DP(0x04), inc16(rd16(DP(0x04))));
    }
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(0x02AF)));
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
    lda16(rd16(ABSY(0x9DE0)));
    fc = 0;
    adc16(rd16(DP(0x08)));
    and16(0x0007);
    wr16(ABSX(0x02DF), A);
    sep(0x20);
    retl();
}

/* $82:8525 PackSpriteSizes (RTL): pack the size bytes $1820-$189F (one
 * per sprite, 2 bits used) four to a byte into the OAM high table
 * $1800-$181F (D = $1800 while it works). */
void PackSpriteSizes_m1x1(void)
{
    push16(D);
    lda8(0x18);
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    lda8(0x00);
    D = A; nz16(D);
    ldy8(0x1C);
    do {
        for (int k = 0; k < 4; k++) {
            lda8((u8)Y);
            lda8(asl8((u8)A));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            u8 o = (u8)(0x20 + 4 * k);
            lda8(rd8(DPX(o + 3)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); ora8(rd8(DPX(o + 2)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); ora8(rd8(DPX(o + 1)));
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); ora8(rd8(DPX(o)));
            wr8(ABSY(0x1800 + k), (u8)A);
        }
        Y = dec8((u8)Y); Y = dec8((u8)Y); Y = dec8((u8)Y); Y = dec8((u8)Y);
    } while (!fn);
    D = pull16(); nz16(D);
    retl();
}

/* $80:C692 RecordCourseTime (RTL): store the race time $03AE-$03B0 for
 * Stadium course $0928 at $0819 + 3(course - 2); a faster time than the
 * best $083A-$083C becomes the record ($084C = course - 1) and is shown
 * as the race time, else the best is shown (VRAM $18D7, attribute $24).
 * Outside the jump ($84 clear) the race time is cleared. */
void RecordCourseTime_m1x1(void)
{
    push8((u8)X);
    push8((u8)Y);
    lda8(0xFF); wr8(DP(0xCB), 0xFF);
    wr8(DP(0xCC), 0);
    lda8(rd8(ABS(0x0928)));
    lda8(dec8((u8)A));
    lda8(dec8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(asl8((u8)A));
    fc = 0;
    adc8(rd8(DP(0x00)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(0x03AE))); wr8(ABSX(0x0819), (u8)A);
    lda8(rd8(ABS(0x03AF))); wr8(ABSX(0x081A), (u8)A);
    lda8(rd8(ABS(0x03B0))); wr8(ABSX(0x081B), (u8)A);
    int record = 0;
    for (int i = 0; i < 3; i++) {
        lda8(rd8(ABS(0x083A + i)));
        cmp8((u8)A, rd8(ABS(0x03AE + i)));
        if (!fc) break;
        if (!fz) { record = 1; break; }
    }
    if (record) {
        lda8(rd8(ABS(0x03AE))); wr8(ABS(0x083A), (u8)A);
        lda8(rd8(ABS(0x03AF))); wr8(ABS(0x083B), (u8)A);
        lda8(rd8(ABS(0x03B0))); wr8(ABS(0x083C), (u8)A);
        lda8(rd8(ABS(0x0928)));
        lda8(dec8((u8)A));
        wr8(ABS(0x084C), (u8)A);
    }
    lda8(0xD7); wr8(DP(0x00), 0xD7);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(0x24); wr8(DP(0x0F), 0x24);
    if (record) { push16(0xC710); QueueRaceTime_m1x1(); }
    else        { push16(0xC6FF); QueueBestTime_m1x1(); }
    lda8(rd8(DP(0x84)));
    if (fz) {
        wr8(ABS(0x03AE), 0);
        wr8(ABS(0x03AF), 0);
        wr8(ABS(0x03B0), 0);
    }
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}

/* $81:A1C8 RivalEngineSound (RTL): unless fading, find the rival nearest
 * to the player by distance ($085E words), send its signed distance / 2
 * (APUIO1) and, when it is within $80 of the scroll $9C, its speed / 8
 * (APUIO3, else 0) for the second engine voice. */
void RivalEngineSound_m1x1(void)
{
    lda8(rd8(ABS(0x1923)));
    if (!fz) { retl(); return; }
    ldx8(0x07);
    ldy8(0x0E);
    do {
        rep(0x20);
        lda16(rd16(ABS(0x085E)));
        wr16(DP(0x00), A);
        lda16(rd16(ABSY(0x085E)));
        fc = 1;
        sbc16(rd16(DP(0x00)));
        wr16(ABSY(0x0010), A);
        if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
        wr16(ABSY(0x0000), A);
        sep(0x20);
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
        X = dec8((u8)X);
    } while (!fz);
    rep(0x20);
    lda16(0x000E);
    wr16(DP(0x10), 0x000E);
    ldx8(0x0C);
    lda16(rd16(DP(0x0E)));
    do {
        cmp16(A, rd16(DPX(0x00)));
        if (fc) {
            wr8(DP(0x10), (u8)X);
            lda16(rd16(DPX(0x00)));
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
    sep(0x20);
    X = rd8(DP(0x10)); nz8((u8)X);
    lda8(rd8(DPX(0x00)));
    lda8(lsr8((u8)A));
    Y = rd8(ABSX(0x0011)); nz8((u8)Y);
    if (fn) { eor8(0xFF); lda8(inc8((u8)A)); }
    wr8(ABS(0x2141), (u8)A);
    rep(0x20);
    lda16(rd16(ABSX(0x085E)));
    fc = 1;
    sbc16(rd16(ABS(0x009C)));
    if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
    cmp16(A, 0x0080);
    if (fc) {
        lda16(0x0000);
    } else {
        sep(0x20);
        lda8((u8)X);
        lda8(lsr8((u8)A));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x01C6)));
        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
        lda8(rd8(ABSX(0x01BE)));
        rep(0x20);
        lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    }
    sep(0x20);
    wr8(ABS(0x2143), (u8)A);
    retl();
}

/* $80:B435 QueueTileRows (RTS): queue $11 rows of $10 tile words from the
 * long pointer $02 (consecutive) for VRAM $00, $00 + $20, ...: each a DMA
 * record at the queue end $7F:0000 (mode 1 to $2118, source in bank $7F,
 * $10 * 2 bytes). */
void QueueTileRows_m1x1(void)
{
    push8((u8)X);
    push8((u8)Y);
    ldy8(0x00);
    lda8(rd8(DP(0x10)));
    lda8(asl8((u8)A));
    wr8(DP(0x08), (u8)A);
    wr8(DP(0x09), 0);
    for (;;) {
        rep(0x30);
        lda16(rd16(LONG(0x7F0000)));
        X = A; nz16(X);
        sep(0x20);
        lda8(0x80); wr8(LONGX(0x7F0002), 0x80);
        lda8(0x01); wr8(LONGX(0x7F0003), 0x01);
        lda8(0x18); wr8(LONGX(0x7F0004), 0x18);
        rep(0x20);
        lda16(X);
        fc = 0;
        adc16(0x000C);
        wr16(LONGX(0x7F0005), A);
        sep(0x20);
        lda8(0x7F); wr8(LONGX(0x7F0007), 0x7F);
        lda8(rd8(DP(0x01))); wr8(LONGX(0x7F0001), (u8)A);
        lda8(rd8(DP(0x00))); wr8(LONGX(0x7F0000), (u8)A);
        lda8(rd8(DP(0x10)));
        wr8(DP(0x06), (u8)A);
        lda8(asl8((u8)A));
        wr8(LONGX(0x7F0008), (u8)A);
        lda8(0x00); wr8(LONGX(0x7F0009), 0x00);
        do {
            u32 p = rd16(DP(0x02)) | ((u32)rd8(DP(0x04)) << 16);
            lda8(rd8((p + Y) & 0xFFFFFF));
            wr8(LONGX(0x7F000C), (u8)A);
            Y = inc16(Y);
            lda8(rd8((p + Y) & 0xFFFFFF));
            wr8(LONGX(0x7F000D), (u8)A);
            X = inc16(X);
            X = inc16(X);
            Y = inc16(Y);
            u8 n = dec8(rd8(DP(0x06)));
            wr8(DP(0x06), n);
        } while (!fz);
        rep(0x20);
        lda16(rd16(LONG(0x7F0000)));
        X = A; nz16(X);
        fc = 0; adc16(0x000C);
        fc = 0; adc16(rd16(DP(0x08)));
        wr16(LONG(0x7F0000), A);
        wr16(LONGX(0x7F000A), A);
        sep(0x20);
        fc = 0;
        lda8(0x20);
        adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        lda8(0x00);
        adc8(rd8(DP(0x01)));
        wr8(DP(0x01), (u8)A);
        u8 r = dec8(rd8(DP(0x11)));
        wr8(DP(0x11), r);
        if (fz) break;
    }
    sep(0x10);
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    ret();
}

/* $80:AE97 BrakeRider (RTS): rider X slows down by the step $80:B876,Y
 * (the caller's Y), unless it is already slower than $80:B881 (by crash
 * state), the player is on the jump with $85 set, or (below page 2) it is
 * a rival. */
void BrakeRider_m1x1(void)
{
    wr8(DP(0x00), (u8)Y);
    lda8(rd8(ABSX(0x01C6)));
    if (fz) {
        lda8(rd8(ABSX(0x01EE)));
        lda8(lsr8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0x01BE)));
        cmp8((u8)A, rd8(ABSY(0xB881)));
        if (!fc) { ret(); return; }
    }
    Y = rd8(DP(0x00)); nz8((u8)Y);
    lda8(rd8(ABS(0x0176)));
    cmp8((u8)A, 0x18);
    if (fz) {
        lda8(rd8(DP(0x85)));
        if (!fz) { ret(); return; }
    }
    lda8(rd8(DP(0x9D)));
    cmp8((u8)A, 0x02);
    if (!fc) {
        cmp8((u8)X, 0x00);
        if (!fz) { ret(); return; }
    }
    lda8(rd8(ABSX(0x01BE)));
    fc = 1;
    sbc8(rd8(ABSY(0xB876)));
    wr8(ABSX(0x01BE), (u8)A);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x01C6)));
    if (!fz) {
        wr8(ABSX(0x01C6), dec8(rd8(ABSX(0x01C6))));
        ret();
        return;
    }
    wr8(ABSX(0x01BE), (u8)A);
    ret();
}

/* $80:B38B QueueTileBlock (RTS): queue a tile block from the list at ($02)
 * (first byte: rows << 4 | words per row, then the tile words) for VRAM
 * $00, one DMA record per row at the queue end $7F:0000, rows $20 words
 * apart. */
void QueueTileBlock_m1x1(void)
{
    push8((u8)X);
    push8((u8)Y);
    ldy8(0x00);
    u32 base = ((u32)DB << 16) + rd16(DP(0x02));
    lda8(rd8((base + Y) & 0xFFFFFF));
    and8(0x0F);
    wr8(DP(0x05), (u8)A);
    lda8(asl8((u8)A));
    wr8(DP(0x08), (u8)A);
    wr8(DP(0x09), 0);
    lda8(rd8((base + Y) & 0xFFFFFF));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(DP(0x04), (u8)A);
    Y = inc8((u8)Y);
    for (;;) {
        rep(0x30);
        lda16(rd16(LONG(0x7F0000)));
        X = A; nz16(X);
        sep(0x20);
        lda8(0x80); wr8(LONGX(0x7F0002), 0x80);
        lda8(0x01); wr8(LONGX(0x7F0003), 0x01);
        lda8(0x18); wr8(LONGX(0x7F0004), 0x18);
        rep(0x20);
        lda16(X);
        fc = 0;
        adc16(0x000C);
        wr16(LONGX(0x7F0005), A);
        sep(0x20);
        lda8(0x7F); wr8(LONGX(0x7F0007), 0x7F);
        lda8(rd8(DP(0x01))); wr8(LONGX(0x7F0001), (u8)A);
        lda8(rd8(DP(0x00))); wr8(LONGX(0x7F0000), (u8)A);
        lda8(rd8(DP(0x05)));
        wr8(DP(0x06), (u8)A);
        lda8(asl8((u8)A));
        wr8(LONGX(0x7F0008), (u8)A);
        lda8(0x00); wr8(LONGX(0x7F0009), 0x00);
        do {
            u32 p = ((u32)DB << 16) + rd16(DP(0x02));
            lda8(rd8((p + Y) & 0xFFFFFF));
            wr8(LONGX(0x7F000C), (u8)A);
            Y = inc16(Y);
            lda8(rd8((p + Y) & 0xFFFFFF));
            wr8(LONGX(0x7F000D), (u8)A);
            X = inc16(X);
            X = inc16(X);
            Y = inc16(Y);
            u8 n = dec8(rd8(DP(0x06)));
            wr8(DP(0x06), n);
        } while (!fz);
        rep(0x20);
        lda16(rd16(LONG(0x7F0000)));
        X = A; nz16(X);
        fc = 0; adc16(0x000C);
        fc = 0; adc16(rd16(DP(0x08)));
        wr16(LONG(0x7F0000), A);
        wr16(LONGX(0x7F000A), A);
        sep(0x20);
        fc = 0;
        lda8(0x20);
        adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        lda8(0x00);
        adc8(rd8(DP(0x01)));
        wr8(DP(0x01), (u8)A);
        u8 r = dec8(rd8(DP(0x04)));
        wr8(DP(0x04), r);
        if (fz) break;
    }
    sep(0x10);
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    ret();
}

void port_ws_rider_sprites(void);   /* roster.h: the port's widescreen riders */

/* $80:CE96 PlaceRiderShadow (RTS): rider X's shadow sprites at OAM
 * buffer $1600 + $07. On the ground ($019E = 0) near the right edge
 * (column $E1+) they become small/large wrapped sprites; further left, at
 * columns $80+ (blinking by height band), a lane marker sprite is drawn at
 * $1700 + $07 from $80:CF2D. Off the ground they hide, except a rider in
 * the air state 1 keeps them until column $F0. */
void PlaceRiderShadow_m1x1(void)
{
    Y = rd8(DP(0x07)); nz8((u8)Y);
    lda8(rd8(ABSX(0x019E)));
    int hide_top = 1;
    if (fz) {
        lda8(rd8(ABSX(0x0196)));
        cmp8((u8)A, 0xE1);
        if (fc) {
            cmp8((u8)A, 0xF0);
            if (fc) {
                lda8((u8)Y); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); Y = (u8)A; nz8((u8)Y);
                lda8(0x03);
                wr8(ABSY(0x1820), 0x03); wr8(ABSY(0x1822), 0x03);
                port_ws_rider_sprites(); ret();
                return;
            }
            lda8(0xF0);
            wr8(ABSY(0x1601), 0xF0); wr8(ABSY(0x1609), 0xF0);
            lda8((u8)Y); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); Y = (u8)A; nz8((u8)Y);
            lda8(0x03);
            wr8(ABSY(0x1821), 0x03); wr8(ABSY(0x1823), 0x03);
            port_ws_rider_sprites(); ret();
            return;
        }
        int marker;
        cmp8((u8)A, 0x80);
        if (!fc) marker = 0;
        else {
            cmp8((u8)A, 0xA0);
            if (!fc) marker = 1;
            else {
                cmp8((u8)A, 0xC0);
                if (!fc) { lda8(rd8(DP(0x63))); and8(0x08); marker = !fz; }
                else { lda8(rd8(DP(0x63))); and8(0x04); marker = !fz; }
            }
        }
        if (marker) {
            lda8(0x08); wr8(ABSY(0x1700), 0x08);
            lda8(0x88); wr8(ABSY(0x1702), 0x88);
            lda8(0x21); wr8(ABSY(0x1703), 0x21);
            lda8(rd8(ABSX(0x0958)));
            push8((u8)Y);
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0xCF2D)));
            Y = pull8(); nz8((u8)Y);
            wr8(ABSY(0x1701), (u8)A);
            push8((u8)Y);
            lda8((u8)Y); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); Y = (u8)A; nz8((u8)Y);
            lda8(0x02); wr8(ABSY(0x1860), 0x02);
            Y = pull8(); nz8((u8)Y);
        }
    } else {
        cmp8((u8)A, 0x02);
        if (!fz) {
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(rd8(ABSX(0x0196)));
                cmp8((u8)A, 0xF0);
                if (!fc) { port_ws_rider_sprites(); ret(); return; }
                hide_top = 0;
            }
        }
    }
    if (hide_top) {
        lda8(0xF0);
        wr8(ABSY(0x1601), 0xF0); wr8(ABSY(0x1609), 0xF0);
    }
    lda8(0xF0);
    wr8(ABSY(0x1605), 0xF0); wr8(ABSY(0x160D), 0xF0);
    port_ws_rider_sprites(); ret();
}

/* $80:E4CB ShowJumpResult (RTS): freeze the sprites ($1958 + 1), draw the
 * ten score sprites at $1700 (X from $80:E5A6 + 6, tiles $80:E59C), the
 * jump number (count $1578 + 1, one or two digits from $80:E588 and
 * $80:E592) at $1600, and queue the course name tile ($80:E584 by $7C) at
 * VRAM $1886. */
void ShowJumpResult_m1x1(void)
{
    wr8(ABS(0x1958), inc8(rd8(ABS(0x1958))));
    ldx8(0x09);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0xE5A6)));
        fc = 0; adc8(0x06);
        wr8(ABSY(0x1700), (u8)A);
        lda8(0x50); wr8(ABSY(0x1701), 0x50);
        lda8(rd8(ABSX(0xE59C))); wr8(ABSY(0x1702), (u8)A);
        lda8(0x31); wr8(ABSY(0x1703), 0x31);
        lda8(0x02); wr8(ABSX(0x1860), 0x02);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x70); wr8(ABS(0x1600), 0x70);
    lda8(0x80); wr8(ABS(0x1604), 0x80);
    lda8(0x38); wr8(ABS(0x1601), 0x38); wr8(ABS(0x1605), 0x38);
    lda8(0x31); wr8(ABS(0x1603), 0x31); wr8(ABS(0x1607), 0x31);
    lda8(0x02); wr8(ABS(0x1820), 0x02); wr8(ABS(0x1821), 0x02);
    wr8(ABS(0x1578), inc8(rd8(ABS(0x1578))));
    X = rd8(ABS(0x1578)); nz8((u8)X);
    lda8(rd8(ABSX(0xE588))); wr8(ABS(0x1602), (u8)A);
    cmp8((u8)X, 0x0A);
    if (!fc) {
        lda8(rd8(ABSX(0xE592))); wr8(ABS(0x1606), (u8)A);
    } else {
        lda8(rd8(ABS(0xE592))); wr8(ABS(0x1606), (u8)A);
        ldx8(0x00);
        lda8(rd8(ABS(0x1578)));
        for (;;) {
            cmp8((u8)A, 0x0A);
            if (!fc) break;
            X = inc8((u8)X);
            fc = 1;
            sbc8(0x0A);
            WAIT_TICK();
        }
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSY(0xE588))); wr8(ABS(0x1602), (u8)A);
        lda8(0x64); wr8(ABS(0x1608), 0x64);
        lda8(0x38); wr8(ABS(0x1609), 0x38);
        lda8(rd8(ABSX(0xE588))); wr8(ABS(0x160A), (u8)A);
        lda8(0x31); wr8(ABS(0x160B), 0x31);
        lda8(0x02); wr8(ABS(0x1822), 0x02);
    }
    X = rd8(DP(0x7C)); nz8((u8)X);
    lda8(0x11); wr8(ABS(0x03CF), 0x11);
    lda8(rd8(ABSX(0xE584))); wr8(ABS(0x03D0), (u8)A);
    rep(0x20);
    lda16(0x1886); wr16(DP(0x00), 0x1886);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0xE582); VramQueueTiles20_m1x1();
    ret();
}

/* $81:9DE4 DrawBigBall (RTS): a 2x2 ball sprite (tiles $DD/$DE/$ED/$EE,
 * attribute $2C) at OAM buffer $17F0 for object $03A3, from its 16-bit
 * position $027F - 10 / $028F + 6; near the left edge (X high byte $FF)
 * or right edge it is clipped by hiding columns and setting the size/X9
 * bits $189C-$189F. */
void DrawBigBall_m1x1(void)
{
    rep(0x20);
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda16(rd16(ABSX(0x027F)));
    fc = 0; adc16(0xFFF6);
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(0x028F)));
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

/* $80:DE5E BonusResultStep (RTS): when the timer $88 runs out keep the
 * best count ($1E88 = max with $09C5); with fewer than 10 tries left
 * ($1E87) queue the result banner ($80:DF26 at VRAM $1950) and the count
 * box at VRAM $1956 ("1 0000" for a full score $09C9, else the count with
 * blank leading digits), then wait $0180 frames and step $77. */
void BonusResultStep_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x09C5)));
    cmp8((u8)A, rd8(ABS(0x1E88)));
    if (fc) wr8(ABS(0x1E88), (u8)A);
    lda8(rd8(ABS(0x1E87)));
    int show = 1;
    if (!fn) { cmp8((u8)A, 0x0A); show = !fc; }
    if (show) {
        rep(0x20);
        lda16(0x1950); wr16(DP(0x00), 0x1950);
        lda16(0xDF26); wr16(DP(0x02), 0xDF26);
        sep(0x20);
        push16(0xDE8C); VramQueueTiles20_m1x1();
        rep(0x20);
        lda16(0x1956); wr16(DP(0x00), 0x1956);
        lda16(0x03CF); wr16(DP(0x02), 0x03CF);
        sep(0x20);
        lda8(0x00);
        wr8(ABS(0x03D0), 0x00);
        wr8(ABS(0x03D0), inc8(rd8(ABS(0x03D0))));
        for (u16 a = 0x03D1; a <= 0x03D4; a++) wr8(ABS(a), 0x00);
        lda8(0x10);
        wr8(ABS(0x03D5), 0x10);
        wr8(ABS(0x03D5), inc8(rd8(ABS(0x03D5))));
        for (u16 a = 0x03D6; a <= 0x03D9; a++) wr8(ABS(a), 0x10);
        lda8(rd8(ABS(0x09C5)));
        cmp8((u8)A, rd8(ABS(0x09C9)));
        if (!fz) {
            lda8(0xFF);
            wr8(ABS(0x03D0), 0xFF);
            wr8(ABS(0x03D5), 0xFF);
            ldy8(0x00);
            lda8(rd8(ABS(0x09C5)));
            for (;;) {
                cmp8((u8)A, 0x0A);
                if (!fc) break;
                sbc8(0x0A);
                Y = inc8((u8)Y);
                WAIT_TICK();
            }
            wr8(ABS(0x03D2), (u8)A);
            ora8(0x10); wr8(ABS(0x03D7), (u8)A);
            lda8((u8)Y);
            wr8(ABS(0x03D1), (u8)A);
            ora8(0x10); wr8(ABS(0x03D6), (u8)A);
            lda8(rd8(ABS(0x03D1)));
            if (fz) {
                lda8(0xFF);
                wr8(ABS(0x03D1), 0xFF); wr8(ABS(0x03D6), 0xFF);
                lda8(rd8(ABS(0x03D2)));
                if (fz) {
                    lda8(0xFF);
                    wr8(ABS(0x03D2), 0xFF); wr8(ABS(0x03D3), 0xFF);
                    wr8(ABS(0x03D7), 0xFF); wr8(ABS(0x03D8), 0xFF);
                }
            }
        }
        lda8(0x25); wr8(ABS(0x03CF), 0x25);
        push16(0xDF19); VramQueueTiles20_m1x1();
    }
    rep(0x20);
    lda16(0x0180); wr16(DP(0x88), 0x0180);
    sep(0x20);
    wr8(DP(0x77), inc8(rd8(DP(0x77))));
    ret();
}

/* $81:C3BA DrawJumpCrowd (RTS): the jump-contest flags. At the start
 * ($3C = 1) begin the countdown ($C3 + 1, $8A = $40 unless $92 is set) and
 * set $92 = 1, $3C = 2. Before the run ($92 clear) every 16 frames of $3C
 * ping the APU. Then draw the eight flag poles ($0936 - scroll $9A, Y from
 * $81:C461, tile $E1-$E3 by $3C, attribute $3F) or retire ones that left
 * the screen ($025E = 1); with $54 set, $3C = $B5. */
void DrawJumpCrowd_m1x1(void)
{
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(rd8(DP(0x92)));
        if (fz) {
            wr8(DP(0xC3), inc8(rd8(DP(0xC3))));
            lda8(0x40); wr8(DP(0x8A), 0x40);
        }
        lda8(0x01); wr8(DP(0x92), 0x01);
        lda8(0x02); wr8(DP(0x3C), 0x02);
    } else {
        Y = rd8(DP(0x92)); nz8((u8)Y);
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
    wr8(DP(0x00), (u8)Y);
    ldy8(0x07);
    do {
        lda8(rd8(ABSY(0x025E)));
        if (fz) {
            wr8(DP(0x08), 0);
            lda8(rd8(ABSY(0x0936)));
            fc = 1;
            sbc8(rd8(ABS(0x009A)));
            wr8(DP(0x01), (u8)A);
            cmp8((u8)A, 0xF0);
            if (fc) {
                cmp8((u8)A, 0xF9);
                if (fc) wr8(DP(0x08), inc8(rd8(DP(0x08))));
                lda8(0x01);
                wr8(ABSY(0x025E), 0x01);
            } else {
                lda8((u8)Y);
                lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(ABSY(0xC461)));
                wr8(ABSX(0x17A1), (u8)A);
                fc = 0; adc8(0x08);
                wr8(ABSX(0x17A5), (u8)A);
                lda8(rd8(DP(0x00)));
                wr8(ABSX(0x17A2), (u8)A);
                ora8(0x10);
                wr8(ABSX(0x17A6), (u8)A);
                lda8(0x3F);
                wr8(ABSX(0x17A3), 0x3F); wr8(ABSX(0x17A7), 0x3F);
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
    if (!fz) { lda8(0xB5); wr8(DP(0x3C), 0xB5); }
    ret();
}

/* $80:B65B StartBonusResult (RTS): screen setup as in practice mode (BG
 * maps, $1905 = $17, the HDMA scroll channels), the try count $1E95 at
 * VRAM $1957, and when any bonus points were scored ($1E92-$1E94) their
 * total as up to three digits at VRAM $19B3; colour math ($190D $41,
 * COLDATA $E0) and sound 3. */
void StartBonusResult_m1x1(void)
{
    lda8(0x01); wr8(ABS(0x2107), 0x01); wr8(ABS(0x1900), 0x01);
    lda8(0x09); wr8(ABS(0x2108), 0x09);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    wr8(DP(0x86), 0);
    wr8(DP(0x87), 0);
    push8(0x80); push16(0xB674); SetupHdmaChannels_m1x1();
    lda8(0x1A); wr8(ABS(0x2109), 0x1A);
    lda8(0x44); wr8(ABS(0x210B), 0x44);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    lda8(rd8(ABS(0x1E95)));
    wr8(ABS(0x03D0), (u8)A);
    ora8(0x10); wr8(ABS(0x03D1), (u8)A);
    lda8(0x21); wr8(ABS(0x03CF), 0x21);
    rep(0x20);
    lda16(0x1957); wr16(DP(0x00), 0x1957);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0xB6A4); VramQueueTiles20_m1x1();
    lda8(rd8(ABS(0x1E92)));
    ora8(rd8(ABS(0x1E93)));
    ora8(rd8(ABS(0x1E94)));
    if (!fz) {
        lda8(rd8(ABS(0x1E94)));
        ldy8(0x00);
        for (;;) { cmp8((u8)A, 0x0A); if (!fc) break; sbc8(0x0A); Y = inc8((u8)Y); WAIT_TICK(); }
        wr8(ABS(0x03D2), (u8)A);
        ora8(0x10); wr8(ABS(0x03D6), (u8)A);
        lda8((u8)Y);
        fc = 0; adc8(rd8(ABS(0x1E93)));
        ldy8(0x00);
        for (;;) { cmp8((u8)A, 0x0A); if (!fc) break; sbc8(0x0A); Y = inc8((u8)Y); WAIT_TICK(); }
        wr8(ABS(0x03D1), (u8)A);
        ora8(0x10); wr8(ABS(0x03D5), (u8)A);
        lda8((u8)Y);
        fc = 0; adc8(rd8(ABS(0x1E92)));
        wr8(ABS(0x03D0), (u8)A);
        ora8(0x10); wr8(ABS(0x03D4), (u8)A);
        lda8(0x00); wr8(ABS(0x03D3), 0x00);
        ora8(0x10); wr8(ABS(0x03D7), (u8)A);
        lda8(rd8(ABS(0x03D0)));
        if (fz) {
            lda8(0xFF);
            wr8(ABS(0x03D0), 0xFF); wr8(ABS(0x03D4), 0xFF);
            lda8(rd8(ABS(0x03D1)));
            if (fz) {
                lda8(0xFF);
                wr8(ABS(0x03D1), 0xFF); wr8(ABS(0x03D5), 0xFF);
            }
        }
        lda8(0x24); wr8(ABS(0x03CF), 0x24);
        rep(0x20);
        lda16(0x19B3); wr16(DP(0x00), 0x19B3);
        lda16(0x03CF); wr16(DP(0x02), 0x03CF);
        sep(0x20);
        push16(0xB724); VramQueueTiles20_m1x1();
    }
    lda8(0x41); wr8(ABS(0x190D), 0x41);
    lda8(0xE0); wr8(ABS(0x2132), 0xE0);
    lda8(0x03);
    push8(0x80); push16(0xB734); QueueSoundB_m1x1();
    ret();
}

/* Queue the 2-tile pair `t0`/`t1` (tables by $B7) for the track column
 * $0266 - $10 into the BG map at `bases` ($80:A84F by 2 * $B7). */
static void queue_bonus_tiles(u16 t0, u16 t1)
{
    rep(0x20);
    lda16(rd16(ABS(0x0266)));
    fc = 1; sbc16(rd16(DP(0x10)));
    and16(0x0020);
    for (int i = 0; i < 5; i++) lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(rd16(ABS(0x0266)));
    fc = 1; sbc16(rd16(DP(0x10)));
    and16(0x001F);
    fc = 0; adc16(rd16(DP(0x00)));
    fc = 0; adc16(rd16(ABSX(0xA84F)));
    wr16(DP(0x00), A);
    lda16(0x0018); wr16(DP(0x02), 0x0018);
    sep(0x20);
    X = rd8(DP(0xB7)); nz8((u8)X);
    lda8(0x21); wr8(DP(0x18), 0x21);
    lda8(rd8(ABSX(t0))); wr8(DP(0x19), (u8)A);
    lda8(rd8(ABSX(t1))); wr8(DP(0x1A), (u8)A);
    lda8(0x08); wr8(DP(0x0F), 0x08);
}

/* $80:A76C DrawBonusPickup (RTL): when an item was hit ($B7), redraw the
 * two map cells behind it (one or two columns back by the tile under the
 * player, $80:A831 rows and $80:A82C markers) with the tiles from
 * $80:A83B/$80:A840 and $80:A845/$80:A84A, then clear $B7. */
void DrawBonusPickup_m1x1(void)
{
    push8((u8)X);
    lda8(rd8(DP(0xB7)));
    if (fz) { X = pull8(); nz8((u8)X); retl(); return; }
    wr8(DP(0x10), 0);
    wr8(DP(0x11), 0);
    lda8(rd8(DP(0xB7)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xA831)));
    wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(ABS(0x0266)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
    Y = rd8(ABS(0x00B7)); nz8((u8)Y);
    cmp8((u8)A, rd8(ABSY(0xA82C)));
    if (!fz) {
        cmp8((u8)A, 0x3C);
        if (!fz) cmp8((u8)A, 0x3B);
        if (fz) wr8(DP(0x10), inc8(rd8(DP(0x10))));
    }
    wr8(DP(0x10), inc8(rd8(DP(0x10))));
    queue_bonus_tiles(0xA83B, 0xA840);
    push16(0xA7DF); VramQueueTiles_m1x1();
    wr8(DP(0x10), dec8(rd8(DP(0x10))));
    lda8(rd8(DP(0xB7)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    queue_bonus_tiles(0xA845, 0xA84A);
    push16(0xA827); VramQueueTiles_m1x1();
    wr8(DP(0xB7), 0);
    X = pull8(); nz8((u8)X);
    retl();
}

void CollectCoin_m1x1(void);

/* As queue_bonus_tiles, for the item index in `dp` with the map bases at
 * `bases` (X = 2 * index on entry). */
static void queue_item_tiles(u8 dp, u16 bases, u16 t0, u16 t1)
{
    rep(0x20);
    lda16(rd16(ABS(0x0266)));
    fc = 1; sbc16(rd16(DP(0x10)));
    and16(0x0020);
    for (int i = 0; i < 5; i++) lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(rd16(ABS(0x0266)));
    fc = 1; sbc16(rd16(DP(0x10)));
    and16(0x001F);
    fc = 0; adc16(rd16(DP(0x00)));
    fc = 0; adc16(rd16(ABSX(bases)));
    wr16(DP(0x00), A);
    lda16(0x0018); wr16(DP(0x02), 0x0018);
    sep(0x20);
    X = rd8(DP(dp)); nz8((u8)X);
    lda8(0x21); wr8(DP(0x18), 0x21);
    lda8(rd8(ABSX(t0))); wr8(DP(0x19), (u8)A);
    lda8(rd8(ABSX(t1))); wr8(DP(0x1A), (u8)A);
    lda8(0x08); wr8(DP(0x0F), 0x08);
}

/* $80:A66F DrawCoinPickup (RTL): when a coin was taken ($B2), redraw the
 * two map cells behind it (one or two columns back, rows $80:A74E, marker
 * $80:A749) with the tiles $80:A758/$80:A75D, count it ($0CDF, CollectCoin),
 * and at the fifth coin start the turbo ($0CE0 = 1, sounds $2D and 1, the
 * meter $0990 = $20). */
void DrawCoinPickup_m1x1(void)
{
    push8((u8)X);
    lda8(rd8(DP(0xB2)));
    if (fz) { X = pull8(); nz8((u8)X); retl(); return; }
    lda8(0x01); wr8(DP(0x10), 0x01);
    wr8(DP(0x11), 0);
    lda8(rd8(DP(0xB2)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    rep(0x20);
    lda16(rd16(ABSX(0xA74E)));
    wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(ABS(0x0266)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
    Y = rd8(ABS(0x00B2)); nz8((u8)Y);
    cmp8((u8)A, rd8(ABSY(0xA749)));
    if (!fz) wr8(DP(0x10), inc8(rd8(DP(0x10))));
    queue_item_tiles(0xB2, 0xA762, 0xA758, 0xA75D);
    push16(0xA6DA); VramQueueTiles_m1x1();
    wr8(DP(0x10), dec8(rd8(DP(0x10))));
    lda8(rd8(DP(0xB2)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    queue_item_tiles(0xB2, 0xA762, 0xA758, 0xA75D);
    push16(0xA722); VramQueueTiles_m1x1();
    wr8(DP(0xB2), 0);
    wr8(ABS(0x0CDF), inc8(rd8(ABS(0x0CDF))));
    push8(0x80); push16(0xA72B); CollectCoin_m1x1();
    lda8(rd8(ABS(0x0CDF)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x2D);
        push8(0x80); push16(0xA738); QueueSoundB_m1x1();
        lda8(0x01); wr8(ABS(0x0CE0), 0x01);
        push8(0x80); push16(0xA741); QueueSoundB_m1x1();
        lda8(0x20); wr8(ABS(0x0990), 0x20);
    }
    X = pull8(); nz8((u8)X);
    retl();
}

/* $80:CD34 TurboMeter (RTS): the player's turbo heat $0990 (fraction
 * $098F) rises with A/B held toward the limit $80:BB1B by button (rate
 * $80:BB17) and cools by $0B otherwise; at $20 it overheats ($60, $0940,
 * sounds $2E/$3E), and above $1A on the turbo button the warning sound
 * $2C plays ($AA). Then the meter's four sprites at $17F0 (blinking
 * while overheated, full while the coin turbo $0CE0 runs). */
void TurboMeter_m1x1(void)
{
    u8 y01;
    lda8(rd8(DP(0x84)));
    ora8(rd8(ABS(0x0CE0)));
    if (!fz) { y01 = 0xFF; goto draw; }
    lda8(rd8(ABS(0x0940)));
    if (!fz) {
        lda8(rd8(DP(0x60)));
        lda8(lsr8((u8)A));
        if (fc) { y01 = 0xFF; goto draw; }
        y01 = 0x01;
        goto draw;
    }
    lda8(rd8(ABS(0x017E)));
    and8(0xC0);
    if (!fz) {
        Y = rd8(DP(0x92)); nz8((u8)Y);
        if (fz) lda8(0x80);
        lda8(asl8((u8)A));
        lda8(rol8((u8)A));
        lda8(rol8((u8)A));
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(0x0990)));
    cmp8((u8)A, rd8(ABSY(0xBB1B)));
    if (!fc) {
        lda8(rd8(ABS(0x098F)));
        fc = 0; adc8(rd8(ABSY(0xBB17)));
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
        lda8(0x2E);
        push8(0x80); push16(0xCDA4); QueueSoundB_m1x1();
        lda8(0x3E);
        push8(0x80); push16(0xCDAA); QueueSoundB_m1x1();
        lda8(pull8());
    }
    cmp8((u8)A, 0x1A);
    if (fc) {
        Y = dec8((u8)Y);
        if (fz) {
            lda8(rd8(DP(0xAA)));
            if (fz) {
                lda8(rd8(ABS(0x0CE0)));
                if (fz) {
                    lda8(0x2C);
                    push8(0x80); push16(0xCDC1); QueueSoundB_m1x1();
                }
                wr8(DP(0xAA), (u8)A);
            }
            y01 = 0xFF;
            goto draw;
        }
    }
    lda8(rd8(DP(0xAA)));
    if (!fz) {
        lda8(0x2D);
        push8(0x80); push16(0xCDCF); QueueSoundB_m1x1();
        wr8(DP(0xAA), 0);
    }
    y01 = 0xFF;
draw:
    lda8(y01);
    wr8(DP(0x01), y01);
    lda8(rd8(ABS(0x0990)));
    fc = 0; adc8(0x58);
    X = rd8(ABS(0x0CE0)); nz8((u8)X);
    if (!fz) lda8(0x78);
    wr8(DP(0x00), (u8)A);
    ldx8(0x0C);
    lda8(rd8(ABS(0x0CE0)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    do {
        lda8(rd8(DP(0x00)));
        wr8(ABSX(0x17F0), (u8)A);
        fc = 0; adc8(0x08);
        wr8(DP(0x00), (u8)A);
        cmp8((u8)A, 0x70);
        lda8(fc ? 0xC7 : 0xF0);
        wr8(ABSX(0x17F1), (u8)A);
        lda8(rd8(ABSY(0xCE17)));
        wr8(ABSX(0x17F2), (u8)A);
        lda8(0x2F);
        wr8(ABSX(0x17F3), 0x2F);
        Y = inc8((u8)Y);
        X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:EBD0 ReadBonusTrack (RTS): the bonus course's ReadJumpTrack. As
 * that routine (landing points, height, tile $09D3 into $0216, tile flags
 * $0A53, tile $84), plus: tile $A7 under or behind the wheel lands the
 * rider back on the ground at height $B0 once the zoom $A2 is $0100; tile
 * 0 starts the run-out ($C7 = 2, height $80) or, before that, the drop
 * ($C7 = 1, $C8 = scroll); tiles 1-$27 end the course once. */
void ReadBonusTrack_m1x1(void)
{
    lda8(rd8(DP(0xC7)));
    cmp8((u8)A, 0x02);
    if (fz) wr8(ABS(0x017E), 0);
    X = rd8(ABS(0x0266)); nz8((u8)X);
    lda8(rd8(ABS(0x09C7)));
    ora8(rd8(ABS(0x01EE)));
    if (!fz) {
        lda8(rd8(ABS(0x0266)));
        cmp8((u8)A, rd8(ABS(0x09C8)));
        if (!fz) {
            wr8(ABS(0x09C8), (u8)A);
            lda8(rd8(ABSX(0x0A93)));
            fc = 0; adc8(rd8(ABS(0x09C5)));
            wr8(ABS(0x09C5), (u8)A);
        }
    }
    lda8(rd8(ABS(0x01CE)));
    if (fz) { lda8(rd8(ABSX(0x0A13))); wr8(ABS(0x09C4), (u8)A); }
    lda8(rd8(ABSX(0x09D3)));
    wr8(ABS(0x0216), (u8)A);
    lda8(rd8(ABSX(0x09D2)));
    cmp8((u8)A, 0xA7);
    int land = fz;
    if (!land) {
        lda8(rd8(ABSX(0x09D3)));
        cmp8((u8)A, 0xA7);
        land = fz;
    }
    if (land) {
        lda8(rd8(ABS(0x01B6)));
        cmp8((u8)A, 0xB0);
        if (fc) {
            rep(0x20);
            lda16(rd16(ABS(0x00A2)));
            cmp16(A, 0x0100);
            sep(0x20);
            if (fz) {
                lda8(0xB0); wr8(ABS(0x01B6), 0xB0);
                wr8(ABS(0x01EE), 0); wr8(ABS(0x0226), 0); wr8(ABS(0x01FE), 0);
                wr8(ABS(0x0176), 0); wr8(ABS(0x01A6), 0);
            }
        }
        goto flags;
    }
    cmp8((u8)A, 0x28);
    if (fc) goto flags;
    cmp8((u8)A, 0x00);
    if (!fz) {
        lda8(rd8(ABS(0x099C)));
        if (fz) {
            lda8(0x01);
            wr8(ABS(0x099C), 0x01);
            wr8(DP(0x84), 0x01);
            lda8(0x2B);
            push8(0x80); push16(0xEC8F); QueueSoundB_m1x1();
        }
        ret();
        return;
    }
    lda8(rd8(ABS(0x01B6)));
    cmp8((u8)A, 0xB0);
    if (fc) {
        rep(0x20);
        lda16(rd16(ABS(0x00A2)));
        cmp16(A, 0x0100);
        sep(0x20);
        if (fz) {
            lda8(0x02); wr8(DP(0xC7), 0x02);
            lda8(0x80); wr8(ABS(0x01B6), 0x80);
            wr8(ABS(0x01EE), 0); wr8(ABS(0x0226), 0); wr8(ABS(0x01FE), 0);
            ret();
            return;
        }
    }
    lda8(0x01); wr8(DP(0xC7), 0x01);
    rep(0x20);
    lda16(rd16(DP(0x9C)));
    wr16(DP(0xC8), A);
    sep(0x20);
    ret();
    return;
flags:
    lda8(rd8(ABSX(0x0A53)));
    if (!fz) {
        lda8(rd8(ABS(0x09C7)));
        ora8(rd8(ABS(0x01EE)));
        if (!fz) { lda8(0x45); wr8(ABS(0x0216), 0x45); ret(); return; }
        lda8(rd8(ABSX(0x09D3)));
        and8(0x40);
        if (!fz) { ret(); return; }
        wr8(ABSX(0x0A53), 0);
        lda8(0x3B);
        wr8(ABSX(0x0B53), 0x3B); wr8(ABSX(0x0B93), 0x3B); wr8(ABSX(0x0BD3), 0x3B);
        wr8(ABSX(0x0C13), 0x3B); wr8(ABSX(0x0C53), 0x3B); wr8(ABSX(0x0C93), 0x3B);
        wr8(ABS(0x021E), 0x3B);
    }
    lda8(rd8(ABSX(0x09D3)));
    cmp8((u8)A, 0x84);
    if (fz) {
        lda8(rd8(ABS(0x09C7)));
        if (fz) { lda8(0xFC); wr8(ABS(0x0216), 0xFC); }
    }
    ret();
}

/* $80:9B0D EnterNumberStep (RTS): a three-digit entry ($1570-$1572,
 * cursor $044F): up/down ($21 bits 3/2, sound $33) step the digit 0-9
 * with wrap, left/right move the cursor (wrapping 0-2); the number never
 * becomes 000 (the last digit is forced to 1, or 9 when stepping down).
 * Queue the digits ($156F, size $23, lower row +$10) at VRAM $0B0F and the
 * cursor arrows; Start ($21 bit 4) confirms ($0450 + 1) and queues the
 * next screen's tiles ($7E:5A80, $20 x 2 rows at VRAM $0300). */
void EnterNumberStep_m1x1(void)
{
    X = rd8(ABS(0x044F)); nz8((u8)X);
    ldy8(0x01);
    lda8(rd8(DP(0x21)));
    and8(0x0C);
    if (!fz) {
        lda8(0x33);
        push8(0x80); push16(0x9B1D); QueueSoundB_m1x1();
        lda8(rd8(DP(0x21)));
        and8(0x08);
        if (!fz) {
            lda8(rd8(ABSX(0x1570)));
            lda8(inc8((u8)A));
            wr8(ABSX(0x1570), (u8)A);
            cmp8((u8)A, 0x0A);
            if (fc) wr8(ABSX(0x1570), 0);
        } else {
            ldy8(0x09);
            lda8(rd8(ABSX(0x1570)));
            lda8(dec8((u8)A));
            wr8(ABSX(0x1570), (u8)A);
            if (fn) { lda8(0x09); wr8(ABSX(0x1570), 0x09); }
        }
    }
    lda8(rd8(DP(0x21)));
    and8(0x03);
    if (!fz) {
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x9C10)));
        fc = 0; adc8(rd8(ABS(0x044F)));
        if (fn) lda8(0x02);
        else { cmp8((u8)A, 0x03); if (fc) lda8(0x00); }
        wr8(ABS(0x044F), (u8)A);
    }
    lda8(rd8(ABS(0x1570)));
    ora8(rd8(ABS(0x1571)));
    ora8(rd8(ABS(0x1572)));
    if (fz) {
        cmp8((u8)X, 0x02);
        if (!fz) ldy8(0x01);
        wr8(ABS(0x1572), (u8)Y);
    }
    for (int i = 0; i < 3; i++) {
        lda8(rd8(ABS(0x1570 + i)));
        ora8(0x10);
        wr8(ABS(0x1573 + i), (u8)A);
    }
    rep(0x20);
    lda16(0x0B0F); wr16(DP(0x00), 0x0B0F);
    lda16(0x156F); wr16(DP(0x02), 0x156F);
    sep(0x20);
    lda8(0x23); wr8(ABS(0x156F), 0x23);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push16(0x9BA6); VramQueueTiles_m1x1();
    lda8(rd8(ABS(0x044F)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    fc = 0; adc8(0x78);
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
    wr8(ABS(0x1820), 0);
    wr8(ABS(0x1821), 0);
    lda8(rd8(DP(0x21)));
    and8(0x10);
    if (!fz) {
        wr8(ABS(0x0450), inc8(rd8(ABS(0x0450))));
        lda8(0x80); wr8(DP(0x02), 0x80);
        lda8(0x5A); wr8(DP(0x03), 0x5A);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x9C0E); QueueTileRows_m1x1();
    }
    ret();
}

void DrawRiderShadowPair_m1x1(void);
int port_ws_shadow_margin(void);   /* roster.h: the port's widescreen */
void port_ws_shadow_fix(void);

/* $80:CC18 DrawRiderShadow (RTS): rider X's shadow while it is in the air
 * on a ramp ($08BC): two sprites under it (DrawRiderShadowPair at OAM
 * buffer $16C0 + Y), blinking every 4 frames, palette from the track piece
 * (or $0E off the track and above height $58), attribute | $21, the lower
 * one hidden while the speed high byte is zero. Only for a moving rider on
 * the original screen ($019E = 1) from column $10. */
void DrawRiderShadow_m1x1(void)
{
    lda8(rd8(ABSX(0x08BC)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x01C6)));
    ora8(rd8(ABSX(0x01BE)));
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x01FE)));
    if (!fz) { ret(); return; }
    int margin = port_ws_shadow_margin();
    if (margin) {
        lda8(rd8(ABSX(0x0196)));
    } else {
        lda8(rd8(ABSX(0x019E)));
        cmp8((u8)A, 0x01);
        if (!fz) { ret(); return; }
        lda8(rd8(ABSX(0x0196)));
        cmp8((u8)A, 0x10);
        if (!fc) { ret(); return; }
    }
    push16(0xCC3A); DrawRiderShadowPair_m1x1();
    lda8(rd8(DP(0x63)));
    and8(0x04);
    if (!fz) {
        lda8(0xF0); wr8(ABSY(0x16C2), 0xF0);
        lda8(0xE0); wr8(ABSY(0x16C6), 0xE0);
    }
    lda8(rd8(ABSX(0x0958)));
    if (fz) {
        lda8(0x0E); wr8(DP(0x02), 0x0E);
    } else {
        push8((u8)Y);
        Y = rd8(ABSX(0x0216)); nz8((u8)Y);
        lda8(rd8(ABSY(0xAD33)));
        Y = pull8(); nz8((u8)Y);
        wr8(DP(0x02), (u8)A);
        cmp8((u8)A, 0x00);
        if (fz) {
            lda8(0xF0);
            wr8(ABSY(0x16C1), 0xF0);
            wr8(ABSY(0x16C5), 0xF0);
        }
    }
    lda8(rd8(ABSX(0x01B6)));
    cmp8((u8)A, 0x58);
    if (!fc) lda8(0x0E);
    else lda8(rd8(DP(0x02)));
    ora8(0x21);
    wr8(ABSY(0x16C3), (u8)A);
    wr8(ABSY(0x16C7), (u8)A);
    lda8(rd8(ABSX(0x01C6)));
    if (fz) { lda8(0xF8); wr8(ABSY(0x16C5), 0xF8); }
    if (margin) port_ws_shadow_fix();
    ret();
}

void BsxReadHeader3_m1x1(void);
void ScrollByPlayerSpeed_m1x1(void);
void RivalTarget_m1x1(void);
void FadePalette_m1x1(void);
void QueueRaceTime_m1x1(void);

/* $80:9FE1 BsxEndIfFlagged (RTS): read the broadcast header; when its
 * flags ($01 & 3) are 3, hand control back to the receiver ($2197 = $80). */
void BsxEndIfFlagged_m1x1(void)
{
    push8(0x80); push16(0x9FE4); BsxReadHeader3_m1x1();
    lda8(rd8(DP(0x01)));
    and8(0x03);
    cmp8((u8)A, 0x03);
    if (fz) { lda8(0x80); wr8(ABS(0x2197), 0x80); }
    ret();
}

/* $80:EE9D CopyPieceColumn (RTS): copy the words at ($05) + Y + 2, ... into
 * the VRAM queue $7F:000E + X until X = $1E. */
void CopyPieceColumn_m1x1(void)
{
    do {
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        rep(0x20);
        lda16(rd16((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        wr16(LONGX(0x7F000E), A);
        X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x1E);
    } while (!fz);
    sep(0x20);
    ret();
}

/* $80:DBD8 CopyPieceColumnPal2 (RTS): as CopyPieceColumn, each word masked
 * to $20FF and given palette 2 ($0800). */
void CopyPieceColumnPal2_m1x1(void)
{
    do {
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        rep(0x20);
        lda16(rd16((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
        and16(0x20FF);
        ora16(0x0800);
        wr16(LONGX(0x7F000E), A);
        X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x1E);
    } while (!fz);
    sep(0x20);
    ret();
}

/* $80:B0D0 RetargetRival (RTS): rival X (not rider 7) picks a new target
 * speed (RivalTarget) every time it passes a 32-column boundary with the
 * random $2E,X at $C0 or more (Y = 1 when above $C0); rider 7 always. */
void RetargetRival_m1x1(void)
{
    ldy8(0x00);
    cmp8((u8)X, 0x07);
    if (!fz) {
        lda8(rd8(ABSX(0x0196)));
        and8(0x10);
        if (fz) { ret(); return; }
        lda8(rd8(DPX(0x2E)));
        cmp8((u8)A, 0xC0);
        if (!fc) { ret(); return; }
        if (!fz) Y = inc8((u8)Y);
    }
    push16(0xB0E8); RivalTarget_m1x1();
    ret();
}

/* $80:DA55 BonusTimerStep (RTS): the bonus course's countdown: every $3C
 * frames ($B5) one second off $B4; at zero the run ends ($84 = 2, $1990 = 2
 * unless set, sound $2D). Then the counter display. */
void BonusTimerStep_m1x1(void)
{
    u8 t = dec8(rd8(DP(0xB5)));
    wr8(DP(0xB5), t);
    if (fz) {
        lda8(0x3C); wr8(DP(0xB5), 0x3C);
        u8 s = dec8(rd8(DP(0xB4)));
        wr8(DP(0xB4), s);
        if (fz) {
            lda8(0x02); wr8(DP(0x84), 0x02);
            X = rd8(ABS(0x1990)); nz8((u8)X);
            if (fz) wr8(ABS(0x1990), (u8)A);
            lda8(0x2D);
            push8(0x80); push16(0xDA72); QueueSoundB_m1x1();
        }
    }
    push16(0xDA75); QueueBonusCount_m1x1();
    ret();
}

/* $80:DA2A BonusScrollStep (RTS): while the bonus run lasts the track
 * scrolls with the player (ScrollByPlayerSpeed); after it the player rides
 * on across the screen ($0196 + speed) and at column $F0 leaves it ($84 =
 * $019E = 2, sound $2D, the counter display); otherwise the speed is
 * cleared for the frame. */
void BonusScrollStep_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    if (fz) { ScrollByPlayerSpeed_m1x1(); return; }
    lda8(rd8(ABS(0x0186)));
    fc = 0; adc8(rd8(ABS(0x0196)));
    wr8(ABS(0x0196), (u8)A);
    cmp8((u8)A, 0xF0);
    if (fc) {
        lda8(0x02);
        wr8(DP(0x84), 0x02);
        wr8(ABS(0x019E), 0x02);
        lda8(0x2D);
        push8(0x80); push16(0xDA4B); QueueSoundB_m1x1();
        push16(0xDA75); QueueBonusCount_m1x1();
        ret();
        return;
    }
    lda8(0x00);
    wr8(ABS(0x0186), 0x00);
    ret();
}

/* $80:9D0E LoadEntryScreenStep (RTS): count $88 down; at zero queue the
 * entry screen's tiles ($7E:5B00, $20 x 2 rows at VRAM $0300) and go to
 * step 9 ($0450). */
void LoadEntryScreenStep_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    lda8(0x00); wr8(DP(0x02), 0x00);
    lda8(0x5B); wr8(DP(0x03), 0x5B);
    lda8(0x7E); wr8(DP(0x04), 0x7E);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x9D30); QueueTileRows_m1x1();
    lda8(0x09); wr8(ABS(0x0450), 0x09);
    ret();
}

/* $81:98FF FadeObjectStep (RTS): object $03A3's palette fade: every $C0
 * frames ($88) move the fade start $B9 by 8 until $16, then advance the
 * object's state ($030F,X); between steps FadePalette. */
void FadeObjectStep_m1x1(void)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(DP(0x88)));
    if (fz) {
        lda8(rd8(DP(0xB9)));
        cmp8((u8)A, 0x16);
        if (fc) {
            wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
            X = rd8(ABS(0x03A3)); nz8((u8)X);
            ret();
            return;
        }
        fc = 0; adc8(0x08);
        wr8(DP(0xB9), (u8)A);
        lda8(0xC0); wr8(DP(0x88), 0xC0);
    }
    wr8(DP(0x88), dec8(rd8(DP(0x88))));
    push8(0x81); push16(0x9921); FadePalette_m1x1();
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    ret();
}

/* $80:C58D TickRaceClock (RTS): the race clock $03AE:$03AF:$03B0 (minutes,
 * seconds, frames): carry the frames at $3C into the seconds and the
 * seconds at $3C into the minutes; unless $CB, show it (QueueRaceTime with
 * attribute $20, which returns for this routine). */
void TickRaceClock_m1x1(void)
{
    lda8(0x20); wr8(DP(0x0F), 0x20);
    lda8(rd8(ABS(0x03B0)));
    cmp8((u8)A, 0x3C);
    if (fc) {
        wr8(ABS(0x03AF), inc8(rd8(ABS(0x03AF))));
        lda8(rd8(ABS(0x03AF)));
        cmp8((u8)A, 0x3C);
        if (fc) {
            wr8(ABS(0x03AE), inc8(rd8(ABS(0x03AE))));
            wr8(ABS(0x03AF), 0);
        }
        wr8(ABS(0x03B0), 0);
    }
    lda8(rd8(DP(0xCB)));
    if (fz) { QueueRaceTime_m1x1(); return; }
    ret();
}

void BuildScoreDigits_m0x1(void);
void BuildJumpScore_m1x1(void);
void PlaceRiderShadow_m1x1(void);
void FadeSkyColours_m1x1(void);

/* $80:E956 ShowJumpCount (RTS): the jump count $1569 as five digits plus
 * the score label (BuildJumpScore), queued at VRAM $1970, and the label
 * tiles $80:E989 at VRAM $1967. */
void ShowJumpCount_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(0x1569)));
    push16(0xE95D); BuildScoreDigits_m0x1();
    push8(0x80); push16(0xE961); BuildJumpScore_m1x1();
    rep(0x20);
    lda16(0x1970); wr16(DP(0x00), 0x1970);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xE976); VramQueueTiles_m1x1();
    rep(0x20);
    lda16(0x1967); wr16(DP(0x00), 0x1967);
    lda16(0xE989); wr16(DP(0x02), 0xE989);
    sep(0x20);
    push16(0xE987); VramQueueTiles20_m1x1();
    ret();
}

/* $80:E9B2 ShowBonusPoints (RTS): the points for jump rank $156B (table
 * $80:F155) as five digits at VRAM $19B3, and the label tiles $80:E9E7 at
 * VRAM $19A7. */
void ShowBonusPoints_m1x1(void)
{
    rep(0x20);
    Y = rd8(ABS(0x156B)); nz8((u8)Y);
    lda16(rd16(ABSY(0xF155)));
    and16(0x00FF);
    push16(0xE9BF); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x19B3); wr16(DP(0x00), 0x19B3);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xE9D4); VramQueueTiles_m1x1();
    rep(0x20);
    lda16(0x19A7); wr16(DP(0x00), 0x19A7);
    lda16(0xE9E7); wr16(DP(0x02), 0xE9E7);
    sep(0x20);
    push16(0xE9E5); VramQueueTiles20_m1x1();
    ret();
}

/* $81:8C3C CopyTrackColumn (RTL): the track streamer's column: from the
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

/* $80:CE5D PlaceAllRiderShadows (RTS): PlaceRiderShadow for every rider
 * slot 7..0 in use ($0950), OAM offset from $80:BAC7; a rider on screen in
 * columns $70-$8F sets $092A. */
void PlaceAllRiderShadows_m1x1(void)
{
    ldy8(0x07);
    do {
        wr8(DP(0x05), (u8)Y);
        lda8(rd8(ABSY(0x0950)));
        if (!fz) {
            X = (u8)A; nz8((u8)X);
            lda8(rd8(ABSY(0xBAC7)));
            wr8(DP(0x07), (u8)A);
            lda8(rd8(ABSX(0x019E)));
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(rd8(ABSX(0x0196)));
                cmp8((u8)A, 0x70);
                if (fc) {
                    cmp8((u8)A, 0x90);
                    if (!fc) { lda8(0x01); wr8(ABS(0x092A), 0x01); }
                }
            }
            push16(0xCE85); PlaceRiderShadow_m1x1();
        }
        Y = rd8(DP(0x05)); nz8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fn);
    ret();
}

/* $81:98B6 SkyFadeStep (RTS): fade the sky (FadeSkyColours); when the
 * object $03A3's timer $031F runs out, advance its state, set colours
 * $1C02-$1C16 to $6D69, flag the upload, $A5 + 1, $88 = $C0, $B9 = 6. */
void SkyFadeStep_m1x1(void)
{
    push8(0x81); push16(0x98B9); FadeSkyColours_m1x1();
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    rep(0x20);
    u16 t = dec16(rd16(ABSX(0x031F)));
    wr16(ABSX(0x031F), t);
    if (fz) {
        wr16(ABSX(0x030F), inc16(rd16(ABSX(0x030F))));
        rep(0x20);
        lda16(0x6D69);
        for (u16 a = 0x1C02; a <= 0x1C16; a += 2) wr16(ABS(a), 0x6D69);
        sep(0x20);
        wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
        wr8(DP(0xA5), inc8(rd8(DP(0xA5))));
        lda8(0xC0); wr8(DP(0x88), 0xC0);
        lda8(0x06); wr8(DP(0xB9), 0x06);
    }
    sep(0x20);
    ret();
}

/* $80:9C14 EntryConfirmStep (RTS): on B ($21 or $26 bit 7): with a press
 * on the second pad ($26) leave for mode $16 (fade, $0450 + 1, sound $45);
 * otherwise redraw the entry screen ($7E:5A00, $20 x 2 rows at VRAM $0300)
 * and go back a step ($0450 - 1). */
void EntryConfirmStep_m1x1(void)
{
    lda8(rd8(DP(0x26)));
    ora8(rd8(DP(0x21)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x26)));
    if (!fz) {
        wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
        wr8(ABS(0x0450), inc8(rd8(ABS(0x0450))));
        lda8(0x45);
        push8(0x80); push16(0x9C2B); QueueSoundB_m1x1();
        lda8(0x16); wr8(DP(0x76), 0x16);
        wr8(DP(0x75), 0);
        wr8(DP(0x77), 0);
        ret();
        return;
    }
    lda8(0x00); wr8(DP(0x02), 0x00);
    lda8(0x5A); wr8(DP(0x03), 0x5A);
    lda8(0x7E); wr8(DP(0x04), 0x7E);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x9C53); QueueTileRows_m1x1();
    wr8(ABS(0x0450), dec8(rd8(ABS(0x0450))));
    ret();
}

void StartCrash_m1x1(void);
void RiderHeightStep_m1x1(void);
void BrakeRider_m1x1(void);
void QueueTileBlock_m1x1(void);
void RiderPose_m1x1(void);
void DrawBigSprite_m1x1(void);

/* $80:9ABF EntryIntroStep (RTS): zoom $A2 in by 2 a frame; at $0140 sound
 * $2D, next step ($0450 + 1), queue the entry screen ($7E:5A00, $20 x 2
 * rows at VRAM $0300) and reset the number to 001 ($1570-$1572, $156F,
 * cursor $044F). */
void EntryIntroStep_m1x1(void)
{
    rep(0x20);
    wr16(ABS(0x00A2), inc16(rd16(ABS(0x00A2))));
    wr16(ABS(0x00A2), inc16(rd16(ABS(0x00A2))));
    lda16(rd16(ABS(0x00A2)));
    cmp16(A, 0x0140);
    if (fc) {
        sep(0x20);
        lda8(0x2D);
        push8(0x80); push16(0x9AD6); QueueSoundB_m1x1();
        wr8(ABS(0x0450), inc8(rd8(ABS(0x0450))));
        lda8(0x00); wr8(DP(0x02), 0x00);
        lda8(0x5A); wr8(DP(0x03), 0x5A);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x9AF8); QueueTileRows_m1x1();
        lda8(0x01); wr8(ABS(0x1572), 0x01);
        wr8(ABS(0x1571), 0);
        wr8(ABS(0x1570), 0);
        wr8(ABS(0x156F), 0);
        wr8(ABS(0x044F), 0);
    }
    sep(0x20);
    ret();
}

/* $80:FBAE OffTrackBrake (RTS, Y kept): rider X on rough ground (not
 * crashed, $020E 0 or bit 1 set) brakes by step 6 (5 with the turbo
 * button, $017E bit 6) and counts as on a ramp ($08BC = 1); on tile type 1
 * the player's turbo heat fraction $098F is held at $1200 at most. */
void OffTrackBrake_m1x1(void)
{
    lda8(rd8(ABSX(0x01EE)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x020E)));
    if (!fz) {
        lda8(lsr8((u8)A));
        lda8(lsr8((u8)A));
        if (!fc) { ret(); return; }
    }
    wr8(DP(0x01), (u8)Y);
    ldy8(0x04);
    lda8(rd8(ABSX(0x017E)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    if (fc) Y = dec8((u8)Y);
    lda8(0x01); wr8(ABSX(0x08BC), 0x01);
    Y = inc8((u8)Y); Y = inc8((u8)Y);
    push16(0xFBD1); BrakeRider_m1x1();
    Y = rd8(ABSX(0x0216)); nz8((u8)Y);
    lda8(rd8(ABSY(0xAD33)));
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

/* $80:F097 BonusSignStep (RTS): while the bonus sign is up ($97) draw it
 * eleven columns ahead of the track ($0279 + $0B, tilemap $0B20 or the other
 * screen): the sign block $80:F0F1 on the first frame (remembering the
 * address in $98), then the blank block $80:F0FC once the column moved on. */
void BonusSignStep_m1x1(void)
{
    lda8(rd8(DP(0x97)));
    if (fz) { sep(0x20); ret(); return; }
    lda8(0x28); wr8(DP(0x0F), 0x28);
    rep(0x20);
    lda16(rd16(ABS(0x0279)));
    fc = 0; adc16(0x000B);
    and16(0x0020);
    eor16(0x0020);
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    fc = 0; adc16(0x0B20);
    wr16(DP(0x00), A);
    lda16(rd16(ABS(0x0279)));
    fc = 0; adc16(0x000B);
    and16(0x001F);
    ora16(rd16(DP(0x00)));
    wr16(DP(0x00), A);
    sep(0x20);
    Y = rd8(DP(0x97)); nz8((u8)Y);
    Y = dec8((u8)Y);
    if (fz) {
        rep(0x20);
        wr16(DP(0x98), A);
        lda16(0xF0F1); wr16(DP(0x02), 0xF0F1);
        sep(0x20);
        wr8(DP(0x97), inc8(rd8(DP(0x97))));
    } else {
        rep(0x20);
        cmp16(A, rd16(DP(0x98)));
        if (fz) { sep(0x20); ret(); return; }
        wr16(DP(0x98), 0);
        lda16(0xF0FC); wr16(DP(0x02), 0xF0FC);
        sep(0x20);
    }
    push16(0xF0ED); QueueTileBlock_m1x1();
    sep(0x20);
    ret();
}

/* $80:9A43 EntryMenuStep (RTS): after the delay $88 queue the menu screen
 * ($7E:5900, $20 x 2 rows at VRAM $0300); on B from pad 2 ($26 bit 7, sound
 * $43, delay $20) go to the chosen step $0451 (choice 5: step $0A), and
 * for the others first queue its screen ($7E:5980). */
void EntryMenuStep_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) {
        u8 t = dec8(rd8(DP(0x88)));
        wr8(DP(0x88), t);
        if (!fz) { ret(); return; }
        lda8(0x00); wr8(DP(0x02), 0x00);
        lda8(0x59); wr8(DP(0x03), 0x59);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x9A69); QueueTileRows_m1x1();
    }
    lda8(rd8(DP(0x26)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x0451)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x20); wr8(DP(0x88), 0x20);
        lda8(0x43);
        push8(0x80); push16(0x9A80); QueueSoundB_m1x1();
        lda8(0x0A); wr8(ABS(0x0450), 0x0A);
        ret();
        return;
    }
    lda8(0x43);
    push8(0x80); push16(0x9A94); QueueSoundB_m1x1();
    lda8(rd8(ABS(0x0451)));
    wr8(ABS(0x0450), (u8)A);
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(0x80); wr8(DP(0x02), 0x80);
    lda8(0x59); wr8(DP(0x03), 0x59);
    lda8(0x7E); wr8(DP(0x04), 0x7E);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x9ABD); QueueTileRows_m1x1();
    ret();
}

/* $80:CAE5 DrawRider (RTS): rider X's 2x2 sprite at column $0196, height
 * $01B6 - 8 (the player outside races and screen $0C rides at $28 when low
 * or flagged $0904; a flagged rival off the bottom is not drawn), with the
 * pose from RiderPose: tiles $80:BA99, the slot's animation frame $1961
 * from $80:CB58 (frame $14 for a landed player at the end of the course). */
void DrawRider_m1x1(void)
{
    lda8(rd8(ABSX(0x0196)));
    wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(0x01B6)));
    fc = 1; sbc8(0x08);
    wr8(DP(0x01), (u8)A);
    int special = 0;
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(DP(0x76)));
        cmp8((u8)A, 0x03);
        if (!fz) {
            cmp8((u8)A, 0x0C);
            if (!fz) special = 1;
        }
    }
    if (special) {
        lda8(rd8(ABS(0x0904)));
        int low = 1;
        if (fz) {
            lda8(rd8(ABS(0x01B6)));
            cmp8((u8)A, 0x30);
            low = !fc;
        }
        if (low) { lda8(0x28); wr8(DP(0x01), 0x28); }
    } else {
        lda8(rd8(ABSX(0x0904)));
        if (!fz) {
            lda8(rd8(DP(0x01)));
            cmp8((u8)A, 0xEC);
            if (!fc) { ret(); return; }
        }
    }
    push16(0xCB1F); RiderPose_m1x1();
    wr8(DP(0xA8), (u8)A);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    rep(0x20);
    lda16(rd16(ABSY(0xBA99)));
    wr16(DP(0x02), A);
    sep(0x20);
    Y = rd8(DP(0xA8)); nz8((u8)Y);
    push8((u8)X);
    X = rd8(DP(0x0D)); nz8((u8)X);
    lda8(rd8(ABSX(0x0950)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSY(0xCB58)));
    wr8(ABSX(0x1961), (u8)A);
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(rd8(ABS(0x01CE)));
        ora8(rd8(ABS(0x01EE)));
        if (fz) {
            lda8(rd8(DP(0x84)));
            cmp8((u8)A, 0x01);
            if (fz) { lda8(0x14); wr8(ABSX(0x1961), 0x14); }
        }
    }
    X = pull8(); nz8((u8)X);
    push16(0xCB56); DrawBigSprite_m1x1();
    ret();
}

/* $80:C0E3 CrashLand (RTS): rider X's speed high byte becomes its crash
 * speed ($08EC) and the crash starts (StartCrash); then the lean step
 * $08FC from $80:BA6F by the d-pad (or $26 when the jump contest's player
 * is fast enough in the air), the height step, and the landed flag $08AC
 * once settled ($0226 = 0). */
/* $80:C0F9: the lean step $08FC from $80:BA6F by the d-pad (or $26 when the
 * jump contest's player is fast enough in the air), the height step, and
 * the landed flag $08AC once settled ($0226 = 0). */
static void crash_tail(void)
{
    lda8(rd8(ABSX(0x017E)));
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(DP(0x96)));
    int fast = 0;
    if (!fz) {
        lda8(rd8(ABS(0x08EC)));
        cmp8((u8)A, rd8(ABS(0x08DC)));
        if (fc) {
            lda8(rd8(DP(0xC7)));
            cmp8((u8)A, 0x01);
            fast = fz;
        }
    }
    if (fast) lda8(0x26);
    else lda8(rd8(ABSY(0xBA6F)));
    wr8(ABSX(0x08FC), (u8)A);
    lda8(rd8(ABSX(0x01B6)));
    wr8(DP(0x01), (u8)A);
    push16(0xC122); RiderHeightStep_m1x1();
    wr8(ABSX(0x01B6), (u8)A);
    lda8(rd8(ABSX(0x0226)));
    if (fz) { lda8(0x01); wr8(ABSX(0x08AC), 0x01); }
}

/* $80:C0E6: A is the crash speed ($08EC); the crash starts, then the tail. */
static void crash_from(void)
{
    wr8(ABSX(0x08EC), (u8)A);
    push16(0xC0EB); StartCrash_m1x1();
    crash_tail();
}

void CrashLand_m1x1(void)
{
    lda8(rd8(ABSX(0x01C6)));
    crash_from();
    ret();
}

/* $80:C0BD: crash at speed 0. */
static void crash_stop(void)
{
    lda8(0x00);
    crash_from();
}

/* $80:C0B4 RiderLandCheck (RTS): rider X in a crash state ($01EE): state 2
 * (tumbling) continues the crash steps unless the player presses right
 * ($017E bit 0 with $7F clear); any other state stops it at speed 0. */
void RiderLandCheck_m1x1(void)
{
    lda8(rd8(ABSX(0x01EE)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x02);
    if (!fz) { crash_stop(); ret(); return; }
    lda8(rd8(ABSX(0x017E)));
    lda8(lsr8((u8)A));
    if (fc) {
        lda8(rd8(DP(0x7F)));
        if (fz) { ret(); return; }
    }
    crash_tail();
    ret();
}

/* $80:FB60 BoostOnTile (RTS): unless crashed, the player's speed grows by
 * $0180 ($01FF in the jump contest) up to $0730; then the crash-stop steps
 * for rider X. */
void BoostOnTile_m1x1(void)
{
    lda8(rd8(ABSX(0x01EE)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x76)));
    cmp8((u8)A, 0x14);
    lda8(fz ? 0xFF : 0x80);
    fc = 0; adc8(rd8(ABS(0x01BE)));
    wr8(ABS(0x01BE), (u8)A);
    lda8(rd8(ABS(0x01C6)));
    adc8(0x01);
    wr8(ABS(0x01C6), (u8)A);
    cmp8((u8)A, 0x07);
    int cap = 0;
    if (fc) {
        if (!fz) cap = 1;
        else {
            lda8(rd8(ABS(0x01BE)));
            cmp8((u8)A, 0x30);
            cap = fc;
        }
    }
    if (cap) {
        lda8(0x07); wr8(ABS(0x01C6), 0x07);
        lda8(0x30); wr8(ABS(0x01BE), 0x30);
    }
    crash_stop();
    ret();
}

/* $80:FC41 LandFlat (RTS): clear the landing flags $09C4/$09C7, then
 * CrashLand. */
void LandFlat_m1x1(void)
{
    lda8(0x00);
    wr8(ABS(0x09C4), 0x00);
    wr8(ABS(0x09C7), 0x00);
    CrashLand_m1x1();
}

/* $80:FC03 SlowTileCheck (RTS): unless crashed or stopped, mark rider X
 * slow ($08F4 = 1) below speed $0200 and its threshold $0246; then by the
 * tile: $0206 = 2 lands flat (LandFlat), the player on boost ground ($0176
 * = $18) BoostOnTile, otherwise the crash-stop steps. */
void SlowTileCheck_m1x1(void)
{
    lda8(rd8(ABSX(0x01EE)));
    if (!fz) { ret(); return; }
    ldy8(0x00);
    lda8(rd8(ABSX(0x01C6)));
    if (fz) { ret(); return; }
    cmp8((u8)A, 0x02);
    if (!fc) {
        lda8(rd8(ABSX(0x01BE)));
        cmp8((u8)A, rd8(ABSX(0x0246)));
        if (!fc) Y = inc8((u8)Y);
    }
    lda8((u8)Y);
    wr8(ABSX(0x08F4), (u8)A);
    lda8(rd8(ABSX(0x0206)));
    cmp8((u8)A, 0x02);
    if (fz) { LandFlat_m1x1(); return; }
    lda8(rd8(ABS(0x0176)));
    cmp8((u8)A, 0x18);
    if (fz) { BoostOnTile_m1x1(); return; }
    crash_stop();
    ret();
}

void QueuePlaceBadge_m1x1(void);
void DrawRider_m1x1(void);
void MirrorJumpRider_m1x1(void);
void TickRaceClock_m1x1(void);

/* $80:B0C8: OR Y into rider X's input $017E. */
static void input_or_y(void)
{
    lda8((u8)Y);
    ora8(rd8(ABSX(0x017E)));
    wr8(ABSX(0x017E), (u8)A);
}

/* $80:B0AF: AND A into rider X's input $017E. */
static void input_and_a(void)
{
    and8(rd8(ABSX(0x017E)));
    wr8(ABSX(0x017E), (u8)A);
}

/* $80:B0A4 CpuReleaseButtons (RTS): a moving CPU rider lets go of the
 * buttons (input $017E & $0F). */
void CpuReleaseButtons_m1x1(void)
{
    lda8(rd8(ABSX(0x01C6)));
    if (fz) { ret(); return; }
    lda8(0x0F);
    input_and_a();
    ret();
}

/* $80:B090 CpuThrottle (RTS): a CPU rider below its target speed
 * $0236/$022E holds the throttle (input | $80), otherwise lets go of the
 * buttons (CpuReleaseButtons). */
void CpuThrottle_m1x1(void)
{
    ldy8(0x80);
    lda8(rd8(ABSX(0x01C6)));
    cmp8((u8)A, rd8(ABSX(0x0236)));
    if (!fc) { input_or_y(); ret(); return; }
    if (fz) {
        lda8(rd8(ABSX(0x01BE)));
        cmp8((u8)A, rd8(ABSX(0x022E)));
        if (!fc) { input_or_y(); ret(); return; }
    }
    CpuReleaseButtons_m1x1();
}

/* $80:B0AD CpuReleaseDpad (RTS): input $017E & $F0. */
void CpuReleaseDpad_m1x1(void)
{
    lda8(0xF0);
    input_and_a();
    ret();
}

/* $80:B0B6 CpuSteer (RTS): a CPU rider steers toward its wanted lean
 * $0971 (when set): d-pad bit 0 (down) when the lean $01E6 is above it,
 * bit 1 (up) when below. */
void CpuSteer_m1x1(void)
{
    push16(0xB0B8); CpuReleaseDpad_m1x1();
    ldy8(0x01);
    lda8(rd8(ABSX(0x0971)));
    if (fz) { ret(); return; }
    cmp8((u8)A, rd8(ABSX(0x01E6)));
    if (fz) { ret(); return; }
    if (fc) Y = inc8((u8)Y);
    input_or_y();
    ret();
}

/* $80:CF8B QueuePlaceBadgeHud (RTS): with a place $086E, the place badge
 * at VRAM $185A, attribute $20 (QueuePlaceBadge). */
void QueuePlaceBadgeHud_m1x1(void)
{
    lda8(rd8(ABS(0x086E)));
    if (fz) { ret(); return; }
    lda8(0x5A); wr8(DP(0x00), 0x5A);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    QueuePlaceBadge_m1x1();
}

/* $80:CA98 DrawAllRiders (RTS, X = the last slot): DrawRider for every
 * slot X..0 whose rider ($0950) is active ($01DE), OAM offset $80:BAC7,
 * palette slot $80:BAD7 (the player blinking palette 5 during the coin
 * turbo in races); the player's jump pose $16 adds the mirror image. */
void DrawAllRiders_m1x1(void)
{
    for (;;) {
        wr8(DP(0x0D), (u8)X);
        lda8(rd8(ABSX(0xBAC7)));
        wr8(DP(0x04), (u8)A);
        lda8(rd8(ABS(0x0CE0)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        ora8(rd8(ABSX(0x0950)));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSX(0x0950)));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x01DE)));
        if (!fz) {
            lda8(rd8(ABSX(0xBAD7)));
            wr8(DP(0x0B), (u8)A);
            lda8(rd8(ABS(0x0CE0)));
            if (!fz) {
                lda8(rd8(DP(0x76)));
                cmp8((u8)A, 0x03);
                if (fz) {
                    cmp8((u8)X, 0x00);
                    if (fz) {
                        lda8(rd8(DP(0x63)));
                        and8(0x02);
                        if (fz) { lda8(0x05); wr8(DP(0x0B), 0x05); }
                    }
                }
            }
            push16(0xCAD1); DrawRider_m1x1();
        }
        cmp8((u8)X, 0x00);
        if (fz) {
            lda8(rd8(DP(0xA8)));
            cmp8((u8)A, 0x16);
            if (fz) { push16(0xCADE); MirrorJumpRider_m1x1(); }
        }
        X = rd8(DP(0x0D)); nz8((u8)X);
        X = dec8((u8)X);
        if (fn) break;
    }
    ret();
}

/* $80:A457 ShowCourseRecord (RTS): with a record course $084C, show its
 * time ($083A-$083C into the race clock) at the VRAM address from
 * $80:97A5 (TickRaceClock), blinking: on even frames the time's queued
 * tiles are blanked ($24). Then $88 = 1. */
void ShowCourseRecord_m1x1(void)
{
    lda8(rd8(ABS(0x084C)));
    if (!fz) {
        lda8(rd8(ABS(0x083A))); wr8(ABS(0x03AE), (u8)A);
        lda8(rd8(ABS(0x083B))); wr8(ABS(0x03AF), (u8)A);
        lda8(rd8(ABS(0x083C))); wr8(ABS(0x03B0), (u8)A);
        lda8(rd8(ABS(0x084C)));
        lda8(dec8((u8)A));
        lda8(asl8((u8)A));
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x97A5))); wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(0x97A6))); wr8(DP(0x01), (u8)A);
        push16(0xA480); TickRaceClock_m1x1();
        lda8(rd8(DP(0x63)));
        and8(0x01);
        if (fz) {
            lda8(rd8(LONG(0x7F0000)));
            fc = 1; sbc8(0x27);
            X = (u8)A; nz8((u8)X);
            lda8(0x24);
            static const u8 offs[] = { 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C,
                                       0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24, 0x26 };
            for (unsigned i = 0; i < sizeof offs; i++)
                wr8(LONG(0x7F0000 + offs[i] + (u8)X), 0x24);
        }
    }
    lda8(0x01);
    wr8(ABS(0x0088), 0x01);
    ret();
}

/* $80:C2BB body: the eight start-line flag poles ($0936 - scroll $9A, Y
 * from $80:BB07, tile Y = $E1-$E3, attribute $3F; retired off the left edge
 * via $025E), then the countdown beeps: $3C at $B4, $78 or $3C plays sound
 * $26 and counts ($C3); with $54 set, $3C = $B5. */
static void start_flags(void)
{
    wr8(DP(0x00), (u8)Y);
    ldy8(0x07);
    do {
        lda8(rd8(ABSY(0x025E)));
        if (fz) {
            wr8(DP(0x08), 0);
            lda8(rd8(ABSY(0x0936)));
            fc = 1;
            sbc8(rd8(ABS(0x009A)));
            wr8(DP(0x01), (u8)A);
            cmp8((u8)A, 0xF0);
            if (fc) {
                cmp8((u8)A, 0xF9);
                if (fc) wr8(DP(0x08), inc8(rd8(DP(0x08))));
                lda8(0x01);
                wr8(ABSY(0x025E), 0x01);
            } else {
                lda8((u8)Y);
                lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
                X = (u8)A; nz8((u8)X);
                lda8(rd8(ABSY(0xBB07)));
                wr8(ABSX(0x17A1), (u8)A);
                fc = 0; adc8(0x08);
                wr8(ABSX(0x17A5), (u8)A);
                lda8(rd8(DP(0x00)));
                wr8(ABSX(0x17A2), (u8)A);
                ora8(0x10);
                wr8(ABSX(0x17A6), (u8)A);
                lda8(0x3F);
                wr8(ABSX(0x17A3), 0x3F); wr8(ABSX(0x17A7), 0x3F);
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
        push8(0x80); push16(0xC32E); QueueSoundB_m1x1();
        wr8(DP(0xC3), inc8(rd8(DP(0xC3))));
    }
}

/* $80:C2BB DrawStartFlags (RTS, Y = the flag tile). */
void DrawStartFlags_m1x1(void)
{
    start_flags();
    ret();
}

/* $80:C290 RaceStartLights (RTS): the start of a race. When the lights
 * begin ($3C = 1): unless already running ($92) count ($C3), set the
 * countdown $8A = $40 and play sound $27 (with the coin turbo also $2D and
 * 1); then $92 = 1, $3C = 2. Before the run ($92 clear) ping the APU every
 * 16 frames of $3C. Then the flags with tile $E1/$E2/$E3 by $3C
 * (DrawStartFlags). */
void RaceStartLights_m1x1(void)
{
    lda8(rd8(DP(0x3C)));
    cmp8((u8)A, 0x01);
    if (fz) {
        lda8(rd8(DP(0x92)));
        if (fz) {
            wr8(DP(0xC3), inc8(rd8(DP(0xC3))));
            lda8(0x40); wr8(DP(0x8A), 0x40);
            lda8(0x27);
            push8(0x80); push16(0xC347); QueueSoundB_m1x1();
            lda8(rd8(ABS(0x0CE0)));
            if (!fz) {
                lda8(0x2D);
                push8(0x80); push16(0xC352); QueueSoundB_m1x1();
                lda8(0x01);
                push8(0x80); push16(0xC358); QueueSoundB_m1x1();
            }
        }
        lda8(0x01); wr8(DP(0x92), 0x01);
        lda8(0x02); wr8(DP(0x3C), 0x02);
    } else {
        Y = rd8(DP(0x92)); nz8((u8)Y);
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
    start_flags();
    ret();
}

void RiderShadowY_m1x1(void);

/* $80:C1F3 CrashRecovery (RTS): every 4 frames (phase $0998, counting
 * down) each crashed rider ($01CE) picks a lane: the player in a race lane
 * 4 (sound $25 when not the CPU's turn $7F, and clears $092D), others a
 * random one ($2F & 3) into $0948. A stopped rider (speed high 0) that is
 * not tumbling ($01EE, $0176; the player not on tile $14) gets back up:
 * speed 0, upright lean 6, run-out $090C = column + 8, its shadow, wave
 * pose 2, $01D6 = 1 (the player with sound $3E); a tumbling one slides at
 * $88. Otherwise the lean keeps turning ($01E6 + $01CE) unless held ($3D). */
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
    ldx8(0x07);
    for (;;) {
        lda8(rd8(ABSX(0x01CE)));
        if (!fz) {
            ldy8(0x04);
            lda8((u8)X);
            ora8(rd8(ABS(0x0CD7)));
            if (fz) {
                wr8(ABS(0x092D), (u8)A);
                lda8(rd8(DP(0x7F)));
                if (fz) {
                    lda8(0x25);
                    push8(0x80); push16(0xC222); QueueSoundB_m1x1();
                }
                lda8(rd8(DP(0x2F)));
                and8(0x03);
                Y = (u8)A; nz8((u8)Y);
            }
            lda8((u8)Y);
            wr8(ABSX(0x0948), (u8)A);
            lda8(rd8(ABSX(0x01C6)));
            if (!fz) goto turn;
            lda8(rd8(ABSX(0x01EE)));
            ora8(rd8(ABSX(0x0176)));
            if (!fz) goto slide;
            cmp8((u8)X, 0x00);
            if (fz) {
                lda8(rd8(ABSX(0x021E)));
                cmp8((u8)A, 0x14);
                if (fz) goto slide;
                lda8((u8)X);
            }
            wr8(ABSX(0x01BE), (u8)A);
            wr8(ABSX(0x0186), (u8)A);
            wr8(ABSX(0x01CE), (u8)A);
            lda8(0x06); wr8(ABSX(0x01E6), 0x06);
            lda8(rd8(ABSX(0x0196)));
            fc = 0; adc8(0x08);
            wr8(ABSX(0x090C), (u8)A);
            push16(0xC25E); RiderShadowY_m1x1();
            lda8(0x02); wr8(ABSX(0x091C), 0x02);
            lda8(lsr8((u8)A));
            wr8(ABSX(0x01D6), (u8)A);
            cmp8((u8)X, 0x00);
            if (fz) {
                lda8(0x3E);
                push8(0x80); push16(0xC271); QueueSoundB_m1x1();
            }
            goto next;
        slide:
            lda8(0x88); wr8(ABSX(0x01BE), 0x88);
        turn:
            lda8(rd8(DPX(0x3D)));
            if (fz) {
                lda8(rd8(ABSX(0x01E6)));
                fc = 0; adc8(rd8(ABSX(0x01CE)));
                and8(0x0F);
                wr8(ABSX(0x01E6), (u8)A);
            }
        }
    next:
        X = dec8((u8)X);
        if (fn) break;
    }
    ret();
}

/* $81:A065 AimRatio (RTL, A = steps): for object X, the direction to the
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
    lda16(rd16(ABS(0x03A1)));
    fc = 1; sbc16(rd16(ABSX(0x028F)));
    wr16(DP(0x12), A);
    if (fn) Y = inc8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    lda16(rd16(DP(0x12)));
    if (fn) { eor16(0xFFFF); lda16(inc16(A)); }
    wr16(DP(0x10), A);
    ldy8(0x00);
    lda16(rd16(ABS(0x039F)));
    fc = 1; sbc16(rd16(ABSX(0x027F)));
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
