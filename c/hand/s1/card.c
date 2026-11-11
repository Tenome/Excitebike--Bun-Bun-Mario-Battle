/*
 * Stadium 1 course cards ("コース N フリー走行" / "コース N 決勝"), hand-written.
 */
#include "hand/hand.h"

void ForceBlank_m1x1(void);
void DmaKick_m0x1(void);
void VramQueueTiles_m1x1(void);

/*
 * $80:8C53 Mode02_Setup (RTS). Works out the broadcast segment from the
 * minute: s = how many of the boundaries {11, 17, 23, 29, 35, 41, 47, 99}
 * ($80:8C4B) have passed; course $7C = s / 2, kind $7D = s & 1 (0 = free
 * run, 1 = final). Uploads the matching card graphic ($88:C000 or $88:C100,
 * $100 bytes) to VRAM $00C0 with the screen off, then queues the 2x4 tile
 * "course number" block (tiles $10 + course * 2 ...) at VRAM $00CD (free run)
 * or $00D0 (final), attribute $18, and starts the fade-in.
 */
void Mode02_Setup_m1x1(void)
{
    wr8(ABS(0x190D), 0);
    wr8(ABS(0x4200), 0);                               /* NMITIMEN off */
    push8(0x80); push16(0x8C5C); ForceBlank_m1x1();

    /* segment from the minute */
    ldx8(0xFF);
    wr8(DP(0x00), 0xFF);
    lda8(rd8(LONG(0x7FFFFF)));
    do {
        wr8(DP(0x00), inc8(rd8(DP(0x00))));
        X = inc8((u8)X);
        cmp8((u8)A, rd8(ABS(0x8C4B + X)));
    } while (fc);
    u8 seg = rd8(DP(0x00));
    u8 course = lsr8(seg);
    wr8(DP(0x7C), course);
    u8 kind = (u8)(rol8(course) & 0x01);
    lda8(kind);
    wr8(DP(0x7D), kind);
    SET_A8(asl8(kind));
    fc = 0;
    adc8(kind);
    wr8(DP(0x00), (u8)A);                              /* kind * 3: entry in $80:8D02 */

    /* card graphic, through channel 0 with D = $4300 */
    rep(0x20);
    push16(D);
    A = 0x4300; D = 0x4300;
    Y = 0x80; wr8(ABS(0x2115), 0x80);
    A = 0x1801; wr16(DP(0x00), 0x1801);
    wr16(ABS(0x2116), 0x00C0);
    wr16(DP(0x05), 0x0100);
    Y = rd8(ABS(0x0000));
    A = rd16(ABS(0x8D02 + Y));                         /* source address */
    X = rd8(ABS(0x8D04 + Y));                          /* source bank */
    Y = 0x01;
    push16(0x8CA6); DmaKick_m0x1();
    D = pull16();
    sep(0x20);
    lda8(rd8(ABS(0x190E)));
    wr8(ABS(0x4200), (u8)A);                           /* NMITIMEN back */
    lda8(0x01);
    wr8(DP(0x75), 0x01);
    wr8(ABS(0x1923), inc8(rd8(ABS(0x1923))));          /* fade in */

    /* "course N" tiles: list at $03D2 = [size $42][8 tiles] */
    lda8(0x42);
    wr8(ABS(0x03D2), 0x42);                            /* 2 wide, 4 rows */
    u8 t = (u8)((rd8(DP(0x7C)) << 1) | 0x10);
    lda8(t);
    for (int row = 0; row < 4; row++) {
        if (row) { fc = 0; adc8(0x0F); }
        wr8(ABS(0x03D3 + row * 2), (u8)A);
        SET_A8(inc8((u8)A));
        wr8(ABS(0x03D4 + row * 2), (u8)A);
    }
    lda8(rd8(DP(0x7D)));
    and8(0x01);
    Y = asl8((u8)A);
    rep(0x20);
    A = rd16(ABS(0x8D08 + Y));
    wr16(DP(0x00), A);                                 /* VRAM $00CD or $00D0 */
    A = 0x03D2;
    wr16(DP(0x02), 0x03D2);
    sep(0x20);
    lda8(0x18);
    wr8(DP(0x0F), 0x18);                               /* attribute */
    push16(0x8D00);
    VramQueueTiles_m1x1();
    ret();
}

/*
 * $80:8C2F Mode02_SegmentCard (RTS). Shows the card until the segment's
 * start minute ($80:8C43[$7C * 2 + $7D]: 7, 13, 19, ...), then moves on
 * ($80:915A, a tail jump).
 */
void Mode02_SegmentCard_m1x1(void)
{
    lda8(rd8(DP(0x7C)));
    SET_A8(asl8((u8)A));
    fc = 0;
    adc8(rd8(DP(0x7D)));
    X = (u8)A;
    lda8(rd8(LONG(0x7FFFFF)));
    cmp8((u8)A, rd8(ABS(0x8C43 + X)));
    if (!fc) {
        ret();
        return;
    }
    if (!r816_call_pc(0x80915A))
        r816_unknown_target(0x808C3F, 0x915A);
}
