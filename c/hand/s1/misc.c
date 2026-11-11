/*
 * Stadium 1: random numbers, timers, the clock display and the waiting modes.
 * Hand-written; arithmetic goes through the original's ADC/SBC/CMP so carry,
 * overflow and decimal mode behave exactly as before.
 */
#include "hand/hand.h"

void VramQueueTiles20_m1x1(void);

/*
 * $81:A1E5 Random (M=1, X=1, RTL). Mixes the PPU's latched H/V counters,
 * the frame counter $63/$64 and the broadcast clock into A. The first ADC
 * uses whatever carry the caller left.
 */
void Random_m1x1(void)
{
    lda8(rd8(ABS(0x2137)));                 /* SLHV: latch the counters */
    lda8(rd8(ABS(0x213F)));                 /* STAT78: reset the read flip-flops */
    adc8(rd8(ABS(0x213C)));                 /* OPHCT low, high */
    adc8(rd8(ABS(0x213C)));
    adc8(rd8(ABS(0x213D)));                 /* OPVCT low, high */
    adc8(rd8(ABS(0x213D)));
    eor8(rd8(DP(0x63)));
    adc8(rd8(DP(0x64)));
    adc8(rd8(LONG(0x7FFFFF)));              /* minute */
    adc8(rd8(LONG(0x7FFFFA)));              /* frame within the minute */
    adc8(rd8(LONG(0x7FFFFB)));
    retl();
}

/*
 * $80:B498 TickTimers (M=1, X=1, RTS). Counts down the byte timers at
 * $39-$4F every frame; every 11th frame ($38 wraps) also $50-$62. Timers
 * stop at zero.
 */
void TickTimers_m1x1(void)
{
    ldx8(0x16);
    u8 slow = dec8(rd8(DP(0x38)));
    wr8(DP(0x38), slow);
    if (fn) {
        lda8(0x0A);
        wr8(DP(0x38), 0x0A);
        ldx8(0x29);
    }
    do {
        lda8(rd8(DPX(0x39)));
        if (!fz)
            wr8(DPX(0x39), dec8(rd8(DPX(0x39))));
        X = dec8((u8)X);
    } while (!fn);
    ret();
}

/* Two-digit decimal split by repeated subtraction, as the original does it:
 * counts tens into *tens (already holding the tile base) and returns the
 * units with the tile base added. */
static u8 split_digits(u8 value, u32 tens_tile)
{
    lda8(value);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc)
            break;
        wr8(tens_tile, (u8)(rd8(tens_tile) + 1));
        fc = 1;
        sbc8(0x0A);
    }
    fc = 0;
    adc8(0x50);                             /* digit tiles start at $50 */
    return (u8)A;
}

/*
 * $80:819F DrawClock (M=1, X=1, RTS). When the clock is shown ($1556), every
 * 30 frames toggle the colon ($1E85) and redraw "HH:MM" as a 5x2 tile block
 * at VRAM $1842: the tile list at $1E1E is [size $25][top row 5][bottom row 5],
 * digit tiles $50-$59 on top and $60-$69 below, the colon $96/$A6 or blank $FC.
 */
void DrawClock_m1x1(void)
{
    lda8(rd8(ABS(0x1556)));
    if ((u8)A == 0) { ret(); return; }
    u8 t = dec8(rd8(ABS(0x1E84)));
    wr8(ABS(0x1E84), t);
    if (t != 0) { ret(); return; }

    lda8(rd8(ABS(0x1E85)));
    eor8(0x01);
    wr8(ABS(0x1E85), (u8)A);                /* colon on/off */
    wr8(ABS(0x1E84), 0x1E);                 /* next toggle in 30 frames */
    wr8(DP(0x00), 0x42);                    /* VRAM $1842 */
    wr8(DP(0x01), 0x18);
    wr8(ABS(0x1E1E), 0x25);                 /* 5 tiles wide, 2 rows */

    wr8(ABS(0x1E1F), 0x50);                 /* hours */
    u8 units = split_digits(rd8(LONG(0x7FFFF9)), ABS(0x1E1F));
    wr8(ABS(0x1E20), units);
    fc = 0; adc8(0x10); wr8(ABS(0x1E25), (u8)A);
    lda8(rd8(ABS(0x1E1F))); fc = 0; adc8(0x10); wr8(ABS(0x1E24), (u8)A);

    wr8(ABS(0x1E22), 0x50);                 /* minutes */
    units = split_digits(rd8(LONG(0x7FFFFF)), ABS(0x1E22));
    wr8(ABS(0x1E23), units);
    fc = 0; adc8(0x10); wr8(ABS(0x1E28), (u8)A);
    lda8(rd8(ABS(0x1E22))); fc = 0; adc8(0x10); wr8(ABS(0x1E27), (u8)A);

    lda8(rd8(ABS(0x1E85)));
    if ((u8)A == 0) {
        wr8(ABS(0x1E21), 0x96);             /* colon, top and bottom halves */
        wr8(ABS(0x1E26), 0xA6);
    } else {
        wr8(ABS(0x1E21), 0xFC);             /* blank */
        wr8(ABS(0x1E26), 0xFC);
    }
    lda8(0x1E);
    wr8(DP(0x02), 0x1E);                    /* tile list at $1E1E */
    wr8(DP(0x03), 0x1E);
    VramQueueTiles20_m1x1();                /* JMP: it returns for us */
}

/*
 * $80:8A46 Mode00_IntroWait (M=1, X=1, RTS). Waits for the race segment
 * (minute 4) or a timer ($88) before switching to the title (mode $0E, step 1).
 * With the debug flag $81:800E set, Start skips the wait.
 */
void Mode00_IntroWait_m1x1(void)
{
    int skip = 0;
    lda8(rd8(LONG(0x81800E)));
    if ((u8)A != 0) {
        lda8(rd8(DP(0x21)));
        and8(0x10);
        skip = (u8)A != 0;
    }
    if (!skip) {
        lda8(rd8(LONG(0x7FFFFF)));
        cmp8((u8)A, 0x04);
        if (!fc) {
            rep(0x20);
            wr16(DP(0x88), dec16(rd16(DP(0x88))));
            sep(0x20);
            if (!fz) { ret(); return; }
        }
    }
    lda8(0x0E);
    wr8(DP(0x76), 0x0E);
    wr8(DP(0x75), 0);
    lda8(0x01);
    wr8(DP(0x77), 0x01);
    ret();
}

/*
 * $80:8B00 Mode0E_TitleWait (M=1, X=1, RTS). At minute 4 the race segment
 * opens: switch to mode $0C (practice) and start fading out ($1923).
 */
void Mode0E_TitleWait_m1x1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x04);
    if (fc) {
        lda8(0x0C);
        wr8(DP(0x76), 0x0C);
        wr8(DP(0x75), 0);
        wr8(DP(0x77), 0);
        wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    }
    ret();
}
