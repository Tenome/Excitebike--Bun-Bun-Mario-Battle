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

/* "Alternate" handlers: each time timer $0369 runs out, flip the step
 * $0319 between 0 and 1. Coming back to 0 uses up one repeat ($0329); with
 * none left, go back to state 0 and idle for a random 64-127 frames ($0379).
 * Otherwise the next timer and action come from two-entry tables (step 0, 1)
 * stored right after each handler's code. */
static void alternate(u16 jsl_ret, u16 timer_table, u16 action_table)
{
    X = rd8(ABS(0x039D));
    nz8((u8)X);
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    u8 step = (u8)(rd8(ABS(0x0319 + X)) ^ 0x01);
    lda8(step);
    wr8(ABS(0x0319 + X), step);
    Y = step;
    if (step == 0) {
        u8 left = dec8(rd8(ABS(0x0329 + X)));
        wr8(ABS(0x0329 + X), left);
        if (left == 0) {
            wr8(ABS(0x0309 + X), 0);
            push8(0x81); push16(jsl_ret); Random_m1x1();
            and8(0x3F);
            fc = 0;
            adc8(0x40);
            wr8(ABS(0x0379 + X), (u8)A);
            ret();
            return;
        }
    }
    lda8(rd8(ABS(timer_table + Y)));
    wr8(ABS(0x0369 + X), (u8)A);
    lda8(rd8(ABS(action_table + Y)));
    wr8(ABS(0x0349 + X), (u8)A);
    ret();
}

void RivalAlternate_B7E0_m1x1(void) { alternate(0xB7FE, 0xB815, 0xB817); }

void RivalAlternate_B914_m1x1(void) { alternate(0xB932, 0xB949, 0xB94B); }

void RivalAlternate_BA55_m1x1(void) { alternate(0xBA73, 0xBA8A, 0xBA8C); }

void RivalAlternate_BB70_m1x1(void) { alternate(0xBB8E, 0xBBA5, 0xBBA7); }

/* Per-rival state dispatchers: jump to the handler for the current object's
 * state ($0309,X) through a table of handler addresses. */
static void dispatch_state(u16 table, u32 site)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0309 + X)));
    SET_A8(asl8((u8)A));
    X = (u8)A;
    u16 handler = rd16(0x810000u | (u16)(table + X));
    if (!r816_call_pc(0x810000u | handler))
        r816_unknown_target(site, handler);
}

void RivalDispatch_9761_m1x1(void) { dispatch_state(0x976C, 0x819769); }

void RivalDispatch_B639_m1x1(void) { dispatch_state(0xB644, 0x81B641); }

void RivalDispatch_B6B3_m1x1(void) { dispatch_state(0xB6BE, 0x81B6BB); }

void RivalDispatch_B7A2_m1x1(void) { dispatch_state(0xB7AD, 0x81B7AA); }

void RivalDispatch_B8D6_m1x1(void) { dispatch_state(0xB8E1, 0x81B8DE); }

void RivalDispatch_BA17_m1x1(void) { dispatch_state(0xBA22, 0x81BA1F); }

void RivalDispatch_BB32_m1x1(void) { dispatch_state(0xBB3D, 0x81BB3A); }

void RivalDispatch_BC8A_m1x1(void) { dispatch_state(0xBC95, 0x81BC92); }

void RivalDispatch_BD52_m1x1(void) { dispatch_state(0xBD5D, 0x81BD5A); }

/*
 * $81:B67F RivalSequence_B67F: each time timer $0369 runs out, take the next
 * step ($0319 + 1) of an 8-step move list: action from $81:B6A3 (2-9) and
 * timer from $81:B6AB (3 or 4 frames). After step 7 the object is switched
 * off ($02E9,X = 0).
 */
void RivalSequence_B67F_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    u8 step = inc8(rd8(ABS(0x0319 + X)));
    wr8(ABS(0x0319 + X), step);
    lda8(step);
    cmp8(step, 0x08);
    if (fc) {
        wr8(ABS(0x02E9 + X), 0);                   /* sequence over */
        ret();
        return;
    }
    Y = step;
    lda8(rd8(ABS(0xB6A3 + Y))); wr8(ABS(0x0349 + X), (u8)A);
    lda8(rd8(ABS(0xB6AB + Y))); wr8(ABS(0x0369 + X), (u8)A);
    ret();
}

/*
 * $81:BB05 RivalSequence_BB05: cycle through steps 1-3 (never 0) of a
 * 4-entry move list when timer $0369 runs out: timer from $81:BB2A
 * (128, 50, 13, 33), action from $81:BB2E (2, 2, 3, 4).
 */
