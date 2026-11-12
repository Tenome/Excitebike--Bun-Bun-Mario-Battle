/*
 * Routines identical (instruction for instruction) to S1's hand-written
 * ones, carried over by tools/port_hand.py with their return addresses
 * moved. Checked by tools/difftest816.py on s4.
 */
#include "hand/hand.h"

enum {
    P1_HELD_HI = 0x20, P1_PRESS_HI = 0x21, P2_HELD_HI = 0x22, P2_PRESS_HI = 0x23,
    P1_HELD_LO = 0x24, P2_HELD_LO = 0x25, P1_PRESS_LO = 0x26, P2_PRESS_LO = 0x27,
    P1_PREV_HI = 0x28, P2_PREV_HI = 0x29, P1_PREV_LO = 0x2A, P2_PREV_LO = 0x2B,
};
enum { BSX_STATUS = 0x2190, BSX_CTRL = 0x2191, BSX_DATA = 0x2192, BSX_ERR = 0x2193,
       BSX_RESET = 0x2197, BSX_CHANNEL = 0x218E };
#define ARG_LEN     DP(0x00)    /* 16-bit byte count */
#define ARG_SRC     DP(0x02)    /* 16-bit source address */
#define ARG_SRCBANK DP(0x04)    /* source bank */
#define BS_CLOCK_F0   LONG(0x7FFFF0)
#define BS_CLOCK_F1   LONG(0x7FFFF1)
#define BS_CLOCK_F2   LONG(0x7FFFF2)
#define BS_CLOCK_F3   LONG(0x7FFFF3)
#define BS_CHANNEL    LONG(0x7FFFF7)    /* 16-bit: opened time channel */
#define BS_HOUR       LONG(0x7FFFF9)
#define BS_FRAME      LONG(0x7FFFFA)    /* 16-bit: frames into the minute */
#define BS_SYNCED     LONG(0x7FFFFC)    /* 16-bit: clock set from the stream */
#define BS_MINUTE     LONG(0x7FFFFF)
enum { MODE_ACTIVE = 0x75, GAME_MODE = 0x76, MODE_STEP = 0x77, MODE_HOLD = 0x50 };
#define QUEUE_HEAD   LONG(0x7F0000)



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

/*
 * $82:8232 DmaFillZero (M=0, X=1, RTL). Clears ARG_LEN bytes at Y:A.
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
 * $82:8000 ReadJoypads (M=1, X=1, RTL). Waits for auto-joypad read to finish,
 * then reads both pads into the direct page joypad block.
 */
void ReadJoypads_m1x1(void)
{
    while (rd8(ABS(REG_HVBJOY)) & 1)
        WAIT_TICK();
    fc = 0;                                 /* the wait loop's LSR leaves C = bit 0 = 0 */
    read_pad(REG_JOY1L, P1_HELD_LO, P1_PRESS_LO, P1_PREV_LO, P1_HELD_HI, P1_PRESS_HI, P1_PREV_HI);
    read_pad(REG_JOY2L, P2_HELD_LO, P2_PRESS_LO, P2_PREV_LO, P2_HELD_HI, P2_PRESS_HI, P2_PREV_HI);
    retl();
}
/*
 * $80:8535 NmiFlushVramQueue (M=1, X=1, RTL), in vblank. DMAs every queued
 * record to VRAM with channel 0, then empties the queue (head = $0002).
 * Head $FFFF means the queue is being written: skip this frame.
 */
