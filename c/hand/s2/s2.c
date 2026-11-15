/*
 * Stadium 2 (the 1997 S2 dump): routines that differ from Stadium 1's and
 * so are written here rather than carried over by tools/port_hand.py
 * (from_s1.c). Checked by tools/difftest816.py s2.
 */
#include "hand/hand.h"
#include "ram.h"

void VramQueueBlock_m1x1(void);
void AddScoreLong_m1x1(void);
void DrawAllRiders_m1x1(void);
void FindObjectSlot_m1x1(void);
void QueueScoreDigitsLong_m1x1(void);
void QueueSound_m1x1(void);
void SetHdmaPreset_m1x1(void);
void VramQueueTilesLong_m1x1(void);

/* $80:A5A7 (RTS): the character name block ($80:A5F4 + 29 * $1542, five
 * bytes lower than in Stadium 1) to VRAM $1C2D. The entry is five
 * instructions that branch into DrawRacerName's tail, which is where the
 * table address differs. */
void DrawRacerNameLow_m1x1(void)
{
    rep(0x20);
    lda16(0x1C2D); wr16(DP(0x00), 0x1C2D);
    sep(0x20);
    lda8(rd8(ABS(0x1542)));
    wr8(ABS(WRMPYA), (u8)A);
    lda8(0x1D);
    wr8(ABS(WRMPYB), 0x1D);
    rep(0x20);
    A = rd16(ABS(RDMPYL));
    fc = 0; adc16(0xA5F4);
    wr16(DP(0x02), A);
    sep(0x20);
    VramQueueBlock_m1x1();
}

/* $80:D80C (RTS): set up Stadium 2's jump contest: the lane rows ($09D3
 * ramps $FC, three empty rows, six scenery rows $3B); the streamer start;
 * the player at column 9, X $30, angle 6 ($90 = 2, $1905 = $17, HDMA
 * preset 1), drawn once at ground $B0 then $B8; HDMA channels 7/6
 * ($80:D8E6 -> $210F, $80:D8ED -> $2108: seven bytes lower than Stadium 1); $88 = $012C, $1B00/$1B02
 * cleared, $1B20 = 9, $1B21 = 8 and $190F = $DC. */
void SetupJumpContest_m1x1(void)
{
    ldx8(0x3F);
    do {
        lda8(0xFC); wr8(ABSX(0x09D3), 0xFC);
        lda8(0x00);
        wr8(ABSX(0x0A13), 0); wr8(ABSX(0x0A53), 0); wr8(ABSX(0x0A93), 0);
        lda8(0x3B);
        wr8(ABSX(0x0B53), 0x3B); wr8(ABSX(0x0B93), 0x3B); wr8(ABSX(0x0BD3), 0x3B);
        wr8(ABSX(0x0C13), 0x3B); wr8(ABSX(0x0C53), 0x3B); wr8(ABSX(0x0C93), 0x3B);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x34); wr8(ABS(TRACK_ROW_PTR), 0x34);
    lda8(0x0A); wr8(ABS(0x0273), 0x0A);
    lda8(0x05); wr8(ABS(TRACK_VRAM_HALF), 0x05);
    lda8(0x74); wr8(ABS(TRACK_VRAM_COL), 0x74);
    lda8(0x08); wr8(ABS(RIDER_WHEEL), 0x08);
    lda8(0x09); wr8(ABS(RIDER_COLUMN), 0x09);
    lda8(0x30); wr8(ABS(RIDER_X), 0x30);
    lda8(0x06); wr8(ABS(RIDER_LEAN), 0x06);
    lda8(0x01);
    wr8(ABS(RIDER_PAGE), 0x01); wr8(ABS(RIDER_ACTIVE), 0x01); wr8(ABS(RIDER_BAND), 0x01);
    lda8(asl8((u8)A));
    wr8(DP(0x90), (u8)A);
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    lda8(0x01);
    push8(0x80); push16(0xD875); SetHdmaPreset_m1x1();
    ldx8(0x00);
    lda8(0xB0); wr8(ABS(0x0CD5), 0xB0); wr8(ABS(RIDER_Y), 0xB0);
    push16(0xD882); DrawAllRiders_m1x1();
    lda8(0xB8); wr8(ABS(0x0CD5), 0xB8); wr8(ABS(RIDER_Y), 0xB8);
    static const u8 ch[12][2] = {
        {0x70,0x42},{0x71,0x0F},{0x72,0xE6},{0x73,0xD8},{0x74,0x80},{0x77,0x00},
        {0x60,0x40},{0x61,0x08},{0x62,0xED},{0x63,0xD8},{0x64,0x80},{0x67,0x00} };
    for (int i = 0; i < 12; i++) { lda8(ch[i][1]); wr8(ABS(0x4300 | ch[i][0]), ch[i][1]); }
    rep(0x20);
    lda16(0x012C); wr16(DP(MODE_TIMER), 0x012C);
    wr16(ABS(0x1B00), 0); wr16(ABS(0x1B02), 0);
    sep(0x20);
    lda8(0x09); wr8(ABS(0x1B20), 0x09);
    lda8(0x08); wr8(ABS(0x1B21), 0x08);
    lda8(0xDC); wr8(ABS(SH_HDMAEN), 0xDC);
    ret();
}

