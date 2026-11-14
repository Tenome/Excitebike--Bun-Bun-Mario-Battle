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
static void bonus_result_body(void);

void BonusResultStep_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    bonus_result_body();
}

/* $80:DE65: the result itself (also entered from $80:DDEF). */
static void bonus_result_body(void)
{
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
        WAIT_TICK();
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
        WAIT_TICK();
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

static void crash_loop(int enter_turn);

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
    crash_loop(0);
}

/* The rider loop of CrashRecovery (riders 7-0). Entered at $80:C279 (the
 * lean turn, enter_turn) for the current X by the player's crash steps. */
static void crash_loop(int enter_turn)
{
    if (enter_turn) goto turn;
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

void DivideAX_m1x1(void);
void ApplyVelocity_m1x1(void);

/* $80:C391 SpeedGauge (RTS): returns X = the speedometer needle: 0 unless
 * the gauge is on ($092D) and it is not the CPU's frame ($7F bit 0); else
 * the player's speed $01C6:$01BE / 8, minus 2, divided by 2 (4 when
 * crashed), at least 1, capped at $2F (wobbling to $2E every 8 frames);
 * bit 7 set when the turbo button is up. */
void SpeedGauge_m1x1(void)
{
    ldx8(0x00);
    lda8(rd8(ABS(0x092D)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(0x7F)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    ldx8(0x02);
    lda8(rd8(ABS(0x01C6))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABS(0x01BE))); wr8(DP(0x01), (u8)A);
    do {
        wr8(DP(0x00), lsr8(rd8(DP(0x00))));
        wr8(DP(0x01), ror8(rd8(DP(0x01))));
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x02);
    Y = rd8(ABS(0x01EE)); nz8((u8)Y);
    if (!fz) lda8(0x04);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(DP(0x01)));
    fc = 1; sbc8(0x02);
    int one = fn;
    if (!one) {
        push16(0xC3C3); DivideAX_m1x1();
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
    lda8(rd8(ABS(0x017E)));
    lda8(asl8((u8)A));
    if (!fc) {
        lda8((u8)X);
        ora8(0x80);
        X = (u8)A; nz8((u8)X);
    }
    ret();
}

/* $81:B8F4 FallingObjectStep (RTS): object $03A3 flaps every 4 frames
 * ($034F toggles 0/1) and moves (ApplyVelocity), falling ($02AF + 1); when
 * it reaches the ground ($02AF wraps to 0) it lands: next state, pose 2,
 * timer cleared, $036F = 4, sound $49. */
void FallingObjectStep_m1x1(void)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (fz) {
        lda8(rd8(ABSX(0x034F)));
        lda8(inc8((u8)A));
        and8(0x01);
        wr8(ABSX(0x034F), (u8)A);
    }
    push8(0x81); push16(0xB909); ApplyVelocity_m1x1();
    wr8(ABSX(0x02AF), inc8(rd8(ABSX(0x02AF))));
    lda8(rd8(ABSX(0x02AF)));
    if (!fz) { ret(); return; }
    wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
    lda8(0x02); wr8(ABSX(0x034F), 0x02);
    wr8(ABSX(0x031F), 0);
    lda8(0x04); wr8(ABSX(0x036F), 0x04);
    lda8(0x49);
    push8(0x81); push16(0xB927); QueueSoundB_m1x1();
    ret();
}

/* $80:BB56 RiderRunOut (RTS): rider X past the finish ($08C4) and not
 * stopped ($01FE) waves ($099E = 1) and on even frames rolls on ($090C +
 * 1) until it is $80:BB30 (by lane $0948) past its column; then, or when
 * not running out, its finish state advances ($01D6 + 1) and the wave
 * timer $45 = 8. */
void RiderRunOut_m1x1(void)
{
    lda8(rd8(ABSX(0x08C4)));
    int done = fz;
    if (!done) {
        lda8(rd8(ABSX(0x01FE)));
        done = !fz;
    }
    if (!done) {
        lda8(0x01); wr8(ABSX(0x099E), 0x01);
        lda8(rd8(DP(0x63)));
        lda8(lsr8((u8)A));
        if (fc) { ret(); return; }
        wr8(ABSX(0x090C), inc8(rd8(ABSX(0x090C))));
        Y = rd8(ABSX(0x0948)); nz8((u8)Y);
        lda8(rd8(ABSX(0x0196)));
        fc = 0; adc8(rd8(ABSY(0xBB30)));
        cmp8((u8)A, rd8(ABSX(0x090C)));
        if (fc) { ret(); return; }
    }
    u8 v = inc8(rd8(ABSX(0x01D6)));
    wr8(ABSX(0x01D6), v);
    if (!fz) { lda8(0x08); wr8(DPX(0x45), 0x08); }
    ret();
}

void Nop_BD39_m1x1(void);

/* One 8x8 hardware multiply: WRMPYA = a, WRMPYB = b, wait (Nop_BD39 at
 * return address `back`), then the product is at RDMPYL/H. */
static void mul8(u8 a, u8 b, u16 back)
{
    lda8(a); wr8(ABS(0x4202), (u8)A);
    lda8(b); wr8(ABS(0x4203), (u8)A);
    push16(back); Nop_BD39_m1x1();
}

/* The 16x16 -> 32 multiply both routines below do: [p] * [q] into r..r+3
 * (direct page), from four 8x8 products. */
static void mul16(u8 p, u8 q, u8 r, const u16 back[4])
{
    mul8(rd8(DP(p)), rd8(DP(q)), back[0]);
    lda8(rd8(ABS(0x4216))); wr8(DP(r), (u8)A);
    lda8(rd8(ABS(0x4217))); wr8(DP(r + 1), (u8)A);
    mul8(rd8(DP(p)), rd8(DP(q + 1)), back[1]);
    lda8(rd8(ABS(0x4216)));
    fc = 0; adc8(rd8(DP(r + 1)));
    wr8(DP(r + 1), (u8)A);
    lda8(rd8(ABS(0x4217)));
    adc8(0x00);
    wr8(DP(r + 2), (u8)A);
    mul8(rd8(DP(p + 1)), rd8(DP(q)), back[2]);
    lda8(rd8(ABS(0x4216)));
    fc = 0; adc8(rd8(DP(r + 1)));
    wr8(DP(r + 1), (u8)A);
    lda8(rd8(ABS(0x4217)));
    adc8(rd8(DP(r + 2)));
    wr8(DP(r + 2), (u8)A);
    mul8(rd8(DP(p + 1)), rd8(DP(q + 1)), back[3]);
    lda8(rd8(ABS(0x4216)));
    fc = 0; adc8(rd8(DP(r + 2)));
    wr8(DP(r + 2), (u8)A);
    lda8(rd8(ABS(0x4217)));
    adc8(0x00);
    wr8(DP(r + 3), (u8)A);
}

/* $80:B98E Mul16 (RTL): $00:$01 * $02:$03 -> $04-$07 with the hardware
 * multiplier. (The high bytes' carry into $07 is the product's own.) */
void Mul16_m1x1(void)
{
    static const u16 back[4] = { 0xB99A, 0xB9B1, 0xB9CD, 0xB9E9 };
    mul16(0x00, 0x02, 0x04, back);
    retl();
}

/* $80:B9FA Mul16b (RTS): $60:$61 * $62:$63 -> $64-$67. */
static void mul16b(void)
{
    static const u16 back[4] = { 0xBA06, 0xBA1D, 0xBA39, 0xBA55 };
    mul16(0x60, 0x62, 0x64, back);
    ret();
}
void Mul16b_m1x1(void) { mul16b(); }
void Mul16b_m1x0(void) { mul16b(); }

void ChangeMode_m1x1(void);

/* $80:865A TitleScrollStep (RTS): the scrolling title picture. With the
 * debug flag ($81:800E) Select jumps straight to the jump contest (screen
 * off, mode $14, final flag, course 1) and Start leaves. Every 5 frames
 * ($8A) the picture scrolls one line ($A6 + 1) and its next row is queued
 * ($7E:4800 + row, $20 x 2 at VRAM $2800 + line); when the timer $88 runs
 * out, or on Start, back to mode 0 (fade). */
void TitleScrollStep_m1x1(void)
{
    lda8(rd8(LONG(0x81800E)));
    if (!fz) {
        lda8(rd8(DP(0x21)));
        and8(0x20);
        if (!fz) {
            lda8(0x80);
            wr8(ABS(0x2100), 0x80);
            wr8(ABS(0x1903), 0x80);
            lda8(0x19); wr8(ABS(0x2109), 0x19);
            lda8(0x17); wr8(ABS(0x1905), 0x17);
            lda8(0x14); wr8(DP(0x76), 0x14);
            lda8(0x01); wr8(DP(0x7D), 0x01);
            lda8(0x01); wr8(DP(0x7C), 0x01);
            ChangeMode_m1x1();
            return;
        }
        lda8(rd8(DP(0x21)));
        and8(0x10);
        if (!fz) goto leave;
    }
    lda8(rd8(DP(0x8A)));
    if (fz) {
        lda8(0x05); wr8(DP(0x8A), 0x05);
        rep(0x20);
        wr16(DP(0xA6), inc16(rd16(DP(0xA6))));
        lda16(rd16(DP(0xA6)));
        and16(0xFFF8);
        lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
        fc = 0; adc16(0x4800);
        wr16(DP(0x02), A);
        sep(0x20);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        rep(0x20);
        lda16(0x0220); wr16(DP(0x10), 0x0220);
        lda16(rd16(ABS(0x00A6)));
        fc = 0; adc16(0x00E0);
        and16(0x01F8);
        lda16(asl16(A)); lda16(asl16(A));
        fc = 0; adc16(0x2800);
        wr16(DP(0x00), A);
        sep(0x20);
        push16(0x86CA); QueueTileRows_m1x1();
    }
    wr8(DP(0x8A), dec8(rd8(DP(0x8A))));
    rep(0x20);
    {
        u16 t = dec16(rd16(DP(0x88)));
        wr16(DP(0x88), t);
    }
    sep(0x20);
    if (!fz) { ret(); return; }
leave:
    lda8(0x00);
    wr8(DP(0x76), 0x00);
    wr8(DP(0x75), 0);
    wr8(DP(0x77), 0);
    wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    ret();
}

/* Queue `rows`x`cols` tiles from $80:src at VRAM `vram` (QueueTileRows). */
static void queue_rows80(u16 vram, u16 src, u16 size, u16 back)
{
    rep(0x20);
    lda16(vram); wr16(DP(0x00), vram);
    lda16(src); wr16(DP(0x02), src);
    lda16(size); wr16(DP(0x10), size);
    sep(0x20);
    lda8(0x80); wr8(DP(0x04), 0x80);
    push16(back); QueueTileRows_m1x1();
}

/* $80:86F8 StartEndScreen (RTS): the end of the broadcast: a $0384-frame
 * timer, BG3 at $1900, $1905 = $17, scrolling on ($90, $9A = $84 = 2),
 * sounds $2D and $45, and the end picture's panels queued (the "please
 * wait" panel too before minute 4; the Stadium 2/4 badge with $81:800C). */
void StartEndScreen_m1x1(void)
{
    rep(0x20);
    lda16(0x0384); wr16(DP(0x88), 0x0384);
    sep(0x20);
    lda8(0x19); wr8(ABS(0x2109), 0x19);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    lda8(0x01); wr8(DP(0x90), 0x01);
    ldx8(0x02);
    wr8(DP(0x9A), 0x02);
    wr8(DP(0x84), 0x02);
    lda8(0x2D);
    push8(0x80); push16(0x871F); QueueSoundB_m1x1();
    lda8(0x45);
    push8(0x80); push16(0x8725); QueueSoundB_m1x1();
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x04);
    if (!fc) queue_rows80(0x06C5, 0x87ED, 0x0218, 0x8747);
    queue_rows80(0x0726, 0x879D, 0x0214, 0x8761);
    queue_rows80(0x0573, 0x884D, 0x030B, 0x877B);
    lda8(rd8(LONG(0x81800C)));
    if (!fz) queue_rows80(0x057B, 0x888F, 0x0302, 0x879B);
    ret();
}

/* $81:A747 LoadHdmaScene (RTL, A = scene or $FF): stop HDMA and clear its
 * state ($190x, colour math $190D = $20); for a scene, copy its three
 * line tables ($81:A812/$A905/$A9F8 from $81:A727, lengths $81:A737) to
 * $0692/$0712/$0792 and its colour tables ($81:AAEB/$AB4B/$ABAB from
 * $81:A72F, lengths $81:A73F) to $0452/$0512/$05D2, point HDMA channels
 * 2-4 at the line tables (mode $40 -> $2132), enable them ($190F = $1C,
 * $0812 = 1), clear colour 0 and flag the palette upload. */
void LoadHdmaScene_m1x1(void)
{
    wr8(ABS(0x420C), 0);
    wr8(ABS(0x190F), 0);
    wr8(ABS(0x190A), 0);
    wr8(ABS(0x190B), 0);
    wr8(ABS(0x1907), 0);
    wr8(ABS(0x1908), 0);
    wr8(ABS(0x1909), 0);
    wr8(ABS(0x190C), 0);
    ldx8(0x20);
    wr8(ABS(0x190D), 0x20);
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
    lda16(rd16(ABSY(0xA737)));
    wr16(DP(0x02), A);
    X = rd16(ABSY(0xA727)); nz16(X);
    Y = 0; nz16(Y);
    sep(0x20);
    do {
        lda8(rd8(ABSX(0xA812))); wr8(ABSY(0x0692), (u8)A);
        lda8(rd8(ABSX(0xA905))); wr8(ABSY(0x0712), (u8)A);
        lda8(rd8(ABSX(0xA9F8))); wr8(ABSY(0x0792), (u8)A);
        X = inc16(X);
        Y = inc16(Y);
        cmp16(Y, rd16(DP(0x02)));
        WAIT_TICK();
    } while (!fz);
    rep(0x20);
    Y = pull16(); nz16(Y);
    lda16(rd16(ABSY(0xA73F)));
    wr16(DP(0x02), A);
    X = rd16(ABSY(0xA72F)); nz16(X);
    Y = 0; nz16(Y);
    sep(0x20);
    do {
        lda8(rd8(ABSX(0xAAEB))); wr8(ABSY(0x0452), (u8)A);
        lda8(rd8(ABSX(0xAB4B))); wr8(ABSY(0x0512), (u8)A);
        lda8(rd8(ABSX(0xABAB))); wr8(ABSY(0x05D2), (u8)A);
        X = inc16(X);
        Y = inc16(Y);
        cmp16(Y, rd16(DP(0x02)));
        WAIT_TICK();
    } while (!fz);
    X = 0x3240; nz16(X);
    wr16(ABS(0x4320), 0x3240); wr16(ABS(0x4330), 0x3240); wr16(ABS(0x4340), 0x3240);
    X = 0x0692; wr16(ABS(0x4322), 0x0692);
    X = 0x0712; wr16(ABS(0x4332), 0x0712);
    X = 0x0792; wr16(ABS(0x4342), 0x0792); nz16(X);
    lda8(0x00);
    wr8(ABS(0x4324), 0); wr8(ABS(0x4334), 0); wr8(ABS(0x4344), 0);
    wr8(ABS(0x4327), 0); wr8(ABS(0x4337), 0); wr8(ABS(0x4347), 0);
    lda8(0x01); wr8(ABS(0x0812), 0x01);
    lda8(0x1C); wr8(ABS(0x190F), 0x1C);
    sep(0x10);
    wr8(ABS(0x1C00), 0);
    wr8(ABS(0x1C01), 0);
    wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
    DB = pull8(); nz8(DB);
    retl();
}

void CheckBonusTime_m1x1(void);
void SetJumpScroll_m1x1(void);
void EndAtBonusMinute_m1x1(void);
void SendSpeedToApu_m1x1(void);
void DrawCountdownSignB_m1x1(void);
void SetBgTileBase_m1x1(void);
void DispatchBy85_m1x1(void);
void NextMode_m1x1(void);

/* $80:9004 ShowCoinRank (RTS): in races, on even frames, the coin count
 * box at VRAM $1848: below 6 coins ($1542) one of the six small boxes
 * ($80:90FE + count * $29, attribute $30); from 6 the big box ($80:90AC, or
 * $80:90D5 with the bonus count $199A) with the count ($1542 or $199D) as
 * three digits. */
void ShowCoinRank_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x76)));
    cmp8((u8)A, 0x03);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x1542)));
    cmp8((u8)A, 0x06);
    if (!fc) {
        wr8(ABS(0x4202), (u8)A);
        lda8(0x29);
        wr8(ABS(0x4203), 0x29);
        rep(0x20);
        lda16(rd16(ABS(0x4216)));
        fc = 0; adc16(0x90FE);
        wr16(DP(0x02), A);
        lda16(0x1848); wr16(DP(0x00), 0x1848);
        sep(0x20);
        lda8(0x30); wr8(DP(0x0F), 0x30);
        push16(0x90AA); QueueTileBlock_m1x1();
        ret();
        return;
    }
    ldy8(0x28);
    lda8(rd8(ABS(0x199A)));
    rep(0x20);
    lda16(fz ? 0x90AC : 0x90D5);
    wr16(DP(0x00), A);
    sep(0x20);
    do {
        lda8(rd8((((u32)DB << 16) + rd16(DP(0x00)) + Y) & 0xFFFFFF));
        wr8(ABSY(0x03CF), (u8)A);
        Y = dec8((u8)Y);
    } while (!fn);
    ldy8(0x00);
    lda8(rd8(ABS(0x199A)));
    if (!fz) lda8(rd8(ABS(0x199D)));
    else lda8(rd8(ABS(0x1542)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(0x03DC), (u8)A);
    ora8(0x10); wr8(ABS(0x03F0), (u8)A);
    lda8((u8)Y);
    ldy8(0x00);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
        WAIT_TICK();
    }
    wr8(ABS(0x03DA), (u8)A);
    ora8(0x10); wr8(ABS(0x03EE), (u8)A);
    lda8((u8)Y);
    wr8(ABS(0x03D8), (u8)A);
    ora8(0x10); wr8(ABS(0x03EC), (u8)A);
    rep(0x20);
    lda16(0x1848); wr16(DP(0x00), 0x1848);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    QueueTileBlock_m1x1();
}

/* $80:D81B BonusCourseFrame (RTS): one frame of the bonus course: the
 * time check, scroll, zoom ($0D14/$0D17 from $A2, $0D1A = $10), the end
 * minute, engine sound, countdown sign and BG tile base; the parallax
 * scrolls ($1B00/$1B02/$1B04 from $9C, $B6 once past $0500). During the
 * start countdown ($88) the beeps at $B4/$78/$3C (clearing the sign
 * sprites, sound $26) and at 0 the start ($8A = $40, sound $27). After it
 * the player's input, the 19 course objects (DispatchBy85) and the hidden
 * sprites; when the run has ended ($84 even, not 0) the next mode. */
void BonusCourseFrame_m1x1(void)
{
    push16(0xD81D); CheckBonusTime_m1x1();
    push8(0x80); push16(0xD821); SetJumpScroll_m1x1();
    lda8(rd8(DP(0xA2)));
    eor8(0xFF);
    lda8(inc8((u8)A));
    fc = 0; adc8(0x01);
    wr8(ABS(0x0D14), (u8)A);
    lda8(rd8(DP(0xA2)));
    fc = 0; adc8(0x80);
    wr8(ABS(0x0D17), (u8)A);
    lda8(0x10); wr8(ABS(0x0D1A), 0x10);
    push16(0xD83C); EndAtBonusMinute_m1x1();
    push16(0xD83F); SendSpeedToApu_m1x1();
    push16(0xD842); DrawCountdownSignB_m1x1();
    push8(0x80); push16(0xD846); SetBgTileBase_m1x1();
    rep(0x20);
    lda16(rd16(DP(0x9C)));
    lda16(lsr16(A));
    wr16(ABS(0x1B02), A);
    wr16(ABS(0x1B04), A);
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    wr16(ABS(0x1B00), A);
    lda16(rd16(ABS(0x00B6)));
    if (fz) {
        lda16(rd16(ABS(0x1B02)));
        cmp16(A, 0x0500);
        if (fc) wr16(ABS(0x00B6), inc16(rd16(ABS(0x00B6))));
    }
    lda16(rd16(DP(0x88)));
    if (!fz) {
        wr16(DP(0x88), dec16(rd16(DP(0x88))));
        lda16(rd16(DP(0x88)));
        cmp16(A, 0x00B4);
        int beep = fz;
        if (!beep) { cmp16(A, 0x0078); beep = fz; }
        if (!beep) { cmp16(A, 0x003C); beep = fz; }
        if (beep) {
            sep(0x20);
            ldx8(0x3C);
            do {
                lda8(0xF0);
                wr8(ABSX(0x1701), 0xF0);
                X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X);
            } while (!fn);
            lda8(0x26);
            push8(0x80); push16(0xD8AD); QueueSoundB_m1x1();
            wr8(DP(0xC3), inc8(rd8(DP(0xC3))));
            ret();
            return;
        }
        cmp16(A, 0x0000);
        if (fz) {
            lda16(0x0040); wr16(DP(0x8A), 0x0040);
            sep(0x20);
            lda8(0x27);
            push8(0x80); push16(0xD893); QueueSoundB_m1x1();
            wr8(DP(0xC3), inc8(rd8(DP(0xC3))));
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
            lda8(0x00); wr8(DP(0x85), 0x00);
            NextMode_m1x1();
            return;
        }
    }
    lda8(rd8(DP(0x20)));
    wr8(ABS(0x017E), (u8)A);
    lda8(0x00); wr8(DP(0x85), 0x00);
    do {
        push16(0xD8CC); DispatchBy85_m1x1();
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

void Hundreds_m0x1(void);

/* $81:CA20 body: $0C = $1569 * (Y - 1) by repeated addition (Y from
 * Hundreds), then the caller's flags back (PLP). */
static void scale_jump_tail(void)
{
    Y = dec8((u8)Y);
    rep(0x20);
    lda16(rd16(ABS(0x1569)));
    for (;;) {
        wr16(DP(0x0C), A);
        Y = dec8((u8)Y);
        if (fn) break;
        fc = 0;
        adc16(rd16(ABS(0x1569)));
        WAIT_TICK();
    }
    setP(pull8());
    retl();
}

/* $81:CA20 ScaleJumpCount (RTL, either A width): Hundreds, then the jump
 * count $1569 times the result less one into $0C. */
void ScaleJumpCount_m1x1(void)
{
    push8(getP());
    push16(0xCA23); Hundreds_m1x1();
    scale_jump_tail();
}
void ScaleJumpCount_m0x1(void)
{
    push8(getP());
    push16(0xCA23); Hundreds_m0x1();
    scale_jump_tail();
}

/* $81:F9C8 BsxFindFile (M=0 X=0, RTL): look up the broadcast directory
 * ($7E:9BEC, $7E:9BF2 entries) for the channel $13D3 and file $13D5: on a
 * match copy its 13-byte record to $13F3 and return C clear; otherwise C
 * set and $7E:13C9 = 3. Entries that are not the channel are skipped by
 * their size (count * 13 + 3 bytes). */
void BsxFindFile_m0x0(void)
{
    push8(getP());
    sep(0x20);
    rep(0x10);
    push8(DB);
    push16(0x7E7E);
    DB = pull8(); nz8(DB);
    DB = pull8(); nz8(DB);
    lda8(rd8(LONG(0x7E9BF2)));
    wr8(LONG(0x7E004B), (u8)A);
    X = 0x0008; nz16(X);
    for (;;) {
        rep(0x20);
        lda16(rd16(LONGX(0x7E9BEC)));
        cmp16(A, rd16(LONG(0x0013D3)));
        sep(0x20);
        if (fz) {
            X = inc16(X); X = inc16(X);
            lda8(rd8(LONGX(0x7E9BEC)));
            wr8(LONG(0x7E0047), (u8)A);
            X = inc16(X);
            for (;;) {
                rep(0x20);
                lda16(rd16(LONGX(0x7E9BED)));
                cmp16(A, rd16(LONG(0x0013D5)));
                sep(0x20);
                if (fz) {
                    Y = 0; nz16(Y);
                    do {
                        lda8(rd8(LONGX(0x7E9BEC)));
                        push16(X);                       /* PHX; TYX */
                        wr8(LONG(0x0013F3 + Y), (u8)A);
                        X = pull16();                    /* PLX */
                        X = inc16(X);
                        Y = inc16(Y);
                        cmp16(Y, 0x000D);
                    } while (!fz);
                    DB = pull8(); nz8(DB);
                    setP(pull8());
                    fc = 0;
                    retl();
                    return;
                }
                rep(0x20);
                lda16(X);
                fc = 0; adc16(0x000D);
                X = A; nz16(X);
                sep(0x20);
                u8 n = dec8(rd8(ABS(0x0047)));
                wr8(ABS(0x0047), n);
                WAIT_TICK();
                if (fz) break;
            }
        } else {
            rep(0x20);
            lda16(rd16(LONGX(0x7E9BEE)));
            and16(0x00FF);
            wr16(LONG(0x7E0047), A);
            lda16(asl16(A));
            adc16(rd16(LONG(0x7E0047)));
            lda16(asl16(A));
            lda16(asl16(A));
            adc16(rd16(LONG(0x7E0047)));
            wr16(ABS(0x0047), X);
            adc16(rd16(LONG(0x7E0047)));
            adc16(0x0003);
            X = A; nz16(X);
            sep(0x20);
        }
        u8 c = dec8(rd8(ABS(0x004B)));
        wr8(ABS(0x004B), c);
        WAIT_TICK();
        if (fz) break;
    }
    lda8(0x03);
    wr8(LONG(0x7E13C9), 0x03);
    DB = pull8(); nz8(DB);
    setP(pull8());
    fc = 1;
    retl();
}

void AddBcdDigits_m1x1(void);

/* $80:9471 ShowItemPoints (RTS): the items' points, $1548 * 10 + $1547 * 8
 * + $1546 * 6 + $1545 * 4 + $1544 * 2, as four digits plus a zero
 * ($03D0-$03D4, lower row +$10), the bonus digits added (AddBcdDigits),
 * leading zeros blank, size $25, queued at VRAM $1977. */
void ShowItemPoints_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(0x1548)));
    and16(0x00FF);
    lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(asl16(A)); lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    wr16(DP(0x02), A);
    lda16(rd16(ABS(0x1547)));
    and16(0x00FF);
    lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x02)));
    wr16(DP(0x02), A);
    lda16(rd16(ABS(0x1546)));
    and16(0x00FF);
    lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    fc = 0; adc16(rd16(DP(0x02)));
    wr16(DP(0x02), A);
    lda16(rd16(ABS(0x1545)));
    and16(0x00FF);
    lda16(asl16(A)); lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x02)));
    wr16(DP(0x02), A);
    lda16(rd16(ABS(0x1544)));
    and16(0x00FF);
    lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x02)));
    wr16(DP(0x02), A);
    static const u16 pow10[3] = { 1000, 100, 10 };
    for (int i = 0; i < 3; i++) {
        ldy8(0x00);
        for (;;) {
            cmp16(A, pow10[i]);
            if (!fc) break;
            sbc16(pow10[i]);
            Y = inc8((u8)Y);
            WAIT_TICK();
        }
        wr16(DP(i < 2 ? 0x02 : 0x04), A);
        lda16((u16)(Y & 0xFF));
        wr16(ABS(0x03D0 + i), A);
        ora16(0x0010);
        wr16(ABS(0x03D5 + i), A);
        if (i < 2) lda16(rd16(DP(0x02)));
    }
    lda16(0x1977); wr16(DP(0x00), 0x1977);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(rd8(DP(0x04)));
    wr8(ABS(0x03D3), (u8)A);
    ora8(0x10); wr8(ABS(0x03D8), (u8)A);
    lda8(0x00); wr8(ABS(0x03D4), 0x00);
    ora8(0x10); wr8(ABS(0x03D9), (u8)A);
    push16(0x952D); AddBcdDigits_m1x1();
    lda8(0x25); wr8(ABS(0x03CF), 0x25);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    ldx8(0x01);
    for (;;) {
        lda8(rd8(ABSX(0x03CF)));
        if (!fz) break;
        lda8(0xFF);
        wr8(ABSX(0x03CF), 0xFF);
        wr8(ABSX(0x03D4), 0xFF);
        X = inc8((u8)X);
        cmp8((u8)X, 0x05);
        if (fc) break;
    }
    VramQueueTiles_m1x1();
}

/* $80:FC5F UpdateLanes (RTS): every rider's lane position $01F6 moves by
 * its steering $01AE (not while landing from a jump, $0176 = $14/$15),
 * stops at a target lane $19A6 or one of the lane lines $80:F6AB, bounces
 * between rows 8 and $39 (the edge flips the steering), and the jump
 * states $020E settle (landing with CrashLand). Then the player's d-pad
 * (up $08 / down $04) steers, with sound $22 on a press or every 4 frames. */