void NmiFlushVramQueue_m1x1(void)
{
    push8(DB);
    push8(0x80); DB = pull8();                         /* PHK / PLB */
    rep(0x30);
    Y = 0;
    wr16(DP(0x02), 0);
    sep(0x20);
    X = 0;
    push8(DB);
    push8(0x7F); DB = pull8();                         /* queue bank */
    wr8(DP(0x00), 0x7F);
    rep(0x20);
    u16 head = rd16(ABS(0x0000));
    A = head;
    wr16(DP(0x04), head);
    int locked = head == 0xFFFF;
    int empty = rd16(DP(0x02)) == 0 && head == 0x0002;   /* $02 was just cleared */
    if (!locked && !empty) {
        u16 x = 2;
        do {
            wr16(LONG(0x004300), rd16(ABS(x + 3)));    /* DMAP0, BBAD0 */
            wr16(LONG(0x002116), rd16(ABS(x + 0)));    /* VMADD */
            wr16(LONG(0x004302), rd16(ABS(x + 5)));    /* A1T0 */
            wr16(LONG(0x004304), rd16(ABS(x + 7)));    /* A1B0 (and DAS0L, rewritten next) */
            wr16(LONG(0x004305), rd16(ABS(x + 8)));    /* DAS0 */
            sep(0x20);
            wr8(LONG(0x002115), rd8(ABS(x + 2)));      /* VMAIN */
            wr8(LONG(0x00420B), 0x01);                 /* MDMAEN: go */
            rep(0x20);
            x = rd16(ABS(x + 0x0A));                   /* next record */
            A = x;
        } while (x != rd16(DP(0x04)));
        X = x;
        A = 0x0002;
        wr16(ABS(0x0000), 0x0002);                     /* queue empty */
    }
    DB = pull8();
    sep(0x30);
    DB = pull8();
    retl();
}
/*
 * Stadium 1 per-frame housekeeping, hand-written (called from the main loop).
 */
#include "hand/hand.h"

/*
 * $80:B4CE StepLfsr (M=1, X=1, RTS). One step of the 96-bit shift register
 * in $2C-$37 (a random source): the new top bit is bit 1 of $2C XOR bit 1 of
 * $2D, shifted in from the left across all twelve bytes. $0000 is used as
 * scratch (absolute, so it lands at DB:0000).
 */
void StepLfsr_m1x1(void)
{
    u8 a = rd8(DP(0x2C)) & 0x02;
    wr8(ABS(0x0000), a);
    a = (u8)((rd8(DP(0x2D)) & 0x02) ^ rd8(ABS(0x0000)));
    lda8(a);
    fc = a != 0;
    for (int i = 0x2C; i <= 0x37; i++)
        wr8(DP(i), ror8(rd8(DP(i))));
    ret();
}
/*
 * $80:8603 Mode00_IntroWait (M=1, X=1, RTS). Waits for the race segment
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
 * $80:86E4 Mode0E_TitleWait (M=1, X=1, RTS). At minute 4 the race segment
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
/*
 * Stadium 1 intro text and title screen modes, hand-written.
 */
#include "hand/hand.h"

void QueueSound_m1x1(void);
void VramQueueCopy_m1x1(void);

/* $80:89C8 FinishModeSetup (RTS): last step of a setup list. Switch to the
 * mode's main handler ($75 = 1, step 0), set $79, start a fade-in. */
void FinishModeSetup_m1x1(void)
{
    lda8(0x01);
    wr8(DP(0x79), 0x01);
    wr8(DP(0x75), 0x01);
    SET_A8(lsr8(0x01));                     /* A = 0, C = 1 */
    wr8(DP(0x77), 0x00);
    lda8(0x01);
    wr8(ABS(0x1923), 0x01);                 /* fade in */
    ret();
}
/*
 * $80:D16E StepObjectX (RTL). Object X ($1500 table, 16-bit position
 * $1500/$1501 with fraction $1520) moves by its velocity $1510: the high
 * nibble is the signed whole part, the low nibble the fraction (in 16ths).
 * Leaves A = carry out of the fraction + whole part.
 */
