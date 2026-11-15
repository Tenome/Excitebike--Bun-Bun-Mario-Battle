/*
 * Stadium 3 (the 1997 S3 dump): routines that differ from Stadium 4's and
 * so are written here rather than carried over by tools/port_hand.py
 * (from_s4.c). Checked by tools/difftest816.py s3.
 */
#include "hand/hand.h"
#include "ram.h"

void QueueTileRows_m1x1(void);
void QueueSoundB_m1x1(void);
void QueueFiveDigits_m1x1(void);

/* The end screen's box rows ($7E:5800 + 256 * hi + lo, two rows of $20) to
 * VRAM $0300 through QueueTileRows; back is the JSR's return address. */
static void end_box3(u8 lo, u8 hi, u16 back)
{
    lda8(lo); wr8(DP(0x02), lo);
    lda8(hi); wr8(DP(0x03), hi);
    lda8(0x7E); wr8(DP(0x04), 0x7E);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(back); QueueTileRows_m1x1();
}

/* $80:99EE EndScreenCode (RTS): after the $88 delay the code box
 * ($7E:5880); then A ($26 bit 7): the step $0450 two on, $88 = $20, the
 * next box ($7E:5980), the five digits at $11:5730 to VRAM $0A15 and sound
 * $34. This is Stadium 2's version (Stadium 4 sets step $0A with sound
 * $43 and does not redraw the box). */
void EndScreenCode_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        u8 t = dec8(rd8(DP(MODE_TIMER)));
        wr8(DP(MODE_TIMER), t);
        if (!fz) { ret(); return; }
        end_box3(0x80, 0x58, 0x9A14);
    }
    lda8(rd8(DP(P1_PRESS_LO)));
    and8(0x80);
    if (fz) { ret(); return; }
    wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
    wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    end_box3(0x80, 0x59, 0x9A43);
    rep(0x20);
    lda16(0x0A15); wr16(DP(0x00), 0x0A15);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9A59); QueueFiveDigits_m1x1();
    lda8(0x34);
    push8(0x80); push16(0x9A5F); QueueSoundB_m1x1();
    ret();
}

/* $80:9A61 EntryMenuStep (RTS): after the $88 delay the menu box
 * ($7E:5900); then A: sound $34 at the code choice (5 in $0451), $43
 * otherwise; either way to step $0451 with $88 = $20 and the next box
 * ($7E:5980). (Stadium 4 jumps straight to the code step $0A.) */
void EntryMenuStep_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        u8 t = dec8(rd8(DP(MODE_TIMER)));
        wr8(DP(MODE_TIMER), t);
        if (!fz) { ret(); return; }
        end_box3(0x00, 0x59, 0x9A87);
    }
    lda8(rd8(DP(P1_PRESS_LO)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x0451)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x34);
        push8(0x80); push16(0x9A9A); QueueSoundB_m1x1();
    } else {
        lda8(0x43);
        push8(0x80); push16(0x9AA2); QueueSoundB_m1x1();
    }
    lda8(rd8(ABS(0x0451))); wr8(ABS(END_STEP), (u8)A);
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    end_box3(0x80, 0x59, 0x9ACB);
    ret();
}

void CourseSelectPreview_m1x1(void);
void JumpRunSteps_m1x1(void);

/* $80:B7F5 (RTS): the course-select preview. */
void CourseSelectStep_m1x1(void)
{
    push8(0x80); push16(0xB7F8); CALL(CourseSelectPreview_m1x1, 0x81C1AE, 3);
    ret();
}

/* $80:B7FA (RTS): the jump contest's run steps (mode entry). */
void JumpRunStepsMode_m1x1(void)
{
    push8(0x80); push16(0xB7FD); JumpRunSteps_m1x1();
    ret();
}

/* $80:F01A StopWhenStill (RTS): when the speed bytes $01BE, $01C6 and $01CE
 * are all zero: $CD = 1, $CF = $20, set bit 0 of $63. (Stadium 4 also puts
 * the rider at height $B0, $01B6.) */
void StopWhenStill_m1x1(void)
{
    lda8(rd8(ABS(RIDER_SPEED_LO)));
    ora8(rd8(ABS(RIDER_SPEED_HI)));
    ora8(rd8(ABS(RIDER_BUMPED)));
    if (fz) {
        lda8(0x01); wr8(DP(0xCD), 0x01);
        lda8(0x20); wr8(DP(0xCF), 0x20);
        lda8(0x01);
        u8 m = rd8(DP(FRAME_COUNT));
        fz = (m & 0x01) == 0;
        wr8(DP(FRAME_COUNT), m | 0x01);
    }
    ret();
}

/* $80:9D82 ModeCountdown (RTS): count $88 down (16-bit); with $0A8C left,
 * $A4 = $0100 (and $A6 = $0100 in the Stadium 4 half, $81:800C set); at 0
 * game mode $11 ($75 / $77 cleared, $1923 - 1). Stadium 4's goes to mode
 * $0F and sets $88 = $20 instead. */
