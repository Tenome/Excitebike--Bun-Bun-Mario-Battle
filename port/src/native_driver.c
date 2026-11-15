/*
 * native_driver.c - run the game from the decompiled C alone.
 *
 * Every routine of the Stadium 4 build is hand-written C (c/hand/s4,
 * difftested against the original), Reset, NMI and IRQ included. In native
 * mode (BUNBUN_NATIVE=1, bunbun.ini game.engine = native) that C is the CPU:
 * no recompiled code runs. The hand-written Reset handler (boot, then a main
 * loop frame after each NMI) runs on a fiber of its own; the NMI and IRQ
 * handlers run on fibers stacked on top of it when the hardware raises them.
 *
 * Time: every bus access the C makes charges the CPU clock (g_cpu.master_cycles)
 * and brings the beam, the APU and the timers up to it before a hardware
 * register is touched, so the code that waits on hardware (the sound CPU's
 * handshakes, the NMI flag) sees it move. When the clock reaches the slice
 * deadline the running fiber yields to this driver, which, like the
 * framework's beam driver (beam_frame_driver.c), runs the field out to V=225,
 * taking each raster IRQ at the beam position it latches at; the field is then
 * drawn from the raster journal as before (GameDrawPpuFrame).
 *
 * An interrupt preempts the C between two bus accesses, which is between two
 * instructions of the original: the handler saves and restores the registers
 * it uses (the runtime's A, X, Y, ... are the machine registers), and the
 * interrupt frame (PB, PC, P) goes on the guest stack as the CPU pushes it.
 *
 * Not in native mode: save states and rewind (the fibers' C stacks are not
 * part of a snapshot).
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_state.h"
#include "common_cpu_infra.h"
#include "common_rtl.h"
#include "snes/cart.h"
#include "snes/ppu.h"
#include "snes/snes.h"
#include "hand_table.h"

extern CpuState g_cpu;
extern Ppu *g_ppu;
extern uint8_t g_ram[];

/* runtime816 machine state (c/runtime816.c) */
extern uint16_t A, X, Y, S, D;
extern uint8_t DB, PB;
extern uint8_t fn, fz, fc, fv, fd, fi, fm, fx;
void r816_bus_write(uint32_t a, uint8_t v);
static void push8(uint8_t v) { r816_bus_write(S, v); S--; }
static void push16(uint16_t v) { push8((uint8_t)(v >> 8)); push8((uint8_t)v); }

extern int r816_native;                 /* 1: the hand-written C is the CPU (r816_bridge.c) */

#if defined(_WIN32)
#include <windows.h>

#define FIELD_CYCLES 357368ull
#define LINE_CYCLES 1364u
#define VBLANK_LINE 225u
#define MAX_SLICES 4096

void ResetVector_m1x1(void);
void NmiVector_m1x1(void);
void IrqVector_m1x1(void);

enum { F_MAIN, F_NMI, F_IRQ };
static void *s_driver, *s_fiber[3];
static int s_stack[4], s_depth;         /* fibers by nesting, s_stack[0] = main */
static int s_done;                      /* the running handler returned */
static uint64_t s_deadline;
static int s_booted;

static void park(uint64_t until)
{
    if (g_cpu.master_cycles < until)
        g_cpu.master_cycles = until;
    snes_refresh_exempt();
    snes_sync_master_clock(g_snes, g_cpu.master_cycles);
    cart_sync_coprocessors(g_snes->cart, g_cpu.master_cycles);
}

/* Called by the bus before every access the C makes (port/src/r816_bridge.c). */
void r816_native_tick(uint32_t a)
{
    uint8_t bank = (uint8_t)(a >> 16);
    uint16_t off = (uint16_t)a;
    /* 6 master clocks for FastROM, 8 for WRAM and the rest: the C has no
     * instruction timing of its own, so its accesses stand in for it */
    g_cpu.master_cycles += (bank >= 0x80 && off >= 0x8000) ? 6 : 8;
    if (!(bank & 0x40) && (off >> 8 == 0x21 || off >> 8 == 0x42 || off >> 8 == 0x43))
        snes_sync_master_clock(g_snes, g_cpu.master_cycles);
    if (g_cpu.master_cycles >= s_deadline && GetCurrentFiber() != s_driver)
        SwitchToFiber(s_driver);
}

static void interrupt_frame(void)
{
    push8(PB);
    push16(0xFFFF);                     /* the C keeps its own place */
    push8((uint8_t)(fc | fz << 1 | fi << 2 | fd << 3 | fx << 4 | fm << 5 | fv << 6 | fn << 7));
    fi = 1; fd = 0;
    PB = 0x80;                          /* the vectors jump into bank $80 */
}

static void CALLBACK main_fiber(void *p)
{
    (void)p;
    ResetVector_m1x1();                 /* never returns */
    for (;;)
        SwitchToFiber(s_driver);
}

static void CALLBACK nmi_fiber(void *p)
{
    (void)p;
    for (;;) {
        NmiVector_m1x1();
        s_done = 1;
        SwitchToFiber(s_driver);
    }
}

static void CALLBACK irq_fiber(void *p)
{
    (void)p;
    for (;;) {
        IrqVector_m1x1();
        s_done = 1;
        SwitchToFiber(s_driver);
    }
}

static void take(int f)
{
    interrupt_frame();
    s_stack[++s_depth] = f;
}

