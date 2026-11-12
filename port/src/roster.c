/*
 * roster.c - the port's character roster for the Gen B engine.
 *
 * The hand-written Gen B routines (c/hand/s4) get every per-character value
 * from here instead of the original fixed tables. Characters 0-4 answer
 * exactly what the ROM tables say (Mario, Yoshi, Kinopio, Wario, Peach), so
 * the game is unchanged until characters are added.
 */
#include "runtime816.h"
#include "roster.h"

int roster_count(void) { return 5; }

u16 roster_rider_offset(int ch, int var, int slotx)
{
    return rd16(LONG(0x820000 | ((var ? 0x8918 : 0x88F0) + ((ch * 8 + slotx) & 0xFF))));
}

u8 roster_rider_bank(int ch, int var, int slot)
{
    return rd8(LONG(0x820000 | ((var ? 0x88DC : 0x88C8) + ((ch * 4 + slot) & 0xFF))));
}

u32 roster_rider_palette(int st, int var, int ch, int i)
{
    u16 x = (u16)(((st ? 100 : 0) + (var ? 50 : 0) + ch * 10) & 0xFF) + i * 2;
    return 0x820000 | rd16(LONG(0x820000 | (0x8A08 + x)));
}