void UpdateLanes_m1x1(void)
{
    ldx8(0x07);
    for (;;) {
        lda8(rd8(ABSX(0x0176)));
        cmp8((u8)A, 0x15);
        int move;
        if (fz) {
            lda8(rd8(ABSX(0x01D6)));
            move = !fz;
        } else {
            cmp8((u8)A, 0x14);
            move = !fz;
        }
        if (move) {
            lda8(rd8(ABSX(0x01F6)));
            fc = 0; adc8(rd8(ABSX(0x01AE)));
            wr8(ABSX(0x01F6), (u8)A);
        }
        lda8(rd8(ABSX(0x199E)));
        if (!fz) {
            lda8(rd8(ABSX(0x01F6)));
            cmp8((u8)A, rd8(ABSX(0x19A6)));
            if (!fz) goto next;
            wr8(ABSX(0x199E), 0);
            goto stop;
        }
        ldy8(0x07);
        do {
            lda8(rd8(ABSX(0x01F6)));
            cmp8((u8)A, rd8(ABSY(0xF6AB)));
            if (fz) goto stop;
            Y = dec8((u8)Y);
        } while (!fn);
        lda8(rd8(ABSX(0x020E)));
        if (!fz) {
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(rd8(ABSX(0x01F6)));
                cmp8((u8)A, 0x20);
                if (!fc) {
                    lda8(0x04); wr8(ABSX(0x020E), 0x04);
                    lda8(rd8(ABSX(0x0176)));
                    int land = fz;
                    if (!land) { cmp8((u8)A, 0x12); land = fz; }
                    if (!land) { cmp8((u8)A, 0x10); land = fz; }
                    if (!land) { cmp8((u8)A, 0x0D); land = fz; }
                    if (!land) goto next;
                    lda8(0x00); wr8(ABSX(0x01FE), 0x00);
                    push16(0xFCCD); CrashLand_m1x1();
                    {
                        u8 v = dec8(rd8(ABSX(0x020E)));
                        wr8(ABSX(0x020E), v);
                    }
                    if (!fz) goto next;
                    /* falls on with A as CrashLand left it */
                    cmp8((u8)A, 0x03);
                    if (fz) goto rows;
                    goto drift;
                }
                goto rows;
            }
            cmp8((u8)A, 0x03);
            if (fz) goto rows;
        drift:
            lda8(rd8(ABSX(0x01F6)));
            cmp8((u8)A, 0x20);
            if (fc) wr8(ABSX(0x01F6), dec8(rd8(ABSX(0x01F6))));
        }
    rows:
        lda8(rd8(ABSX(0x01F6)));
        cmp8((u8)A, 0x08);
        if (!fc) {
            lda8(rd8(ABSX(0x01D6)));
            if (fz) lda8(0x07);
            else {
                lda8(rd8(ABSX(0x01F6)));
                cmp8((u8)A, 0x02);
                if (!fc) lda8(0x01);
            }
            wr8(ABSX(0x01F6), (u8)A);
            cmp8((u8)X, 0x00);
            if (fz) goto flip;
            goto edge;
        }
        cmp8((u8)A, 0x3A);
        if (!fc) goto next;
        lda8(0x39); wr8(ABSX(0x01F6), 0x39);
    edge:
        lda8(rd8(ABSX(0x01D6)));
        ora8(rd8(ABSX(0x0940)));
        if (!fz) goto next;
    flip:
        lda8(rd8(ABSX(0x01AE)));
        if (fz) lda8(0xFF);
        eor8(0xFE);
        wr8(ABSX(0x01AE), (u8)A);
        goto next;
    stop:
        lda8(0x00);
        wr8(ABSX(0x01AE), 0x00);
    next:
        X = dec8((u8)X);
        if (fn) break;
    }
    lda8(rd8(DP(0x92)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x01CE)));
    ora8(rd8(ABS(0x0940)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x01EE)));
    if (!fz) {
        lda8(rd8(ABS(0x08F4)));
        cmp8((u8)A, 0x02);
        if (!fz) { ret(); return; }
        wr8(ABS(0x08F4), inc8(rd8(ABS(0x08F4))));
    }
    lda8(rd8(ABS(0x01D6)));
    if (!fz) {
        cmp8((u8)A, 0x05);
        if (!fz) { ret(); return; }
    }
    Y = rd8(ABS(0x01AE)); nz8((u8)Y);
    lda8(rd8(ABS(0x017E)));
    and8(0x04);
    if (!fz) lda8(0xFF);
    else {
        lda8(rd8(ABS(0x017E)));
        and8(0x08);
        if (fz) { ret(); return; }
        lda8(0x01);
    }
    wr8(ABS(0x01AE), (u8)A);
    lda8(rd8(DP(0x21)));
    and8(0x0C);
    if (fz) {
        lda8(rd8(DP(0x63)));
        and8(0x03);
        if (!fz) { ret(); return; }
    }
    lda8(0x22);
    push8(0x80); push16(0xFD7C); QueueSoundB_m1x1();
    ret();
}

void ReadStreamByte_m1x0(void);
void ReadStreamWord_m1x0(void);

/* $82:8C22: unpack the stream at [$08] to [$00] (X = 16-bit, Y counts the
 * output). Each command byte (until $FF): below $E0 the top three bits are
 * the command and the low five + 1 the length; from $E0 bits 2-4 are the
 * command and the length is ((b & 3) << 8 | next byte) + 1. Commands: 0
 * copy bytes, $20 fill with a byte, $40 fill with a word (alternating
 * bytes), $60 fill with an incrementing byte, $80+ copy from earlier
 * output (big-endian offset). */
/* The original's cycles for the last unpack (per command and per byte of
 * each loop), for the port's bridge, which charges them after the call. */
unsigned g_unpack_cycles;

static void unpack_stream(void)
{
    g_unpack_cycles = 40;
    rep(0x10);
    Y = 0; nz16(Y);
    for (;;) {
        lda8(rd8(ILDP(0x08)));
        cmp8((u8)A, 0xFF);
        if (fz) { sep(0x10); ret(); return; }
        X = rd16(DP(0x08)); X = inc16(X);
        if (fz) { X = 0x8000; nz16(X); wr8(DP(0x0A), inc8(rd8(DP(0x0A)))); }
        wr16(DP(0x08), X);
        wr8(DP(0x05), (u8)A);
        g_unpack_cycles += 75;
        cmp8((u8)A, 0xE0);
        if (!fc) {
            and8(0xE0);
            push8((u8)A);
            lda8(rd8(DP(0x05)));
            rep(0x21); fc = 0;
            and16(0x001F);
        } else {
            lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
            and8(0xE0);
            push8((u8)A);
            lda8(rd8(DP(0x05)));
            and8(0x03);
            A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
            push16(0x8C58); ReadStreamByte_m1x0();
            rep(0x21); fc = 0;
        }
        lda16(inc16(A));
        wr16(DP(0x03), A);
        lda16(Y);
        adc16(rd16(DP(0x03)));
        wr16(DP(0x05), A);
        sep(0x20);
        u8 cmd = pull8();
        lda8(cmd);
        if (fz) {                                      /* copy bytes */
            X = rd16(DP(0x08)); nz16(X);
            do {
                lda8(rd8(ILDP(0x08)));
                X = inc16(X);
                if (fz) { X = 0x8000; nz16(X); wr8(DP(0x0A), inc8(rd8(DP(0x0A)))); }
                wr16(DP(0x08), X);
                wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                Y = inc16(Y);
                cmp16(Y, rd16(DP(0x05)));
                g_unpack_cycles += 34;
                WAIT_TICK();
            } while (!fz);
        } else if (fn) {                               /* copy from the output */
            push16(0x8CB3); ReadStreamWord_m1x0();
            X = A; nz16(X);
            do {
                push16(Y);
                Y = X; nz16(Y);
                lda8(rd8((ILDP(0x00) + Y) & 0xFFFFFF));
                Y = pull16(); nz16(Y);
                wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                X = inc16(X);
                Y = inc16(Y);
                cmp16(Y, rd16(DP(0x05)));
                g_unpack_cycles += 37;
                WAIT_TICK();
            } while (!fz);
        } else {
            lda8(asl8((u8)A));
            if (!fn) {                                 /* $20: fill with a byte */
                push16(0x8C97); ReadStreamByte_m1x0();
                X = rd16(DP(0x03)); nz16(X);
                do {
                    wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                    Y = inc16(Y);
                    X = dec16(X);
                    g_unpack_cycles += 14;
                    WAIT_TICK();
                } while (!fz);
            } else {
                lda8(asl8((u8)A));
                if (!fn) {                             /* $40: fill with a word */
                    push16(0x8CA4); ReadStreamWord_m1x0();
                    X = rd16(DP(0x03)); nz16(X);
                    do {
                        A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
                        wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                        Y = inc16(Y);
                        X = dec16(X);
                        g_unpack_cycles += 17;
                        WAIT_TICK();
                    } while (!fz);
                } else {                               /* $60: incrementing fill */
                    push16(0x8C72); ReadStreamByte_m1x0();
                    X = rd16(DP(0x03)); nz16(X);
                    do {
                        wr8((ILDP(0x00) + Y) & 0xFFFFFF, (u8)A);
                        lda8(inc8((u8)A));
                        Y = inc16(Y);
                        X = dec16(X);
                        g_unpack_cycles += 16;
                        WAIT_TICK();
                    } while (!fz);
                }
            }
        }
    }
}

/* $82:8BFE UnpackToWram (RTS, A = entry): unpack stream A of the set at
 * $00 (pointer table $82:96A9, three bytes each) to $7E:4000. */
void UnpackToWram_m1x1(void)
{
    rep(0x30);
    lda16(asl16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    Y = A; nz16(Y);
    sep(0x20);
    lda8(rd8(ABSY(0x96A9))); wr8(DP(0x08), (u8)A);
    lda8(rd8(ABSY(0x96AA))); wr8(DP(0x09), (u8)A);
    lda8(rd8(ABSY(0x96AB))); wr8(DP(0x0A), (u8)A);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x40); wr8(DP(0x01), 0x40);
    lda8(0x7E); wr8(DP(0x02), 0x7E);
    unpack_stream();
}

/* $82:8BD8 UnpackToLowRam (RTS, A = entry): as UnpackToWram, to $00:1C00
 * (the palette buffer). */
void UnpackToLowRam_m1x1(void)
{
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x00)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0x96A9))); wr8(DP(0x08), (u8)A);
    lda8(rd8(ABSY(0x96AA))); wr8(DP(0x09), (u8)A);
    lda8(rd8(ABSY(0x96AB))); wr8(DP(0x0A), (u8)A);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x1C); wr8(DP(0x01), 0x1C);
    lda8(0x00); wr8(DP(0x02), 0x00);
    unpack_stream();
}

void AimRatio_m1x1(void);
void LoadHdmaScene_m1x1(void);
void DrawAllRiders_m1x1(void);
void ShowJumpCount_m1x1(void);
void ShowBonusPoints_m1x1(void);
void SelectMain_m1x1(void);
void ScaleJumpCount_m1x1(void);
void QueuePlaceBadgeHud_m1x1(void);

/* $81:A053 AimVelocity (RTL, A = steps): object X's velocity toward its
 * target from AimRatio ($02AF vertical, $029F horizontal). */
void AimVelocity_m1x1(void)
{
    push8(0x81); push16(0xA056); AimRatio_m1x1();
    lda8(rd8(DP(0x00))); wr8(ABSX(0x02AF), (u8)A);
    lda8(rd8(DP(0x0A))); wr8(ABSX(0x029F), (u8)A);
    retl();
}

/* $80:DD2B StartBonusTry (RTS): count the try ($1E95), load the bonus HDMA
 * scene (3 before minute $28, else 2) and set $3A = $20. */
void StartBonusTry_m1x1(void)
{
    wr8(ABS(0x1E95), inc8(rd8(ABS(0x1E95))));
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x28);
    lda8(fc ? 0x02 : 0x03);
    push8(0x80); push16(0xDD3F); LoadHdmaScene_m1x1();
    lda8(0x20); wr8(DP(0x3A), 0x20);
    ret();
}

/* The jump result screens draw the player (DrawAllRiders, slot 0) in the
 * waving pose $19 and count $8C down; at 0 sound $42, next step ($CD) and
 * $8C = $20. Returns 1 when the step changed. */
static int jump_result_wave(u16 back)
{
    ldx8(0x00);
    push16(back); DrawAllRiders_m1x1();
    lda8(0x19); wr8(ABS(0x1961), 0x19);
    u8 t = dec8(rd8(DP(0x8C)));
    wr8(DP(0x8C), t);
    if (!fz) return 0;
    lda8(0x42);
    push8(0x80); push16((u16)(back + 0x0F)); QueueSoundB_m1x1();
    wr8(DP(0xCD), inc8(rd8(DP(0xCD))));
    lda8(0x20); wr8(DP(0x8C), 0x20);
    return 1;
}

/* $80:E93C JumpResultCount (RTS): the wave, then the jump count
 * (ShowJumpCount, into which it falls). */
void JumpResultCount_m1x1(void)
{
    if (!jump_result_wave(0xE940)) { ret(); return; }
    ShowJumpCount_m1x1();
}

/* $80:E998 JumpResultBonus (RTS): the wave, then the bonus points
 * (ShowBonusPoints, into which it falls). */
void JumpResultBonus_m1x1(void)
{
    if (!jump_result_wave(0xE99C)) { ret(); return; }
    ShowBonusPoints_m1x1();
}

/* $80:EA3E JumpResultWait (RTS): the waving player; after $B4 frames
 * ($88) or a button on either pad, back to the jump contest (mode $14,
 * sprites frozen). */
void JumpResultWait_m1x1(void)
{
    ldx8(0x00);
    push16(0xEA42); DrawAllRiders_m1x1();
    lda8(0x19); wr8(ABS(0x1961), 0x19);
    rep(0x20);
    wr16(DP(0x88), inc16(rd16(DP(0x88))));
    lda16(rd16(DP(0x88)));
    cmp16(A, 0x00B4);
    if (!fz) {
        sep(0x20);
        lda8(rd8(DP(0x21)));
        ora8(rd8(DP(0x26)));
        if (fz) { ret(); return; }
    }
    sep(0x20);
    lda8(0x14); wr8(DP(0x76), 0x14);
    wr8(ABS(0x1958), inc8(rd8(ABS(0x1958))));
    ChangeMode_m1x1();
}

/* $80:8F26 StartPlayerSelect (RTS): reset the select state ($1E8B, the
 * five name letters from $80:8F64), step on, no fade ($1923 = 1), BG1 at
 * $0100, clear the held buttons, $198E = $50, the select screen itself,
 * a $0708-frame timer, an empty track and no HDMA scene. */
void StartPlayerSelect_m1x1(void)
{
    wr8(ABS(0x1E8B), 0);
    ldx8(0x04);
    do {
        lda8(rd8(ABSX(0x8F64)));
        wr8(ABSX(0x1554), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(0x75), inc8(rd8(DP(0x75))));
    lda8(0x01); wr8(ABS(0x1923), 0x01);
    lda8(0x01); wr8(ABS(0x2107), 0x01);
    wr8(DP(0x20), 0);
    wr8(DP(0x24), 0);
    lda8(0x50); wr8(ABS(0x198E), 0x50);
    push16(0x8F4B); SelectMain_m1x1();
    rep(0x20);
    lda16(0x0708); wr16(ABS(0x0088), 0x0708);
    lda16(0x0000); wr16(LONG(0x7F4000), 0x0000);
    sep(0x20);
    lda8(0x00);
    push8(0x80); push16(0x8F62); LoadHdmaScene_m1x1();
    ret();
}

/* $80:E8E5 JumpResultTally (RTS): the waving player; every 8 frames ($CF)
 * the pose frame $CE steps (frames $80:E92F); after the fourth, next
 * step ($CD), timers $88 = $80, $8A = $8C = $20, and the total
 * $1995 = ScaleJumpCount's $0C + the rank points $80:F155. */
void JumpResultTally_m1x1(void)
{
    ldx8(0x00);
    push16(0xE8E9); DrawAllRiders_m1x1();
    u8 t = dec8(rd8(DP(0xCF)));
    wr8(DP(0xCF), t);
    lda8(t);
    if (fz) {
        wr8(DP(0xCE), inc8(rd8(DP(0xCE))));
        lda8(rd8(DP(0xCE)));
        cmp8((u8)A, 0x04);
        if (fc) {
            wr8(DP(0xCE), dec8(rd8(DP(0xCE))));
            wr8(DP(0xCD), inc8(rd8(DP(0xCD))));
            rep(0x20);
            lda16(0x0080); wr16(DP(0x88), 0x0080);
            lda16(0x0020); wr16(DP(0x8A), 0x0020);
            lda16(0x0020); wr16(DP(0x8C), 0x0020);
            push8(0x80); push16(0xE910); ScaleJumpCount_m0x1();
            Y = rd8(ABS(0x156B)); nz8((u8)Y);
            lda16(rd16(ABSY(0xF155)));
            and16(0x00FF);
            fc = 0; adc16(rd16(DP(0x0C)));
            wr16(ABS(0x1995), A);
            sep(0x20);
        }
        lda8(0x08); wr8(DP(0xCF), 0x08);
    }
    X = rd8(DP(0xCE)); nz8((u8)X);
    lda8(rd8(ABSX(0xE92F)));
    wr8(ABS(0x1961), (u8)A);
    ret();
}

/* $80:91FC ResetRaceBoard (RTS): every rider's place $086E = $7D * 8
 * (high byte 0), the place badge, the six track rows $09D3-$0CD2 cleared
 * to $3B, sprite 0 parked, and the streamer's start ($0276 = $34, column
 * $14 on page 5, $0279 = $0A). */
void ResetRaceBoard_m1x1(void)
{
    lda8(rd8(DP(0x7D)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    ldx8(0x07);
    do {
        wr8(ABSX(0x086E), (u8)A);
        wr8(ABSX(0x086F), 0);
        X = dec8((u8)X);
    } while (!fn);
    push16(0x920E); QueuePlaceBadgeHud_m1x1();
    ldx8(0x7F);
    lda8(0x3B);
    do {
        wr8(ABSX(0x09D3), 0x3B); wr8(ABSX(0x0A53), 0x3B); wr8(ABSX(0x0AD3), 0x3B);
        wr8(ABSX(0x0B53), 0x3B); wr8(ABSX(0x0BD3), 0x3B); wr8(ABSX(0x0C53), 0x3B);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0xBF); wr8(ABS(0x1601), 0xBF);
    lda8(0xF7); wr8(ABS(0x1600), 0xF7);
    lda8(0xFF); wr8(ABS(0x1602), 0xFF);
    lda8(0x34); wr8(ABS(0x0276), 0x34);
    lda8(0x14); wr8(ABS(0x0277), 0x14);
    lda8(0x0A); wr8(ABS(0x0279), 0x0A);
    lda8(0x05); wr8(ABS(0x0278), 0x05);
    ret();
}

void Add32_m1x1(void);

/* The 32-bit accumulation both code routines do: $10-$13 += value * mult
 * (Mul16 into $04-$07, then Add32), `call` being the JSL's return address
 * (the JSR to Add32 follows it). */
static void acc_mul(u8 value, u16 mult, u16 call)
{
    rep(0x20);
    lda16(value);
    wr16(DP(0x00), A);
    lda16(mult); wr16(DP(0x02), mult);
    sep(0x20);
    push8(0x80); push16(call); Mul16_m1x1();
    push16((u16)(call + 3)); Add32_m1x1();
}

/* $80:A17E StoreEntryNumber (RTL): the three entered digits ($1570-$1572)
 * as a number (100s, 10s, 1s), whose bits are spread over the code bytes
 * $0850-$0854 (two bits each into bits 4-5). */
void StoreEntryNumber_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x10), 0);
    wr16(DP(0x12), 0);
    sep(0x20);
    acc_mul(rd8(ABS(0x1570)), 0x0064, 0xA196);
    acc_mul(rd8(ABS(0x1571)), 0x000A, 0xA1AE);
    lda8(rd8(ABS(0x1572)));
    wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA1BF); Add32_m1x1();
    lda8(rd8(DP(0x10)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    and8(0x30); ora8(rd8(ABS(0x0854))); wr8(ABS(0x0854), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    and8(0x30); ora8(rd8(ABS(0x0853))); wr8(ABS(0x0853), (u8)A);
    lda8(rd8(DP(0x10)));
    and8(0x30); ora8(rd8(ABS(0x0852))); wr8(ABS(0x0852), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    and8(0x30); ora8(rd8(ABS(0x0851))); wr8(ABS(0x0851), (u8)A);
    lda8(rd8(DP(0x11)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    and8(0x30); ora8(rd8(ABS(0x0850))); wr8(ABS(0x0850), (u8)A);
    retl();
}

/* $80:A070 LoadPackNumber (RTL): the five decimal digits at $11:5730
 * (the memory pack) as a number, stored as nibbles in $0850-$0853 and the
 * top bits (<< 4, & $1E) in $0855. */
void LoadPackNumber_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x10), 0);
    wr16(DP(0x12), 0);
    sep(0x20);
    acc_mul(rd8(LONG(0x115730)), 0x2710, 0xA089);
    acc_mul(rd8(LONG(0x115731)), 0x03E8, 0xA0A2);
    acc_mul(rd8(LONG(0x115732)), 0x0064, 0xA0BB);
    acc_mul(rd8(LONG(0x115733)), 0x000A, 0xA0D4);
    lda8(rd8(LONG(0x115734)));
    wr8(DP(0x04), (u8)A);
    wr8(DP(0x05), 0); wr8(DP(0x06), 0); wr8(DP(0x07), 0);
    push16(0xA0E6); Add32_m1x1();
    lda8(rd8(DP(0x12)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
    and8(0x1E); wr8(ABS(0x0855), (u8)A);
    lda8(rd8(DP(0x11)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(0x0850), (u8)A);
    lda8(rd8(DP(0x11)));
    and8(0x0F); wr8(ABS(0x0851), (u8)A);
    lda8(rd8(DP(0x10)));
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    wr8(ABS(0x0852), (u8)A);
    lda8(rd8(DP(0x10)));
    and8(0x0F); wr8(ABS(0x0853), (u8)A);
    retl();
}

void LoadRiderPalettes_m1x1(void);

/* $80:D656 StartBonusCourse (RTS): the bonus course set-up: rider
 * palettes; 64 columns of track rows cleared (tile $FC, heights and flags
 * 0, the six rows $3B); the streamer's start; the player alone on screen
 * (column $30, wheel column 9, lean 6, page 1, active, $94 = 2); its HDMA
 * scene (3 before minute $28, else 1); the player drawn at height $B0,
 * then set to $B8; the parallax tables $0D11-$0D25 from $80:D782 on HDMA
 * channels 5-7; zoom $9E = $A2 = $0100, a $012C-frame countdown, the
 * parallax scrolls cleared and HDMA on ($190F = $9C). */
void StartBonusCourse_m1x1(void)
{
    push8(0x80); push16(0xD659); LoadRiderPalettes_m1x1();
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
    lda8(0x34); wr8(ABS(0x0276), 0x34);
    lda8(0x0A); wr8(ABS(0x0279), 0x0A);
    lda8(0x05); wr8(ABS(0x0278), 0x05);
    lda8(0x74); wr8(ABS(0x0277), 0x74);
    lda8(0x08); wr8(ABS(0x018E), 0x08);
    lda8(0x09); wr8(ABS(0x0266), 0x09);
    lda8(0x30); wr8(ABS(0x0196), 0x30);
    lda8(0x06); wr8(ABS(0x01E6), 0x06);
    lda8(0x01);
    wr8(ABS(0x019E), 0x01); wr8(ABS(0x01DE), 0x01); wr8(ABS(0x0958), 0x01);
    lda8(asl8((u8)A));
    wr8(DP(0x94), (u8)A);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x28);
    lda8(fc ? 0x01 : 0x03);
    push8(0x80); push16(0xD6CF); LoadHdmaScene_m1x1();
    ldx8(0x00);
    lda8(0xB0); wr8(ABS(0x0CD5), 0xB0); wr8(ABS(0x01B6), 0xB0);
    push16(0xD6DC); DrawAllRiders_m1x1();
    lda8(0xB8); wr8(ABS(0x0CD5), 0xB8); wr8(ABS(0x01B6), 0xB8);
    ldx8(0x14);
    do {
        lda8(rd8(ABSX(0xD782)));
        wr8(ABSX(0x0D11), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    static const u8 dma[3][4] = { { 0x42, 0x0F, 0x11, 0x0D }, { 0x42, 0x10, 0x18, 0x0D }, { 0x40, 0x08, 0x1F, 0x0D } };
    for (int c = 0; c < 3; c++) {
        u16 b = (u16)(0x4370 - 0x10 * c);
        for (int k = 0; k < 4; k++) { lda8(dma[c][k]); wr8(ABS(b + k), dma[c][k]); }
        lda8(0x00); wr8(ABS(b + 4), 0x00);
        lda8(0x00); wr8(ABS(b + 7), 0x00);
    }
    rep(0x20);
    lda16(0x0100); wr16(DP(0x9E), 0x0100); wr16(DP(0xA2), 0x0100);
    lda16(0x012C); wr16(DP(0x88), 0x012C);
    wr16(ABS(0x1B00), 0); wr16(ABS(0x1B02), 0); wr16(ABS(0x1B04), 0); wr16(ABS(0x1B06), 0);
    lda16(0x0000); wr16(ABS(0x1B20), 0x0000);
    lda16(0x006E); wr16(ABS(0x1B22), 0x006E);
    sep(0x20);
    lda8(0x43); wr8(ABS(0x1B40), 0x43);
    lda8(0x43); wr8(ABS(0x1B41), 0x43);
    lda8(0x9C); wr8(ABS(0x190F), 0x9C);
    ret();
}

void ReadBroadcastDate_m1x1(void);
void MergeBsxFlags_m1x1(void);
void CountCourseBits_m1x1(void);
void ObjectFacing_m1x1(void);
void ApplyVelocityX_m1x1(void);

/* $80:9FF3 ShowEntryCode (RTL): build the entry code (memory pack number,
 * broadcast date, entered number, the BS-X flags, the course bits) and
 * show its eleven bytes $0850-$085A as tiles ((b & $38) << 2 + (b & 7) *
 * 2) in three groups: 4 at VRAM $0871, 4 at $0875, 4 at $0879 (sizes $14,
 * $14, $13; attributes 7, 3, $0B). */
void ShowEntryCode_m1x1(void)
{
    push8(0x80); push16(0x9FF6); LoadPackNumber_m1x1();
    push8(0x80); push16(0x9FFA); ReadBroadcastDate_m1x1();
    push8(0x80); push16(0x9FFE); StoreEntryNumber_m1x1();
    push8(0x80); push16(0xA002); MergeBsxFlags_m1x1();
    push8(0x80); push16(0xA006); CountCourseBits_m1x1();
    ldx8(0x0A);
    do {
        lda8(rd8(ABSX(0x0850)));
        and8(0x38);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABSX(0x0850)));
        and8(0x07);
        lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        wr8(ABSX(0x03D0), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    static const struct { u16 size_at, vram, src; u8 size, attr; u16 back; } g[3] = {
        { 0x03CF, 0x0871, 0x03CF, 0x14, 0x07, 0xA03A },
        { 0x03D3, 0x0875, 0x03D3, 0x14, 0x03, 0xA054 },
        { 0x03D7, 0x0879, 0x03D7, 0x13, 0x0B, 0xA06E },
    };
    for (int i = 0; i < 3; i++) {
        lda8(g[i].size); wr8(ABS(g[i].size_at), g[i].size);
        rep(0x20);
        lda16(g[i].vram); wr16(DP(0x00), g[i].vram);
        lda16(g[i].src); wr16(DP(0x02), g[i].src);
        sep(0x20);
        lda8(g[i].attr); wr8(DP(0x0F), g[i].attr);
        push16(g[i].back); VramQueueTiles_m1x1();
    }
    retl();
}

/* $81:9F3A SteerObjectStep (RTL, Y = flags): object X's velocity toward
 * its target (AimVelocity) with sign-extended high bytes; flag bit 0
 * reverses it (flee), bit 1 updates its facing, bit 2 moves it in X only,
 * bit 3 in both. An axis that passes the target ($039F/$03A1) snaps onto
 * it; C set when both did. */
void SteerObjectStep_m1x1(void)
{
    wr8(DP(0x04), (u8)Y);
    push8(0x81); push16(0x9F3F); AimVelocity_m1x1();
    ldy8(0x00);
    lda8(rd8(ABSX(0x029F)));
    if (fn) Y = dec8((u8)Y);
    lda8((u8)Y); wr8(ABSX(0x02A0), (u8)A);
    ldy8(0x00);
    lda8(rd8(ABSX(0x02AF)));
    if (fn) Y = dec8((u8)Y);
    lda8((u8)Y); wr8(ABSX(0x02B0), (u8)A);
    rep(0x20);
    wr16(DP(0x04), lsr16(rd16(DP(0x04))));      /* LSR $04 with A 16-bit */
    if (fc) {
        lda16(rd16(ABSX(0x029F))); eor16(0xFFFF); lda16(inc16(A)); wr16(ABSX(0x029F), A);
        lda16(rd16(ABSX(0x02AF))); eor16(0xFFFF); lda16(inc16(A)); wr16(ABSX(0x02AF), A);
    }
    sep(0x20);
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9F7B); ObjectFacing_m1x1(); }
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9F83); ApplyVelocityX_m1x1(); }
    wr8(DP(0x04), lsr8(rd8(DP(0x04))));
    if (fc) { push8(0x81); push16(0x9F8B); ApplyVelocity_m1x1(); }
    rep(0x20);
    wr16(DP(0x00), 0);
    lda16(rd16(ABSX(0x029F)));
    if (!fz) {
        lda16(rd16(ABSX(0x027F)));
        fc = 1; sbc16(rd16(ABS(0x039F)));
        eor16(rd16(ABSX(0x029F)));
        if (!fn) {
            lda16(rd16(ABS(0x039F))); wr16(ABSX(0x027F), A);
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
    }
    lda16(rd16(ABSX(0x028F)));
    if (!fz) {
        lda16(rd16(ABSX(0x028F)));
        fc = 1; sbc16(rd16(ABS(0x03A1)));
        eor16(rd16(ABSX(0x02AF)));
        if (!fn) {
            lda16(rd16(ABS(0x03A1))); wr16(ABSX(0x028F), A);
            wr16(DP(0x00), inc16(rd16(DP(0x00))));
        }
    }
    sep(0x20);
    lda8(rd8(DP(0x00)));
    cmp8((u8)A, 0x02);
    fc = fz ? 1 : 0;
    retl();
}

/* $80:B2D6 VramQueueTilesLong (RTL): VramQueueTiles for bank $81/$82. */
void VramQueueTilesLong_m1x1(void)
{
    push16(0xB2D8); VramQueueTiles_m1x1();
    retl();
}

/* $80:B431 QueueTileRowsLong (RTL): QueueTileRows for other banks. */
void QueueTileRowsLong_m1x1(void)
{
    push16(0xB433); QueueTileRows_m1x1();
    retl();
}

/* $80:F901 LanePieceTables (RTS, Y = lane): the pointer $03:$04 to lane
 * Y's track-piece row ($80:F6C0 / $80:F6C6). */
void LanePieceTables_m1x1(void)
{
    lda8(rd8(ABSY(0xF6C0))); wr8(DP(0x03), (u8)A);
    lda8(rd8(ABSY(0xF6C6))); wr8(DP(0x04), (u8)A);
    ret();
}

/* $80:F8F7 LanePieceTablesLong (RTL): LanePieceTables with DB = $80. */
void LanePieceTablesLong_m1x1(void)
{
    push8(DB);
    lda8(0x80);
    push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xF8FE); LanePieceTables_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:F8DB body: lane Y's row pointer, $07 = lane, then the piece at
 * column $08: its height byte into $10 and A = its type byte ($0180
 * further on). */
static void lane_piece(void)
{
    push16(0xF8DD); LanePieceTables_m1x1();
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

/* $80:F8DB LanePieceAt (RTS, Y = lane, $08 = column). */
void LanePieceAt_m1x1(void) { lane_piece(); }

/* $80:F8D6 RiderPieceAt (RTS, Y = lane): LanePieceAt at rider X's wheel
 * column $0266. */
void RiderPieceAt_m1x1(void)
{
    lda8(rd8(ABSX(0x0266)));
    wr8(DP(0x08), (u8)A);
    lane_piece();
}

/* $80:B051 IsPieceFlat (RTS): C clear when the piece type A is $1F. */
void IsPieceFlat_m1x1(void)
{
    cmp8((u8)A, 0x1F);
    fc = fz ? 0 : 1;
    ret();
}

/* $80:B04E RiderOnFlat (RTS, Y = lane): RiderPieceAt, then IsPieceFlat. */
void RiderOnFlat_m1x1(void)
{
    push16(0xB050); RiderPieceAt_m1x1();
    IsPieceFlat_m1x1();
}

/* $80:C049 HeightAbove (RTS): A - rider X's jump height $01FE - its lane
 * $01F6 - $09C7 - 8. */
void HeightAbove_m1x1(void)
{
    fc = 1;
    sbc8(rd8(ABSX(0x01FE)));
    sbc8(rd8(ABSX(0x01F6)));
    sbc8(rd8(ABS(0x09C7)));
    fc = 1;
    sbc8(0x08);
    ret();
}

/* $80:C046 GroundHeightAbove (RTS): HeightAbove from the ground line
 * $0CD5. */
void GroundHeightAbove_m1x1(void)
{
    lda8(rd8(ABS(0x0CD5)));
    HeightAbove_m1x1();
}

void RiderAhead_m1x1(void);
void CrashLand_m1x1(void);
void RiderOnFlat_m1x1(void);
void GroundHeightAbove_m1x1(void);
void VramQueueTilesLong_m1x1(void);
void QueueTileRowsLong_m1x1(void);

/* $80:DC95 / $80:EF5C: the player stops dead (flags $01CE = $0999 = $FF,
 * speed, jump height and fractions cleared). */
static void player_stop(void)
{
    lda8(0xFF);
    wr8(ABS(0x01CE), 0xFF);
    wr8(ABS(0x0999), 0xFF);
    lda8(0x00);
    wr8(ABS(0x01BE), 0); wr8(ABS(0x01C6), 0); wr8(ABS(0x09C4), 0);
    wr8(ABS(0x09C7), 0); wr8(ABS(0x0186), 0); wr8(ABS(0x08F4), 0);
}

/* $80:DCC1 / $80:EF88: the player crash-lands, copying $08D4 / $08DC to
 * $08E4 / $08EC. */
static void player_crash_tail(u16 base)
{
    push16((u16)(base + 0x53)); CrashLand_m1x1();
    lda8(rd8(ABS(0x08D4))); wr8(ABS(0x08E4), (u8)A);
    lda8(rd8(ABS(0x08DC))); wr8(ABS(0x08EC), (u8)A);
    ret();
}

/* $80:DC70 / $80:EF37 (two copies, base = the entry): the player against
 * the track column at its wheel ($0266). On the ground ($01EE = 0) with
 * no jump in progress, a step up taller than its speed $01B6 stops it.
 * In the air, landing at or below the column height crash-lands it
 * (stopping it first when it lands below); otherwise $0958 = 1 when the
 * piece is a $FC ramp top. */
static void player_column(u16 base)
{
    Y = rd8(ABS(0x0266)); nz8((u8)Y);
    lda8(rd8(ABS(0x01EE)));
    if (!fz) {
        lda8(rd8(ABS(0x0904)));
        if (!fz) { ret(); return; }
        lda8(rd8(ABSY(0x0A13)));
        Y = inc8((u8)Y);
        cmp8((u8)A, rd8(ABSY(0x0A13)));
        if (fz) { ret(); return; }
        push16((u16)(base + 0x18)); GroundHeightAbove_m1x1();
        fc = 0; adc8(0x08);
        fc = 1; sbc8(rd8(ABSY(0x0A13)));
        cmp8((u8)A, rd8(ABS(0x01B6)));
        if (fc) { ret(); return; }
        player_stop();
        ret();
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
        if (stop) { push16((u16)(base + 0x50)); player_stop(); pull16(); }
        player_crash_tail(base);
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

void PlayerColumnCheck_m1x1(void) { player_column(0xDC70); }
void PlayerColumnCheckB_m1x1(void) { player_column(0xEF37); }

/* $80:AFE9 / $80:B01E: CPU rider X picks a lane change. Up (AFE9) wants
 * lane $08A4 - 1 (from lane 0 it tries down instead), down (B01E) lane
 * + 1 (from lane 4, up). The lane must be flat (RiderOnFlat) and clear
 * of a rider ahead (RiderAhead at row $0958 -/+ 1); next to the far
 * lane (3 going up, 2 going down) that lane must be flat too. Then the
 * steering $01AE = +1 (up) / -1 (down). */
static void steer_lane(int up, int from_ahead)
{
    u16 b;
    if (!from_ahead) {
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
        b = up ? 0xAFE9 : 0xB01E;
        push16((u16)(b + 0x0A)); RiderOnFlat_m1x1();
        if (fc) { ret(); return; }
    }
    b = up ? 0xAFE9 : 0xB01E;
    Y = rd8(ABSX(0x0958));
    Y = up ? inc8((u8)Y) : dec8((u8)Y);
    wr8(DP(0x02), (u8)Y);
    ldy8(0x00);
    push16((u16)(b + 0x17)); RiderAhead_m1x1();
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x08A4)));
    cmp8((u8)A, up ? 0x03 : 0x02);
    if (fz) {
        ldy8(up ? 0x02 : 0x03);
        push16((u16)(b + 0x25)); RiderOnFlat_m1x1();
        adc8(0x00);
        if (!fz) { ret(); return; }
    }
    lda8(up ? 0x01 : 0xFF);
    wr8(ABSX(0x01AE), (u8)A);
    ret();
}

void CpuSteerUp_m1x1(void) { steer_lane(1, 0); }
void CpuSteerDown_m1x1(void) { steer_lane(0, 0); }

/* $80:B017: down, unless rider X's $0216 is $E6 (then up). */
static void steer_by_e6(void)
{
    lda8(rd8(ABSX(0x0216)));
    cmp8((u8)A, 0xE6);
    steer_lane(fz, 0);
}

/* $81:C883 (RTL): queue the tiles at $03CF (size $20) to VRAM $19F3 and
 * the two rows at $81:C8B5 ($020B) to $19E7. */
void QueueResultsTiles_m1x1(void)
{
    rep(0x20);
    lda16(0x19F3); wr16(DP(0x00), 0x19F3);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push8(0x81); push16(0xC898); VramQueueTilesLong_m1x1();
    rep(0x20);
    lda16(0x19E7); wr16(DP(0x00), 0x19E7);
    lda16(0xC8B5); wr16(DP(0x02), 0xC8B5);
    ldx8(0x81); wr8(DP(0x04), 0x81);
    lda16(0x020B); wr16(DP(0x10), 0x020B);
    sep(0x20);
    push8(0x81); push16(0xC8B3); QueueTileRowsLong_m1x1();
    retl();
}

/* $82:84EE (RTL): for each of the four players flagged in $1E8D, queue
 * its marker tiles ($82:8518, size $2C) at the VRAM address $82:851D. */
void QueuePlayerMarkers_m1x1(void)
{
    push8(DB);
    lda8(0x82); push8(0x82);
    DB = pull8(); nz8(DB);
    ldy8(0x03);
    do {
        lda8(rd8(ABSY(0x1E8D)));
        if (!fz) {
            lda8((u8)Y);
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            rep(0x20);
            lda16(rd16(ABSX(0x851D)));
            wr16(DP(0x00), A);
            lda16(0x8518); wr16(DP(0x02), 0x8518);
            sep(0x20);
            lda8(0x2C); wr8(DP(0x0F), 0x2C);
            push8(0x82); push16(0x8512); VramQueueTilesLong_m1x1();
        }
        Y = dec8((u8)Y);
    } while (!fn);
    DB = pull8(); nz8(DB);
    retl();
}

void LanePieceTables_m1x1(void);
void LanePieceTablesLong_m1x1(void);

/* $81:8BEB (RTL): fill column $0F of the six lane rows (5 down to 0) from
 * the course buffer, two bytes a lane from entry $1A down: the piece
 * $7F:000E,X (skipping $FC ramp tops) and its height $7F:200E,X ($0180
 * further on in the row). */
void FillLaneColumn_m1x1(void)
{
    ldx8(0x05);
    lda8(0x1A); wr8(DP(0x0C), 0x1A);
    do {
        Y = X; nz8((u8)Y);
        push8(0x81); push16(0x8BF5); LanePieceTablesLong_m1x1();
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

/* $80:DB7B / $80:EE40 (two copies, tables = the per-course pointer
 * tables): the next column ($09C3) of course $027D's lane data into
 * column $0F of lanes 0-3; lane 2's piece (0 = $3B) also fills the six
 * scenery rows $0B53-$0C93. */
static void course_column(u16 lo_table, u16 hi_table, u16 back)
{
    X = rd8(ABS(0x027D)); nz8((u8)X);
    lda8(rd8(ABSX(lo_table))); wr8(DP(0x09), (u8)A);
    lda8(rd8(ABSX(hi_table))); wr8(DP(0x0A), (u8)A);
    ldy8(0x00); wr8(DP(0x0E), 0);
    do {
        Y = rd8(DP(0x0E)); nz8((u8)Y);
        push16(back); LanePieceTables_m1x1();
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

void CourseColumn_m1x1(void) { course_column(0xD30D, 0xD316, 0xDB90); }
void CourseColumnB_m1x1(void) { course_column(0xE02F, 0xE040, 0xEE55); }

/* $81:9B5D (RTL): object X's map tile (row $028F, column $027F in pixels)
 * from the map $81:9B7F (+$A0 unless $FF), queued as one tile word
 * (palette $11) at VRAM $1840 + offset. */
void QueueMapTile_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABSX(0x028F)));
    and16(0x00F8);
    lda16(asl16(A)); lda16(asl16(A));
    wr16(DP(0x00), A);
    lda16(rd16(ABSX(0x027F)));
    lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
    fc = 0; adc16(rd16(DP(0x00)));
    Y = (u8)A; nz8((u8)Y);
    fc = 0; adc16(0x1840);
    wr16(DP(0x00), A);
    lda16(0x03A5); wr16(DP(0x02), 0x03A5);
    lda16(rd16(ABSY(0x9B7F)));
    and16(0x00FF);
    cmp16(A, 0x00FF);
    if (!fz) { fc = 0; adc16(0x00A0); }
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    ora16(0x0011);
    wr16(ABS(0x03A5), A);
    sep(0x20);
    lda8(0x1C); wr8(DP(0x0F), 0x1C);
    push8(0x81); push16(0x9B9D); VramQueueTilesLong_m1x1();
    retl();
}

/* $81:C6B4 (RTS): the banner at VRAM $194B. Once the score $9C (nonzero)
 * reaches $1985 it holds frame 2 (and is left alone from $197F's level 2
 * on); before that it alternates frames 0 / 2 every 32 frames ($63). Two rows ($020A) of
 * the frame at $81:C6B0. */
void QueueBlinkBanner_m1x1(void)
{
    rep(0x20);
    lda16(0x194B); wr16(DP(0x00), 0x194B);
    lda16(rd16(DP(0x9C)));
    int alternate = fz;
    if (!alternate) { cmp16(A, rd16(ABS(0x1985))); alternate = !fc; }
    if (!alternate) {
        lda16(rd16(ABS(0x197F)));
        and16(0x000F);
        cmp16(A, 0x0002);
        if (fc) { sep(0x20); ret(); return; }
        ldx8(0x02);
    } else {
        lda16(0x194B); wr16(DP(0x00), 0x194B);
        lda16(rd16(DP(0x63)));
        and16(0x0020);
        lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A)); lda16(lsr16(A));
        X = (u8)A; nz8((u8)X);
    }
    lda16(rd16(ABSX(0xC6B0)));
    wr16(DP(0x02), A);
    lda16(0x020A); wr16(DP(0x10), 0x020A);
    sep(0x20);
    lda8(0x81); wr8(DP(0x04), 0x81);
    push8(0x81); push16(0xC6F5); QueueTileRowsLong_m1x1();
    sep(0x20);
    ret();
}

/* $82:83A1 (RTL): the five digits at [$18] (leading zeros as blank $0C)
 * as a two-row number ($03D0 / $03D6 +$10, a trailing 0 / $10) queued
 * with size $26 at VRAM $00. X and Y are kept. */
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
        wr8(ABSY(0x03D0), (u8)A);
        ora8(0x10);
        wr8(ABSY(0x03D6), (u8)A);
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x05);
    } while (!fc);
    wr8(ABS(0x03D5), 0);
    lda8(0x10); wr8(ABS(0x03DB), 0x10);
    lda8(0x26); wr8(ABS(0x03CF), 0x26);
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push8(0x82); push16(0x83DE); VramQueueTilesLong_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}

