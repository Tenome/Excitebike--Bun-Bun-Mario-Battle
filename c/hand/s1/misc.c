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

/* $80:C337 Tick08CC (RTS): count down the per-rider timer $08CC,X; it is
 * cancelled (zeroed) while any of bits 6-7 of $0178,X are set. */
void Tick08CC_m1x1(void)
{
    lda8(rd8(ABS(0x08CC + X)));
    if (fz) { ret(); return; }
    wr8(ABS(0x08CC + X), dec8(rd8(ABS(0x08CC + X))));
    lda8(rd8(ABS(0x0178 + X)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(0x00);
    wr8(ABS(0x08CC + X), 0x00);
    ret();
}

/* $80:C98F DivideAX (RTS): divide A by X by repeated subtraction while the
 * result stays positive (signed): X = quotient, A = remainder. */
void DivideAX_m1x1(void)
{
    u8 d = (u8)X;
    wr8(DP(0x0F), d);
    ldx8(0x00);
    do {
        WAIT_TICK();
        X = inc8((u8)X);
        fc = 1;
        sbc8(rd8(DP(0x0F)));
    } while (!fn);
    X = dec8((u8)X);
    adc8(rd8(DP(0x0F)));
    ret();
}

/* Fill words $7F:000E..$7F:000C+last with a tile word (a blank row in the
 * VRAM queue buffer). */
static void fill_queue_row(u8 last, u16 tile)
{
    ldx8(last);
    rep(0x20);
    lda16(tile);
    do {
        wr16(LONGX(0x7F000C), tile);
        X = dec8((u8)X);
        X = dec8((u8)X);
    } while (!fz);
    sep(0x20);
}

/* $80:DCD8 BlankQueueRow (RTS): 15 blank tiles ($00FC) at $7F:000E. */
void BlankQueueRow_m1x1(void)
{
    fill_queue_row(0x1E, 0x00FC);
    ret();
}

/* $81:8C21 BlankQueueRowPal2 (RTL): 10 blank tiles, palette 2 ($08FC). */
void BlankQueueRowPal2_m1x1(void)
{
    fill_queue_row(0x14, 0x08FC);
    retl();
}

/* $81:97CC RivalCountdown_97CC: count the 16-bit $0319,X down; at zero go
 * to the next state ($0309 + 1, 16-bit) and reload 208 frames. */
void RivalCountdown_97CC_m1x1(void)
{
    X = rd8(ABS(0x039D));
    rep(0x20);
    u16 t = dec16(rd16(ABS(0x0319 + X)));
    wr16(ABS(0x0319 + X), t);
    if (t == 0) {
        wr16(ABS(0x0309 + X), inc16(rd16(ABS(0x0309 + X))));
        lda16(0x00D0);
        wr16(ABS(0x0319 + X), 0x00D0);
    }
    sep(0x20);
    ret();
}

void VramQueueTiles_m1x1(void);

/* $80:95AE DrawText95C4 (RTS): queue the tile list at $80:95C4 to VRAM
 * $1CA4 with attribute byte $24. */
void DrawText95C4_m1x1(void)
{
    rep(0x20);
    lda16(0x1CA4); wr16(DP(0x00), 0x1CA4);
    lda16(0x95C4); wr16(DP(0x02), 0x95C4);
    sep(0x20);
    lda8(0x24);
    wr8(DP(0x0F), 0x24);
    push16(0x95C2); VramQueueTiles_m1x1();
    ret();
}

/* $80:9E77 BeginFadeIn (RTS): start fading in ($1923 + 1), advance the mode
 * flag $75, set the mode timer $88 to 900 frames, main screen BG1+OBJ
 * ($1905 = $11) and clear $190D. */
void BeginFadeIn_m1x1(void)
{
    wr8(ABS(0x1923), inc8(rd8(ABS(0x1923))));
    wr8(DP(0x75), inc8(rd8(DP(0x75))));
    rep(0x20);
    lda16(0x0384);
    wr16(DP(0x88), 0x0384);
    sep(0x20);
    lda8(0x11);
    wr8(ABS(0x1905), 0x11);
    wr8(ABS(0x190D), 0);
    ret();
}

/* $80:AB18 SendSpeedToApu (RTS): engine pitch = player speed ($01C0:$01B8)
 * >> 3, low byte to APUIO2. */
void SendSpeedToApu_m1x1(void)
{
    lda8(rd8(ABS(0x01C0)));
    A = (u16)((A << 8) | (A >> 8));             /* XBA */
    nz8((u8)A);
    lda8(rd8(ABS(0x01B8)));
    rep(0x20);
    lda16(lsr16(lsr16(lsr16(A))));
    sep(0x20);
    wr8(ABS(0x2142), (u8)A);
    ret();
}

/* $80:DA85 DispatchBy85 (RTS): tail-jump through the routine table at
 * $80:D409 indexed by $85 (pointer kept at $00/$01, X = 0). */
void DispatchBy85_m1x1(void)
{
    lda8(rd8(DP(0x85)));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(0xD409 + Y))); wr8(DP(0x00), (u8)A);
    Y = inc8((u8)Y);
    lda8(rd8(ABS(0xD409 + Y))); wr8(DP(0x01), (u8)A);
    ldx8(0x00);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}

