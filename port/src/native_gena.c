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
