/*
 * Stadium 3-4 (Gen B): mode changes and the end of a race, hand-written.
 */
#include "hand/hand.h"
#include "ram.h"
#include "roster.h"

void ChangeMode_m1x1(void);

/* $80:8F17 ChangeMode (RTS): start the mode in $76 from its first step:
 * APUIO3 = 0, $75 = $77 = 0, fade out ($1923 = -1). */
void ChangeMode_m1x1(void)
{
    wr8(ABS(APUIO3), 0);
    lda8(0x00);
    wr8(DP(MODE_ACTIVE), 0x00);
    wr8(DP(MODE_STEP), 0x00);
    lda8(0xFF);
    wr8(ABS(FADE_STATE), 0xFF);
    ret();
}

/* $80:8F15 NextMode (RTS): $76 + 1, then ChangeMode. */
void NextMode_m1x1(void)
{
    wr8(DP(GAME_MODE), inc8(rd8(DP(GAME_MODE))));
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
        wr8(ABS(GAUGE_ON), 0x00);
        wr8(ABS(OVERHEAT_LOCK), 0x00);
        wr8(DP(OVERHEATED), 0x00);
        X = rd8(DP(RACE_END)); nz8((u8)X);
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
    lda8(rd8(DP(FINAL_RACE)));
    if (port_race_is_final()) {
        lda8(0x13);
        wr8(DP(GAME_MODE), 0x13);
        push16(0xAB6F); ChangeMode_m1x1();
    } else {
        push16(0xAB74); NextMode_m1x1();
        wr8(DP(UPLOAD_SRC), 0);
        wr8(DP(0x87), 0);
    }
    lda8(rd8(ABS(ATTRACT_DEMO)));
    if (fz) { ret(); return; }
    ldx8(0x2B);
    do {
        wr8(DPX(0x38), 0);
        X = dec8((u8)X);
    } while (!fn);
    wr8(DP(GAME_MODE), 0);
    wr8(DP(TURN_LATCH), inc8(rd8(DP(TURN_LATCH))));
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
    unsigned v = (unsigned)(rd8(ABS(RIDER_TARGET_HI + a)) << 8 | rd8(ABS(RIDER_TARGET_LO + a)));
    v = v * (unsigned)pct / 100u;
    if (v > 0xFFFF) v = 0xFFFF;
    wr8(ABS(RIDER_TARGET_HI + a), (u8)(v >> 8));
    wr8(ABS(RIDER_TARGET_LO + a), (u8)v);
}

/* $80:BE66 RivalTarget (RTS, Y = the rider's lane state $019E): CPU rider
 * X's target speed $0236/$022E,X. After the race (\$84) high byte 3; riders
 * flagged in $80:BEB9 (1-3) ride at the player's speed while the player is
 * on the ground and moving, keeping near it (+1 up to 4 when behind in the
 * same lane); otherwise the high byte from $80:BAEF by lane state and a
 * random low byte from $80:BAE7 ($2F,X & $0F). */