void ModeCountdown_m1x1(void)
{
    rep(0x20);
    wr16(DP(MODE_TIMER), dec16(rd16(DP(MODE_TIMER))));
    lda16(rd16(DP(MODE_TIMER)));
    cmp16(A, 0x0A8C);
    if (fz) {
        lda16(0x0100); wr16(DP(0xA4), 0x0100);
        lda16(rd16(LONG(0x81800C)));
        if (!fz) { lda16(0x0100); wr16(DP(0xA6), 0x0100); }
    }
    lda16(rd16(DP(MODE_TIMER)));
    sep(0x20);
    if (!fz) { ret(); return; }
    lda8(0x11); wr8(DP(GAME_MODE), 0x11);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    ret();
}

void ChangeMode_m1x1(void);

/* $80:865A TitleScrollStep (RTS): the scrolling title picture. With the
 * debug flag ($81:800E) Select jumps straight to a race (screen off, mode
 * $03, final flag; Stadium 4 starts the jump contest, course 1, instead) and Start leaves. Every 5 frames
 * ($8A) the picture scrolls one line ($A6 + 1) and its next row is queued
 * ($7E:4800 + row, $20 x 2 at VRAM $2800 + line); when the timer $88 runs
 * out, or on Start, back to mode 0 (fade). */
void TitleScrollStep_m1x1(void)
{
    lda8(rd8(LONG(0x81800E)));
    if (!fz) {
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x20);
        if (!fz) {
            lda8(0x80);
            wr8(ABS(INIDISP), 0x80);
            wr8(ABS(SH_INIDISP), 0x80);
            lda8(0x19); wr8(ABS(BG3SC), 0x19);
            lda8(0x17); wr8(ABS(SH_TM), 0x17);
            lda8(0x03); wr8(DP(GAME_MODE), 0x03);
            lda8(0x01); wr8(DP(FINAL_RACE), 0x01);
            ChangeMode_m1x1();
            return;
        }
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x10);
        if (!fz) goto leave;
    }
    lda8(rd8(DP(TIMER_A)));
    if (fz) {
        lda8(0x05); wr8(DP(TIMER_A), 0x05);
        rep(0x20);
        wr16(DP(0xA6), inc16(rd16(DP(0xA6))));
        lda16(rd16(DP(0xA6)));
        and16(0xFFF8);
        lda16(asl16(A)); lda16(asl16(A)); lda16(asl16(A));
        fc = 0; adc16(0x4800);
        wr16(DP(0x02), A);
        sep(0x20);
        lda8(0x7E); wr8(DP(0x04), 0x7E);
        rep(0x20);
        lda16(0x0220); wr16(DP(0x10), 0x0220);
        lda16(rd16(ABS(0x00A6)));
        fc = 0; adc16(0x00E0);
        and16(0x01F8);
        lda16(asl16(A)); lda16(asl16(A));
        fc = 0; adc16(0x2800);
        wr16(DP(0x00), A);
        sep(0x20);
        push16(0x86C8); QueueTileRows_m1x1();
    }
    wr8(DP(TIMER_A), dec8(rd8(DP(TIMER_A))));
    rep(0x20);
    {
        u16 t = dec16(rd16(DP(MODE_TIMER)));
        wr16(DP(MODE_TIMER), t);
    }
    sep(0x20);
    if (!fz) { ret(); return; }
leave:
    lda8(0x00);
    wr8(DP(GAME_MODE), 0x00);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    ret();
}

/* $80:EBD9 ReadBonusTrack (RTS): the bonus course's ReadJumpTrack. As
 * that routine (landing points, height, tile $09D3 into $0216, tile flags
 * $0A53, tile $84), plus: tile 0 starts the run-out ($C7 = 2, height $80)
 * once the zoom $A2 is $0100 at ground height, or else the drop ($C7 = 1,
 * $C8 = scroll); tiles 1-$27 end the course once (sound $2B). Stadium 4
 * adds the landing on tile $A7. */
