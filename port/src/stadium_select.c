/*
 * stadium_select.c - which Stadium the exe plays, and the in-game Stadium
 * select on the title screen (the Gen B exe: 1-4; the Gen A exe: 1-2,
 * where only the side byte changes).
 *
 * Stadiums 3 and 4 are this build's own (side 0 / 1 at $81:800C). Stadiums
 * 1 and 2 run in this engine with their own courses from data/s1.sfc
 * (src/courses.c), on side 0 / 1 as in Gen A, with Gen A's hour (no jump
 * contests, one bonus game at the end: src/roster.c). The launcher's
 * Game > Stadium (game.stadium, BUNBUN_STADIUM) picks the first one.
 *
 * In the game: on the title and the attract demo (modes 0, $0E, $0C) a
 * "< STADIUM n >" label sits under the picture; L and R step through the
 * Stadiums. The choice takes effect from the next course loaded (the
 * player select and race that follow) and is saved to bunbun.ini. The label
 * is drawn by the port's renderer as an extra object (nothing in VRAM or
 * OAM), in a small font of its own. The title logo follows the side byte,
 * so it reads 3 for Stadium 1 and 4 for Stadium 2; the label is the choice.
 */
#include <stdint.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "bunbun_settings.h"
#include "snes/ppu.h"
#include "snes/snes.h"
#include "snes/cart.h"

extern Ppu *g_ppu;
extern Snes *g_snes;
extern uint8_t g_ram[];
int BunbunCoursesUse(int set);   /* src/courses.c */

static int s_stadium;            /* 1-4, 0 before the first apply */

#ifdef BUNBUN_GENB
enum { ST_FIRST = 1, ST_LAST = 4, ST_DEFAULT = 4 };
#else
enum { ST_FIRST = 1, ST_LAST = 2, ST_DEFAULT = 1 };   /* the Gen A exe: Stadiums 1-2 */
#endif

int BunbunStadium(void) { return s_stadium ? s_stadium : ST_DEFAULT; }

/* Make st the Stadium: the side byte, and in the Gen B exe the course set
 * and the rules. */
void BunbunStadiumApply(int st)
{
    if (st < ST_FIRST || st > ST_LAST)
        st = ST_DEFAULT;
#ifdef BUNBUN_GENB
    if (st <= 2 && !BunbunCoursesUse(st - 1)) {
        SDL_Log("[bunbun] Stadium %d needs data/s1.sfc", st);
        st = st + 2;                                  /* its side's own Stadium */
        BunbunCoursesUse(-1);
    } else if (st > 2) {
        BunbunCoursesUse(-1);
    }
#endif
    if (g_snes && g_snes->cart && g_snes->cart->rom && g_snes->cart->romSize > 0x800C)
        g_snes->cart->rom[0x800C] = (uint8_t)((st - 1) & 1);   /* 1, 3 -> side 0; 2, 4 -> side 1 */
    s_stadium = st;
}

/* ---- the label ---------------------------------------------------------- */

static const char *glyph(char c)
{
    switch (c) {
    case 'S': return ".######.##....####.......######.......####....##.######.........";
    case 'T': return "########...##......##......##......##......##......##...........";
    case 'A': return "..####...##..##.##....############....####....####....##........";
    case 'D': return "######..##...##.##....####....####....####...##.######..........";
    case 'I': return ".######....##......##......##......##......##....######.........";
    case 'U': return "##....####....####....####....####....####....##.######.........";
    case 'M': return "##....#####..#############.##.####....####....####....##........";
    case '1': return "...##.....###......##......##......##......##....######.........";
    case '2': return ".#####..##...##......##....###....##.....##.....#######.........";
    case '3': return "######.......##......##...####.......##......##.######..........";
    case '4': return "....##.....###....####...##.##..#######.....##......##..........";
    case '<': return "....##.....##.....##.....##.......##.......##.......##..........";
    case '>': return ".##.......##.......##.......##.....##.....##.....##.............";
    default:  return NULL;
    }
}

#define LBL_CHARS 13
#define LBL_W (LBL_CHARS * 8 + 2)
#define LBL_H 10

static void draw_label(uint16_t *pix, const char *text)
{
    static uint8_t on[LBL_H][LBL_W];
    memset(on, 0, sizeof on);
    for (int i = 0; text[i] && i < LBL_CHARS; i++) {
        const char *g = glyph(text[i]);
        if (!g)
            continue;
        size_t len = strlen(g);
        for (int r = 0; r < 8; r++)
            for (int c = 0; c < 8; c++) {
                size_t k = (size_t)(r * 8 + c);
                if (k < len && g[k] == '#')
                    on[r + 1][i * 8 + c + 1] = 1;
            }
    }
    for (int y = 0; y < LBL_H; y++)
        for (int x = 0; x < LBL_W; x++) {
            uint16_t px = 0;
            if (on[y][x])
                px = 0x8000 | 0x7FFF;                         /* white */
            else
                for (int dy = -1; dy <= 1 && !px; dy++)
                    for (int dx = -1; dx <= 1 && !px; dx++) {
                        int yy = y + dy, xx = x + dx;
                        if (yy >= 0 && yy < LBL_H && xx >= 0 && xx < LBL_W && on[yy][xx])
                            px = 0x8000 | (8 << 10);           /* dark blue outline */
                    }
            pix[y * LBL_W + x] = px;
        }
}

/* Per frame (begin_sim_frame, after the roster's extra riders): on the title
 * and the attract demo, L / R change the Stadium and the label is shown. */
void BunbunStadiumSelectFrame(void)
{
    if (!g_ppu || !s_stadium)
        return;
    static int shown;
    uint8_t mode = g_ram[0x76];
    if (mode != 0x00 && mode != 0x0E && mode != 0x0C) {
        if (shown) {                              /* the Gen A exe has nobody else to clear it */
#ifndef BUNBUN_GENB
            PpuSetExtraObjects(g_ppu, NULL, 0);
#endif
            shown = 0;
        }
        return;                                   /* the title (0 / $0E), the attract demo ($0C) */
    }
    uint8_t press = g_ram[0x26];                              /* P1_PRESS_LO: A X L R */
    int st = s_stadium;
    if (press & 0x20) st = st == ST_FIRST ? ST_LAST : st - 1;   /* L */
    if (press & 0x10) st = st == ST_LAST ? ST_FIRST : st + 1;   /* R */
    if (st != s_stadium) {
        int want = st;
        BunbunStadiumApply(st);
        if (s_stadium != want && want <= 2)                  /* no data/s1.sfc: skip 1-2 */
            BunbunStadiumApply(press & 0x20 ? 4 : 3);
        char v[2] = { (char)('0' + s_stadium), 0 };
        bunbun_setting_set("game.stadium", v);
        bunbun_settings_save();
    }
    static uint16_t pix[LBL_W * LBL_H];
    static PpuExtraObject obj;
    char text[LBL_CHARS + 1];
    SDL_snprintf(text, sizeof text, "< STADIUM %d >", s_stadium);
    draw_label(pix, text);
    obj = (PpuExtraObject){ .x = 128 - LBL_W / 2, .y = 213, .width = LBL_W, .height = LBL_H,
                            .stride = LBL_W, .priority = 3, .math = 0, .oamSlot = 0,
                            .order = 1, .pixels = pix };
    PpuSetExtraObjects(g_ppu, &obj, 1);
    shown = 1;
}
