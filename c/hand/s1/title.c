/*
 * Stadium 1 intro text and title screen modes, hand-written.
 */
#include "hand/hand.h"

void QueueSound_m1x1(void);
void VramQueueCopy_m1x1(void);

/* $80:8D16 FinishModeSetup (RTS): last step of a setup list. Switch to the
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

/* $80:8A6D Mode0D_Setup (RTS): the rules text (mode $0D). From minute 3 on
 * there is no time to read it: go straight to mode 0. Otherwise BG3 tilemap
 * at $2A00 (BG3SC $2A), BG1/2 tiles at $4000, BG3 tiles at $1000, scroll timer
 * $88 = $0A8C frames, fade in. */
void Mode0D_Setup_m1x1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x03);
    if (fc) {
        lda8(0x00);
        wr8(ABS(0x0076), 0);
        wr8(DP(0x75), 0);
        wr8(DP(0x77), 0);
        ret();
        return;
    }
    lda8(0x2A); wr8(ABS(0x2109), 0x2A);     /* BG3SC */
    lda8(0x44); wr8(ABS(0x210B), 0x44);     /* BG12NBA */
    lda8(0x01); wr8(ABS(0x210C), 0x01);     /* BG34NBA */
    rep(0x20);
    A = 0x0A8C; wr16(DP(0x88), 0x0A8C);
    sep(0x20);
    wr8(ABS(0x1923), inc8(rd8(ABS(0x1923))));
    FinishModeSetup_m1x1();                 /* JMP */
}

/* Queue a block of text tiles from bank $80 (VramQueueCopy arguments). */
static void queue_text(u16 jsr_ret, u16 vram, u16 src, u16 size)
{
    rep(0x20);
    wr16(DP(0x00), vram);
    wr16(DP(0x02), src);
    A = size;
    wr16(DP(0x10), size);                   /* words per row, rows */
    sep(0x20);
    lda8(0x80);
    wr8(DP(0x04), 0x80);
    push16(jsr_ret);
    VramQueueCopy_m1x1();
}

/* $80:8B14 Mode0E_Setup (RTS): the title. Title timer $88 = $0384, BG3
 * tilemap at $1900, main screen BG1-3 + sprites (TM $17), sounds $2D and $45.
 * Before minute 4 also queue the "please wait" line ($80:8BCF, 2 rows x 24
 * tiles at VRAM $06C5); always queue $80:8B7F (2 x 20 tiles at $0726). */
void Mode0E_Setup_m1x1(void)
{
    rep(0x20);
    A = 0x0384; wr16(DP(0x88), 0x0384);
    sep(0x20);
    lda8(0x19); wr8(ABS(0x2109), 0x19);     /* BG3SC */
    lda8(0x01); wr8(ABS(0x210C), 0x01);     /* BG34NBA */
    lda8(0x17); wr8(ABS(0x1905), 0x17);     /* TM shadow */
    lda8(0x01); wr8(DP(0x8C), 0x01);
    ldx8(0x02);
    wr8(DP(0x92), 0x02);
    wr8(DP(0x84), 0x02);
    lda8(0x2D); push8(0x80); push16(0x8B3B); QueueSound_m1x1();
    lda8(0x45); push8(0x80); push16(0x8B41); QueueSound_m1x1();
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x04);
    if (!fc)
        queue_text(0x8B63, 0x06C5, 0x8BCF, 0x0218);
    queue_text(0x8B7D, 0x0726, 0x8B7F, 0x0214);
    ret();
}

/* $80:8A9D Mode0D_IntroText (RTS): scroll the rules text up one line every
 * 5 frames ($8A): line $9E's two tile rows come from $8B:E800 + line / 8 * 64
 * and go to the BG3 tilemap row 28 lines below the top (wrapping in 64 rows).
 * When the timer $88 runs out (or Start with the debug flag), go to mode 0. */
void Mode0D_IntroText_m1x1(void)
{
    int done = 0;
    lda8(rd8(LONG(0x81800E)));
    if ((u8)A != 0) {
        lda8(rd8(DP(0x21)));
        and8(0x10);
        done = (u8)A != 0;
    }
    if (!done) {
        lda8(rd8(DP(0x8A)));
        if ((u8)A == 0) {
            lda8(0x05);
            wr8(DP(0x8A), 0x05);
            rep(0x20);
            u16 line = inc16(rd16(DP(0x9E)));
            wr16(DP(0x9E), line);
            A = (u16)((line & 0xFFF8) << 3);
            fc = 0; adc16(0xE800);
            wr16(DP(0x02), A);              /* source $8B:E800 + 8 * (line & ~7) */
            sep(0x20);
            lda8(0x8B);
            wr8(DP(0x04), 0x8B);
            rep(0x20);
            A = 0x0220; wr16(DP(0x10), 0x0220);   /* 32 words, 2 rows */
            A = rd16(ABS(0x009E));
            fc = 0; adc16(0x00E0);
            A = (u16)((A & 0x01F8) << 2);
            fc = 0; adc16(0x2800);
            wr16(DP(0x00), A);              /* BG3 tilemap $2800 + row */
            sep(0x20);
            push16(0x8AE6);
            VramQueueCopy_m1x1();
        }
        wr8(DP(0x8A), dec8(rd8(DP(0x8A))));
        rep(0x20);
        u16 t = dec16(rd16(DP(0x88)));
        wr16(DP(0x88), t);
        sep(0x20);
        if (t != 0) {
            ret();
            return;
        }
    }
    lda8(0x00);
    wr8(DP(0x76), 0x00);
    wr8(DP(0x75), 0);
    wr8(DP(0x77), 0);
    wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    ret();
}
