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
#include "bunbun_settings.h"

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
    if (bunbun_option("BUNBUN_LUIGI", "game.luigi", 1))   /* launcher: Game > Expanded roster */
        add_luigi();
    g_ready = 1;
}

int roster_count(void) { return g_count; }

/* ---- added characters ride on an original index ---------------------------- */
#define ALT_CHAR 0x7E15FE           /* free work RAM: the real character, 0 = none */
static const u8 kEngineChar[MAX_CH] = { 0, 1, 2, 3, 4, 1 };   /* Luigi rides as Yoshi */

u8 roster_engine_char(int ch) { return ch >= 0 && ch < g_count ? kEngineChar[ch] : (u8)ch; }

void roster_note_choice(int ch) { wr8(LONG(ALT_CHAR), (u8)(ch >= 5 ? ch : 0)); }

/* $154F as the routines read it -> the roster character */
static int real_char(int ch)
{
    u8 alt = rd8(LONG(ALT_CHAR));
    if (alt >= 5 && alt < g_count && kEngineChar[alt] == ch)
        return alt;
    return ch;
}

/* Characters 0-4 answer from the ROM as the original indexes it (also for
 * out-of-range indexes the original would read), added ones from the roster. */
u16 roster_rider_offset(int ch, int var, int slotx)
{
    ch = real_char(ch);
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].off[var ? 1 : 0][(slotx >> 1) & 3];
    return rd16(LONG(0x820000 | ((var ? 0x8918 : 0x88F0) + ((ch * 8 + slotx) & 0xFF))));
}

u8 roster_rider_bank(int ch, int var, int slot)
{
    ch = real_char(ch);
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].bank[var ? 1 : 0][slot & 3];
    return rd8(LONG(0x820000 | ((var ? 0x88DC : 0x88C8) + ((ch * 4 + slot) & 0xFF))));
}

u32 roster_rider_palette(int st, int var, int ch, int i)
{
    ch = real_char(ch);
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].pal[st ? 1 : 0][var ? 1 : 0][i % 5];
    u16 x = (u16)(((st ? 100 : 0) + (var ? 50 : 0) + ch * 10) & 0xFF) + i * 2;
    return 0x820000 | rd16(LONG(0x820000 | (0x8A08 + x)));
}

u32 roster_name(int st, int ch, u16 orig)
{
    ch = real_char(ch);
    (void)st;
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].name;
    return 0x800000 | orig;
}

u8 roster_winner_pose(int ch, int rider)
{
    ch = real_char(ch);
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].pose[rider & 7];
    return rd8(LONG(0x810000 | (0xC158 + ((ch * 8 + rider) & 0xFF))));
}

/* ---- PLAYER SELECT carousel ------------------------------------------------- */
/* With the original five characters the carousel follows the ROM tables
 * exactly. With added characters it is laid out here: portraits 0-4 are the
 * Stadium 4 ones (Mario, Wario, Kinopio, Yoshi, Peach), 5 is Luigi with the
 * Stadium 3 portrait the S4 build keeps (tile $60 in the second sprite name
 * table, palette $2C, name row 5). Ring order and stop angles are spaced
 * evenly (steps of 2, so every stop is even). */
typedef struct { u8 ch, tile, nt, selpal, name_row; } Portrait;
static const Portrait kPortraits[6] = {
    { 0, 0x40, 0, 0x20, 0 },      /* Mario */
    { 3, 0xA6, 0, 0x26, 1 },      /* Wario */
    { 2, 0xA0, 0, 0x24, 2 },      /* Kinopio */
    { 1, 0x46, 0, 0x22, 3 },      /* Yoshi (Stadium 4 portrait) */
    { 4, 0x00, 1, 0x2A, 4 },      /* Peach */
    { 5, 0x60, 1, 0x2C, 5 },      /* Luigi (Stadium 3 portrait) */
};
/* ring position k -> portrait, and each portrait's ring offset */
static const u8 kRing[6] = { 0, 1, 2, 4, 3, 5 };
static u8 ring_stop(int k) { return (u8)(((k * 256 / 6) + 1) & ~1); }   /* 0,42,86,128,170,214 */

