/*
 * Stadium 3-4 (Gen B): mode changes and the end of a race, hand-written.
 */
#include "hand/hand.h"
#include "roster.h"

void ChangeMode_m1x1(void);

/* $80:8F17 ChangeMode (RTS): start the mode in $76 from its first step:
 * APUIO3 = 0, $75 = $77 = 0, fade out ($1923 = -1). */
void ChangeMode_m1x1(void)
{
    wr8(ABS(0x2143), 0);
    lda8(0x00);
    wr8(DP(0x75), 0x00);
    wr8(DP(0x77), 0x00);
    lda8(0xFF);
    wr8(ABS(0x1923), 0xFF);
    ret();
}

/* $80:8F15 NextMode (RTS): $76 + 1, then ChangeMode. */
void NextMode_m1x1(void)
{
    wr8(DP(0x76), inc8(rd8(DP(0x76))));
    ChangeMode_m1x1();
}

/* $80:AB59 RaceFinishStep (RTS): after the finish (or a timeout), count
 * $52 down from $10; at 1 the final goes on to the jump contest (mode $13)
 * and a free run to its results (next mode, $86/$87 = 0); a practice run
 * ($0CD7) returns to the title. Before that, clear the race flags
 * ($092D, $0940, $60). In the port a free run raced against the final's
 * rivals still counts as a free run (port_race_is_final). */
void RaceFinishStep_m1x1(void)
{
    lda8(rd8(DP(0x52)));
    if (fz) { lda8(0x10); wr8(DP(0x52), 0x10); }
    cmp8((u8)A, 0x01);
    if (!fz) {
        lda8(0x00);
        wr8(ABS(0x092D), 0x00);
        wr8(ABS(0x0940), 0x00);
        wr8(DP(0x60), 0x00);
        X = rd8(DP(0x84)); nz8((u8)X);
        X = dec8((u8)X);
        if (!fz) { ret(); return; }
        ldx8(0x12);
        lda8(rd8(DP(0x52)));
        lda8(lsr8((u8)A));
        if (!fc) {
            ldy8(0x06);
            lda8(0xFE);
            do {
                X = inc8((u8)X);
                Y = dec8((u8)Y);
            } while (!fn);
        }
        ret();
        return;
    }
    lda8(rd8(DP(0x7D)));
    if (port_race_is_final()) {
        lda8(0x13);
        wr8(DP(0x76), 0x13);
        push16(0xAB6F); ChangeMode_m1x1();
    } else {
        push16(0xAB74); NextMode_m1x1();
        wr8(DP(0x86), 0);
        wr8(DP(0x87), 0);
    }
    lda8(rd8(ABS(0x0CD7)));
    if (fz) { ret(); return; }
    ldx8(0x2B);
    do {
        wr8(DPX(0x38), 0);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(0x76), 0);
    wr8(DP(0x79), inc8(rd8(DP(0x79))));
    ChangeMode_m1x1();
}

/* The port's rival pace on rider X's target speed $0236/$022E,X (the
 * registers and flags are left as they are). */
static void pace_target(void)
{
    int pct = port_rival_pace();
    if (pct == 100)
        return;
    u16 a = (u16)(X & 0xFF);
    unsigned v = (unsigned)(rd8(ABS(0x0236 + a)) << 8 | rd8(ABS(0x022E + a)));
    v = v * (unsigned)pct / 100u;
    if (v > 0xFFFF) v = 0xFFFF;
    wr8(ABS(0x0236 + a), (u8)(v >> 8));
    wr8(ABS(0x022E + a), (u8)v);
}

/* $80:BE66 RivalTarget (RTS, Y = the rider's lane state $019E): CPU rider
 * X's target speed $0236/$022E,X. After the race (\$84) high byte 3; riders
 * flagged in $80:BEB9 (1-3) ride at the player's speed while the player is
 * on the ground and moving, keeping near it (+1 up to 4 when behind in the
 * same lane); otherwise the high byte from $80:BAEF by lane state and a
 * random low byte from $80:BAE7 ($2F,X & $0F). */
void RivalTarget_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    if (!fz) {
        lda8(0x03);
        goto table_lo;
    }
    lda8(rd8(ABSX(0xBEB9)));
    if (!fz) {
        lda8(rd8(ABSX(0x019E)));
        lda8(lsr8((u8)A));
        lda8(lsr8((u8)A));
        if (!fc) {
            lda8(rd8(ABS(0x01C6)));
            if (!fz) {
                wr8(ABSX(0x0236), (u8)A);
                lda8(rd8(ABS(0x01BE)));
                wr8(ABSX(0x022E), (u8)A);
                lda8(rd8(ABSX(0x019E)));
                int bump = fz;
                if (!bump) {
                    lda8(rd8(ABSX(0x0196)));
                    cmp8((u8)A, 0x40);
                    if (fc) goto table;
                    lda8(rd8(ABS(0x0958)));
                    cmp8((u8)A, rd8(ABS(0x095B)));
                    bump = !fz;
                }
                if (bump) {
                    lda8(rd8(ABSX(0x0236)));
                    cmp8((u8)A, 0x04);
                    if (!fc) wr8(ABSX(0x0236), inc8(rd8(ABSX(0x0236))));
                }
                pace_target();
                ret();
                return;
            }
        }
    }
table:
    lda8(rd8(ABSY(0xBAEF)));
table_lo:
    wr8(ABSX(0x0236), (u8)A);
    lda8(rd8(DPX(0x2F)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBAE7)));
    wr8(ABSX(0x022E), (u8)A);
    pace_target();
    ret();
}