void RivalSequence_BB05_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    u8 step = (u8)((rd8(ABS(0x0319 + X)) + 1) & 0x03);
    lda8(step);
    wr8(ABS(0x0319 + X), step);
    if (step == 0) {
        step = 1;
        lda8(1);
        wr8(ABS(0x0319 + X), 1);
    }
    Y = step;
    lda8(rd8(ABS(0xBB2A + Y))); wr8(ABS(0x0369 + X), (u8)A);
    lda8(rd8(ABS(0xBB2E + Y))); wr8(ABS(0x0349 + X), (u8)A);
    ret();
}

/* Add the signed 4.4 speed at speed,X to an object coordinate: the low nibble
 * (x16) into the fraction frac,X, the high nibble, sign-extended, plus the
 * carry into the 16-bit position pos/pos+1,X. */
static void apply_speed(u16 speed, u16 frac, u16 pos)
{
    u8 v = rd8(ABS(speed + X));
    lda8(v);
    lda8(asl8(asl8(asl8(asl8(v)))));
    fc = 0;
    adc8(rd8(ABS(frac + X)));
    wr8(ABS(frac + X), (u8)A);
    push8(getP());                                 /* keep the fraction's carry */
    lda8(v);
    lda8(lsr8(lsr8(lsr8(lsr8(v)))));
    cmp8((u8)A, 0x08);
    ldy8(0x00);
    if (fc) {                                      /* negative speed: sign-extend */
        ora8(0xF0);
        Y = dec8((u8)Y);
    }
    setP(pull8());
    adc8(rd8(ABS(pos + X)));
    wr8(ABS(pos + X), (u8)A);
    lda8((u8)Y);
    adc8(rd8(ABS(pos + 1 + X)));
    wr8(ABS(pos + 1 + X), (u8)A);
}

/* $81:9F4C ApplyVelocity (RTL): Y position $0289/$028A,X += speed $02A9,X
 * (fraction $02C9,X). */
void ApplyVelocity_m1x1(void)
{
    apply_speed(0x02A9, 0x02C9, 0x0289);
    retl();
}

/* $81:9F1E ApplyVelocityX (RTL): X position $0279/$027A,X += speed $0299,X
 * (fraction $02B9,X). */
void ApplyVelocityX_m1x1(void)
{
    apply_speed(0x0299, 0x02B9, 0x0279);
    retl();
}

void QueueSound_m1x1(void);

/*
 * $81:B64A RivalTumble_B64A: flip action $0349 between frames 0 and 1 every
 * 4 frames, move by the speed $02A9 (counting up towards zero each frame);
 * when the speed reaches zero, go to the next state ($0309 + 1) with action 2,
 * step 0, timer 4, and play sound $49.
 */
void RivalTumble_B64A_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(DP(0x63)));
    and8(0x03);
    if ((u8)A == 0) {
        lda8(rd8(ABS(0x0349 + X)));
        lda8(inc8((u8)A));
        and8(0x01);
        wr8(ABS(0x0349 + X), (u8)A);
    }
    push8(0x81); push16(0xB65F); ApplyVelocity_m1x1();
    u8 sp = inc8(rd8(ABS(0x02A9 + X)));
    wr8(ABS(0x02A9 + X), sp);
    lda8(sp);
    if (sp != 0) { ret(); return; }
    wr8(ABS(0x0309 + X), inc8(rd8(ABS(0x0309 + X))));
    lda8(0x02);
    wr8(ABS(0x0349 + X), 0x02);
    wr8(ABS(0x0319 + X), 0);
    lda8(0x04);
    wr8(ABS(0x0369 + X), 0x04);
    lda8(0x49);
    push8(0x81); push16(0xB67D); QueueSound_m1x1();
    ret();
}

/*
 * $81:B94D RivalPause_B94D: action 9 while timer $0379 runs; when it is out,
 * timer $0369 = 22 frames, repeat count $0329 = 1-4 (random), step 0, next
 * state.
 */
void RivalPause_B94D_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0379 + X)));
    if ((u8)A == 0) {
        lda8(0x16);
        wr8(ABS(0x0369 + X), 0x16);
        push8(0x81); push16(0xB95D); Random_m1x1();
        and8(0x03);
        lda8(inc8((u8)A));
        wr8(ABS(0x0329 + X), (u8)A);
        wr8(ABS(0x0319 + X), 0);
        wr8(ABS(0x0309 + X), inc8(rd8(ABS(0x0309 + X))));
    }
    lda8(0x09);
    wr8(ABS(0x0349 + X), 0x09);
    ret();
}

/*
 * $81:BD23 RivalSequence_BD23: like RivalSequence_BB05 with the tables at
 * $81:BD4A (timers) and $81:BD4E (actions), cycling steps 1-3 but going back
 * to step 2 after step 3.
 */
