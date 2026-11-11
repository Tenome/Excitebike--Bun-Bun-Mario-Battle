/*
 * hand.h - helpers for hand-written routines that replace lifted ones.
 *
 * Hand routines run on the same machine model as the lifted code
 * (c/runtime816.h), so they can be swapped in one at a time and checked by
 * tools/difftest816.py. They are written for reading: named registers and
 * RAM, structured control flow, and a comment that says what the routine
 * does. Rules:
 *   - keep the order of hardware writes (the difftest compares it);
 *   - keep stack traffic the original makes (PHP/PHX bytes land in WRAM);
 *   - end with ret() for RTS or retl() for RTL;
 *   - list the routine in c/hand/<ver>/hand.txt with its compare mode
 *     ("mem": WRAM, hardware writes, S/D/DB; "full": also A/X/Y and flags).
 */
#ifndef HAND_H
#define HAND_H

#include "runtime816.h"
#include <setjmp.h>

extern jmp_buf r816_bail;
extern long r816_ticks, r816_limit;
/* Use inside loops that wait on hardware, like the lifted code does. */
#define WAIT_TICK() do { if (++r816_ticks > r816_limit) longjmp(r816_bail, 1); } while (0)

static inline void ret(void) { pull16(); }
static inline void retl(void) { pull16(); pull8(); }

/* SNES registers (bus A addresses; accessed with DB-relative absolute
 * addressing in the original, so use ABS() to read/write them). */
enum {
    REG_M7A = 0x211B, REG_M7B = 0x211C, REG_WMADDL = 0x2181, REG_WMADDH = 0x2183,
    REG_MDMAEN = 0x420B, REG_HVBJOY = 0x4212,
    REG_JOY1L = 0x4218, REG_JOY1H = 0x4219, REG_JOY2L = 0x421A, REG_JOY2H = 0x421B,
    REG_DMAP0 = 0x4300, REG_A1T0L = 0x4302, REG_A1B0 = 0x4304, REG_DAS0L = 0x4305,
};

#endif