/* $82:83E2 (RTL, Y = last index): the Y + 1 digits at ($02) as a two-row
 * number (lower row +$10), leading zeros blanked ($FF), size Y + 1 | $20,
 * queued at VRAM $00. */
void QueueDigitsAt_m1x1(void)
{
    lda8((u8)Y);
    wr8(DP(0x04), (u8)A);
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    do {
        lda8(rd8((((u32)DB << 16) + rd16(DP(0x02)) + Y) & 0xFFFFFF));
        wr8(ABSY(0x03D0), (u8)A);
        ora8(0x10);
        wr8(ABSX(0x03D1), (u8)A);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
    } while (!fn);
    lda8(rd8(DP(0x04)));
    lda8(inc8((u8)A));
    ora8(0x20);
    wr8(ABS(0x03CF), (u8)A);
    ldx8(0x00);
    lda8(rd8(DP(0x04)));
    lda8(inc8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    for (;;) {
        lda8(rd8(ABSX(0x03D0)));
        if (!fz) break;
        lda8(0xFF);
        wr8(ABSX(0x03D0), 0xFF);
        wr8(ABSY(0x03D0), 0xFF);
        Y = inc8((u8)Y);
        X = inc8((u8)X);
        cmp8((u8)X, rd8(DP(0x04)));
        if (fc) break;
    }
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push8(0x82); push16(0x8422); VramQueueTilesLong_m1x1();
    retl();
}

/* $80:C997 (RTS): the player (not crashed, not off the track) running into
 * the ball: lane $01F6 - 3 .. + 4 around the ball's $097C, X within $12 of
 * $090F, ball ($08C7) out and the player at screen page 1 ($01A1), and the
 * ball low enough (GroundHeightAbove - 8 below speed $01B6). Then the
 * player stops (flags $01CE / $01B1 / $01D9 / $091F) and A = 1. */
void BallHit_m1x1(void)
{
    lda8(rd8(ABS(0x01D6)));
    ora8(rd8(ABS(0x0940)));
    if (!fz) { ret(); return; }
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(0x01A1)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x08C7)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x01F6)));
    fc = 1; sbc8(0x03);
    cmp8((u8)A, rd8(ABS(0x097C)));
    if (fc) { ret(); return; }
    adc8(0x07);
    cmp8((u8)A, rd8(ABS(0x097C)));
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(0x0196)));
    cmp8((u8)A, rd8(ABS(0x090F)));
    if (fc) { ret(); return; }
    adc8(0x12);
    cmp8((u8)A, rd8(ABS(0x090F)));
    if (!fc) { ret(); return; }
    push16(0xC9CF); GroundHeightAbove_m1x1();
    fc = 1; sbc8(0x08);
    cmp8((u8)A, rd8(ABS(0x01B6)));
    if (fc) { ret(); return; }
    ldy8(0xFF); wr8(ABS(0x01CE), 0xFF);
    Y = inc8((u8)Y); wr8(ABS(0x01B1), (u8)Y);
    Y = inc8((u8)Y); wr8(ABS(0x01D9), (u8)Y);
    Y = inc8((u8)Y); wr8(ABS(0x091F), (u8)Y);
    lda8(0x01);
    ret();
}

void QueueDigitsAt_m1x1(void);
void QueueFiveDigits_m1x1(void);
void QueueMapTile_m1x1(void);
void VelocityDirection_m1x1(void);
void SteerObjectStep_m1x1(void);
void QueueTileRows_m1x1(void);
void CpuReleaseButtons_m1x1(void);

/* $80:9567 (RTS): the six digits $1549 become $0CE5 - $1549 (digit by
 * digit, then borrowing), queued at VRAM $19B6 (QueueDigitsAt, size $20). */
void ShowDigitDifference_m1x1(void)
{
    ldx8(0x05);
    do {
        lda8(rd8(ABSX(0x0CE5)));
        fc = 1; sbc8(rd8(ABSX(0x1549)));
        wr8(ABSX(0x1549), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    ldx8(0x05);
    do {
        lda8(rd8(ABSX(0x1549)));
        if (fn) {
            fc = 0; adc8(0x0A);
            wr8(ABSX(0x1549), (u8)A);
            wr8(ABSX(0x1548), dec8(rd8(ABSX(0x1548))));
        }
        X = dec8((u8)X);
    } while (!fn);
    rep(0x20);
    lda16(0x1549); wr16(DP(0x02), 0x1549);
    lda16(0x19B6); wr16(DP(0x00), 0x19B6);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    ldy8(0x05);
    push8(0x80); push16(0x95A0); QueueDigitsAt_m1x1();
    ret();
}

/* $80:9949 (RTS): when the timer $88 runs out, the five digits at
 * $00:0CE5 at VRAM $0975, $88 = $20, and the end screen's next steps
 * ($0450 = 4, $0451 = 5; or 3 when $085D is set). */
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
    push8(0x80); push16(0x9962); QueueFiveDigits_m1x1();
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(rd8(ABS(0x085D)));
    if (fz) {
        lda8(0x04); wr8(ABS(0x0450), 0x04);
        lda8(0x05); wr8(ABS(0x0451), 0x05);
    } else {
        lda8(0x03); wr8(ABS(0x0450), 0x03);
    }
    ret();
}

/* $81:984C (RTS): object $03A3 follows its path ($81:9C3F, four bytes a
 * point: target $039F / $03A1) with SteerObjectStep; on arriving it moves
 * to the next point, and at the $8000 end marker resets the point to $60
 * and steps its state $030F. Otherwise its map tile (unless $0280) and
 * its velocity direction. */
void ObjectFollowPath_m1x1(void)
{
    rep(0x30);
    X = rd16(ABS(0x03A3)); nz16(X);
    lda16(rd16(ABSX(0x031F)));
    lda16(asl16(A)); lda16(asl16(A));
    Y = A; nz16(Y);
    lda16(rd16(ABSY(0x9C3F))); wr16(ABS(0x039F), A);
    lda16(rd16(ABSY(0x9C41))); wr16(ABS(0x03A1), A);
    sep(0x30);
    ldy8(0x0C);
    lda8(0x20);
    push8(0x81); push16(0x986C); SteerObjectStep_m1x1();
    if (fc) {
        rep(0x30);
        wr16(ABSX(0x031F), inc16(rd16(ABSX(0x031F))));
        lda16(rd16(ABSX(0x031F)));
        lda16(asl16(A)); lda16(asl16(A));
        Y = A; nz16(Y);
        lda16(rd16(ABSY(0x9C3F)));
        cmp16(A, 0x8000);
        sep(0x30);
        if (fz) {
            rep(0x20);
            lda16(0x0060); wr16(ABSX(0x031F), 0x0060);
            sep(0x20);
            wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
            ret();
            return;
        }
    }
    lda8(rd8(ABSX(0x0280)));
    if (fz) { push8(0x81); push16(0x989A); QueueMapTile_m1x1(); }
    push8(0x81); push16(0x989E); VelocityDirection_m1x1();
    ret();
}

/* $80:99F0 (RTS): the end screen's entry code. After the $88 delay the
 * code box (2 rows of $20 from $7E:5880) is queued at $0300; then a
 * press of A ($26 bit 7) gives sound $43, $88 = $20, step $0450 = $0A and
 * queues the five digits at $11:5730 to VRAM $0A15. */
void EndScreenCode_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) {
        u8 t = dec8(rd8(DP(0x88)));
        wr8(DP(0x88), t);
        if (!fz) { ret(); return; }
        lda8(0x80); wr8(DP(0x02), 0x80);
        lda8(0x58); wr8(DP(0x03), 0x58);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x9A16); QueueTileRows_m1x1();
    }
    lda8(rd8(DP(0x26)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(0x43);
    push8(0x80); push16(0x9A26); QueueSoundB_m1x1();
    lda8(0x0A); wr8(ABS(0x0450), 0x0A);
    rep(0x20);
    lda16(0x0A15); wr16(DP(0x00), 0x0A15);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9A41); QueueFiveDigits_m1x1();
    ret();
}

/* $80:AF87 (RTS, X = rider): CPU riders 0-3 on screen page 1 decide on a
 * lane change. Blocked by a rider ahead in its lane: let go of the
 * buttons and, on a $1F piece in the right half of the screen, steer
 * away (by its pad bit 0 / $0216). Otherwise, not already steering: at
 * a $0C piece move toward the middle lanes; else, unless on a $1E
 * piece, at X $80-$EF try a lane up. */
void CpuLaneDecision_m1x1(void)
{
    cmp8((u8)X, 0x04);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x019E)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x0958)));
    wr8(DP(0x02), (u8)A);
    ldy8(0x02);
    push16(0xAF9B); RiderAhead_m1x1();
    if (fc) {
        push16(0xAFA0); CpuReleaseButtons_m1x1();
        lda8(rd8(ABSX(0x021E)));
        cmp8((u8)A, 0x1F);
        if (!fz) { ret(); return; }
        lda8(rd8(ABSX(0x0196)));
        lda8(asl8((u8)A));
        if (!fc) { ret(); return; }
        lda8(rd8(DPX(0x2C)));
        lda8(lsr8((u8)A));
        if (fc) steer_by_e6();
        else steer_lane(1, 0);
        return;
    }
    cmp8((u8)X, 0x04);
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x01AE)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x021E)));
    cmp8((u8)A, 0x0C);
    if (fz) {
        lda8(rd8(ABSX(0x08A4)));
        cmp8((u8)A, 0x03);
        steer_lane(fc, 1);
        return;
    }
    cmp8((u8)A, 0x1E);
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x019E)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x0196)));
    cmp8((u8)A, 0xF0);
    if (fc) { ret(); return; }
    cmp8((u8)A, 0x80);
    if (!fc) { ret(); return; }
    steer_lane(1, 0);
}

/* $80:DC95 / $80:DC9D and $80:EF5C / $80:EF64: the player stops (the
 * full stop, or only the speed and jump state cleared). */
static void player_halt(void)
{
    lda8(0x00);
    wr8(ABS(0x01BE), 0); wr8(ABS(0x01C6), 0); wr8(ABS(0x09C4), 0);
    wr8(ABS(0x09C7), 0); wr8(ABS(0x0186), 0); wr8(ABS(0x08F4), 0);
    ret();
}

void PlayerStop_m1x1(void)
{
    lda8(0xFF);
    wr8(ABS(0x01CE), 0xFF);
    wr8(ABS(0x0999), 0xFF);
    player_halt();
}

void PlayerHalt_m1x1(void) { player_halt(); }
void PlayerStopB_m1x1(void) { PlayerStop_m1x1(); }
void PlayerHaltB_m1x1(void) { player_halt(); }

/* $81:AC0B (RTL): an empty hook. */
void EmptyHook_m1x1(void)
{
    retl();
}

/* $80:F9C2 (RTS): rider X's $01A6 = $0981 = A, then A = its $024E -
 * $023E. */
void SetRiderPair_m1x1(void)
{
    wr8(ABSX(0x01A6), (u8)A);
    wr8(ABSX(0x0981), (u8)A);
    lda8(rd8(ABSX(0x024E)));
    fc = 1; sbc8(rd8(ABSX(0x023E)));
    ret();
}

/* $80:F8D3 RiderLanePieceAt (RTS): RiderPieceAt in rider X's lane $08A4. */
void RiderLanePieceAt_m1x1(void)
{
    Y = rd8(ABSX(0x08A4)); nz8((u8)Y);
    RiderPieceAt_m1x1();
}

void RiderLanePieceAt_m1x1(void);
void SetRiderPair_m1x1(void);
void SpawnPlayerSpark_m1x1(void);
void StartWheelie_m1x1(void);
void CheckBonusTime_m1x1(void);
void EndAtBonusMinute_m1x1(void);
void SetBgTileBase_m1x1(void);
void EmptyHook_m1x1(void);

/* $80:B194 (RTS): for riders 7-0, the piece under the wheel (type $021E,
 * height $0216) and the four piece heights from there on ($0884 + 4X,
 * the column wrapping at $40). */
void ReadRiderPieces_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        wr8(DP(0x0C), (u8)A);
        lda8(0x04); wr8(DP(0x0B), 0x04);
        push16(0xB1A1); RiderLanePieceAt_m1x1();
        wr8(ABSX(0x021E), (u8)A);
        lda8(rd8(DP(0x10)));
        wr8(ABSX(0x0216), (u8)A);
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

/* The player's lane piece ($0180 on: its type byte), Y left 16-bit. */
static void player_piece_type(u16 back)
{
    push16(back); RiderLanePieceAt_m1x1();
    rep(0x30);
    lda16(Y);
    fc = 0; adc16(0x0180);
    Y = A; nz16(Y);
    sep(0x20);
}

static u32 piece_addr(void)
{
    return (((u32)DB << 16) + rd16(DP(0x03)) + Y) & 0xFFFFFF;
}

/* The course map entry ($7F:4000) for the column $54 back from the
 * scroll $AE (wrapping by $B0), by the position table $7F:6000: X = the
 * entry, Y = the player's lane, 8-bit A. */
static void course_map_slot(void)
{
    rep(0x30);
    lda16(rd16(DP(0xAE)));
    fc = 1; sbc16(0x0054);
    if (fn) { fc = 0; adc16(rd16(DP(0xB0))); }
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

/* $80:FA0E (RTS, X = rider): the player on the ground (and $84 clear)
 * turns its piece flat ($1F) and sets $B2 from its lane ($80:FA39). */
void FlattenPlayerPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x01EE)));
    ora8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    player_piece_type(0xFA1B);
    lda8(0x1F);
    wr8(piece_addr(), 0x1F);
    sep(0x10);
    push8((u8)X);
    X = rd8(ABS(0x08A4)); nz8((u8)X);
    lda8(rd8(ABSX(0xFA39)));
    wr8(DP(0xB2), (u8)A);
    sep(0x10);
    X = pull8(); nz8((u8)X);
    ret();
}

/* $80:FA40 (RTS, X = rider): for the player on the ground in a lane with
 * an arrow ($80:FAB0): sound $32, $1979 = 1, and an arrow piece ($15)
 * becomes used ($12), with the course map entry set to the lane's
 * $80:FAA9 and $B7 from $80:FA39. Then StartWheelie. */
void UseArrowPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(0x01EE)));
        if (fz) {
            Y = rd8(ABS(0x08A4)); nz8((u8)Y);
            lda8(rd8(ABSY(0xFAB0)));
            if (!fz) {
                lda8(0x32);
                push8(0x80); push16(0xFA56); QueueSoundB_m1x1();
                lda8(0x01); wr8(ABS(0x1979), 0x01);
                player_piece_type(0xFA5E);
                lda8(rd8(piece_addr()));
                cmp8((u8)A, 0x15);
                if (fz) {
                    lda8(0x12);
                    wr8(piece_addr(), 0x12);
                    push16(X);
                    course_map_slot();
                    lda8(rd8(ABSY(0xFAA9)));
                    wr8(LONGX(0x7F4000), (u8)A);
                    lda8(rd8(ABSY(0xFA39)));
                    wr8(DP(0xB7), (u8)A);
                    X = pull16(); nz16(X);
                }
            }
        }
    }
    sep(0x30);
    StartWheelie_m1x1();
}

/* $80:FAB7 (RTS, X = rider): the player on the ground collects a pickup:
 * a spark, player $7C's flag $1E8D, and in a lane with one ($80:FB1E)
 * sound $47 and the pickup piece ($16) cleared to flat ($1F), its course
 * map entry set to $80. */
void CollectPickupPiece_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(0x01EE)));
        if (fz) {
            push8(0x80); push16(0xFAC3); SpawnPlayerSpark_m1x1();
            Y = rd8(DP(0x7C)); nz8((u8)Y);
            lda8(0x01); wr8(ABSY(0x1E8D), 0x01);
            Y = rd8(ABS(0x08A4)); nz8((u8)Y);
            lda8(rd8(ABSY(0xFB1E)));
            if (!fz) {
                lda8(0x47);
                push8(0x80); push16(0xFAD8); QueueSoundB_m1x1();
                player_piece_type(0xFADB);
                lda8(rd8(piece_addr()));
                cmp8((u8)A, 0x16);
                if (fz) {
                    lda8(0x1F);
                    wr8(piece_addr(), 0x1F);
                    push16(X);
                    course_map_slot();
                    lda8(0x80);
                    wr8(LONGX(0x7F4000), 0x80);
                    X = pull16(); nz16(X);
                }
            }
        }
    }
    sep(0x30);
    ret();
}

/* $80:F96E (RTS, X = rider): jump height $01FE = $026E - A. */
static void height_below(void)
{
    wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(0x026E)));
    fc = 1; sbc8(rd8(DP(0x00)));
    wr8(ABSX(0x01FE), (u8)A);
    ret();
}

void HeightBelow_m1x1(void) { height_below(); }

/* $80:F969 (RTS): SetRiderPair(1), then HeightBelow. */
void RiderPairHeight_m1x1(void)
{
    lda8(0x01);
    push16(0xF96D); SetRiderPair_m1x1();
    height_below();
}

/* $80:F97A (RTS, X = rider): the player's speed ($01BE / $01C6) + $40,
 * at most $0340, then RiderPairHeight. */
