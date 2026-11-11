/*
 * Stadium 1 kernel routines, hand-written. Each replaces the lifted routine
 * of the same name (see hand.txt) and is checked by tools/difftest816.py.
 */
#include "hand/hand.h"
#include "lifted.h"

/* Direct page scratch used as call arguments by the DMA helpers. */
#define ARG_LEN     DP(0x00)    /* 16-bit byte count */
#define ARG_SRC     DP(0x02)    /* 16-bit source address */
#define ARG_SRCBANK DP(0x04)    /* source bank */

/* Joypad state (direct page, see config/s1/ram.txt). */
enum {
    P1_HELD_HI = 0x20, P1_PRESS_HI = 0x21, P2_HELD_HI = 0x22, P2_PRESS_HI = 0x23,
    P1_HELD_LO = 0x24, P2_HELD_LO = 0x25, P1_PRESS_LO = 0x26, P2_PRESS_LO = 0x27,
    P1_PREV_HI = 0x28, P2_PREV_HI = 0x29, P1_PREV_LO = 0x2A, P2_PREV_LO = 0x2B,
};

/*
 * $80:8539 DmaKick (M=0, X=1, RTS). Called with the direct page at $4300, so
 * the "scratch" stores land in DMA channel 0: A = source address (A1T0),
 * X = source bank (A1B0), Y = channel mask written to MDMAEN.
 */
void DmaKick_m0x1(void)
{
    wr16(DP(0x02), A);
    wr8(DP(0x04), (u8)X);
    wr8(ABS(REG_MDMAEN), (u8)Y);
    ret();
}

/*
 * $80:8541 DmaCopyToWram (M=0, X=1, RTL). Copies ARG_LEN bytes from
 * ARG_SRCBANK:ARG_SRC to WRAM address Y:A through WMDATA with DMA channel 0.
 */
void DmaCopyToWram_m0x1(void)
{
    wr16(ABS(REG_WMADDL), A);
    wr8(ABS(REG_WMADDH), (u8)Y);
    wr16(ABS(REG_DMAP0), 0x8000);           /* DMAP0 = $00 (A->B), BBAD0 = $80 (WMDATA) */
    wr16(ABS(REG_A1T0L), rd16(ARG_SRC));
    wr8(ABS(REG_A1B0), rd8(ARG_SRCBANK));
    wr16(ABS(REG_DAS0L), rd16(ARG_LEN));
    wr8(ABS(REG_MDMAEN), 0x01);
    A = rd16(ARG_LEN);
    Y = 0x01;
    retl();
}

/*
 * $80:8562 DmaFillZero (M=0, X=1, RTL). Clears ARG_LEN bytes at Y:A.
 * Sets the Mode 7 multiplier to 1 x 0 so MPYL ($2134) reads zero, then DMAs
 * from that register (B->A, fixed B address) into memory.
 */
void DmaFillZero_m0x1(void)
{
    u16 dst = A;
    u8 bank = (u8)Y;
    wr16(ARG_SRC, 0);                        /* also used as the zero for M7B */
    wr16(ABS(REG_A1T0L), dst);
    wr8(ABS(REG_A1B0), bank);
    wr8(ABS(REG_M7A), 0x01);                 /* M7A = $0001 (two writes) */
    wr8(ABS(REG_M7A), 0x00);
    wr8(ABS(REG_M7B), rd8(ARG_SRC));         /* x 0 */
    wr16(ABS(REG_DMAP0), 0x3480);            /* DMAP0 = $80 (B->A), BBAD0 = $34 (MPYL) */
    wr16(ABS(REG_DAS0L), rd16(ARG_LEN));
    wr8(ABS(REG_MDMAEN), 0x01);
    A = rd16(ARG_LEN);
    Y = 0x01;
    retl();
}

/*
 * $80:82B6 QueueSound (M=1, X=1, RTL). Appends sound command A to the
 * 64-entry ring buffer at $1400 (write index $65) and remembers it in $71.
 * The original saves P and X on the stack around the work (kept: the stack
 * bytes are visible in WRAM).
 */
void QueueSound_m1x1(void)
{
    u8 cmd = (u8)A;
    push8(getP());
    push16(X);
    wr8(ABS(0x0071), cmd);
    u8 slot = rd8(ABS(0x0065));
    wr8(ABS(0x1400 + slot), cmd);
    slot = (u8)((slot + 1) & 0x3F);
    wr8(ABS(0x0065), slot);
    SET_A8(slot);
    X = pull16();
    setP(pull8());
    retl();
}

/* One pad: ignore anything that is not a standard controller (low nibble of
 * JOYxL set), then update held / newly pressed / previous for both halves. */