/* $81:9852 (RTS): Stadium 2's (as Stadium 1's, but the count is queued
 * with attribute $08 instead of $28) results bonus count-down object. Every 4
 * frames ($88) the bonus $B2 counts down by one into the score (sound
 * $44, the count at VRAM $1CB6, $09AF = 1, the score digits). At 0 it
 * hands the podium on: the player place (the slot where $086E is 1)
 * plus 8 * $154F picks the type-7 object state 3 (timer $10) and the
 * type from $81:9936 goes to state 4 with long timers; its own state
 * steps. */
void ResultsBonusCount_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) goto tick;
    lda8(rd8(DP(0xB2)));
    if (fz) {
        ldy8(0x0E);
        do {
            lda8(rd8(ABSY(PLAYER_PLACE)));
            lda8(dec8((u8)A));
            if (fz) break;
            Y = dec8((u8)Y);
            Y = dec8((u8)Y);
        } while (!fn);
        lda8((u8)Y);
        lda8(lsr8((u8)A));
        wr8(DP(0x00), (u8)A);
        lda8(rd8(ABS(0x1542)));
        lda8(asl8((u8)A)); lda8(asl8((u8)A)); lda8(asl8((u8)A));
        fc = 0; adc8(rd8(DP(0x00)));
        wr8(DP(0x00), (u8)A);
        Y = (u8)A; nz8((u8)Y);
        lda8(0x03); wr8(DP(0x01), 0x03);
        lda8(0x07);
        push8(0x81); push16(0x9882); FindObjectSlot_m1x1();
        lda8(rd8(DP(0x01))); wr8(ABSY(OBJ_STATE), (u8)A);
        lda8(0x10); wr8(ABSY(OBJ_DELAY), 0x10);
        X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
        wr8(ABSX(OBJ_STATE), inc8(rd8(ABSX(OBJ_STATE))));
        Y = rd8(DP(0x00)); nz8((u8)Y);
        lda8(rd8(ABSY(0x9936)));
        push8(0x81); push16(0x989B); FindObjectSlot_m1x1();
        lda8(0x04); wr8(ABSY(OBJ_STATE), 0x04);
        lda8(0xFF); wr8(ABSY(OBJ_DELAY), 0xFF);
        lda8(0xFF); wr8(ABSY(OBJ_COUNTER), 0xFF);
        goto tick;
    }
    wr8(DP(0xB2), dec8(rd8(DP(0xB2))));
    lda8(0x44);
    push8(0x81); push16(0x98B5); QueueSound_m1x1();
    ldy8(0x00);
    lda8(rd8(DP(0xB2)));
    for (;;) {
        cmp8((u8)A, 0x0A);
        if (!fc) break;
        sbc8(0x0A);
        Y = inc8((u8)Y);
    }
    wr8(DP(0x00), (u8)A);
    ora8(0x50); wr8(ABS(0x03D5), (u8)A);
    eor8(0x30); wr8(ABS(0x03DA), (u8)A);
    cmp8((u8)Y, 0x00);
    if (!fz) {
        lda8((u8)Y);
        ora8(0x50); wr8(ABS(0x03D4), (u8)A);
        eor8(0x30); wr8(ABS(0x03D9), (u8)A);
    } else {
        lda8(0xFC); wr8(ABS(0x03D4), 0xFC); wr8(ABS(0x03D9), 0xFC);
    }
    lda8((u8)Y);
    ora8(rd8(DP(0x00)));
    if (fz) {
        lda8(0xFC);
        wr8(ABS(0x03D5), 0xFC); wr8(ABS(0x03D6), 0xFC);
        wr8(ABS(0x03DA), 0xFC); wr8(ABS(0x03DB), 0xFC);
    }
    lda8(0xFC); wr8(ABS(DIGIT_ROW), 0xFC); wr8(ABS(0x03D8), 0xFC);
    lda8(0x04); wr8(DP(MODE_TIMER), 0x04);
    rep(0x20);
    lda16(0x1CB6); wr16(DP(0x00), 0x1CB6);
    lda16(0x03D2); wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x08); wr8(DP(0x0F), 0x08);
    push8(0x81); push16(0x991C); VramQueueTilesLong_m1x1();
    lda8(0x01); wr8(ABS(0x09AF), 0x01);
    push8(0x81); push16(0x9925); AddScoreLong_m1x1();
    wr8(DP(0x00), dec8(rd8(DP(0x00))));
    lda8(0x28); wr8(DP(0x0F), 0x28);
    push8(0x81); push16(0x992F); QueueScoreDigitsLong_m1x1();