static int general(void) { return g_count > 5; }

u32 roster_voice_stream(int ch) { return ch == 5 && g_count > 5 ? EXT_LUIGI_VOICE : 0; }

u8 roster_select_ring_offset(int i)
{
    if (!general())
        return rd8(LONG(0x808F64 + i));
    for (int k = 0; k < 6; k++)
        if (kRing[k] == i)
            return ring_stop(k);
    return 0;
}

int roster_select_is_stop(u8 a0)
{
    if (!general())
        return a0 == 0x00 || a0 == 0x34 || a0 == 0x66 || a0 == 0x9A || a0 == 0xCC;
    for (int k = 0; k < 6; k++)
        if ((u8)(0x100 - ring_stop(k)) == a0)
            return 1;
    return 0;
}

static int angle_dist(u8 a) { return a < 128 ? a : 256 - a; }

/* the portrait whose angle is closest to the front (0) */
u8 roster_select_cursor(u8 a0)
{
    if (!general()) {
        for (int y = 0; y < 5; y++)
            if (a0 >= rd8(ABS(0x8CEC + y)))
                return (u8)y;
        return 0;
    }
    int best = 0, bd = 999;
    for (int k = 0; k < 6; k++) {
        int d = angle_dist((u8)(a0 + ring_stop(k)));
        if (d < bd) { bd = d; best = k; }
    }
    return (u8)best;
}

u8 roster_select_oam(u8 cursor, int i)
{
    if (!general())
        return rd8(ABS(0x8CF1 + (u8)(cursor * 5 + i)));
    /* front first: rank by distance from the front */
    u8 a0 = rd8(ABS(0x1554));
    int di = angle_dist((u8)(a0 + roster_select_ring_offset(i)));
    int rank = 0;
    for (int j = 0; j < 6; j++) {
        int dj = angle_dist((u8)(a0 + roster_select_ring_offset(j)));
        if (dj < di || (dj == di && j < i))
            rank++;
    }
    return (u8)(rank * 36);
}

void roster_select_portrait(int st, int i, u8 *tile, u8 *nt, u8 *selpal)
{
    if (!general()) {
        *tile = rd8(LONG((st ? 0x808D19 : 0x808D0A) + i));
        *nt = rd8(LONG((st ? 0x808D23 : 0x808D14) + i));
        *selpal = rd8(LONG((st ? 0x808D1E : 0x808D0F) + i));
        return;
    }
    *tile = kPortraits[i % 6].tile;
    *nt = kPortraits[i % 6].nt;
    *selpal = kPortraits[i % 6].selpal;
}

u8 roster_select_name_row(int st, int i)
{
    if (!general())
        return i == 3 ? rd8(LONG(0x808E46 + (st ? 1 : 0))) : (u8)i;
    return kPortraits[i % 6].name_row;
}

u8 roster_select_char(u8 cursor)
{
    if (!general())
        return rd8(LONG(0x808B21 + cursor));
    return kPortraits[kRing[cursor % 6]].ch;
}

int roster_select_general(void) { return general(); }

/* The broadcast clock is hidden when the game plays without it (launcher:
 * Game > Game flow). */
int port_hide_clock(void)
{
    static int v = -1;
    if (v < 0) v = strcmp(bunbun_setting("game.flow", "menu"), "broadcast") != 0;
    return v;
}

/* CPU rivals in every race (launcher: Game > CPU rivals). The engine has
 * eight rider slots, and the free run uses only the player's: from the
 * course preview of a free run to the end of that race the final's flag $7D
 * is set, which brings in the seven rivals. The race still ends like the free
 * run it is: port_race_is_final sends it to its results, and its time-up
 * minute (table $80:C468, entry $7C * 2 + $7D) is lent to the final's entry
 * for the course while it runs. The flag is cleared before the results. */