void RivalSequence_BD23_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    u8 step = (u8)((rd8(ABS(0x0319 + X)) + 1) & 0x03);
    lda8(step);
    wr8(ABS(0x0319 + X), step);
    Y = step; nz8(step);
    if (step == 0) {
        step = 2;
        lda8(2);
        wr8(ABS(0x0319 + X), 2);
        Y = 2;
    }
    lda8(rd8(ABS(0xBD4A + Y))); wr8(ABS(0x0369 + X), (u8)A);
    lda8(rd8(ABS(0xBD4E + Y))); wr8(ABS(0x0349 + X), (u8)A);
    ret();
}

/*
 * $81:BECE RivalWalkAway_BECE: every 4 frames step the 4-frame cycle ($0319)
 * and move right one pixel ($0279); at X >= $A0 go back to state 0 with a
 * 64-frame timer. The action each frame is $81:BF00[step].
 */
void RivalWalkAway_BECE_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A == 0) {
        lda8((u8)((rd8(ABS(0x0319 + X)) + 1) & 0x03));
        wr8(ABS(0x0319 + X), (u8)A);
        lda8(0x04);
        wr8(ABS(0x0369 + X), 0x04);
        u8 x = inc8(rd8(ABS(0x0279 + X)));
        wr8(ABS(0x0279 + X), x);
        lda8(x);
        cmp8(x, 0xA0);
        if (fc) {
            wr8(ABS(0x0309 + X), 0);
            lda8(0x40);
            wr8(ABS(0x0369 + X), 0x40);
        }
    }
    ldy8(rd8(ABS(0x0319 + X)));
    lda8(rd8(ABS(0xBF00 + Y)));
    wr8(ABS(0x0349 + X), (u8)A);
    ret();
}

/* Random 64-127 (Random & $3F) + $40, called from bank $81 with the JSL return
 * address jsl_ret (the address of the JSL's last byte). */
static u8 random_64_127(u16 jsl_ret)
{
    push8(0x81); push16(jsl_ret); Random_m1x1();
    and8(0x3F);
    fc = 0;
    adc8(0x40);
    return (u8)A;
}

/*
 * $81:B9D5 RivalSequence_B9D5: 7-step move list (timers $81:BA09, actions
 * $81:BA10); after the last step, start again from step 0 with a random
 * 64-127 frame timer.
 */
void RivalSequence_B9D5_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    u8 step = inc8(rd8(ABS(0x0319 + X)));
    lda8(step);
    Y = step;
    wr8(ABS(0x0319 + X), step);
    cmp8(step, 0x07);
    if (fc) {
        Y = 0;
        wr8(ABS(0x0319 + X), 0);
        wr8(ABS(0x0369 + X), random_64_127(0xB9F1));
    } else {
        lda8(rd8(ABS(0xBA09 + Y)));
        wr8(ABS(0x0369 + X), (u8)A);
    }
    lda8(rd8(ABS(0xBA10 + Y)));
    wr8(ABS(0x0349 + X), (u8)A);
    ret();
}

/*
 * $81:BAAD RivalSequence_BAAD: 3-step move list (timers $81:BAE0, actions
 * $81:BAE4); when the cycle wraps, go to state 2 with a random 64-127 frame
 * wait ($0379).
 */
void RivalSequence_BAAD_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    u8 step = (u8)((rd8(ABS(0x0319 + X)) + 1) & 0x03);
    lda8(step);
    wr8(ABS(0x0319 + X), step);
    Y = step; nz8(step);
    if (step == 0) {
        lda8(0x02);
        wr8(ABS(0x0309 + X), 0x02);
        wr8(ABS(0x0379 + X), random_64_127(0xBAC9));
        ret();
        return;
    }
    lda8(rd8(ABS(0xBAE0 + Y))); wr8(ABS(0x0369 + X), (u8)A);
    lda8(rd8(ABS(0xBAE4 + Y))); wr8(ABS(0x0349 + X), (u8)A);
    ret();
}

/*
 * $81:BC4B RivalSequence_BC4B: 5-step move list (timers $81:BC7E, actions
 * $81:BC84); after it, state 4 with a random 64-127 frame wait ($0379).
 */
void RivalSequence_BC4B_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    u8 step = inc8(rd8(ABS(0x0319 + X)));
    lda8(step);
    wr8(ABS(0x0319 + X), step);
    Y = step;
    cmp8(step, 0x06);
    if (fc) {
        lda8(0x04);
        wr8(ABS(0x0309 + X), 0x04);
        wr8(ABS(0x0379 + X), random_64_127(0xBC67));
        ret();
        return;
    }
    lda8(rd8(ABS(0xBC7E + Y))); wr8(ABS(0x0369 + X), (u8)A);
    lda8(rd8(ABS(0xBC84 + Y))); wr8(ABS(0x0349 + X), (u8)A);
    ret();
}

/* Step a move list held in two tables (timer at $81:tt, action at $81:aa)
 * for the current object X, step Y. */
