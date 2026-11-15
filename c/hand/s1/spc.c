/*
 * Stadium 1 (Gen A): sample upload to the sound CPU through the boot ROM's
 * transfer handshake ($80:88E0), as Stadium 4's c/hand/s4/spc.c. Not built
 * into the port (it uploads natively, and a hand-written handshake loop
 * cannot wait for the sound CPU there); checked with
 * tools/difftest816.py --apu-echo.
 */
#include "hand/hand.h"
#include "ram.h"

/* Upload the stream at the long pointer $00-$02 with the driver's transfer
 * handshake (APUIO0-2), then clear the ports. */
static void upload_from_dp(void)
{
    wr8(ABS(0x1978), 0);
    push8(DB);
    lda8(0x00);
    push8(0x00);
    DB = pull8(); nz8(DB);
    push8(getP());
    rep(0x30);
    Y = 0;
    lda16(0xBBAA);
    do {
        WAIT_TICK();
        cmp16(A, rd16(ABS(APUIO0)));
    } while (!fz);
    sep(0x20);
    lda8(0xCC);
    for (;;) {
        push8((u8)A);
        rep(0x20);
        u32 src = ILDP(0x00);
        u16 len = rd16((src + Y) & 0xFFFFFF); Y += 2;
        X = len;
        u16 dest = rd16((src + Y) & 0xFFFFFF); Y += 2;
        wr16(ABS(APUIO2), dest);
        sep(0x20);
        cmp16(X, 0x0001);
        lda8(0x00);
        lda8(rol8((u8)A));
        wr8(ABS(APUIO1), (u8)A);
        adc8(0x7F);
        lda8(pull8());
        wr8(ABS(APUIO0), (u8)A);
        while (rd8(ABS(APUIO0)) != (u8)A)
            WAIT_TICK();
        if (!fv)
            break;
        /* one block of X bytes */
        lda8(rd8((ILDP(0x00) + Y) & 0xFFFFFF)); Y++;
        A = (u16)((A << 8) | (A >> 8));
        lda8(0x00);
        for (;;) {
            rep(0x20);
            wr16(ABS(APUIO0), A);
            sep(0x20);
            X--;
            if (X == 0) break;
            A = (u16)((A << 8) | (A >> 8));
            lda8(rd8((ILDP(0x00) + Y) & 0xFFFFFF)); Y++;
            if (Y == 0x8000) { Y = 0; wr8(DP(0x02), inc8(rd8(DP(0x02)))); }
            A = (u16)((A << 8) | (A >> 8));
            while (rd8(ABS(APUIO0)) != (u8)A)
                WAIT_TICK();
            lda8(inc8((u8)A));
        }
        do {                                    /* CMP: carry set on the match */
            WAIT_TICK();
            cmp8((u8)A, rd8(ABS(APUIO0)));
        } while (!fz);
        do { adc8(0x03); } while (fz);
    }
    wr8(ABS(APUIO0), 0);
    wr8(ABS(APUIO1), 0);
    wr8(ABS(APUIO2), 0);
    wr8(ABS(APUIO3), 0);
    setP(pull8());
    DB = pull8(); nz8(DB);
}

/* $80:88E0 SpcUpload (RTL), X = stream number: $80:88CE/88D4/88DA[X]. */
void SpcUpload_m1x1(void)
{
    lda8(0xFF);
    wr8(ABS(APUIO0), 0xFF);
    lda8(rd8(LONG(0x8088CE + X))); wr8(DP(0x00), (u8)A);
    lda8(rd8(LONG(0x8088D4 + X))); wr8(DP(0x01), (u8)A);
    lda8(rd8(LONG(0x8088DA + X))); wr8(DP(0x02), (u8)A);
    upload_from_dp();
    retl();
}

/* $80:9EAB (RTS): start the closing credits: with NMI off the credits
 * music (stream 0x05) uploaded, NMI back, $75 + 1; BG3 / tile
 * bases; scroll $9E = $FFFF, colour math $1C00 = $90 / $1C01 =
 * $20, $1915 + 1; sound command $10; HDMA channel 7 (direct, to $2100,
 * table $00:0455) with the table copied from $80:9F27;
 * $190F = $80, $1905 = 4, $087E / $0880 / $0882 / $88
 * cleared. */