tick:
    wr8(DP(MODE_TIMER), dec8(rd8(DP(MODE_TIMER))));
    X = rd8(ABS(OBJ_CURRENT)); nz8((u8)X);
    ret();
}

/* $80:CF08 (RTS): the engine temperature: while racing (no turbo $0CE0,
 * not over $84) the heat $0990:$098F rises by $80:BDFA,Y toward the
 * level $80:BDFE,Y for the button held (Y from $0178 bits 6-7, A
 * forced before the run) and cools by $0B above it; at $20 it overheats
 * ($60 = $0940 = heat, sounds $2E / $3E); from $1A with A held the
 * warning ($A2, sound $2C) starts, below it stops (sound $2D). Then the
 * gauge: 13 sprites from X $58 + heat ($78 with turbo) at $17F0, tiles
 * $80:CFEB (+4 with turbo), rows below $70 hidden. */
void TemperatureGauge_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    ora8(rd8(ABS(TURBO_ON)));
    u8 top;
    if (!fz) goto off;
    lda8(rd8(ABS(0x0940)));
    if (!fz) {
        lda8(rd8(DP(OVERHEATED)));
        lda8(lsr8((u8)A));
        if (fc) goto off;
        top = 0x01;
        goto draw;
    }
    lda8(rd8(ABS(RIDER_INPUT)));
    and8(0xC0);
    if (!fz) {
        Y = rd8(DP(RACE_STARTED)); nz8((u8)Y);
        if (fz) lda8(0x80);
        lda8(asl8((u8)A)); lda8(rol8((u8)A)); lda8(rol8((u8)A));
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(ENGINE_HEAT)));
    cmp8((u8)A, rd8(ABSY(0xBDFE)));
    if (!fc) {
        lda8(rd8(ABS(0x098F)));
        fc = 0; adc8(rd8(ABSY(0xBDFA)));
        wr8(ABS(0x098F), (u8)A);
        if (fc) wr8(ABS(ENGINE_HEAT), inc8(rd8(ABS(ENGINE_HEAT))));
    } else if (!fz) {
        lda8(rd8(ABS(0x098F)));
        fc = 1; sbc8(0x0B);
        wr8(ABS(0x098F), (u8)A);
        if (!fc) {
            lda8(rd8(ABS(ENGINE_HEAT)));
            if (!fz) wr8(ABS(ENGINE_HEAT), dec8(rd8(ABS(ENGINE_HEAT))));
        }
    }
    ldx8(0x00);
    lda8(rd8(ABS(ENGINE_HEAT)));
    cmp8((u8)A, 0x20);
    if (fc) {
        wr8(DP(OVERHEATED), (u8)A);
        wr8(ABS(0x0940), (u8)A);
        push8((u8)A);
        lda8(0x2E); push8(0x80); push16(0xCF78); QueueSound_m1x1();
        lda8(0x3E); push8(0x80); push16(0xCF7E); QueueSound_m1x1();
        lda8(pull8());
    }
    cmp8((u8)A, 0x1A);
    int warn = 0;
    if (fc) { Y = dec8((u8)Y); warn = fz; }
    if (warn) {
        lda8(rd8(DP(0xA2)));
        if (fz) {
            lda8(rd8(ABS(TURBO_ON)));
            if (fz) { lda8(0x2C); push8(0x80); push16(0xCF95); QueueSound_m1x1(); }
            wr8(DP(0xA2), (u8)A);   /* Stadium 2 has no LDA #$01 here: $A2 takes A as it is */
        }
    } else {
        lda8(rd8(DP(0xA2)));
        if (!fz) {
            lda8(0x2D); push8(0x80); push16(0xCFA3); QueueSound_m1x1();
            wr8(DP(0xA2), 0);
        }
    }