void RivalTarget_m1x1(void)
{
    lda8(rd8(DP(RACE_END)));
    if (!fz) {
        lda8(0x03);
        goto table_lo;
    }
    lda8(rd8(ABSX(0xBEB9)));
    if (!fz) {
        lda8(rd8(ABSX(RIDER_PAGE)));
        lda8(lsr8((u8)A));
        lda8(lsr8((u8)A));
        if (!fc) {
            lda8(rd8(ABS(RIDER_SPEED_HI)));
            if (!fz) {
                wr8(ABSX(RIDER_TARGET_HI), (u8)A);
                lda8(rd8(ABS(RIDER_SPEED_LO)));
                wr8(ABSX(RIDER_TARGET_LO), (u8)A);
                lda8(rd8(ABSX(RIDER_PAGE)));
                int bump = fz;
                if (!bump) {
                    lda8(rd8(ABSX(RIDER_X)));
                    cmp8((u8)A, 0x40);
                    if (fc) goto table;
                    lda8(rd8(ABS(RIDER_BAND)));
                    cmp8((u8)A, rd8(ABS(RIDER_BAND + 0x03)));
                    bump = !fz;
                }
                if (bump) {
                    lda8(rd8(ABSX(RIDER_TARGET_HI)));
                    cmp8((u8)A, 0x04);
                    if (!fc) wr8(ABSX(RIDER_TARGET_HI), inc8(rd8(ABSX(RIDER_TARGET_HI))));
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
    wr8(ABSX(RIDER_TARGET_HI), (u8)A);
    lda8(rd8(DPX(0x2F)));
    and8(0x0F);
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABSY(0xBAE7)));
    wr8(ABSX(RIDER_TARGET_LO), (u8)A);
    pace_target();
    ret();
}

/* $80:889B Mode02_SegmentCard (RTS): the course card waits for its minute
 * ($80:88AF by course $7C * 2 + final $7D), then the next mode. */
void Mode02_SegmentCard_m1x1(void)
{
    lda8(rd8(DP(COURSE_SLOT)));
    lda8(asl8((u8)A));
    fc = 0; adc8(rd8(DP(FINAL_RACE)));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(LONG(BCAST_MINUTE)));
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
    u16 t = dec16(rd16(DP(MODE_TIMER)));
    wr16(DP(MODE_TIMER), t);
    sep(0x20);
    if (!fz) { ret(); return; }
    lda8(0x0F); wr8(DP(GAME_MODE), 0x0F);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    rep(0x20);
    lda16(0x0020); wr16(DP(MODE_TIMER), 0x0020);
    sep(0x20);
    ret();
}

/* $80:98B2 Mode0F_EndScreen (RTS): the broadcast-over check, then the
 * end screen's step $0450 through the table $80:98C8 (JMP ($0000)). */
void Mode0F_EndScreen_m1x1(void)
{
    push16(0x98B4); BsxEndIfFlagged_m1x1();
    lda8(rd8(ABS(END_STEP)));
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
    lda8(rd8(LONG(DEBUG_ENABLED)));
    if (!fz) {
        lda8(rd8(DP(P1_PRESS_HI)));
        and8(0x10);
        if (!fz) { ldy8(0x00); demo_exit(); return; }
    }
    lda8(rd8(DP(RACE_STARTED)));
    if (fz) {
        wr8(DP(FRAME_COUNT), 0);
        lda8(rd8(LONG(0x9CD200))); wr8(ABS(0x197C), (u8)A);
        lda8(rd8(LONG(0x9CD300))); wr8(ABS(DEMO_TIME), (u8)A);
        lda8(0x01); wr8(ABS(DEMO_INDEX), 0x01);
    } else {
        lda8(rd8(ABS(DEMO_TIME)));
        if (!fz) {
            wr8(ABS(DEMO_TIME), dec8(rd8(ABS(DEMO_TIME))));
            X = rd8(ABS(DEMO_INDEX)); nz8((u8)X);
            X = dec8((u8)X);
            lda8(rd8(LONGX(0x9CD200)));
            wr8(ABS(RIDER_INPUT), (u8)A);
            wr8(ABS(DEMO_PAD), (u8)A);
        } else {
            X = rd8(ABS(DEMO_INDEX)); nz8((u8)X);
            wr8(ABS(DEMO_INDEX), inc8(rd8(ABS(DEMO_INDEX))));
            lda8(rd8(LONGX(0x9CD300)));
            cmp8((u8)A, 0xF8);
            if (fc) {
                eor8(0xFF);
                Y = (u8)A; nz8((u8)Y);
                demo_exit();
                return;
            }
            wr8(ABS(DEMO_TIME), (u8)A);
            lda8(rd8(LONGX(0x9CD200)));
            wr8(ABS(RIDER_INPUT), (u8)A);
            wr8(ABS(DEMO_PAD), (u8)A);
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
    lda8(rd8(LONG(BCAST_MINUTE)));
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
    lda8(rd8(DP(P1_PRESS_HI)));
    and8(0x10);
    if (fz) { ret(); return; }
    lda8(0x2D); wr8(ABS(APUIO0), 0x2D);
    ldy8(0x00);
    demo_exit();
}

void SpawnDustPuff_m1x1(void);
void QueueSoundB_m1x1(void);

/* $80:ABAC: a dust puff every 16 frames; while overheated ($60 odd)
 * $00 = $58, X = 7, Y = $1C. */
static void overheat_dust(void)
{
    lda8(rd8(DP(FRAME_COUNT)));
    and8(0x0F);
    if (fz) { push8(0x80); push16(0xABB5); SpawnDustPuff_m1x1(); }
    lda8(rd8(DP(OVERHEATED)));
    lda8(lsr8((u8)A));
    if (fc) {
        lda8(0x58); wr8(DP(0x00), 0x58);
        ldx8(0x07);
        lda8((u8)X);
        lda8(asl8((u8)A)); lda8(asl8((u8)A));
        Y = (u8)A; nz8((u8)Y);
    }
}

/* $80:C19B OverheatStep (RTS): while the engine is overheated ($60 or
 * $0940): above step 8 the stopped player cools down standing (speed 0, or
 * $C0 while landing; steering toward lane $80:BACF) with dust puffs; at
 * step 8 the overheat ends (sound $2D, heat $0990 = $08CC = 5, finish
 * state 5 unless set). */
void OverheatStep_m1x1(void)
{
    lda8(rd8(DP(OVERHEATED)));
    ora8(rd8(ABS(OVERHEAT_LOCK)));
    if (fz) { ret(); return; }
    lda8(rd8(DP(OVERHEATED)));
    cmp8((u8)A, 0x08);
    if (fz) {
        lda8(0x00); wr8(ABS(OVERHEAT_LOCK), 0x00);
        lda8(0x2D);
        push8(0x80); push16(0xC1DF); QueueSoundB_m1x1();
        lda8(0x05);
        wr8(ABS(ENGINE_HEAT), 0x05);
        wr8(ABS(RIDER_COUNTDOWN), 0x05);
        Y = rd8(ABS(RIDER_TIMER)); nz8((u8)Y);
        if (fz) wr8(ABS(RIDER_TIMER), (u8)A);
        ret();
        return;
    }
    if (!fc) { ret(); return; }
    lda8(rd8(ABS(RIDER_SPEED_HI)));
    ora8(rd8(ABS(RIDER_BUMPED)));
    ora8(rd8(ABS(RIDER_TIMER)));
    if (fz) {
        lda8(rd8(ABS(RIDER_SCRIPT)));
        if (!fz) {
            lda8(0xC0); wr8(ABS(RIDER_SPEED_LO), 0xC0);
        } else {
            wr8(ABS(RIDER_SPEED_LO), (u8)A);
            ldx8(0x01);
            lda8(rd8(ABS(RIDER_LANE)));
            cmp8((u8)A, rd8(ABS(0xBACF)));
            if (fz) X = dec8((u8)X);
            wr8(ABS(RIDER_STEER), (u8)X);
        }
    }
    overheat_dust();
    ret();
}

/* $80:FE3A ItemFlashState (RTS): the item-hit flash $5E: cleared when the
 * race is over or overheated; while it runs returns X = 6 (and A = $40 on
 * its odd steps from $0D) or X = 0 on even steps from $0D, Y = 7. */
void ItemFlashState_m1x1(void)
{
    lda8(rd8(DP(RACE_END)));
    ora8(rd8(DP(OVERHEATED)));
    if (!fz) { lda8(0x00); wr8(DP(FLASH_TIMER), 0x00); ret(); return; }
    lda8(rd8(DP(FLASH_TIMER)));
    if (fz) { ret(); return; }
    ldx8(0x00);
    ldy8(0x07);
    lda8(rd8(DP(FLASH_TIMER)));
    cmp8((u8)A, 0x0D);
    if (fc) {
        lda8(lsr8((u8)A));
        if (!fc) { ret(); return; }
        lda8(0x40);
    }
    ldx8(0x06);
    ret();
}

/* $80:F375: the next recorded demo entry (as DemoInputStep's): a duration
 * of $F8 or more ends the demo through exit $FF - it; else the duration
 * and input are taken, C clear. */
static void demo_next_entry4(void)
{
    X = rd8(ABS(DEMO_INDEX)); nz8((u8)X);
    wr8(ABS(DEMO_INDEX), inc8(rd8(ABS(DEMO_INDEX))));
    lda8(rd8(LONGX(0x9CD300)));
    cmp8((u8)A, 0xF8);
    if (fc) {
        eor8(0xFF);
        Y = (u8)A; nz8((u8)Y);
        demo_exit();
        return;
    }
    wr8(ABS(DEMO_TIME), (u8)A);
    lda8(rd8(LONGX(0x9CD200)));
    wr8(ABS(RIDER_INPUT), (u8)A);
    wr8(ABS(DEMO_PAD), (u8)A);
    fc = 0;
    ret();
}

/* Demo exits that continue with the next entry: $80:F3A2 ($099D = $0990
 * = $10), $80:F3BA and $80:F3BE (A = $35 + $80:D200,X, unused). */
void DemoExitBoost_m1x1(void)
{
    lda8(0x10);
    wr8(ABS(0x099D), 0x10);
    wr8(ABS(ENGINE_HEAT), 0x10);
    demo_next_entry4();
}
void DemoExitSkip40_m1x1(void)
{
    lda8(0x40);
    demo_next_entry4();
}
void DemoExitSkip35_m1x1(void)
{
    lda8(0x35);
    fc = 0; adc8(rd8(ABSX(0xD200)));
    demo_next_entry4();
}