static void read_pad(u16 reg_lo, u8 held_lo, u8 press_lo, u8 prev_lo,
                     u8 held_hi, u8 press_hi, u8 prev_hi)
{
    wr8(DP(0x00), 0);
    wr8(DP(0x01), 0);
    u8 lo = rd8(ABS(reg_lo));
    if ((lo & 0x0F) == 0) {
        wr8(DP(0x00), lo);
        wr8(DP(0x01), rd8(ABS(reg_lo + 1)));
    }
    lo = rd8(DP(0x00));
    wr8(DP(held_lo), lo);
    /* EOR prev / AND held: the original re-reads the held byte from memory */
    wr8(DP(press_lo), (u8)((lo ^ rd8(DP(prev_lo))) & rd8(DP(held_lo))));
    wr8(DP(prev_lo), lo);
    u8 hi = rd8(DP(0x01));
    wr8(DP(held_hi), hi);
    u8 pressed = (u8)((hi ^ rd8(DP(prev_hi))) & rd8(DP(held_hi)));
    wr8(DP(press_hi), pressed);
    wr8(DP(prev_hi), hi);
    SET_A8(pressed);
    Y = hi;
}

/*
 * $80:B4D9 ReadJoypads (M=1, X=1, RTL). Waits for auto-joypad read to finish,
 * then reads both pads into the direct page joypad block.
 */
void ReadJoypads_m1x1(void)
{
    while (rd8(ABS(REG_HVBJOY)) & 1)
        WAIT_TICK();
    read_pad(REG_JOY1L, P1_HELD_LO, P1_PRESS_LO, P1_PREV_LO, P1_HELD_HI, P1_PRESS_HI, P1_PREV_HI);
    read_pad(REG_JOY2L, P2_HELD_LO, P2_PRESS_LO, P2_PREV_LO, P2_HELD_HI, P2_PRESS_HI, P2_PREV_HI);
    retl();
}

/*
 * $81:A01B LoadCourse (M=1, X=1, RTL). Expands the current course into the
 * track buffer $7F:4000 (one byte per column, see tools/course.py).
 *
 *   course = $0CD7 != 0 ? special course ($81:800C * 2 + 8)
 *                       : $81:800C * 4 + $7C       ($81:800C picks the Stadium)
 *   bytes from $81:8C81 + start[course], count[course] source bytes:
 *     b & $40 -> run: next byte n, b written n times
 *     b == $09 -> also written to the next five columns
 *
 * Does nothing when the buffer already holds a course and $7D (laps run?) is
 * non-zero. The source counter lives in $00 and is decremented as a byte,
 * like the original.
 */
void LoadCourse_m1x1(void)
{
    if (rd16(LONG(0x7F4000)) != 0 && (rd16(DP(0x7D)) & 0xFF) != 0) {
        sep(0x30);
        retl();
        return;
    }
    rep(0x30);
    for (int i = 0x0FFE; i >= 0; i -= 2)
        wr16(LONG(0x7F4000 + i), 0);

    u16 stadium = rd16(LONG(0x81800C));
    u16 index;
    if (rd16(ABS(0x0CD7)) != 0) {
        index = (u16)(stadium * 2 + 0x10);
    } else {
        wr16(DP(0x00), rd16(DP(0x7C)) & 0xFF);
        index = (u16)((stadium * 4 + rd16(DP(0x00))) * 2);
    }
    wr16(DP(0x00), rd16(LONG(0x818C6D + index)));     /* source byte count */
    u16 src = rd16(LONG(0x818C59 + index));            /* offset into $81:8C81 */

    sep(0x20);
    push8(DB);
    push8(0x7F);                                       /* LDA #$7F / PHA / PLB */
    DB = pull8();                                      /* buffer writes use DB */
    u16 col = 0;
    do {
        u8 b = rd8(LONG(0x818C81 + src));
        wr8(ABS(0x4000 + col), b);
        if (b == 0x09)
            for (int k = 1; k <= 5; k++)
                wr8(ABS(0x4000 + col + k), 0x09);
        if (b & 0x40) {
            wr8(DP(0x02), b);
            wr8(DP(0x00), (u8)(rd8(DP(0x00)) - 1));
            src++;
            u16 n = rd8(LONG(0x818C81 + src));
            push16(src);                               /* PHX ... PLX around the run */
            for (u16 k = 0; k < n; k++)
                wr8(ABS(0x4000 + col + k), rd8(DP(0x02)));
            src = pull16();
            col = (u16)(col + n - 1);
        }
        src++;
        col++;
        wr8(DP(0x00), (u8)(rd8(DP(0x00)) - 1));
    } while (rd8(DP(0x00)) != 0);
    X = src;
    Y = col;
    sep(0x10);
    DB = pull8();
    retl();
}

