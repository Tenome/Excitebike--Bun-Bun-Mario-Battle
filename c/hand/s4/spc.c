/*
 * Stadium 3-4 (Gen B): sample upload to the sound CPU, as the original does
 * it ($82:8593). The port replaces this with the framework's native upload
 * (port/src/r816_bridge.c); this version is what tools/difftest816.py runs.
 */
#include "hand/hand.h"

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
    while (rd16(ABS(0x2140)) != 0xBBAA)
        WAIT_TICK();
    sep(0x20);
    lda8(0xCC);
    for (;;) {
        push8((u8)A);
        rep(0x20);
        u32 src = ILDP(0x00);
        u16 len = rd16((src + Y) & 0xFFFFFF); Y += 2;
        X = len;
        u16 dest = rd16((src + Y) & 0xFFFFFF); Y += 2;
        wr16(ABS(0x2142), dest);
        sep(0x20);
        cmp16(X, 0x0001);
        lda8(0x00);
        lda8(rol8((u8)A));
        wr8(ABS(0x2141), (u8)A);
        adc8(0x7F);
        lda8(pull8());
        wr8(ABS(0x2140), (u8)A);
        while (rd8(ABS(0x2140)) != (u8)A)
            WAIT_TICK();
        if (!fv)
            break;
        /* one block of X bytes */
        lda8(rd8((ILDP(0x00) + Y) & 0xFFFFFF)); Y++;
        A = (u16)((A << 8) | (A >> 8));
        lda8(0x00);
        for (;;) {
            rep(0x20);
            wr16(ABS(0x2140), A);
            sep(0x20);
            X--;
            if (X == 0) break;
            A = (u16)((A << 8) | (A >> 8));
            lda8(rd8((ILDP(0x00) + Y) & 0xFFFFFF)); Y++;
            if (Y == 0x8000) { Y = 0; wr8(DP(0x02), inc8(rd8(DP(0x02)))); }
            A = (u16)((A << 8) | (A >> 8));
            while (rd8(ABS(0x2140)) != (u8)A)
                WAIT_TICK();
            lda8(inc8((u8)A));
        }
        while (rd8(ABS(0x2140)) != (u8)A)
            WAIT_TICK();
        do { adc8(0x03); } while (fz);
    }
    wr8(ABS(0x2140), 0);
    wr8(ABS(0x2141), 0);
    wr8(ABS(0x2142), 0);
    wr8(ABS(0x2143), 0);
    setP(pull8());
    DB = pull8(); nz8(DB);
}

/* $82:8593 SpcUploadB (RTL), X = stream number: $82:857E/8585/858C[X]. */
void SpcUploadB_m1x1(void)
{
    lda8(0xFF);
    wr8(ABS(0x2140), 0xFF);
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
    wr8(ABS(0x2140), 0xFF);
    wr8(DP(0x00), (u8)stream);
    wr8(DP(0x01), (u8)(stream >> 8));
    wr8(DP(0x02), (u8)(stream >> 16));
    upload_from_dp();
}