off:
    top = 0xFF;
draw:
    lda8(top);
    wr8(DP(0x01), (u8)A);
    lda8(rd8(ABS(ENGINE_HEAT)));
    fc = 0; adc8(0x58);
    X = rd8(ABS(TURBO_ON)); nz8((u8)X);
    if (!fz) lda8(0x78);
    wr8(DP(0x00), (u8)A);
    ldx8(0x0C);
    lda8(rd8(ABS(TURBO_ON)));
    lda8(asl8((u8)A)); lda8(asl8((u8)A));
    Y = (u8)A; nz8((u8)Y);
    do {
        lda8(rd8(DP(0x00)));
        wr8(ABSX(0x17F0), (u8)A);
        fc = 0; adc8(0x08);
        wr8(DP(0x00), (u8)A);
        cmp8((u8)A, 0x70);
        lda8(fc ? 0xC7 : 0xF0);
        wr8(ABSX(0x17F1), (u8)A);
        lda8(rd8(ABSY(0xCFEB)));
        wr8(ABSX(0x17F2), (u8)A);
        lda8(0x21); wr8(ABSX(0x17F3), 0x21);
        Y = inc8((u8)Y);
        X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X); X = dec8((u8)X);
    } while (!fn);
    ret();
}

void VramQueueCopy_m1x1(void);
void QueueFiveDigits_m1x1(void);

/* The end screen's box rows ($89:D800 + 256 * hi + lo, two rows of $20) to
 * VRAM $0300 through VramQueueCopy; back is the JSR's return address. */
static void end_box2(u8 lo, u8 hi, u16 back)
{
    lda8(lo); wr8(DP(0x02), lo);
    lda8(hi); wr8(DP(0x03), hi);
    lda8(0x89); wr8(DP(0x04), 0x89);
    lda8(0x20); wr8(DP(0x10), 0x20);
    lda8(0x02); wr8(DP(0x11), 0x02);
    lda8(0x00); wr8(DP(0x00), 0x00);
    lda8(0x03); wr8(DP(0x01), 0x03);
    push16(back); VramQueueCopy_m1x1();
}

/* $80:9AF4 EndScreenCode (RTS): after the $88 delay the code box
 * ($89:D880); then A ($26 bit 7): the step $0453 two on, $88 = $20, the
 * next box ($89:D980), the five digits at $11:5730 to VRAM $0A15 and sound
 * $34. (Stadium 1 instead sets step $0A with sound $43 and does not redraw
 * the box.) */
void EndScreenCode_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        u8 t = dec8(rd8(DP(MODE_TIMER)));
        wr8(DP(MODE_TIMER), t);
        if (!fz) { ret(); return; }
        end_box2(0x80, 0xD8, 0x9B1A);
    }
    lda8(rd8(DP(P1_PRESS_LO)));
    and8(0x80);
    if (fz) { ret(); return; }
    wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
    wr8(ABS(END_STEP), inc8(rd8(ABS(END_STEP))));
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    end_box2(0x80, 0xD9, 0x9B49);
    rep(0x20);
    lda16(0x0A15); wr16(DP(0x00), 0x0A15);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9B5F); QueueFiveDigits_m1x1();
    lda8(0x34);
    push8(0x80); push16(0x9B65); QueueSound_m1x1();
    ret();
}

