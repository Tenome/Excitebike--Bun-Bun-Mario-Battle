/*
 * Stadium 3-4 (Gen B): sample upload to the sound CPU, as the original does
 * it ($82:8593). The port replaces this with the framework's native upload
 * (port/src/r816_bridge.c); this version is what tools/difftest816.py runs.
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

/* $82:8593 SpcUploadB (RTL), X = stream number: $82:857E/8585/858C[X]. */
void SpcUploadB_m1x1(void)
{
    lda8(0xFF);
    wr8(ABS(APUIO0), 0xFF);
    lda8(rd8(LONG(0x82857E + X))); wr8(DP(0x00), (u8)A);
    lda8(rd8(LONG(0x828585 + X))); wr8(DP(0x01), (u8)A);
    lda8(rd8(LONG(0x82858C + X))); wr8(DP(0x02), (u8)A);
    upload_from_dp();
    retl();
}

/* The same upload from a given stream address (an added character's). */
void SpcUploadFrom(u32 stream)
{
    lda8(0xFF);
    wr8(ABS(APUIO0), 0xFF);
    wr8(DP(0x00), (u8)stream);
    wr8(DP(0x01), (u8)(stream >> 8));
    wr8(DP(0x02), (u8)(stream >> 16));
    upload_from_dp();
}

void SpcUploadB_m1x1(void);

/* $80:9DDA (RTS): start the closing credits: with NMI off the credits
 * music (stream 0x06) uploaded, NMI back, $75 + 1; BG3 / tile
 * bases; scroll $A6 = $FFFF, colour math $1C00 = $90 / $1C01 =
 * $20, $1915 + 1; sound command $10; HDMA channel 7 (direct, to $2100,
 * table $00:0452) with the table copied from $80:9E5E;
 * $190F = $80, $1905 = 4, $1906 cleared, BG mode 9, $087E / $0880 / $0882 / $88
 * cleared. */
void StartCredits_m1x1(void)
{
    wr8(ABS(NMITIMEN), 0);
    ldx8(0x06);
    push8(0x80); push16(0x9DE2); SpcUploadB_m1x1();
    lda8(rd8(ABS(SH_NMITIMEN))); wr8(ABS(NMITIMEN), (u8)A);
    wr8(DP(MODE_ACTIVE), inc8(rd8(DP(MODE_ACTIVE))));
    lda8(0x32); wr8(ABS(BG3SC), 0x32);
    lda8(0x44); wr8(ABS(BG12NBA), 0x44);
    lda8(0x01); wr8(ABS(BG34NBA), 0x01);
    rep(0x20);
    lda16(0xFFFF); wr16(DP(0xA6), 0xFFFF);
    lda16(0x0090); wr16(ABS(PALETTE_BUF), 0x0090);
    lda16(0x0020); wr16(ABS(0x1C01), 0x0020);
    sep(0x20);
    wr8(ABS(PALETTE_DIRTY), inc8(rd8(ABS(PALETTE_DIRTY))));
    lda8(0x10); wr8(ABS(APUIO0), 0x10);
    lda8(0x00); wr8(ABS(DMAP7), 0);
    lda8(0x00); wr8(ABS(BBAD7), 0);
    lda8(0x52); wr8(ABS(A1T7L), 0x52);
    lda8(0x04); wr8(ABS(A1T7H), 0x04);
    lda8(0x00); wr8(ABS(A1B7), 0);
    lda8(0x00); wr8(ABS(DASB7), 0);
    ldx8(0x40);
    do {
        lda8(rd8(ABSX(0x9E5E))); wr8(ABSX(0x0452), (u8)A);
        X = dec8((u8)X);
    } while (!fn);
    lda8(0x80); wr8(ABS(SH_HDMAEN), 0x80);
    lda8(0x04); wr8(ABS(SH_TM), 0x04);
        wr8(ABS(SH_TS), 0);
    lda8(0x09); wr8(ABS(BGMODE), 0x09);
    wr8(ABS(0x087E), 0);
    wr8(ABS(0x0880), 0);
    wr8(ABS(0x0882), 0);
    wr8(DP(MODE_TIMER), 0);
    ret();
}
