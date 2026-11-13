/*
 * widescreen_bunbun.c - the widescreen policy for Bun Bun Mario Battle.
 *
 * The framework renders a wider PPU field when the game declares
 * native_widescreen and returns a wider frame from prepare_frame
 * (snesrecomp/runner/src/widescreen.h, docs/WIDESCREEN_PATTERNS.md). What
 * each screen shows in the extra columns is the game's policy, set every
 * frame in begin_sim_frame.
 *
 * Setting (launcher: Game > Widescreen, bunbun.ini display.widescreen):
 *   off    256 columns, the original picture (nothing here runs)
 *   16:9   348 columns (46 each side): 16:9 with the SNES's 8:7 pixels
 *
 * Races and the jump contest show the wide view; every other screen is
 * pillarboxed (the course preview's track band and frame would each need
 * their own fill rules). Gen B
 * (Stadium 3-4) only: the addresses below are that engine's.
 *
 * Riders in the margins: PlaceRiderShadow ($80:CE96, hand C) ends with
 * port_ws_rider_sprites (src/roster.c), which unhides a rider's sprites
 * inside the wide view, and DrawRiderShadow ($80:CC18, hand C) draws the
 * jump shadows of riders in the margins (port_ws_shadow_*).
 *
 * Track lookahead. The race's track layer lives in a 64-column tilemap, and
 * the game streams one 8-pixel column into it ($81:8A4D) just before that
 * column reaches the original right edge, so the right margin would show
 * columns from the lap before. Each frame this replays the streamer's
 * decoding on copies of its state ($027B track position, $027D piece,
 * $027E offset in the piece, $0277/$0278 tilemap column) and writes the next
 * columns into the tilemap ahead of the game, exactly as the game will
 * write them itself a few frames later. The game's state is never touched:
 * the 64-column map has 20 columns the 44 visible ones never reach, and the
 * columns written are the ones the game overwrites with the same tiles.
 */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "bunbun_settings.h"
#include "snes/ppu.h"
#include "snes/snes.h"
#include "snes/cart.h"

extern Ppu *g_ppu;
extern Snes *g_snes;
extern uint8_t g_ram[];

enum { kLookahead = 8 };          /* columns past the game's next one */

/* BUNBUN_WS_DEBUG=1: report cells the game later wrote differently. */
static uint16_t s_mine[0x8000];
static uint8_t s_have[0x8000];
static int s_debug = -1;
static void check_prediction(unsigned frame)
{
    if (s_debug < 0) { const char *e = getenv("BUNBUN_WS_DEBUG"); s_debug = e && *e == '1'; }
    if (!s_debug) return;
    int bad = 0;
    for (int a = 0; a < 0x8000; a++)
        if (s_have[a] && g_ppu->vram[a] != s_mine[a]) {
            if (bad++ < 6)
                SDL_Log("[ws] f%u vram %04X mine %04X game %04X (0277=%02X 0278=%02X 027B=%02X%02X 027D=%02X 027E=%02X)",
                        frame, a, s_mine[a], g_ppu->vram[a], g_ram[0x277], g_ram[0x278], g_ram[0x27C], g_ram[0x27B], g_ram[0x27D], g_ram[0x27E]);
            s_have[a] = 0;
        }
}

static int ws_extra(void)
{
#ifndef BUNBUN_GENB
    return 0;                     /* the policy below is the Gen B engine's */
#endif
    static int extra = -1;
    if (extra < 0) {
        const char *v = bunbun_setting("display.widescreen", "off");
        extra = strcmp(v, "16:9") == 0 ? 46 : 0;
    }
    return extra;
}

int BunbunWsExtra(void) { return ws_extra(); }

void BunbunWsPrepareFrame(int drawable_w, int drawable_h, int *frame_w, int *frame_h)
{
    (void)drawable_w; (void)drawable_h;
    *frame_w = 256 + 2 * ws_extra();
    *frame_h = 224;
}

static uint8_t rom81(uint16_t a)
{
    const uint8_t *rom = g_snes->cart->rom;
    return rom[0x8000u | (a & 0x7FFFu)];
}

static uint16_t rom81w(uint16_t a)
{
    return (uint16_t)(rom81(a) | rom81((uint16_t)(a + 1)) << 8);
}