void PlayerSpeedBoost_m1x1(void)
{
    lda8(rd8(ABS(0x01BE))); wr8(DP(0x10), (u8)A);
    lda8(rd8(ABS(0x01C6))); wr8(DP(0x11), (u8)A);
    rep(0x20);
    lda16(rd16(DP(0x10)));
    fc = 0; adc16(0x0040);
    cmp16(A, 0x0340);
    if (fc) lda16(0x0340);
    sep(0x20);
    wr8(ABS(0x01BE), (u8)A);
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    wr8(ABS(0x01C6), (u8)A);
    lda8(0x01);
    push16(0xF9A1); SetRiderPair_m1x1();
    height_below();
}

/* $80:DD45 Mode0A (RTS): the bonus course: its time and minute checks,
 * the BG tile base, then (once $3A is clear) its step $77 through the
 * table $80:DD5F. */
void Mode0A_m1x1(void)
{
    push16(0xDD47); CheckBonusTime_m1x1();
    push16(0xDD4A); EndAtBonusMinute_m1x1();
    push8(0x80); push16(0xDD4E); SetBgTileBase_m1x1();
    push8(0x80); push16(0xDD52); EmptyHook_m1x1();
    lda8(rd8(DP(0x3A)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x77)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    r816_call_pc(0x800000u | rd16(LONGX(0x80DD5F)));
}

void HeightBelow_m1x1(void);
void HeightAbove_m1x1(void);

/* $80:F94B (RTS, X = rider): jump height $01FE and
 * its base $026E = A. */
void SetJumpBase_m1x1(void)
{
    wr8(ABSX(0x01FE), (u8)A);
    wr8(ABSX(0x026E), (u8)A);
    ret();
}

/* $80:F9D0 (RTS, X = rider): unless it is in jump state 1, SetRiderPair(3)
 * and the height 6 below its base; in state 4 it crash-lands into
 * state 2 ($08AC = 2 too). */
void RiderDropStep_m1x1(void)
{
    lda8(rd8(ABSX(0x020E)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) { ret(); return; }
    }
    lda8(0x03);
    push16(0xF9DD); SetRiderPair_m1x1();
    lda8(asl8((u8)A));
    push16(0xF9E1); HeightBelow_m1x1();
    lda8(rd8(ABSX(0x020E)));
    cmp8((u8)A, 0x04);
    if (!fz) { ret(); return; }
    push16(0xF9EB); CrashLand_m1x1();
    lda8(0x02);
    wr8(ABSX(0x020E), 0x02);
    wr8(ABSX(0x08AC), 0x02);
    ret();
}

/* $80:BFDB: rider X crashes on landing: sound $3D for the player, $3D,X
 * = $0A, crashed ($01CE), then sound $24 (player) / $29 if on screen
 * page 1. */
static void landing_crash(void)
{
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(0x3D);
        push8(0x80); push16(0xBFE4); QueueSoundB_m1x1();
    }
    lda8(0x0A); wr8(DPX(0x3D), 0x0A);
    lda8(0xFF); wr8(ABSX(0x01CE), 0xFF);
    lda8((u8)X);
    if (!fz) lda8(0x05);
    fc = 0; adc8(0x24);
    wr8(DP(0x10), (u8)A);
    lda8(rd8(ABSX(0x019E)));
    cmp8((u8)A, 0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x10)));
    push8(0x80); push16(0xC004); QueueSoundB_m1x1();
    ret();
}

/* $80:BF5C (RTS, X = rider): an airborne rider ($01EE) whose height above
 * the ground (less $09C4) has dropped below its fall speed $01B6 lands,
 * unless held ($0904) or not falling ($08AC). The player lands with
 * sound $21 (and in the jump contest, state $C7 = 2 and the landing
 * rank from its pose, $80:C036, if none yet). On landing the jump state
 * clears; then unless crashed ($01CE) or already at speed ($08F4), a
 * landing angle $01E6 outside the window $80:BA73 / $80:BA83 for its
 * speed (+8 when slow) crashes it (sound $3D for the player, $3D,X = $0A,
 * then sound $24 / $29 if on screen); inside it, a changed angle on
 * the flat bounces it ($08CC = 4, $08F4 = 2, $01EE = 1). The fall speed
 * then becomes the height less one. With $84 set it only levels the
 * angle ($01E6 = 6) when still. */
void RiderLanding_m1x1(void)
{
    lda8(rd8(ABSX(0x01EE)));
    if (fz) { ret(); return; }
    push16(0xBF63); GroundHeightAbove_m1x1();
    sbc8(rd8(ABS(0x09C4)));
    wr8(DP(0x01), (u8)A);
    cmp8((u8)A, rd8(ABSX(0x01B6)));
    if (fc) { ret(); return; }
    lda8(rd8(ABSX(0x0904)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x08AC)));
    if (fz) { ret(); return; }
    lda8((u8)X);
    if (fz) {
        lda8(0x21);
        push8(0x80); push16(0xBF80); QueueSoundB_m1x1();
        lda8(rd8(DP(0xC7)));
        if (!fz) {
            lda8(0x02); wr8(DP(0xC7), 0x02);
            lda8(rd8(ABS(0x156B)));
            if (fn) {
                Y = rd8(ABSX(0x1961)); nz8((u8)Y);
                lda8(rd8(ABSY(0xC036)));
                wr8(ABS(0x156B), (u8)A);
            }
        }
    }
    lda8(0x00);
    wr8(ABSX(0x01EE), 0);
    wr8(ABSX(0x08AC), 0);
    lda8(rd8(ABS(0x09C4)));
    wr8(ABS(0x09C7), (u8)A);
    lda8(rd8(ABSX(0x01CE)));
    if (!fz) goto fall;
    lda8(rd8(DP(0x84)));
    if (!fz) {
        lda8(rd8(ABSX(0x01A6)));
        if (!fz) { ret(); return; }
        lda8(0x06); wr8(ABSX(0x01E6), 0x06);
        ret();
        return;
    }
    Y = rd8(ABSX(0x08F4));
    wr8(ABSX(0x08F4), (u8)A);
    lda8((u8)Y);
    if (!fz) goto fall;
    lda8(rd8(ABSX(0x01A6)));
    Y = rd8(ABSX(0x01C6)); nz8((u8)Y);
    cmp8((u8)Y, 0x02);
    {
        int slow = !fc;
        if (!slow && fz) {
            Y = rd8(ABSX(0x01BE)); nz8((u8)Y);
            cmp8((u8)Y, 0x80);
            slow = !fc;
        }
        if (slow) { fc = 0; adc8(0x08); }
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSX(0x01E6)));
    cmp8((u8)A, rd8(ABSY(0xBA73)));
    if (fc) {
        cmp8((u8)A, rd8(ABSY(0xBA83)));
        if (!fc) {
            lda8(rd8(ABSX(0x08B4)));
            cmp8((u8)A, rd8(ABSX(0x01E6)));
            if (!fz) {
                lda8(rd8(ABSX(0x01A6)));
                ora8(rd8(ABSX(0x0226)));
                if (fz) {
                    lda8(0x04); wr8(ABSX(0x08CC), 0x04);
                    lda8(lsr8((u8)A)); wr8(ABSX(0x08F4), (u8)A);
                    lda8(lsr8((u8)A)); wr8(ABSX(0x01EE), (u8)A);
                }
            }
            goto fall;
        }
    }
    landing_crash();
    return;
fall:
    wr8(DP(0x01), dec8(rd8(DP(0x01))));
    lda8(rd8(DP(0x01)));
    wr8(ABSX(0x01B6), (u8)A);
    ret();
}

/* $80:FD7E (RTS): every rider's shadow; an airborne rider near the left
 * or right screen edge (page 0 from $F6, page 1 below $F4) also gets a
 * marker sprite ($16A0 + 4X: X + $0A, the height above $B6, frame
 * $80:F6CC by its $0981, attribute $2F, size 2). */
void DrawShadowsAndMarkers_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        int marker;
        lda8(rd8(ABSX(0x019E)));
        cmp8((u8)A, 0x01);
        if (fz) {
            lda8(rd8(ABSX(0x0196)));
            cmp8((u8)A, 0xF4);
            marker = !fc;
        } else {
            cmp8((u8)A, 0x00);
            marker = 0;
            if (fz) {
                lda8(rd8(ABSX(0x0196)));
                cmp8((u8)A, 0xF6);
                marker = fc;
            }
        }
        if (marker) {
            lda8(rd8(ABSX(0x01EE)));
            if (!fz) {
                lda8(0xB6);
                push16(0xFDA8); HeightAbove_m1x1();
                wr8(ABSY(0x16A1), (u8)A);
                lda8(rd8(ABSX(0x0196)));
                fc = 0; adc8(0x0A);
                wr8(ABSY(0x16A0), (u8)A);
                wr8(DP(0x01), (u8)Y);
                Y = rd8(ABSX(0x0981)); nz8((u8)Y);
                lda8(rd8(ABSY(0xF6CC)));
                Y = rd8(DP(0x01)); nz8((u8)Y);
                wr8(ABSY(0x16A2), (u8)A);
                push8((u8)Y);
                lda8((u8)Y);
                fc = 0; adc8(0xA0);
                lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
                Y = (u8)A; nz8((u8)Y);
                lda8(0x02); wr8(ABSY(0x1820), 0x02);
                Y = pull8(); nz8((u8)Y);
                lda8(0x2F); wr8(ABSY(0x16A3), 0x2F);
            }
        }
        push16(0xFDD9); DrawRiderShadow_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:89B4 (RTS): clear $9E / $A2 (words) and $9B, the empty hook, sound
 * 3, then FinishModeSetup (into which it falls). */
void StartModeWithSound_m1x1(void)
{
    rep(0x20);
    wr16(DP(0x9E), 0); wr16(DP(0xA2), 0);
    sep(0x20);
    wr8(DP(0x9B), 0);
    push8(0x80); push16(0x89C1); EmptyHook_m1x1();
    lda8(0x03);
    push8(0x80); push16(0x89C7); QueueSoundB_m1x1();
    FinishModeSetup_m1x1();
}

void AddSpeedToX_m1x1(void);
void RiderLanding_m1x1(void);
void RiderLandCheck_m1x1(void);
void Tick08CC_m1x1(void);
void ScrollByPlayerSpeed_m1x1(void);
void ScrollRivalsRelative_m1x1(void);
void SetJumpBase_m1x1(void);
void PlaceRiderShadow_m1x1(void);
void AddScore_m1x1(void);

/* $80:BCBD (RTS): while racing ($92), every rider moves, lands and ticks
 * ($08CC); the screen scrolls with the player and the rivals follow; the
 * player's speed high byte $0186 adds into the distance ($0994 fraction,
 * $93), flipping $91 on each carry out. */
void RaceMoveStep_m1x1(void)
{
    lda8(rd8(DP(0x92)));
    if (fz) { ret(); return; }
    ldx8(0x07);
    do {
        push16(0xBCC5); AddSpeedToX_m1x1();
        push16(0xBCC8); RiderLanding_m1x1();
        push16(0xBCCB); RiderLandCheck_m1x1();
        push16(0xBCCE); Tick08CC_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    push16(0xBCD4); ScrollByPlayerSpeed_m1x1();
    push16(0xBCD7); ScrollRivalsRelative_m1x1();
    lda8(rd8(ABS(0x0186)));
    lda8(lsr8((u8)A)); lda8(ror8((u8)A)); lda8(ror8((u8)A)); lda8(ror8((u8)A));
    fc = 0; adc8(rd8(ABS(0x0994)));
    wr8(ABS(0x0994), (u8)A);
    lda8(rd8(ABS(0x0186)));
    adc8(rd8(DP(0x93)));
    wr8(DP(0x93), (u8)A);
    if (fc) {
        lda8(rd8(DP(0x91)));
        eor8(0x01);
        wr8(DP(0x91), (u8)A);
    }
    ret();
}

/* $80:BD09 (RTL): ScrollByPlayerSpeed for other banks. */
void ScrollByPlayerSpeedLong_m1x1(void)
{
    push16(0xBD0B); ScrollByPlayerSpeed_m1x1();
    retl();
}

/* $80:CE8C (RTL): PlaceRiderShadow with DB = $80. */
void PlaceRiderShadowLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xCE93); PlaceRiderShadow_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:FE64 (RTL): AddScore for other banks. */
void AddScoreLong_m1x1(void)
{
    push16(0xFE66); AddScore_m1x1();
    retl();
}

/* $80:F9A5 (RTS, X = rider): SetRiderPair(4), the jump base at 8 (+$10
 * more when $0206 is set), and $0246 = $40. */
void RiderJumpBase_m1x1(void)
{
    lda8(0x04);
    push16(0xF9A9); SetRiderPair_m1x1();
    lda8(asl8((u8)A));
    push16(0xF9AD); SetJumpBase_m1x1();
    lda8(rd8(ABSX(0x0206)));
    if (!fz) {
        lda8(rd8(ABSX(0x01FE)));
        fc = 0; adc8(0x10);
        push16(0xF9BB); SetJumpBase_m1x1();
    }
    lda8(0x40);
    wr8(ABSX(0x0246), 0x40);
    ret();
}

void PlaceRiderShadowLong_m1x1(void);
void AddScoreLong_m1x1(void);
void LanePieceAt_m1x1(void);
void IsPieceFlat_m1x1(void);
void CpuSteerUp_m1x1(void);
void CpuSteerDown_m1x1(void);
void RandomB_m1x1(void);
void RivalTarget_m1x1(void);

/* $81:C5D7 (RTS, Y = slot): the shadow of the rider in slot Y ($0950),
 * with $07 from $81:C5CF; a rider in the middle of screen page 1 ($70-
 * $8F) sets $092A. */
void SlotRiderShadow_m1x1(void)
{
    wr8(DP(0x05), (u8)Y);
    lda8(rd8(ABSY(0x0950)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSY(0xC5CF)));
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
    push8(0x81); push16(0xC5FC); PlaceRiderShadowLong_m1x1();
    ret();
}

/* $82:9159 (RTL): the stadium bonus for $0928 (3 bytes at $82:9131 + 4 *
 * (stadium - 2)) into $09AD-$09AF and added to the score; $00 = stadium
 * - 2. X is kept. */
void AddStadiumBonus_m1x1(void)
{
    push8(DB);
    lda8(0x82); push8(0x82);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    lda8(rd8(ABS(0x0928)));
    lda8(dec8((u8)A)); lda8(dec8((u8)A));
    wr8(DP(0x00), (u8)A);
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x9131))); wr8(ABS(0x09AD), (u8)A);
    lda8(rd8(ABSX(0x9132))); wr8(ABS(0x09AE), (u8)A);
    lda8(rd8(ABSX(0x9133))); wr8(ABS(0x09AF), (u8)A);
    push8(0x82); push16(0x917E); AddScoreLong_m1x1();
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:B0EA (RTS, X = rider): look at the four pieces ahead ($0884 + 4X).
 * A hurdle ($C0/$C1 or $70-$73) sends the rider looking for a flat lane
 * at that column ($40 rows), up first (lanes above, from $08A4 - 1 to
 * 1) and then down (to 5), steering into the first one found; a ramp
 * top ($48-$4A) sets the target angle $0971 = 9; otherwise $0971 = the
 * piece angle $08B4 (0 while landing, $0176). Returns A = the hurdle
 * count $0D. */
void CpuAvoidHurdle_m1x1(void)
{
    lda8(0x00);
    wr8(DP(0x09), 0); wr8(DP(0x0D), 0);
    lda8((u8)X);
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    wr8(DP(0x0A), (u8)A);
    ldy8(0x03);
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
    fc = 0; adc8(rd8(ABSX(0x0266)));
    and8(0x3F);
    wr8(DP(0x08), (u8)A);
    lda8(0x40); wr8(DP(0x09), 0x40);
    Y = rd8(ABSX(0x08A4)); nz8((u8)Y);
    for (;;) {
        Y = dec8((u8)Y);
        cmp8((u8)Y, 0x01);
        if (fn) break;
        push16(0xB14A); LanePieceAt_m1x1();
        Y = rd8(DP(0x07)); nz8((u8)Y);
        push16(0xB14F); IsPieceFlat_m1x1();
        if (!fc) { push16(0xB154); CpuSteerUp_m1x1(); goto angle; }
    }
    Y = rd8(ABSX(0x08A4)); nz8((u8)Y);
    for (;;) {
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x06);
        if (!fn) goto angle;
        push16(0xB162); LanePieceAt_m1x1();
        Y = rd8(DP(0x07)); nz8((u8)Y);
        push16(0xB167); IsPieceFlat_m1x1();
        if (!fc) { push16(0xB16C); CpuSteerDown_m1x1(); goto angle; }
    }
angle:
    ldy8(0x00);
    lda8(rd8(ABSX(0x0176)));
    if (fz) { Y = rd8(ABSX(0x08B4)); nz8((u8)Y); }
    lda8((u8)Y);
store:
    wr8(ABSX(0x0971), (u8)A);
    lda8(rd8(DP(0x0D)));
    ret();
}

/* $80:BDDC (RTS, X = rival): place a rival entering the screen: a random
 * lane ($80:BAF2), X $A0 on page 0 or $40 on page 1, angle 6, the
 * player's column + $80:BB00 and $018E. Entering on page 1 it must not
 * land on a piece taller than $3B-$92 (then A = 0 and nothing more);
 * on a $E6 ramp it starts mid-jump (state 1 at height $30 from lane $20
 * down, else state 3). Then its target speed (RivalTarget), a 1-in-64
 * chance of a fast start ($01C6 = (page ^ 2) * 2), $3B = $10 and
 * $01DE = 1. */
void PlaceRival_m1x1(void)
{
    push8(0x80); push16(0xBDDF); RandomB_m1x1();
    and8(0x03);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBAF2)));
    wr8(ABSX(0x01F6), (u8)A);
    Y = rd8(ABSX(0x019E)); nz8((u8)Y);
    lda8(fz ? 0xA0 : 0x40);
    wr8(ABSX(0x0196), (u8)A);
    lda8(0x06); wr8(ABSX(0x01E6), 0x06);
    lda8(rd8(ABS(0x0266)));
    fc = 0; adc8(rd8(ABSY(0xBB00)));
    and8(0x3F);
    wr8(ABSX(0x0266), (u8)A);
    lda8(rd8(ABS(0x018E)));
    wr8(ABSX(0x018E), (u8)A);
    lda8((u8)Y);
    if (!fz) {
        ldy8(0x05);
        push16(0xBE15); RiderPieceAt_m1x1();
        lda8(rd8(DP(0x10)));
        cmp8((u8)A, 0xE4);
        if (!fz) {
            cmp8((u8)A, 0x3B);
            if (!fc) { lda8(0x00); ret(); return; }
            cmp8((u8)A, 0x93);
            if (fc) { lda8(0x00); ret(); return; }
            ldy8(0x02);
            push16(0xBE28); RiderPieceAt_m1x1();
            cmp8((u8)A, 0xE6);
            if (fz) {
                lda8(rd8(ABSX(0x01F6)));
                cmp8((u8)A, 0x20);
                if (fc) {
                    lda8(0x30); wr8(ABSX(0x01FE), 0x30);
                    lda8(0x01);
                } else lda8(0x03);
                wr8(ABSX(0x020E), (u8)A);
            }
        }
    }
    Y = rd8(ABSX(0x019E)); nz8((u8)Y);
    push16(0xBE47); RivalTarget_m1x1();
    push8(0x80); push16(0xBE4B); RandomB_m1x1();
    and8(0x3F);
    if (fz) {
        lda8(rd8(ABSX(0x019E)));
        eor8(0x02);
        lda8(asl8((u8)A));
        wr8(ABSX(0x01C6), (u8)A);
    }
    lda8(0x10); wr8(DP(0x3B), 0x10);
    lda8(0x01); wr8(ABSX(0x01DE), 0x01);
    ret();
}

void CpuSteer_m1x1(void);
void CpuThrottle_m1x1(void);
void CpuAvoidHurdle_m1x1(void);
void CheckLaneTile_m1x1(void);
void RetargetRival_m1x1(void);
void CpuLaneDecision_m1x1(void);
void UpdateRiderDistances_m1x1(void);
void PlaceRival_m1x1(void);
void AddStadiumBonus_m1x1(void);
void StepObjectX_m1x1(void);
void DrawRiderMarker_m1x1(void);
void QueueBonusDigits_m1x1(void);

/* $80:AF3F (RTS): the CPU riders 1-7 on the track ($01DE) and not crashed,
 * off the track or changing lanes: steer; on the ground also throttle,
 * look for hurdles, and with none the lane tile check, retargeting (when
 * $092A) and (unless $84) a lane decision. In the air the target angle
 * $0971 follows the piece angle $08B4. */
void CpuRidersStep_m1x1(void)
{
    ldx8(0x01);
    do {
        wr8(DP(0x0F), (u8)X);
        lda8(rd8(ABSX(0x01DE)));
        if (fz) goto next;
        lda8(rd8(ABSX(0x01CE)));
        ora8(rd8(ABSX(0x01D6)));
        ora8(rd8(ABSX(0x199E)));
        if (!fz) goto next;
        push16(0xAF55); CpuSteer_m1x1();
        lda8(rd8(ABSX(0x01EE)));
        if (!fz) {
            lda8(rd8(ABSX(0x08B4)));
            wr8(ABSX(0x0971), (u8)A);
            goto next;
        }
        push16(0xAF5D); CpuThrottle_m1x1();
        push16(0xAF60); CpuAvoidHurdle_m1x1();
        if (!fz) goto next;
        push8(0x80); push16(0xAF66); CheckLaneTile_m1x1();
        lda8(rd8(ABS(0x092A)));
        if (fz) goto next;
        push16(0xAF6E); RetargetRival_m1x1();
        lda8(rd8(DP(0x84)));
        if (!fz) goto next;
        push16(0xAF75); CpuLaneDecision_m1x1();
    next:
        X = rd8(DP(0x0F)); nz8((u8)X);
        X = inc8((u8)X);
        cmp8((u8)X, 0x08);
    } while (!fc);
    ret();
}

/* $80:BD4F (RTS): distances, then every rival slot 7-1 not on the track
 * whose distance $085E is ahead of the player's $9C by $170 or more, or
 * behind by up to $A0 (when the player is that far in), gets placed
 * (PlaceRival), unless $84. */
void SpawnRivals_m1x1(void)
{
    push16(0xBD51); UpdateRiderDistances_m1x1();
    ldy8(0x0E);
    ldx8(0x07);
    do {
        lda8(rd8(ABSX(0x01DE)));
        if (fz) {
            rep(0x20);
            lda16(rd16(DP(0x9C)));
            fc = 0; adc16(0x0170);
            cmp16(A, rd16(ABSY(0x085E)));
            int place = fc;
            if (!place) {
                lda16(rd16(DP(0x9C)));
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
                    push16(0xBD7E); PlaceRival_m1x1();
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

/* $80:FDDE (RTS, X = rider): the player crossing the finish once ($03AC,
 * not $84): $5E = $AC = $1D; with a finish result $093F sound $4A (1st)
 * or $28, the stadium bonus, $D4 = $80, a win counted ($1E87) without
 * rivals, and $5E cleared when $60; with none, $84 = 1 and sounds $2B,
 * $2D, $80, $5E cleared. */
void CrossFinish_m1x1(void)
{
    cmp8((u8)X, 0x00);
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x03AC)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    wr8(ABS(0x03AC), inc8(rd8(ABS(0x03AC))));
    lda8(0x1D); wr8(DP(0x5E), 0x1D); wr8(DP(0xAC), 0x1D);
    lda8(rd8(ABS(0x093F)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) { lda8(0x4A); push8(0x80); push16(0xFE02); QueueSoundB_m1x1(); }
        else { lda8(0x28); push8(0x80); push16(0xFE0A); QueueSoundB_m1x1(); }
        push8(0x80); push16(0xFE0E); AddStadiumBonus_m1x1();
        lda8(0x80); wr8(DP(0xD4), 0x80);
        lda8(rd8(DP(0x7D)));
        if (fz) wr8(ABS(0x1E87), inc8(rd8(ABS(0x1E87))));
        lda8(rd8(DP(0x60)));
        if (fz) { ret(); return; }
    } else {
        lda8(0x01); wr8(DP(0x84), 0x01);
        lda8(0x2B); push8(0x80); push16(0xFE28); QueueSoundB_m1x1();
        lda8(0x2D); push8(0x80); push16(0xFE2E); QueueSoundB_m1x1();
        lda8(0x80); push8(0x80); push16(0xFE34); QueueSoundB_m1x1();
    }
    lda8(0x00); wr8(DP(0x5E), 0x00);
    ret();
}

/* $82:84B2 (RTL): A = the byte at $82:84BA + A. X is kept. */
void ByteTable84BA_m1x1(void)
{
    push8((u8)X);
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONGX(0x8284BA)));
    X = pull8(); nz8((u8)X);
    retl();
}

/* $80:BF03 (RTS): when the ball is due ($092C) and rider 7 is off screen
 * (not page 1, page 0 left of $EA), it enters as rider 3 on page 2 via
 * PlaceRival, with $01C9 = 0, $01B9 = $08C7 = $92, and crashes into its
 * landing state. */
void LaunchBall_m1x1(void)
{
    lda8(rd8(ABS(0x092C)));
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x01E1)));
    ldx8(0x07);
    lda8(rd8(ABSX(0x019E)));
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    lda8(lsr8((u8)A));
    if (!fc) {
        lda8(rd8(ABSX(0x0196)));
        cmp8((u8)A, 0xEA);
        if (fc) { ret(); return; }
    }
    ldy8(0x00);
    wr8(ABS(0x092C), 0);
    Y = inc8((u8)Y);
    ldx8(0x02);
    wr8(ABS(0x01A1), 0x02);
    X = inc8((u8)X);
    push16(0xBF2B); PlaceRival_m1x1();
    if (fz) { ret(); return; }
    lda8(0x00); wr8(ABS(0x01C9), 0x00);
    lda8(0x92); wr8(ABS(0x01B9), 0x92); wr8(ABS(0x08C7), 0x92);
    landing_crash();
}

/* $80:D169 (RTL): StepObjectX for object X + 8. */
void StepObjectX8_m1x1(void)
{
    lda8((u8)X);
    fc = 0; adc8(0x08);
    X = (u8)A; nz8((u8)X);
    StepObjectX_m1x1();
}

/* $80:D145 (RTL, X = object): move object X and X + 8, count up its timer
 * $1518 and draw its marker; once the timer turns positive, clear $1530
 * and count $1541 up to 5. */
void MarkerObjectStep_m1x1(void)
{
    push8(0x80); push16(0xD148); StepObjectX_m1x1();
    push8((u8)X);
    push8(0x80); push16(0xD14D); StepObjectX8_m1x1();
    X = pull8(); nz8((u8)X);
    wr8(ABSX(0x1518), inc8(rd8(ABSX(0x1518))));
    push8(0x80); push16(0xD155); DrawRiderMarker_m1x1();
    lda8(rd8(ABSX(0x1518)));
    if (!fn) {
        wr8(ABSX(0x1530), 0);
        lda8(rd8(ABS(0x1541)));
        cmp8((u8)A, 0x05);
        if (!fc) wr8(ABS(0x1541), inc8(rd8(ABS(0x1541))));
    }
    retl();
}

/* $80:FF00 (RTS): QueueBonusDigits at VRAM $1B38, size $20. */
void QueueBonusDigitsAt_m1x1(void)
{
    lda8(0x38); wr8(DP(0x00), 0x38);
    lda8(0x1B); wr8(DP(0x01), 0x1B);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    QueueBonusDigits_m1x1();
}

void ShowCoinRank_m1x1(void);
void MarkerObjectStep_m1x1(void);
void ReadRiderPieces_m1x1(void);
void TrackRiderPace_m1x1(void);
void QueueBonusDigitsAt_m1x1(void);
void ByteTable84BA_m1x1(void);

/* $80:D12B (RTL): the coin rank, then each active marker object (even
 * slots 6-0, $1530) counts its word timer $1538 up and steps. */
void MarkerObjects_m1x1(void)
{
    push16(0xD12D); ShowCoinRank_m1x1();
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x1530)));
        if (!fz) {
            rep(0x20);
            wr16(ABSX(0x1538), inc16(rd16(ABSX(0x1538))));
            sep(0x20);
            push8(0x80); push16(0xD13F); MarkerObjectStep_m1x1();
        }
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    retl();
}

/* $80:B208 (RTS): $7F = frame & 7, the riders' pieces, and in the race
 * modes (below 9, no fade $1923, not $0CD7) the pace, score and bonus
 * digits. */
void RaceFrameStep_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x07);
    wr8(DP(0x7F), (u8)A);
    push16(0xB210); ReadRiderPieces_m1x1();
    lda8(rd8(ABS(0x0CD7)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x1923)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x76)));
    cmp8((u8)A, 0x09);
    if (fc) { ret(); return; }
    push16(0xB223); TrackRiderPace_m1x1();
    push16(0xB226); AddScore_m1x1();
    push16(0xB229); QueueBonusDigitsAt_m1x1();
    ret();
}

/* $82:8424 (RTL): the five digits at [$18] (leading zeros as $0A) and a
 * trailing 0 through the font table, +$10, queued from $03CF (size $16)
 * at VRAM $00. X and Y are kept. */
void QueueFontDigits_m1x1(void)
{
    push8((u8)X); push8((u8)Y);
    ldx8(0x00); ldy8(0x00);
    do {
        lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
        int blank = 0;
        if (fz) { cmp8((u8)X, 0x00); blank = fz; }
        if (blank) lda8(0x0A);
        else X = inc8((u8)X);
        push8(0x82); push16(0x843A); ByteTable84BA_m1x1();
        fc = 0; adc8(0x10);
        wr8(ABSY(0x03D0), (u8)A);
        Y = inc8((u8)Y);
        cmp8((u8)Y, 0x05);
    } while (!fc);
    lda8(0x00);
    push8(0x82); push16(0x844B); ByteTable84BA_m1x1();
    fc = 0; adc8(0x10);
    wr8(ABS(0x03D5), (u8)A);
    lda8(0x16); wr8(ABS(0x03CF), 0x16);
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x10); wr8(DP(0x0F), 0x10);
    push8(0x82); push16(0x8467); VramQueueTilesLong_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}

/* $81:9828 (RTS): an empty step. */
void EmptyStep9828_m1x1(void)
{
    ret();
}