void StepObjectX_m1x1(void)
{
    u8 v = rd8(ABS(0x1510 + X));
    wr8(DP(0x01), (u8)(v << 4));               /* fraction */
    u8 whole = lsr8(lsr8(lsr8(lsr8(v))));
    cmp8(whole, 0x08);
    if (whole >= 0x08)
        whole |= 0xF0;                         /* sign-extend the nibble */
    wr8(DP(0x00), whole);
    ldy8(whole & 0x80 ? 0xFF : 0x00);
    wr8(DP(0x02), (u8)Y);                      /* high byte of the step */
    lda8(rd8(ABS(0x1520 + X)));
    fc = 0;
    adc8(rd8(DP(0x01)));
    wr8(ABS(0x1520 + X), (u8)A);
    u8 carry = fc;
    push8(carry);                              /* PHA */
    lda8(rd8(ABS(0x1500 + X)));
    fc = carry;
    adc8(rd8(DP(0x00)));
    wr8(ABS(0x1500 + X), (u8)A);
    lda8(rd8(ABS(0x1501 + X)));
    adc8(rd8(DP(0x02)));
    wr8(ABS(0x1501 + X), (u8)A);
    lda8(pull8());
    fc = 0;
    adc8(rd8(DP(0x00)));
    retl();
}
/*
 * $80:CF5D ComputePlaces (RTS). For every rider X: place = 8 minus the
 * riders it is ahead of: riders with lower progress, and riders with equal
 * progress and a lower index.
 */
void ComputePlaces_m1x1(void)
{
    rep(0x20);
    for (int x = 0x0E; x >= 0; x -= 2) {
        X = (u16)x;
        wr8(DP(0x00), (u8)x);
        A = 0x0008;
        wr16(ABS(0x086E + x), 0x0008);
        for (int y = 0x0E; y >= 0; y -= 2) {
            Y = (u16)y;
            if ((u8)y == rd8(DP(0x00)))
                continue;
            u16 mine = rd16(ABS(0x085E + x)), theirs = rd16(ABS(0x085E + y));
            A = mine;
            if (mine < theirs)
                continue;
            if (mine == theirs && (u8)y >= rd8(DP(0x00)))
                continue;
            wr16(ABS(0x086E + x), dec16(rd16(ABS(0x086E + x))));
        }
    }
    X = 0xFE;
    Y = 0xFE;
    sep(0x20);
    ret();
}
/* $81:A025 ApplyVelocity (RTL): Y position $0289/$028A,X += speed $02A9,X
 * (fraction $02C9,X). */
void ApplyVelocity_m1x1(void)
{
    apply_speed(0x02AF, 0x02CF, 0x028F);
    retl();
}
/* $80:D228 ClearWords1530 (RTL): zero the low bytes of the four words at
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
/* $80:BA66 Nop_BD39 (RTS): eight NOPs, an empty hook. */
void Nop_BD39_m1x1(void)
{
    ret();
}
/* $80:C09F Tick08CC (RTS): count down the per-rider timer $08CC,X; it is
 * cancelled (zeroed) while any of bits 6-7 of $0178,X are set. */
void Tick08CC_m1x1(void)
{
    lda8(rd8(ABS(0x08CC + X)));
    if (fz) { ret(); return; }
    wr8(ABS(0x08CC + X), dec8(rd8(ABS(0x08CC + X))));
    lda8(rd8(ABS(0x017E + X)));
    and8(0xC0);
    if (fz) { ret(); return; }
    lda8(0x00);
    wr8(ABS(0x08CC + X), 0x00);
    ret();
}
/* $80:C741 DivideAX (RTS): divide A by X by repeated subtraction while the
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
/* $80:DBC6 BlankQueueRow (RTS): 15 blank tiles ($00FC) at $7F:000E. */
void BlankQueueRow_m1x1(void)
{
    fill_queue_row(0x1E, 0x00FC);
    ret();
}
/* $81:8C2A BlankQueueRowPal2 (RTL): 10 blank tiles, palette 2 ($08FC). */
void BlankQueueRowPal2_m1x1(void)
{
    fill_queue_row(0x14, 0x08FC);
    retl();
}
/* $80:9DA6 BeginFadeIn (RTS): start fading in ($1923 + 1), advance the mode
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
/* $80:AB34 SendSpeedToApu (RTS): engine pitch = player speed ($01C0:$01B8)
 * >> 3, low byte to APUIO2. */
void SendSpeedToApu_m1x1(void)
{
    lda8(rd8(ABS(0x01C6)));
    A = (u16)((A << 8) | (A >> 8));             /* XBA */
    nz8((u8)A);
    lda8(rd8(ABS(0x01BE)));
    rep(0x20);
    lda16(lsr16(lsr16(lsr16(A))));
    sep(0x20);
    wr8(ABS(0x2142), (u8)A);
    ret();
}
/*
 * Stadium 1: rider position and scroll arithmetic. Hand-written; ADC/SBC go
 * through the machine model so carries chain exactly as in the original.
 */
