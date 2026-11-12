/*
 * roster_rom.c - the roster as the original ROM tables define it (five
 * characters). Linked by tools/difftest816.py; the port uses
 * port/src/roster.c instead.
 */
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

u32 roster_name(int st, int ch, u16 orig)
{
    (void)st; (void)ch;
    return 0x800000 | orig;
}

u8 roster_winner_pose(int ch, int rider)
{
    return rd8(LONG(0x810000 | (0xC158 + ((ch * 8 + rider) & 0xFF))));
}