/* BS-X handshake area at the top of WRAM ($7F:FFE0-$7F:FFFF). */
#define BS_CLOCK_F0   LONG(0x7FFFF0)
#define BS_CLOCK_F1   LONG(0x7FFFF1)
#define BS_CLOCK_F2   LONG(0x7FFFF2)
#define BS_CLOCK_F3   LONG(0x7FFFF3)
#define BS_CHANNEL    LONG(0x7FFFF7)    /* 16-bit: opened time channel */
#define BS_HOUR       LONG(0x7FFFF9)
#define BS_FRAME      LONG(0x7FFFFA)    /* 16-bit: frames into the minute */
#define BS_SYNCED     LONG(0x7FFFFC)    /* 16-bit: clock set from the stream */
#define BS_MINUTE     LONG(0x7FFFFF)
enum { BSX_STATUS = 0x2190, BSX_CTRL = 0x2191, BSX_DATA = 0x2192, BSX_ERR = 0x2193,
       BSX_RESET = 0x2197, BSX_CHANNEL = 0x218E };

/* $81:F999 (JSR, keeps A): restart the minute's frame counter; X = 1 marks
 * "clock taken from the stream this frame". Stack traffic as the original. */
static void restart_minute(u16 ret_addr, u8 a)
{
    push16(ret_addr);
    push8(a);
    wr16(BS_FRAME, 0);
    X = 1;
    pull8();
    pull16();
}

/*
 * $81:F8BB BsxPollStream (M=1, X=1, RTL), once per frame from the main loop.
 *
 * With a satellite receiver: when a packet is waiting on the opened time
 * channel and it is a clean clock packet, copy minute, hour and the other
 * clock bytes into the handshake area (restarting the frame counter when the
 * minute changes or on the first packet), then re-arm the stream.
 * Always: run the local clock. $7F:FFFA counts frames; after $0E4C (3660)
 * frames the minute advances (and $B0 counts it). At minute 60, unless the
 * hour is $11, write $80 to $2197 (end of broadcast: the BS-X unit takes over).
 */
void BsxPollStream_m1x1(void)
{
    push8(getP());
    sep(0x30);
    X = 0;
    if (rd8(ABS(BSX_STATUS)) != 0 && rd16(ABS(BSX_CHANNEL)) == rd16(BS_CHANNEL)) {
        if ((rd8(ABS(BSX_ERR)) & 0x0C) == 0) {
            u8 b = 0;
            for (int i = 0; i < 6; i++)                /* header: 5 skipped, 6th = 1 */
                b = rd8(ABS(BSX_DATA));
            int ok = b == 1 && rd8(ABS(BSX_DATA)) == 1;
            if (ok) {
                u8 z = rd8(ABS(BSX_DATA));
                z |= rd8(ABS(BSX_DATA));
                z |= rd8(ABS(BSX_DATA));
                ok = z == 0;
            }
            if (ok) {
                rd8(ABS(BSX_DATA));
                u8 minute = rd8(ABS(BSX_DATA));
                if (minute != rd8(BS_MINUTE))
                    restart_minute(0xF913, minute);
                wr8(BS_MINUTE, minute);
                wr8(BS_HOUR, rd8(ABS(BSX_DATA)));
                wr8(BS_CLOCK_F3, rd8(ABS(BSX_DATA)));
                wr8(BS_CLOCK_F2, rd8(ABS(BSX_DATA)));
                wr8(BS_CLOCK_F1, rd8(ABS(BSX_DATA)));
                rd8(ABS(BSX_DATA));
                u8 last = rd8(ABS(BSX_DATA));
                wr8(BS_CLOCK_F0, last);
                if (rd8(BS_SYNCED) == 0) {
                    restart_minute(0xF946, last);
                    wr8(BS_SYNCED, 1);
                }
            }
        }
        wr8(ABS(BSX_CTRL), 1);                     /* re-arm the stream */
        wr8(ABS(BSX_DATA), 1);
    }

    /* local clock */
    u16 frame = rd16(BS_FRAME);
    if (X == 0) {
        frame++;
        if (frame == 0x0E4C) {
            wr8(DP(0xB0), (u8)(rd8(DP(0xB0)) + 1));
            u8 minute = (u8)(rd8(BS_MINUTE) + 1);
            wr8(BS_MINUTE, minute);
            if (minute == 0x3C && rd8(BS_HOUR) != 0x11)
                wr8(ABS(BSX_RESET), 0x80);
            wr16(BS_SYNCED, 0);
            frame = 0;
        }
    }
    wr16(BS_FRAME, frame);
    setP(pull8());
    retl();
}

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
        if (!r816_call_pc(0x800000 | rd16(0x0000)))
            r816_unknown_target(0x808A2B, rd16(0x0000));
        return;
    }
    /* step 0 (or >= $81): reset the mode's working state */
    push16(0x8A30); sub_80B21D_m1x1();
    push16(0x8A33); sub_80B873_m1x1();
    for (u16 a = 0x0818; a != 0x008B; a--)
        wr8((u16)(D + a), 0);
    X = 0x8B;
    SET_A8(0);
    ret();
}