/* $81:9788 SpawnObject (RTL, Y = type, $00 / $02 = X / Y position): take
 * the first free object slot (even 14-0, $02EF): type $02FF, active,
 * script $81:AD67, position, and its timers, frames and velocities
 * cleared; then the type's init routine ($81:AD7B, $0E = slot). C set
 * when no slot is free. X is kept. */
void SpawnObject_m1x1(void)
{
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    ldx8(0x0E);
    for (;;) {
        lda8(rd8(ABSX(0x02EF)));
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
    wr8(ABSX(0x02FF), (u8)A);
    lda8(0x01); wr8(ABSX(0x02EF), 0x01);
    rep(0x20);
    lda16(Y);
    lda16(asl16(A));
    Y = (u8)A; nz8((u8)Y);
    lda16(rd16(ABSY(0xAD67))); wr16(ABSX(0x038F), A);
    lda16(rd16(DP(0x00))); wr16(ABSX(0x027F), A);
    lda16(rd16(DP(0x02))); wr16(ABSX(0x028F), A);
    static const u16 clr[11] = { 0x031F, 0x032F, 0x036F, 0x037F, 0x030F, 0x035F,
                                 0x034F, 0x029F, 0x02AF, 0x02BF, 0x02CF };
    for (int i = 0; i < 11; i++) wr16(ABSX(clr[i]), 0);
    sep(0x20);
    wr8(DP(0x0E), (u8)X);
    X = Y; nz8((u8)X);
    push16(0x97E3);
    r816_call_pc(0x810000u | rd16(LONGX(0x81AD7B)));
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    fc = 0;
    retl();
}

/* $81:9829 FindObjectSlot (RTL, A = type): Y = the slot (even 14-0) of
 * the object of type A, or negative when none. */
void FindObjectSlot_m1x1(void)
{
    ldy8(0x0E);
    do {
        cmp8((u8)A, rd8(ABSY(0x02FF)));
        if (fz) break;
        Y = dec8((u8)Y);
        Y = dec8((u8)Y);
    } while (!fn);
    retl();
}

/* $82:8259 (RTL): the three bytes at [$18] (minutes, seconds, hundredths)
 * as a time "M'SS"HH" in two rows ($03D0-$03D7, lower row +$10; a
 * leading zero minute tens blank $0C, the marks $0A / $0B), size $28,
 * queued at VRAM $00. X and Y are kept. */
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
    wr8(ABS(0x03D1), (u8)A);
    ora8(0x10); wr8(ABS(0x03D9), (u8)A);
    lda8((u8)X);
    cmp8((u8)A, 0x00);
    if (fz) lda8(0x0C);
    wr8(ABS(0x03D0), (u8)A);
    ora8(0x10); wr8(ABS(0x03D8), (u8)A);
    static const u16 lo[2] = { 0x03D4, 0x03D7 };
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
    lda8(0x0A); wr8(ABS(0x03D2), 0x0A);
    lda8(inc8((u8)A)); wr8(ABS(0x03D5), (u8)A);
    lda8(0x1A); wr8(ABS(0x03DA), 0x1A);
    lda8(inc8((u8)A)); wr8(ABS(0x03DD), (u8)A);
    lda8(0x28); wr8(ABS(0x03CF), 0x28);
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x01); wr8(DP(0x0F), 0x01);
    push8(0x82); push16(0x82E6); VramQueueTilesLong_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    retl();
}

void QueueTimeDigits_m1x1(void);
void SpawnObject_m1x1(void);
void FindObjectSlot_m1x1(void);

/* $80:98DE (RTS): the results list, one line every $20 frames ($88): the
 * time of entry $044F (3 bytes at $083D) at its VRAM row ($80:993F),
 * sound $42; after five, the next end-screen steps ($0450 = 4 and
 * $0451 = 1, or 2 when $085C is set). */
void EndScreenTimes_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (!fz) { ret(); return; }
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(rd8(ABS(0x044F)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    fc = 0; adc8(rd8(ABS(0x044F)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0x083D))); wr8(ABS(0x03AE), (u8)A);
    lda8(rd8(ABSY(0x083E))); wr8(ABS(0x03AF), (u8)A);
    lda8(rd8(ABSY(0x083F))); wr8(ABS(0x03B0), (u8)A);
    rep(0x20);
    lda16(rd16(ABSX(0x993F))); wr16(DP(0x00), A);
    lda16(0x03AE); wr16(DP(0x18), 0x03AE);
    sep(0x20);
    lda8(0x00); wr8(DP(0x1A), 0x00);
    push8(0x80); push16(0x9917); QueueTimeDigits_m1x1();
    lda8(0x42);
    push8(0x80); push16(0x991D); QueueSoundB_m1x1();
    wr8(ABS(0x044F), inc8(rd8(ABS(0x044F))));
    lda8(rd8(ABS(0x044F)));
    cmp8((u8)A, 0x05);
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(0x085C)));
    if (fz) {
        lda8(0x04); wr8(ABS(0x0450), 0x04);
        lda8(0x01); wr8(ABS(0x0451), 0x01);
    } else {
        lda8(0x02); wr8(ABS(0x0450), 0x02);
    }
    ret();
}

/* $81:C018 / $81:C085: object $03A3's frame sequence. When its timer
 * $036F runs out it steps $031F (timer from the table at times), and on
 * frame `spawn` spawns a type-8 object (state 1, $032F = 4) at its
 * position less ($16, dy); past the last frame it goes to state 4 with
 * a timer of 8. Its frame $034F comes from the table at frames. */
static void frame_sequence(u8 count, u16 times, u8 spawn, u16 dy,
                           u16 frames, u16 back)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABSX(0x036F)));
    if (fz) {
        lda8(rd8(ABSX(0x031F)));
        lda8(inc8((u8)A));
        cmp8((u8)A, count);
        if (fc) {
            lda8(0x04); wr8(ABSX(0x030F), 0x04);
            lda8(0x08); wr8(ABSX(0x036F), 0x08);
            wr8(ABSX(0x031F), 0);
            ret();
            return;
        }
        wr8(ABSX(0x031F), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABSY(times)));
        wr8(ABSX(0x036F), (u8)A);
        lda8(rd8(ABSX(0x031F)));
        cmp8((u8)A, spawn);
        if (fz) {
            push8((u8)Y);
            rep(0x20);
            lda16(rd16(ABSX(0x027F)));
            fc = 1; sbc16(0x0016);
            wr16(DP(0x00), A);
            lda16(rd16(ABSX(0x028F)));
            fc = 1; sbc16(dy);
            wr16(DP(0x02), A);
            sep(0x20);
            ldy8(0x08);
            push8(0x81); push16(back); SpawnObject_m1x1();
            Y = rd8(DP(0x0E)); nz8((u8)Y);
            lda8(0x01); wr8(ABSY(0x031F), 0x01);
            lda8(0x04); wr8(ABSY(0x032F), 0x04);
            Y = pull8(); nz8((u8)Y);
        }
    }
    Y = rd8(ABSX(0x031F)); nz8((u8)Y);
    lda8(rd8(ABSY(frames)));
    wr8(ABSX(0x034F), (u8)A);
    ret();
}

void FrameSequenceA_m1x1(void) { frame_sequence(0x04, 0xC081, 0x02, 0x001C, 0xC07D, 0xC055); }
void FrameSequenceB_m1x1(void) { frame_sequence(0x06, 0xC0F0, 0x03, 0x0018, 0xC0EA, 0xC0C2); }

/* $81:B96C (RTS): object $03A3 drifts every 4 frames, its X by the word
 * $031F (reversing every 8 steps, $032F) and its Y up; above row $48 it
 * parks at ($F0, $F0), steps its state, removes the type-9 and type-0
 * objects and waits $80-$FF frames. */
void DriftUpObject_m1x1(void)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (fz) {
        rep(0x20);
        lda16(rd16(ABSX(0x031F)));
        fc = 0; adc16(rd16(ABSX(0x027F)));
        wr16(ABSX(0x027F), A);
        wr16(ABSX(0x032F), dec16(rd16(ABSX(0x032F))));
        if (fz) {
            lda16(0x0008); wr16(ABSX(0x032F), 0x0008);
            lda16(rd16(ABSX(0x031F)));
            eor16(0xFFFF);
            lda16(inc16(A));
            wr16(ABSX(0x031F), A);
        }
    }
    sep(0x20);
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if (!fz) { ret(); return; }
    wr8(ABSX(0x028F), dec8(rd8(ABSX(0x028F))));
    lda8(rd8(ABSX(0x028F)));
    cmp8((u8)A, 0x48);
    if (fc) { ret(); return; }
    rep(0x20);
    lda16(0x00F0);
    wr16(ABSX(0x027F), 0x00F0);
    wr16(ABSX(0x028F), 0x00F0);
    sep(0x20);
    wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
    lda8(0x09);
    push8(0x81); push16(0xB9BD); FindObjectSlot_m1x1();
    lda8(0x00); wr8(ABSY(0x02EF), 0x00);
    lda8(0x00);
    push8(0x81); push16(0xB9C8); FindObjectSlot_m1x1();
    lda8(0x00); wr8(ABSY(0x02EF), 0x00);
    push8(0x81); push16(0xB9D1); RandomB_m1x1();
    and8(0x7F);
    fc = 0; adc8(0x80);
    wr8(ABSX(0x036F), (u8)A);
    ret();
}

/* $81:B9DB (RTS): object $03A3 releases balloons: sound $2B once at the
 * start (before place 4, $086E), then on odd-high frames of the counter
 * $8A whenever its timer $036F is out: count ($033F), wait $20-$5F
 * frames and spawn a type-9 object at X $40-$BF, Y $D0 with sound $48
 * and a random rise $B6-$C5 ($02AF). From place 4 on, only until the
 * first one. */
void ReleaseBalloons_m1x1(void)
{
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    lda8(rd8(ABS(0x086E)));
    cmp8((u8)A, 0x04);
    if (fc) {
        lda8(rd8(ABSX(0x033F)));
        if (!fz) { ret(); return; }
    } else {
        lda8(rd8(DP(0x8A)));
        if (fz) {
            lda8(rd8(ABSX(0x033F)));
            if (fz) {
                lda8(0x2B);
                push8(0x81); push16(0xB9FA); QueueSoundB_m1x1();
            }
        }
    }
    rep(0x20);
    wr16(DP(0x8A), inc16(rd16(DP(0x8A))));
    sep(0x20);
    lda8(rd8(DP(0x8B)));
    and8(0x01);
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x036F)));
    if (!fz) { ret(); return; }
    wr8(ABSX(0x033F), inc8(rd8(ABSX(0x033F))));
    push8(0x81); push16(0xBA12); RandomB_m1x1();
    and8(0x3F);
    fc = 0; adc8(0x20);
    wr8(ABSX(0x036F), (u8)A);
    ldy8(0x09);
    push8(0x81); push16(0xBA20); SpawnObject_m1x1();
    if (fc) { ret(); return; }
    X = rd8(DP(0x0E)); nz8((u8)X);
    push8(0x81); push16(0xBA28); RandomB_m1x1();
    and8(0x7F);
    fc = 0; adc8(0x40);
    wr8(ABSX(0x027F), (u8)A);
    lda8(0xD0); wr8(ABSX(0x028F), 0xD0);
    lda8(0x48);
    push8(0x81); push16(0xBA3B); QueueSoundB_m1x1();
    push8(0x81); push16(0xBA3F); RandomB_m1x1();
    and8(0x0F);
    fc = 0; adc8(0xB6);
    wr8(ABSX(0x02AF), (u8)A);
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    ret();
}

void MoveObjectX_m1x1(void);

/* $80:997E (RTS): the end screen's record entry. After the $88 delay the
 * box (2 rows of $20 from $7E:5800) is queued at $0300; then a press of
 * A ($26 bit 7) gives step $0450 = 1, $88 = $20, the next box ($7E:5980)
 * and the record time at $11:572C at VRAM $09D3 with sound $43. */
void EndScreenRecord_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) {
        u8 t = dec8(rd8(DP(0x88)));
        wr8(DP(0x88), t);
        if (!fz) { ret(); return; }
        lda8(0x00); wr8(DP(0x02), 0x00);
        lda8(0x58); wr8(DP(0x03), 0x58);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        lda8(0x20); wr8(DP(0x10), 0x20);
        lda8(0x02); wr8(DP(0x11), 0x02);
        lda8(0x00); wr8(DP(0x00), 0x00);
        lda8(0x03); wr8(DP(0x01), 0x03);
        push16(0x99A4); QueueTileRows_m1x1();
    }
    lda8(rd8(DP(0x26)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(0x01); wr8(ABS(0x0450), 0x01);
    lda8(0x20); wr8(DP(0x88), 0x20);
    lda8(0x80); wr8(DP(0x02), 0x80);
    lda8(0x59); wr8(DP(0x03), 0x59);
    lda8(0x7E); wr8(DP(0x04), 0x7E);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x99D2); QueueTileRows_m1x1();
    rep(0x20);
    lda16(0x09D3); wr16(DP(0x00), 0x09D3);
    lda16(0x572C); wr16(DP(0x18), 0x572C);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x99E8); QueueTimeDigits_m1x1();
    lda8(0x43);
    push8(0x80); push16(0x99EE); QueueSoundB_m1x1();
    ret();
}

/* $81:973F (RTS): MoveObjectX for object X + 8. */
void MoveObjectX8_m1x1(void)
{
    lda8((u8)X);
    fc = 0; adc8(0x08);
    X = (u8)A; nz8((u8)X);
    MoveObjectX_m1x1();
}

void MoveObjectX8_m1x1(void);
void SpeedGauge_m1x1(void);

/* $81:9604 (RTS, Y = object): a falling confetti piece: its frame $011E
 * cycles every 4 frames ($0126); it moves (X and X + 8), its sprite
 * ($1610 + 2Y: position $0106 / $010E, tile and attribute from
 * $81:966F / $81:9673 by frame, frame 0 from timer $F0 on, size 2),
 * and when its timer $0146 wraps it goes to state 3 with a $20 wait. */
void ConfettiStep_m1x1(void)
{
    X = Y; nz8((u8)X);
    wr8(ABSX(0x0126), dec8(rd8(ABSX(0x0126))));
    if (fz) {
        lda8(rd8(ABSX(0x011E)));
        lda8(inc8((u8)A));
        and8(0x03);
        wr8(ABSX(0x011E), (u8)A);
        lda8(0x04); wr8(ABSX(0x0126), 0x04);
    }
    push8(0x81); push16(0x961B); MoveObjectX_m1x1();
    push8((u8)X);
    push8(0x81); push16(0x9620); MoveObjectX8_m1x1();
    X = pull8(); nz8((u8)X);
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
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
    lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02); wr8(ABSY(0x1824), 0x02);
    wr8(ABSX(0x0146), inc8(rd8(ABSX(0x0146))));
    lda8(rd8(ABSX(0x0146)));
    if (fz) {
        lda8(0x03); wr8(ABSX(0x012E), 0x03);
        lda8(0x20); wr8(ABSX(0x0126), 0x20);
        wr8(ABSX(0x011E), 0);
    }
    ret();
}

/* $81:9554 (RTS): an empty confetti state. */
void EmptyStep9554_m1x1(void)
{
    ret();
}

/* $80:CA89 (RTL): DrawAllRiders with DB = $80. */
void DrawAllRidersLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xCA90); DrawAllRiders_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:F15B (RTL, 16-bit A): BuildScoreDigits for other banks. */
void BuildScoreDigitsLong_m0x1(void)
{
    push16(0xF15D); BuildScoreDigits_m0x1();
    retl();
}

/* $80:C38B (RTS): riders 7-1 on screen page 1 ($019E odd) at X $30-$3F
 * flag $092D (cleared elsewhere on that page); Y is left $40 or $80. */
void FlagRidersAtLine_m1x1(void)
{
    ldx8(0x08);
    for (;;) {
        X = dec8((u8)X);
        if (fz) break;
        lda8(rd8(ABSX(0x019E)));
        lda8(lsr8((u8)A));
        if (!fc) continue;
        ldy8(0x40);
        lda8(rd8(ABSX(0x0196)));
        cmp8((u8)A, 0x30);
        if (fc) {
            ldy8(0x80);
            cmp8((u8)A, 0x40);
            if (!fc) { lda8(0x01); wr8(ABSX(0x092D), 0x01); continue; }
        }
        lda8(rd8(ABSX(0x092D)));
        if (fz) continue;
        lda8((u8)Y);
        lda8(0x00);
        wr8(ABSX(0x092D), 0x00);
    }
    ret();
}

/* $80:AB46 (RTS): on even frames the ball check; on frames 1 mod 4 the
 * line flags and the speed gauge. */
void BallOrGauge_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    lda8(lsr8((u8)A));
    if (!fc) {
        push16(0xAB4D); BallHit_m1x1();
        ret();
        return;
    }
    lda8(lsr8((u8)A));
    if (fc) { ret(); return; }
    push16(0xAB54); FlagRidersAtLine_m1x1();
    SpeedGauge_m1x1();
}

void ClearWords1530_m1x1(void);
void DrawAllRidersLong_m1x1(void);
void ScaleJumpCount_m1x1(void);
void BuildScoreDigitsLong_m0x1(void);
void QueueResultsTiles_m1x1(void);

/* $80:D21C (RTS): clear the coin counts $1540-$1542, show the coin rank,
 * then clear the marker objects (ClearWords1530, into which it falls). */
void ResetCoinCounts_m1x1(void)
{
    wr8(ABS(0x1540), 0); wr8(ABS(0x1541), 0); wr8(ABS(0x1542), 0);
    push16(0xD227); ShowCoinRank_m1x1();
    ClearWords1530_m1x1();
}

/* $81:C84F (RTL): the jump result's total: the waving player, and when
 * $8C runs out sound $42, the next step ($CD), $8C = $20, and the total
 * (ScaleJumpCount's $0C + the rank points $80:F155) built and queued
 * (QueueResultsTiles, into which it falls). */
void JumpResultTotal_m1x1(void)
{
    ldx8(0x00);
    push8(0x81); push16(0xC854); DrawAllRidersLong_m1x1();
    lda8(0x19); wr8(ABS(0x1961), 0x19);
    u8 t = dec8(rd8(DP(0x8C)));
    wr8(DP(0x8C), t);
    if (!fz) { retl(); return; }
    lda8(0x42);
    push8(0x81); push16(0xC863); QueueSoundB_m1x1();
    wr8(DP(0xCD), inc8(rd8(DP(0xCD))));
    lda8(0x20); wr8(DP(0x8C), 0x20);
    push8(0x81); push16(0xC86D); ScaleJumpCount_m1x1();
    rep(0x20);
    X = rd8(ABS(0x156B)); nz8((u8)X);
    lda16(rd16(LONGX(0x80F155)));
    and16(0x00FF);
    fc = 0; adc16(rd16(DP(0x0C)));
    wr16(DP(0x0C), A);
    push8(0x81); push16(0xC882); BuildScoreDigitsLong_m0x1();
    QueueResultsTiles_m1x1();
}

/* $81:9538 (RTL): each confetti piece (even slots 6-0) runs its state
 * $0116 through the table $81:954C (Y = slot). X is kept. */
void ConfettiAll_m1x1(void)
{
    push8((u8)X);
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x0116)));
        lda8(asl8((u8)A));
        push8((u8)X);
        Y = X; nz8((u8)Y);
        X = (u8)A; nz8((u8)X);
        push16(0x9544);
        r816_call_pc(0x810000u | rd16(LONGX(0x81954C)));
        X = pull8(); nz8((u8)X);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    X = pull8(); nz8((u8)X);
    retl();
}

/* $80:DBFC / $80:EEBB (two copies; b = the second, which also silences
 * APU port 2 and clears $01C6 on the get-up): the crashed player
 * ($01CE). Still tumbling ($0999): on the ground, stopped and level
 * (angle 6, $0176 clear) it gets up; otherwise it slides at $88 and,
 * once $3B is clear, sets $3B = 2 and turns its lean (CrashRecovery's
 * loop from $80:C279). Stopped: the speed is cleared and the timer $3A
 * starts at $20; below $1C it slides, and at 1 the tumble ends: $0999 =
 * 1, $09C4 = 0 and (unless $0176) the player halts and crash-lands. */
static void player_crash_step(int b)
{
    u16 base = b ? 0xEF37 : 0xDC70;
    lda8(rd8(ABS(0x01CE)));
    if (fz) { ret(); return; }
    lda8(0x00); wr8(ABS(0x092D), 0x00);
    lda8(rd8(DP(0x7F)));
    if (fz) lda8(0x01);
    lda8(rd8(ABS(0x0999)));
    if (!fz) {
        lda8(rd8(ABS(0x01EE)));
        if (fz) {
            lda8(rd8(ABS(0x01C6)));
            if (fz) {
                lda8(rd8(ABS(0x0176)));
                int up = 0;
                if (fz) {
                    lda8(rd8(ABS(0x01E6)));
                    cmp8((u8)A, 0x06);
                    up = fz;
                }
                if (up) {
                    if (b) { lda8(0x00); wr8(ABS(0x2142), 0x00); }
                    lda8(0x00);
                    wr8(ABS(0x01BE), 0);
                    if (b) wr8(ABS(0x01C6), 0);
                    wr8(ABS(0x01CE), 0);
                    wr8(ABS(0x0999), 0);
                    ret();
                    return;
                }
                lda8(0x88); wr8(ABS(0x01BE), 0x88);
            }
        }
        goto slide;
    }
    lda8(0x00);
    wr8(ABS(0x01BE), 0); wr8(ABS(0x01C6), 0); wr8(ABS(0x0186), 0);
    lda8(rd8(DP(0x3A)));
    if (fz) { lda8(0x20); wr8(DP(0x3A), 0x20); }
    cmp8((u8)A, 0x1C);
    if (fc) { ret(); return; }
    cmp8((u8)A, 0x01);
    if (!fz) goto slide;
    wr8(ABS(0x0999), (u8)A);
    lda8(lsr8((u8)A));
    wr8(ABS(0x09C4), (u8)A);
    lda8(rd8(ABS(0x0176)));
    if (!fz) { ret(); return; }
    push16(b ? 0xEF33 : 0xDC6C); player_halt();
    player_crash_tail(base);
    return;
slide:
    lda8(rd8(DP(0x3B)));
    if (!fz) { ret(); return; }
    lda8(0x02); wr8(DP(0x3B), 0x02);
    crash_loop(1);
}

void PlayerCrashStep_m1x1(void) { player_crash_step(0); }
void PlayerCrashStepB_m1x1(void) { player_crash_step(1); }

/* $80:BB82 (RTS, X = rider): a rider walking to its start lane steers
 * toward $80:BACF (by rider; lane 4's entry for a CPU rider in the air);
 * on arrival it stops steering, $091C = 0 and steps $01D6; the player
 * also clears $0CDF / $0CE0 ($0990 too when $0CE0 was set), resets the
 * coin counts and plays sound $80. Then $45,X = 8. */
void WalkToStartLane_m1x1(void)
{
    lda8((u8)X);
    Y = (u8)A; nz8((u8)Y);
    if (!fz) {
        lda8(rd8(ABSX(0x01FE)));
        if (!fz) ldy8(0x04);
    }
    lda8(rd8(ABSX(0x01F6)));
    cmp8((u8)A, rd8(ABSY(0xBACF)));
    if (!fz) {
        ldy8(0x01);
        if (fc) ldy8(0xFF);
        lda8((u8)Y);
        wr8(ABSX(0x01AE), (u8)A);
        ret();
        return;
    }
    lda8(0x00);
    wr8(ABSX(0x091C), 0);
    wr8(ABSX(0x01AE), 0);
    wr8(ABSX(0x01D6), inc8(rd8(ABSX(0x01D6))));
    lda8((u8)X);
    if (fz) {
        Y = rd8(ABS(0x0CE0)); nz8((u8)Y);
        if (!fz) wr8(ABS(0x0990), (u8)A);
        wr8(ABS(0x0CE0), (u8)A);
        wr8(ABS(0x0CDF), (u8)A);
        push8(0x80); push16(0xBBC1); ResetCoinCounts_m1x1();
        lda8(0x80);
        push8(0x80); push16(0xBBC7); QueueSoundB_m1x1();
        lda8(0x02);
    }
    lda8(0x08);
    wr8(DPX(0x45), 0x08);
    ret();
}

/* $80:C57E (RTS): the race clock at VRAM $1897, counting $03B0 up unless
 * the race is over ($84); then TickRaceClock (into which it falls). */
void RaceClockStep_m1x1(void)
{
    lda8(0x97); wr8(DP(0x00), 0x97);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(rd8(DP(0x84)));
    if (fz) wr8(ABS(0x03B0), inc8(rd8(ABS(0x03B0))));
    TickRaceClock_m1x1();
}

/* $82:82EA (RTL): as QueueTimeDigits, through the font table
 * (ByteTable84BA, lower row +$20, a leading zero minute $0A, the marks
 * $56 / $58), size $28 from $03CF with $0F = $10. X and Y are kept. */
void QueueFontTime_m1x1(void)
{
    push8(DB);
    lda8(0x82); push8(0x82);
    DB = pull8(); nz8(DB);
    push8((u8)X); push8((u8)Y);
    ldx8(0x00); ldy8(0x00);
    lda8(rd8((ILDP(0x18) + Y) & 0xFFFFFF));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        X = inc8((u8)X);
    }
    push8(0x82); push16(0x8303); ByteTable84BA_m1x1();
    wr8(ABS(0x03D1), (u8)A);
    fc = 0; adc8(0x20); wr8(ABS(0x03D9), (u8)A);
    lda8((u8)X);
    cmp8((u8)A, 0x00);
    if (fz) lda8(0x0A);
    push8(0x82); push16(0x8317); ByteTable84BA_m1x1();
    wr8(ABS(0x03D0), (u8)A);
    fc = 0; adc8(0x20); wr8(ABS(0x03D8), (u8)A);
    static const u16 lo[2] = { 0x03D4, 0x03D7 };
    static const u16 back[2][2] = { { 0x8332, 0x8340 }, { 0x835B, 0x8369 } };
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
        push8(0x82); push16(back[i][0]); ByteTable84BA_m1x1();
        wr8(ABS(lo[i]), (u8)A);
        fc = 0; adc8(0x20); wr8(ABS(lo[i] + 8), (u8)A);
        lda8((u8)X);
        push8(0x82); push16(back[i][1]); ByteTable84BA_m1x1();
        wr8(ABS(lo[i] - 1), (u8)A);
        fc = 0; adc8(0x20); wr8(ABS(lo[i] + 7), (u8)A);
    }
    lda8(0x56); wr8(ABS(0x03D2), 0x56);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABS(0x03D5), (u8)A);
    lda8(0x76); wr8(ABS(0x03DA), 0x76);
    lda8(inc8((u8)A)); lda8(inc8((u8)A)); wr8(ABS(0x03DD), (u8)A);
    lda8(0x28); wr8(ABS(0x03CF), 0x28);
    rep(0x20);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x10); wr8(DP(0x0F), 0x10);
    push8(0x82); push16(0x839C); VramQueueTilesLong_m1x1();
    Y = pull8(); nz8((u8)Y);
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}

void QueueResultsCount_m1x1(void);
void QueueNameplate_m1x1(void);

/* $80:DDEF (RTS): the bonus result count-up. A button skips to the final
 * count ($09C6 = $09C5, the nameplate when it is a full score $09C9) and
 * straight on to the result. Otherwise every 4 frames ($88) the shown
 * count $09C6 steps up with sound $44; at the end, next step ($77), a
 * $40 wait and for a full score the banner $80:DE4F at VRAM $18F4. */
void BonusCountUp_m1x1(void)
{
    lda8(rd8(DP(0x21)));
    ora8(rd8(DP(0x26)));
    if (!fz) {
        lda8(rd8(ABS(0x09C5)));
        wr8(ABS(0x09C6), (u8)A);
        wr8(DP(0x10), (u8)A);
        push16(0xDDFF); QueueResultsCount_m1x1();
        wr8(DP(0x77), inc8(rd8(DP(0x77))));
        lda8(rd8(ABS(0x09C5)));
        cmp8((u8)A, rd8(ABS(0x09C9)));
        if (fz) { push16(0xDE0C); QueueNameplate_m1x1(); }
        bonus_result_body();
        return;
    }
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
        push8(0x80); push16(0xDE27); QueueSoundB_m1x1();
        lda8(0x04); wr8(DP(0x88), 0x04);
        QueueResultsCount_m1x1();
        return;
    }
    wr8(DP(0x77), inc8(rd8(DP(0x77))));
    lda8(0x40); wr8(DP(0x88), 0x40);
    lda8(rd8(ABS(0x09C5)));
    cmp8((u8)A, rd8(ABS(0x09C9)));
    if (!fz) { ret(); return; }
    rep(0x20);
    lda16(0x18F4); wr16(DP(0x00), 0x18F4);
    lda16(0xDE4F); wr16(DP(0x02), 0xDE4F);
    sep(0x20);
    VramQueueTiles20_m1x1();
}

void QueuePlaceBadge_m1x1(void);
void VramQueueTiles_m1x1(void);

/* $80:96B1: add the result points to the score, sound $43, and the score
 * ($0CE5, six digits) at VRAM $1B16. */
static void results_score_tail(void)
{
    push16(0x96B3); AddScore_m1x1();
    lda8(0x43);
    push8(0x80); push16(0x96B9); QueueSoundB_m1x1();
    rep(0x20);
    lda16(0x0CE5); wr16(DP(0x02), 0x0CE5);
    lda16(0x1B16); wr16(DP(0x00), 0x1B16);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    ldy8(0x05);
    push8(0x80); push16(0x96D1); QueueDigitsAt_m1x1();
    ret();
}

/* $80:9608 (RTS, 16-bit A on entry): the race results' place line: the
 * place badge at VRAM $1A08 (or the "no time" text $80:9462 at $1A04
 * when $085E is 0), then the place points for place $086E ($80:944A /
 * $80:9452 digits, $BA from $80:945A; a leading zero blank, from 8th
 * place all blank) at $1A17, added into $09AD / $09AE and the score. */
