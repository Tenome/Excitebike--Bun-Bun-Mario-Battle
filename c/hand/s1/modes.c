/*
 * Stadium 1 game-mode framework, hand-written. Calls into other routines
 * (lifted or hand-written), so it links against c/auto/s1/lifted.h.
 */
#include "hand/hand.h"
#ifndef R816_EXTERNAL_BUS
#include "lifted.h"
#endif

/* Game mode state (direct page). */
enum { MODE_ACTIVE = 0x75, GAME_MODE = 0x76, MODE_STEP = 0x77, MODE_HOLD = 0x50 };

/*
 * $80:89ED RunGameModeScript (M=1, X=1, RTS), once per frame.
 *
 * A game mode is a setup list plus a main handler, both indexed by $76:
 *   $75 != 0 : run the main handler   ($80:B65D[mode])
 *   $75 == 0 : setup (skipped while $50 != 0). Step 0 resets the mode's state
 *              ($80:B21D, $80:B873, clear $008C-$0818); step n calls entry
 *              n-1 of the list at $80:B683[mode]. The list's last entry sets
 *              $75 to switch to the main handler.
 * Handlers are entered by JMP ($0000), so they return to our caller.
 */
/* Read a 16-bit table entry the way the original does: 8-bit index registers,
 * so both the doubled index and the +1 for the high byte wrap at $FF. */
static u16 table_word(u32 base, u8 index)
{
    u8 i = (u8)(index << 1);
    return (u16)(rd8(base + i) | rd8(base + (u8)(i + 1)) << 8);
}

void RunGameModeScript_m1x1(void)
{
    u8 mode = rd8(DP(GAME_MODE));
    if (rd8(DP(MODE_ACTIVE)) != 0) {
        u16 handler = table_word(ABS(0xB65D), mode);
        wr8(DP(0x00), (u8)handler);
        wr8(DP(0x01), (u8)(handler >> 8));
        fc = mode >> 7;                                /* ASL A: handlers see C = bit 7 */
        Y = (u8)((mode << 1) + 1);                     /* handlers see the dispatcher's */
        SET_A8(handler >> 8);                          /* Y and A, as in the original */
        nz8((u8)(handler >> 8));
        if (!r816_call_pc(0x800000 | rd16(0x0000)))
            r816_unknown_target(0x808A00, rd16(0x0000));
        return;
    }
    if (rd8(DP(MODE_HOLD)) != 0) {
        ret();
        return;
    }
    u16 list = table_word(ABS(0xB683), mode);          /* this mode's setup list */
    wr8(DP(0x02), (u8)list);
    wr8(DP(0x03), (u8)(list >> 8));
    X = (u8)((mode << 1) + 1);
    u8 step = rd8(DP(MODE_STEP));
    wr8(DP(MODE_STEP), (u8)(step + 1));
    u8 entry = (u8)(step - 1);
    Y = entry;                                         /* LDY $77 / DEY */
    SET_A8(list >> 8);                                 /* last byte the original loaded */
    if (!(entry & 0x80)) {                             /* steps 1..$80: list entry */
        u32 base = ((u32)DB << 16) + rd16(DP(0x02));
        u8 y = (u8)(entry << 1);
        wr8(DP(0x00), rd8((base + y) & 0xFFFFFF));
        y = (u8)(y + 1);
        u8 hi = rd8((base + y) & 0xFFFFFF);
        wr8(DP(0x01), hi);
        Y = y;
        SET_A8(hi);
        nz8(hi);
        fc = entry >> 7;                               /* TYA / ASL A: always 0 here */
        if (!r816_call_pc(0x800000 | rd16(0x0000)))
            r816_unknown_target(0x808A2B, rd16(0x0000));
        return;
    }
    /* step 0 (or >= $81): reset the mode's working state */
    push16(0x8A30); CALL(sub_80B21D_m1x1, 0x80B21D, 2);
    push16(0x8A33); CALL(sub_80B873_m1x1, 0x80B873, 2);
    for (u16 a = 0x0818; a != 0x008B; a--)
        wr8((u16)(D + a), 0);
    X = 0x8B;
    SET_A8(0);
    fc = 1; fz = 1; fn = 0;                            /* CPX #$008B ended the loop */
    ret();
}