/* $80:9B67 EndScreenChoice (RTS): after the $88 delay the box ($89:D900);
 * then A: sound $34 at the code choice (step 5 in $0454), $43 otherwise;
 * either way to step $0454 with $88 = $20 and the next box ($89:D980).
 * (Stadium 1 jumps straight to the code step $0A from step 5.) */
void EndScreenChoice_m1x1(void)
{
    lda8(rd8(DP(MODE_TIMER)));
    if (!fz) {
        u8 t = dec8(rd8(DP(MODE_TIMER)));
        wr8(DP(MODE_TIMER), t);
        if (!fz) { ret(); return; }
        end_box2(0x00, 0xD9, 0x9B8D);
    }
    lda8(rd8(DP(P1_PRESS_LO)));
    and8(0x80);
    if (fz) { ret(); return; }
    lda8(rd8(ABS(0x0454)));
    cmp8((u8)A, 0x05);
    if (fz) {
        lda8(0x34);
        push8(0x80); push16(0x9BA0); QueueSound_m1x1();
    } else {
        lda8(0x43);
        push8(0x80); push16(0x9BA8); QueueSound_m1x1();
    }
    lda8(rd8(ABS(0x0454))); wr8(ABS(END_STEP), (u8)A);
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    end_box2(0x80, 0xD9, 0x9BD1);
    ret();
}

void QueueTimeDigits_m1x1(void);
void SaveBestRecords_m1x1(void);

/* $80:973F (RTS): the broadcast totals screen: BG3 / tile bases, the
 * saved best total ($11:572C) at $09D3 and best score ($11:5730) at $0A15;
 * the four course times $083D-$0848 rounded through $80:C6C9 and summed
 * (minutes, seconds, hundredths) into $084D-$084F and $0849-$084B; the
 * records saved; $1905 = $17, $1923 + 1, $75 + 1, $0452/$0453 cleared, $88
 * = $20, $9A = $D8; unlike Stadium 1, $190D is
 * left as it is. */
void BroadcastTotals_m1x1(void)
{
    lda8(0x19); wr8(ABS(BG3SC), 0x19);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    rep(0x20);
    lda16(0x09D3); wr16(DP(0x00), 0x09D3);
    lda16(0x572C); wr16(DP(0x18), 0x572C);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9763); QueueTimeDigits_m1x1();
    rep(0x20);
    lda16(0x0A15); wr16(DP(0x00), 0x0A15);
    lda16(0x5730); wr16(DP(0x18), 0x5730);
    sep(0x20);
    lda8(0x11); wr8(DP(0x1A), 0x11);
    push8(0x80); push16(0x9779); QueueFiveDigits_m1x1();
    wr8(ABS(0x084D), 0);
    wr8(ABS(0x084E), 0);
    wr8(ABS(0x084F), 0);
    ldx8(0x00);
    do {
        Y = rd8(ABSX(0x083F)); nz8((u8)Y);
        lda8(rd8(ABSY(0xC6C9))); wr8(ABSX(0x083F), (u8)A);
        fc = 0; adc8(rd8(ABS(0x084F)));
        wr8(ABS(0x084F), (u8)A);
        cmp8((u8)A, 0x64);
        if (fc) {
            sbc8(0x64);
            wr8(ABS(0x084F), (u8)A);
            u8 v = inc8(rd8(ABS(0x084E))); wr8(ABS(0x084E), v);
            if (fz) wr8(ABS(0x084D), inc8(rd8(ABS(0x084D))));
        }
        lda8(rd8(ABSX(0x083E)));
        fc = 0; adc8(rd8(ABS(0x084E)));
        wr8(ABS(0x084E), (u8)A);
        cmp8((u8)A, 0x3C);
        if (fc) {
            sbc8(0x3C);
            wr8(ABS(0x084E), (u8)A);
            wr8(ABS(0x084D), inc8(rd8(ABS(0x084D))));
        }
        lda8(rd8(ABSX(0x083D)));
        fc = 0; adc8(rd8(ABS(0x084D)));
        wr8(ABS(0x084D), (u8)A);
        lda8(rd8(ABS(0x084D))); wr8(ABS(0x0849), (u8)A);
        lda8(rd8(ABS(0x084E))); wr8(ABS(0x084A), (u8)A);
        lda8(rd8(ABS(0x084F))); wr8(ABS(0x084B), (u8)A);
        X = inc8((u8)X); X = inc8((u8)X); X = inc8((u8)X);
        cmp8((u8)X, 0x0C);
    } while (!fc);
    push8(0x80); push16(0x97E2); SaveBestRecords_m1x1();
    lda8(0x17); wr8(ABS(SH_TM), 0x17);
    wr8(ABS(FADE_STATE), inc8(rd8(ABS(FADE_STATE))));
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    wr8(ABS(END_STEP), 0);
    wr8(ABS(ENTRY_CURSOR), 0);
    lda8(0x20); wr8(DP(MODE_TIMER), 0x20);
    lda8(0xD8); wr8(DP(0x9A), 0xD8);
    ret();
}

