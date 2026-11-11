/*
 * Stadium 1 VRAM transfer queue, hand-written.
 *
 * The queue lives at $7F:0000. The first word is the offset of the next free
 * record; each record is
 *   +0  VRAM word address        +2  VMAIN ($80: increment after high byte)
 *   +3  DMAP ($01: two registers) +4  BBAD ($18: VMDATAL)
 *   +5  source address (low byte only: $7F:00xx + $0C, see below), +7 bank $7F
 *   +8  byte count               +A  offset of the following record
 *   +C  the data itself
 * The NMI handler ($80:876B) walks the records and DMAs each one.
 */
#include "hand/hand.h"

#define QUEUE_HEAD   LONG(0x7F0000)

static inline u32 q(u16 off, u16 field) { return (0x7F0000u + field + off) & 0xFFFFFF; }

/*
 * $80:B3FE VramQueueCopy (M=1, X=1, RTS). Queues $11 rows of $10 words from
 * the long pointer at $02-$04 to VRAM address $00/$01, one record per row;
 * the VRAM address advances one tilemap row ($20 words) per record. The
 * source pointer runs on across rows (16-bit Y).
 *
 * Note: +5/+6 store only the low byte of (offset + $0C) and a zero, so the
 * record's own source address is only right while the queue stays under
 * $100 bytes. The NMI side decides whether that matters (kept as is).
 */
void VramQueueCopy_m1x1(void)
{
    u8 x_in = (u8)X, y_in = (u8)Y;
    push8(x_in);
    push8(y_in);
    u16 y = 0;
    wr8(DP(0x08), (u8)(rd8(DP(0x10)) << 1));          /* bytes per row */
    wr8(DP(0x09), 0);
    u32 src = ILDP(0x02);
    do {
        u16 off = rd16(QUEUE_HEAD);
        wr8(q(off, 2), 0x80);
        wr8(q(off, 3), 0x01);
        wr8(q(off, 4), 0x18);
        wr8(q(off, 5), (u8)((off & 0xFF) + 0x0C));
        wr8(q(off, 6), 0x00);
        wr8(q(off, 7), 0x7F);
        wr8(q(off, 1), rd8(DP(0x01)));                /* VRAM address */
        wr8(q(off, 0), rd8(DP(0x00)));
        u8 words = rd8(DP(0x10));
        wr8(DP(0x06), words);
        wr8(q(off, 8), (u8)(words << 1));
        wr8(q(off, 9), 0x00);
        u16 x = off;
        do {
            wr8(q(x, 0x0C), rd8((src + y) & 0xFFFFFF)); y++;
            wr8(q(x, 0x0D), rd8((src + y) & 0xFFFFFF)); y++;
            x += 2;
            wr8(DP(0x06), (u8)(rd8(DP(0x06)) - 1));
        } while (rd8(DP(0x06)) != 0);
        u16 head = rd16(QUEUE_HEAD);
        u16 next = (u16)(head + 0x0C + rd16(DP(0x08)));
        wr16(QUEUE_HEAD, next);
        wr16(q(head, 0x0A), next);                     /* link to the next record */
        /* next tilemap row: VRAM address += $20 (8-bit adds, carry chain) */
        SET_A8(0x20); fc = 0;
        adc8(rd8(DP(0x00))); wr8(DP(0x00), (u8)A);
        SET_A8(0x00);
        adc8(rd8(DP(0x01))); wr8(DP(0x01), (u8)A);
        wr8(DP(0x11), (u8)(rd8(DP(0x11)) - 1));
        X = x; Y = y;
    } while (rd8(DP(0x11)) != 0);
    sep(0x10);
    Y = pull8();
    ldx8(pull8());
    ret();
}

/* Fill in one record header at queue offset `off` (shared by the queuers). */
static void begin_record(u16 off, u8 words)
{
    wr8(q(off, 2), 0x80);
    wr8(q(off, 3), 0x01);
    wr8(q(off, 4), 0x18);
    wr8(q(off, 5), (u8)((off & 0xFF) + 0x0C));
    wr8(q(off, 6), 0x00);
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
    SET_A8(0x20); fc = 0;
    adc8(rd8(DP(0x00))); wr8(DP(0x00), (u8)A);
    SET_A8(0x00);
    adc8(rd8(DP(0x01))); wr8(DP(0x01), (u8)A);
}

/*
 * $80:B2A9 VramQueueTiles (M=1, X=1, RTS). Queues a block of tilemap entries
 * from a byte list at ($02): the first byte holds the size (low nibble =
 * words per row, high nibble = rows), then one tile number per entry, which
 * is ORed with the attribute word $0E/$0F ($0E is cleared, so the caller's
 * $0F supplies palette/priority bits). One queue record per row; the VRAM
 * address $00/$01 advances $20 words per row.
 */
void VramQueueTiles_m1x1(void)
{
    wr8(DP(0x0E), 0);
    u8 x_in = (u8)X, y_in = (u8)Y;
    push8(x_in);
    push8(y_in);
    u32 list = IDP(0x02);                              /* DB:($02) */
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

/* $80:B2A5 VramQueueTiles20: VramQueueTiles with attribute high byte $20. */
void VramQueueTiles20_m1x1(void)
{
    wr8(DP(0x0F), 0x20);
    SET_A8(0x20);
    VramQueueTiles_m1x1();
}