void ReadBonusTrack_m1x1(void)
{
    lda8(rd8(DP(JUMP_STATE)));
    cmp8((u8)A, 0x02);
    if (fz) wr8(ABS(RIDER_INPUT), 0);
    X = rd8(ABS(RIDER_COLUMN)); nz8((u8)X);
    lda8(rd8(ABS(0x09C7)));
    ora8(rd8(ABS(RIDER_AIR)));
    if (!fz) {
        lda8(rd8(ABS(RIDER_COLUMN)));
        cmp8((u8)A, rd8(ABS(0x09C8)));
        if (!fz) {
            wr8(ABS(0x09C8), (u8)A);
            lda8(rd8(ABSX(0x0A93)));
            fc = 0; adc8(rd8(ABS(0x09C5)));
            wr8(ABS(0x09C5), (u8)A);
        }
    }
    lda8(rd8(ABS(RIDER_BUMPED)));
    if (fz) { lda8(rd8(ABSX(0x0A13))); wr8(ABS(0x09C4), (u8)A); }
    lda8(rd8(ABSX(0x09D3)));
    wr8(ABS(RIDER_TILE_Y), (u8)A);
    cmp8((u8)A, 0x28);
    if (fc) goto flags;
    cmp8((u8)A, 0x00);
    if (!fz) {
        lda8(rd8(ABS(0x099C)));
        if (fz) {
            lda8(0x01);
            wr8(ABS(0x099C), 0x01);
            wr8(DP(0x84), 0x01);
            lda8(0x2B);
            push8(0x80); push16(0xEC61); QueueSoundB_m1x1();
        }
        ret();
        return;
    }
    lda8(rd8(ABS(RIDER_Y)));
    cmp8((u8)A, 0xB0);
    if (fc) {
        rep(0x20);
        lda16(rd16(ABS(0x00A2)));
        cmp16(A, 0x0100);
        sep(0x20);
        if (fz) {
            lda8(0x02); wr8(DP(JUMP_STATE), 0x02);
            lda8(0x80); wr8(ABS(RIDER_Y), 0x80);
            wr8(ABS(RIDER_AIR), 0); wr8(ABS(0x0226), 0); wr8(ABS(RIDER_HEIGHT), 0);
            ret();
            return;
        }
    }
    lda8(0x01); wr8(DP(JUMP_STATE), 0x01);
    rep(0x20);
    lda16(rd16(DP(SCROLL_X)));
    wr16(DP(0xC8), A);
    sep(0x20);
    ret();
    return;
flags:
    lda8(rd8(ABSX(0x0A53)));
    if (!fz) {
        lda8(rd8(ABS(0x09C7)));
        ora8(rd8(ABS(RIDER_AIR)));
        if (!fz) { lda8(0x45); wr8(ABS(RIDER_TILE_Y), 0x45); ret(); return; }
        lda8(rd8(ABSX(0x09D3)));
        and8(0x40);
        if (!fz) { ret(); return; }
        wr8(ABSX(0x0A53), 0);
        lda8(0x3B);
        wr8(ABSX(0x0B53), 0x3B); wr8(ABSX(0x0B93), 0x3B); wr8(ABSX(0x0BD3), 0x3B);
        wr8(ABSX(0x0C13), 0x3B); wr8(ABSX(0x0C53), 0x3B); wr8(ABSX(0x0C93), 0x3B);
        wr8(ABS(RIDER_TILE), 0x3B);
    }
    lda8(rd8(ABSX(0x09D3)));
    cmp8((u8)A, 0x84);
    if (fz) {
        lda8(rd8(ABS(0x09C7)));
        if (fz) { lda8(0xFC); wr8(ABS(RIDER_TILE_Y), 0xFC); }
    }
    ret();
}

/* The IRQ handler, Stadium 4's nine bytes higher ($80:B898). */
void ParallaxLines_m1x1(void);
void ScaleByScroll_m0x1(void);

static void rti4(void) { setP(pull8()); pull16(); pull8(); }

/* $80:FFA9 -> $80:B898 Irq (RTI): on the H/V timer IRQ, the parallax
 * lines when $96 is set; with $95 set, the scroll $9C as multiplier and
 * the HDMA line table at $1B00 from fixed factors, $1B20 / $1B22 =
 * factor $A0 and $9C. */
void IrqVector_m1x1(void)
{
    static const struct { u16 f; u8 dp; u16 ret; } k[8] = {
        { 0x0120, 0x0E, 0xB8C5 }, { 0x00E0, 0x0C, 0xB8CD }, { 0x00D0, 0x0A, 0xB8D5 },
        { 0x00C0, 0x08, 0xB8DD }, { 0x00B0, 0x06, 0xB8E5 }, { 0x00A0, 0x04, 0xB8ED },
        { 0x00A0, 0x02, 0xB8F5 }, { 0x0040, 0x00, 0xB8FD },
    };
    rep(0x30);
    push16(A); push16(X); push16(Y); push16(D);
    lda16(0x1B00); D = A; nz16(D);
    push8(DB); push8(0x80); DB = pull8(); nz8(DB);
    sep(0x30);
    lda8(rd8(ABS(TIMEUP)));
    if (fn) {
        lda8(rd8(ABS(0x0096)));
        if (!fz) { push16(0xB8B3); ParallaxLines_m1x1(); }
        lda8(rd8(ABS(0x0095)));
        if (!fz) {
            rep(0x20);
            lda16(rd16(ABS(SCROLL_X)));
            wr16(DP(0x60), A);
            for (int i = 0; i < 8; i++) {
                lda16(k[i].f);
                push16(k[i].ret); ScaleByScroll_m0x1();
                wr16(DP(k[i].dp), A);
            }
            wr16(DP(0x10), 0);
            lda16(0x00A0);
            push16(0xB907); ScaleByScroll_m0x1();
            wr16(DP(0x20), A);
            lda16(rd16(ABS(SCROLL_X)));
            wr16(DP(0x22), A);
            wr16(DP(0x24), 0);
            sep(0x20);
        }
    }
    rep(0x30);
    DB = pull8(); nz8(DB);
    D = pull16(); nz16(D);
    Y = pull16(); nz16(Y);
    X = pull16(); nz16(X);
    A = pull16(); nz16(A);
    rti4();
}
/* Interrupt return: P, then PC and PB from the interrupt frame. */