/* $81:8A4D's decoding, side-effect free. */
static void track_lookahead(void)
{
    uint16_t pos = (uint16_t)(g_ram[0x027B] | g_ram[0x027C] << 8);
    uint8_t piece = g_ram[0x027D], sub = g_ram[0x027E];
    uint8_t col = g_ram[0x0277], page = g_ram[0x0278];
    uint8_t lapsign = g_ram[0x0929], laps = g_ram[0x0928], c93f = g_ram[0x093F];
    const uint8_t *track = &g_ram[0x14000];                  /* $7F:4000 */
    uint8_t mask = rom81(0x8070);
    for (int n = 0; n < kLookahead + 1; n++) {
        uint16_t ptr;
        uint8_t s;
        int guard = 0;
        for (;;) {
            if (sub == 0) {                                    /* next track code */
                pos++;
                if (pos >= 0x1000)
                    return;
                uint8_t b = track[pos];
                if (b == 0)
                    return;                                    /* end of the course */
                piece = (b & mask) ? rom81((uint16_t)(0x8A30 + (b & 0x0F))) : (uint8_t)(b & 0x7F);
            }
            ptr = (uint16_t)(rom81((uint16_t)(0x8010 + piece)) | rom81((uint16_t)(0x8040 + piece)) << 8);
            s = rom81((uint16_t)(ptr + sub));
            if (s != 0)
                break;
            sub = 0;                                           /* the piece is done */
            if (piece == 0x09) {                               /* the gate ends the lap */
                pos = 0;
                laps++;
                lapsign = 0;
            }
            if (++guard > 8)
                return;
        }
        uint16_t ent[16];
        for (int x = 2; x <= 0x14; x += 2)
            ent[x / 2] = 0x08FC;                               /* blank rows above the piece */
        uint8_t y = sub;
        for (int x = s; x != 0x1E && x < 0x20; x += 2) {
            y += 2;
            uint16_t w = rom81w((uint16_t)(ptr + y));
            ent[x / 2] = (uint16_t)((w & 0x07FF) | 0x0800);
        }
        y += 2;
        sub = y;
        if (piece == 0x09 && c93f != 1 && y >= 0x4A && y < 0xB0) {   /* the LAP n sign */
            uint8_t x = lapsign++;
            uint8_t a = rom81((uint16_t)(0x8A3D + x));
            if (x == 4)
                a = laps >= 10 ? 1 : a;
            else if (x > 4)
                a = laps >= 10 ? (uint8_t)(laps - 10) : laps;
            ent[13] = (uint16_t)((ent[13] & 0xFF00) | a);   /* a byte write */
        }
        uint16_t dest = (uint16_t)(((page << 8) | col) - 0x20);
        for (int i = 0; i < 14; i++)
        {
            uint16_t a = (uint16_t)(dest + 32 * i) & 0x7FFF;
            g_ppu->vram[a] = ent[1 + i];
            s_mine[a] = ent[1 + i];
            s_have[a] = 1;
        }
        if (++col == 0x20) {
            col = 0;
            page ^= 4;
        }
    }
}

/* Per frame, after the host has set the symmetric budget. */
void BunbunWsBeginFrame(unsigned number)
{
    (void)number;
    int extra = ws_extra();
    if (!extra || !g_ppu || !g_snes || !g_snes->cart || !g_snes->cart->rom)
        return;
    uint8_t mode = g_ram[0x76];
    if (mode == 0x03) {                                        /* a race: the wide view */
        check_prediction(number);
        /* only once the streamer runs: during the race setup ($027B still
         * 0) other code draws the stands into the same tilemap */
        if (g_ram[0x027B] | g_ram[0x027C])
            track_lookahead();
        /* The bottom HUD is BG2 lines 184-223 at scroll 0: a checkered flag
         * across the whole width (the text is on another layer). Reflect
         * it about the native edges so the flag continues into the
         * margins; above line 184 BG2 still carries the track's scroll. */
        PpuSetWidescreenLayerWorldMirrorBand(g_ppu, 1, 184, 224, 0, 256);
        return;
    }
    if (mode == 0x14)                                          /* the jump contest: its */
        return;                       /* scene fills the whole tilemap, so it extends as is */
    PpuSetExtraSideSpace(g_ppu, 0, 0, 0);                      /* elsewhere: pillarbox */
}
