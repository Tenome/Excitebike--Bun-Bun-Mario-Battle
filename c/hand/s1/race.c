/*
 * Stadium 1 race setup, hand-written.
 */
#include "hand/hand.h"

void QueueSound_m1x1(void);

/* $80:9201 EnableHudSplit (RTS): $91 = 1, so the NMI handler also enables the
 * H/V timer IRQ that splits the HUD from the track. */
void EnableHudSplit_m1x1(void)
{
    lda8(0x01);
    wr8(DP(0x91), 0x01);
    ret();
}

/* $80:9206 ClearRaceState (RTS): zero the race work areas $0884-$09B1
 * (direct page relative, 16-bit index) and $09B2-$0A12 (through the pointer
 * at $00, which is left pointing at $09B2), and $80-$84; sound $2D. */
void ClearRaceState_m1x1(void)
{
    rep(0x10);
    u16 x = 0x0884;
    do {
        wr8((u16)(D + x), 0);
        x++;
        cmp16(x, 0x09B2);
    } while (!fc);
    X = x;
    sep(0x10);
    lda8(0x09); wr8(DP(0x01), 0x09);
    lda8(0xB2); wr8(DP(0x00), 0xB2);
    lda8(0x00);
    for (int y = 0x60; y >= 0; y--) {
        Y = (u16)y;
        wr8(IDPY(0x00), 0);
    }
    Y = 0xFF; nz8(0xFF);
    for (int k = 3; k >= 0; k--) {
        X = (u16)k;
        wr8(DPX(0x80), 0);
    }
    X = 0xFF;
    wr8(DP(0x84), 0);
    lda8(0x2D);
    push8(0x80); push16(0x9234); QueueSound_m1x1();
    ret();
}

void VramQueueTiles20_m1x1(void);
void VramQueueBlock_m1x1(void);

/* Point the queuers at a tile list in bank $80 for VRAM `vram`. */
static void set_list(u16 vram, u16 list)
{
    rep(0x20);
    A = vram; wr16(DP(0x00), vram);
    A = list; wr16(DP(0x02), list);
    sep(0x20);
}

/*
 * $80:A5B7 DrawRacerName (RTS). In a final ($7D) also queues the block at
 * $80:A5F4 to VRAM $185B. Then the racer's name: 29-byte records at
 * $80:A5F9 indexed by the character in $1542 (multiplied with the hardware
 * multiplier), to VRAM $1851.
 */
void DrawRacerName_m1x1(void)
{
    lda8(rd8(DP(0x7D)));
    if ((u8)A != 0) {
        set_list(0x185B, 0xA5F4);
        push16(0xA5CB);
        VramQueueTiles20_m1x1();
    }
    rep(0x20);
    A = 0x1851; wr16(DP(0x00), 0x1851);
    sep(0x20);
    lda8(rd8(ABS(0x1542)));
    wr8(ABS(0x4202), (u8)A);                           /* WRMPYA = character */
    lda8(0x1D);
    wr8(ABS(0x4203), 0x1D);                            /* WRMPYB = 29 */
    rep(0x20);
    A = rd16(ABS(0x4216));                             /* RDMPY */
    fc = 0;
    adc16(0xA5F9);
    wr16(DP(0x02), A);
    sep(0x20);
    VramQueueBlock_m1x1();                             /* JMP */
}

/* $80:A66D DrawHudLabelA (RTS): fixed tile block $80:A67E to VRAM $18D1. */
void DrawHudLabelA_m1x1(void)
{
    set_list(0x18D1, 0xA67E);
    VramQueueBlock_m1x1();
}

/* $80:A68F DrawHudLabelB (RTS): unless $84, block $80:A6A5 to VRAM $1891. */
void DrawHudLabelB_m1x1(void)
{
    lda8(rd8(DP(0x84)));
    if ((u8)A != 0) { ret(); return; }
    set_list(0x1891, 0xA6A5);
    VramQueueBlock_m1x1();
}

