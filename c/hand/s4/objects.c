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