void ResultsPlacePoints_m0x1(void)
{
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    lda8(rd8(ABS(0x085E)));
    ora8(rd8(ABS(0x085F)));
    if (!fz) {
        rep(0x20);
        lda16(0x1A08); wr16(DP(0x00), 0x1A08);
        sep(0x20);
        push16(0x9621); QueuePlaceBadge_m1x1();
    } else {
        rep(0x20);
        lda16(0x1A04); wr16(DP(0x00), 0x1A04);
        lda16(0x9462); wr16(DP(0x02), 0x9462);
        sep(0x20);
        push16(0x9634); VramQueueTiles_m1x1();
    }
    X = rd8(ABS(0x086E)); nz8((u8)X);
    X = dec8((u8)X);
    lda8(rd8(ABSX(0x944A))); wr8(ABS(0x03D0), (u8)A);
    ora8(0x10); wr8(ABS(0x03D5), (u8)A);
    lda8(rd8(ABSX(0x9452))); wr8(ABS(0x03D1), (u8)A);
    ora8(0x10); wr8(ABS(0x03D6), (u8)A);
    lda8(rd8(ABSX(0x945A))); wr8(DP(0xBA), (u8)A);
    lda8(0x25); wr8(ABS(0x03CF), 0x25);
    wr8(ABS(0x03D2), 0); wr8(ABS(0x03D3), 0); wr8(ABS(0x03D4), 0);
    lda8(0x10);
    wr8(ABS(0x03D7), 0x10); wr8(ABS(0x03D8), 0x10); wr8(ABS(0x03D9), 0x10);
    lda8(rd8(ABSX(0x944A)));
    if (fz) {
        lda8(0xFF);
        wr8(ABS(0x03D0), 0xFF); wr8(ABS(0x03D5), 0xFF);
        cmp8((u8)X, 0x07);
        if (fc) {
            wr8(ABS(0x03D1), 0xFF); wr8(ABS(0x03D6), 0xFF);
            wr8(ABS(0x03D2), 0xFF); wr8(ABS(0x03D7), 0xFF);
            wr8(ABS(0x03D3), 0xFF); wr8(ABS(0x03D8), 0xFF);
        }
    }
    rep(0x20);
    lda16(0x1A17); wr16(DP(0x00), 0x1A17);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0x96A0); VramQueueTiles20_m1x1();
    lda8(rd8(ABS(0x03D0)));
    if (!fn) wr8(ABS(0x09AD), (u8)A);
    lda8(rd8(ABS(0x03D1)));
    if (!fn) wr8(ABS(0x09AE), (u8)A);
    results_score_tail();
}

/* $80:96E0 (RTS, 16-bit A): the jump results' score line: the score
 * $156D at VRAM $1AB6, its digits 2-5 into $09AD-$09B0 (blanks skipped),
 * then into the score as above. */
void ResultsJumpPoints_m0x1(void)
{
    lda16(rd16(ABS(0x156D)));
    push16(0x96E5); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x1AB6); wr16(DP(0x00), 0x1AB6);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    push16(0x96F6); VramQueueTiles20_m1x1();
    rep(0x20);
    lda16(rd16(ABS(0x156D)));
    push16(0x96FE); BuildScoreDigits_m0x1();
    sep(0x20);
    ldx8(0x02);
    do {
        lda8(rd8(ABSX(0x03CF)));
        if (!fn) wr8(ABSX(0x09AB), (u8)A);
        X = inc8((u8)X);
        cmp8((u8)X, 0x06);
    } while (!fc);
    results_score_tail();
}

void RaceClockStep_m1x1(void);
void BrakeRider_m1x1(void);

/* $80:C3E1 (RTS): while racing ($92) the race time runs: frames $82 (to
 * 60), seconds $81 (to 60) and minutes $80, with the hundredths
 * ($80:C470 -> two digits $80:C4AC at $09B8) and seconds digits ($09B5)
 * and minutes ($09B3) copied to $09BB-$09C1, and the clock shown. At
 * the broadcast's end minute for this race ($80:C468, entry $7C * 2 +
 * $7D; past frame $0708 of it, $CA = 1) or after 4 minutes the race
 * ends: $085E cleared, $84 = 2, sound $2D. */
void RaceTimer_m1x1(void)
{
    lda8(rd8(DP(0x92)));
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
    lda16(rd16(ABSX(0xC470)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC4AC))); wr16(ABS(0x09B8), A);
    lda16(rd16(DP(0x81)));
    lda16(asl16(A));
    X = (u8)A; nz8((u8)X);
    lda16(rd16(ABSX(0xC4AC))); wr16(ABS(0x09B5), A);
    sep(0x20);
    lda8(rd8(DP(0x80))); wr8(ABS(0x09B3), (u8)A);
    ldx8(0x06);
    do {
        lda8(rd8(ABSX(0x09B3)));
        wr8(ABSX(0x09BB), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    push16(0xC42A); RaceClockStep_m1x1();
    push8((u8)X);
    lda8(rd8(DP(0x7C)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x7D)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xC468)));
    X = pull8(); nz8((u8)X);
    cmp8((u8)A, rd8(LONG(0x7FFFFF)));
    if (fc) {
        if (!fz) goto check_minutes;
        rep(0x20);
        lda16(rd16(LONG(0x7FFFFA)));
        cmp16(A, 0x0708);
        sep(0x20);
        if (fc) { lda8(0x01); wr8(ABS(0x00CA), 0x01); }
    check_minutes:
        lda8(rd8(DP(0x80)));
        cmp8((u8)A, 0x04);
        if (!fc) { ret(); return; }
    }
    wr8(ABS(0x085E), 0); wr8(ABS(0x085F), 0);
    lda8(0x02); wr8(DP(0x84), 0x02);
    lda8(0x2D);
    push8(0x80); push16(0xC466); QueueSoundB_m1x1();
    ret();
}

/* $80:AE93 (RTS, X = rider): BrakeRider (into which it falls) on the
 * rider whose turn it is ($7F). */
void BrakeOnTurn_m1x1(void)
{
    cmp8((u8)X, rd8(DP(0x7F)));
    if (!fz) { ret(); return; }
    BrakeRider_m1x1();
}

void RiderOnRamp_m1x1(void);
void AccelerateRider_m1x1(void);
void BrakeOnTurn_m1x1(void);

/* $80:C083: lean rider X one step toward angle A, every Y frames ($45,X). */
static void lean_toward(void)
{
    wr8(DP(0x00), (u8)A);
    lda8(rd8(DPX(0x45)));
    if (!fz) { ret(); return; }
    lda8((u8)Y);
    wr8(DPX(0x45), (u8)A);
    lda8(rd8(ABSX(0x01E6)));
    cmp8((u8)A, rd8(DP(0x00)));
    if (fz) { ret(); return; }
    if (fc) {
        wr8(ABSX(0x01E6), dec8(rd8(ABSX(0x01E6))));
        wr8(ABSX(0x01E6), dec8(rd8(ABSX(0x01E6))));
    }
    wr8(ABSX(0x01E6), inc8(rd8(ABSX(0x01E6))));
    ret();
}

/* $80:AEDB: the d-pad leans rider X (every $80:B889 frames by air state):
 * up ($00 bit 0) leans back to $80:B87D; otherwise forward to $80:B87F,
 * and holding a button past it (on the ground) keeps tipping until angle
 * $0D flips it ($01CE = 1, $3D,X = $0A, sound $24 / $29). */
static void lean_by_pad(void)
{
    lda8(rd8(DPX(0x45)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x01EE)));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xB889)));
    wr8(DPX(0x45), (u8)A);
    {
        u8 v = rd8(DP(0x00));
        fc = v & 1;
        v >>= 1;
        wr8(DP(0x00), v);
        nz8(v);
    }
    if (fc) {
        lda8(rd8(ABSX(0x01E6)));
        cmp8((u8)A, rd8(ABSY(0xB87D)));
        if (fz) { ret(); return; }
        if (!fc) { wr8(ABSX(0x01E6), inc8(rd8(ABSX(0x01E6)))); ret(); return; }
        wr8(ABSX(0x01E6), dec8(rd8(ABSX(0x01E6))));
        ret();
        return;
    }
    lda8(rd8(ABSX(0x08F4)));
    and8(0x02);
    wr8(ABSX(0x08F4), (u8)A);
    lda8(rd8(ABSX(0x01E6)));
    cmp8((u8)A, rd8(ABSY(0xB87F)));
    if (!fc) { wr8(ABSX(0x01E6), inc8(rd8(ABSX(0x01E6)))); ret(); return; }
    lda8(rd8(ABSX(0x017E)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABSX(0x01EE)));
    if (!fz) { ret(); return; }
    wr8(ABSX(0x01E6), inc8(rd8(ABSX(0x01E6))));
    lda8(0x0D); wr8(DPX(0x45), 0x0D);
    lda8(rd8(ABSX(0x01E6)));
    cmp8((u8)A, 0x0D);
    if (!fc) { ret(); return; }
    lda8(0x01); wr8(ABSX(0x01CE), 0x01);
    lda8(0x0A); wr8(DPX(0x3D), 0x0A);
    lda8((u8)X);
    if (!fz) lda8(0x05);
    fc = 0; adc8(0x24);
    push8(0x80); push16(0xAF3D); QueueSoundB_m1x1();
    ret();
}

/* $80:AC0E (RTS, X = rider): rider X's throttle and lean from its pad
 * $017E. Not crashed, held or changing lanes, on the ground with a button
 * down (A/B: $C0) it accelerates (AccelerateRider) up to the top speed
 * $80:B886 / $80:B883 (by button and, for the player, $84), with sound $3F
 * when the player starts from still; otherwise it brakes (BrakeOnTurn,
 * Y = how hard). Then, not crashed: in the air or fast enough ($A0) the
 * d-pad leans it; on the ground it settles toward the piece angle $08B4. */
void RiderThrottle_m1x1(void)
{
    lda8(rd8(ABSX(0x017E)));
    and8(0x03);
    wr8(DP(0x0A), (u8)A);
    ldy8(0x00);
    lda8(rd8(ABSX(0x08CC)));
    ora8(rd8(ABSX(0x01CE)));
    ora8(rd8(ABSX(0x199E)));
    if (!fz) goto brake;
    lda8(rd8(ABSX(0x01D6)));
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
    lda8(rd8(ABSX(0x01EE)));
    if (!fz) {
        ldy8(0x04);
        lda8(rd8(DP(0x0A)));
        if (fz) goto brake;
        Y = (u8)A; nz8((u8)Y);
        goto harder;
    }
    lda8(rd8(ABSX(0x017E)));
    and8(0xC0);
    if (fz) goto brake;
    wr8(DP(0x00), (u8)X);
    cmp8((u8)X, 0x00);
    if (fz) {
        lda8(rd8(ABSX(0x01BE)));
        ora8(rd8(ABSX(0x01C6)));
        if (fz) {
            lda8(rd8(DP(0x21)));
            and8(0xC0);
            if (!fz) { lda8(0x3F); push8(0x80); push16(0xAC5B); QueueSoundB_m1x1(); }
        }
    }
    lda8(rd8(ABSX(0x017E)));
    lda8(asl8((u8)A));
    if (!fc) wr8(DP(0x00), inc8(rd8(DP(0x00))));
    push16(0xAC66); RiderOnRamp_m1x1();
    if (!fz) {
        lda8(rd8(ABSX(0x020E)));
        if (fz) {
            lda8(rd8(ABSX(0x01C6)));
            if (!fz) goto brake;
        }
    }
    Y = rd8(DP(0x00)); nz8((u8)Y);
    lda8((u8)X);
    if (!fz) goto accel;
top:
    lda8(rd8(ABSX(0x01C6)));
    cmp8((u8)A, rd8(ABSY(0xB886)));
    if (!fc) goto accel;
    if (!fz) goto harder;
    lda8(rd8(ABSX(0x01BE)));
    cmp8((u8)A, rd8(ABSY(0xB883)));
    if (fz) goto after;
    if (fc) goto harder;
accel:
    push16(0xAC8E); AccelerateRider_m1x1();
    goto after;
harder:
    Y = inc8((u8)Y);
brake:
    push16(0xAC9C); BrakeOnTurn_m1x1();
after:
    lda8(rd8(ABSX(0x01CE)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x01EE)));
    int pad = !fz;
    if (!pad) {
        lda8(rd8(ABSX(0x0176)));
        ora8(rd8(DP(0x84)));
        if (fz) {
            lda8(rd8(ABSX(0x01C6)));
            pad = !fz;
            if (!pad) {
                lda8(rd8(ABSX(0x01BE)));
                cmp8((u8)A, 0xA0);
                pad = fc;
            }
        }
    }
    if (pad) {
        lda8(rd8(DP(0x0A)));
        if (!fz) {
            wr8(DP(0x00), (u8)A);
            lean_by_pad();
            return;
        }
    }
    lda8(rd8(ABSX(0x01EE)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABSX(0x08B4)));
    cmp8((u8)A, rd8(ABSX(0x01E6)));
    if (fz) { ret(); return; }
    ldy8(0x05);
    lean_toward();
}

void RiderThrottle_m1x1(void);

/* $80:ABCD (RTS, X = last rider): the throttle step for riders X-0 on the
 * track ($01DE); crashed, off-track or line-changing riders (not on the
 * $05 start walk) get no pad input. Then, unless $54, the player's
 * button flags $092D (once, not after the race $84). */
void RidersThrottle_m1x1(void)
{
    do {
        lda8(rd8(ABSX(0x01DE)));
        if (!fz) {
            lda8(rd8(ABSX(0x01D6)));
            cmp8((u8)A, 0x05);
            if (!fz) {
                ora8(rd8(ABSX(0x01CE)));
                ora8(rd8(ABSX(0x0940)));
                if (!fz) { lda8(0x00); wr8(ABSX(0x017E), 0x00); }
            }
            push16(0xABE8); RiderThrottle_m1x1();
        }
        X = dec8((u8)X);
    } while (!fn);
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x017E)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x092D)));
    ora8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    lda8(0x01); wr8(ABS(0x092D), 0x01);
    ret();
}

/* $81:97E8 (RTL): the object list (even slots 14-0, $03A3 = slot): each
 * active object ($02EF) counts its timers $036F / $037F down, runs its
 * type's step ($81:AD8F) and, if still active, its draw ($81:ADA3);
 * $1958 cleared first. */
void UpdateObjects_m1x1(void)
{
    wr8(ABS(0x1958), 0);
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    ldx8(0x0E);
    do {
        wr8(ABS(0x03A3), (u8)X);
        lda8(rd8(ABSX(0x02EF)));
        if (!fz) {
            lda8(rd8(ABSX(0x036F)));
            if (!fz) wr8(ABSX(0x036F), dec8(rd8(ABSX(0x036F))));
            lda8(rd8(ABSX(0x037F)));
            if (!fz) wr8(ABSX(0x037F), dec8(rd8(ABSX(0x037F))));
            lda8(rd8(ABSX(0x02FF)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            push8((u8)X);
            push16(0x9812);
            r816_call_pc(0x810000u | rd16(LONGX(0x81AD8F)));
            X = pull8(); nz8((u8)X);
            Y = rd8(ABS(0x03A3)); nz8((u8)Y);
            lda8(rd8(ABSY(0x02EF)));
            if (!fz) {
                push16(0x981E);
                r816_call_pc(0x810000u | rd16(LONGX(0x81ADA3)));
            }
        }
        X = rd8(ABS(0x03A3)); nz8((u8)X);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fn);
    DB = pull8(); nz8(DB);
    retl();
}

void UpdateObjects_m1x1(void);
void ResultsPlacePoints_m0x1(void);
void ResultsJumpPoints_m0x1(void);
void ShowCourseRecord_m1x1(void);

/* $80:9713: the course records list (before the results, $7D = 0): every
 * $20 frames ($0088) the next of up to ten times ($0819, three bytes;
 * $FF ends the list) at its VRAM row ($80:97A5) with sound $42, sound
 * $43 after the tenth; past ten the course record itself. Leaving ($8A
 * runs out, or Start with the debug flag $81:800E) goes to the results
 * (mode 2, $7D + 1). */
static void course_records_step(void)
{
    X = rd8(DP(0x7C)); nz8((u8)X);
    rep(0x20);
    wr16(DP(0x8A), dec16(rd16(DP(0x8A))));
    if (!fz) {
        sep(0x20);
        lda8(rd8(LONG(0x81800E)));
        int leave = 0;
        if (!fz) {
            lda8(rd8(DP(0x21)));
            and8(0x10);
            leave = !fz;
        }
        if (!leave) {
            wr8(ABS(0x0088), dec8(rd8(ABS(0x0088))));
            if (!fz) { ret(); return; }
            lda8(0x20); wr8(ABS(0x0088), 0x20);
            lda8(rd8(ABS(0x044F)));
            cmp8((u8)A, 0x0A);
            if (fc) { push16(0x973C); ShowCourseRecord_m1x1(); ret(); return; }
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            fc = 0; adc8(rd8(ABS(0x044F)));
            Y = (u8)A; nz8((u8)Y);
            lda8(rd8(ABSY(0x0819)));
            cmp8((u8)A, 0xFF);
            if (!fz) {
                wr8(ABS(0x03AE), (u8)A);
                lda8(rd8(ABSY(0x081A))); wr8(ABS(0x03AF), (u8)A);
                lda8(rd8(ABSY(0x081B))); wr8(ABS(0x03B0), (u8)A);
                lda8(0x42);
                push8(0x80); push16(0x9770); QueueSoundB_m1x1();
                rep(0x20);
                lda16(rd16(ABSX(0x97A5))); wr16(DP(0x00), A);
                sep(0x20);
                push16(0x977C); TickRaceClock_m1x1();
            } else {
                lda8(rd8(ABS(0x044F)));
                if (fz) {
                    lda8(0x0A); wr8(ABS(0x044F), 0x0A);
                    lda8(0x01); wr8(ABS(0x0088), 0x01);
                    ret();
                    return;
                }
                lda8(0x09); wr8(ABS(0x044F), 0x09);
            }
            wr8(ABS(0x044F), inc8(rd8(ABS(0x044F))));
            lda8(rd8(ABS(0x044F)));
            cmp8((u8)A, 0x0A);
            if (fz) { lda8(0x43); push8(0x80); push16(0x97A3); QueueSoundB_m1x1(); }
            ret();
            return;
        }
    }
    sep(0x20);
    lda8(0x02); wr8(DP(0x76), 0x02);
    wr8(DP(0x75), 0); wr8(DP(0x77), 0);
    wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    wr8(DP(0x7D), inc8(rd8(DP(0x7D))));
    ret();
}

/* $80:95A2 (RTS): the end-of-broadcast screens. Before the results
 * ($7D = 0) the course records; then the results: objects, the place and
 * jump points lines at timer $04EC / $0474 of $8A, and when $8A runs out
 * (or Start with the debug flag) the next broadcast part: entries 1 and 3
 * go to mode $17 with $1E87 - $0A, the others to the next entry ($7C + 1)
 * in mode $18 with $1905 = $15. */
void BroadcastEndScreens_m1x1(void)
{
    lda8(rd8(DP(0x7D)));
    if (fz) { course_records_step(); return; }
    push8(0x80); push16(0x95AF); UpdateObjects_m1x1();
    X = rd8(DP(0x7C)); nz8((u8)X);
    rep(0x20);
    wr16(DP(0x8A), dec16(rd16(DP(0x8A))));
    if (!fz) {
        lda16(rd16(DP(0x8A)));
        cmp16(A, 0x04EC);
        if (fz) { push16(0x95C1); ResultsPlacePoints_m0x1(); }
        else {
            cmp16(A, 0x0474);
            if (fz) { push16(0x95CB); ResultsJumpPoints_m0x1(); }
        }
        sep(0x20);
        lda8(rd8(LONG(0x81800E)));
        if (fz) { ret(); return; }
        lda8(rd8(DP(0x21)));
        and8(0x10);
        if (fz) { ret(); return; }
    }
    sep(0x20);
    wr8(DP(0x75), 0); wr8(DP(0x77), 0);
    wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    cmp8((u8)X, 0x03);
    int bonus = fz;
    if (!bonus) { cmp8((u8)X, 0x01); bonus = fz; }
    if (bonus) {
        lda8(0x17); wr8(DP(0x76), 0x17);
        lda8(rd8(ABS(0x1E87)));
        fc = 1; sbc8(0x0A);
        wr8(ABS(0x1E87), (u8)A);
    } else {
        wr8(DP(0x7D), 0);
        wr8(DP(0x7C), inc8(rd8(DP(0x7C))));
        lda8(0x18); wr8(DP(0x76), 0x18);
        lda8(0x15); wr8(ABS(0x1905), 0x15);
    }
    ret();
}

/* $82:9028 (RTL): the stadium bonus banner on its timer $D4: at $7E the
 * "bonus" text ($82:9113 at VRAM $1935) and the stadium's bonus points
 * ($82:9131 + 4 * (stadium - 2), leading zeros blank, a trailing 0) at
 * $1977; at 0 both rows are cleared ($82:90F5 / $82:9108). */
void StadiumBonusBanner_m1x1(void)
{
    push8(DB);
    lda8(0x82); push8(0x82);
    DB = pull8(); nz8(DB);
    lda8(rd8(DP(0xD4)));
    if (!fz) {
        wr8(DP(0xD4), dec8(rd8(DP(0xD4))));
        lda8(rd8(DP(0xD4)));
        if (fz) {
            rep(0x20);
            lda16(0x1935); wr16(DP(0x00), 0x1935);
            lda16(0x90F5); wr16(DP(0x02), 0x90F5);
            sep(0x20);
            lda8(0x20); wr8(DP(0x0F), 0x20);
            push8(0x82); push16(0x9056); VramQueueTilesLong_m1x1();
            rep(0x20);
            lda16(0x1977); wr16(DP(0x00), 0x1977);
            lda16(0x9108); wr16(DP(0x02), 0x9108);
            sep(0x20);
            lda8(0x20); wr8(DP(0x0F), 0x20);
            push8(0x82); push16(0x906C); VramQueueTilesLong_m1x1();
        } else {
            cmp8((u8)A, 0x7E);
            if (fz) {
                rep(0x20);
                lda16(0x1935); wr16(DP(0x00), 0x1935);
                lda16(0x9113); wr16(DP(0x02), 0x9113);
                sep(0x20);
                lda8(0x20); wr8(DP(0x0F), 0x20);
                push8(0x82); push16(0x9085); VramQueueTilesLong_m1x1();
                lda8(rd8(ABS(0x0928)));
                lda8(dec8((u8)A)); lda8(dec8((u8)A));
                wr8(DP(0x00), (u8)A);
                lda8(asl8((u8)A)); lda8(asl8((u8)A));
                X = (u8)A; nz8((u8)X);
                for (int i = 0; i < 4; i++) {
                    lda8(rd8(ABSX(0x9131 + i)));
                    wr8(ABS(0x03D0 + i), (u8)A);
                    ora8(0x10);
                    wr8(ABS(0x03D5 + i), (u8)A);
                }
                wr8(ABS(0x03D4), 0);
                lda8(0x10); wr8(ABS(0x03D9), 0x10);
                lda8(0x25); wr8(ABS(0x03CF), 0x25);
                ldx8(0x00);
                for (;;) {
                    lda8(rd8(ABSX(0x03D0)));
                    if (!fz) break;
                    lda8(0xFF);
                    wr8(ABSX(0x03D0), 0xFF);
                    wr8(ABSX(0x03D5), 0xFF);
                    X = inc8((u8)X);
                    cmp8((u8)X, 0x02);
                    if (fc) break;
                }
                rep(0x20);
                lda16(0x1977); wr16(DP(0x00), 0x1977);
                lda16(0x03CF); wr16(DP(0x02), 0x03CF);
                sep(0x20);
                lda8(0x20); wr8(DP(0x0F), 0x20);
                push8(0x82); push16(0x90F2); VramQueueTilesLong_m1x1();
            }
        }
    }
    DB = pull8(); nz8(DB);
    retl();
}

/* $82:8CF0 SpeedReadout (RTL): on odd frames the player's speed
 * ($01C6:$01BE; while it holds steady and nonzero it jitters by a random
 * 0 / 1, $D0 = last) as "NNN.N" at VRAM $1B22 (leading zeros blank,
 * the point $54); 0 before the jump contest's first run ($C7 = 2 with
 * $1569 = 0). */
void SpeedReadout_m1x1(void)
{
    lda8(rd8(DP(0x63)));
    and8(0x01);
    if (fz) { retl(); return; }
    lda8(rd8(ABS(0x01C6)));
    A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
    lda8(rd8(ABS(0x01BE)));
    rep(0x20);
    cmp16(A, rd16(DP(0xD0)));
    int jitter = fz;
    if (jitter) { cmp16(A, 0x0000); jitter = !fz; }
    if (jitter) {
        wr16(DP(0xD0), A);
        sep(0x20);
        push8(0x82); push16(0x8D10); RandomB_m1x1();
        rep(0x20);
        and16(0x0001);
        fc = 0; adc16(rd16(DP(0xD0)));
    } else {
        wr16(DP(0xD0), A);
    }
    wr16(DP(0x00), 0); wr16(DP(0x02), 0); wr16(DP(0x04), 0);
    wr16(DP(0x0E), A);
    lda16(rd16(ABS(0x00C7)));
    and16(0x00FF);
    cmp16(A, 0x0002);
    if (fz) {
        lda16(rd16(ABS(0x1569)));
        if (fz) wr16(DP(0x0E), 0);
    }
    lda16(rd16(DP(0x0E)));
    static const u16 pow10[3] = { 1000, 100, 10 };
    static const u16 cnt[3] = { 0x00, 0x02, 0x04 };
    for (int i = 0; i < 3; i++) {
        for (;;) {
            cmp16(A, pow10[i]);
            if (!fc) break;
            sbc16(pow10[i]);
            wr16(DP(cnt[i]), inc16(rd16(DP(cnt[i]))));
            WAIT_TICK();
        }
    }
    sep(0x20);
    wr8(ABS(0x03D4), (u8)A);
    ora8(0x10); wr8(ABS(0x03D9), (u8)A);
    lda8(0xFF); wr8(ABS(0x03D3), 0xFF);
    lda8(0x54); wr8(ABS(0x03D8), 0x54);
    lda8(rd8(DP(0x04)));
    wr8(ABS(0x03D2), (u8)A);
    ora8(0x10); wr8(ABS(0x03D7), (u8)A);
    lda8(rd8(DP(0x00)));
    if (fz) {
        lda8(0xFF); wr8(ABS(0x03D0), 0xFF); wr8(ABS(0x03D5), 0xFF);
    } else {
        wr8(ABS(0x03D0), (u8)A);
        ora8(0x10); wr8(ABS(0x03D5), (u8)A);
    }
    lda8(rd8(DP(0x02)));
    int blank = 0;
    if (fz) { Y = rd8(DP(0x00)); nz8((u8)Y); blank = fz; }
    if (blank) {
        lda8(0xFF); wr8(ABS(0x03D1), 0xFF); wr8(ABS(0x03D6), 0xFF);
    } else {
        wr8(ABS(0x03D1), (u8)A);
        ora8(0x10); wr8(ABS(0x03D6), (u8)A);
    }
    lda8(0x25); wr8(ABS(0x03CF), 0x25);
    rep(0x20);
    lda16(0x1B22); wr16(DP(0x00), 0x1B22);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push8(0x82); push16(0x8DC5); VramQueueTilesLong_m1x1();
    retl();
}

void DrawJumpRiderSprites_m1x1(void);
void SendSpeedToApu_m1x1(void);
void DrawCountdownLights_m1x1(void);
void DispatchBy85B_m1x1(void);
void SetJumpScroll_m1x1(void);
void QueueBestTime_m1x1(void);
void SpeedReadout_m1x1(void);

/* $80:E85B (RTS): the jump contest's frame. Waiting for the next run
 * ($C7 = 2) the rider brakes and is drawn standing ($0216 = $35). Then
 * the engine sound, the countdown lights, $A0 = distance / 2 and the
 * start timer $88 (sound $26 at $40; at 0, $8E = $80, sound $27, the
 * run count $C3 + 1). Unless paused ($54) or past the end ($84 even
 * and nonzero: to mode $0A with $50 = 2), the player's pad drives the
 * rider, the twenty jump steps run ($85 = 0-$13), and the speed readout
 * and scroll update. */
void JumpContestFrame_m1x1(void)
{
    lda8(rd8(ABS(0x00C7)));
    cmp8((u8)A, 0x02);
    if (fz) {
        ldy8(0x05);
        push16(0xE866); BrakeOnTurn_m1x1();
        lda8(0x01); wr8(ABS(0x08BC), 0x01);
        ldx8(0x00);
        Y = X; nz8((u8)Y);
        lda8(0x35); wr8(ABS(0x0216), 0x35);
        push16(0xE876); DrawJumpRiderSprites_m1x1();
    }
    push16(0xE879); SendSpeedToApu_m1x1();
    push16(0xE87C); DrawCountdownLights_m1x1();
    rep(0x20);
    lda16(rd16(DP(0x9C)));
    lda16(lsr16(A));
    wr16(DP(0xA0), A);
    lda16(rd16(DP(0x88)));
    if (!fz) {
        cmp16(A, 0x0040);
        if (fz) {
            lda16(0x0026);
            push8(0x80); push16(0xE893); QueueSoundB_m0x1();
        }
        wr16(DP(0x88), dec16(rd16(DP(0x88))));
        lda16(rd16(DP(0x88)));
        if (!fz) { sep(0x20); ret(); return; }
        lda16(0x0080); wr16(DP(0x8E), 0x0080);
        sep(0x20);
        lda8(0x27);
        push8(0x80); push16(0xE8A6); QueueSoundB_m1x1();
        wr8(DP(0xC3), inc8(rd8(DP(0xC3))));
        wr8(ABS(0x1958), 0);
    }
    sep(0x20);
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0x84)));
    if (!fz) {
        lda8(lsr8((u8)A));
        if (!fc) {
            lda8(0x02); wr8(DP(0x50), 0x02);
            lda8(0x00); wr8(DP(0x85), 0x00);
            lda8(0x0A);
            ChangeMode_m1x1();
            return;
        }
    }
    lda8(rd8(DP(0x20)));
    wr8(ABS(0x017E), (u8)A);
    lda8(0x00); wr8(DP(0x85), 0x00);
    do {
        push16(0xE8C4); DispatchBy85B_m1x1();
        wr8(DP(0x85), inc8(rd8(DP(0x85))));
        lda8(rd8(DP(0x85)));
        cmp8((u8)A, 0x14);
    } while (!fc);
    push8(0x80); push16(0xE8D0); SpeedReadout_m1x1();
    push8(0x80); push16(0xE8D4); SetJumpScroll_m1x1();
    sep(0x20);
    ret();
}

