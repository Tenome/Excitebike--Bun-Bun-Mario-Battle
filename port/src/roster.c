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
    u8  voice[2][2][8];             /* crash voice per rider [Stadium][variant] */
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
    for (int st = 0; st < 2; st++)
        for (int v = 0; v < 2; v++)
            for (int r = 0; r < 8; r++)
                h->voice[st][v][r] = rom8(0x800000 | (0xC8E9 + ((st * 0x50 + v * 0x28 + c * 8 + r) & 0xFF)));
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

/* SPC upload streams ([len16][dest16][data]..., len 0 ends) in a LoROM
 * image; the reader steps to the next bank at $8000 like the uploader. */
static size_t lorom_off(u32 a) { return ((a >> 16) & 0x7F) << 15 | (a & 0x7FFF); }
static u32 lorom_next(u32 a) { a++; return (a & 0xFFFF) ? a : ((a & 0xFF0000) + 0x18000); }
typedef struct { u16 dest, len; u32 at; } SpcChunk;
static int spc_chunks(const uint8_t *img, size_t size, u32 a, SpcChunk *out, int max)
{
    int n = 0;
    for (;;) {
        u8 b[4];
        for (int i = 0; i < 4; i++) {
            if (lorom_off(a) >= size) return n;
            b[i] = img[lorom_off(a)]; a = lorom_next(a);
        }
        u16 len = (u16)(b[0] | b[1] << 8);
        if (!len || n >= max) return n;
        out[n].len = len; out[n].dest = (u16)(b[2] | b[3] << 8); out[n].at = a;
        n++;
        for (u32 i = 0; i < len; i++) a = lorom_next(a);
    }
}
static void spc_copy(uint8_t *dst, const uint8_t *img, u32 a, size_t n)
{
    for (size_t i = 0; i < n; i++) { dst[i] = img[lorom_off(a)]; a = lorom_next(a); }
}

/* Luigi's race sound set. The Stadium 4 driver lays Yoshi's voice sample
 * over Luigi's at ARAM $8E6D (directory entries $0C and $10 both point
 * there), and the Stadium 3 race set ($9D:8000, instruments $20-$24 up to
 * $EEB3) leaves no room below the echo buffer ($EF00) for Luigi's 6381-byte
 * sample. So Luigi's race takes Yoshi's own small race set (the S4 $9D:8000
 * stream: songs at $C360 on driver instruments), and Luigi's sample from the
 * Stadium 3 driver ($9E:8000 chunk $5F00, at $8E6D) goes to $C400 with
 * directory entry $0C pointed at it: sound $38 (Luigi's crash voice) plays
 * Luigi, sound $35 still plays Yoshi (entry $10). */
#define LUIGI_SAMPLE_ARAM 0xC400
#define LUIGI_SAMPLE_LEN  6381
static size_t build_luigi_sound(const uint8_t *s3, size_t s3size, uint8_t *out, size_t cap)
{
    SpcChunk c[32];
    size_t o = 0;
    int n = spc_chunks(s_rom, s_rom_size, 0x9D8000, c, 32);      /* S4: Yoshi's race set */
    for (int i = 0; i < n; i++) {
        if (o + 4 + c[i].len > cap) return 0;
        out[o++] = (u8)c[i].len; out[o++] = (u8)(c[i].len >> 8);
        out[o++] = (u8)c[i].dest; out[o++] = (u8)(c[i].dest >> 8);
        spc_copy(out + o, s_rom, c[i].at, c[i].len); o += c[i].len;
    }
    n = spc_chunks(s3, s3size, 0x9E8000, c, 32);                  /* S3 driver */
    int k = -1;
    for (int i = 0; i < n; i++)
        if (c[i].dest == 0x5F00 && c[i].len >= 0x8E6D - 0x5F00 + LUIGI_SAMPLE_LEN) k = i;
    if (k < 0 || o + 8 + LUIGI_SAMPLE_LEN + 4 + 4 > cap) return 0;
    u32 at = c[k].at;
    for (u32 i = 0; i < 0x8E6D - 0x5F00; i++) at = lorom_next(at);
    out[o++] = (u8)LUIGI_SAMPLE_LEN; out[o++] = (u8)(LUIGI_SAMPLE_LEN >> 8);
    out[o++] = (u8)LUIGI_SAMPLE_ARAM; out[o++] = (u8)(LUIGI_SAMPLE_ARAM >> 8);
    spc_copy(out + o, s3, at, LUIGI_SAMPLE_LEN); o += LUIGI_SAMPLE_LEN;
    u16 end = (u16)(LUIGI_SAMPLE_ARAM + LUIGI_SAMPLE_LEN);       /* no loop: loop = end, as S3 */
    const u8 dir[8] = { 4, 0, 0x30, 0x5C, (u8)LUIGI_SAMPLE_ARAM, (u8)(LUIGI_SAMPLE_ARAM >> 8), (u8)end, (u8)(end >> 8) };
    memcpy(out + o, dir, 8); o += 8;
    const u8 fin[4] = { 0, 0, 0x00, 0x08 };                      /* end, start $0800 */
    memcpy(out + o, fin, 4); o += 4;
    return o;
}

