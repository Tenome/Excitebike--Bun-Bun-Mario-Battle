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
 * NmiFlushVramQueue ($80:8979) DMAs every record during vblank (records
 * start at $7F:0002; head $0002 = empty, $FFFF = locked) and resets the head.
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
 * $100 bytes (a record past offset $F4 would DMA from the wrong place). In
 * play the queue stays smaller (largest head seen over 6,000 intro/title
 * frames: $E2), so the original never hits it; kept as is.
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

/*
 * $80:8979 NmiFlushVramQueue (M=1, X=1, RTL), in vblank. DMAs every queued
 * record to VRAM with channel 0, then empties the queue (head = $0002).
 * Head $FFFF means the queue is being written: skip this frame.
 */
void NmiFlushVramQueue_m1x1(void)
{
    push8(DB);
    push8(0x80); DB = pull8();                         /* PHK / PLB */
    rep(0x30);
    Y = 0;
    wr16(DP(0x02), 0);
    sep(0x20);
    X = 0;
    push8(DB);
    push8(0x7F); DB = pull8();                         /* queue bank */
    wr8(DP(0x00), 0x7F);
    rep(0x20);
    u16 head = rd16(ABS(0x0000));
    A = head;
    wr16(DP(0x04), head);
    int locked = head == 0xFFFF;
    int empty = rd16(DP(0x02)) == 0 && head == 0x0002;   /* $02 was just cleared */
    if (!locked && !empty) {
        u16 x = 2;
        do {
            wr16(LONG(0x004300), rd16(ABS(x + 3)));    /* DMAP0, BBAD0 */
            wr16(LONG(0x002116), rd16(ABS(x + 0)));    /* VMADD */
            wr16(LONG(0x004302), rd16(ABS(x + 5)));    /* A1T0 */
            wr16(LONG(0x004304), rd16(ABS(x + 7)));    /* A1B0 (and DAS0L, rewritten next) */
            wr16(LONG(0x004305), rd16(ABS(x + 8)));    /* DAS0 */
            sep(0x20);
            wr8(LONG(0x002115), rd8(ABS(x + 2)));      /* VMAIN */
            wr8(LONG(0x00420B), 0x01);                 /* MDMAEN: go */
            rep(0x20);
            x = rd16(ABS(x + 0x0A));                   /* next record */
            A = x;
        } while (x != rd16(DP(0x04)));
        X = x;
        A = 0x0002;
        wr16(ABS(0x0000), 0x0002);                     /* queue empty */
    }
    DB = pull8();
    sep(0x30);
    DB = pull8();
    retl();
}

void DmaKick_m0x1(void);

/* One VRAM upload through channel 0 (D = $4300): DmaKick with the return
 * address the original JSR pushes. */
static void vram_upload(u16 jsr_ret, u16 vram, u16 bytes, u16 src, u8 bank)
{
    wr16(ABS(0x2116), vram);                           /* VMADD */
    wr16(DP(0x05), bytes);                             /* DAS0 */
    A = src;
    X = bank;
    Y = 1;                                             /* channel 0 */
    push16(jsr_ret);
    DmaKick_m0x1();
}

/*
 * $80:876B NmiFixedUploads (M=1, X=1, RTS), in vblank, with the direct page
 * moved to the DMA registers ($4300) so DP $00-$06 are channel 0.
 *
 *  - four streamed sprite-graphics slots (when $1E12 != 0): source words
 *    $1E12/$1E14/$1E16/$1E18, banks $1543-$1546, each 2 x $100 bytes to
 *    VRAM $2000/$2100, $2080/$2180, $2200/$2300, $2280/$2380 (two tile rows);
 *  - $200 bytes from $9E:($86) to VRAM $4A00 when $86 != 0;
 *  - the whole palette from $00:1C00 when $1915 is set (then cleared);
 *  - $200 bytes from $8A:($1E0A) to VRAM ($1E10) when $1E0C and $1E10 are set;
 *  - OAM: $220 bytes from $00:1600.
 */
void NmiFixedUploads_m1x1(void)
{
    push16(D);
    A = 0x4300; D = 0x4300;
    rep(0x10);
    wr8(ABS(0x2115), 0x80);                            /* VMAIN: +1 after high byte */
    Y = 0x1801;
    wr16(DP(0x00), 0x1801);                            /* DMAP0 = $01, BBAD0 = $18 (VMDATA) */
    rep(0x20);
    sep(0x10);

    if (rd16(ABS(0x1E12)) != 0) {
        static const struct { u16 ret1, ret2, vram, src; u16 bank; } slot[4] = {
            {0x879F, 0x87B4, 0x2000, 0x1E12, 0x1543},
            {0x87CA, 0x87DF, 0x2080, 0x1E14, 0x1544},
            {0x87F5, 0x880A, 0x2200, 0x1E16, 0x1545},
            {0x8820, 0x8835, 0x2280, 0x1E18, 0x1546},
        };
        for (int i = 0; i < 4; i++) {
            u8 bank = rd8(ABS(slot[i].bank));
            vram_upload(slot[i].ret1, slot[i].vram, 0x0100, rd16(ABS(slot[i].src)), bank);
            /* second half: +$100 VRAM words, +$200 source bytes (CLC / ADC) */
            wr16(ABS(0x2116), (u16)(slot[i].vram + 0x100));
            wr16(DP(0x05), 0x0100);
            A = rd16(ABS(slot[i].src));
            fc = 0;
            adc16(0x0200);
            push16(slot[i].ret2);
            DmaKick_m0x1();
        }
    }
    rep(0x20);
    sep(0x10);
    if (rd16(ABS(0x0086)) != 0)
        vram_upload(0x8853, 0x4A00, 0x0200, rd16(ABS(0x0086)), 0x9E);

    rep(0x10);
    sep(0x20);
    if (rd8(ABS(0x1915)) != 0) {                        /* palette upload request */
        wr8(ABS(0x2121), 0);                           /* CGADD */
        Y = 0x2200; wr16(DP(0x00), 0x2200);            /* DMAP0 = $00, BBAD0 = $22 (CGDATA) */
        Y = 0x1C00; wr16(DP(0x02), 0x1C00);
        SET_A8(0);  wr8(DP(0x04), 0x00);
        Y = 0x0200; wr16(DP(0x05), 0x0200);
        SET_A8(1);  wr8(ABS(0x420B), 0x01);
        wr8(ABS(0x1915), 0);
    }
    if (rd8(ABS(0x1E0C)) != 0) {
        rep(0x20);
        sep(0x10);
        if (rd16(ABS(0x1E10)) != 0) {
            Y = 0x80; wr8(ABS(0x2115), 0x80);
            A = 0x1801; wr16(DP(0x00), 0x1801);
            vram_upload(0x88A7, rd16(ABS(0x1E10)), 0x0200, rd16(ABS(0x1E0A)), 0x8A);
        }
    }
    rep(0x20);
    sep(0x10);
    wr16(ABS(0x2102), 0);                              /* OAMADD = 0 */
    wr16(DP(0x00), 0x0400);                            /* DMAP0 = $00, BBAD0 = $04 (OAMDATA) */
    wr16(DP(0x02), 0x1600);
    wr16(DP(0x04), 0x0000);
    wr16(DP(0x05), 0x0220);
    Y = 1; wr8(ABS(0x420B), 0x01);
    A = 0x1801; wr16(DP(0x00), 0x1801);                /* leave channel 0 set up for VRAM */
    sep(0x30);
    D = pull16();
    ret();
}