#include "hand/hand.h"

/*
 * $80:BCA9 RiderShadowY (RTS): $0979,X = ($090C,X - $0190,X - 8) * 2
 * + $01F0,X + 4, with the original's carry chaining (the second SBC and ADC
 * use the carry of the step before).
 */
void RiderShadowY_m1x1(void)
{
    lda8(rd8(ABS(0x090C + X)));
    fc = 1;
    sbc8(rd8(ABS(0x0196 + X)));
    sbc8(0x08);
    lda8(asl8((u8)A));
    fc = 0;
    adc8(rd8(ABS(0x01F6 + X)));
    adc8(0x04);
    wr8(ABS(0x0979 + X), (u8)A);
    ret();
}
/*
 * $80:A113 Add32 (RTS): the 32-bit value at $10-$13 += $04-$07. The high
 * word adds the low word's carry first ($0000 + $12 + C), then $06.
 */
void Add32_m1x1(void)
{
    rep(0x20);
    lda16(rd16(DP(0x04)));
    fc = 0;
    adc16(rd16(DP(0x10)));
    wr16(DP(0x10), A);
    lda16(0x0000);
    adc16(rd16(DP(0x12)));
    fc = 0;
    adc16(rd16(DP(0x06)));
    wr16(DP(0x12), A);
    sep(0x20);
    ret();
}
/* $80:D968 DispatchBy85 (RTS): tail-jump through the routine table at
 * $80:D409 indexed by $85 (pointer kept at $00/$01, X = 0). */
void DispatchBy85_m1x1(void)
{
    lda8(rd8(DP(0x85)));
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(0xD234 + Y))); wr8(DP(0x00), (u8)A);
    Y = inc8((u8)Y);
    lda8(rd8(ABS(0xD234 + Y))); wr8(DP(0x01), (u8)A);
    ldx8(0x00);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}
/* $80:F3C7 StartMode01 (RTS): mode 1 from step 0 with a 1800-frame timer
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
/* $80:9898 BsxReadSerialByte (RTL): clock 8 bits out of the BS-X serial
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
/* $80:9F76 GrowBar087E (RTS): while $087E < 15, count it up and redraw a
 * symmetric bar: $0474 = old length, and the length-1 ... 1 values going
 * out both ways from $0455+$1D/$0455+$21. */
void GrowBar087E_m1x1(void)
{
    lda8(rd8(ABS(0x087E)));
    cmp8((u8)A, 0x0F);
    if (fz) { ret(); return; }
    wr8(ABS(0x087E), inc8(rd8(ABS(0x087E))));
    wr8(ABS(0x0471), (u8)A);
    ldx8(0x1D);
    ldy8(0x21);
    for (;;) {
        lda8(dec8((u8)A));
        if (fz) break;
        wr8(ABS(0x0452 + X), (u8)A);
        wr8(ABS(0x0452 + Y), (u8)A);
        Y = inc8((u8)Y); Y = inc8((u8)Y);
        X = dec8((u8)X); X = dec8((u8)X);
        if (fn) break;
    }
    ret();
}
/* $80:9FB6 StepColourCycle (RTS): every 14 frames ($0882 countdown), copy
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
            lda16(rd16(LONGX(0x9CD000)));
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
/* $80:A36B MergeBsxFlags (RTL): OR the BS-X player data bits into the
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
/*
 * $80:B178 LaneFromY (RTS): for each of the 8 riders, $08A4,X = lane 5..1
 * from the track Y $01F0,X: 5 above Y $10, then one lane lower per 8 pixels.
 */
