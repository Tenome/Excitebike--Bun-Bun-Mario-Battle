/*
 * Stadium 3-4 (Gen B): rider sprites and palettes, hand-written.
 * Per-character data comes from the roster (roster.h), so the port can have
 * more characters than the original's tables hold.
 */
#include "hand/hand.h"
#include "roster.h"

/*
 * $82:8965 RiderGfxCycle (RTS): while $1E0C is set, every other frame queue
 * the next of 16 animation strips: source $C000 + $82:8940[n] ($1E0A) to
 * VRAM $4F00 + (n & 3) * $100 ($1E10), n = $1E0E counting up.
 */
void RiderGfxCycle_m1x1(void)
{
    rep(0x20);
    lda16(rd16(ABS(0x1E0C)));
    if (!fz) {
        wr16(ABS(0x1E10), 0);
        wr16(ABS(0x1E0E), inc16(rd16(ABS(0x1E0E))));
        lda16(rd16(ABS(0x1E0E)));
        and16(0x0001);
        if (fz) {
            lda16(rd16(ABS(0x1E0E)));
            and16(0x001E);
            X = (u8)A; nz8((u8)X);
            lda16(rd16(ABS(0x8940 + X)));
            fc = 0;
            adc16(0xC000);
            wr16(ABS(0x1E0A), A);
            lda16((u16)X);
            and16(0x0006);
            lda16(lsr16(A));
            A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);   /* XBA */
            fc = 0;
            adc16(0x4F00);
            wr16(ABS(0x1E10), A);
        }
    }
    sep(0x20);
    ret();
}

/*
 * $82:87FD RiderSpriteSets (RTL), every frame: set the four rider sprite
 * slots' animation sources. Slot s shows frame f = $1961,s of its rider set
 * (roster_rider_offset/bank for the player's character $154F and the race
 * variant $1E91): $1E12+2s = set offset + (f & $FE) * $200 + (f & 1) * $100,
 * bank $1550+s. Slot 0 frame $15 is replaced by $82:8960[$091C]. With a fade
 * running ($1923) or in a mode without riders ($82:928E[$76] = 0) the
 * streams are switched off.
 */
void RiderSpriteSets_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82);
    DB = pull8(); nz8(DB);
    push16(0x8804); RiderGfxCycle_m1x1();
    lda8(rd8(ABS(0x1923)));
    int on = 0;
    if (fz) {
        X = rd8(DP(0x76));
        lda8(rd8(ABS(0x928E + X)));
        on = !fz;
    }
    if (!on) {
        rep(0x20);
        for (int a = 0x1E12; a <= 0x1E18; a += 2)
            wr16(ABS(a), 0);
        sep(0x20);
        DB = pull8(); nz8(DB);
        retl();
        return;
    }
    rep(0x20);
    ldx8(0x06);
    ldy8(0x03);
    u8 ch = rd8(ABS(0x154F)), var = rd8(ABS(0x1E91));
    do {
        lda16(rd16(ABS(0x1961 + Y)));
        wr16(DP(0x0E), A);
        if (Y == 0) {
            and16(0x00FF);
            cmp16(A, 0x0015);
            if (fz) {
                lda16(rd16(ABS(0x091C + Y)));
                push8((u8)X);
                X = (u8)A;
                lda16(rd16(LONG(0x828960 + X)));
                X = pull8();
                wr16(DP(0x0E), A);
            }
        }
        and16(0x0001);
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);       /* XBA */
        wr16(DP(0x00), A);
        wr16(ABS(0x000C), 0);
        wr8(DP(0x0C), (u8)X);
        /* X = ch * 8 + slot * 2: the original's table index */
        lda16((u16)(ch << 3));
        fc = 0;
        adc16(rd16(DP(0x0C)));
        X = (u8)A; nz8((u8)X);
        u8 slotx = (u8)(X - (ch << 3));               /* byte index in the record: slot * 2 */
        lda16(rd16(ABS(0x1E91)) & 0x00FF);
        lda16(roster_rider_offset(ch, var, slotx));
        wr16(DP(0x1E), A);
        lda16(rd16(DP(0x0E)));
        and16(0x00FE);
        lda16(asl16(A));
        A = (u16)((A << 8) | (A >> 8)); nz8((u8)A);       /* XBA */
        fc = 0;
        adc16(rd16(DP(0x00)));
        fc = 0;
        adc16(rd16(DP(0x1E)));
        X = rd8(DP(0x0C));
        wr16(ABS(0x1E12 + X), A);
        X = dec8((u8)X);
        X = dec8((u8)X);
        Y = dec8((u8)Y);
    } while (!fn);
    sep(0x20);
    lda8(rd8(ABS(0x154F)));
    lda8(asl8((u8)A));
    lda8(asl8((u8)A));
    X = (u8)A; nz8((u8)X);
    lda8(rd8(ABS(0x1E91)));
    for (int s = 0; s < 4; s++) {
        lda8(roster_rider_bank(ch, var, s));
        wr8(ABS(0x1550 + s), (u8)A);
    }
    DB = pull8(); nz8(DB);
    retl();
}

/*
 * $82:899B LoadRiderPalettes (RTL): copy the five rider palettes of the
 * player's character (roster_rider_palette: Stadium, race variant,
 * character) to the palette buffer $1D20-$1DBF (sprite palettes 1-5) and
 * request the upload ($1915).
 */
void LoadRiderPalettes_m1x1(void)
{
    push8(DB);
    lda8(0x82);
    push8(0x82);
    DB = pull8(); nz8(DB);
    int st = rd8(LONG(0x81800C)) != 0;
    int var = rd8(ABS(0x1E91)) != 0;
    u8 ch = rd8(ABS(0x154F));
    /* $00 = st * 100 + var * 50 + ch * 10 (8-bit), as the original leaves it */
    wr8(DP(0x00), (u8)((st ? 100 : 0) + (var ? 50 : 0) + ch * 10));
    wr8(DP(0x01), 0);
    rep(0x30);
    Y = 0;
    for (int i = 0; i < 5; i++) {
        lda16((u16)(i * 2));
        fc = 0;
        adc16(rd16(DP(0x00)));                      /* index into $82:8A08 (sets V) */
        u32 pal = roster_rider_palette(st, var, ch, i);
        wr16(DP(0x02), (u16)pal);
        push16((u16)i);
        X = (u16)(i * 32);
        for (int k = 0; k < 32; k += 2) {
            lda16(rd16(LONG(pal + k)));
            wr16(ABS(0x1D20 + X), A);
            X += 2;
        }
        wr16(DP(0x04), 0);
        Y = pull16();
        Y++;
        cmp16(Y, 0x0005);
    }
    sep(0x30);
    wr8(ABS(0x1915), inc8(rd8(ABS(0x1915))));
    DB = pull8(); nz8(DB);
    retl();
}