/* $80:889B Mode02_SegmentCard (RTS): the course card waits for its minute
 * ($80:88AF by course $7C * 2 + final $7D), then the next mode. */
void Mode02_SegmentCard_m1x1(void)
{
    lda8(rd8(DP(0x7C)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(0x7D)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, rd8(ABSX(0x88AF)));
    if (!fc) { ret(); return; }
    NextMode_m1x1();
}

void BsxEndIfFlagged_m1x1(void);

/* $80:9DBD Mode11 (RTS): wait $88 frames, then the end screen (mode $0F,
 * fade, a $20-frame timer). */
void Mode11_m1x1(void)
{
    rep(0x20);
    u16 t = dec16(rd16(DP(0x88)));
    wr16(DP(0x88), t);
    sep(0x20);
    if (!fz) { ret(); return; }
    lda8(0x0F); wr8(DP(0x76), 0x0F);
    wr8(DP(0x75), 0);
    wr8(DP(0x77), 0);
    wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    rep(0x20);
    lda16(0x0020); wr16(DP(0x88), 0x0020);
    sep(0x20);
    ret();
}

/* $80:98B2 Mode0F_EndScreen (RTS): the broadcast-over check, then the
 * end screen's step $0450 through the table $80:98C8 (JMP ($0000)). */
void Mode0F_EndScreen_m1x1(void)
{
    push16(0x98B4); BsxEndIfFlagged_m1x1();
    lda8(rd8(ABS(0x0450)));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABSX(0x98C8))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSX(0x98C9))); wr8(DP(0x01), (u8)A);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}

void VramQueueTiles20_m1x1(void);

/* $80:F395: leave the demo through the table $80:F213/$80:F218 by Y
 * (JMP ($0000)). */
static void demo_exit(void)
{
    lda8(rd8(ABSY(0xF213))); wr8(DP(0x00), (u8)A);
    lda8(rd8(ABSY(0xF218))); wr8(DP(0x01), (u8)A);
    r816_call_pc(0x800000u | rd16(LONG(0x000000)));
}

/* $80:F330 DemoInputStep (RTS): the attract demo's recorded input: the
 * debug flag lets Start leave; on the first frame load the first entry
 * ($9C:D200 input, $9C:D300 duration, $63 = 0); each frame feed the input
 * to the player ($017E, $098E) for its duration $0CE3, then the next
 * entry; a duration of $F8 or more ends the demo through exit $FF - it.
 * C clear while it plays. */
void DemoInputStep_m1x1(void)
{
    lda8(rd8(LONG(0x81800E)));
    if (!fz) {
        lda8(rd8(DP(0x21)));
        and8(0x10);
        if (!fz) { ldy8(0x00); demo_exit(); return; }
    }
    lda8(rd8(DP(0x92)));
    if (fz) {
        wr8(DP(0x63), 0);
        lda8(rd8(LONG(0x9CD200))); wr8(ABS(0x197C), (u8)A);
        lda8(rd8(LONG(0x9CD300))); wr8(ABS(0x0CE3), (u8)A);
        lda8(0x01); wr8(ABS(0x0CE1), 0x01);
    } else {
        lda8(rd8(ABS(0x0CE3)));
        if (!fz) {
            wr8(ABS(0x0CE3), dec8(rd8(ABS(0x0CE3))));
            X = rd8(ABS(0x0CE1)); nz8((u8)X);
            X = dec8((u8)X);
            lda8(rd8(LONGX(0x9CD200)));
            wr8(ABS(0x017E), (u8)A);
            wr8(ABS(0x098E), (u8)A);
        } else {
            X = rd8(ABS(0x0CE1)); nz8((u8)X);
            wr8(ABS(0x0CE1), inc8(rd8(ABS(0x0CE1))));
            lda8(rd8(LONGX(0x9CD300)));
            cmp8((u8)A, 0xF8);
            if (fc) {
                eor8(0xFF);
                Y = (u8)A; nz8((u8)Y);
                demo_exit();
                return;
            }
            wr8(ABS(0x0CE3), (u8)A);
            lda8(rd8(LONGX(0x9CD200)));
            wr8(ABS(0x017E), (u8)A);
            wr8(ABS(0x098E), (u8)A);
        }
    }
    fc = 0;
    ret();
}

/* $80:F4C8 TitlePushStart (RTS): from minute 6 the PUSH START panel
 * blinks (one of three frames by $C5, tiles from $80:F50D at VRAM $18F1);
 * Start (sound $2D) leaves through exit 0. */
void TitlePushStart_m1x1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, 0x06);
    if (!fc) { ret(); return; }
    wr8(DP(0xC5), inc8(rd8(DP(0xC5))));
    lda8(rd8(DP(0xC5)));
    and8(0x20);
    lda8(lsr8((u8)A));
    wr8(DP(0x00), (u8)A);
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    fc = 0; adc8(rd8(DP(0x00)));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    wr8(DP(0x00), lsr8(rd8(DP(0x00))));
    fc = 0; adc8(rd8(DP(0x00)));
    rep(0x20);
    and16(0x00FF);
    fc = 0; adc16(0xF50D);
    wr16(DP(0x02), A);
    lda16(0x18F1); wr16(DP(0x00), 0x18F1);
    sep(0x20);
    push16(0xF4FB); VramQueueTiles20_m1x1();
    lda8(rd8(DP(0x21)));
    and8(0x10);
    if (fz) { ret(); return; }
    lda8(0x2D); wr8(ABS(0x2140), 0x2D);
    ldy8(0x00);
    demo_exit();
}