/* $80:E3FB StartMode01 (RTS): mode 1 from step 0 with a 1800-frame timer
 * ($88), start fading out ($1923 - 1), return C = 1. */
void StartMode01_m1x1(void)
{
    rep(0x20);
    lda16(0x0708);
    wr16(DP(0x88), 0x0708);
    sep(0x20);
    lda8(0x01);
    wr8(DP(0x76), 0x01);
    wr8(DP(0x75), 0);
    wr8(DP(0x77), 0);
    wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    fc = 1;
    ret();
}

/* $80:E043 AnimSourceFromFrame (RTL): $86 = $B800 + (($63 & $30) << 5),
 * one of four 512-byte animation frames, changing every 16 frames. */
void AnimSourceFromFrame_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(0x63)));
    and16(0x0030);
    for (int i = 0; i < 5; i++)
        lda16(asl16(A));
    fc = 0;
    adc16(0xB800);
    wr16(ABS(0x0086), A);
    sep(0x20);
    retl();
}

/* $80:8589 InitFadeState (RTL): $2C = $0F, screen off with forced blank
 * ($1903 = $80), scroll $92 = 0, and the four channels $1959-$195C = 1-4. */
void InitFadeState_m1x1(void)
{
    lda8(0x0F); wr8(DP(0x2C), 0x0F);
    lda8(0x80); wr8(ABS(0x1903), 0x80);
    wr8(DP(0x92), 0);
    lda8(0x01);
    for (int a = 0x1959; a <= 0x195C; a++) {
        wr8(ABS(a), (u8)A);
        if (a != 0x195C)
            lda8(inc8((u8)A));
    }
    retl();
}

/* $80:9E46 BeginFadeInBg3 (RTS): BG3 map at $2800 (BG3SC $2A), tile bases
 * BG1/2 $4000 and BG3 $1000, then like BeginFadeIn but with a 1800-frame
 * timer, main screen $17 and colour math $190D = $E3. */
void BeginFadeInBg3_m1x1(void)
{
    lda8(0x2A); wr8(ABS(0x2109), 0x2A);
    lda8(0x44); wr8(ABS(0x210B), 0x44);
    lda8(0x01); wr8(ABS(0x210C), 0x01);
    wr8(ABS(0x1923), inc8(rd8(ABS(0x1923))));
    wr8(DP(0x75), inc8(rd8(DP(0x75))));
    rep(0x20);
    lda16(0x0708);
    wr16(DP(0x88), 0x0708);
    sep(0x20);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    lda8(0xE3); wr8(ABS(0x190D), 0xE3);
    ret();
}

/* $80:99A1 BsxReadSerialByte (RTL): clock 8 bits out of the BS-X serial
 * port $2199 (write $80 then $81, read bit 7), MSB first, into A. */
void BsxReadSerialByte_m1x1(void)
{
    ldy8(0x08);
    do {
        lda8(0x80); wr8(ABS(0x2199), 0x80);
        lda8(0x81); wr8(ABS(0x2199), 0x81);
        lda8(rd8(ABS(0x2199)));
        lda8(asl8((u8)A));                     /* bit into carry */
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);
        lda8(rol8((u8)A));                     /* shift it into the byte */
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);
        Y = dec8((u8)Y);
    } while (!fz);
    A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);
    retl();
}

/* $80:A03F GrowBar087E (RTS): while $087E < 15, count it up and redraw a
 * symmetric bar: $0474 = old length, and the length-1 ... 1 values going
 * out both ways from $0455+$1D/$0455+$21. */
