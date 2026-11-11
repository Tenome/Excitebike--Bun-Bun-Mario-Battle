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

void QueueSound_m1x1(void);

/*
 * $80:8322 FadeStep (M=1, X=1, RTL), each frame while $1923 (fade speed,
 * signed) is non-zero. Brightness = INIDISP shadow low nibble + $1923:
 *   reaches 15  -> clamp, fade-in done ($1923 = 0);
 *   below 0     -> fade-out done: clear scroll $92-$95, $1958, $1E0C, sound
 *                  command $80, clock redraw next frame, forced blank,
 *                  APUIO2 and $1961-$1964 cleared, $1923 = 0.
 */
void FadeStep_m1x1(void)
{
    lda8(rd8(ABS(0x1903)));
    and8(0x0F);
    fc = 0;
    adc8(rd8(ABS(0x1923)));
    if (fn) {
        for (int a = 0x92; a <= 0x95; a++)
            wr8(DP(a), 0);
        wr8(ABS(0x1958), 0);
        wr8(ABS(0x1E0C), 0);
        lda8(0x80);
        push8(0x80); push16(0x8348); QueueSound_m1x1();
        lda8(0x01);
        wr8(ABS(0x1E84), 0x01);
        lda8(0x80);
        wr8(ABS(0x1903), 0x80);
        wr8(ABS(0x2142), 0);                    /* APUIO2 */
        for (int a = 0x1961; a <= 0x1964; a++)
            wr8(ABS(a), 0);
        wr8(ABS(0x1923), 0);
    } else {
        cmp8((u8)A, 0x0F);
        if (fc) {
            lda8(0x0F);
            wr8(ABS(0x1923), 0);
        }
    }
    wr8(ABS(0x1903), (u8)A);
    retl();
}

/*
 * $80:823C BroadcastEventInject (M=1, X=1, RTS), each frame from the main
 * loop. In a race (mode 3, not the special course), when the broadcast minute
 * changes ($7F:FFFF != $B7) to one of the event minutes in $80:82A7
 * (8, 9, 20, 21, 32, 33, 44, 45), place this course's item ($80:82AF[$08A4])
 * on the next plain-ground column ($40) ahead of the player ($0275 + 3), not
 * right after obstacles $17/$19/$1B/$03/$04. The course wraps at the $09 goal.
 */
void BroadcastEventInject_m1x1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, rd8(ABS(0x00B7)));
    if (fz) goto done;
    lda8(rd8(ABS(0x0CD7)));
    if (!fz) goto done;
    lda8(rd8(DP(0x76)));
    cmp8((u8)A, 0x03);
    if (!fz) goto done;
    {
        int x = 7;
        lda8(rd8(LONG(0x7FFFFF)));
        for (; x >= 0; x--) {
            cmp8((u8)A, rd8(ABS(0x82A7 + x)));
            if (fz)
                break;
        }
        X = (u8)x;
        fn = 1; fz = 0;                          /* DEX left X negative */
        if (x < 0) {
            X = 0xFF;
            ret();
            return;
        }
    }
    rep(0x10);
    {
        u16 col = (u16)(rd16(ABS(0x0275)) + 3);
        u8 v;
        for (;;) {
            v = rd8(LONG(0x7F4000 + col));
            if (v == 0x09)
                col = 0;                         /* goal: wrap to the start */
            col++;
            if (v != 0x40)
                continue;
            u8 before = rd8(LONG(0x7F3FFE + col));
            if (before == 0x17 || before == 0x19 || before == 0x1B ||
                before == 0x03 || before == 0x04)
                continue;
            cmp8(before, 0x04);                      /* C as the last CMP leaves it */
            lda8(before);
            break;
        }
        rep(0x20);
        Y = rd16(ABS(0x08A4)) & 0xFF;
        A = Y;
        sep(0x20);
        u8 item = rd8(ABS(0x82AF + Y));
        lda8(item);
        wr8(LONG(0x7F3FFF + col), item);
        wr8(ABS(0x00B0), 0);
        X = col;
    }
done:
    sep(0x10);
    ret();
}

/* $80:D3FD ClearWords1530 (RTL): zero the low bytes of the four words at
 * $1530-$1537 (X is preserved on the stack). */
void ClearWords1530_m1x1(void)
{
    push8((u8)X);
    for (int x = 6; x >= 0; x -= 2)
        wr8(ABS(0x1530 + x), 0);
    X = pull8();
    nz8((u8)X);
    retl();
}

/* $81:F999 ResetMinuteFrame (RTS): restart the frame-within-minute counter
 * $7F:FFFA (16-bit) and return X = 1; A is preserved. */
void ResetMinuteFrame_m1x1(void)
{
    push8((u8)A);
    rep(0x20);
    lda16(0x0000);
    wr16(LONG(0x7FFFFA), 0x0000);
    sep(0x20);
    ldx8(0x01);
    lda8(pull8());
    ret();
}

/* $80:A458 SumBytes0850 (RTL): $085B = (sum of $0850-$085A) & $3F. */
void SumBytes0850_m1x1(void)
{
    ldx8(0x0A);
    lda8(0x00);
    for (;;) {
        fc = 0;
        adc8(rd8(ABS(0x0850 + X)));
        X = dec8((u8)X);
        if (fn) break;
    }
    and8(0x3F);
    wr8(ABS(0x085B), (u8)A);
    retl();
}

/* $80:BD39 Nop_BD39 (RTS): eight NOPs, an empty hook. */
void Nop_BD39_m1x1(void)
{
    ret();
}

/* $80:BFBA AddSpeedFraction (RTS): $0180,X = $01C0,X, plus one when adding
 * the low speed byte $01B8,X to the fraction $0914,X carries. */
void AddSpeedFraction_m1x1(void)
{
    lda8(rd8(ABS(0x01C0 + X)));
    wr8(ABS(0x0180 + X), (u8)A);
    lda8(rd8(ABS(0x01B8 + X)));
    fc = 0;
    adc8(rd8(ABS(0x0914 + X)));
    wr8(ABS(0x0914 + X), (u8)A);
    if (fc)
        wr8(ABS(0x0180 + X), inc8(rd8(ABS(0x0180 + X))));
    ret();
}
