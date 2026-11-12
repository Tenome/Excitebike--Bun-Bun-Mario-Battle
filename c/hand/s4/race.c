/*
 * Stadium 3-4 (Gen B): race HUD and results, hand-written.
 */
#include "hand/hand.h"
#include "roster.h"

void VramQueueTiles20_m1x1(void);
void VramQueueTiles_m1x1(void);
void VramQueueTilesFrom(u32 list);

static void queue20(u16 vram, u16 list)
{
    rep(0x20);
    lda16(vram); wr16(DP(0x00), vram);
    lda16(list); wr16(DP(0x02), list);
    sep(0x20);
}

/*
 * $80:A4DA DrawRaceHud (RTS): the race HUD labels: block $80:A58B at VRAM
 * $1882, the course number ($80:A590 + course $7C * 2) at $1886, block
 * $80:A574 at $1B24 (attribute $20); in a final ($7D) also $80:A586 at $185C
 * and $80:A583 at $1859; then the player's name (roster_name) at $1852.
 */
void DrawRaceHud_m1x1(void)
{
    queue20(0x1882, 0xA58B);
    push16(0xA4EA); VramQueueTiles20_m1x1();
    rep(0x20);
    lda16(0x1886); wr16(DP(0x00), 0x1886);
    lda16(rd16(DP(0x7C)) & 0x00FF);
    lda16(asl16(A));
    fc = 0;
    adc16(0xA590);
    wr16(DP(0x02), A);
    sep(0x20);
    push16(0xA502); VramQueueTiles20_m1x1();
    queue20(0x1B24, 0xA574);
    lda8(0x20); wr8(DP(0x0F), 0x20);
    push16(0xA517); VramQueueTiles_m1x1();
    lda8(rd8(DP(0x7D)));
    if (!fz) {
        queue20(0x185C, 0xA586);
        push16(0xA52C); VramQueueTiles20_m1x1();
        queue20(0x1859, 0xA583);
        push16(0xA53D); VramQueueTiles20_m1x1();
    }
    rep(0x20);
    lda16(0x1852); wr16(DP(0x00), 0x1852);
    sep(0x20);
    u8 ch = rd8(ABS(0x154F));
    lda8(ch);
    wr8(ABS(0x4202), ch);                           /* WRMPYA: ch * 15 */
    lda8(0x0F);
    wr8(ABS(0x4203), 0x0F);
    rep(0x20);
    int st = rd8(LONG(0x81800C)) != 0;
    lda16(rd16(ABS(0x4216)));                       /* RDMPYL: ch * 15 */
    fc = 0;
    adc16(0xA598);
    wr16(DP(0x02), A);
    lda16(rd16(LONG(0x81800C)));
    if (!fz) {
        lda16(rd16(DP(0x02)));
        fc = 0;
        adc16(0x004B);
        wr16(DP(0x02), A);
    }
    /* the original's pointer, or the roster's own name for an added character */
    u32 name = roster_name(st, ch, rd16(DP(0x02)));
    wr16(DP(0x02), (u16)name);
    sep(0x20);
    /* JMP VramQueueTiles20 with the list at `name` */
    wr8(DP(0x0F), 0x20);
    SET_A8(0x20);
    VramQueueTilesFrom(name);
}

/*
 * $81:C0F6 WinnerFigure (RTS), result object X ($03A3): every 8 frames flip
 * its frame bit ($031F) and step it left ($027F); until it reaches its spot
 * (X < $95 in the jump contest $0309 = 4, else < $90) keep walking; there,
 * show the winner's pose: the rider in first place ($086E,rider = 1) of a
 * race of character $154F (roster_winner_pose) -> $030F, timer 64. The action
 * $034F is $81:C156[frame bit].
 */
void WinnerFigure_m1x1(void)
{
    X = rd8(ABS(0x03A3));
    lda8(rd8(ABS(0x036F + X)));
    if (fz) {
        lda8(rd8(ABS(0x031F + X)));
        eor8(0x01);
        wr8(ABS(0x031F + X), (u8)A);
        lda8(0x08);
        wr8(ABS(0x036F + X), 0x08);
        wr8(ABS(0x027F + X), dec8(rd8(ABS(0x027F + X))));
        lda8(rd8(ABS(0x0309)));
        cmp8((u8)A, 0x04);
        u8 limit = fz ? 0x95 : 0x90;
        lda8(rd8(ABS(0x027F + X)));
        cmp8((u8)A, limit);
        if (!fc) {
            ldy8(0x0E);
            for (;;) {
                lda8(dec8(rd8(ABS(0x086E + Y))));
                if (fz) break;
                Y = dec8((u8)Y);
                Y = dec8((u8)Y);
                if (fn) break;
            }
            lda8((u8)Y);
            lda8(lsr8((u8)A));
            wr8(DP(0x00), (u8)A);
            u8 rider = (u8)A;
            u8 ch = rd8(ABS(0x154F));
            lda8((u8)(ch << 3));
            fc = 0;
            adc8(rd8(DP(0x00)));
            wr8(DP(0x00), (u8)A);
            Y = (u8)A; nz8((u8)Y);
            lda8(roster_winner_pose(ch, rider));
            wr8(ABS(0x030F + X), (u8)A);
            lda8(0x40);
            wr8(ABS(0x036F + X), 0x40);
        }
    }
    ldy8(rd8(ABS(0x031F + X)));
    lda8(rd8(ABS(0xC156 + Y)));
    wr8(ABS(0x034F + X), (u8)A);
    ret();
}