void GrowBar087E_m1x1(void)
{
    lda8(rd8(ABS(0x087E)));
    cmp8((u8)A, 0x0F);
    if (fz) { ret(); return; }
    wr8(ABS(0x087E), inc8(rd8(ABS(0x087E))));
    wr8(ABS(0x0474), (u8)A);
    ldx8(0x1D);
    ldy8(0x21);
    for (;;) {
        lda8(dec8((u8)A));
        if (fz) break;
        wr8(ABS(0x0455 + X), (u8)A);
        wr8(ABS(0x0455 + Y), (u8)A);
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        X = dec8((u8)X); X = dec8((u8)X);
        if (fn) break;
    }
    ret();
}

/* $80:A07F StepColourCycle (RTS): every 14 frames ($0882 countdown), copy
 * the next colour word of the list at $88:BC00 (index $0880, $FF = stopped)
 * to $1C00 and flag the palette upload ($1915). */
void StepColourCycle_m1x1(void)
{
    lda8(rd8(ABS(0x0880)));
    lda8(inc8((u8)A));
    if (!fz) {
        lda8(rd8(ABS(0x0882)));
        if (fz) {
            X = rd8(ABS(0x0880));
            rep(0x20);
            lda16(rd16(LONGX(0x88BC00)));
            wr16(ABS(0x1C00), A);
            sep(0x20);
            wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
            wr8(ABS(0x0880), inc8(rd8(ABS(0x0880))));
            wr8(ABS(0x0880), inc8(rd8(ABS(0x0880))));
            lda8(0x0E);
            wr8(ABS(0x0882), 0x0E);
        }
    }
    wr8(ABS(0x0882), dec8(rd8(ABS(0x0882))));
    ret();
}

/* $80:A41C MergeBsxFlags (RTL): OR the BS-X player data bits into the
 * unlock flags: $0856 |= $11:5719|$11:571D, $0857 |= $571A|$571E,
 * $0858 |= $571B|$571F, $0859 |= ($5716 << 4) | $571C. */
void MergeBsxFlags_m1x1(void)
{
    static const u16 dst[3] = { 0x0856, 0x0857, 0x0858 };
    for (int i = 0; i < 3; i++) {
        lda8(rd8(LONG(0x115719 + i)));
        ora8(rd8(LONG(0x11571D + i)));
        wr8(ABS(dst[i]), tsb8(rd8(ABS(dst[i]))));
    }
    lda8(rd8(LONG(0x115716)));
    for (int i = 0; i < 4; i++)
        lda8(asl8((u8)A));
    ora8(rd8(LONG(0x11571C)));
    wr8(ABS(0x0859), tsb8(rd8(ABS(0x0859))));
    retl();
}

void VramQueueCopy_m1x1(void);

/* $80:9E1D EndScreenTimeout (RTS): when the timer $88 runs out, queue the
 * 16x17 tilemap at $89:DB00 to VRAM $0300 and set $0453 = 9. */
void EndScreenTimeout_m1x1(void)
{
    u8 t = dec8(rd8(DP(0x88)));
    wr8(DP(0x88), t);
    if (t != 0) { ret(); return; }
    lda8(0x00); wr8(DP(0x02), 0x00);
    lda8(0xDB); wr8(DP(0x03), 0xDB);
    lda8(0x89); wr8(DP(0x04), 0x89);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(0x9E3F); VramQueueCopy_m1x1();
    lda8(0x09);
    wr8(ABS(0x0453), 0x09);
    ret();
}

void BsxReadSerialByte_m1x1(void);

/* $80:9982 BsxReadSerial3 (RTL): select the BS-X serial device ($2199 = $81),
 * read three bytes to $00-$02,X (direct page), deselect ($2199 = $41). */
void BsxReadSerial3_m1x1(void)
{
    lda8(0x81);
    wr8(ABS(0x2199), 0x81);
    static const u16 rets[3] = { 0x998C, 0x9992, 0x9998 };
    for (int i = 0; i < 3; i++) {
        push8(0x80); push16(rets[i]); BsxReadSerialByte_m1x1();
        wr8(DPX(i), (u8)A);
    }
    lda8(0x41);
    wr8(ABS(0x2199), 0x41);
    retl();
}

void BsxReadSerial3_m1x1(void);

/* $80:A0AA BsxCheckSerial (RTS): read 3 bytes from the BS-X serial port to
 * $00-$02,X; when bits 0-1 of $01 are both set, write $80 to $2197. */
void BsxCheckSerial_m1x1(void)
{
    push8(0x80); push16(0xA0AD); BsxReadSerial3_m1x1();
    lda8(rd8(DP(0x01)));
    and8(0x03);
    cmp8((u8)A, 0x03);
    if (fz) {
        lda8(0x80);
        wr8(ABS(0x2197), 0x80);
    }
    ret();
}