static void move_step(u16 timers, u16 actions)
{
    lda8(rd8(ABS(timers + Y))); wr8(ABS(0x0369 + X), (u8)A);
    lda8(rd8(ABS(actions + Y))); wr8(ABS(0x0349 + X), (u8)A);
}

/*
 * $81:B896 RivalSequence_B896: steps 1-4 of a move list (timers $81:B8CC,
 * actions $81:B8D1), then back to step 1 with its timer plus a random 0-63.
 */
void RivalSequence_B896_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    lda8(inc8(rd8(ABS(0x0319 + X))));
    cmp8((u8)A, 0x05);
    if (fc) {
        lda8(0x01);
        wr8(ABS(0x0319 + X), 0x01);
        Y = 1; nz8(1);
        push8(0x81); push16(0xB8AF); Random_m1x1();
        and8(0x3F);
        fc = 0;
        adc8(rd8(ABS(0xB8CC + Y)));
        wr8(ABS(0x0369 + X), (u8)A);
    } else {
        wr8(ABS(0x0319 + X), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABS(0xB8CC + Y)));
        wr8(ABS(0x0369 + X), (u8)A);
    }
    lda8(rd8(ABS(0xB8D1 + Y)));
    wr8(ABS(0x0349 + X), (u8)A);
    ret();
}

/* Shared by $81:BCC8 and $81:B976: a 4-step cycle repeated $0329 times, then
 * state `next` with a random 64-127 frame wait. */
static void cycle_repeat(u8 next, u16 jsl_ret, u16 timers, u16 actions)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    lda8((u8)((rd8(ABS(0x0319 + X)) + 1) & 0x03));
    wr8(ABS(0x0319 + X), (u8)A);
    Y = (u8)A; nz8((u8)Y);
    if (Y == 0) {
        u8 n = dec8(rd8(ABS(0x0329 + X)));
        wr8(ABS(0x0329 + X), n);
        if (n == 0) {
            if (next) { lda8(next); wr8(ABS(0x0309 + X), next); }
            else wr8(ABS(0x0309 + X), 0);
            wr8(ABS(0x0379 + X), random_64_127(jsl_ret));
            ret();
            return;
        }
    }
    move_step(timers, actions);
    ret();
}

/* $81:BCC8: 4-step cycle ($81:BCFE/$81:BD02) x $0329, then state 0. */
void RivalCycle_BCC8_m1x1(void) { cycle_repeat(0, 0xBCE7, 0xBCFE, 0xBD02); }

/* $81:B976: 4-step cycle ($81:B9AE/$81:B9B2) x $0329, then state 2. */
void RivalCycle_B976_m1x1(void) { cycle_repeat(2, 0xB997, 0xB9AE, 0xB9B2); }

/*
 * $81:B836 RivalSequence_B836: steps 1-5 (timers $81:B86D, actions
 * $81:B873); after step 5, state 2 with a random 64-127 frame wait.
 */
void RivalSequence_B836_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A != 0) { ret(); return; }
    lda8(inc8(rd8(ABS(0x0319 + X))));
    cmp8((u8)A, 0x06);
    if (fc)
        lda8(0x00);
    wr8(ABS(0x0319 + X), (u8)A);
    Y = (u8)A; nz8((u8)Y);
    if (Y == 0) {
        lda8(0x02);
        wr8(ABS(0x0309 + X), 0x02);
        wr8(ABS(0x0379 + X), random_64_127(0xB856));
        ret();
        return;
    }
    move_step(0xB86D, 0xB873);
    ret();
}

/*
 * $81:BBA9 RivalBlink_BBA9: flip between actions (bit 0 of $0349) every 10
 * frames; when the wait $0379 is over, timer 9, repeat count 1-4 (random),
 * step 0, action 8, next state.
 */
void RivalBlink_BBA9_m1x1(void)
{
    X = rd8(ABS(0x039D));
    lda8(rd8(ABS(0x0369 + X)));
    if ((u8)A == 0) {
        lda8(rd8(ABS(0x0349 + X)));
        eor8(0x01);
        wr8(ABS(0x0349 + X), (u8)A);
        lda8(0x0A);
        wr8(ABS(0x0369 + X), 0x0A);
    }
    lda8(rd8(ABS(0x0379 + X)));
    if ((u8)A != 0) { ret(); return; }
    lda8(0x0A); wr8(ABS(0x0369 + X), 0x0A);
    lda8(0x09); wr8(ABS(0x0369 + X), 0x09);
    push8(0x81); push16(0xBBD0); Random_m1x1();
    and8(0x03);
    lda8(inc8((u8)A));
    wr8(ABS(0x0329 + X), (u8)A);
    wr8(ABS(0x0319 + X), 0);
    wr8(ABS(0x0309 + X), inc8(rd8(ABS(0x0309 + X))));
    lda8(0x08);
    wr8(ABS(0x0349 + X), 0x08);
    ret();
}
