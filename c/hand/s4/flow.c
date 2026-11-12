/*
 * Stadium 3-4 (Gen B): mode changes and the end of a race, hand-written.
 */
#include "hand/hand.h"
#include "roster.h"

void ChangeMode_m1x1(void);

/* $80:8F17 ChangeMode (RTS): start the mode in $76 from its first step:
 * APUIO3 = 0, $75 = $77 = 0, fade out ($1923 = -1). */
void ChangeMode_m1x1(void)
{
    wr8(ABS(0x2143), 0);
    lda8(0x00);
    wr8(DP(0x75), 0x00);
    wr8(DP(0x77), 0x00);
    lda8(0xFF);
    wr8(ABS(0x1923), 0xFF);
    ret();
}

/* $80:8F15 NextMode (RTS): $76 + 1, then ChangeMode. */
void NextMode_m1x1(void)
{
    wr8(DP(0x76), inc8(rd8(DP(0x76))));
    ChangeMode_m1x1();
}

/* $80:AB59 RaceFinishStep (RTS): after the finish (or a timeout), count
 * $52 down from $10; at 1 the final goes on to the jump contest (mode $13)
 * and a free run to its results (next mode, $86/$87 = 0); a practice run
 * ($0CD7) returns to the title. Before that, clear the race flags
 * ($092D, $0940, $60). In the port a free run raced against the final's
 * rivals still counts as a free run (port_race_is_final). */
void RaceFinishStep_m1x1(void)
{
    lda8(rd8(DP(0x52)));
    if (fz) { lda8(0x10); wr8(DP(0x52), 0x10); }
    cmp8((u8)A, 0x01);
    if (!fz) {
        lda8(0x00);
        wr8(ABS(0x092D), 0x00);
        wr8(ABS(0x0940), 0x00);
        wr8(DP(0x60), 0x00);
        X = rd8(DP(0x84)); nz8((u8)X);
        X = dec8((u8)X);
        if (!fz) { ret(); return; }
        ldx8(0x12);
        lda8(rd8(DP(0x52)));
        lda8(lsr8((u8)A));
        if (!fc) {
            ldy8(0x06);
            lda8(0xFE);
            do {
                X = inc8((u8)X);
                Y = dec8((u8)Y);
            } while (!fn);
        }
        ret();
        return;
    }
    lda8(rd8(DP(0x7D)));
    if (port_race_is_final()) {
        lda8(0x13);
        wr8(DP(0x76), 0x13);
        push16(0xAB6F); ChangeMode_m1x1();
    } else {
        push16(0xAB74); NextMode_m1x1();
        wr8(DP(0x86), 0);
        wr8(DP(0x87), 0);
    }
    lda8(rd8(ABS(0x0CD7)));
    if (fz) { ret(); return; }
    ldx8(0x2B);
    do {
        wr8(DPX(0x38), 0);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(0x76), 0);
    wr8(DP(0x79), inc8(rd8(DP(0x79))));
    ChangeMode_m1x1();
}