void StartCredits_m1x1(void)
{
    wr8(ABS(NMITIMEN), 0);
    ldx8(0x05);
    push8(0x80); push16(0x9EB3); SpcUpload_m1x1();
    lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    lda8(0x32); wr8(ABS(BG3SC), 0x32);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    rep(0x20);
    lda16(0xFFFF); wr16(DP(0x9E), 0xFFFF);
    lda16(0x0090); wr16(ABS(PALETTE_BUF), 0x0090);
    lda16(0x0020); wr16(ABS(0x1C01), 0x0020);
    sep(0x20);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    lda8(0x10); wr8(ABS(APUIO0), 0x10);
    lda8(0x00); wr8(ABS(DMAP7), 0);
    lda8(0x00); wr8(ABS(BBAD7), 0);
    lda8(0x55); wr8(ABS(A1T7L), 0x55);
    lda8(0x04); wr8(ABS(A1T7H), 0x04);
    lda8(0x00); wr8(ABS(A1B7), 0);
    lda8(0x00); wr8(ABS(DASB7), 0);
    ldx8(0x40);
    do {
        lda8(rd8(ABSX(0x9F27))); wr8(ABSX(0x0455), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x80); wr8(ABS(SH_HDMAEN), 0x80);
    lda8(0x04); wr8(ABS(SH_TM), 0x04);
        wr8(ABS(0x087E), 0);
    wr8(ABS(0x0880), 0);
    wr8(ABS(0x0882), 0);
    wr8(DP(MODE_TIMER), 0);
    ret();
}

void DrawSelectWheels_m1x1(void);
void TurnSelectWheels_m1x1(void);
void SegmentNext_m1x1(void);
void SelectTimerDigits_m1x1(void);
void QueueSound_m1x1(void);

/* The rider chosen: character $1542 (from $1547 >> shift), with NMI off
 * the rider sprite buffers $1400-$14FF and the wheel state $65-$70
 * cleared, the character's sound bank uploaded (stream $1542 + 1), sound
 * $3F, the select delay $1554 = $60, NMI back on. */
static void select_rider1(int shift, u16 back_upload, u16 back_sound)
{
    if (shift == 4) and8(0x30);
    for (int k = 0; k < shift; k++) lda8(lsr8((u8)A));
    wr8(ABS(0x1542), (u8)A);
    wr8(ABS(NMITIMEN), 0);
    rep(0x20);
    ldx8(0x3E);
    do {
        wr16(ABSX(SND_RING), 0); wr16(ABSX(SND_RING + 0x40), 0);
        wr16(ABSX(SND_RING + 0x80), 0); wr16(ABSX(SND_RING + 0xC0), 0);
        X = dec8((u8)X); X = dec8((u8)X);
    } while (!fn);
    sep(0x20);
    static const u8 wheel[12] = { 0x69, 0x65, 0x6D, 0x6A, 0x66, 0x6E, 0x6B, 0x67, 0x6F, 0x6C, 0x68, 0x70 };
    for (int k = 0; k < 12; k++) wr8(DP(wheel[k]), 0);
    X = rd8(ABS(0x1542)); nz8((u8)X);
    X = inc8((u8)X);
    push8(0x80); push16(back_upload); SpcUpload_m1x1();
    lda8(0x3F);
    push8(0x80); push16(back_sound); QueueSound_m1x1();
    lda8(0x60); wr8(ABS(0x1554), 0x60);
    lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
}

/* $80:8D2F / $80:8DE6 (RTS): one frame of a rider select screen: after a
 * choice the delay $1554 runs out (then the next segment); otherwise the
 * wheels turn ($1E8B) and are drawn, the timer $88 counts down (at 1 the
 * pad is released), and when it runs out with no rider placed ($1701 =
 * $F0), or on A ($24 bit 7) unless locked ($154B), the rider under the
 * cursor ($1547) is chosen. The second screen also clears $1542 while
 * waiting and redraws the timer digits. */
static void rider_select1(int second, u16 base)
{
    lda8(rd8(ABS(0x1554)));
    if (!fz) {
        cmp8((u8)A, 0x01);
        if (fz) {
            wr8(DP(P1_HELD_HI), 0);
            push8(0x80); push16(base + 0x18); DrawSelectWheels_m1x1();
            SegmentNext_m1x1();
            return;
        }
        wr8(DP(P1_HELD_HI), 0);
        if (second) {
            wr8(ABS(0x1554), dec8(rd8(ABS(0x1554))));
            push8(0x80); push16(base + 0x11); DrawSelectWheels_m1x1();
        } else {
            push8(0x80); push16(base + 0x0E); DrawSelectWheels_m1x1();
            wr8(ABS(0x1554), dec8(rd8(ABS(0x1554))));
        }
        ret();
        return;
    }
    rep(0x20);
    lda16(rd16(ABS(MODE_TIMER)));
    cmp16(A, 0x0001);
    sep(0x20);
    if (fz) wr8(DP(P1_HELD_HI), 0);
    lda8(rd8(ABS(0x1E8B))); wr8(DP(0x1F), (u8)A);
    push8(0x80); push16(base + 0x32); TurnSelectWheels_m1x1();
    push8(0x80); push16(base + 0x36); DrawSelectWheels_m1x1();
    rep(0x20);
    wr16(ABS(MODE_TIMER), dec16(rd16(ABS(MODE_TIMER))));
    sep(0x20);
    int choose = 0;
    if (fz) {
        lda8(rd8(ABS(0x1701)));
        cmp8((u8)A, 0xF0);
        if (!fz) choose = 1;
        else { lda8(0x01); wr8(ABS(MODE_TIMER), 0x01); }
    } else {
        if (second) wr8(ABS(0x1542), 0);
        lda8(rd8(DP(P1_HELD_LO)));
        and8(0x80);
        if (!fz) {
            lda8(rd8(ABS(0x154B)));
            if (fz) choose = 1;
        }
    }
    if (choose) {
        lda8(rd8(ABS(0x1547)));
        if (second) select_rider1(6, 0x8E86, 0x8E90);
        else select_rider1(4, 0x8DCC, 0x8DD6);
    }
    push16(second ? 0x8E9E : 0x8DE4); SelectTimerDigits_m1x1();
    ret();
}

void RiderSelectFirst_m1x1(void)  { rider_select1(0, 0x8D2F); }
void RiderSelectSecond_m1x1(void) { rider_select1(1, 0x8DE6); }
