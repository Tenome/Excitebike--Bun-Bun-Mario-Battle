/*
 * Stadium 3-4 (Gen B): small screen routines, hand-written.
 */
#include "hand/hand.h"
#include "roster.h"

void VramQueueTiles_m1x1(void);

/* Two-digit split by repeated subtraction into the tens byte at `tens`
 * (cleared first); returns the units. */
static u8 split_tens(u8 v, u16 tens)
{
    wr8(ABS(tens), 0);
    lda8(v);
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        wr8(ABS(tens), inc8(rd8(ABS(tens))));
        fc = 1;
        sbc8(0x0A);
    }
    return (u8)A;
}

/*
 * $80:81A9 DrawClockB (RTS): while the clock is on ($1566), every 30 frames
 * blink the colon ($1E85) and queue "HH:MM" as a 5x2 tile block at VRAM
 * $1842 (list $1E1E: size $25, digits $00-$09 on top and $10-$19 below,
 * colon $46/$56 or blank $FF; attribute $20, or $24 with $CA set while the
 * colon is off). $1566 negative forces a redraw now. In the port the digits
 * are drawn blank when the game plays without the broadcast clock.
 */
void DrawClockB_m1x1(void)
{
    lda8(rd8(ABS(0x1566)));
    if (!fn) {
        if (fz) { ret(); return; }
        u8 t = dec8(rd8(ABS(0x1E84)));
        wr8(ABS(0x1E84), t);
        if (t != 0) { ret(); return; }
    }
    lda8(0x01); wr8(ABS(0x1566), 0x01);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    lda8(rd8(ABS(0x1E85)));
    eor8(0x01);
    wr8(ABS(0x1E85), (u8)A);
    lda8(0x1E); wr8(ABS(0x1E84), 0x1E);
    lda8(0x42); wr8(DP(0x00), 0x42);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(0x25); wr8(ABS(0x1E1E), 0x25);
    u8 u = split_tens(rd8(LONG(0x7FFFF9)), 0x1E1F);       /* hour */
    wr8(ABS(0x1E20), u);
    ora8(0x10); wr8(ABS(0x1E25), (u8)A);
    lda8(rd8(ABS(0x1E1F))); fc = 0; adc8(0x10); wr8(ABS(0x1E24), (u8)A);
    u = split_tens(rd8(LONG(0x7FFFFF)), 0x1E22);           /* minute */
    wr8(ABS(0x1E23), u);
    ora8(0x10); wr8(ABS(0x1E28), (u8)A);
    lda8(rd8(ABS(0x1E22))); fc = 0; adc8(0x10); wr8(ABS(0x1E27), (u8)A);
    lda8(rd8(ABS(0x1E85)));
    if (fz) {
        lda8(0x46); wr8(ABS(0x1E21), 0x46);
        lda8(0x56); wr8(ABS(0x1E26), 0x56);
    } else {
        lda8(0xFF);
        wr8(ABS(0x1E21), 0xFF);
        wr8(ABS(0x1E26), 0xFF);
        lda8(rd8(ABS(0x00CA)));
        if (!fz) { lda8(0x24); wr8(DP(0x0F), 0x24); }
    }
    if (port_hide_clock())
        for (int a = 0x1E1F; a <= 0x1E28; a++)
            wr8(ABS(a), 0xFF);
    lda8(0x1E); wr8(DP(0x02), 0x1E);
    lda8(0x1E); wr8(DP(0x03), 0x1E);
    VramQueueTiles_m1x1();                            /* JMP: it returns for us */
}
