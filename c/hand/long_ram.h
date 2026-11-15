/*
 * long_ram.h - the long (24-bit) addresses every Stadium uses alike: the
 * work RAM in bank $7F, the broadcast clock, the save block and two ROM
 * switches. They are the same in all four dumps, so each <ver>/ram.h
 * includes this file. Used as LONG(BCAST_MINUTE), LONGX(VQ_DATA).
 *
 * A comment that starts with [X] marks a name for indexed operands only
 * (LONGX): the field of a record at base + X. tools/ramname.py keeps such a
 * name off the plain operand at the same address.
 */
#ifndef HAND_LONG_RAM_H
#define HAND_LONG_RAM_H

/* ---- the VRAM upload queue ($7F:0000), flushed by NmiFlushVramQueue ----
 * $7F:0000 holds the offset of the queue's end. The records follow from
 * offset 2, each: VRAM address, VMAIN, DMAP, BBAD, 3-byte source, byte count,
 * offset of the next record, then the data (usually the source itself). */
#define VQ_END          0x7F0000  /* word: queue end offset ($0002 empty, $FFFF being written) */
#define VQ_VMADD        0x7F0000  /* [X] word: VRAM word address */
#define VQ_VMADD_HI     0x7F0001  /* [X] its high byte */
#define VQ_VMAIN        0x7F0002  /* [X] VMAIN ($80: step after the high byte) */
#define VQ_DMAP         0x7F0003  /* [X] DMA mode ($01: two registers) */
#define VQ_BBAD         0x7F0004  /* [X] B-bus register ($18 = VMDATAL) */
#define VQ_SRC          0x7F0005  /* [X] word: source address */
#define VQ_SRC_BANK     0x7F0007  /* [X] source bank */
#define VQ_SIZE         0x7F0008  /* [X] byte count, low */
#define VQ_SIZE_HI      0x7F0009  /* [X] byte count, high */
#define VQ_NEXT         0x7F000A  /* [X] word: offset of the next record */
#define VQ_DATA         0x7F000C  /* [X] the record's data (X = record offset + byte) */
#define VQ_DATA_HI      0x7F000D  /* [X] the data word's high byte */
/* the first record (at offset 2), written in place by the track column code */
#define VQ0_VMADD       0x7F0002
#define VQ0_VMAIN       0x7F0004
#define VQ0_DMAP        0x7F0005
#define VQ0_BBAD        0x7F0006
#define VQ0_SRC         0x7F0007
#define VQ0_SRC_HI      0x7F0008
#define VQ0_SRC_BANK    0x7F0009
#define VQ0_SIZE        0x7F000A
#define VQ0_NEXT        0x7F000C
#define VQ0_DATA        0x7F000E  /* [+$40] the first record's data (indexed by byte) */

/* ---- the track ---- */
#define TRACK_BUF       0x7F4000  /* [+$2000] the course expanded to one byte per column ($81:A01B) */
#define TRACK_COL_INDEX 0x7F6000  /* words: the column index built from TRACK_BUF */

/* ---- the broadcast clock ($7F:FFE0-$7F:FFFF; BsxPollStream copies the
 * satellite clock here, or the game ticks it itself without a receiver) ---- */
#define NO_RECEIVER     0x7FFFE1  /* BsxDetect: 1 = no Satellaview receiver */
#define BSX_CLOCK_0     0x7FFFF0  /* the stream's clock bytes (the save keeps a copy) */
#define BSX_CLOCK_1     0x7FFFF1
#define BSX_CLOCK_2     0x7FFFF2
#define BSX_CLOCK_3     0x7FFFF3  /* low 3 bits: the weekday */
#define BS_TIME_CHAN    0x7FFFF7  /* word: the opened time channel */
#define BCAST_HOUR      0x7FFFF9  /* the broadcast's hour (reset: 19) */
#define BCAST_FRAME     0x7FFFFA  /* word: frames into the minute (3660 a minute) */
#define BCAST_FRAME_HI  0x7FFFFB  /* its high byte */
#define BSX_SYNCED      0x7FFFFC  /* word: the clock was set from the stream */
#define BCAST_MINUTE    0x7FFFFF  /* the broadcast's minute, 0-60 (the race opens at 4) */

/* ---- the save block (bank $10-$12, $5000-$5FFF; checked by SaveHeaderCheck) ---- */
#define SAVE_SUM_INV    0x105002  /* word: the checksum's complement */
#define SAVE_SUM        0x105004  /* word: 16-bit sum of the three banks' $5000-$5FFF */
#define SAVE_TITLE      0x115700  /* [+$10] 16 bytes: the ROM title, a copy of $00:FFC0 */
#define SAVE_CLOCK      0x115710  /* 4 bytes: BSX_CLOCK_0-3 when saved */
#define SAVE_CLOCK_HI   0x115712
#define SAVE_HOUR       0x115714  /* word: the hour when saved */
#define SAVE_STADIUM    0x115716  /* word: STADIUM_SELECT when saved */
#define SAVE_DAYS       0x115718  /* [X] by weekday: the day's entry (Gen A: two set unlock Wario) */
#define SAVE_TIMES      0x115720  /* [+$C] 4 x 3 bytes: the best run's course times */
#define SAVE_BEST_TIME  0x11572C  /* the best total time: minutes */
#define SAVE_BEST_SEC   0x11572D  /* seconds */
#define SAVE_BEST_FRAC  0x11572E  /* hundredths */
#define SAVE_BEST_SCORE 0x115730  /* [+$5] 5 digits, the high one first */

/* ---- ROM ---- */
#define ROM_TITLE       0x00FFC0  /* [X] the cartridge header's title */
#define STADIUM_SELECT  0x81800C  /* 0 = the first Stadium of the build, 1 = the second */
#define DEBUG_ENABLED   0x81800E  /* debug keys: minute step, pause, frame advance */

#endif
