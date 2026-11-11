/*
 * Stadium 1 rival-rider AI: "wait, then step" state handlers, hand-written.
 *
 * Objects (riders and other actors) are parallel arrays of 16 entries at
 * $0279-$0389 indexed by the object number; $039D holds the object being
 * updated. The AI uses
 *   $0309,X state        $0319,X step counter   $0349,X action (input-like:
 *   $0369,X timer to set $0379,X timer that runs  0 = none, 1-6 = moves)
 *
 * Seven handlers share one shape (found by scanning for its byte pattern):
 * while timer $0379 runs, hold action `waiting`;
 * when it expires, arm $0369 with `next_timer`, reset the step counter,
 * advance the state and start action `go`.
 */
#include "hand/hand.h"

static void wait_then_step(u8 next_timer, u8 go, int waiting_loads, u8 waiting)
{
    X = rd8(ABS(0x039D));
    nz8((u8)X);
    lda8(rd8(ABS(0x0379 + X)));
    if ((u8)A == 0) {
        lda8(next_timer);
        wr8(ABS(0x0369 + X), next_timer);
        wr8(ABS(0x0319 + X), 0);
        wr8(ABS(0x0309 + X), inc8(rd8(ABS(0x0309 + X))));
        lda8(go);
        wr8(ABS(0x0349 + X), go);
    } else if (waiting_loads) {
        lda8(waiting);
        wr8(ABS(0x0349 + X), waiting);
    } else {
        wr8(ABS(0x0349 + X), 0);
    }
    ret();
}

/* $81B819: next timer $08, go action 6, waiting action 0. */
void RivalWaitStep_B819_m1x1(void) { wait_then_step(0x08, 6, 0, 0); }

/* $81B879: next timer $20, go action 3, waiting action 0. */
void RivalWaitStep_B879_m1x1(void) { wait_then_step(0x20, 3, 0, 0); }

/* $81B9B6: next timer $15, go action 3, waiting action 4. */
void RivalWaitStep_B9B6_m1x1(void) { wait_then_step(0x15, 3, 1, 4); }

/* $81BA8E: next timer $08, go action 6, waiting action 5. */
void RivalWaitStep_BA8E_m1x1(void) { wait_then_step(0x08, 6, 1, 5); }

/* $81BAE8: next timer $80, go action 2, waiting action 0. */
void RivalWaitStep_BAE8_m1x1(void) { wait_then_step(0x80, 2, 0, 0); }

/* $81BC2E: next timer $09, go action 3, waiting action 0. */
void RivalWaitStep_BC2E_m1x1(void) { wait_then_step(0x09, 3, 0, 0); }

/* $81BD06: next timer $C0, go action 4, waiting action 0. */
void RivalWaitStep_BD06_m1x1(void) { wait_then_step(0xC0, 4, 0, 0); }

/* The same shape with a random pick: on expiry also set $0329,X to a random
 * 1-4 (Random & 3, + 1). */
void Random_m1x1(void);

static void wait_then_step_random(u16 jsl_ret, u8 next_timer, u8 go)
{
    X = rd8(ABS(0x039D));
    nz8((u8)X);
    lda8(rd8(ABS(0x0379 + X)));
    if ((u8)A != 0) {
        wr8(ABS(0x0349 + X), 0);
        ret();
        return;
    }
    lda8(next_timer);
    wr8(ABS(0x0369 + X), next_timer);
    push8(0x81); push16(jsl_ret); Random_m1x1();
    and8(0x03);
    SET_A8(inc8((u8)A));
    wr8(ABS(0x0329 + X), (u8)A);
    wr8(ABS(0x0319 + X), 0);
    wr8(ABS(0x0309 + X), inc8(rd8(ABS(0x0309 + X))));
    lda8(go);
    wr8(ABS(0x0349 + X), go);
    ret();
}

/* $81B7B9: next timer $20, go action 1. */
void RivalWaitRandom_B7B9_m1x1(void) { wait_then_step_random(0xB7C9, 0x20, 1); }

/* $81B8ED: next timer $10, go action 1. */
void RivalWaitRandom_B8ED_m1x1(void) { wait_then_step_random(0xB8FD, 0x10, 1); }

/* $81BA2E: next timer $0A, go action 1. */
void RivalWaitRandom_BA2E_m1x1(void) { wait_then_step_random(0xBA3E, 0x0A, 1); }

/* $81BB49: next timer $09, go action 1. */
void RivalWaitRandom_BB49_m1x1(void) { wait_then_step_random(0xBB59, 0x09, 1); }

/* $81BCA1: next timer $10, go action 1. */
void RivalWaitRandom_BCA1_m1x1(void) { wait_then_step_random(0xBCB1, 0x10, 1); }