static int g_rivals_forced;
static int g_rivals_slot = -1;
static uint8_t g_rivals_saved;
extern uint8_t g_ram[];                 /* WRAM (common_rtl.h) */

int port_race_is_final(void)
{
    return g_ram[0x7D] != 0 && !g_rivals_forced;
}

static uint8_t *end_minutes(void)       /* $80:C468 in the cartridge image */
{
    if (!g_snes || !g_snes->cart || !g_snes->cart->rom || g_snes->cart->romSize < 0x4480)
        return NULL;
    return g_snes->cart->rom + 0x4468;
}

/* Rival pace (launcher: Game > Rival pace): a percentage on every CPU
 * rival's target speed, which RivalTarget ($80:BE66) sets and the riders
 * accelerate toward. */
int port_rival_pace(void)
{
    static int pct = -1;
    if (pct < 0) {
        const char *p = bunbun_setting("game.pace", "normal");
        pct = strcmp(p, "easy") == 0 ? 85 : strcmp(p, "hard") == 0 ? 115 : 100;
    }
    return pct;
}

void RivalsAfterFrame(void)
{
    static int on = -1;
    if (on < 0) on = strcmp(bunbun_setting("game.rivals", "all"), "all") == 0;
    uint8_t mode = g_ram[0x76];
    if (g_rivals_forced) {
        if (mode == 0x18 || mode == 0x03) {
            g_ram[0x7D] = 1;
            return;
        }
        g_ram[0x7D] = 0;
        g_rivals_forced = 0;
        uint8_t *t = end_minutes();
        if (t && g_rivals_slot >= 0)
            t[g_rivals_slot] = g_rivals_saved;
        g_rivals_slot = -1;
    } else if (on && mode == 0x18 && g_ram[0x7D] == 0 && g_ram[0x0CD7] == 0) {
        uint8_t *t = end_minutes();
        int c = g_ram[0x7C];
        if (!t || c > 7)
            return;
        g_rivals_slot = c * 2 + 1;
        g_rivals_saved = t[g_rivals_slot];
        t[g_rivals_slot] = t[c * 2];
        g_rivals_forced = 1;
        g_ram[0x7D] = 1;
    }
}

/* Widescreen riders (src/widescreen_bunbun.c). A rider's screen X is
 * ($019E - 1) * 256 + $0196: page 0 left of the original screen, 1 on it,
 * 2 right of it. DrawBigSprite wrote its four 16x16 sprites at $1600 + $07
 * (columns X and X + 16, rows Y and Y + 16) with the low byte of X, and
 * PlaceRiderShadow hid the ones off page 1. Inside the wide view, give each
 * column its X9 bit and put the rows back. */
int BunbunWsExtra(void);
void port_ws_rider_sprites(void)
{
    int extra = BunbunWsExtra();
    if (!extra || g_ram[0x76] != 0x03)
        return;
    unsigned r = X & 0xFF;
    int x = ((int)g_ram[0x019E + r] - 1) * 256 + g_ram[0x0196 + r];
    if (x <= -32 - extra || x >= 256 + extra)
        return;                                        /* outside the wide view */
    if (x >= 0 && x <= 256 - 32)
        return;                                        /* the original handles it */
    unsigned o = g_ram[0x07];
    uint8_t y = (uint8_t)(g_ram[0x01B6 + r] - 8);
    g_ram[0x1601 + o] = y;       g_ram[0x1605 + o] = y;
    g_ram[0x1609 + o] = (uint8_t)(y + 16); g_ram[0x160D + o] = (uint8_t)(y + 16);
    unsigned k = o >> 2;
    uint8_t x9a = (uint8_t)((x >> 8) & 1), x9b = (uint8_t)(((x + 16) >> 8) & 1);
    g_ram[0x1820 + k] = (uint8_t)(2 | x9a); g_ram[0x1822 + k] = (uint8_t)(2 | x9a);
    g_ram[0x1821 + k] = (uint8_t)(2 | x9b); g_ram[0x1823 + k] = (uint8_t)(2 | x9b);
}