/* $80:FEFC (RTL): QueueBonusDigits for other banks. */
void QueueBonusDigitsLong_m1x1(void)
{
    push16(0xFEFE); QueueBonusDigits_m1x1();
    retl();
}

/* $80:C619 (RTL): QueueBestTime with DB = $80. */
void QueueBestTimeLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xC620); QueueBestTime_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}

void QueueBestTimeLong_m1x1(void);
void QueueBonusDigitsLong_m1x1(void);

/* $82:8DC7 (RTL): the new-record blink on its timer $CB: at 1 the best
 * time back at VRAM $18D7; every 16 frames the clock at $1897 alternates
 * between blank and the stadium's record ($0819 + 3 * (stadium - 2):
 * minute, seconds, hundredths via $80:C470; the marks $47 / $48). */
void BestTimeBlink_m1x1(void)
{
    lda8(rd8(DP(0xCB)));
    if (fz) { retl(); return; }
    wr8(DP(0xCB), dec8(rd8(DP(0xCB))));
    if (fz) {
        lda8(0xD7); wr8(DP(0x00), 0xD7);
        lda8(0x18); wr8(DP(0x01), 0x18);
        lda8(0x24); wr8(DP(0x0F), 0x24);
        push8(0x82); push16(0x8DDF); QueueBestTimeLong_m1x1();
    }
    lda8(rd8(DP(0xCB)));
    and8(0x0F);
    if (!fz) { retl(); return; }
    lda8(rd8(DP(0xCB)));
    and8(0x10);
    if (fz) {
        ldx8(0x0F);
        lda8(0xFF);
        do {
            wr8(ABSX(0x03CF), 0xFF);
            X = dec8((u8)X);
        } while (!fn);
    } else {
        lda8(rd8(ABS(0x0928)));
        lda8(dec8((u8)A)); lda8(dec8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        X = (u8)A; nz8((u8)X);
        Y = rd8(ABSX(0x081B)); nz8((u8)Y);
        lda8(rd8(ABSY(0xC470)));
        static const u16 lo[2] = { 0x03D6, 0x03D3 };
        for (int i = 0; i < 2; i++) {
            if (i) lda8(rd8(ABSX(0x081A)));
            ldy8(0x00);
            for (;;) {
                cmp8((u8)A, 0x0A);
                if (!fc) break;
                sbc8(0x0A);
                Y = inc8((u8)Y);
            }
            wr8(ABS(lo[i]), (u8)A);
            ora8(0x10); wr8(ABS(lo[i] + 7), (u8)A);
            lda8((u8)Y);
            wr8(ABS(lo[i] - 1), (u8)A);
            ora8(0x10); wr8(ABS(lo[i] + 6), (u8)A);
        }
        lda8(rd8(ABSX(0x0819)));
        wr8(ABS(0x03D0), (u8)A);
        ora8(0x10); wr8(ABS(0x03D7), (u8)A);
        lda8(0xFF); wr8(ABS(0x03D8), 0xFF); wr8(ABS(0x03DB), 0xFF);
        lda8(0x47); wr8(ABS(0x03D1), 0x47);
        lda8(0x48); wr8(ABS(0x03D4), 0x48);
    }
    rep(0x20);
    lda16(0x1897); wr16(DP(0x00), 0x1897);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x27); wr8(ABS(0x03CF), 0x27);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push8(0x82); push16(0x8E7D); VramQueueTilesLong_m1x1();
    retl();
}

/* $81:9926 (RTS): the results' bonus count-down object. Every 4 frames
 * ($88) the bonus $BA counts down by one into the score (sound $44, the
 * count at VRAM $1CB6, $09AF = 1, the bonus digits). At 0 it hands the
 * podium on: the player's place (the slot where $086E is 1) plus 8 *
 * $154F picks the type-7 object's state 3 (timer $10) and the type from
 * $81:9A06 goes to state 4 with long timers; its own state steps. */
void ResultsBonusCount_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) goto tick;
    lda8(rd8(DP(0xBA)));
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
        lda8(rd8(ABS(0x154F)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(0x03); wr8(DP(0x01), 0x03);
        lda8(0x07);
        push8(0x81); push16(0x9956); FindObjectSlot_m1x1();
        lda8(rd8(DP(0x01))); wr8(ABSY(0x030F), (u8)A);
        lda8(0x10); wr8(ABSY(0x036F), 0x10);
        X = rd8(ABS(0x03A3)); nz8((u8)X);
        wr8(ABSX(0x030F), inc8(rd8(ABSX(0x030F))));
        Y = rd8(DP(0x00)); nz8((u8)Y);
        lda8(rd8(ABSY(0x9A06)));
        push8(0x81); push16(0x996F); FindObjectSlot_m1x1();
        lda8(0x04); wr8(ABSY(0x030F), 0x04);
        lda8(0xFF); wr8(ABSY(0x036F), 0xFF);
        lda8(0xFF); wr8(ABSY(0x037F), 0xFF);
        goto tick;
    }
    wr8(DP(0xBA), dec8(rd8(DP(0xBA))));
    lda8(0x44);
    push8(0x81); push16(0x9989); QueueSoundB_m1x1();
    ldy8(0x00);
    lda8(rd8(DP(0xBA)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
    }
    wr8(DP(0x00), (u8)A);
    wr8(ABS(0x03D2), (u8)A);
    ora8(0x10); wr8(ABS(0x03D7), (u8)A);
    cmp8((u8)Y, 0x00);
    if (!fz) {
        lda8((u8)Y);
        wr8(ABS(0x03D1), (u8)A);
        ora8(0x10); wr8(ABS(0x03D6), (u8)A);
    } else {
        lda8(0xFF); wr8(ABS(0x03D1), 0xFF); wr8(ABS(0x03D6), 0xFF);
    }
    lda8((u8)Y);
    ora8(rd8(DP(0x00)));
    if (fz) {
        lda8(0xFF);
        wr8(ABS(0x03D2), 0xFF); wr8(ABS(0x03D3), 0xFF);
        wr8(ABS(0x03D7), 0xFF); wr8(ABS(0x03D8), 0xFF);
    }
    lda8(0xFF); wr8(ABS(0x03D0), 0xFF); wr8(ABS(0x03D5), 0xFF);
    lda8(0x04); wr8(DP(0x88), 0x04);
    rep(0x20);
    lda16(0x1CB6); wr16(DP(0x00), 0x1CB6);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x99EC); VramQueueTilesLong_m1x1();
    lda8(0x01); wr8(ABS(0x09AF), 0x01);
    push8(0x81); push16(0x99F5); AddScoreLong_m1x1();
    wr8(DP(0x00), dec8(rd8(DP(0x00))));
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x99FF); QueueBonusDigitsLong_m1x1();
tick:
    wr8(DP(0x88), dec8(rd8(DP(0x88))));
    X = rd8(ABS(0x03A3)); nz8((u8)X);
    ret();
}

void TitlePushStart_m1x1(void);

/* $80:F3DD (RTS): the attract demo's pad display: a 2x2 d-pad sprite
 * (OAM $1680, tiles from $80:F4BF by the direction table $80:F4AF on
 * $098E's low nibble, attribute $26) and a 4-tile button sprite (OAM
 * $1670, tiles from $80:F4C4 by $098E bits 7-6, attribute $28), both
 * large; then PUSH START. */
void DemoPadDisplay_m1x1(void)
{
    lda8(rd8(ABS(0x098E)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF4AF)));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF4BF)));
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
    lda8(rd8(ABSY(0xF4C4)));
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
    push16(0xF4AD); TitlePushStart_m1x1();
    ret();
}

void sub_81C8E1_m0x1(void);
void LoadRiderPalettes_m1x1(void);
void FillBonusColumns_m1x1(void);

/* $80:F107 (RTS): the jump contest's HUD numbers: the last distance $1567
 * (half-units, $81:C8E1) at VRAM $1856, and the best score $156D (raised
 * to ScaleJumpCount's $0C + the rank points when higher) at $1897. */
void JumpHudNumbers_m1x1(void)
{
    rep(0x20);
    lda16(0x1856); wr16(DP(0x00), 0x1856);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(0x0020); wr16(DP(0x0F), 0x0020);
    lda16(rd16(ABS(0x1567)));
    push8(0x80); push16(0xF11E); CALL(sub_81C8E1_m0x1, 0x81C8E1, 3);
    push8(0x80); push16(0xF122); ScaleJumpCount_m1x1();
    rep(0x20);
    Y = rd8(ABS(0x156B)); nz8((u8)Y);
    lda16(rd16(ABSY(0xF155)));
    and16(0x00FF);
    fc = 0; adc16(rd16(DP(0x0C)));
    cmp16(A, rd16(ABS(0x156D)));
    if (fc) wr16(ABS(0x156D), A);
    lda16(rd16(ABS(0x156D)));
    push16(0xF13E); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x1897); wr16(DP(0x00), 0x1897);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xF153); VramQueueTiles_m1x1();
    ret();
}

/* $80:E5B0 (RTS): set up the jump contest: rider palettes; the lane rows
 * ($09D3 ramps $FC, three empty rows, six scenery rows $3B); the streamer
 * start; the player at column 9, X $30, angle 6 on page 1 ($94 = 2,
 * $1905 = $17, HDMA scene 3), drawn once at ground $80 then $B8; HDMA
 * channels 6/7 ($82:97B1 -> $210D, $82:9AB2 -> $210F); BG offsets $0100,
 * $88 = $78, the line buffer $1000-$13FF cleared, scores cleared,
 * $190F = $DC; the bonus columns and the HUD numbers; $156B = $FF and
 * BG34NBA = 1. */
void SetupJumpContest_m1x1(void)
{
    push8(0x80); push16(0xE5B3); LoadRiderPalettes_m1x1();
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
    lda8(0x34); wr8(ABS(0x0276), 0x34);
    lda8(0x0A); wr8(ABS(0x0279), 0x0A);
    lda8(0x05); wr8(ABS(0x0278), 0x05);
    lda8(0x74); wr8(ABS(0x0277), 0x74);
    lda8(0x08); wr8(ABS(0x018E), 0x08);
    lda8(0x09); wr8(ABS(0x0266), 0x09);
    lda8(0x30); wr8(ABS(0x0196), 0x30);
    lda8(0x06); wr8(ABS(0x01E6), 0x06);
    lda8(0x01);
    wr8(ABS(0x019E), 0x01); wr8(ABS(0x01DE), 0x01); wr8(ABS(0x0958), 0x01);
    lda8(asl8((u8)A));
    wr8(DP(0x94), (u8)A);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    lda8(0x03);
    push8(0x80); push16(0xE61D); LoadHdmaScene_m1x1();
    ldx8(0x00);
    lda8(0x80); wr8(ABS(0x0CD5), 0x80); wr8(ABS(0x01B6), 0x80);
    push16(0xE62A); DrawAllRiders_m1x1();
    lda8(0xB8); wr8(ABS(0x0CD5), 0xB8); wr8(ABS(0x01B6), 0xB8);
    static const u8 ch7[6][2] = { {0x70,0x42},{0x71,0x0F},{0x72,0xB2},{0x73,0x9A},{0x74,0x82},{0x77,0x00} };
    static const u8 ch6[6][2] = { {0x60,0x42},{0x61,0x0D},{0x62,0xB1},{0x63,0x97},{0x64,0x82},{0x67,0x00} };
    for (int i = 0; i < 6; i++) { lda8(ch7[i][1]); wr8(ABS(0x4300 | ch7[i][0]), ch7[i][1]); }
    for (int i = 0; i < 6; i++) { lda8(ch6[i][1]); wr8(ABS(0x4300 | ch6[i][0]), ch6[i][1]); }
    wr8(ABS(0x0096), inc8(rd8(ABS(0x0096))));
    rep(0x30);
    lda16(0x0100); wr16(DP(0x9E), 0x0100); wr16(DP(0xA2), 0x0100);
    lda16(0x0078); wr16(DP(0x88), 0x0078);
    X = 0x03FE; nz16(X);
    do {
        wr16(ABSX(0x1000), 0);
        X = dec16(X);
        X = dec16(X);
    } while (!fn);
    wr16(ABS(0x1993), 0); wr16(ABS(0x1997), 0);
    sep(0x30);
    wr8(ABS(0x1999), 0);
    lda8(0xDC); wr8(ABS(0x190F), 0xDC);
    push16(0xE69C); FillBonusColumns_m1x1();
    push16(0xE69F); JumpHudNumbers_m1x1();
    lda8(0xFF); wr8(ABS(0x156B), 0xFF);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    ret();
}

void BuildJumpScore_m1x1(void);
void ScaleJumpCount_m0x1(void);

/* $80:F07E (RTS): the last jump distance $1567 (half-units) at VRAM
 * $1A75. */
void ShowLastDistance_m1x1(void)
{
    rep(0x20);
    lda16(0x1A75); wr16(DP(0x00), 0x1A75);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(0x0020); wr16(DP(0x0F), 0x0020);
    lda16(rd16(ABS(0x1567)));
    push8(0x80); push16(0xF095); CALL(sub_81C8E1_m0x1, 0x81C8E1, 3);
    ret();
}

/* $81:C7EF (RTL): the jump result's distance step: sound $42, next step
 * ($CD), $8C = $20, this run's distance $1569 at VRAM $1932 and its
 * label rows ($81:C82F, $0208) at $1927. */
void JumpResultDistance_m1x1(void)
{
    lda8(0x42);
    push8(0x81); push16(0xC7F4); QueueSoundB_m1x1();
    wr8(DP(0xCD), inc8(rd8(DP(0xCD))));
    lda8(0x20); wr8(DP(0x8C), 0x20);
    rep(0x20);
    lda16(0x1932); wr16(DP(0x00), 0x1932);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(0x0020); wr16(DP(0x0F), 0x0020);
    lda16(rd16(ABS(0x1569)));
    push8(0x81); push16(0xC812); CALL(sub_81C8E1_m0x1, 0x81C8E1, 3);
    rep(0x20);
    lda16(0x1927); wr16(DP(0x00), 0x1927);
    lda16(0xC82F); wr16(DP(0x02), 0xC82F);
    ldx8(0x81); wr8(DP(0x04), 0x81);
    lda16(0x0208); wr16(DP(0x10), 0x0208);
    sep(0x20);
    push8(0x81); push16(0xC82D); QueueTileRowsLong_m1x1();
    retl();
}

/* $80:EA68 (RTS): the jump result board: best distance ($1567) at $1856,
 * the best score at $1897 (this run's when the board is still counting,
 * $88 = 0), this run's distance at $1932 and score parts at $1970 /
 * $19B3 / $19F3; a new best ($1569 > $1567 or score > $156D) blinks with
 * $8A and, at $88 = $3B ($7F without $1999), plays sound $43. */
void JumpResultBoard_m1x1(void)
{
    wr8(DP(0xD6), 0);
    rep(0x20);
    lda16(0x1856); wr16(DP(0x00), 0x1856);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(0x0020); wr16(DP(0x0F), 0x0020);
    lda16(rd16(ABS(0x1567)));
    push8(0x80); push16(0xEA81); CALL(sub_81C8E1_m0x1, 0x81C8E1, 3);
    rep(0x20);
    lda16(rd16(DP(0x88)));
    int best = !fz;
    if (!best) {
        push8(0x80); push16(0xEA8B); ScaleJumpCount_m0x1();
        Y = rd8(ABS(0x156B)); nz8((u8)Y);
        lda16(rd16(ABSY(0xF155)));
        and16(0x00FF);
        fc = 0; adc16(rd16(DP(0x0C)));
        cmp16(A, rd16(ABS(0x156D)));
        best = !fc;
    }
    if (best) lda16(rd16(ABS(0x156D)));
    push16(0xEAA2); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x1897); wr16(DP(0x00), 0x1897);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xEAB7); VramQueueTiles_m1x1();
    rep(0x20);
    lda16(0x1932); wr16(DP(0x00), 0x1932);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(rd16(ABS(0x1567)));
    cmp16(A, rd16(ABS(0x1569)));
    if (fc) lda16(0x0020);
    else {
        wr16(DP(0xD6), inc16(rd16(DP(0xD6))));
        lda16(rd16(DP(0x8A)));
    }
    wr16(DP(0x0F), A);
    lda16(rd16(ABS(0x1569)));
    push8(0x80); push16(0xEADD); CALL(sub_81C8E1_m0x1, 0x81C8E1, 3);
    rep(0x20);
    lda16(rd16(ABS(0x1569)));
    push16(0xEAE5); BuildScoreDigits_m0x1();
    push8(0x80); push16(0xEAE9); BuildJumpScore_m1x1();
    rep(0x20);
    lda16(0x1970); wr16(DP(0x00), 0x1970);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xEAFE); VramQueueTiles_m1x1();
    rep(0x20);
    Y = rd8(ABS(0x156B)); nz8((u8)Y);
    lda16(rd16(ABSY(0xF155)));
    and16(0x00FF);
    push16(0xEB0C); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x19B3); wr16(DP(0x00), 0x19B3);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    sep(0x20);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xEB21); VramQueueTiles_m1x1();
    push8(0x80); push16(0xEB25); ScaleJumpCount_m1x1();
    rep(0x20);
    Y = rd8(ABS(0x156B)); nz8((u8)Y);
    lda16(rd16(ABSY(0xF155)));
    and16(0x00FF);
    fc = 0; adc16(rd16(DP(0x0C)));
    wr16(DP(0x0C), A);
    push16(0xEB38); BuildScoreDigits_m0x1();
    rep(0x20);
    lda16(0x19F3); wr16(DP(0x00), 0x19F3);
    lda16(0x03CF); wr16(DP(0x02), 0x03CF);
    lda16(rd16(ABS(0x156D)));
    cmp16(A, rd16(DP(0x0C)));
    sep(0x20);
    if (fc) lda8(0x20);
    else {
        wr8(DP(0xD6), inc8(rd8(DP(0xD6))));
        lda8(rd8(DP(0x8A)));
    }
    wr8(DP(0x0F), (u8)A);
    push16(0xEB5A); VramQueueTiles_m1x1();
    lda8(rd8(ABS(0x1999)));
    u16 mark = fz ? 0x007F : 0x003B;
    rep(0x20);
    lda16(rd16(DP(0x88)));
    cmp16(A, mark);
    sep(0x20);
    if (!fz) { ret(); return; }
    lda8(rd8(DP(0xD6)));
    if (fz) { ret(); return; }
    lda8(0x43);
    push8(0x80); push16(0xEB81); QueueSoundB_m1x1();
    ret();
}

/* $80:EA01 (RTS): the jump result board step: while $88 counts down, the
 * new-best blink $8A ($24 steady with $1999, else $20 / $24 every 16
 * frames); at 0 the best distance is kept ($1567 = max with $1569),
 * $8A = $20 and the step $CD advances. Then the waving player and the
 * board. */
void JumpResultBoardStep_m1x1(void)
{
    lda8(rd8(DP(0x88)));
    if (!fz) {
        lda8(dec8((u8)A));
        wr8(DP(0x88), (u8)A);
        X = rd8(ABS(0x1999)); nz8((u8)X);
        if (!fz) lda8(0x24);
        else {
            and8(0x10);
            lda8(lsr8((u8)A)); lda8(lsr8((u8)A));
            ora8(0x20);
        }
        wr8(DP(0x8A), (u8)A);
    } else {
        rep(0x20);
        lda16(rd16(ABS(0x1569)));
        cmp16(A, rd16(ABS(0x1567)));
        if (fc) wr16(ABS(0x1567), A);
        sep(0x20);
        lda8(0x20); wr8(DP(0x8A), 0x20);
        wr8(DP(0xCD), inc8(rd8(DP(0xCD))));
    }
    ldx8(0x00);
    push16(0xEA34); DrawAllRiders_m1x1();
    lda8(0x19); wr8(ABS(0x1961), 0x19);
    push16(0xEA3C); JumpResultBoard_m1x1();
    ret();
}

void ShowJumpCount_m1x1(void);
void ShowBonusPoints_m1x1(void);
void JumpResultDistance_m1x1(void);

/* $80:E938 (RTL): ShowJumpCount for other banks. */
void ShowJumpCountLong_m1x1(void)
{
    push16(0xE93A); ShowJumpCount_m1x1();
    retl();
}

/* $80:E994 (RTL): ShowBonusPoints for other banks. */
void ShowBonusPointsLong_m1x1(void)
{
    push16(0xE996); ShowBonusPoints_m1x1();
    retl();
}

/* $81:C956 (RTL): a button on the jump result screen skips ahead: from
 * step $CD 2-5 the remaining parts (distance, count, bonus points, the
 * total) are shown at once; then step 5, $8C = 2, $88 = $3C and
 * $1999 = 1. */
void JumpResultSkip_m1x1(void)
{
    lda8(rd8(DP(0x21)));
    ora8(rd8(DP(0x26)));
    if (fz) { retl(); return; }
    lda8(rd8(DP(0xCD)));
    cmp8((u8)A, 0x02);
    if (fn) { retl(); return; }
    int from = 2;
    if (!fz) {
        cmp8((u8)A, 0x03);
        if (fz) from = 3;
        else {
            cmp8((u8)A, 0x04);
            if (fz) from = 4;
            else {
                cmp8((u8)A, 0x05);
                if (fz) from = 5;
                else {
                    cmp8((u8)A, 0x05);
                    if (fc) { retl(); return; }
                }
            }
        }
    }
    if (from <= 2) { push8(0x81); push16(0xC977); JumpResultDistance_m1x1(); }
    if (from <= 3) { push8(0x81); push16(0xC97B); ShowJumpCountLong_m1x1(); }
    if (from <= 4) { push8(0x81); push16(0xC97F); ShowBonusPointsLong_m1x1(); }
    push8(0x81); push16(0xC983); QueueResultsTiles_m1x1();
    lda8(0x05); wr8(DP(0xCD), 0x05);
    lda8(0x02); wr8(DP(0x8C), 0x02);
    lda8(0x3C); wr8(DP(0x88), 0x3C);
    lda8(0x01); wr8(ABS(0x1999), 0x01);
    retl();
}

void DrawJumpRank_m1x1(void);
void CheckCourseTime_m1x1(void);
void JumpResultSkip_m1x1(void);
void ShowEntryCode_m1x1(void);
void QueueFontTime_m1x1(void);
void QueueFontDigits_m1x1(void);
void sub_82846B_m1x1(void);

/* $80:E6D2 (RTS): the jump contest mode. At one of the broadcast minutes
 * $80:E749 it ends: mode 4 (BG3 at $19, $1905 = $17, $86 cleared) after
 * keeping the best distance and score ($156B set). Otherwise the rank,
 * the course time, the result skip and the step $CD through $80:E739. */
void JumpContestMode_m1x1(void)
{
    ldx8(0x03);
    lda8(rd8(LONG(0x7FFFFF)));
    for (;;) {
        cmp8((u8)A, rd8(ABSX(0xE749)));
        if (fz) break;
        X = dec8((u8)X);
        if (fn) {
            push16(0xE72A); DrawJumpRank_m1x1();
            push16(0xE72D); CheckCourseTime_m1x1();
            push8(0x80); push16(0xE731); JumpResultSkip_m1x1();
            lda8(rd8(DP(0xCD)));
            lda8(asl8((u8)A));
            X = (u8)A; nz8((u8)X);
            r816_call_pc(0x800000u | rd16(LONGX(0x80E739)));
            return;
        }
    }
    lda8(0x04); wr8(DP(0x76), 0x04);
    lda8(0x19); wr8(ABS(0x2109), 0x19);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    wr8(DP(0x86), 0); wr8(DP(0x87), 0);
    X = rd8(ABS(0x156B)); nz8((u8)X);
    cmp8((u8)X, 0xFF);
    if (!fz) {
        rep(0x20);
        lda16(rd16(ABS(0x1569)));
        cmp16(A, rd16(ABS(0x1567)));
        if (fc) wr16(ABS(0x1567), A);
        push8(0x80); push16(0xE710); ScaleJumpCount_m0x1();
        lda16(rd16(LONGX(0x80F155)));
        and16(0x00FF);
        fc = 0; adc16(rd16(DP(0x0C)));
        cmp16(A, rd16(ABS(0x156D)));
        if (fc) wr16(ABS(0x156D), A);
    }
    sep(0x20);
    ChangeMode_m1x1();
}

void ScrollJumpRider_m1x1(void);
void DispatchByDistance_m1x1(void);
void SlotRiderShadow_m1x1(void);

/* $81:C56E (RTS): park the jump rider at X $38, ground $B8. */
void ParkJumpRider_m1x1(void)
{
    lda8(0x38); wr8(ABS(0x0196), 0x38);
    lda8(0xB8); wr8(ABS(0x01B6), 0xB8);
    ret();
}

/* $80:AC04 (RTL): RiderThrottle with DB = $80. */
void RiderThrottleLong_m1x1(void)
{
    push8(DB);
    lda8(0x80); push8(0x80);
    DB = pull8(); nz8(DB);
    push16(0xAC0B); RiderThrottle_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:BCEF (RTL): AddSpeedToX for other banks. */
void AddSpeedToXLong_m1x1(void)
{
    push16(0xBCF1); AddSpeedToX_m1x1();
    retl();
}

/* $80:CD0A (RTL): DrawRiderShadowPair for other banks. */
void DrawRiderShadowPairLong_m1x1(void)
{
    push16(0xCD0C); DrawRiderShadowPair_m1x1();
    retl();
}

/* $81:C483 (RTS): the jump contest rider. In the run ($197F = 2) the
 * player rides (on the track, A held: throttle, move, scroll); otherwise
 * (or with $56) it is parked. Then it is drawn with its shadow, and its
 * pose $1961: by $197F off the run ($81:C469), by $56 while landing
 * ($80:CB58, $A8), else by the run frame count $1981 against $81:C473
 * (the run-up poses $81:C46B, walking at speed $0080 with the wheel
 * spark sprites $16C0 cycling through $1987; past the table speed $0500)
 * and the distance step. */
void JumpRiderStep_m1x1(void)
{
    lda8(rd8(ABS(0x197F)));
    cmp8((u8)A, 0x02);
    if (!fz) {
        push16(0xC48C); ParkJumpRider_m1x1();
    } else {
        lda8(0x01); wr8(ABS(0x01DE), 0x01);
        lda8(0x80); wr8(ABS(0x017E), 0x80);
        ldx8(0x00);
        push8(0x81); push16(0xC49E); RiderThrottleLong_m1x1();
        push8(0x81); push16(0xC4A2); AddSpeedToXLong_m1x1();
        ldy8(0x00);
        push16(0xC4A7); ScrollJumpRider_m1x1();
    }
    lda8(rd8(DP(0x56)));
    if (!fz) { push16(0xC4AE); ParkJumpRider_m1x1(); }
    ldx8(0x00);
    wr8(ABS(0x0950), 0);
    push8(0x81); push16(0xC4B7); DrawAllRidersLong_m1x1();
    ldy8(0x00);
    push16(0xC4BC); SlotRiderShadow_m1x1();
    lda8(rd8(ABS(0x197F)));
    cmp8((u8)A, 0x02);
    if (!fz) {
        X = rd8(ABS(0x197F)); nz8((u8)X);
        lda8(rd8(ABSX(0xC469)));
        wr8(ABS(0x1961), (u8)A);
        ret();
        return;
    }
    lda8(rd8(DP(0x56)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (!fz) {
            lda8(dec8((u8)A));
            ora8(0x10);
            wr8(DP(0xA8), (u8)A);
            X = (u8)A; nz8((u8)X);
            lda8(rd8(LONGX(0x80CB58)));
        }
        wr8(ABS(0x1961), (u8)A);
        ret();
        return;
    }
    rep(0x20);
    wr16(ABS(0x1981), inc16(rd16(ABS(0x1981))));
    ldx8(0x00);
    lda16(rd16(ABS(0x1981)));
    int past = 0;
    for (;;) {
        cmp16(A, rd16(ABSX(0xC473)));
        if (!fc) break;
        X = inc8((u8)X);
        X = inc8((u8)X);
        cmp8((u8)X, 0x10);
        if (fz) { past = 1; break; }
    }
    if (past) {
        sep(0x20);
        lda8(0x00); wr8(ABS(0x01BE), 0x00);
        lda8(0x05); wr8(ABS(0x01C6), 0x05);
    } else {
        sep(0x20);
        lda8((u8)X);
        lda8(lsr8((u8)A));
        X = (u8)A; nz8((u8)X);
        if (fz) { lda8(rd8(DP(0x63))); and8(0x01); }
        else lda8(rd8(ABSX(0xC46B)));
        wr8(ABS(0x1961), (u8)A);
        lda8(0x80); wr8(ABS(0x01BE), 0x80);
        lda8(0x00); wr8(ABS(0x01C6), 0x00);
        lda8(rd8(ABS(0x019E)));
        cmp8((u8)A, 0x02);
        if (!fc) {
            ldx8(0x00);
            Y = X; nz8((u8)Y);
            lda8(rd8(ABS(0x0196)));
            push8(0x81); push16(0xC52C); DrawRiderShadowPairLong_m1x1();
            wr8(ABS(0x1987), inc8(rd8(ABS(0x1987))));
            lda8(rd8(ABS(0x1987)));
            cmp8((u8)A, 0x01);
            if (fz) {
                lda8(0xF0); wr8(ABSY(0x16C2), 0xF0);
                lda8(0xE0); wr8(ABSY(0x16C6), 0xE0);
            } else {
                cmp8((u8)A, 0x02);
                if (fz) {
                    lda8(0xF0); wr8(ABSY(0x16C1), 0xF0); wr8(ABSY(0x16C5), 0xF0);
                } else wr8(ABS(0x1987), 0);
            }
            lda8(0x2F); wr8(ABSY(0x16C3), 0x2F); wr8(ABSY(0x16C7), 0x2F);
        }
    }
    push16(0xC56C); DispatchByDistance_m1x1();
    ret();
}

void DrawJumpCrowd_m1x1(void);
void JumpRiderStep_m1x1(void);
void AddSpeedToXLong_m1x1(void);
void ScrollByPlayerSpeedLong_m1x1(void);
void SpinWheels8_m1x1(void);
void LoadRaceColumn_m1x1(void);

/* $81:C2D9 (RTS): the jump run's start: once racing ($92), the run length
 * $1985 for this entry ($81:C2C9 by $7C, +8 with the debug flag), speed
 * $0500 and the next run step ($197F). Then the crowd and the rider. */
void JumpRunStart_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(0x92)));
    if (!fz) {
        wr16(DP(0x00), 0);
        lda16(rd16(LONG(0x81800C)));
        if (!fz) { lda16(0x0008); wr16(DP(0x00), 0x0008); }
        lda16(rd16(DP(0x7C)));
        and16(0x00FF);
        lda16(asl16(A));
        fc = 0; adc16(rd16(DP(0x00)));
        X = (u8)A; nz8((u8)X);
        lda16(rd16(LONGX(0x81C2C9)));
        wr16(ABS(0x1985), A);
        sep(0x20);
        lda8(0x00); wr8(ABS(0x01BE), 0x00);
        lda8(0x05); wr8(ABS(0x01C6), 0x05);
        wr8(ABS(0x197F), inc8(rd8(ABS(0x197F))));
    }
    sep(0x20);
    push16(0xC310); DrawJumpCrowd_m1x1();
    push16(0xC313); JumpRiderStep_m1x1();
    ret();
}