static int start(void)
{
    if (IsThreadAFiber())
        s_driver = GetCurrentFiber();
    else
        s_driver = ConvertThreadToFiber(NULL);
    s_fiber[F_MAIN] = CreateFiber(256 * 1024, main_fiber, NULL);
    s_fiber[F_NMI] = CreateFiber(128 * 1024, nmi_fiber, NULL);
    s_fiber[F_IRQ] = CreateFiber(128 * 1024, irq_fiber, NULL);
    if (!s_driver || !s_fiber[F_MAIN] || !s_fiber[F_NMI] || !s_fiber[F_IRQ]) {
        fprintf(stderr, "[native] could not create the fibers\n");
        return 0;
    }
    s_stack[0] = F_MAIN;
    s_depth = 0;
    return 1;
}

static int in_irq(void)
{
    for (int i = 1; i <= s_depth; i++)
        if (s_stack[i] == F_IRQ)
            return 1;
    return 0;
}

void BunbunNativeRunFrame(void)
{
    const int booting = !s_booted;
    int slices = 0;
    if (booting) {
        if (!start()) {
            r816_native = 0;
            return;
        }
        s_booted = 1;
        fprintf(stderr, "[native] running the hand-written C (no recompiled code)\n");
    }
    uint32_t to_vblank = snes_master_clocks_until_line(g_snes, VBLANK_LINE);
    uint64_t frame_end = g_snes->beamMasterLast + (to_vblank ? to_vblank : FIELD_CYCLES);

    if (!booting) {
        ppu_checkOverscan(g_ppu);
        ppu_handleVblank(g_ppu);
    }
    snes_set_hdma_beam_enabled(g_snes, false);
    snes_set_raster_irq_beam_enabled(true);
    ppu_rasterBegin(g_ppu);

    if (!booting && g_snes->nmiEnabled) {
        g_snes->inNmi = true;
        take(F_NMI);
    }

    while (g_cpu.master_cycles < frame_end && slices++ < MAX_SLICES) {
        /* a raster IRQ the beam latched, unless masked or already running */
        if (!fi && g_snes->inIrq && !in_irq())
            take(F_IRQ);
        uint32_t to_irq = snes_master_clocks_until_irq(g_snes);
        if (to_irq)
            to_irq += 1;
        if (to_irq == 0 || to_irq > LINE_CYCLES)
            to_irq = LINE_CYCLES;
        uint64_t next = g_cpu.master_cycles + to_irq;
        if (next > frame_end)
            next = frame_end;
        s_deadline = next;
        s_done = 0;
        SwitchToFiber(s_fiber[s_stack[s_depth]]);
        if (s_done) {
            if (s_stack[s_depth] == F_NMI)
                g_snes->inNmi = false;
            if (s_depth > 0)
                s_depth--;
            continue;                   /* the interrupted code resumes now */
        }
        park(next);
    }
    g_snes->inNmi = false;

    /* walk the beam to the field boundary (V=225), as the beam driver does */
    {
        int walk = 0;
        if (g_cpu.master_cycles < frame_end)
            park(frame_end);
        while (g_snes->beamMasterLast < frame_end && walk++ < 1200) {
            uint64_t before = g_snes->beamMasterLast;
            uint64_t want = frame_end - before;
            snes_advance_master_cycles(g_snes, want > UINT32_MAX ? UINT32_MAX : (uint32_t)want);
            if (g_snes->beamMasterLast == before)
                break;
        }
        cart_sync_coprocessors(g_snes->cart, g_cpu.master_cycles);
    }
}

/* A computed transfer (jump table) to the routine at pc: its hand-written C,
 * the variant for the current register widths. */
static void call_checked(const HandEntry *e)
{
    uint16_t s0 = S;
    e->fn();
    static int check = -1;          /* BUNBUN_NATIVE_CHECK=1: report stack imbalance */
    if (check < 0) { const char *c = getenv("BUNBUN_NATIVE_CHECK"); check = c && *c == '1'; }
    if (check && S != (uint16_t)(s0 + 2) && S != (uint16_t)(s0 + 3)) {
        static int n;
        if (n++ < 16)
            fprintf(stderr, "[native] $%06X (m%dx%d) moved S by %d\n", (unsigned)e->pc, e->m, e->x,
                    (int)(int16_t)(S - s0));
    }
}

int r816_native_call(uint32_t pc24)
{
    const HandEntry *any = NULL;
    for (int i = 0; i < g_hand_table_count; i++) {
        const HandEntry *e = &g_hand_table[i];
        if (e->pc != pc24)
            continue;
        if (e->m == fm && e->x == fx) {
            call_checked(e);
            return 1;
        }
        if (!any)
            any = e;
    }
    if (any) {
        call_checked(any);
        return 1;
    }
    static int warned;
    if (warned++ < 8)
        fprintf(stderr, "[native] no hand-written routine at $%06X (S=%04X, stack %02X %02X %02X %02X, $76=%02X DB=%02X X=%04X)\n",
                (unsigned)pc24, S, g_ram[(uint16_t)(S + 1)], g_ram[(uint16_t)(S + 2)],
                g_ram[(uint16_t)(S + 3)], g_ram[(uint16_t)(S + 4)], g_ram[0x76], DB, X);
    if (warned <= 8)
        fprintf(stderr, "[native]   $0116-$011D: %02X %02X %02X %02X %02X %02X %02X %02X  Y=%04X fm=%d fx=%d\n",
                g_ram[0x116], g_ram[0x117], g_ram[0x118], g_ram[0x119], g_ram[0x11A], g_ram[0x11B],
                g_ram[0x11C], g_ram[0x11D], Y, fm, fx);
    return 0;
}

#else
void BunbunNativeRunFrame(void) { r816_native = 0; }
void r816_native_tick(uint32_t a) { (void)a; }
int r816_native_call(uint32_t pc24) { (void)pc24; return 0; }
#endif
