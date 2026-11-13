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

u32 roster_voice_stream(int ch) { (void)ch; return 0; }

u8 roster_select_ring_offset(int i) { return rd8(LONG(0x808F64 + i)); }

int roster_select_is_stop(u8 a0)
{
    return a0 == 0x00 || a0 == 0x34 || a0 == 0x66 || a0 == 0x9A || a0 == 0xCC;
}

u8 roster_select_cursor(u8 a0)
{
    for (int y = 0; y < 5; y++)
        if (a0 >= rd8(ABS(0x8CEC + y)))            /* DB-relative, as the original */
            return (u8)y;
    return 0;
}

u8 roster_select_oam(u8 cursor, int i) { return rd8(ABS(0x8CF1 + (u8)(cursor * 5 + i))); }

void roster_select_portrait(int st, int i, u8 *tile, u8 *nt, u8 *selpal)
{
    *tile = rd8(LONG((st ? 0x808D19 : 0x808D0A) + i));
    *nt = rd8(LONG((st ? 0x808D23 : 0x808D14) + i));
    *selpal = rd8(LONG((st ? 0x808D1E : 0x808D0F) + i));
}

u8 roster_select_name_row(int st, int i) { return i == 3 ? rd8(LONG(0x808E46 + (st ? 1 : 0))) : (u8)i; }

u8 roster_select_char(u8 cursor) { return rd8(LONG(0x808B21 + cursor)); }

int roster_select_general(void) { return 0; }

u8 roster_engine_char(int ch) { return (u8)ch; }
void roster_note_choice(int ch) { (void)ch; }

int port_hide_clock(void) { return 0; }
int port_race_is_final(void) { return rd8(DP(0x7D)) != 0; }
int port_rival_pace(void) { return 100; }
void port_ws_rider_sprites(void) { }