/* Big two-tile-wide digit, two rows: tiles d, d+1 over d+16, d+17, as the
 * 2x2 list at $0CF8 for VramQueueTiles20 to VRAM $00/$01. */
static void queue_big_digit(u16 jsr_ret, u8 first)
{
    lda8(first);
    wr8(ABS(0x0CF9), (u8)A);
    SET_A8(inc8((u8)A)); wr8(ABS(0x0CFA), (u8)A);
    fc = 0; adc8(0x0F); wr8(ABS(0x0CFB), (u8)A);
    SET_A8(inc8((u8)A)); wr8(ABS(0x0CFC), (u8)A);
    lda8(0x22); wr8(ABS(0x0CF8), 0x22);                /* 2 wide, 2 rows */
    lda8(0xF8); wr8(DP(0x02), 0xF8);
    lda8(0x0C); wr8(DP(0x03), 0x0C);
    push16(jsr_ret);
    VramQueueTiles20_m1x1();
}

/* $80:D113 DrawPlace (RTS): when $086E (the player's place, 1-8) is set,
 * draw it as a big digit (tiles $30 + (place - 1) * 2) at VRAM $1859. */
void DrawPlace_m1x1(void)
{
    lda8(rd8(ABS(0x086E)));
    if ((u8)A == 0) { ret(); return; }
    lda8(0x59); wr8(DP(0x00), 0x59);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(rd8(ABS(0x086E)));
    SET_A8(dec8((u8)A));
    SET_A8(asl8((u8)A));
    fc = 0; adc8(0x30);
    queue_big_digit(0xD149, (u8)A);
    ret();
}

/* $80:D14B DrawLap (RTS): the lap number $0928 as a big digit from the
 * tile table $80:D17D, at VRAM $1894. */
void DrawLap_m1x1(void)
{
    lda8(0x94); wr8(DP(0x00), 0x94);
    lda8(0x18); wr8(DP(0x01), 0x18);
    lda8(rd8(ABS(0x0928)));
    X = dec8((u8)A);
    queue_big_digit(0xD17B, rd8(ABS(0xD17D + X)));
    ret();
}

/* $80:9236 RaceHudInit (RTS): draw the HUD labels and "lap 1" once
 * ($0928 = 1 just for the drawing, then 0). */
void RaceHudInit_m1x1(void)
{
    lda8(0x01);
    wr8(ABS(0x0928), 0x01);
    push16(0x923D); DrawHudLabelB_m1x1();
    push16(0x9240); DrawLap_m1x1();
    push16(0x9243); DrawHudLabelA_m1x1();
    wr8(ABS(0x0928), 0);
    ret();
}

/*
 * $80:9248 RaceSetup (RTS): racer name (not on the special course), lap
 * state ($086E = $7D * 8, $086F-$0876 = 0), lap number, the six 128-byte
 * tables $09D3-$0CD2 filled with $3B, (places $086E-$0875 = $7D * 8) the first OAM entry set to an
 * off-screen sprite, and the HUD timer digits' positions $0270-$0273.
 */
void RaceSetup_m1x1(void)
{
    lda8(rd8(ABS(0x0CD7)));
    if ((u8)A == 0) {
        push16(0x924F);
        DrawRacerName_m1x1();
    }
    lda8(rd8(DP(0x7D)));
    SET_A8(asl8(asl8(asl8((u8)A))));
    for (int x = 7; x >= 0; x--) {
        wr8(ABS(0x086E + x), (u8)A);
        wr8(ABS(0x086F + x), 0);
    }
    X = 0xFF;
    push16(0x9262); DrawPlace_m1x1();
    lda8(0x3B);
    for (int x = 0x7F; x >= 0; x--)
        for (int t = 0; t < 6; t++)
            wr8(ABS(0x09D3 + t * 0x80 + x), 0x3B);
    X = 0xFF;
    lda8(0xBF); wr8(ABS(0x1601), 0xBF);
    lda8(0xF7); wr8(ABS(0x1600), 0xF7);
    lda8(0xFF); wr8(ABS(0x1602), 0xFF);
    lda8(0x34); wr8(ABS(0x0270), 0x34);
    lda8(0x14); wr8(ABS(0x0271), 0x14);
    lda8(0x0A); wr8(ABS(0x0273), 0x0A);
    lda8(0x05); wr8(ABS(0x0272), 0x05);
    ret();
}

