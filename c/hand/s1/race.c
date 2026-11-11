/*
 * Stadium 1 race setup, hand-written.
 */
#include "hand/hand.h"

void QueueSound_m1x1(void);

/* $80:9201 EnableHudSplit (RTS): $91 = 1, so the NMI handler also enables the
 * H/V timer IRQ that splits the HUD from the track. */
void EnableHudSplit_m1x1(void)
{
    lda8(0x01);
    wr8(DP(0x91), 0x01);
    ret();
}

/* $80:9206 ClearRaceState (RTS): zero the race work areas $0884-$09B1
 * (direct page relative, 16-bit index) and $09B2-$0A12 (through the pointer
 * at $00, which is left pointing at $09B2), and $80-$84; sound $2D. */
void ClearRaceState_m1x1(void)
{
    rep(0x10);
    u16 x = 0x0884;
    do {
        wr8((u16)(D + x), 0);
        x++;
        cmp16(x, 0x09B2);
    } while (!fc);
    X = x;
    sep(0x10);
    lda8(0x09); wr8(DP(0x01), 0x09);
    lda8(0xB2); wr8(DP(0x00), 0xB2);
    lda8(0x00);
    for (int y = 0x60; y >= 0; y--) {
        Y = (u16)y;
        wr8(IDPY(0x00), 0);
    }
    Y = 0xFF; nz8(0xFF);
    for (int k = 3; k >= 0; k--) {
        X = (u16)k;
        wr8(DPX(0x80), 0);
    }
    X = 0xFF;
    wr8(DP(0x84), 0);
    lda8(0x2D);
    push8(0x80); push16(0x9234); QueueSound_m1x1();
    ret();
}
