/*
 * roster.h - per-character data used by the Gen B (Stadium 3-4) routines.
 *
 * The original keeps one fixed table per kind of data, indexed by the
 * character ($154F) times the record size, so it cannot grow. The
 * hand-written routines ask these functions instead. roster_rom.c answers
 * from the original tables (used by tools/difftest816.py, so the routines
 * are checked against the original byte for byte); the port links
 * port/src/roster.c, which answers the same for the five original
 * characters and adds more (Luigi from the Stadium 3 dump).
 *
 *   ch      character: 0 Mario, 1 Yoshi, 2 Kinopio, 3 Wario, 4 Peach, 5+ added
 *   var     race variant, $1E91 (0/1, picked at random at the player select)
 *   slot    rider sprite slot 0-3 (0 = the player)
 *   st      Stadium: $81:800C (0 = Stadium 3, 1 = Stadium 4)
 */
#ifndef ROSTER_H
#define ROSTER_H
#include "runtime816.h"

/* Number of selectable characters. */
int roster_count(void);
/* The engine keeps an added character on the index of the original one it
 * plays like (Luigi rides as character 1, Yoshi's slot), so every routine
 * that still indexes the original tables keeps working; the roster
 * remembers which character it really is in a byte of free work RAM
 * ($15FE, unused by the original). roster_engine_char gives the index for
 * $154F; roster_note_choice records the choice. The roster functions below
 * take $154F as read and resolve the real character themselves. */
u8 roster_engine_char(int ch);
void roster_note_choice(int ch);
/* Rider animation set of sprite slot s in a race of character ch: word
 * offset (frame base; slotx = s * 2, the byte index the original uses in
 * its 8-byte record) and ROM bank ($82:88F0/$82:8918, $82:88C8/$82:88DC). */
u16 roster_rider_offset(int ch, int var, int slotx);
u8 roster_rider_bank(int ch, int var, int slot);
/* 24-bit address of rider palette i (0-4, 16 colours) for character ch
 * ($82:8A08 + st * 100 + var * 50 + ch * 10 + i * 2, bank $82). */
u32 roster_rider_palette(int st, int var, int ch, int i);

/* 24-bit address of the racer name, a 15-byte tile list ($80:A598 +
 * ch * 15, Stadium 4 + $4B; first byte = size, then the tile numbers).
 * orig is the bank $80 pointer the original computed (it uses the hardware
 * multiplier); the original characters keep it. */
u32 roster_name(int st, int ch, u16 orig);
/* Pose of the winning rider at the end of a race of character ch
 * ($81:C158 + ch * 8 + rider). */
u8 roster_winner_pose(int ch, int rider);

/* Voice sample stream uploaded when ch is chosen: 0 = the original
 * ($82:8593 with X = ch + 1), else the 24-bit address of the stream. */
u32 roster_voice_stream(int ch);

/* PLAYER SELECT carousel: portrait i (0..roster_count()-1) on the ring. */
u8 roster_select_ring_offset(int i);           /* start angle ($80:8F64) */
int roster_select_is_stop(u8 a0);               /* ring stops here (portrait 0's angle) */
u8 roster_select_cursor(u8 a0);                 /* front position from a0 ($80:8CEC) */
u8 roster_select_oam(u8 cursor, int i);         /* OAM offset, front first ($80:8CF1) */
void roster_select_portrait(int st, int i, u8 *tile, u8 *nt, u8 *selpal);
u8 roster_select_name_row(int st, int i);       /* name sprite row ($80:8E46 for i = 3) */
u8 roster_select_char(u8 cursor);               /* character in front ($80:8B21) */
/* 1 when the carousel is laid out by the roster (more than the original
 * five): every portrait's angle then follows portrait 0's. */
int roster_select_general(void);

/* Port options the hand-written routines consult (0 in the original). */
int port_hide_clock(void);                      /* draw the broadcast clock blank */
/* The race just run was the final ($7D). The port can run any race with the
 * final's seven CPU rivals; such a race still ends like the free run it is. */
int port_race_is_final(void);
/* Percent applied to a CPU rival's target speed ($80:BE66); 100 original. */
int port_rival_pace(void);

#endif