/* $81:C5AA (RTS): the player moves and the screen follows. */
void MoveAndScroll_m1x1(void)
{
    ldx8(0x00);
    push8(0x81); push16(0xC5AF); AddSpeedToXLong_m1x1();
    push8(0x81); push16(0xC5B3); ScrollByPlayerSpeedLong_m1x1();
    ret();
}

/* $81:C331 (RTS): the jump run. Until the run length $1985 the track
 * streams ($81:8A4D), the rider moves, the crowd and wheels animate.
 * At the end, unless the time is short ($88 < $0654), the landing
 * banner ($81:C315, $0207 at VRAM $194D), speed $0080, $56 = 3, sound
 * $45, $198A = $30 and the next step. Then the rider. */
void JumpRun_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(0x9C)));
    cmp16(A, rd16(ABS(0x1985)));
    if (!fc) {
        sep(0x20);
        push8(0x81); push16(0xC383); LoadRaceColumn_m1x1();
        push16(0xC386); MoveAndScroll_m1x1();
        push16(0xC389); DrawJumpCrowd_m1x1();
        push16(0xC38C); SpinWheels8_m1x1();
    } else {
        lda16(rd16(DP(0x88)));
        cmp16(A, 0x0654);
        if (fc) {
            wr16(ABS(0x1981), 0);
            wr16(ABS(0x1988), 0);
            lda16(0x194D); wr16(DP(0x00), 0x194D);
            lda16(0xC315); wr16(DP(0x02), 0xC315);
            lda16(0x0207); wr16(DP(0x10), 0x0207);
            sep(0x20);
            lda8(0x81); wr8(DP(0x04), 0x81);
            push8(0x81); push16(0xC35F); QueueTileRowsLong_m1x1();
            lda8(0x80); wr8(ABS(0x01BE), 0x80);
            lda8(0x00); wr8(ABS(0x01C6), 0x00);
            lda8(0x03); wr8(DP(0x56), 0x03);
            lda8(0x45);
            push8(0x81); push16(0xC373); QueueSoundB_m1x1();
            lda8(0x30); wr8(ABS(0x198A), 0x30);
            wr8(ABS(0x197F), inc8(rd8(ABS(0x197F))));
        }
    }
    sep(0x20);
    push16(0xC391); JumpRiderStep_m1x1();
    ret();
}

void QueueBlinkBanner_m1x1(void);

/* $80:8F10 (RTL): ChangeMode for other banks. */
void ChangeModeLong_m1x1(void)
{
    push16(0x8F12); ChangeMode_m1x1();
    retl();
}

/* $81:C3B1 (RTS): leave the jump contest for mode 3. */
void LeaveJumpContest_m1x1(void)
{
    lda8(0x03); wr8(DP(0x76), 0x03);
    push8(0x81); push16(0xC3B8); ChangeModeLong_m1x1();
    ret();
}

/* $81:C29A (RTL): the jump contest's run steps: Start with the debug flag
 * leaves; otherwise $88 counts up and the run step $197F runs through
 * $81:C2C1 (start, run, ...), then the banner. */
void JumpRunSteps_m1x1(void)
{
    lda8(rd8(LONG(0x81800E)));
    if (!fz) {
        lda8(rd8(DP(0x21)));
        and8(0x10);
        if (!fz) { push16(0xC2A8); LeaveJumpContest_m1x1(); retl(); return; }
    }
    push8(DB);
    push8(0x81);
    DB = pull8(); nz8(DB);
    rep(0x20);
    wr16(DP(0x88), inc16(rd16(DP(0x88))));
    sep(0x20);
    lda8(rd8(ABS(0x197F)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    push16(0xC2BB);
    r816_call_pc(0x810000u | rd16(LONGX(0x81C2C1)));
    push16(0xC2BE); QueueBlinkBanner_m1x1();
    DB = pull8(); nz8(DB);
    retl();
}

/* $80:B7F1 (RTS): the jump contest's run steps (mode entry). */
void JumpRunStepsMode_m1x1(void)
{
    push8(0x80); push16(0xB7F4); JumpRunSteps_m1x1();
    ret();
}

void BlankQueueRowPal2_m1x1(void);
void CopyTrackColumn_m1x1(void);
void FillLaneColumn_m1x1(void);

/* $80:FE5A (RTS): A + 1, wrapping at $40. */
void NextColumn_m1x1(void)
{
    fc = 0; adc8(0x01);
    cmp8((u8)A, 0x40);
    if (fz) lda8(0x00);
    ret();
}

/* $80:FE56 (RTL): NextColumn for other banks. */
void NextColumnLong_m1x1(void)
{
    push16(0xFE58); NextColumn_m1x1();
    retl();
}

/* $81:8A42 (RTL, X = row type): the row's piece list ($05) from
 * $81:8010 / $81:8040 (DB = $81). */
void RaceRowList_m1x1(void)
{
    lda8(rd8(ABSX(0x8010))); wr8(DP(0x05), (u8)A);
    lda8(rd8(ABSX(0x8040))); wr8(DP(0x06), (u8)A);
    retl();
}

/* $81:8A4D (RTL): load the race course one column ahead (DB = $81). The
 * expanded course $7F:4000 ([$07]) holds the lap count at entry 0 ($0C in
 * a race, $7D clear; $0928 + 1, $AE cleared) and then row entries: with a
 * $81:8070 bit a row type from $81:8A30 by the low nibble, else the low
 * seven bits. Each time the builder column $0279 reaches the player
 * column $0266 the row list ($81:8A42) gives the next column: blanked,
 * its entry recorded at $7F:6000 + $AE, filled (CopyTrackColumn), on the
 * finish row ($09) the lap digits ($81:8A3D by $0929, then the lap $0928
 * as 1 and its units) at $7F:0026, and queued for VRAM at $0277 - $20
 * (FillLaneColumn; $0277 wraps at $20 with $0278 ^ 4). At the end of a
 * finish row the next lap starts ($03AC cleared, $0928 + 1, $093F - 1,
 * $098A = 1 on the last; the course restarts, $B0 = $AE). */
void LoadRaceColumn_m1x1(void)
{
    push8(DB);
    lda8(0x81); push8(0x81);
    DB = pull8(); nz8(DB);
    lda8(rd8(DP(0x7C)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(0x00); wr8(DP(0x07), 0x00);
    lda8(0x40); wr8(DP(0x08), 0x40);
    lda8(0x7F); wr8(DP(0x09), 0x7F);
    lda8(rd8(ABS(0x0279)));
    cmp8((u8)A, rd8(ABS(0x0266)));
    if (!fz) goto out;
    lda8(rd8(ABS(0x027E)));
    if (!fz) goto column;
next_entry:
    WAIT_TICK();
    rep(0x10);
    Y = rd16(ABS(0x027B)); nz16(Y);
    if (fz) {
        lda8(rd8(ILDPY(0x07)));
        wr8(ABS(0x093F), (u8)A);
        wr8(ABS(0x0928), inc8(rd8(ABS(0x0928))));
        wr8(DP(0xAE), 0);
        wr8(DP(0xAF), 0);
        lda8(rd8(DP(0x7D)));
        if (fz) { lda8(0x0C); wr8(ABS(0x093F), 0x0C); }
    }
read_entry:
    rep(0x20);
    wr16(ABS(0x027B), inc16(rd16(ABS(0x027B))));
    sep(0x20);
    if (fx) { Y = rd8(ABS(0x027B)); nz8((u8)Y); }
    else { Y = rd16(ABS(0x027B)); nz16(Y); }
    lda8(rd8(ILDPY(0x07)));
    if (fz) goto out;
    sep(0x10);
    bit8(rd8(ABS(0x8070)), 0);
    if (!fz) {
        and8(0x0F);
        X = (u8)A; nz8((u8)X);
        lda8(rd8(ABSX(0x8A30)));
        wr8(ABS(0x027D), (u8)A);
    } else {
        sep(0x10);
        ldx8(0x01);
        and8(0x7F);
        wr8(ABS(0x027D), (u8)A);
    }
column:
    X = rd8(ABS(0x027D)); nz8((u8)X);
    push8(0x81); push16(0x8AC1); RaceRowList_m1x1();
    Y = rd8(ABS(0x027E)); nz8((u8)Y);
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
    if (fz) {
        wr8(ABS(0x027E), (u8)A);
        lda8(rd8(ABS(0x027D)));
        cmp8((u8)A, 0x09);
        if (!fz) goto next_entry;
        wr8(ABS(0x03AC), 0);
        wr8(ABS(0x0928), inc8(rd8(ABS(0x0928))));
        wr8(ABS(0x093F), dec8(rd8(ABS(0x093F))));
        if (fz) { lda8(0x01); wr8(ABS(0x098A), 0x01); }
        lda8(0x00);
        wr8(ABS(0x027B), 0);
        wr8(ABS(0x027C), 0);
        wr8(ABS(0x0929), 0);
        rep(0x20);
        lda16(rd16(DP(0xAE)));
        wr16(DP(0xB0), A);
        wr16(DP(0xAE), 0);
        sep(0x20);
        goto read_entry;
    }
    push8(0x81); push16(0x8ACF); BlankQueueRowPal2_m1x1();
    lda8(rd8((((u32)DB << 16) + rd16(DP(0x05)) + Y) & 0xFFFFFF));
    push8((u8)A);
    rep(0x30);
    X = rd16(DP(0xAE)); nz16(X);
    lda16(rd16(ABS(0x027B)));
    wr16(LONGX(0x7F6000), A);
    wr16(DP(0xAE), inc16(rd16(DP(0xAE))));
    wr16(DP(0xAE), inc16(rd16(DP(0xAE))));
    sep(0x30);
    X = pull8(); nz8((u8)X);
    push8(0x81); push16(0x8AE8); CopyTrackColumn_m1x1();
    Y = inc8((u8)Y);
    Y = inc8((u8)Y);
    wr8(ABS(0x027E), (u8)Y);
    lda8(rd8(ABS(0x027D)));
    cmp8((u8)A, 0x09);
    if (fz) {
        lda8(rd8(ABS(0x093F)));
        cmp8((u8)A, 0x01);
        if (!fz) {
            cmp8((u8)Y, 0x4A);
            if (fc) {
                cmp8((u8)Y, 0xB0);
                if (!fc) {
                    X = rd8(ABS(0x0929)); nz8((u8)X);
                    lda8(rd8(ABSX(0x8A3D)));
                    wr8(LONG(0x7F0026), (u8)A);
                    wr8(ABS(0x0929), inc8(rd8(ABS(0x0929))));
                    cmp8((u8)X, 0x04);
                    if (fc) {
                        if (fz) {
                            lda8(rd8(ABS(0x0928)));
                            cmp8((u8)A, 0x0A);
                            if (fc) { lda8(0x01); wr8(LONG(0x7F0026), 0x01); }
                        } else {
                            lda8(rd8(ABS(0x0928)));
                            cmp8((u8)A, 0x0A);
                            if (fc) { fc = 1; sbc8(0x0A); }
                            wr8(LONG(0x7F0026), (u8)A);
                        }
                    }
                }
            }
        }
    }
    lda8(0x81); wr8(LONG(0x7F0004), 0x81);
    lda8(0x01); wr8(LONG(0x7F0005), 0x01);
    lda8(0x18); wr8(LONG(0x7F0006), 0x18);
    lda8(0x02);
    fc = 0; adc8(0x0C);
    wr8(LONG(0x7F0007), (u8)A);
    lda8(0x00); wr8(LONG(0x7F0008), 0);
    lda8(0x7F); wr8(LONG(0x7F0009), 0x7F);
    rep(0x20);
    lda16(0x0002);
    fc = 0; adc16(0x0028);
    wr16(LONG(0x7F0000), A);
    wr16(LONG(0x7F000C), A);
    lda16(rd16(ABS(0x0277)));
    fc = 1; sbc16(0x0020);
    wr16(LONG(0x7F0002), A);
    lda16(0x001C); wr16(LONG(0x7F000A), 0x001C);
    sep(0x20);
    lda8(rd8(ABS(0x0276)));
    wr8(DP(0x0F), (u8)A);
    push8(0x81); push16(0x8B88); FillLaneColumn_m1x1();
    lda8(rd8(ABS(0x0276)));
    push8(0x81); push16(0x8B8F); NextColumnLong_m1x1();
    wr8(ABS(0x0276), (u8)A);
    wr8(ABS(0x0277), inc8(rd8(ABS(0x0277))));
    lda8(rd8(ABS(0x0277)));
    cmp8((u8)A, 0x20);
    if (fz) {
        lda8(0x00); wr8(ABS(0x0277), 0);
        lda8(rd8(ABS(0x0278)));
        eor8(0x04);
        wr8(ABS(0x0278), (u8)A);
    }
    lda8(rd8(ABS(0x0266)));
    push8(0x81); push16(0x8BB0); NextColumnLong_m1x1();
    wr8(ABS(0x0279), (u8)A);
    DB = pull8(); nz8(DB);
    retl();
    return;
out:
    sep(0x10);
    DB = pull8(); nz8(DB);
    retl();
}

void BlankQueueRow_m1x1(void);
void CopyPieceColumnPal2_m1x1(void);
void CourseColumn_m1x1(void);

/* $80:DBF1 (RTS, X = row type): the row piece list ($05) from
 * $80:D2FB / $80:D304. */
void CourseRowList_m1x1(void)
{
    lda8(rd8(ABSX(0xD2FB))); wr8(DP(0x05), (u8)A);
    lda8(rd8(ABSX(0xD304))); wr8(DP(0x06), (u8)A);
    ret();
}

static u8 course_list4(u32 dp)
{
    return rd8((((u32)DB << 16) + rd16(DP(dp)) + Y) & 0xFFFFFF);
}

/* $80:DA77 (RTS): build the course one column ahead. The course script
 * ($07 from $80:D270 / $80:D272 by the header flag $81:800C) starts with
 * the scenery byte $09C9. Each time the builder column $0279 reaches the
 * player column $0266: the next script entry (0 = stop, 9 = loop to
 * entry 1 as row type 1, $80+ = that many empty rows, else a row type
 * $027D) and its row list ($80:DBF1) give the column pieces; a column
 * is blanked, filled (CopyPieceColumnPal2) and queued for VRAM at $0277 + $07E0 (row
 * pointer $0276, column $0277 wrapping $80 -> $60 with $0278 ^ 4), and
 * the builder column steps. */
void BuildCourseColumn_m1x1(void)
{
    lda8(rd8(LONG(0x81800C)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0xD270))); wr8(DP(0x07), (u8)A);
    lda8(rd8(ABSX(0xD272))); wr8(DP(0x08), (u8)A);
    Y = rd8(ABS(0x027B)); nz8((u8)Y);
    if (fz) {
        lda8(course_list4(0x07));
        wr8(ABS(0x09C9), (u8)A);
        Y = inc8((u8)Y);
        wr8(ABS(0x027B), (u8)Y);
    }
    lda8(rd8(ABS(0x0279)));
    cmp8((u8)A, rd8(ABS(0x0266)));
    if (!fz) { ret(); return; }
    int step;   /* 0 = next entry, 1 = counting empty rows, 2 = same row */
    lda8(rd8(ABS(0x027A)));
    if (!fz) step = 1;
    else {
        lda8(rd8(ABS(0x027E)));
        step = fz ? 0 : 2;
    }
    for (;;) {
        WAIT_TICK();
        if (step == 0) {
            Y = rd8(ABS(0x027B)); nz8((u8)Y);
            wr8(ABS(0x027B), inc8(rd8(ABS(0x027B))));
            lda8(course_list4(0x07));
            if (fz) { ret(); return; }
            cmp8((u8)A, 0x09);
            if (fz) {
                lda8(0x01);
                wr8(ABS(0x027B), 0x01);
            }
            cmp8((u8)A, 0x80);
            if (fc) {
                and8(0x7F);
                wr8(ABS(0x027A), (u8)A);
                step = 1;
            } else {
                wr8(ABS(0x027D), (u8)A);
                step = 2;
            }
        }
        if (step == 1) {
            wr8(ABS(0x027A), dec8(rd8(ABS(0x027A))));
            lda8(0x00);
            wr8(ABS(0x09C3), 0);
            wr8(ABS(0x027E), 0);
            wr8(ABS(0x027D), 0);
        }
        /* L80DBE5 */
        X = rd8(ABS(0x027D)); nz8((u8)X);
        push16(0xDAD8); CourseRowList_m1x1();
        Y = rd8(ABS(0x027E)); nz8((u8)Y);
        lda8(course_list4(0x05));
        if (!fz) break;
        wr8(ABS(0x027E), (u8)A);
        wr8(ABS(0x09C3), (u8)A);
        step = 0;
    }
    push16(0xDAE5); BlankQueueRow_m1x1();
    lda8(course_list4(0x05));
    X = (u8)A; nz8((u8)X);
    push16(0xDAEB); CopyPieceColumnPal2_m1x1();
    Y = inc8((u8)Y);
    Y = inc8((u8)Y);
    wr8(ABS(0x027E), (u8)Y);
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
    lda16(rd16(ABS(0x0277)));
    fc = 0; adc16(0x07E0);
    wr16(LONG(0x7F0002), A);
    lda16(0x001E); wr16(LONG(0x7F000A), 0x001E);
    sep(0x20);
    lda8(rd8(ABS(0x0276)));
    wr8(DP(0x0F), (u8)A);
    push16(0xDB47); CourseColumn_m1x1();
    lda8(rd8(ABS(0x0276)));
    push16(0xDB4D); NextColumn_m1x1();
    wr8(ABS(0x0276), (u8)A);
    wr8(ABS(0x0277), inc8(rd8(ABS(0x0277))));
    lda8(rd8(ABS(0x0277)));
    cmp8((u8)A, 0x80);
    if (fz) {
        lda8(0x60); wr8(ABS(0x0277), 0x60);
        lda8(rd8(ABS(0x0278)));
        eor8(0x04);
        wr8(ABS(0x0278), (u8)A);
    }
    lda8(rd8(ABS(0x0266)));
    push16(0xDB6D); NextColumn_m1x1();
    wr8(ABS(0x0279), (u8)A);
    ret();
}


void ClearOnBonusTiles_m1x1(void);
void RiderPieceAt_m1x1(void);

/* $80:F850 (RTS, A = action): jump through the rider piece action table
 * $80:F6DF. Not registered on its own: difftest feeds it out-of-range
 * actions, which read past the table. */
static void piece_action4(void)
{
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xF6DF))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xF6E0))); wr8(DP(0x01), (u8)A);
    if (!r816_call_pc(0x800000u | rd16(LONG(0x000000))))
        r816_unknown_target(0x80F85C, rd16(LONG(0x000000)));
}

/* ($0A),Y in bank DB. */
static u8 script_byte4(void)
{
    return rd8((((u32)DB << 16) + rd16(DP(0x0A)) + Y) & 0xFFFFFF);
}

/* $80:F78E RunRiderPieces (RTS, X = rider): the rider's piece script.
 * Its distance $024E advances by the speed $0186. With no script running
 * ($0176 = 0) a piece type $021E below $1A starts script type + 1 (a $1A+
 * piece clears bonus tiles and, in jump state 3 in lanes 0-2, starts $0C
 * + 1; otherwise the player's $1979 clears). In a free run ($7D set) a
 * rider outside X $00B0-$026F instead drops out of the script and its
 * jump (lane height $80:F8CD). Then $0256 = 0 and $024E = $018E - 1, and
 * the script ($80:F536 / $80:F54F) runs pairs (distance, action) up to
 * $024E: an action with a $80:F6DE bit sets the angle ($01E6, $01A6 from
 * $80:F6D3) on the ground; one with a $80:F6DD bit sets $0226 / $023E;
 * others go through PieceAction ($80:F850). At the $FF end the script
 * stops ($0176, $01A6 cleared, $01FE cleared on the ground outside jump
 * state 1, state 2 -> 3) and $08BC clears. */
void RunRiderPieces_m1x1(void)
{
    lda8(rd8(ABSX(0x0186)));
    fc = 0; adc8(rd8(ABSX(0x024E)));
    wr8(ABSX(0x024E), (u8)A);
    lda8(rd8(ABSX(0x0176)));
    if (fz) {
        lda8(rd8(ABSX(0x021E)));
        cmp8((u8)A, 0x1A);
        if (fc) {
            push8(0x80); push16(0xF7A7); ClearOnBonusTiles_m1x1();
            int start = 0;
            lda8(rd8(ABSX(0x020E)));
            cmp8((u8)A, 0x03);
            if (fz) {
                lda8(rd8(ABSX(0x08A4)));
                cmp8((u8)A, 0x03);
                if (!fc) {
                    lda8(0x0C);
                    wr8(ABSX(0x021E), 0x0C);
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
        wr8(ABSX(0x0176), (u8)A);
        wr8(ABSX(0x0176), inc8(rd8(ABSX(0x0176))));
        lda8(rd8(DP(0x7D)));
        if (!fz) {
            lda8(rd8(ABSX(0x019E)));
            A = (u16)((A >> 8) | (A << 8)); nz8((u8)A);
            lda8(rd8(ABSX(0x0196)));
            rep(0x20);
            int out = 0;
            cmp16(A, 0x00B0);
            if (!fc) out = 1;
            else {
                cmp16(A, 0x0270);
                out = fc;
            }
            if (out) {
                sep(0x20);
                wr8(ABSX(0x0226), 0);
                wr8(ABSX(0x01A6), 0);
                wr8(ABSX(0x0981), 0);
                wr8(ABSX(0x0176), 0);
                wr8(ABSX(0x01FE), 0);
                Y = rd8(ABSX(0x08A4)); nz8((u8)Y);
                lda8(rd8(ABSY(0xF8CD)));
                wr8(ABSX(0x01F6), (u8)A);
                wr8(ABSX(0x01D6), 0);
                wr8(ABSX(0x199E), 0);
                wr8(ABSX(0x01AE), 0);
                ret();
                return;
            }
        }
        sep(0x20);
        lda8(0x00);
        wr8(ABSX(0x0256), 0);
        lda8(rd8(ABSX(0x018E)));
        wr8(ABSX(0x024E), (u8)A);
        wr8(ABSX(0x024E), dec8(rd8(ABSX(0x024E))));
    }
    for (;;) {
        /* L80F816 */
        Y = rd8(ABSX(0x0176)); nz8((u8)Y);
        lda8(rd8(ABSY(0xF536))); wr8(DP(0x0A), (u8)A);
        lda8(rd8(ABSY(0xF54F))); wr8(DP(0x0B), (u8)A);
        Y = rd8(ABSX(0x0256)); nz8((u8)Y);
        lda8(script_byte4());
        cmp8((u8)A, 0xFF);
        if (fz) break;
        wr8(DP(0x0F), (u8)A);
        cmp8((u8)A, rd8(ABSX(0x024E)));
        if (!fz && fc) { ret(); return; }
        Y = inc8((u8)Y);
        lda8(script_byte4());
        bit8(rd8(ABS(0xF6DE)), 0);
        if (!fz) {
            /* L80F85F */
            lda8(rd8(ABSX(0x01EE)));
            ora8(rd8(ABSX(0x01CE)));
            if (fz) {
                lda8(script_byte4());
                and8(0x0F);
                wr8(DP(0x00), (u8)A);
                lda8(rd8(ABSX(0x020E)));
                cmp8((u8)A, 0x01);
                if (!fz) {
                    lda8(rd8(DP(0x00)));
                    wr8(ABSX(0x01E6), (u8)A);
                    lda8(rd8(ABSX(0x0176)));
                    cmp8((u8)A, 0x03);
                    if (!fz) {
                        lda8(rd8(ABSX(0x01E6)));
                        fc = 1; sbc8(0x02);
                        Y = (u8)A; nz8((u8)Y);
                        lda8(rd8(ABSY(0xF6D3)));
                        wr8(ABSX(0x01A6), (u8)A);
                    }
                }
            }
        } else {
            bit8(rd8(ABS(0xF6DD)), 0);
            if (fz) {
                push16(0xF847); piece_action4();
                wr8(ABSX(0x0256), inc8(rd8(ABSX(0x0256))));
                wr8(ABSX(0x0256), inc8(rd8(ABSX(0x0256))));
                if (!fz) continue;
                piece_action4();
                return;
            }
            /* L80F8C1 */
            and8(0x1F);
            wr8(ABSX(0x0226), (u8)A);
            lda8(rd8(DP(0x0F)));
            wr8(ABSX(0x023E), (u8)A);
            if (fn) {
                /* falls into the table bytes at $80:F8CD, decoded as code,
                 * and on into RiderPieceAt */
                and8(rd8((((u32)DB << 16) + rd16(DP(0x32))) & 0xFFFFFF));
                { u32 ea = DP(0x1A); wr8(ea, rol8(rd8(ea))); }
                lda8(inc8((u8)A));
                { u32 ea = ABS(0xA4BC); wr8(ea, asl8(rd8(ea))); }
                push8(getP());
                RiderPieceAt_m1x1();
                return;
            }
        }
        /* L80F88D */
        wr8(ABSX(0x0256), inc8(rd8(ABSX(0x0256))));
        wr8(ABSX(0x0256), inc8(rd8(ABSX(0x0256))));
        if (fz) break;
    }
    /* L80F898 */
    lda8(0x00);
    wr8(ABSX(0x0176), 0);
    wr8(ABSX(0x01A6), 0);
    lda8(rd8(ABSX(0x0206)));
    if (fz) {
        lda8(rd8(ABSX(0x020E)));
        cmp8((u8)A, 0x01);
        if (!fz) {
            lda8(0x00);
            wr8(ABSX(0x01FE), 0);
            lda8(rd8(ABSX(0x020E)));
            cmp8((u8)A, 0x02);
            if (fz) wr8(ABSX(0x020E), inc8(rd8(ABSX(0x020E))));
        }
    }
    lda8(0x00);
    wr8(ABSX(0x08BC), 0);
    ret();
}

void RidersThrottle_m1x1(void);
void SpinWheels_m1x1(void);
void TurboMeter_m1x1(void);
void RiderShadowY_m1x1(void);
void GroundHeightAbove_m1x1(void);
void SortRidersByPosition_m1x1(void);
void DrawAllRiders_m1x1(void);

/* $80:ABC7 (RTS): while racing ($92), RidersThrottle for all eight riders;
 * otherwise only its closing check (unpaused, the player holding A/B and
 * $092D, $84 clear: $092D = 1). */
void ThrottleAll_m1x1(void)
{
    lda8(rd8(DP(0x92)));
    if (!fz) {
        ldx8(0x07);
        RidersThrottle_m1x1();
        return;
    }
    lda8(rd8(DP(0x54)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x017E)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x092D)));
    ora8(rd8(DP(0x84)));
    if (!fz) { ret(); return; }
    lda8(0x01); wr8(ABS(0x092D), 0x01);
    ret();
}

/* $80:BB46 (RTS, X = rider): the rider's state step $01D6 through the
 * table $80:BB1F / $80:BB25. */
void RiderStateStep_m1x1(void)
{
    Y = rd8(ABSX(0x01D6)); nz8((u8)Y);
    lda8(rd8(ABSY(0xBB1F))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xBB25))); wr8(DP(0x01), (u8)A);
    if (!r816_call_pc(0x800000u | rd16(LONG(0x000000))))
        r816_unknown_target(0x80BB53, rd16(LONG(0x000000)));
}

/* $80:BB3A (RTS): for riders 7-0 the state step and the shadow height. */
void RiderStepAll_m1x1(void)
{
    ldx8(0x07);
    do {
        push16(0xBB3E); RiderStateStep_m1x1();
        push16(0xBB41); RiderShadowY_m1x1();
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:CA93 (RTS): sort the riders by position, then draw all eight. */
void SortAndDrawRiders_m1x1(void)
{
    push16(0xCA95); SortRidersByPosition_m1x1();
    ldx8(0x07);
    DrawAllRiders_m1x1();
}

/* $80:F755 (RTS): SpinWheels for all eight riders. */
void SpinAllWheels_m1x1(void)
{
    ldx8(0x07);
    SpinWheels_m1x1();
}

/* $80:F786 (RTS): the piece scripts of riders 7-1, then the player's. */
void RunAllRiderPieces_m1x1(void)
{
    ldx8(0x07);
    do {
        push16(0xF78A); RunRiderPieces_m1x1();
        X = dec8((u8)X);
    } while (!fz);
    RunRiderPieces_m1x1();
}

/* $80:F931 (RTS, X = last rider): riders X-0 on the ground get their
 * height $01B6 from GroundHeightAbove. */
void RiderGroundHeightsFrom_m1x1(void)
{
    do {
        lda8(rd8(ABSX(0x01EE)));
        if (fz) {
            push16(0xF938); GroundHeightAbove_m1x1();
            wr8(ABSX(0x01B6), (u8)A);
        }
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:F92F (RTS): the same for all eight riders. */
void RiderGroundHeights_m1x1(void)
{
    ldx8(0x07);
    RiderGroundHeightsFrom_m1x1();
}

/* $80:FBF5 (RTS): for riders 7-0 with a pending piece action $0226, run
 * it (PieceAction). */
void RunPieceActions_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8(rd8(ABSX(0x0226)));
        if (!fz) { push16(0xFBFE); piece_action4(); }
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* $80:F32D (RTS): TurboMeter. */
void TurboMeterJmp_m1x1(void)
{
    TurboMeter_m1x1();
}
