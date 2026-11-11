/*
 * r816_bridge.c - run hand-written (decompiled) routines inside the port.
 *
 * The hand-written C in c/hand/<ver>/ runs on the machine model of
 * c/runtime816.h. Here that model is bound to snesrecomp's CPU and bus:
 *   - memory goes through cpu_read8/cpu_write8, so WRAM, ROM and every PPU,
 *     APU and DMA register behave as for the recompiled code;
 *   - each replacement is an hle_func: copy the CpuState registers in, run the
 *     hand routine (its ret()/retl() pops the real return frame from the guest
 *     stack, as the hle_func contract requires), copy the registers back.
 *
 * Every routine bridged here passed tools/difftest816.py against the original
 * on the registers its callers read (c/hand/<ver>/hand.txt).
 *
 * Cycles: the hand code does not count cycles, so each wrapper charges a
 * fixed estimate of the original's cost. The frame driver only needs the
 * work of a frame to fit in the frame; exact equality with the plain build is
 * not expected once a routine is bridged.
 */
#include "cpu_state.h"
#include "common_cpu_infra.h"

/* runtime816 machine state (c/runtime816.c), declared here without the
 * header so its short names do not meet snesrecomp's. */
extern uint16_t A, X, Y, S, D;
extern uint8_t DB, PB;
extern uint8_t fn, fz, fc, fv, fd, fi, fm, fx;

uint8_t r816_bus_read(uint32_t a) { return cpu_read8(&g_cpu, (uint8)(a >> 16), (uint16)a); }
void r816_bus_write(uint32_t a, uint8_t v) { cpu_write8(&g_cpu, (uint8)(a >> 16), (uint16)a, v); }

/* The difftest harness provides these; the port has no hardware log. */
uint8_t hw_read(uint16_t reg) { return cpu_read8(&g_cpu, 0x00, reg); }
void hw_write(uint16_t reg, uint8_t v) { cpu_write8(&g_cpu, 0x00, reg, v); }
void open_write(uint32_t a, uint8_t v) { (void)a; (void)v; }

/* Loop guard used by hand code that waits on hardware: never bails here. */
#include <setjmp.h>
jmp_buf r816_bail;
long r816_ticks, r816_limit = 0x7FFFFFFF;

static void from_cpu(CpuState *cpu)
{
    cpu_mirrors_to_p(cpu);
    A = cpu->A; X = cpu->X; Y = cpu->Y; S = cpu->S; D = cpu->D; DB = cpu->DB; PB = cpu->PB;
    uint8 p = cpu->P;
    fc = p & 1; fz = (p >> 1) & 1; fi = (p >> 2) & 1; fd = (p >> 3) & 1;
    fx = (p >> 4) & 1; fm = (p >> 5) & 1; fv = (p >> 6) & 1; fn = p >> 7;
}

static void to_cpu(CpuState *cpu, uint32 cycles)
{
    cpu->A = A; cpu->X = X; cpu->Y = Y; cpu->S = S; cpu->D = D; cpu->DB = DB;
    cpu->P = (uint8)(fc | fz << 1 | fi << 2 | fd << 3 | fx << 4 | fm << 5 | fv << 6 | fn << 7);
    cpu_p_to_mirrors(cpu);
    cpu->cycles += cycles;
    cpu->master_cycles += (uint64_t)cycles * 8;
}

#define BRIDGE(hle_name, hand_fn, cycles)                 \
    void hand_fn(void);                                   \
    RecompReturn hle_name(CpuState *cpu)                  \
    {                                                     \
        from_cpu(cpu);                                    \
        hand_fn();                                        \
        to_cpu(cpu, cycles);                              \
        return RECOMP_RETURN_NORMAL;                      \
    }

#ifndef BUNBUN_GENB
/* Stadium 1-2 (Gen A) routines from c/hand/s1/kernel.c */
BRIDGE(HleHand_QueueSound,    QueueSound_m1x1,    60)
BRIDGE(HleHand_DmaCopyToWram, DmaCopyToWram_m0x1, 60)
BRIDGE(HleHand_DmaFillZero,   DmaFillZero_m0x1,   70)
BRIDGE(HleHand_LoadCourse,    LoadCourse_m1x1,    30000)
BRIDGE(HleHand_BsxPollStream, BsxPollStream_m1x1, 120)
/* c/hand/s1/vram.c */
BRIDGE(HleHand_VramQueueCopy,    VramQueueCopy_m1x1,    1500)
BRIDGE(HleHand_VramQueueTiles,   VramQueueTiles_m1x1,   1200)
BRIDGE(HleHand_VramQueueTiles20, VramQueueTiles20_m1x1, 1200)
BRIDGE(HleHand_NmiFlushVramQueue, NmiFlushVramQueue_m1x1, 400)
BRIDGE(HleHand_NmiFixedUploads,   NmiFixedUploads_m1x1,   600)
/* c/hand/s1/frame.c */
BRIDGE(HleHand_StepLfsr,         StepLfsr_m1x1,         80)
BRIDGE(HleHand_ClearOamBuffer,   ClearOamBuffer_m1x1,   1500)
BRIDGE(HleHand_PackOamHighTable, PackOamHighTable_m1x1, 900)
#endif