static void add_luigi(void)
{
    uint8_t *s3 = bunbun_data_load_dump("s3.sfc");
    if (!s3) {
        SDL_Log("[roster] data/s3.sfc not found: Luigi is not available");
        return;
    }
    put(EXT_LUIGI_SET, s3 + (((0x9A8000 >> 16) & 0x7F) << 15), 0x4000);    /* $9A:8000 */
    {
        static uint8_t snd[0x8000];
        size_t n = build_luigi_sound(s3, 0x100000, snd, sizeof snd);
        if (n)
            put(EXT_LUIGI_VOICE, snd, n);
        else    /* fall back to the Stadium 3 race set as it is */
            put(EXT_LUIGI_VOICE, s3 + (((0x9D8000 >> 16) & 0x7F) << 15), 0x3000);
        SDL_Log("[roster] Luigi sound set: %u bytes", (unsigned)n);
    }
    SDL_free(s3);

    /* Luigi takes Yoshi's place in Yoshi's own races (lineup Luigi, Wario,
     * Mario, Peach), with his set, his Stadium 3 palette ($82:8AF0) and name
     * (the Stadium 3 name table). Rider 3 is then Yoshi instead of Peach, so
     * the two meet: Yoshi's own rider set, palette and crash voice, as
     * Stadium 4's Mario race has him in that slot. */
    Character *l = &g_ch[5];
    *l = g_ch[1];
    l->label = "Luigi";
    for (int v = 0; v < 2; v++) {
        l->bank[v][0] = (u8)(EXT_LUIGI_SET >> 16);
        l->off[v][0] = (u16)EXT_LUIGI_SET;
        l->bank[v][3] = g_ch[1].bank[v][0];
        l->off[v][3] = g_ch[1].off[v][0];
    }
    for (int st = 0; st < 2; st++)
        for (int v = 0; v < 2; v++) {
            l->pal[st][v][0] = 0x828AF0;
            l->pal[st][v][3] = g_ch[1].pal[1][v][0];      /* Yoshi, $82:8B10 */
            l->voice[st][v][0] = 0x38;                    /* Luigi's own voice (Stadium 3) */
            l->voice[st][v][3] = g_ch[1].voice[1][v][0];  /* Yoshi's voice $35 */
        }
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

static int extra_char(int rider);
u8 roster_crash_voice(int st, int var, int ch, int rider)
{
    int xc = extra_char(rider & 7);
    if (xc >= 0)
        return g_ch[xc].voice[st ? 1 : 0][var ? 1 : 0][0];   /* its own voice */
    ch = real_char(ch);
    if (ch >= 5 && ch < g_count)
        return g_ch[ch].voice[st ? 1 : 0][var ? 1 : 0][rider & 7];
    return rd8(ABS(0xC8E9 + (((st ? 0x50 : 0) + (var ? 0x28 : 0) + ch * 8 + rider) & 0xFF)));
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
static void ws_rider_sprites(void)
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

/* Widescreen jump shadows: DrawRiderShadow ($80:CC18) draws a rider's two
 * shadow sprites at X - 7 and X - 15 only on page 1 from column $10. In the
 * wide view it also draws them for a rider in the margins, and the two
 * sprites ($16C0/$16C4 + Y, size entries $1850/$1851 + Y/4) get X9. */
static int ws_rider_x(void)
{
    unsigned r = X & 0xFF;
    return ((int)g_ram[0x019E + r] - 1) * 256 + g_ram[0x0196 + r];
}

int port_ws_shadow_margin(void)
{
    int extra = BunbunWsExtra();
    if (!extra || g_ram[0x76] != 0x03)
        return 0;
    unsigned r = X & 0xFF;
    if (g_ram[0x019E + r] == 1 && g_ram[0x0196 + r] >= 0x10)
        return 0;                                      /* the original draws it */
    int x = ws_rider_x();
    return x > -extra && x < 256 + extra + 15;
}

void port_ws_shadow_fix(void)
{
    int x = ws_rider_x();
    unsigned k = (Y & 0xFF) >> 2;
    g_ram[0x1850 + k] = (uint8_t)(((x - 7) & 0x1FF) >> 8);
    g_ram[0x1851 + k] = (uint8_t)(((x - 15) & 0x1FF) >> 8);
}

/* ---- every character in the race ---------------------------------------------
 * (launcher: Game > Every character races.) A race shows four characters: the
 * player's (sprite slot 0) and three rivals, whose animation frames stream
 * into four VRAM slots, and the four Koopa riders 4-7, drawn from one fixed
 * sheet. Here the characters not in the lineup take Koopa riders 4, 5, ...
 * Their frames could not stream (no VRAM or sprite palettes are free for
 * them), so they are drawn by the port's renderer as extra objects, with
 * their own rider set and palette, in the place and the OAM order of the
 * Koopa's sprites:
 *
 *   PlaceRiderShadow ($80:CE96, the last routine to touch a rider's four
 *   16x16 sprites, at OAM buffer $1600 + $07) ends in port_ws_rider_sprites.
 *   For such a rider each visible sprite is parked at Y = $F0 (off screen)
 *   with attribute MAGIC and a ticket number as its tile; the ticket records
 *   the quadrant, Y, animation frame ($1961 + rider) and character. X and its
 *   ninth bit stay, so the sprite keeps its place through the game's own OAM
 *   DMA. Before each frame is drawn, RosterExtraRidersFrame finds the parked
 *   sprites in the PPU's OAM and draws each quadrant from its ticket. */
#define XR_MAGIC 0xA5               /* attribute: V flip, priority 2, palette 2, table 1 */
typedef struct { u8 slot, q, y, frame, ch; u32 stamp; } XRec;
static XRec s_xr[256];
static u8 s_xnext;
static u32 s_xframe;

static int all_riders_on(void)
{
    static int v = -1;
    if (v < 0) v = bunbun_option("BUNBUN_ALL_RIDERS", "game.allriders", 1);
    return v && g_ready;
}

/* The character whose own rider set (slot 0 of its own race) is this one. */
static int set_owner(u8 bank, u16 off, int var)
{
    for (int c = 0; c < g_count; c++)
        if (g_ch[c].bank[var][0] == bank && g_ch[c].off[var][0] == off)
            return c;
    return -1;
}

/* Character drawn for rider 4-7, or -1 for the Koopa. */
static int extra_char(int rider)
{
    if (rider < 4 || !all_riders_on())
        return -1;
    int var = g_ram[0x1E91] ? 1 : 0;
    u8 ch = g_ram[0x154F];
    unsigned in = 0;
    for (int s = 0; s < 4; s++) {
        int c = set_owner(roster_rider_bank(ch, var, s), roster_rider_offset(ch, var, s * 2), var);
        if (c >= 0) in |= 1u << c;
    }
    int k = rider - 4;
    for (int c = 0; c < g_count; c++)
        if (!(in & (1u << c)) && k-- == 0)
            return c;
    return -1;
}

static void extra_rider_sprites(void)
{
    unsigned r = X & 0xFF;
    if (r < 4 || r > 7)
        return;
    int ch = extra_char((int)r);
    if (ch < 0)
        return;
    unsigned o = g_ram[0x07];
    for (unsigned q = 0; q < 4; q++) {
        unsigned e = 0x1600 + o + 4 * q;
        u8 y = g_ram[e + 1];
        if (y >= 0xE0)
            continue;                                  /* hidden by the game */
        u8 t = s_xnext++;
        s_xr[t] = (XRec){ (u8)((o >> 2) + q), (u8)q, y, g_ram[0x1961 + r], (u8)ch, s_xframe };
        g_ram[e + 1] = 0xF0;
        g_ram[e + 2] = t;
        g_ram[e + 3] = XR_MAGIC;
    }
}

void port_ws_rider_sprites(void)
{
    ws_rider_sprites();
    extra_rider_sprites();
}

/* One 16x16 quadrant q of animation frame f of character ch, RGB15 with bit
 * 15 set on opaque pixels. Frame f is the 32x32 picture at set + (f & $FE) *
 * $200 + (f & 1) * $100: a row of eight tiles, the next row $200 on; the four
 * 16x16 sprites are tiles 0-1, 2-3, 4-5, 6-7 of it (as the slots in VRAM). */
static void draw_quadrant(uint16_t *pix, int ch, int f, int q)
{
    int st = rd8(LONG(0x81800C)) ? 1 : 0, var = g_ram[0x1E91] ? 1 : 0;
    u32 set = (u32)g_ch[ch].bank[var][0] << 16 | g_ch[ch].off[var][0];
    u32 a = set + (u32)(f & 0xFE) * 0x200 + (u32)(f & 1) * 0x100;
    u32 pal = g_ch[ch].pal[st][var][0];
    uint16_t col[16];
    for (int i = 0; i < 16; i++)
        col[i] = (uint16_t)(rom16(pal + i * 2) & 0x7FFF);
    for (int ty = 0; ty < 2; ty++)
        for (int tx = 0; tx < 2; tx++) {
            u32 t = a + (u32)ty * 0x200 + (u32)(q * 2 + tx) * 32;
            for (int row = 0; row < 8; row++) {
                u8 p0 = rom8(t + row * 2), p1 = rom8(t + row * 2 + 1);
                u8 p2 = rom8(t + 16 + row * 2), p3 = rom8(t + 17 + row * 2);
                for (int px = 0; px < 8; px++) {
                    int b = 7 - px;
                    int c = (p0 >> b & 1) | (p1 >> b & 1) << 1 | (p2 >> b & 1) << 2 | (p3 >> b & 1) << 3;
                    pix[(ty * 8 + row) * 16 + tx * 8 + px] = c ? (uint16_t)(col[c] | 0x8000) : 0;
                }
            }
        }
}

#include "snes/ppu.h"
extern Ppu *g_ppu;
void RosterExtraRidersFrame(void)
{
    static PpuExtraObject objs[32];
    static uint16_t pix[32][256];
    s_xframe++;
    if (!g_ppu)
        return;
    int n = 0;
    if (all_riders_on())
        for (int slot = 0; slot < 128 && n < 32; slot++) {
            uint16_t w0 = g_ppu->oam[slot * 2], w1 = g_ppu->oam[slot * 2 + 1];
            if ((w0 >> 8) != 0xF0 || (w1 >> 8) != XR_MAGIC)
                continue;
            const XRec *rc = &s_xr[w1 & 0xFF];
            if (rc->slot != slot || s_xframe - rc->stamp > 8)
                continue;
            int x = (w0 & 0xFF) | ((g_ppu->highOam[slot >> 2] >> ((slot & 3) * 2)) & 1) << 8;
            if (x >= 256 + g_ppu->extraRightCur)
                x -= 512;
            draw_quadrant(pix[n], rc->ch, rc->frame, rc->q);
            objs[n] = (PpuExtraObject){ .x = x, .y = rc->y, .width = 16, .height = 16, .stride = 16,
                                        .priority = 2, .math = 0, .oamSlot = (uint8_t)slot,
                                        .order = 1, .pixels = pix[n] };
            n++;
        }
    PpuSetExtraObjects(g_ppu, n ? objs : NULL, (size_t)n);
}

/* ---- the BIOS exits and course overrides in native mode ---------------------
 * The recompiled port replaces these with hle_funcs (src/gen_stubs.c,
 * src/courses.c); in native mode the hand-written routines run and ask here. */
#include "hand/hand.h"
extern int r816_native;
extern int g_port_reset_request;

/* HleEndScreenStart: Start restarts the broadcast hour (minute 0, frame 0)
 * and goes to the credits, mode $12. */
int port_end_screen_start(void)
{
    if (!r816_native || !(rd8(DP(0x21)) & 0x10))
        return 0;
    wr8(LONG(0x7FFFFF), 0);
    wr8(LONG(0x7FFFFA), 0);
    wr8(LONG(0x7FFFFB), 0);
    lda8(0x12); wr8(DP(0x76), 0x12);
    wr8(DP(0x75), 0);
    wr8(DP(0x77), 0);
    wr8(ABS(0x1923), dec8(rd8(ABS(0x1923))));
    ret();
    return 1;
}

/* HleCreditsExit: the credits' Start ($80:9F3A) resets the console at the
 * next frame instead of jumping into the BIOS. */
int port_bsx_exit(u32 site)
{
    if (!r816_native || site != 0x809F3A)
        return 0;
    g_port_reset_request = 1;
    sep(0x20);
    ret();
    return 1;
}

void BunbunCourseOverride(int idx);     /* src/courses.c */
void port_course_override(int idx)
{
    if (r816_native)
        BunbunCourseOverride(idx);
}
