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
/* Rider animation set of sprite slot s in a race of character ch: word
 * offset (frame base; slotx = s * 2, the byte index the original uses in
 * its 8-byte record) and ROM bank ($82:88F0/$82:8918, $82:88C8/$82:88DC). */
u16 roster_rider_offset(int ch, int var, int slotx);
u8 roster_rider_bank(int ch, int var, int slot);
/* 24-bit address of rider palette i (0-4, 16 colours) for character ch
 * ($82:8A08 + st * 100 + var * 50 + ch * 10 + i * 2, bank $82). */
u32 roster_rider_palette(int st, int var, int ch, int i);

#endif
