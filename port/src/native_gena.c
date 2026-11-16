/*
 * native_gena.c - the Gen A (Stadium 1-2) port hooks of the hand-written C
 * (c/hand/s1/port.h) for native mode (src/native_driver.c). The recompiled
 * port does the same in hle_funcs: HleEndScreenStart and HleCreditsExit
 * (src/gen_stubs.c) and HleHand_LoadCourse's custom courses
 * (src/r816_bridge.c).
 */
#include "hand/hand.h"
#include "port.h"

extern int r816_native;
extern int g_port_reset_request;
extern uint8_t g_ram[];
int BunbunCustomCourse(int index, uint8_t *buf, int cap);   /* src/mods_folder.c */

/* EndScreenStart ($80:9DE0) with Start: restart the broadcast hour (minute 0,
 * frame 0) and go to the credits, mode $12. */
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

/* The credits' Start ($80:A003): a console reset at the next frame instead
 * of the BS-X BIOS. */
int port_bsx_exit(u32 site)
{
    if (!r816_native || site != 0x80A003)
        return 0;
    g_port_reset_request = 1;
    sep(0x20);
    ret();
    return 1;
}

/* A custom course replaces the one LoadCourse expanded (index as
 * HleHand_LoadCourse computes it: Stadium * 4 + course, the special course
 * Stadium * 2 + 8). */
void port_course_loaded(void)
{
    if (!r816_native)
        return;
    int st = rd8(LONG(0x81800C));
    int idx = (g_ram[0x0CD7] | g_ram[0x0CD8]) ? st * 2 + 8 : st * 4 + g_ram[0x7C];
    BunbunCustomCourse(idx, &g_ram[0x14000], 0x1000);           /* $7F:4000 */
}

/* Widescreen riders (src/widescreen_bunbun.c). Rider X's screen X is
 * ($0198 - 1) * 256 + $0190: page 0 left of the original screen, 1 on it,
 * 2 right of it. DrawBigSprite wrote its four 16x16 sprites at $1600 + $07
 * (columns X and X + 16, rows Y and Y + 16) with the low byte of X, and
 * ClipRiderSprite hid the columns off the original screen. Inside the wide
 * view, give each column its X9 bit and put the rows back (Y = $01B0 - 8, as
 * DrawRider placed them). Gen B does the same in src/roster.c. */
int BunbunWsExtra(void);
void port_ws_clip_rider(void)
{
    int extra = BunbunWsExtra();
    if (!extra || (g_ram[0x76] != 0x03 && g_ram[0x76] != 0x0C))
        return;
    unsigned r = X & 0xFF;
    if (r > 7)
        return;
    int x = ((int)g_ram[0x0198 + r] - 1) * 256 + g_ram[0x0190 + r];
    if (x <= -32 - extra || x >= 256 + extra)
        return;                                        /* outside the wide view */
    if (x >= 0 && x <= 256 - 32)
        return;                                        /* the original handles it */
    unsigned o = g_ram[0x07];
    uint8_t y = (uint8_t)(g_ram[0x01B0 + r] - 8);
    g_ram[0x1601 + o] = y;                  g_ram[0x1605 + o] = y;
    g_ram[0x1609 + o] = (uint8_t)(y + 16);  g_ram[0x160D + o] = (uint8_t)(y + 16);
    unsigned k = o >> 2;
    uint8_t x9a = (uint8_t)((x >> 8) & 1), x9b = (uint8_t)(((x + 16) >> 8) & 1);
    g_ram[0x1820 + k] = (uint8_t)(2 | x9a); g_ram[0x1822 + k] = (uint8_t)(2 | x9a);
    g_ram[0x1821 + k] = (uint8_t)(2 | x9b); g_ram[0x1823 + k] = (uint8_t)(2 | x9b);
}

/* Widescreen wheel-spark pairs: WheelSpark ($80:CDF6) draws a rider's two
 * sprites at X - 7 and X - 15 only on page 1 from column $10. In the wide
 * view it also draws them for a rider in the margins, and the two sprites
 * ($16C0/$16C4 + Y, size entries $1850/$1851 + Y/4) get X9. */
static int gena_rider_x(void)
{
    unsigned r = X & 0xFF;
    return ((int)g_ram[0x0198 + r] - 1) * 256 + g_ram[0x0190 + r];
}

int port_ws_spark_margin(void)
{
    int extra = BunbunWsExtra();
    if (!extra || (g_ram[0x76] != 0x03 && g_ram[0x76] != 0x0C) || (X & 0xFF) > 7)
        return 0;
    unsigned r = X & 0xFF;
    if ((g_ram[0x0198 + r] & 1) && g_ram[0x0190 + r] >= 0x10)
        return 0;                                      /* the original draws it */
    int x = gena_rider_x();
    return x > -extra && x < 256 + extra + 15;
}

void port_ws_spark_fix(void)
{
    int x = gena_rider_x();
    unsigned k = (Y & 0xFF) >> 2;
    g_ram[0x1850 + k] = (uint8_t)(((x - 7) & 0x1FF) >> 8);
    g_ram[0x1851 + k] = (uint8_t)(((x - 15) & 0x1FF) >> 8);
}
