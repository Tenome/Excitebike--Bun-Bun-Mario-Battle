/*
 * Stadium 3-4 (Gen B) VRAM transfer queue, hand-written. Same queue as
 * Stadium 1 (c/hand/s1/vram.c: records at $7F:0000, head word first), with
 * one fix: the record's source address (+5/+6) is stored as a full word, so
 * records past offset $F4 work.
 */
#include "hand/hand.h"

#define QUEUE_HEAD   LONG(0x7F0000)

static inline u32 q(u16 off, u16 field) { return (0x7F0000u + field + off) & 0xFFFFFF; }

/* Record header at queue offset `off`: VMAIN $80, DMAP $01, BBAD $18, source
 * $7F:(off + $0C), VRAM address $00/$01, byte count words * 2. */
static void begin_record(u16 off, u8 words)
{
    wr8(q(off, 2), 0x80);
    wr8(q(off, 3), 0x01);
    wr8(q(off, 4), 0x18);
    wr16(q(off, 5), (u16)(off + 0x0C));
    wr8(q(off, 7), 0x7F);
    wr8(q(off, 1), rd8(DP(0x01)));
    wr8(q(off, 0), rd8(DP(0x00)));
    wr8(DP(0x06), words);
    wr8(q(off, 8), (u8)(words << 1));
    wr8(q(off, 9), 0x00);
}

/* Close a record: link it, and step the VRAM address one tilemap row. */
static void end_record(void)
{
    u16 head = rd16(QUEUE_HEAD);
    u16 next = (u16)(head + 0x0C + rd16(DP(0x08)));
    wr16(QUEUE_HEAD, next);
    wr16(q(head, 0x0A), next);
    A = next;                                       /* high byte stays in B */
    SET_A8(0x20); fc = 0;
    adc8(rd8(DP(0x00))); wr8(DP(0x00), (u8)A);
    SET_A8(0x00);
    adc8(rd8(DP(0x01))); wr8(DP(0x01), (u8)A);
}

/*
 * $80:B2DE VramQueueTiles (M=1, X=1, RTS): a block of tilemap entries from
 * the byte list at ($02): first byte = size (low nibble words per row, high
 * nibble rows), then one tile number per entry ORed with the attribute word
 * $0E/$0F ($0E cleared). One queue record per row; the VRAM address $00/$01
 * advances $20 words per row.
 */
/* The queue body, reading the list from a full 24-bit address (the port's
 * added characters keep their names outside bank $80). */
void VramQueueTilesFrom(u32 list)
{
    wr8(DP(0x0E), 0);
    push8((u8)X);
    push8((u8)Y);
    u8 size = rd8(list);
    u8 words = size & 0x0F;
    wr8(DP(0x05), words);
    wr8(DP(0x08), (u8)(words << 1));
    wr8(DP(0x09), 0);
    wr8(DP(0x04), size >> 4);
    u16 y = 0;
    do {
        u16 off = rd16(QUEUE_HEAD);
        begin_record(off, rd8(DP(0x05)));
        u16 x = off;
        do {
            y++;
            u16 tile = (u16)((rd16((list + y) & 0xFFFFFF) & 0x00FF) | rd16(DP(0x0E)));
            wr16(q(x, 0x0C), tile);
            x += 2;
            wr8(DP(0x06), (u8)(rd8(DP(0x06)) - 1));
        } while (rd8(DP(0x06)) != 0);
        end_record();
        wr8(DP(0x04), (u8)(rd8(DP(0x04)) - 1));
        X = x; Y = y;
    } while (rd8(DP(0x04)) != 0);
    sep(0x10);
    Y = pull8();
    ldx8(pull8());
    ret();
}

void VramQueueTiles_m1x1(void)
{
    VramQueueTilesFrom(IDP(0x02));
}

/* $80:B2DA VramQueueTiles20: VramQueueTiles with attribute high byte $20. */
void VramQueueTiles20_m1x1(void)
{
    wr8(DP(0x0F), 0x20);
    SET_A8(0x20);
    VramQueueTiles_m1x1();
}