/* $80:9E50 Mode10_Ending (RTS): count $88 down (16-bit); with $0708 left,
 * $9E = $0100; at 0 game mode $11 ($75 / $77 cleared, $1923 - 1). Stadium 1
 * has no $9E step and goes to mode $0F. */
void Mode10_Ending_m1x1(void)
{
    rep(0x20);
    wr16(DP(MODE_TIMER), dec16(rd16(DP(MODE_TIMER))));
    lda16(rd16(DP(MODE_TIMER)));
    cmp16(A, 0x0708);
    if (fz) { lda16(0x0100); wr16(DP(0x9E), 0x0100); }
    lda16(rd16(DP(MODE_TIMER)));
    sep(0x20);
    if (!fz) { ret(); return; }
    lda8(0x11); wr8(DP(GAME_MODE), 0x11);
    wr8(DP(MODE_ACTIVE), 0);
    wr8(DP(MODE_STEP), 0);
    wr8(ABS(FADE_STATE), dec8(rd8(ABS(FADE_STATE))));
    ret();
}

/* Reset: the boot and the main loop (never returns; checked in fragments.txt). */
R816_EXPORT void ResetBoot1(void);
R816_EXPORT void ResetBoot1Spc(void);
R816_EXPORT void ResetBoot2(void);
R816_EXPORT void MainLoopFrame1(void);
static void wait_apu_idle1(void);
void BroadcastEventInject_m1x1(void);
void BsxDetect_m1x1(void);
void BsxOpenTimeChannel_m1x1(void);
void BsxPollStream_m1x1(void);
void ClearOamBuffer_m1x1(void);
void DrawClock_m1x1(void);
void FadeStep_m1x1(void);
void InitFadeState_m1x1(void);
void InitPpuRegisters_m1x1(void);
void LaneFromY_m1x1(void);
void PackOamHighTable_m1x1(void);
void RaceBookkeeping_m1x1(void);
void ReadJoypads_m1x1(void);
void RiderPaletteWords_m1x1(void);
void RunGameModeScript_m1x1(void);
void SpcUpload_m1x1(void);
void StepLfsr_m1x1(void);
void TickTimers_m1x1(void);

/* $80:FFA5 -> $80:8000 Reset (Stadium 2): the boot, then a main loop frame after each
 * NMI. */
/* The boot alone, and one turn of the main loop (a frame if the NMI has
 * flagged 081A), for a frame-loop driver (port/src/native_driver.c). */
R816_EXPORT void ResetBootAll(void)
{
    ResetBoot1();
    ResetBoot1Spc();
    wait_apu_idle1();
    ResetBoot2();
}

R816_EXPORT void MainLoopTurn(void)
{
    lda8(rd8(ABS(NMI_FRAME_READY)));
    if (!fz)
        MainLoopFrame1();
}

