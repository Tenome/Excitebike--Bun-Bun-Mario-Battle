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