/* Rider progress along the course (16-bit, one word per rider at $085E) and
 * place (1-8, one word per rider at $086E; the player is rider 0). */

/*
 * $80:D0BA UpdatePlayerPlace (RTS). During a race ($086E set, $84 clear,
 * $8E set): place = 8 minus the riders whose progress is lower than the
 * player's (equal does not count).
 */
void UpdatePlayerPlace_m1x1(void)
{
    if (rd8(ABS(0x086E)) == 0 || rd8(DP(0x84)) != 0 || rd8(DP(0x8E)) == 0) {
        ret();
        return;
    }
    rep(0x20);
    A = 0x0008;
    wr16(ABS(0x086E), 0x0008);
    u16 me = rd16(ABS(0x085E));
    A = me;
    for (u8 y = 0x0E; y != 0; y -= 2) {
        Y = y;
        u16 other = rd16(ABS(0x085E + y));
        cmp16(me, other);
        if (me > other)
            wr16(ABS(0x086E), dec16(rd16(ABS(0x086E))));
    }
    Y = 0;
    nz8(0);
    sep(0x20);
    ret();
}

/*
 * $80:D0E5 ComputePlaces (RTS). For every rider X: place = 8 minus the
 * riders it is ahead of: riders with lower progress, and riders with equal
 * progress and a lower index.
 */
void ComputePlaces_m1x1(void)
{
    rep(0x20);
    for (int x = 0x0E; x >= 0; x -= 2) {
        X = (u16)x;
        wr8(DP(0x00), (u8)x);
        A = 0x0008;
        wr16(ABS(0x086E + x), 0x0008);
        for (int y = 0x0E; y >= 0; y -= 2) {
            Y = (u16)y;
            if ((u8)y == rd8(DP(0x00)))
                continue;
            u16 mine = rd16(ABS(0x085E + x)), theirs = rd16(ABS(0x085E + y));
            A = mine;
            if (mine < theirs)
                continue;
            if (mine == theirs && (u8)y >= rd8(DP(0x00)))
                continue;
            wr16(ABS(0x086E + x), dec16(rd16(ABS(0x086E + x))));
        }
    }
    X = 0xFE;
    Y = 0xFE;
    sep(0x20);
    ret();
}

/*
 * $80:D38E DrawCoinIcons (RTL). One HUD coin sprite per coin collected
 * ($1540, up to 5): sprite n (n = count .. 1) at OAM $1704 + n * 4, position
 * from $80:D3B5 + n * 2, tile/attribute $2D88, large size.
 */
void DrawCoinIcons_m1x1(void)
{
    u8 x = (u8)(rd8(ABS(0x1540)) << 1);
    for (; x != 0; x -= 2) {
        X = x;
        u8 y = (u8)(x << 1);
        Y = y;
        rep(0x20);
        A = rd16(ABS(0xD3B5 + x));
        wr16(ABS(0x1704 + y), A);                 /* X, Y position */
        A = 0x2D88;
        wr16(ABS(0x1706 + y), 0x2D88);            /* tile $88, attribute $2D */
        sep(0x20);
        Y = (u8)(x >> 1);
        lda8(0x02);
        wr8(ABS(0x1861 + Y), 0x02);               /* large */
    }
    X = 0;
    fc = 1; fz = 1; fn = 0;                       /* the loop's CPX #0 at exit */
    retl();
}