void ResetVector_m1x1(void)
{
    ResetBootAll();
    for (;;) {
        do lda8(rd8(ABS(NMI_FRAME_READY))); while (fz);
        MainLoopFrame1();
    }
}
/* $80:8000-$80:807A: native mode, D = 0, stack $1FFF; BS-X detection (the
 * time channel when $7F:FFE1 is clear); broadcast clock minute 2, frame 2,
 * $7F:FFF9 = $13, $7F:FFFB = 5; DB = 0; NMI, DMA and APU ports off, forced
 * blank, the PPU registers. */
R816_EXPORT void ResetBoot1(void)
{
    fi = 1;
    fc = 0;                                 /* CLC; XCE (native mode) */
    rep(0x30);
    lda16(0x0000); D = A; nz16(D);
    lda16(0x1FFF); S = A;
    sep(0x30);
    push8(0x80); push16(0x8012); BsxDetect_m1x1();
    lda8(rd8(LONG(0x7FFFE1)));
    if (fz) { push8(0x80); push16(0x801C); BsxOpenTimeChannel_m1x1(); }
    lda8(0x13); wr8(LONG(0x7FFFF9), 0x13);
    lda8(0x02); wr8(LONG(0x7FFFFF), 0x02); wr8(LONG(0x7FFFFA), 0x02);
    lda8(0x05); wr8(LONG(0x7FFFFB), 0x05);
    lda8(0x1E); wr8(ABS(0x1E84), 0x1E);
    wr8(ABS(0x1E85), 0);
    sep(0x20);
    lda8(0x00); push8(0x00); DB = pull8(); nz8(DB);
    wr8(ABS(NMITIMEN), 0);
    wr8(ABS(HDMAEN), 0);
    wr8(ABS(MDMAEN), 0);
    wr8(ABS(APUIO0), 0);
    wr8(ABS(APUIO1), 0);
    wr8(ABS(APUIO2), 0);
    wr8(ABS(APUIO3), 0);
    lda8(0x80); wr8(ABS(INIDISP), 0x80);
    push8(0x80); push16(0x805E); InitPpuRegisters_m1x1();
    lda8(0x00);
    wr8(ABS(0x1E1C), 0);
    wr8(DP(0xF2), 0);
    wr8(DP(0xF4), 0);
    lda8(0x00);
    wr8(LONG(0x7FFFE3), 0);
    wr8(LONG(0x7FFFE4), 0);
}
/* $80:8072: the first SPC upload (set 0). */
R816_EXPORT void ResetBoot1Spc(void)
{
    ldx8(0x00);
    push8(0x80); push16(0x8077); SpcUpload_m1x1();
}
/* $80:8078: NMI off and wait until all four APU ports read 0; Stadium 2's
 * loop goes back to the NMI write each time (Stadium 1's only to the port
 * reads, as Stadium 4's does to the write). */
static void wait_apu_idle1(void)
{
    do {
        wr8(ABS(NMITIMEN), 0);
        lda8(rd8(ABS(APUIO0)));
        ora8(rd8(ABS(APUIO1)));
        ora8(rd8(ABS(APUIO2)));
        ora8(rd8(ABS(APUIO3)));
    } while (!fz);
}
/* $80:8089-$80:80E9: the second SPC upload, the fade state; DB = $80; IRQ
 * timer at V $D0 / H $98, FastROM, NMI on; APU port 0 = $FC; clock and
 * game state defaults; game mode $0D. ($80:80EB-$80:80FD, skipped by the
 * BRA, is a leftover debug start: $1542 = 1, mode 4, $7D = 1, TM = $17.) */
