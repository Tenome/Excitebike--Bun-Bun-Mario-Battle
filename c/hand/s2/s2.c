/*
 * Stadium 2 (the 1997 S2 dump): routines that differ from Stadium 1's and
 * so are written here rather than carried over by tools/port_hand.py
 * (from_s1.c). Checked by tools/difftest816.py s2.
 */
#include "hand/hand.h"

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
    wr8(ABS(0x4202), (u8)A);
    lda8(0x1D);
    wr8(ABS(0x4203), 0x1D);
    rep(0x20);
    A = rd16(ABS(0x4216));
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
    lda8(0x34); wr8(ABS(0x0270), 0x34);
    lda8(0x0A); wr8(ABS(0x0273), 0x0A);
    lda8(0x05); wr8(ABS(0x0272), 0x05);
    lda8(0x74); wr8(ABS(0x0271), 0x74);
    lda8(0x08); wr8(ABS(0x0188), 0x08);
    lda8(0x09); wr8(ABS(0x0260), 0x09);
    lda8(0x30); wr8(ABS(0x0190), 0x30);
    lda8(0x06); wr8(ABS(0x01E0), 0x06);
    lda8(0x01);
    wr8(ABS(0x0198), 0x01); wr8(ABS(0x01D8), 0x01); wr8(ABS(0x0958), 0x01);
    lda8(asl8((u8)A));
    wr8(DP(0x90), (u8)A);
    lda8(0x17); wr8(ABS(0x1905), 0x17);
    lda8(0x01);
    push8(0x80); push16(0xD875); SetHdmaPreset_m1x1();
    ldx8(0x00);
    lda8(0xB0); wr8(ABS(0x0CD5), 0xB0); wr8(ABS(0x01B0), 0xB0);
    push16(0xD882); DrawAllRiders_m1x1();
    lda8(0xB8); wr8(ABS(0x0CD5), 0xB8); wr8(ABS(0x01B0), 0xB8);
    static const u8 ch[12][2] = {
        {0x70,0x42},{0x71,0x0F},{0x72,0xE6},{0x73,0xD8},{0x74,0x80},{0x77,0x00},
        {0x60,0x40},{0x61,0x08},{0x62,0xED},{0x63,0xD8},{0x64,0x80},{0x67,0x00} };
    for (int i = 0; i < 12; i++) { lda8(ch[i][1]); wr8(ABS(0x4300 | ch[i][0]), ch[i][1]); }
    rep(0x20);
    lda16(0x012C); wr16(DP(0x88), 0x012C);
    wr16(ABS(0x1B00), 0); wr16(ABS(0x1B02), 0);
    sep(0x20);
    lda8(0x09); wr8(ABS(0x1B20), 0x09);
    lda8(0x08); wr8(ABS(0x1B21), 0x08);
    lda8(0xDC); wr8(ABS(0x190F), 0xDC);
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
    lda8(rd8(DP(0x88)));
    if (!fz) goto tick;
    lda8(rd8(DP(0xB2)));
    if (fz) {
        ldy8(0x0E);
        do {
            lda8(rd8(ABSY(0x086E)));
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
        lda8(rd8(DP(0x01))); wr8(ABSY(0x0309), (u8)A);
        lda8(0x10); wr8(ABSY(0x0369), 0x10);
        X = rd8(ABS(0x039D)); nz8((u8)X);
        wr8(ABSX(0x0309), inc8(rd8(ABSX(0x0309))));
        Y = rd8(DP(0x00)); nz8((u8)Y);
        lda8(rd8(ABSY(0x9936)));
        push8(0x81); push16(0x989B); FindObjectSlot_m1x1();
        lda8(0x04); wr8(ABSY(0x0309), 0x04);
        lda8(0xFF); wr8(ABSY(0x0369), 0xFF);
        lda8(0xFF); wr8(ABSY(0x0379), 0xFF);
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
    lda8(0xFC); wr8(ABS(0x03D3), 0xFC); wr8(ABS(0x03D8), 0xFC);
    lda8(0x04); wr8(DP(0x88), 0x04);
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
    wr8(DP(0x88), dec8(rd8(DP(0x88))));
    X = rd8(ABS(0x039D)); nz8((u8)X);
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
    ora8(rd8(ABS(0x0CE0)));
    u8 top;
    if (!fz) goto off;
    lda8(rd8(ABS(0x0940)));
    if (!fz) {
        lda8(rd8(DP(0x60)));
        lda8(lsr8((u8)A));
        if (fc) goto off;
        top = 0x01;
        goto draw;
    }
    lda8(rd8(ABS(0x0178)));
    and8(0xC0);
    if (!fz) {
        Y = rd8(DP(0x8E)); nz8((u8)Y);
        if (fz) lda8(0x80);
        lda8(asl8((u8)A)); lda8(rol8((u8)A)); lda8(rol8((u8)A));
    }
    Y = (u8)A; nz8((u8)Y);
    lda8(rd8(ABS(0x0990)));
    cmp8((u8)A, rd8(ABSY(0xBDFE)));
    if (!fc) {
        lda8(rd8(ABS(0x098F)));
        fc = 0; adc8(rd8(ABSY(0xBDFA)));
        wr8(ABS(0x098F), (u8)A);
        if (fc) wr8(ABS(0x0990), inc8(rd8(ABS(0x0990))));
    } else if (!fz) {
        lda8(rd8(ABS(0x098F)));
        fc = 1; sbc8(0x0B);
        wr8(ABS(0x098F), (u8)A);
        if (!fc) {
            lda8(rd8(ABS(0x0990)));
            if (!fz) wr8(ABS(0x0990), dec8(rd8(ABS(0x0990))));
        }
    }
    ldx8(0x00);
    lda8(rd8(ABS(0x0990)));
    cmp8((u8)A, 0x20);
    if (fc) {
        wr8(DP(0x60), (u8)A);
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
            lda8(rd8(ABS(0x0CE0)));
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
    lda8(rd8(ABS(0x0990)));
    fc = 0; adc8(0x58);
    X = rd8(ABS(0x0CE0)); nz8((u8)X);
    if (!fz) lda8(0x78);
    wr8(DP(0x00), (u8)A);
    ldx8(0x0C);
    lda8(rd8(ABS(0x0CE0)));
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
