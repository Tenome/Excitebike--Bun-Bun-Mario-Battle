/*
 * roster.c - the port's character roster for the Gen B engine.
 *
 * The hand-written Gen B routines (c/hand/s4) get every per-character value
 * from here instead of the original fixed tables (roster.h). At start-up the
 * five original characters are read from the ROM's own tables, so they
 * behave exactly as before, and characters the original does not have are
 * added after them:
 *
 *   0 Mario  1 Yoshi  2 Kinopio  3 Wario  4 Peach   (the Stadium 4 build)
 *   5 Luigi  (Stadium 3: the S4 build still holds his name, palette and voice
 *             numbers; his rider sprites and voice samples come from the
 *             Stadium 3 dump in data/s3.sfc)
 *
 * The cartridge image is extended to 4 MB for the added data: the original
 * 1 MB is mirrored into every MB (so mirrored bank accesses read what they
 * always did), and banks $A0-$BF then receive the new data.
 */
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>
#include "runtime816.h"
#include "roster.h"
#include "bunbun_data.h"

#define MAX_CH 16

typedef struct {
    const char *label;
    u8  bank[2][4];                 /* rider set per race variant and sprite slot */
    u16 off[2][4];
    u32 pal[2][2][5];               /* [Stadium][variant][palette] */
    u32 name;                       /* 15-byte tile list */
    u8  pose[8];                    /* winner figure per rider */
} Character;

static Character g_ch[MAX_CH];
static int g_count = 5;
static int g_ready;

/* Extended image (set up by RosterInit). */
#define EXT_LUIGI_SET    0xA08000u  /* 16 KB rider set (S3 $9A:8000) */
#define EXT_LUIGI_VOICE  0xA18000u  /* voice sample stream (S3 $9D:8000) */

static uint8_t *s_rom;
static size_t s_rom_size;

static u8 rom8(u32 a) { return s_rom[(((a >> 16) & 0x7F) << 15 | (a & 0x7FFF)) % s_rom_size]; }
static u16 rom16(u32 a) { return (u16)(rom8(a) | rom8(a + 1) << 8); }

static void from_rom(int c)
{
    Character *h = &g_ch[c];
    for (int v = 0; v < 2; v++)
        for (int s = 0; s < 4; s++) {
            h->bank[v][s] = rom8(0x820000 | ((v ? 0x88DC : 0x88C8) + ((c * 4 + s) & 0xFF)));
            h->off[v][s] = rom16(0x820000 | ((v ? 0x8918 : 0x88F0) + ((c * 8 + s * 2) & 0xFF)));
        }
    for (int st = 0; st < 2; st++)
        for (int v = 0; v < 2; v++)
            for (int i = 0; i < 5; i++) {
                u16 x = (u16)(((st ? 100 : 0) + (v ? 50 : 0) + c * 10) & 0xFF) + i * 2;
                h->pal[st][v][i] = 0x820000 | rom16(0x820000 | (0x8A08 + x));
            }
    h->name = 0x800000 | (u16)(0xA598 + c * 15 + 0x4B);   /* Stadium 4 names */
    for (int r = 0; r < 8; r++)
        h->pose[r] = rom8(0x810000 | (0xC158 + ((c * 8 + r) & 0xFF)));
}

/* Grow the cartridge image to 4 MB (mirrors of the original) once. */
#include "snes/snes.h"
#include "snes/cart.h"
#include "common_cpu_infra.h"
static int extend_image(void)
{
    Cart *cart = g_snes ? g_snes->cart : NULL;
    if (!cart || !cart->rom)
        return 0;
    if (cart->romSize >= 0x400000)
        return 1;
    size_t old = cart->romSize;
    uint8_t *big = malloc(0x400000);
    if (!big)
        return 0;
    for (size_t at = 0; at < 0x400000; at += old)
        memcpy(big + at, cart->rom, old);
    free(cart->rom);                /* cart_load allocated it with malloc */
    cart->rom = big;
    cart->romSize = 0x400000;
    return 1;
}

static void put(u32 snes, const uint8_t *src, size_t n)
{
    size_t off = ((snes >> 16) & 0x7F) << 15 | (snes & 0x7FFF);
    if (off + n <= s_rom_size)
        memcpy(s_rom + off, src, n);
}

static void add_luigi(void)
{
    uint8_t *s3 = bunbun_data_load_dump("s3.sfc");
    if (!s3) {
        SDL_Log("[roster] data/s3.sfc not found: Luigi is not available");
        return;
    }
    put(EXT_LUIGI_SET, s3 + (((0x9A8000 >> 16) & 0x7F) << 15), 0x4000);    /* $9A:8000 */
    put(EXT_LUIGI_VOICE, s3 + (((0x9D8000 >> 16) & 0x7F) << 15), 0x3000);  /* $9D:8000 */
    SDL_free(s3);

    /* Luigi takes Yoshi's place in Yoshi's own races: same lineup, his set,
     * his Stadium 3 palette ($82:8AF0) and name (the Stadium 3 name table). */
    Character *l = &g_ch[5];
    *l = g_ch[1];
    l->label = "Luigi";
    for (int v = 0; v < 2; v++) {
        l->bank[v][0] = (u8)(EXT_LUIGI_SET >> 16);
        l->off[v][0] = (u16)EXT_LUIGI_SET;
    }
    for (int st = 0; st < 2; st++)
        for (int v = 0; v < 2; v++)
            l->pal[st][v][0] = 0x828AF0;
    l->name = 0x800000 | (u16)(0xA598 + 1 * 15);
    g_count = 6;
    SDL_Log("[roster] Luigi added (character 5)");
}

void RosterInit(void)
{
    if (g_ready)
        return;
    if (!extend_image())
        return;
    s_rom = g_snes->cart->rom;
    s_rom_size = g_snes->cart->romSize;
    static const char *labels[5] = { "Mario", "Yoshi", "Kinopio", "Wario", "Peach" };
    for (int c = 0; c < 5; c++) {
        from_rom(c);
        g_ch[c].label = labels[c];
    }
    add_luigi();
    g_ready = 1;
}

int roster_count(void) { return g_count; }

/* Characters 0-4 answer from the ROM as the original indexes it (also for
 * out-of-range indexes the original would read), added ones from the roster. */
u16 roster_rider_offset(int ch, int var, int slotx)
{
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].off[var ? 1 : 0][(slotx >> 1) & 3];
    return rd16(LONG(0x820000 | ((var ? 0x8918 : 0x88F0) + ((ch * 8 + slotx) & 0xFF))));
}

u8 roster_rider_bank(int ch, int var, int slot)
{
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].bank[var ? 1 : 0][slot & 3];
    return rd8(LONG(0x820000 | ((var ? 0x88DC : 0x88C8) + ((ch * 4 + slot) & 0xFF))));
}

u32 roster_rider_palette(int st, int var, int ch, int i)
{
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].pal[st ? 1 : 0][var ? 1 : 0][i % 5];
    u16 x = (u16)(((st ? 100 : 0) + (var ? 50 : 0) + ch * 10) & 0xFF) + i * 2;
    return 0x820000 | rd16(LONG(0x820000 | (0x8A08 + x)));
}

u32 roster_name(int st, int ch, u16 orig)
{
    (void)st;
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].name;
    return 0x800000 | orig;
}

u8 roster_winner_pose(int ch, int rider)
{
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].pose[rider & 7];
    return rd8(LONG(0x810000 | (0xC158 + ((ch * 8 + rider) & 0xFF))));
}