void LaneFromY_m1x1(void)
{
    ldx8(0x07);
    do {
        ldy8(0x05);
        lda8(rd8(ABS(0x01F6 + X)));
        fc = 1;
        sbc8(0x10);
        if (!fn) {
            for (;;) {
                Y = dec8((u8)Y);
                if (fz) break;
                fc = 1;
                sbc8(0x08);
                if (fn) break;
            }
        }
        lda8((u8)Y);
        wr8(ABS(0x08A4 + X), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/*
 * $80:F90C MarkFastRider (RTS): a rider on the ground ($01E8,X = 0) with
 * $01E0,X < 7 (signed compare by N) going faster than $02:80 (speed
 * $01C0:$01B8) gets $01C8,X = $FF.
 */
void MarkFastRider_m1x1(void)
{
    lda8(rd8(ABS(0x01EE + X)));
    if (!fz) { ret(); return; }
    lda8(rd8(ABS(0x01E6 + X)));
    cmp8((u8)A, 0x07);
    if (!fn) { ret(); return; }
    lda8(rd8(ABS(0x01C6 + X)));
    cmp8((u8)A, 0x03);
    if (!fc) {
        cmp8((u8)A, 0x02);
        if (!fz) { ret(); return; }
        lda8(rd8(ABS(0x01BE + X)));
        lda8(asl8((u8)A));
        if (!fc) { ret(); return; }
    }
    lda8(0xFF);
    wr8(ABS(0x01CE + X), 0xFF);
    ret();
}
/*
 * $80:CD0E DrawRiderShadowPair (RTS): two 8x8 sprites in the OAM buffer at
 * $16C0,Y and $16C4,Y: Y = A - 7 and A - 15, X = $01B0,X + 16 and + 8,
 * tiles $E0 and $F0.
 */
void DrawRiderShadowPair_m1x1(void)
{
    fc = 0; adc8(0xF9); wr8(ABS(0x16C0 + Y), (u8)A);
    fc = 0; adc8(0xF8); wr8(ABS(0x16C4 + Y), (u8)A);
    lda8(rd8(ABS(0x01B6 + X)));
    fc = 0; adc8(0x08); wr8(ABS(0x16C5 + Y), (u8)A);
    fc = 0; adc8(0x08); wr8(ABS(0x16C1 + Y), (u8)A);
    lda8(0xE0); wr8(ABS(0x16C2 + Y), 0xE0);
    lda8(0xF0); wr8(ABS(0x16C6 + Y), 0xF0);
    ret();
}
/*
 * $80:BD22 ScrollRivalsRelative (RTS): for riders 7..1 that are on the course
 * ($01D8,X != 0), move them by their own speed $0180,X minus the player's
 * $0180 (the camera follows the player): position $0198:$0190,X and the
 * column-relative $090C,X.
 */
void ScrollRivalsRelative_m1x1(void)
{
    ldx8(0x07);
    do {
        lda8(rd8(ABS(0x01DE + X)));
        if (!fz) {
            lda8(rd8(ABS(0x0186 + X)));
            fc = 0;
            adc8(rd8(ABS(0x0196 + X)));
            if (fc)
                wr8(ABS(0x019E + X), inc8(rd8(ABS(0x019E + X))));
            fc = 1;
            sbc8(rd8(ABS(0x0186)));
            wr8(ABS(0x0196 + X), (u8)A);
            if (!fc)
                wr8(ABS(0x019E + X), dec8(rd8(ABS(0x019E + X))));
            lda8(rd8(ABS(0x090C + X)));
            fc = 1;
            sbc8(rd8(ABS(0x0186)));
            wr8(ABS(0x090C + X), (u8)A);
        }
        X = dec8((u8)X);
    } while (!fz);
    ret();
}
/*
 * $80:C74F RiderLaneBand (RTS): $0958,X = which band of the track the rider
 * is in by Y position $01F0,X against the 5 limits at $80:BE1D (signed
 * compares), 0 while airborne ($01D0,X != 0) or past the last limit; bands
 * are 2 higher when $0208,X = 1.
 */
void RiderLaneBand_m1x1(void)
{
    ldy8(0x00);
    lda8(rd8(ABS(0x01D6 + X)));
    if (fz) {
        lda8(rd8(ABS(0x01F6 + X)));
        for (;;) {
            cmp8((u8)A, rd8(ABS(0xBB35 + Y)));
            if (fn) break;
            Y = inc8((u8)Y);
            cmp8((u8)Y, 0x05);
            if (!fn) { ldy8(0x00); break; }
        }
        lda8((u8)Y);
        if (!fz) {
            lda8(0x01);
            cmp8(0x01, rd8(ABS(0x020E + X)));
            if (fz) {
                Y = inc8((u8)Y);
                Y = inc8((u8)Y);
            }
        }
    }
    lda8((u8)Y);
    wr8(ABS(0x0958 + X), (u8)A);
    ret();
}
/*
 * $80:D5FD SetupSpriteRow15 (RTS): fill 15 OAM entries at $1700 from the
 * tables at $80:D7F5 (X), $80:D804 (Y) and $80:D7E6 (tile + $A0), attribute
 * $31, size bits $1860,X = 2 (large); $1958 counts up.
 */
void SetupSpriteRow15_m1x1(void)
{
    wr8(ABS(0x1958), inc8(rd8(ABS(0x1958))));
    ldx8(0x0E);
    do {
        lda8((u8)X);
        lda8(asl8((u8)A));
        lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
        lda8(rd8(ABS(0xD638 + X))); wr8(ABS(0x1700 + Y), (u8)A);
        lda8(rd8(ABS(0xD647 + X))); wr8(ABS(0x1701 + Y), (u8)A);
        lda8(rd8(ABS(0xD629 + X)));
        fc = 1;
        sbc8(0x60);
        wr8(ABS(0x1702 + Y), (u8)A);
        lda8(0x31); wr8(ABS(0x1703 + Y), 0x31);
        lda8(0x02); wr8(ABS(0x1860 + X), 0x02);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/*
 * $80:C173 RiderHeightStep (RTS), with the caller's carry: 16-bit
 * $08EC:$08E4,X += $08FC,X; then A = $08EC,X + $01 (+ carry), and $01 =
 * A - $08DC,X. On a carry out of the first add, $0904,X is halved; on a
 * borrow in the subtract, $0904,X = 1. Returns A = $01 with the subtract's
 * carry.
 */
void RiderHeightStep_m1x1(void)
{
    lda8(rd8(ABS(0x08E4 + X)));
    adc8(rd8(ABS(0x08FC + X)));
    wr8(ABS(0x08E4 + X), (u8)A);
    lda8(rd8(ABS(0x08EC + X)));
    adc8(0x00);
    wr8(ABS(0x08EC + X), (u8)A);
    adc8(rd8(DP(0x01)));
    if (fc)
        wr8(ABS(0x0904 + X), lsr8(rd8(ABS(0x0904 + X))));
    fc = 1;
    sbc8(rd8(ABS(0x08DC + X)));
    wr8(DP(0x01), (u8)A);
    if (!fc) {
        lda8(0x01);
        wr8(ABS(0x0904 + X), 0x01);
    }
    lda8(rd8(DP(0x01)));
    ret();
}
/*
 * $81:9FCE ObjectFacing (RTL): $02D9,X = the direction of the larger of the
 * two speed components $0299,X and $02A9,X (signed): 3/2 for $0299 positive
 * / negative, 1/0 for $02A9; unchanged when they are equal. Y is preserved.
 */
void ObjectFacing_m1x1(void)
{
    push8((u8)Y);
    ldy8(0x03);
    lda8(rd8(ABS(0x029F + X)));
    if (fn) {
        Y = dec8((u8)Y);
        eor8(0xFF);
        lda8(inc8((u8)A));
    }
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x02), (u8)Y);
    ldy8(0x01);
    lda8(rd8(ABS(0x02AF + X)));
    if (fn) {
        Y = dec8((u8)Y);
        eor8(0xFF);
        lda8(inc8((u8)A));
    }
    cmp8((u8)A, rd8(DP(0x00)));
    if (!fz) {
        if (!fc)
            ldy8(rd8(DP(0x02)));
        lda8((u8)Y);
        wr8(ABS(0x02DF + X), (u8)A);
    }
    Y = pull8();
    nz8((u8)Y);
    retl();
}
/*
 * $80:FE68 AddScore (RTS): add the six decimal digits $09AC-$09B1 to the
 * score digits $0CE5-$0CEA (index 5 = units), carrying tens up, then clear
 * the addend. (A digit that does not carry leaves its "minus 10" in $0CE4.)
 */
void AddScore_m1x1(void)
{
    ldx8(0x05);
    fc = 0;
    for (;;) {
        lda8(rd8(ABS(0x0CE5 + X)));
        adc8(rd8(ABS(0x09AC + X)));
        wr8(ABS(0x0CE5 + X), (u8)A);
        X = dec8((u8)X);
        if (fn) break;
        fc = 1;
        sbc8(0x0A);
        if (fc) {
            wr8(ABS(0x0CE6 + X), (u8)A);
            fc = 1;
        } else {
            wr8(ABS(0x0CE4), (u8)A);
            fc = 0;
        }
    }
    lda8(0x00);
    ldx8(0x05);
    do {
        wr8(ABS(0x09AC + X), 0x00);
        X = dec8((u8)X);
    } while (!fn);
    ret();
}
/* $81:9FF7 ApplyVelocityX (RTL): X position $0279/$027A,X += speed $0299,X
 * (fraction $02B9,X). */
void ApplyVelocityX_m1x1(void)
{
    apply_speed(0x029F, 0x02BF, 0x027F);
    retl();
}
/*
 * $81:93FE SpawnPlayerSpark (RTL): start effect slot 0 at the player's
 * position ($0190, $01B0): kind 3 ($0110), 3 frames ($0120), speeds
 * $F8/$D0 ($0138/$0140), and set $09AD = 1.
 */
void SpawnPlayerSpark_m1x1(void)
{
    push8(DB);
    lda8(0x81);
    push8(0x81);
    DB = pull8(); nz8(DB);
    push8((u8)X);
    ldx8(0x00);
    lda8(rd8(ABS(0x0196))); wr8(ABS(0x0106 + X), (u8)A);
    lda8(rd8(ABS(0x01B6))); wr8(ABS(0x010E + X), (u8)A);
    lda8(0x00); wr8(ABS(0x011E + X), 0x00);
    lda8(0x03); wr8(ABS(0x0116 + X), 0x03);
    lda8(0xF8); wr8(ABS(0x013E + X), 0xF8);
    lda8(0xD0); wr8(ABS(0x0146 + X), 0xD0);
    lda8(0x03); wr8(ABS(0x0126 + X), 0x03);
    wr8(ABS(0x012E + X), 0);
    lda8(0x01); wr8(ABS(0x09AD), 0x01);
    X = pull8(); nz8((u8)X);
    DB = pull8(); nz8(DB);
    retl();
}
/*
 * $81:96C2 StepSpark (RTS), effect slot Y: count its frames $0120 down (kind
 * $0110 cleared at zero), rise one pixel every other frame, and draw it as
 * sprite (Y + 4) in the OAM buffer: tile $8E, attribute $2D, large.
 */
void StepSpark_m1x1(void)
{
    X = Y; nz8((u8)X);
    u8 t = dec8(rd8(ABS(0x0126 + X)));
    wr8(ABS(0x0126 + X), t);
    if (t == 0)
        wr8(ABS(0x0116 + X), 0);
    lda8(rd8(ABS(0x0126 + X)));
    lda8(lsr8((u8)A));
    if (fc)
        wr8(ABS(0x010E + X), dec8(rd8(ABS(0x010E + X))));
    lda8((u8)X);
    lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(0x0106 + X))); wr8(ABS(0x1610 + Y), (u8)A);
    lda8(rd8(ABS(0x010E + X))); wr8(ABS(0x1611 + Y), (u8)A);
    lda8(0x8E); wr8(ABS(0x1612 + Y), 0x8E);
    lda8(0x2D); wr8(ABS(0x1613 + Y), 0x2D);
    lda8((u8)Y);
    lda8(lsr8((u8)A));
    lda8(lsr8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    lda8(0x02);
    wr8(ABS(0x1824 + Y), 0x02);
    ret();
}
