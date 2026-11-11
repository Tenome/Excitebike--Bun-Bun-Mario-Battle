/*
 * BunBun S1 — per-game runtime glue.
 *
 * THIS FILE IS THE PORT. Everything else the scaffold produced is layout,
 * build wiring, and packaging; this is where the actual work happens.
 *
 * The framework does not, and cannot, drive an arbitrary SNES game on its
 * own. Two responsibilities always land on the game host:
 *
 *   1. Deciding what "one frame" means for THIS title, and returning from
 *      run_frame() at that boundary. A real ROM's reset vector never
 *      returns — it enters a main loop that waits on vblank — so somebody
 *      has to choose the yield point.
 *   2. Delivering NMI/IRQ at the hardware edge (see
 *      snesrecomp/docs/FRAME_MODEL_HOSTS.md).
 *
 * A new port starts on the framework's beam-aligned driver
 * (runner/src/beam_frame_driver.h): one frame is one PPU field, cut at the
 * beam's vblank edge; interrupts -- raster splits and a coprocessor's IRQ
 * line -- are taken where the beam latches them; parked time still runs the
 * beam, APU and coprocessors; and the field is rasterized from the raster
 * journal with real HDMA. It is the framework's, so a fix to it reaches this
 * port on a submodule pull.
 *
 * This file used to hold its own copy of a simpler driver: frames measured
 * from wherever the CPU last stopped, NMI delivered at an arbitrary beam
 * position, the field drawn with end-of-frame register state. A game that
 * uploads in vblank between INIDISP=$80 and $0F flickered black, and one
 * that chains raster splits fell apart. Replace these calls only for what
 * the driver does not do; MetalWarriorsSNESRecomp's src/mw_rtl.c and
 * GundamWingEndlessDuelSNESRecomp's src/game_rtl.c are developed examples of
 * title-specific drivers.
 */

#include "game_rtl.h"

#include "beam_frame_driver.h"
#include "common_rtl.h"
#include <stdlib.h>

/* Set by port replacements (src/gen_stubs.c) that stand in for "return to the
 * BS-X BIOS": the console is reset at the next frame boundary, which is what
 * leaving to the BIOS menu and launching the game again amounts to. */
int g_port_reset_request;

/* Stadium selection. Each Gen A build carries both Stadium 1 and Stadium 2
 * (courses, rules, end screen) and picks one with the byte at $81:800C
 * (0 = Stadium 1, 1 = Stadium 2); Gen B builds carry Stadiums 3 and 4 the same
 * way. The 1998 rebroadcasts are one build per generation with this byte
 * flipped. BUNBUN_STADIUM=1|2 (Gen A) or 3|4 (Gen B) selects it here. The
 * ROM image is patched in memory after verification; the file is untouched. */
#include <stdlib.h>
#include "snes/cart.h"
void BunbunCoursesInit(void);
static void GameInitialize(void)
{
#ifdef BUNBUN_GENB
    static int courses_ready;
    if (!courses_ready) { courses_ready = 1; BunbunCoursesInit(); }
#endif
    const char *v = getenv("BUNBUN_STADIUM");
    if (v && g_snes && g_snes->cart && g_snes->cart->rom && g_snes->cart->romSize > 0x800C) {
        int st = atoi(v);
#ifdef BUNBUN_GENB
        if (st == 3 || st == 4)     /* Gen B build: 0 = Stadium 3, 1 = Stadium 4 */
            g_snes->cart->rom[0x800C] = (uint8_t)(st - 3);
#else
        if (st == 1 || st == 2)     /* Gen A build: 0 = Stadium 1, 1 = Stadium 2 */
            g_snes->cart->rom[0x800C] = (uint8_t)(st - 1);
#endif
    }
}

/* Optional: BUNBUN_SKIPWAIT=1 lets P1 Start skip the broadcast's waiting
 * screens (the title's "please wait" and the 2-minute course cards, where the
 * SoundLink narration used to play) by advancing the broadcast clock one
 * minute: $7F:FFFF = minute, $7F:FFFA/B = frame within the minute. The game's
 * own schedule checks then move on. Gen A modes: $0E title wait, $02 course
 * card. (The intro text, mode $0D, scrolls on its own timer; advancing the
 * clock there would skip whole courses, so it is left alone.) */
static int SkipWaitEnabled(void)
{
    static int v = -1;
    if (v < 0) { const char *e = getenv("BUNBUN_SKIPWAIT"); v = e && *e && *e != '0'; }
    return v;
}

static void SkipWaitAfterFrame(void)
{
#ifndef BUNBUN_GENB
    uint8 mode = g_ram[0x76], pressed = g_ram[0x21];
    if ((mode == 0x0E || mode == 0x02) && (pressed & 0x10) && g_ram[0x1FFFF] < 59) {
        g_ram[0x1FFFF]++;
        g_ram[0x1FFFA] = 0;
        g_ram[0x1FFFB] = 0;
    }
#endif
}

void GameRunOneFrame(void)
{
    snes_beam_frame_driver_run_frame();
    if (SkipWaitEnabled())
        SkipWaitAfterFrame();
    if (g_port_reset_request) {
        g_port_reset_request = 0;
        RtlReset(1);
        snes_beam_frame_driver_reset();
    }
}

void GameDrawPpuFrame(void)
{
    snes_beam_frame_driver_draw_ppu_frame();
}

void GameSessionReset(void)
{
    /* Rematch / soft-return: clear anything that must not survive a new
     * session. recomp-ai-rules/NETPLAY.md §3 — sticky state that "has always
     * been fine" is the usual desync culprit, because single-player never
     * re-enters the boot path twice in one process. */
    snes_beam_frame_driver_reset();
}

const RtlGameInfo kGameInfo = {
    .title = "bunbuns1",
    .initialize = &GameInitialize,
    .run_frame = &GameRunOneFrame,
    .draw_ppu_frame = &GameDrawPpuFrame,
    #ifdef BUNBUN_GENB
    .save_name_prefix = "save_genb",
#else
    .save_name_prefix = "save",
#endif
    .session_reset = &GameSessionReset,
    .hardware_reset = &GameSessionReset,
    .state_save_extra = &snes_beam_frame_driver_state_save,
    .state_load_extra = &snes_beam_frame_driver_state_load,
    .on_state_loaded = &snes_beam_frame_driver_state_loaded,
    .snapshot_guard_identity = &snes_beam_frame_driver_state_identity,
    .snapshot_preflight = &snes_beam_frame_driver_state_preflight,
};