R816_EXPORT void ResetBoot2(void)
{
    ldx8(0x01);
    push8(0x80); push16(0x808E); SpcUpload_m1x1();
    push8(0x80); push16(0x8092); InitFadeState_m1x1();
    lda8(0x80); push8(0x80); DB = pull8(); nz8(DB);
    rep(0x20);
    lda16(0x00D0); wr16(ABS(VTIMEL), A);
    lda16(0x0098); wr16(ABS(HTIMEL), A);
    sep(0x20);
    lda8(0x01); wr8(ABS(MEMSEL), 0x01);
    lda8(0x81); wr8(ABS(NMITIMEN), 0x81);
    wr8(ABS(SH_NMITIMEN), 0x81);
    wr8(ABS(APUIO1), 0);
    wr8(ABS(APUIO2), 0);
    wr8(ABS(APUIO3), 0);
    lda8(0xFC); wr8(ABS(APUIO0), 0xFC);
    wr8(ABS(0x1E87), 0);
    wr8(ABS(0x1E88), 0);
    wr8(ABS(0x1556), 0);
    lda8(0x04); wr8(ABS(0x083A), 0x04);
    wr8(ABS(0x083B), 0);
    wr8(ABS(0x083C), 0);
    lda8(0x04);
    wr8(ABS(0x083D), 0x04);
    wr8(ABS(0x0840), 0x04);
    wr8(ABS(0x0843), 0x04);
    wr8(ABS(0x0846), 0x04);
    lda8(0x0D); wr8(ABS(GAME_MODE), 0x0D);
}
/* $80:8103-$80:819C: one main loop frame after NMI ($081A set): minute to
 * $B7, the BS-X stream, the random shift register, the joypads, IRQs on.
 * In the debug build ($81:800E) pad 2's L/R step the broadcast minute
 * (restarting it) and Select / Start toggle a pause ($1917) that skips the
 * game logic. Then a fade step, or the frame counter $63, timers, broadcast
 * events, the OAM buffer and the game mode script; the race's lanes and
 * bookkeeping when $90; rider palettes, the OAM high table, the clock;
 * $081A cleared. */
R816_EXPORT void MainLoopFrame1(void)
{
    lda8(rd8(LONG(0x7FFFFF)));
    wr8(ABS(0x00B7), (u8)A);
    push8(0x80); push16(0x810D); BsxPollStream_m1x1();
    push16(0x8110); StepLfsr_m1x1();
    push8(0x80); push16(0x8114); ReadJoypads_m1x1();
    fi = 0;
    lda8(rd8(LONG(0x81800E)));
    if (!fz) {
        lda8(rd8(DP(P2_PRESS_LO)));
        and8(0xC0);
        if (!fz) {
            lda8(asl8((u8)A));
            lda8(rd8(LONG(0x7FFFFF)));
            if (fc) {
                lda8(dec8((u8)A));
                if (fn) lda8(0x00);
            } else {
                lda8(inc8((u8)A));
            }
            wr8(LONG(0x7FFFFF), (u8)A);
            wr8(ABS(0x00B0), inc8(rd8(ABS(0x00B0))));
            lda8(0x01); wr8(ABS(0x1E84), 0x01);
            rep(0x20);
            lda16(0x0000); wr16(LONG(0x7FFFFA), 0);
            sep(0x20);
        }
        lda8(rd8(DP(P1_PRESS_LO)));
        and8(0x20);
        if (!fz) wr8(ABS(DEBUG_PAUSE), inc8(rd8(ABS(DEBUG_PAUSE))));
        lda8(rd8(DP(P1_PRESS_LO)));
        and8(0x10);
        if (fz) {
            lda8(rd8(ABS(DEBUG_PAUSE)));
            and8(0x01);
            wr8(ABS(DEBUG_PAUSE), (u8)A);
            if (!fz) goto paused;
        }
    }
    lda8(rd8(ABS(FADE_STATE)));
    if (!fz) {
        push8(0x80); push16(0x8171); FadeStep_m1x1();
        goto tail;
    }
    rep(0x20);
    wr16(DP(FRAME_COUNT), inc16(rd16(DP(FRAME_COUNT))));
    sep(0x20);
    push16(0x817C); TickTimers_m1x1();
    push16(0x817F); BroadcastEventInject_m1x1();
    push16(0x8182); ClearOamBuffer_m1x1();
    push16(0x8185); RunGameModeScript_m1x1();
paused:
    lda8(rd8(DP(0x90)));
    if (!fz) {
        push16(0x818C); LaneFromY_m1x1();
        push16(0x818F); RaceBookkeeping_m1x1();
    }
tail:
    push16(0x8192); RiderPaletteWords_m1x1();
    push16(0x8195); PackOamHighTable_m1x1();
    push16(0x8198); DrawClock_m1x1();
    wr8(ABS(NMI_FRAME_READY), 0);
}
