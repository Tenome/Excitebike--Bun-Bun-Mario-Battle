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

/* Optional DMA catalogue (BUNBUN_DMALOG=<file>): every distinct DMA started
 * by hand code (all of the game's uploads go through DmaKick,
 * NmiFlushVramQueue, NmiFixedUploads and friends) is written once with the
 * PPU state that tells what it is: destination register and address (VRAM
 * word, CGRAM entry or OAM), BG mode, tile and map bases, OBSEL and the game
 * mode $76. tools/assets.py turns it into the asset list. */
#include <stdio.h>
#include <stdlib.h>
#include "snes/ppu.h"
#include "common_rtl.h"
#include "snes/snes.h"
#include "snes/cart.h"
static uint32_t s_wmadd;                /* WMADD ($2181-$2183) as hand code set it */
static void dma_catalogue(uint8 enable)
{
    static FILE *f;
    static int init;
    static uint64_t seen[4096];
    static int nseen;
    if (!init) {
        const char *e = getenv("BUNBUN_DMALOG");
        init = 1;
        if (e && *e) {
            f = fopen(e, "w");
            if (f) fprintf(f, "# src size bbad dmap dest bgmode bgTileAdr bgsc0 bgsc1 bgsc2 bgsc3 obsel mode\n");
        }
    }
    if (!f || !g_ppu)
        return;
    for (int c = 0; c < 8; c++) {
        if (!(enable & (1 << c)))
            continue;
        uint16 base = (uint16)(0x4300 + c * 16);
        uint8 dmap = cpu_read8(&g_cpu, 0, base), bbad = cpu_read8(&g_cpu, 0, base + 1);
        uint32_t src = cpu_read8(&g_cpu, 0, base + 2) | cpu_read8(&g_cpu, 0, base + 3) << 8 |
                       (uint32_t)cpu_read8(&g_cpu, 0, base + 4) << 16;
        uint32_t size = cpu_read8(&g_cpu, 0, base + 5) | cpu_read8(&g_cpu, 0, base + 6) << 8;
        if (!size) size = 0x10000;
        uint16 dest = 0;
        if (bbad == 0x18 || bbad == 0x19) dest = g_ppu->vramPointer;
        else if (bbad == 0x22) dest = g_ppu->cgramPointer;
        else if (bbad == 0x04) dest = g_ppu->oamAdr;
        else if (bbad == 0x80) dest = (uint16)s_wmadd;   /* WRAM (bank in bit 16) */
        uint64_t key = (uint64_t)src | (uint64_t)size << 24 | (uint64_t)bbad << 41 | (uint64_t)dest << 49;
        int dup = 0;
        for (int i = 0; i < nseen; i++)
            if (seen[i] == key) { dup = 1; break; }
        if (dup)
            continue;
        if (nseen < 4096)
            seen[nseen++] = key;
        fprintf(f, "%06X %05X %02X %02X %04X %02X %04X %02X %02X %02X %02X %02X %02X\n",
                (unsigned)src, (unsigned)size, bbad, dmap, dest, g_ppu->bgmode,
                g_ppu->bgTileAdr, g_ppu->bgXsc[0], g_ppu->bgXsc[1], g_ppu->bgXsc[2],
                g_ppu->bgXsc[3], g_ppu->obsel, g_ram[0x76]);
        fflush(f);
    }
}

void r816_bus_write(uint32_t a, uint8_t v)
{
    if (((a >> 16) & 0x40) == 0) {
        uint16 r = (uint16)a;
        if (r == 0x420B && v)
            dma_catalogue(v);
        else if (r == 0x2181) s_wmadd = (s_wmadd & 0x1FF00) | v;
        else if (r == 0x2182) s_wmadd = (s_wmadd & 0x100FF) | (uint32_t)v << 8;
        else if (r == 0x2183) s_wmadd = (s_wmadd & 0x0FFFF) | (uint32_t)(v & 1) << 16;
    }
    cpu_write8(&g_cpu, (uint8)(a >> 16), (uint16)a, v);
}

/* The difftest harness provides these; the port has no hardware log. */
uint8_t hw_read(uint16_t reg) { return cpu_read8(&g_cpu, 0x00, reg); }
void hw_write(uint16_t reg, uint8_t v) { r816_bus_write(reg, v); }
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

/* Hand code calling a recompiled routine whose return frame it has already
 * pushed (CALL in c/hand/hand.h). Only safe for callees that never yield to
 * the frame scheduler (see the note at the end of this file). */
int r816_call_native(uint32_t pc24, int frame)
{
    to_cpu(&g_cpu, 0);
    uint32 ret_pc = 0;
    cpu_dispatch_call_pc_pushed(&g_cpu, pc24, pc24, (uint8)frame, &ret_pc);
    from_cpu(&g_cpu);
    return 1;
}

/* Hand code tail-jumping to a routine by address (JMP ($0000) dispatch):
 * the target returns to our caller through the frame already on the stack. */
static int g_tail_pending;
static RecompReturn g_tail_result;

int r816_call_pc(uint32_t pc24)
{
    to_cpu(&g_cpu, 0);
    g_tail_result = cpu_dispatch_pc_from(&g_cpu, 0x800000u | pc24, g_cpu.S, pc24);
    g_tail_pending = 1;          /* the hand routine returns right after this */
    from_cpu(&g_cpu);
    return 1;
}

#define BRIDGE(hle_name, hand_fn, cycles)                 \
    void hand_fn(void);                                   \
    RecompReturn hle_name(CpuState *cpu)                  \
    {                                                     \
        from_cpu(cpu);                                    \
        g_tail_pending = 0;                               \
        hand_fn();                                        \
        to_cpu(cpu, cycles);                              \
        /* a tail transfer hands its result (a scheduler   \
         * yield included) back up, as compiled code does */ \
        return g_tail_pending ? g_tail_result : RECOMP_RETURN_NORMAL; \
    }

/* As BRIDGE, with the cycle cost read after the call (for routines whose
 * work depends on data they only see while running). */
#define BRIDGE_AFTER(hle_name, hand_fn, cycles_expr)          void hand_fn(void);                                       RecompReturn hle_name(CpuState *cpu)                      {                                                             from_cpu(cpu);                                            g_tail_pending = 0;                                       hand_fn();                                                to_cpu(cpu, (uint32)(cycles_expr));                       return g_tail_pending ? g_tail_result : RECOMP_RETURN_NORMAL;     }

/* As BRIDGE, with the cycle cost worked out before the call (for routines
 * that do real work only on some frames). */
#define BRIDGE_DYN(hle_name, hand_fn, cycles_expr)        \
    void hand_fn(void);                                   \
    RecompReturn hle_name(CpuState *cpu)                  \
    {                                                     \
        uint32 cyc_;                                      \
        from_cpu(cpu);                                    \
        g_tail_pending = 0;                               \
        cyc_ = (uint32)(cycles_expr);                     \
        hand_fn();                                        \
        to_cpu(cpu, cyc_);                                \
        return g_tail_pending ? g_tail_result : RECOMP_RETURN_NORMAL; \
    }

#ifndef BUNBUN_GENB
/* Stadium 1-2 (Gen A) routines from c/hand/s1/kernel.c */
BRIDGE(HleHand_QueueSound,    QueueSound_m1x1,    60)
BRIDGE(HleHand_DmaCopyToWram, DmaCopyToWram_m0x1, 60)
BRIDGE(HleHand_DmaFillZero,   DmaFillZero_m0x1,   70)
/* LoadCourse with custom courses: after the original expands the course into
 * the track buffer $7F:4000, a mods/<mod>/courses/courseN.bin (one byte per
 * column, as tools/course.py --dump and the course editor write it; N = the
 * table index, Stadium * 4 + course, 8-9 the special courses) replaces it.
 * Only when the routine really loaded (not on a later lap). */
int BunbunCustomCourse(int index, uint8_t *buf, int cap);
void LoadCourse_m1x1(void);
RecompReturn HleHand_LoadCourse(CpuState *cpu)
{
    int loads = !(g_ram[0x14000] | g_ram[0x14001]) || g_ram[0x7D] == 0;
    from_cpu(cpu);
    g_tail_pending = 0;
    LoadCourse_m1x1();
    to_cpu(cpu, 30000);
    if (loads && g_snes && g_snes->cart) {
        int st = g_snes->cart->rom[0x800C];                     /* $81:800C */
        int idx = (g_ram[0x0CD7] | g_ram[0x0CD8]) ? st * 2 + 8 : st * 4 + g_ram[0x7C];
        BunbunCustomCourse(idx, &g_ram[0x14000], 0x1000);       /* $7F:4000 */
    }
    return RECOMP_RETURN_NORMAL;
}
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
/* c/hand/s1/misc.c */
BRIDGE(HleHand_Random,           Random_m1x1,           60)
BRIDGE(HleHand_TickTimers,       TickTimers_m1x1,       400)
BRIDGE(HleHand_DrawClock,        DrawClock_m1x1,        500)
BRIDGE(HleHand_Mode00_IntroWait, Mode00_IntroWait_m1x1, 60)
BRIDGE(HleHand_Mode0E_TitleWait, Mode0E_TitleWait_m1x1, 40)
/* c/hand/s1/init.c */
BRIDGE(HleHand_ForceBlank,       ForceBlank_m1x1,       30)
BRIDGE(HleHand_ClearTilemaps,    ClearTilemaps_m1x1,    200)
BRIDGE(HleHand_InitPpuRegisters, InitPpuRegisters_m1x1, 600)
BRIDGE(HleHand_FadeStep,             FadeStep_m1x1,             60)
BRIDGE(HleHand_BroadcastEventInject, BroadcastEventInject_m1x1, 120)
/* c/hand/s1/title.c */
BRIDGE(HleHand_FinishModeSetup,  FinishModeSetup_m1x1,  40)
BRIDGE(HleHand_Mode0D_Setup,     Mode0D_Setup_m1x1,     80)
BRIDGE(HleHand_Mode0E_Setup,     Mode0E_Setup_m1x1,     2500)
BRIDGE(HleHand_Mode0D_IntroText, Mode0D_IntroText_m1x1, 1500)
/* c/hand/s1/card.c (Mode02_SegmentCard tail-jumps into recompiled code) */
BRIDGE(HleHand_Mode02_Setup,       Mode02_Setup_m1x1,       1500)
BRIDGE(HleHand_Mode02_SegmentCard, Mode02_SegmentCard_m1x1, 40)
BRIDGE(HleHand_SegmentNext,        SegmentNext_m1x1,        40)
/* c/hand/s1/race.c */
BRIDGE(HleHand_EnableHudSplit,     EnableHudSplit_m1x1,     20)
BRIDGE(HleHand_ClearRaceState,     ClearRaceState_m1x1,     2000)
BRIDGE(HleHand_DrawRacerName,      DrawRacerName_m1x1,      800)
BRIDGE(HleHand_DrawHudLabelA,      DrawHudLabelA_m1x1,      800)
BRIDGE(HleHand_DrawHudLabelB,      DrawHudLabelB_m1x1,      800)
BRIDGE(HleHand_DrawPlace,      DrawPlace_m1x1,      500)
BRIDGE(HleHand_DrawLap,       DrawLap_m1x1,       500)
BRIDGE(HleHand_RaceHudInit,        RaceHudInit_m1x1,        2000)
BRIDGE(HleHand_RaceSetup,          RaceSetup_m1x1,          6000)
/* c/hand/s1/objects.c */
BRIDGE(HleHand_ClipRiderSprite,    ClipRiderSprite_m1x1,    60)
BRIDGE(HleHand_StepObjectX,        StepObjectX_m1x1,        80)
BRIDGE(HleHand_DrawSmallObject,    DrawSmallObject_m1x1,    80)
BRIDGE(HleHand_UpdatePlayerPlace,  UpdatePlayerPlace_m1x1,  200)
BRIDGE(HleHand_ComputePlaces,      ComputePlaces_m1x1,      1500)
BRIDGE(HleHand_DrawCoinIcons,      DrawCoinIcons_m1x1,      200)
/* c/hand/s1/ai.c */
BRIDGE(HleHand_RivalWaitStep_B819, RivalWaitStep_B819_m1x1, 40)
BRIDGE(HleHand_RivalWaitStep_B879, RivalWaitStep_B879_m1x1, 40)
BRIDGE(HleHand_RivalWaitStep_B9B6, RivalWaitStep_B9B6_m1x1, 40)
BRIDGE(HleHand_RivalWaitStep_BA8E, RivalWaitStep_BA8E_m1x1, 40)
BRIDGE(HleHand_RivalWaitStep_BAE8, RivalWaitStep_BAE8_m1x1, 40)
BRIDGE(HleHand_RivalWaitStep_BC2E, RivalWaitStep_BC2E_m1x1, 40)
BRIDGE(HleHand_RivalWaitStep_BD06, RivalWaitStep_BD06_m1x1, 40)
BRIDGE(HleHand_RivalWaitRandom_B7B9, RivalWaitRandom_B7B9_m1x1, 120)
BRIDGE(HleHand_RivalWaitRandom_B8ED, RivalWaitRandom_B8ED_m1x1, 120)
BRIDGE(HleHand_RivalWaitRandom_BA2E, RivalWaitRandom_BA2E_m1x1, 120)
BRIDGE(HleHand_RivalWaitRandom_BB49, RivalWaitRandom_BB49_m1x1, 120)
BRIDGE(HleHand_RivalWaitRandom_BCA1, RivalWaitRandom_BCA1_m1x1, 120)
BRIDGE(HleHand_RivalAlternate_B7E0, RivalAlternate_B7E0_m1x1, 120)
BRIDGE(HleHand_RivalAlternate_B914, RivalAlternate_B914_m1x1, 120)
BRIDGE(HleHand_RivalAlternate_BA55, RivalAlternate_BA55_m1x1, 120)
BRIDGE(HleHand_RivalAlternate_BB70, RivalAlternate_BB70_m1x1, 120)
BRIDGE(HleHand_RivalDispatch_9761, RivalDispatch_9761_m1x1, 30)
BRIDGE(HleHand_RivalDispatch_B639, RivalDispatch_B639_m1x1, 30)
BRIDGE(HleHand_RivalDispatch_B6B3, RivalDispatch_B6B3_m1x1, 30)
BRIDGE(HleHand_RivalDispatch_B7A2, RivalDispatch_B7A2_m1x1, 30)
BRIDGE(HleHand_RivalDispatch_B8D6, RivalDispatch_B8D6_m1x1, 30)
BRIDGE(HleHand_RivalDispatch_BA17, RivalDispatch_BA17_m1x1, 30)
BRIDGE(HleHand_RivalDispatch_BB32, RivalDispatch_BB32_m1x1, 30)
BRIDGE(HleHand_RivalDispatch_BC8A, RivalDispatch_BC8A_m1x1, 30)
BRIDGE(HleHand_RivalDispatch_BD52, RivalDispatch_BD52_m1x1, 30)
BRIDGE(HleHand_ApplyVelocity,      ApplyVelocity_m1x1,      40)
BRIDGE(HleHand_RivalTumble_B64A,   RivalTumble_B64A_m1x1,   120)
BRIDGE(HleHand_RivalSequence_B67F, RivalSequence_B67F_m1x1, 40)
BRIDGE(HleHand_RivalSequence_BB05, RivalSequence_BB05_m1x1, 40)
BRIDGE(HleHand_ClearWords1530,   ClearWords1530_m1x1,   40)
BRIDGE(HleHand_ResetMinuteFrame, ResetMinuteFrame_m1x1, 30)
BRIDGE(HleHand_SumBytes0850,     SumBytes0850_m1x1,     100)
BRIDGE(HleHand_Nop_BD39,         Nop_BD39_m1x1,         20)
BRIDGE(HleHand_AddSpeedFraction, AddSpeedFraction_m1x1, 40)
BRIDGE(HleHand_Tick08CC,            Tick08CC_m1x1,            40)
BRIDGE(HleHand_DivideAX,            DivideAX_m1x1,            100)
BRIDGE(HleHand_BlankQueueRow,       BlankQueueRow_m1x1,       100)
BRIDGE(HleHand_BlankQueueRowPal2,   BlankQueueRowPal2_m1x1,   80)
BRIDGE(HleHand_RivalCountdown_97CC, RivalCountdown_97CC_m1x1, 40)
BRIDGE(HleHand_DrawText95C4,        DrawText95C4_m1x1,        200)
BRIDGE(HleHand_BeginFadeIn,         BeginFadeIn_m1x1,         40)
BRIDGE(HleHand_SendSpeedToApu,      SendSpeedToApu_m1x1,      40)
BRIDGE(HleHand_RiderShadowY,        RiderShadowY_m1x1,        40)
BRIDGE(HleHand_ScrollByPlayerSpeed, ScrollByPlayerSpeed_m1x1, 40)
BRIDGE(HleHand_Add32,               Add32_m1x1,               40)
BRIDGE(HleHand_DispatchBy85,        DispatchBy85_m1x1,        40)
BRIDGE(HleHand_StartMode01,         StartMode01_m1x1,         40)
BRIDGE(HleHand_AnimSourceFromFrame, AnimSourceFromFrame_m1x1, 40)
BRIDGE(HleHand_InitFadeState, InitFadeState_m1x1, 80)
BRIDGE(HleHand_BeginFadeInBg3, BeginFadeInBg3_m1x1, 80)
BRIDGE(HleHand_BsxReadSerialByte, BsxReadSerialByte_m1x1, 80)
BRIDGE(HleHand_GrowBar087E, GrowBar087E_m1x1, 80)
BRIDGE(HleHand_StepColourCycle, StepColourCycle_m1x1, 80)
BRIDGE(HleHand_MergeBsxFlags, MergeBsxFlags_m1x1, 80)
BRIDGE(HleHand_ScrollFarLayer, ScrollFarLayer_m1x1, 80)
BRIDGE(HleHand_LaneFromY, LaneFromY_m1x1, 80)
BRIDGE(HleHand_MarkFastRider, MarkFastRider_m1x1, 80)
BRIDGE(HleHand_DmaKick, DmaKick_m0x1, 20)
BRIDGE(HleHand_RivalPause_B94D, RivalPause_B94D_m1x1, 120)
BRIDGE(HleHand_RivalSequence_BD23, RivalSequence_BD23_m1x1, 40)
BRIDGE(HleHand_DrawRiderShadowPair, DrawRiderShadowPair_m1x1, 60)
BRIDGE(HleHand_PlayerOverheatWarn, PlayerOverheatWarn_m1x1, 100)
BRIDGE(HleHand_ScrollRivalsRelative, ScrollRivalsRelative_m1x1, 200)
BRIDGE(HleHand_EndScreenTimeout, EndScreenTimeout_m1x1, 200)
BRIDGE(HleHand_RivalWalkAway_BECE, RivalWalkAway_BECE_m1x1, 60)
BRIDGE(HleHand_RiderLaneBand, RiderLaneBand_m1x1, 80)
BRIDGE(HleHand_AnimateWheels, AnimateWheels_m1x1, 200)
BRIDGE(HleHand_SetupSpriteRow15, SetupSpriteRow15_m1x1, 400)
BRIDGE(HleHand_RivalSequence_B9D5, RivalSequence_B9D5_m1x1, 120)
BRIDGE(HleHand_RivalSequence_BAAD, RivalSequence_BAAD_m1x1, 120)
BRIDGE(HleHand_RivalSequence_BC4B, RivalSequence_BC4B_m1x1, 120)
BRIDGE(HleHand_BsxReadSerial3, BsxReadSerial3_m1x1, 400)
BRIDGE(HleHand_RiderHeightStep, RiderHeightStep_m1x1, 60)
BRIDGE(HleHand_RivalSequence_B896, RivalSequence_B896_m1x1, 120)
BRIDGE(HleHand_RivalCycle_BCC8, RivalCycle_BCC8_m1x1, 120)
BRIDGE(HleHand_RivalCycle_B976, RivalCycle_B976_m1x1, 120)
BRIDGE(HleHand_RivalSequence_B836, RivalSequence_B836_m1x1, 120)
BRIDGE(HleHand_RivalBlink_BBA9, RivalBlink_BBA9_m1x1, 120)
BRIDGE(HleHand_ObjectFacing, ObjectFacing_m1x1, 120)
BRIDGE(HleHand_RiderStartCheck, RiderStartCheck_m1x1, 80)
BRIDGE(HleHand_AddScore, AddScore_m1x1, 200)
BRIDGE(HleHand_AccelerateRider, AccelerateRider_m1x1, 80)
BRIDGE(HleHand_ClearRiderState, ClearRiderState_m1x1, 150)
BRIDGE(HleHand_ApplyVelocityX, ApplyVelocityX_m1x1, 40)
BRIDGE(HleHand_SpawnPlayerSpark, SpawnPlayerSpark_m1x1, 80)
BRIDGE(HleHand_StepSpark, StepSpark_m1x1, 80)
BRIDGE(HleHand_ClearMidScreenRiders, ClearMidScreenRiders_m1x1, 300)
BRIDGE(HleHand_BsxCheckSerial, BsxCheckSerial_m1x1, 400)
/* Not bridged: RunGameModeScript (c/hand/s1/modes.c). Its reset path makes
 * non-tail calls into recompiled routines, and a recompiled routine may yield
 * to the frame scheduler mid-call (a long one like $80:B873 does), which hand
 * C cannot resume. Rule: bridge only routines whose calls stay inside
 * hand-written code. */
/* carried over from Stadium 4 (c/hand/s1/from_s4.c) */
BRIDGE(HleHand_SinCos, SinCos_m1x1, 150)
BRIDGE(HleHand_Reach20, Reach20_m1x1, 20)
BRIDGE(HleHand_Reach2, Reach2_m1x1, 20)
BRIDGE(HleHand_RiderStartLane, RiderStartLane_m1x1, 25)
BRIDGE(HleHand_StepFallingSprite, StepFallingSprite_m1x1, 60)
BRIDGE(HleHand_SortRidersByPosition, SortRidersByPosition_m1x1, 700)
BRIDGE(HleHand_SetRiderLean, SetRiderLean_m1x1, 35)
BRIDGE(HleHand_DrawSpark, DrawSpark_m1x1, 60)
BRIDGE(HleHand_SpawnDustPuff, SpawnDustPuff_m1x1, 70)
BRIDGE(HleHand_MoveObjectX, MoveObjectX_m1x1, 70)
BRIDGE(HleHand_DrawDustPair, DrawDustPair_m1x1, 80)
BRIDGE_DYN(HleHand_FadeSkyColours, FadeSkyColours_m1x1, (g_ram[0x63] & 7) ? 15 : 15 + 45 * 4)
BRIDGE(HleHand_DrawBigSprite, DrawBigSprite_m1x1, 120)
BRIDGE(HleHand_SpawnExhaust, SpawnExhaust_m1x1, 70)
BRIDGE(HleHand_MirrorJumpRider, MirrorJumpRider_m1x1, 240)
BRIDGE(HleHand_VelocityDirection, VelocityDirection_m1x1, 120)
BRIDGE(HleHand_DrawBigBall, DrawBigBall_m1x1, 90)
BRIDGE(HleHand_CopyTrackColumn, CopyTrackColumn_m1x1, 400)
BRIDGE(HleHand_CpuReleaseButtons, CpuReleaseButtons_m1x1, 25)
BRIDGE_DYN(HleHand_AimRatio, AimRatio_m1x1, (A & 0xFF) ? 120 + 25 * (A & 0xFF) : 15)
BRIDGE(HleHand_RiderRunOut, RiderRunOut_m1x1, 50)
BRIDGE(HleHand_Mode11, Mode11_m1x1, 30)
BRIDGE(HleHand_ItemFlashState, ItemFlashState_m1x1, 30)
BRIDGE(HleHand_EmptyHook, EmptyHook_m1x1, 5)
BRIDGE(HleHand_StepObjectX8, StepObjectX8_m1x1, 88)
BRIDGE(HleHand_EmptyStep9828, EmptyStep9828_m1x1, 6)
BRIDGE(HleHand_EmptyStep9554, EmptyStep9554_m1x1, 6)
BRIDGE(HleHand_FlagRidersAtLine, FlagRidersAtLine_m1x1, 140)
BRIDGE(HleHand_LanePieceTables, LanePieceTables_m1x1, 15)
BRIDGE(HleHand_HeightAbove, HeightAbove_m1x1, 18)
BRIDGE(HleHand_GroundHeightAbove, GroundHeightAbove_m1x1, 21)
BRIDGE(HleHand_FindObjectSlot, FindObjectSlot_m1x1, 60)
BRIDGE(HleHand_MoveObjectX8, MoveObjectX8_m1x1, 76)
BRIDGE(HleHand_CpuThrottle, CpuThrottle_m1x1, 35)
BRIDGE(HleHand_CpuReleaseDpad, CpuReleaseDpad_m1x1, 15)
BRIDGE(HleHand_CpuSteer, CpuSteer_m1x1, 45)
BRIDGE_DYN(HleHand_AimVelocity, AimVelocity_m1x1, (A & 0xFF) ? 140 + 25 * (A & 0xFF) : 35)
BRIDGE(HleHand_PlayerHalt, PlayerHalt_m1x1, 30)
BRIDGE(HleHand_PlayerStop, PlayerStop_m1x1, 40)
BRIDGE(HleHand_PlayerColumnCheck, PlayerColumnCheck_m1x1, 60)
BRIDGE(HleHand_MarkerObjectStep, MarkerObjectStep_m1x1, 260)
BRIDGE(HleHand_LanePieceAt, LanePieceAt_m1x1, 58)
BRIDGE(HleHand_RiderPieceAt, RiderPieceAt_m1x1, 63)
BRIDGE(HleHand_LanePieceTablesLong, LanePieceTablesLong_m1x1, 36)
BRIDGE(HleHand_Mode0F_EndScreen, Mode0F_EndScreen_m1x1, 190)
BRIDGE(HleHand_Mode0A, Mode0A_m1x1, 120)
BRIDGE(HleHand_ShowRaceClock, ShowRaceClock_m1x1, 1320)
BRIDGE(HleHand_TickRaceClock, TickRaceClock_m1x1, 1340)
BRIDGE(HleHand_RiderAhead, RiderAhead_m1x1, 150)
BRIDGE_DYN(HleHand_CrossFinish, CrossFinish_m1x1, ((X & 0xFF) == 0 && !g_ram[0x03AF] && !g_ram[0x84]) ? 250 : 10)
BRIDGE_DYN(HleHand_BonusBannerSprites, BonusBannerSprites_m1x1, g_ram[0xBB] ? 90 : 8)
BRIDGE_DYN(HleHand_RaceBannerSprites, RaceBannerSprites_m1x1, g_ram[0xBB] ? 90 : 8)
BRIDGE(HleHand_QueueScoreDigits, QueueScoreDigits_m1x1, 850)
BRIDGE_DYN(HleHand_EndScreenPrompt, EndScreenPrompt_m1x1, ((g_ram[0x26] | g_ram[0x21]) & 0x80) ? 700 : 15)
BRIDGE(HleHand_EndScreenScrollIn, EndScreenScrollIn_m1x1, 30)
BRIDGE(HleHand_SkyFadeObject, SkyFadeObject_m1x1, 200)
BRIDGE(HleHand_IdleCycleObject, IdleCycleObject_m1x1, 40)
BRIDGE(HleHand_FillLaneColumn, FillLaneColumn_m1x1, 490)
BRIDGE(HleHand_QueueFiveDigits, QueueFiveDigits_m1x1, 760)
BRIDGE(HleHand_CourseColumn, CourseColumn_m1x1, 280)
BRIDGE_DYN(HleHand_MarkerObjects, MarkerObjects_m1x1, 40 + ((g_ram[0x63] & 7) ? 0 : 60) + 280 * (!!g_ram[0x1530] + !!g_ram[0x1532] + !!g_ram[0x1534] + !!g_ram[0x1536]))
BRIDGE(HleHand_SparkPieceStep, SparkPieceStep_m1x1, 90)
BRIDGE_DYN(HleHand_EndScreenScore, EndScreenScore_m1x1, g_ram[0x88] == 1 ? 820 : 15)
BRIDGE_DYN(HleHand_EndScreenCode, EndScreenCode_m1x1, 20 + (g_ram[0x88] == 1 ? 680 : 0) + ((g_ram[0x88] <= 1 && (g_ram[0x26] & 0x80)) ? 900 : 0))
BRIDGE_DYN(HleHand_StartJump, StartJump_m1x1, (X & 0xFF) ? 50 : 130)
BRIDGE(HleHand_RiderOnRamp, RiderOnRamp_m1x1, 40)
BRIDGE(HleHand_BallHit, BallHit_m1x1, 50)
BRIDGE(HleHand_SelectWheelPosition, SelectWheelPosition_m1x1, 180)
BRIDGE(HleHand_UpdateRiderDistances, UpdateRiderDistances_m1x1, 180)
BRIDGE(HleHand_CrashLand, CrashLand_m1x1, 150)
BRIDGE(HleHand_RiderLandCheck, RiderLandCheck_m1x1, 150)
BRIDGE(HleHand_SpeedBumpCrash, SpeedBumpCrash_m1x1, 160)
BRIDGE_DYN(HleHand_BallOrGauge, BallOrGauge_m1x1, !(g_ram[0x63] & 1) ? 60 : ((g_ram[0x63] & 2) ? 10 : 150))
BRIDGE(HleHand_TurnSelectWheels, TurnSelectWheels_m1x1, 820)
BRIDGE(HleHand_SlowPieceStep, SlowPieceStep_m1x1, 160)
BRIDGE_DYN(HleHand_DriftUpObject, DriftUpObject_m1x1, (g_ram[0x63] & 3) ? 20 : 80)
BRIDGE(HleHand_SelectTimerDigits, SelectTimerDigits_m1x1, 750)
BRIDGE(HleHand_ShowBonusBest, ShowBonusBest_m1x1, 1300)
BRIDGE(HleHand_PaceBonus, PaceBonus_m1x1, 40)
BRIDGE(HleHand_Mul16, Mul16_m1x1, 230)
BRIDGE(HleHand_Mul16c, Mul16c_m1x1, 230)
BRIDGE(HleHand_ShowBestTime, ShowBestTime_m1x1, 1320)
BRIDGE(HleHand_ConfettiStep, ConfettiStep_m1x1, 250)
BRIDGE(HleHand_SpawnObject, SpawnObject_m1x1, 200)
BRIDGE(HleHand_PodiumWalkIn, PodiumWalkIn_m1x1, 40)
BRIDGE(HleHand_FrameSequenceA, FrameSequenceA_m1x1, 40)
BRIDGE(HleHand_FrameSequenceB, FrameSequenceB_m1x1, 40)
BRIDGE(HleHand_ReleaseBalloons, ReleaseBalloons_m1x1, 50)
BRIDGE(HleHand_RaceClockStep, RaceClockStep_m1x1, 1355)
BRIDGE_DYN(HleHand_RaceTimer, RaceTimer_m1x1, g_ram[0x8E] ? 1500 : 8)
BRIDGE(HleHand_CoinCollected, CoinCollected_m1x1, 260)
BRIDGE_DYN(HleHand_EndScreenChoice, EndScreenChoice_m1x1, 20 + (g_ram[0x88] == 1 ? 680 : 0) + ((g_ram[0x88] <= 1 && (g_ram[0x26] & 0x80)) ? 800 : 0))
BRIDGE(HleHand_BonusResultStart, BonusResultStart_m1x1, 2000)
BRIDGE_DYN(HleHand_BonusCountUp, BonusCountUp_m1x1, g_ram[0x88] == 1 ? 700 : 12)
BRIDGE(HleHand_RivalTarget, RivalTarget_m1x1, 60)
BRIDGE_DYN(HleHand_RiderLanding, RiderLanding_m1x1, g_ram[0x01E8 + (X & 0xFF)] ? 90 : 8)
BRIDGE_DYN(HleHand_LaunchBall, LaunchBall_m1x1, g_ram[0x092C] ? 80 : 8)
BRIDGE_DYN(HleHand_RaceMoveStep, RaceMoveStep_m1x1, g_ram[0x8E] ? 2500 : 8)
BRIDGE(HleHand_RetargetRival, RetargetRival_m1x1, 60)
BRIDGE_DYN(HleHand_WheelSpark, WheelSpark_m1x1, g_ram[0x08BC + (X & 0xFF)] ? 120 : 8)
BRIDGE(HleHand_PieceFlatish, PieceFlatish_m1x1, 14)
BRIDGE(HleHand_PieceOK, PieceOK_m1x1, 80)
BRIDGE(HleHand_CpuSteerUp, CpuSteerUp_m1x1, 150)
BRIDGE(HleHand_CpuSteerDown, CpuSteerDown_m1x1, 150)
BRIDGE(HleHand_CpuLaneDecision, CpuLaneDecision_m1x1, 150)
BRIDGE(HleHand_CpuAvoidHurdle, CpuAvoidHurdle_m1x1, 180)
static uint32 cpu_riders_cost1(void)
{
    uint32 c = 10;
    for (int x = 1; x < 8; x++) {
        if (g_ram[0x01D8 + x] && !(g_ram[0x01C8 + x] | g_ram[0x01D0 + x]))
            c += g_ram[0x01E8 + x] ? 60 : 450;
        else
            c += 20;
    }
    return c;
}
BRIDGE_DYN(HleHand_CpuRidersStep, CpuRidersStep_m1x1, cpu_riders_cost1())
BRIDGE(HleHand_DrawSparksAndMarkers, DrawSparksAndMarkers_m1x1, 1000)
BRIDGE(HleHand_PlaceRival, PlaceRival_m1x1, 380)
BRIDGE(HleHand_SpawnRivals, SpawnRivals_m1x1, 420)
BRIDGE(HleHand_WalkBackOn, WalkBackOn_m1x1, 50)
BRIDGE(HleHand_PlayerTileState, PlayerTileState_m1x1, 70)
BRIDGE(HleHand_PackEntryNumber, PackEntryNumber_m1x1, 1100)
BRIDGE_DYN(HleHand_FadePaletteStep, FadePaletteStep_m1x1, (g_ram[0x63] & 7) ? 12 : 30 + 70 * (g_ram[0xB1] + 2) / 2)
BRIDGE(HleHand_PackRecordTime, PackRecordTime_m1x1, 850)
BRIDGE(HleHand_SpriteTextRow, SpriteTextRow_m1x1, 220)
BRIDGE(HleHand_RecordCourseTime, RecordCourseTime_m1x1, 1420)
BRIDGE(HleHand_FadeInObject, FadeInObject_m1x1, 80)
BRIDGE(HleHand_ShowBonusLabel, ShowBonusLabel_m1x1, 900)
BRIDGE(HleHand_RiderPose, RiderPose_m1x1, 45)
BRIDGE_DYN(HleHand_CrashRecovery, CrashRecovery_m1x1, ((g_ram[0x63] & 3) == g_ram[0x0998]) ? 300 : 15)
BRIDGE_DYN(HleHand_PlayerCrashStep, PlayerCrashStep_m1x1, g_ram[0x01C8] ? 120 : 8)
BRIDGE(HleHand_BonusTimerStep, BonusTimerStep_m1x1, 900)
BRIDGE(HleHand_DrawRider, DrawRider_m1x1, 330)
BRIDGE_DYN(HleHand_SpeedGauge, SpeedGauge_m1x1, (g_ram[0x092D] && !(g_ram[0x7F] & 1)) ? 260 : 15)
BRIDGE_DYN(HleHand_SteerObjectStep, SteerObjectStep_m1x1, (A & 0xFF) ? 330 + 25 * (A & 0xFF) : 220)
BRIDGE(HleHand_BrakeRider, BrakeRider_m1x1, 40)
BRIDGE_DYN(HleHand_SlowPieceBrake, SlowPieceBrake_m1x1, g_ram[0x01E8 + (X & 0xFF)] ? 8 : 90)
BRIDGE_DYN(HleHand_DrawAllRiders, DrawAllRiders_m1x1, 40 + 380 * ((X & 0xFF) + 1))
BRIDGE_DYN(HleHand_EngineSound, EngineSound_m1x1, g_ram[0x1923] ? 8 : 420)
BRIDGE_DYN(HleHand_BonusRunOut, BonusRunOut_m1x1, g_ram[0x84] ? 40 : 45)
BRIDGE(HleHand_SegmentTimer, SegmentTimer_m1x1, 60)
BRIDGE_DYN(HleHand_FinishStep, FinishStep_m1x1, (g_ram[0x60] | g_ram[0x0940]) ? 90 : 10)
BRIDGE(HleHand_CheckEndingMinute, CheckEndingMinute_m1x1, 30)
BRIDGE(HleHand_DrawStartFlags, DrawStartFlags_m1x1, 300)
BRIDGE_DYN(HleHand_RaceStartLights, RaceStartLights_m1x1, (g_ram[0x3C] == 1 || !g_ram[0x8E]) ? 330 : 15)
BRIDGE(HleHand_PackEntryCode, PackEntryCode_m1x1, 1300)
BRIDGE(HleHand_QueueTimeDigits, QueueTimeDigits_m1x1, 850)
BRIDGE_DYN(HleHand_DrawObjectSprites, DrawObjectSprites_m1x1, 120 + 60 * g_ram[0x038A + g_ram[0x039D]])
BRIDGE(HleHand_DemoInputStep, DemoInputStep_m1x1, 50)
BRIDGE_DYN(HleHand_DemoPushStart, DemoPushStart_m1x1, g_ram[0x1FFFF] >= 5 ? 1450 : 15)
static uint32 update_objects_cost1(void)
{
    uint32 c = 30;
    for (int x = 0; x <= 14; x += 2)
        c += g_ram[0x02E9 + x] ? 350 : 12;
    return c;
}
BRIDGE_DYN(HleHand_UpdateObjects, UpdateObjects_m1x1, update_objects_cost1())
BRIDGE_DYN(HleHand_EndScreenTimes, EndScreenTimes_m1x1, g_ram[0x88] == 1 ? 1050 : 12)
BRIDGE_DYN(HleHand_EndScreenRecord, EndScreenRecord_m1x1, 20 + (g_ram[0x88] == 1 ? 680 : 0) + ((g_ram[0x88] <= 1 && (g_ram[0x26] & 0x80)) ? 1600 : 0))
BRIDGE(HleHand_Mode09, Mode09_m1x1, 900)
BRIDGE_DYN(HleHand_BonusResultStep, BonusResultStep_m1x1, g_ram[0x88] == 1 ? 3500 : 10)
BRIDGE(HleHand_TemperatureGauge, TemperatureGauge_m1x1, 420)
BRIDGE_DYN(HleHand_EntryNumberEditor, EntryNumberEditor_m1x1, (g_ram[0x21] & 0x10) ? 1500 : 800)
BRIDGE(HleHand_UpdateLanes, UpdateLanes_m1x1, 520)
BRIDGE(HleHand_DrawSelectWheels, DrawSelectWheels_m1x1, 1200)
BRIDGE_DYN(HleHand_DemoPadDisplay, DemoPadDisplay_m1x1, 200 + (g_ram[0x1FFFF] >= 5 ? 1450 : 15))
BRIDGE_DYN(HleHand_EraseCollectedCoin, EraseCollectedCoin_m1x1, g_ram[0xAA] ? 1700 : 12)
BRIDGE_DYN(HleHand_RedrawUsedItem, RedrawUsedItem_m1x1, g_ram[0xAF] ? 1500 : 12)
/* StartDemo ($80:E2B6) passes difftest on s1 but is not hooked: it loads
 * the course and runs past the frame, and with it the Gen A run shifts. */
BRIDGE(HleHand_RiderLanePieceAt, RiderLanePieceAt_m1x1, 66)
BRIDGE(HleHand_ResetCoinMarkers, ResetCoinMarkers_m1x1, 40)
BRIDGE(HleHand_Mode01, Mode01_m1x1, 20)
/* VramQueueTilesLong ($80:B2A1) is verified but unhooked: a trampoline
 * whose cost is the callee's. */
BRIDGE(HleHand_FlattenPlayerPiece, FlattenPlayerPiece_m1x1, 90)
BRIDGE(HleHand_ReadRiderPieces, ReadRiderPieces_m1x1, 1400)
BRIDGE(HleHand_QueueMapTile, QueueMapTile_m1x1, 200)
BRIDGE(HleHand_WalkToStartLane, WalkToStartLane_m1x1, 100)
BRIDGE(HleHand_StepSpriteSlots, StepSpriteSlots_m1x1, 300)
BRIDGE(HleHand_FollowObjectPath, FollowObjectPath_m1x1, 250)
BRIDGE(HleHand_UseArrowPiece, UseArrowPiece_m1x1, 120)
BRIDGE(HleHand_CollectPickupPiece, CollectPickupPiece_m1x1, 120)
BRIDGE(HleHand_PlayerSelectStart, PlayerSelectStart_m1x1, 200)
BRIDGE(HleHand_RaceBookkeeping, RaceBookkeeping_m1x1, 2000)
BRIDGE(HleHand_ShowEntryPassword, ShowEntryPassword_m1x1, 3000)
BRIDGE(HleHand_FinalSchedule, FinalSchedule_m1x1, 250)
/* RiderPaletteWords ($80:B538): ~40 cycles when it only clears the words
 * ($1923 set or no palettes in mode $76 per $80:B5F5), ~520 otherwise. */
uint8 r816_bus_read(uint32 a);
static uint32 rider_palette_cost1(void)
{
    if (g_ram[0x1923] || !r816_bus_read(0x80B5F5 + g_ram[0x76])) return 40;
    return 520;
}
BRIDGE_DYN(HleHand_RiderPaletteWords, RiderPaletteWords_m1x1, rider_palette_cost1())
/* SetHdmaPreset ($81:A620, A = preset): ~45 cycles for $FF, else ~38 per
 * byte of its two table sets ($81:A614 / $81:A61A lengths) plus ~170. */
static uint32 hdma_preset_cost1(uint8 a)
{
    if (a == 0xFF) return 45;
    uint32 y = (uint32)a * 2;
    uint32 n1 = r816_bus_read(0x81A614 + y) | r816_bus_read(0x81A615 + y) << 8;
    uint32 n2 = r816_bus_read(0x81A61A + y) | r816_bus_read(0x81A61B + y) << 8;
    return 170 + 38 * ((n1 + n2) & 0xFFFF);
}
BRIDGE_DYN(HleHand_SetHdmaPreset, SetHdmaPreset_m1x1, hdma_preset_cost1((uint8)A))
BRIDGE(HleHand_SetupJumpContest, SetupJumpContest_m1x1, 7000)
/* RiderGroundHeightsFrom ($80:E90F): riders X-0, ~43 cycles each on the
 * ground ($01E8 clear) and ~12 in the air. */
static uint32 ground_heights_cost1(uint8 last)
{
    uint32 c = 6;
    for (int i = 0; i <= last && i < 8; i++) c += g_ram[0x01E8 + i] ? 12 : 43;
    return c;
}
BRIDGE(HleHand_SetRiderPair, SetRiderPair_m1x1, 30)
BRIDGE(HleHand_SetJumpBase, SetJumpBase_m1x1, 16)
BRIDGE(HleHand_HeightBelow, HeightBelow_m1x1, 25)
BRIDGE(HleHand_RiderKickUp, RiderKickUp_m1x1, 62)
BRIDGE(HleHand_RiderHalfDrop, RiderHalfDrop_m1x1, 60)
BRIDGE(HleHand_RiderHighJump, RiderHighJump_m1x1, 66)
BRIDGE(HleHand_RiderPairHeight, RiderPairHeight_m1x1, 58)
BRIDGE(HleHand_RiderJumpBase, RiderJumpBase_m1x1, 100)
BRIDGE(HleHand_RiderDropStep, RiderDropStep_m1x1, 85)
BRIDGE(HleHand_ClearJumpState, ClearJumpState_m1x1, 20)
BRIDGE(HleHand_StartJumpState, StartJumpState_m1x1, 28)
BRIDGE_DYN(HleHand_RiderGroundHeightsFrom, RiderGroundHeightsFrom_m1x1, ground_heights_cost1((uint8)X))
BRIDGE_DYN(HleHand_RiderGroundHeights, RiderGroundHeights_m1x1, 2 + ground_heights_cost1(7))
BRIDGE(HleHand_RunRiderPieces, RunRiderPieces_m1x1, 90)
/* BuildCourseColumn ($80:DB89): ~44 cycles while the builder column $0273
 * is not at the player column $0260, ~1200 when it builds one. */
static uint32 build_column_cost1(void)
{
    return g_ram[0x0273] != g_ram[0x0260] ? 44 : 1200;
}
BRIDGE(HleHand_NextColumn, NextColumn_m1x1, 20)
BRIDGE(HleHand_NextColumnLong, NextColumnLong_m1x1, 34)
BRIDGE(HleHand_AddScoreLong, AddScoreLong_m1x1, 160)
BRIDGE(HleHand_QueueScoreDigitsLong, QueueScoreDigitsLong_m1x1, 300)
BRIDGE(HleHand_CourseRowList, CourseRowList_m1x1, 24)
BRIDGE_DYN(HleHand_BuildCourseColumn, BuildCourseColumn_m1x1, build_column_cost1())
BRIDGE(HleHand_Mode04_FreeRunSchedule, Mode04_FreeRunSchedule_m1x1, 120)
/* ResultsBonusCount ($81:9852): ~25 cycles between counts ($88 set),
 * ~450 on a count. */
static uint32 bonus_count_cost1(void)
{
    return g_ram[0x88] ? 25 : 450;
}
BRIDGE(HleHand_ShowCourseBest, ShowCourseBest_m1x1, 1360)
/* RacePlaceRiders ($80:A89C) passes difftest on s1 but is not hooked here:
 * it calls LoadCourse, and only HleHand_LoadCourse applies custom courses,
 * so the recompiled routine keeps calling it through the hook. */
BRIDGE_DYN(HleHand_ResultsBonusCount, ResultsBonusCount_m1x1, bonus_count_cost1())
BRIDGE(HleHand_SortAndDrawRiders, SortAndDrawRiders_m1x1, 3800)
BRIDGE_DYN(HleHand_BrakeOnTurn, BrakeOnTurn_m1x1, (X & 0xFF) == g_ram[0x7F] ? 50 : 10)
BRIDGE(HleHand_RiderThrottle, RiderThrottle_m1x1, 180)
BRIDGE(HleHand_RiderCollisions, RiderCollisions_m1x1, 800)
/* LoadRaceColumn ($81:8A4D): ~50 cycles while the builder column $0273 is
 * not at the player column $0260, ~1400 when it loads one. */
static uint32 race_column_cost1(void)
{
    return g_ram[0x0273] != g_ram[0x0260] ? 50 : 1400;
}
BRIDGE(HleHand_RaceRowList, RaceRowList_m1x1, 30)
BRIDGE_DYN(HleHand_LoadRaceColumn, LoadRaceColumn_m1x1, race_column_cost1())
BRIDGE(HleHand_DrawRacerNameLow, DrawRacerNameLow_m1x1, 1300)
/* ThrottleAll ($80:AB9A) / ThrottleFrom ($80:ABA0): ~205 cycles per
 * racing rider ($01D8; RiderThrottle's 180 included), ~12 otherwise. */
static uint32 throttle_from_cost1(int last)
{
    uint32 c = 20;
    for (int x = 0; x <= last && x < 8; x++) c += g_ram[0x01D8 + x] ? 205 : 12;
    return c;
}
/* RiderStepAll ($80:BE22): per rider its state step ($01D0: the hooked
 * steps' costs) plus RiderShadowY (40). */
static uint32 rider_steps_cost1(void)
{
    static const uint16 step[6] = { 6, 50, 100, 50, 25, 80 };
    uint32 c = 6;
    for (int x = 0; x < 8; x++) {
        uint8 st = g_ram[0x01D0 + x];
        c += 70 + (st < 6 ? step[st] : 0);
    }
    return c;
}
/* RunPieceActions ($80:EB80): ~12 per rider, ~60 more per pending action. */
static uint32 piece_actions_cost1(void)
{
    uint32 c = 10;
    for (int x = 0; x < 8; x++) c += g_ram[0x0220 + x] ? 72 : 12;
    return c;
}
BRIDGE_DYN(HleHand_ThrottleFrom, ThrottleFrom_m1x1, throttle_from_cost1(X & 0xFF))
BRIDGE_DYN(HleHand_ThrottleAll, ThrottleAll_m1x1, g_ram[0x8E] ? 4 + throttle_from_cost1(7) : 25)
BRIDGE(HleHand_RiderStateStep, RiderStateStep_m1x1, 70)
BRIDGE_DYN(HleHand_RiderStepAll, RiderStepAll_m1x1, rider_steps_cost1())
BRIDGE(HleHand_AnimateAllWheels, AnimateAllWheels_m1x1, 202)
BRIDGE(HleHand_RunAllRiderPieces, RunAllRiderPieces_m1x1, 800)
BRIDGE_DYN(HleHand_RunPieceActions, RunPieceActions_m1x1, piece_actions_cost1())
BRIDGE(HleHand_TemperatureGaugeJmp, TemperatureGaugeJmp_m1x1, 423)
/* Mode03_Race ($80:A9BA) and Mode0C ($80:E319) pass difftest on s1 but are
 * not hooked: they only call some forty routines that are hooked one by one,
 * and one bridge cost could not follow their per-frame cycle total. */
/* BsxFindFile ($81:F9C8) passes difftest on s1 but is not hooked here:
 * with it the Gen A race run diverges (it is hooked in Gen B, where both
 * runs are unchanged). */
#else
/* Stadium 3-4 (Gen B) routines from c/hand/s4 */
BRIDGE(HleHand_RiderGfxCycle,     RiderGfxCycle_m1x1,     60)
BRIDGE(HleHand_RiderSpriteSets,   RiderSpriteSets_m1x1,   600)
BRIDGE(HleHand_LoadRiderPalettes, LoadRiderPalettes_m1x1, 1200)
BRIDGE(HleHand_DrawRaceHud,       DrawRaceHud_m1x1,       3000)
BRIDGE(HleHand_VramQueueTiles20,  VramQueueTiles20_m1x1,  600)
BRIDGE(HleHand_VramQueueTiles,    VramQueueTiles_m1x1,    600)
BRIDGE(HleHand_WinnerFigure,      WinnerFigure_m1x1,      120)
BRIDGE(HleHand_SelectMain,        SelectMain_m1x1,        3000)
BRIDGE(HleHand_SelectTimer,       SelectTimer_m1x1,       600)
BRIDGE(HleHand_SelectRotate,      SelectRotate_m1x1,      1500)
BRIDGE(HleHand_SelectDraw,        SelectDraw_m1x1,        2500)
BRIDGE(HleHand_SelectHideRest,    SelectHideRest_m1x1,    300)
BRIDGE(HleHand_SelectSlideY,      SelectSlideY_m1x1,      200)
BRIDGE(HleHand_SelectTransition,  SelectTransition_m1x1,  3000)
BRIDGE(HleHand_SelectName,        SelectName_m1x1,        300)
BRIDGE(HleHand_CarouselPos,       CarouselPos_m1x1,       250)
BRIDGE(HleHand_SinCos,            SinCos_m1x1,            150)
BRIDGE(HleHand_QueueSoundB,       QueueSoundB_m1x1,       60)
BRIDGE(HleHand_DrawClockB,        DrawClockB_m1x1,        800)
BRIDGE(HleHand_RiderCollisions,   RiderCollisions_m1x1,   800)
BRIDGE(HleHand_Reach20,           Reach20_m1x1,           20)
BRIDGE(HleHand_Reach2,            Reach2_m1x1,            20)
BRIDGE(HleHand_CollectCoin,       CollectCoin_m1x1,       400)
BRIDGE(HleHand_CoinMilestone,     CoinMilestone_m1x1,     100)
BRIDGE(HleHand_RandomB,           RandomB_m1x1,           60)
/* small Gen B leaves (c/hand/s4/leaves.c). The cycle figure is charged at 8
 * master cycles each, while code in the FastROM banks ($80+) runs at 6, so
 * a routine called in a hot loop needs (CPU cycles * 6 / 8): ReadStreamByte
 * (25 CPU cycles, once per byte of the graphics decompressor) is 19; at 20
 * or more the decompressor ends a frame late and the race drifts. */
BRIDGE(HleHand_QueueNameplate, QueueNameplate_m1x1, 1240)
BRIDGE(HleHand_QueueResultsRow, QueueResultsRow_m1x1, 40)
BRIDGE(HleHand_FadeApuVolume, FadeApuVolume_m1x1, 30)
BRIDGE(HleHand_ReadStreamByte, ReadStreamByte_m1x0, 19)
BRIDGE(HleHand_AddSpeedToX, AddSpeedToX_m1x1, 40)
BRIDGE(HleHand_BlankQueueRowB, BlankQueueRowB_m1x1, 250)
BRIDGE(HleHand_StepAnimTimer, StepAnimTimer_m1x1, 40)
BRIDGE(HleHand_DispatchBy85B, DispatchBy85B_m1x1, 40)
BRIDGE(HleHand_ScrollByPlayerSpeed, ScrollByPlayerSpeed_m1x1, 35)
BRIDGE(HleHand_EndAtBonusMinute, EndAtBonusMinute_m1x1, 25)
BRIDGE(HleHand_ScrollJumpRider, ScrollJumpRider_m1x1, 40)
BRIDGE(HleHand_DispatchByDistance, DispatchByDistance_m1x1, 90)
BRIDGE(HleHand_SetJumpBanner, SetJumpBanner_m1x1, 40)
BRIDGE(HleHand_AddBcdDigits, AddBcdDigits_m1x1, 150)
BRIDGE(HleHand_ObjectAdvance8, ObjectAdvance8_m1x1, 45)
BRIDGE(HleHand_ObjectAdvance20, ObjectAdvance20_m1x1, 45)
BRIDGE(HleHand_ObjectAdvance80, ObjectAdvance80_m1x1, 45)
BRIDGE(HleHand_ObjectAdvance9, ObjectAdvance9_m1x1, 30)
BRIDGE(HleHand_ObjectAdvanceC0, ObjectAdvanceC0_m1x1, 30)
BRIDGE(HleHand_ObjectAdvanceOr4, ObjectAdvanceOr4_m1x1, 30)
BRIDGE(HleHand_ObjectAdvanceOr5, ObjectAdvanceOr5_m1x1, 30)
BRIDGE(HleHand_TickTimers, TickTimers_m1x1, 220)
BRIDGE(HleHand_BsxReadHeader3, BsxReadHeader3_m1x1, 260)
BRIDGE(HleHand_CheckCourseTime, CheckCourseTime_m1x1, 30)
BRIDGE(HleHand_StopWhenStill, StopWhenStill_m1x1, 30)
BRIDGE(HleHand_ClearOnBonusTiles, ClearOnBonusTiles_m1x1, 25)
BRIDGE(HleHand_StepJumpBanner, StepJumpBanner_m1x1, 70)
BRIDGE(HleHand_CheckBonusTime, CheckBonusTime_m1x1, 25)
BRIDGE(HleHand_Hundreds, Hundreds_m1x1, 60)
BRIDGE(HleHand_SetupJumpScreen, SetupJumpScreen_m1x1, 40)
BRIDGE(HleHand_RiderStartLane, RiderStartLane_m1x1, 25)
BRIDGE(HleHand_ScrollBySpeed16, ScrollBySpeed16_m1x1, 30)
BRIDGE(HleHand_ScrollBySpeed16B, ScrollBySpeed16B_m1x1, 30)
BRIDGE(HleHand_SetBgTileBase, SetBgTileBase_m1x1, 35)
BRIDGE(HleHand_SetupCourseScreen, SetupCourseScreen_m1x1, 40)
BRIDGE(HleHand_StepObjectScript, StepObjectScript_m1x1, 35)
BRIDGE(HleHand_ReadStreamWord, ReadStreamWord_m1x0, 32)
/* c/hand/s4/objects.c */
BRIDGE(HleHand_ObjectAdvanceRandom20, ObjectAdvanceRandom20_m1x1, 95)
BRIDGE(HleHand_ObjectAdvanceRandom10, ObjectAdvanceRandom10_m1x1, 95)
BRIDGE(HleHand_ObjectAdvanceRandom0A, ObjectAdvanceRandom0A_m1x1, 95)
BRIDGE(HleHand_ObjectAdvanceRandom09, ObjectAdvanceRandom09_m1x1, 95)
BRIDGE(HleHand_ObjectAdvanceRandom10B, ObjectAdvanceRandom10B_m1x1, 95)
BRIDGE(HleHand_ObjectAdvanceRandom16, ObjectAdvanceRandom16_m1x1, 95)
BRIDGE(HleHand_ObjectCycleA, ObjectCycleA_m1x1, 35)
BRIDGE(HleHand_ObjectCycleB, ObjectCycleB_m1x1, 35)
BRIDGE(HleHand_SetupBonusScreen, SetupBonusScreen_m1x1, 45)
BRIDGE(HleHand_FillBonusColumns, FillBonusColumns_m1x1, 330)
BRIDGE(HleHand_ResultsWait, ResultsWait_m1x1, 30)
BRIDGE(HleHand_SetJumpRiderSprites, SetJumpRiderSprites_m1x1, 50)
BRIDGE(HleHand_JumpRunUp, JumpRunUp_m1x1, 95)
BRIDGE(HleHand_ResetSoundState, ResetSoundState_m1x1, 60)
BRIDGE(HleHand_StartBonusMode, StartBonusMode_m1x1, 170)
BRIDGE(HleHand_StartWheelie, StartWheelie_m1x1, 165)
BRIDGE(HleHand_StartIntroMode, StartIntroMode_m1x1, 90)
/* c/hand/s4/objects.c, part 2 */
BRIDGE(HleHand_RankPlayer, RankPlayer_m1x1, 90)
BRIDGE(HleHand_ObjectWalkCycle, ObjectWalkCycle_m1x1, 35)
BRIDGE(HleHand_LoadStadiumPalette, LoadStadiumPalette_m1x1, 180)
BRIDGE(HleHand_CountCourseBits, CountCourseBits_m1x1, 900)
BRIDGE(HleHand_StartJumpMode, StartJumpMode_m1x1, 120)
BRIDGE(HleHand_ShowResultsBanner, ShowResultsBanner_m1x1, 2440)
BRIDGE(HleHand_ObjectBlinkA, ObjectBlinkA_m1x1, 40)
BRIDGE(HleHand_ObjectBlinkB, ObjectBlinkB_m1x1, 40)
BRIDGE(HleHand_ObjectBlinkC, ObjectBlinkC_m1x1, 40)
BRIDGE(HleHand_ObjectBlinkD, ObjectBlinkD_m1x1, 40)
BRIDGE(HleHand_ObjectRepeat4A, ObjectRepeat4A_m1x1, 40)
BRIDGE(HleHand_ObjectRepeat4B, ObjectRepeat4B_m1x1, 40)
BRIDGE(HleHand_ObjectRun4, ObjectRun4_m1x1, 40)
BRIDGE(HleHand_ObjectRun6, ObjectRun6_m1x1, 40)
BRIDGE(HleHand_ObjectRunOnce6, ObjectRunOnce6_m1x1, 40)
BRIDGE(HleHand_ObjectLoop7, ObjectLoop7_m1x1, 40)
BRIDGE(HleHand_ObjectLoop5, ObjectLoop5_m1x1, 45)
BRIDGE(HleHand_ObjectFlicker, ObjectFlicker_m1x1, 40)
BRIDGE(HleHand_SpinWheels, SpinWheels_m1x1, 150)
BRIDGE(HleHand_SpinWheels8, SpinWheels8_m1x1, 150)
BRIDGE(HleHand_EndJumpMode, EndJumpMode_m1x1, 30)
BRIDGE(HleHand_QueueBonusDigits, QueueBonusDigits_m1x1, 1300)
/* c/hand/s4/objects.c, part 3 */
BRIDGE(HleHand_SetupHdmaChannels, SetupHdmaChannels_m1x1, 60)
BRIDGE(HleHand_RiderAhead, RiderAhead_m1x1, 150)
BRIDGE(HleHand_DrawRiderMarker, DrawRiderMarker_m1x1, 60)
BRIDGE(HleHand_StepFallingSprite, StepFallingSprite_m1x1, 60)
BRIDGE(HleHand_DrawCountdownLights, DrawCountdownLights_m1x1, 160)
BRIDGE(HleHand_ObjectRepeat4Rest, ObjectRepeat4Rest_m1x1, 40)
BRIDGE(HleHand_SortRidersByPosition, SortRidersByPosition_m1x1, 700)
BRIDGE(HleHand_QueueResultsCount, QueueResultsCount_m1x1, 1260)
BRIDGE(HleHand_DrawCountdownSign, DrawCountdownSign_m1x1, 70)
BRIDGE(HleHand_DrawCountdownSignB, DrawCountdownSignB_m1x1, 70)
BRIDGE(HleHand_StartCrash, StartCrash_m1x1, 120)
BRIDGE(HleHand_AddBonusPoints, AddBonusPoints_m1x1, 260)
BRIDGE(HleHand_SetRiderLean, SetRiderLean_m1x1, 35)
BRIDGE(HleHand_QueuePlaceBadge, QueuePlaceBadge_m1x1, 1270)
BRIDGE(HleHand_SetJumpScroll, SetJumpScroll_m1x1, 60)
BRIDGE(HleHand_DrawSpark, DrawSpark_m1x1, 60)
/* c/hand/s4/objects.c, part 4 */
BRIDGE(HleHand_StartPracticeMode, StartPracticeMode_m1x1, 120)
BRIDGE(HleHand_DrawJumpRank, DrawJumpRank_m1x1, 50)
BRIDGE(HleHand_StepFade, StepFade_m1x1, 50)
BRIDGE(HleHand_Mode17_BonusIntro, Mode17_BonusIntro_m1x1, 40)
BRIDGE(HleHand_RiderAtStart, RiderAtStart_m1x1, 45)
BRIDGE(HleHand_DrawStar, DrawStar_m1x1, 60)
BRIDGE(HleHand_SpawnDustPuff, SpawnDustPuff_m1x1, 70)
BRIDGE(HleHand_ClearRaceState, ClearRaceState_m1x1, 8360)
BRIDGE(HleHand_ReadBroadcastDate, ReadBroadcastDate_m1x1, 100)
BRIDGE(HleHand_MoveObjectX, MoveObjectX_m1x1, 70)
/* QueueRaceTime ($80:C5B0) passes difftest but is not hooked: it runs
 * every race frame close to the frame deadline, and as one uninterruptible
 * call it moved the race start by two frames in the menu-flow run. */
BRIDGE(HleHand_QueueBestTime, QueueBestTime_m1x1, 1310)
BRIDGE(HleHand_AccelerateRider, AccelerateRider_m1x1, 50)
BRIDGE(HleHand_UpdateRiderDistances, UpdateRiderDistances_m1x1, 180)
/* c/hand/s4/objects.c, part 5 */
BRIDGE(HleHand_DrawDustPair, DrawDustPair_m1x1, 80)
BRIDGE(HleHand_RiderOnRamp, RiderOnRamp_m1x1, 40)
BRIDGE(HleHand_BuildJumpScore, BuildJumpScore_m1x1, 120)
BRIDGE(HleHand_Mode09_BonusIntro, Mode09_BonusIntro_m1x1, 40)
BRIDGE(HleHand_TrackRiderPace, TrackRiderPace_m1x1, 40)
BRIDGE(HleHand_HideAllSprites, HideAllSprites_m1x1, 1360)
/* c/hand/s4/objects.c, part 6 */
BRIDGE(HleHand_CheckLaneTile, CheckLaneTile_m1x1, 45)
BRIDGE(HleHand_DrawJumpRiderSprites, DrawJumpRiderSprites_m1x1, 130)
BRIDGE_DYN(HleHand_FadeSkyColours, FadeSkyColours_m1x1,
           (g_ram[0x63] & 7) ? 15 : 15 + 45 * 4)
BRIDGE(HleHand_DrawBigSprite, DrawBigSprite_m1x1, 120)
BRIDGE(HleHand_QueueBonusCount, QueueBonusCount_m1x1, 1320)
BRIDGE(HleHand_SpawnExhaust, SpawnExhaust_m1x1, 70)
BRIDGE(HleHand_RiderAtFinish, RiderAtFinish_m1x1, 60)
/* c/hand/s4/objects.c, part 7 */
BRIDGE(HleHand_BuildScoreDigits, BuildScoreDigits_m0x1, 230)
/* every 8th frame: about 40 units per colour from $B9 down */
BRIDGE_DYN(HleHand_FadePalette, FadePalette_m1x1,
           (g_ram[0x63] & 7) ? 20 : 20 + 40 * (g_ram[0xB9] / 2 + 1))
BRIDGE(HleHand_MirrorJumpRider, MirrorJumpRider_m1x1, 240)
BRIDGE(HleHand_PlaceMinuteMarker, PlaceMinuteMarker_m1x1, 40)
BRIDGE(HleHand_RiderPose, RiderPose_m1x1, 40)
BRIDGE(HleHand_ReadJumpTrack, ReadJumpTrack_m1x1, 60)
/* c/hand/s4/objects.c, part 8 */
BRIDGE(HleHand_VelocityDirection, VelocityDirection_m1x1, 120)
BRIDGE(HleHand_PackSpriteSizes, PackSpriteSizes_m1x1, 840)
BRIDGE(HleHand_RecordCourseTime, RecordCourseTime_m1x1, 1410)
BRIDGE(HleHand_RivalEngineSound, RivalEngineSound_m1x1, 320)
/* $11 rows of $10 tile words: about 60 units a row and 22 a word */
BRIDGE_DYN(HleHand_QueueTileRows, QueueTileRows_m1x1,
           30 + ((g_ram[0x11] ? g_ram[0x11] : 256) * (60 + 22 * g_ram[0x10])))
BRIDGE(HleHand_BrakeRider, BrakeRider_m1x1, 40)
/* c/hand/s4/objects.c, part 9 */
/* header byte at DB:($02): rows << 4 | words; about 60 units a row and 22
 * a word */
static uint32 tile_block_cost(void)
{
    uint16 p = (uint16)(r816_bus_read((uint16)(D + 2)) | r816_bus_read((uint16)(D + 3)) << 8);
    uint8 h = r816_bus_read(((uint32)DB << 16) | p);
    uint32 rows = (h >> 4) ? (h >> 4) : 16, words = (h & 15) ? (h & 15) : 16;
    return 30 + rows * (60 + 22 * words);
}
BRIDGE_DYN(HleHand_QueueTileBlock, QueueTileBlock_m1x1, tile_block_cost())
BRIDGE(HleHand_PlaceRiderShadow, PlaceRiderShadow_m1x1, 50)
BRIDGE(HleHand_ShowJumpResult, ShowJumpResult_m1x1, 1540)
BRIDGE(HleHand_DrawBigBall, DrawBigBall_m1x1, 90)
/* c/hand/s4/objects.c, part 10 */
BRIDGE_DYN(HleHand_BonusResultStep, BonusResultStep_m1x1, g_ram[0x88] == 1 ? 2600 : 10)
BRIDGE(HleHand_DrawJumpCrowd, DrawJumpCrowd_m1x1, 250)
BRIDGE(HleHand_StartBonusResult, StartBonusResult_m1x1, 2700)
BRIDGE_DYN(HleHand_DrawBonusPickup, DrawBonusPickup_m1x1, g_ram[0xB7] ? 2600 : 15)
/* c/hand/s4/flow.c */
BRIDGE(HleHand_ChangeMode, ChangeMode_m1x1, 20)
BRIDGE(HleHand_NextMode, NextMode_m1x1, 25)
BRIDGE(HleHand_RaceFinishStep, RaceFinishStep_m1x1, 40)
/* c/hand/s4/objects.c, part 11 */
BRIDGE_DYN(HleHand_DrawCoinPickup, DrawCoinPickup_m1x1, g_ram[0xB2] ? 2700 : 15)
BRIDGE(HleHand_TurboMeter, TurboMeter_m1x1, 240)
BRIDGE(HleHand_ReadBonusTrack, ReadBonusTrack_m1x1, 60)
BRIDGE(HleHand_EnterNumberStep, EnterNumberStep_m1x1, 1500)
/* c/hand/s4/flow.c */
BRIDGE(HleHand_RivalTarget, RivalTarget_m1x1, 45)
/* c/hand/s4/objects.c, part 12 */
BRIDGE(HleHand_DrawRiderShadow, DrawRiderShadow_m1x1, 130)
/* c/hand/s4/objects.c, part 13 */
BRIDGE(HleHand_BsxEndIfFlagged, BsxEndIfFlagged_m1x1, 150)
BRIDGE(HleHand_CopyPieceColumn, CopyPieceColumn_m1x1, 300)
BRIDGE(HleHand_CopyPieceColumnPal2, CopyPieceColumnPal2_m1x1, 330)
BRIDGE(HleHand_RetargetRival, RetargetRival_m1x1, 60)
BRIDGE(HleHand_BonusTimerStep, BonusTimerStep_m1x1, 1360)
BRIDGE(HleHand_BonusScrollStep, BonusScrollStep_m1x1, 100)
BRIDGE_DYN(HleHand_LoadEntryScreenStep, LoadEntryScreenStep_m1x1, g_ram[0x88] == 1 ? 1000 : 15)
BRIDGE_DYN(HleHand_FadeObjectStep, FadeObjectStep_m1x1, 40 + ((g_ram[0x63] & 7) ? 20 : 20 + 40 * (g_ram[0xB9] / 2 + 1)))
BRIDGE(HleHand_TickRaceClock, TickRaceClock_m1x1, 1340)
/* c/hand/s4/objects.c, part 14 */
BRIDGE(HleHand_ShowJumpCount, ShowJumpCount_m1x1, 3000)
BRIDGE(HleHand_ShowBonusPoints, ShowBonusPoints_m1x1, 2900)
BRIDGE(HleHand_CopyTrackColumn, CopyTrackColumn_m1x1, 400)
BRIDGE(HleHand_PlaceAllRiderShadows, PlaceAllRiderShadows_m1x1, 560)
BRIDGE_DYN(HleHand_SkyFadeStep, SkyFadeStep_m1x1, 40 + ((g_ram[0x63] & 7) ? 15 : 15 + 45 * 4))
BRIDGE_DYN(HleHand_EntryConfirmStep, EntryConfirmStep_m1x1, ((g_ram[0x21] | g_ram[0x26]) & 0x80) ? 1000 : 15)
/* c/hand/s4/objects.c, part 15 */
BRIDGE_DYN(HleHand_EntryIntroStep, EntryIntroStep_m1x1, (g_ram[0xA2] | g_ram[0xA3] << 8) >= 0x013E ? 1000 : 20)
BRIDGE(HleHand_OffTrackBrake, OffTrackBrake_m1x1, 80)
BRIDGE_DYN(HleHand_BonusSignStep, BonusSignStep_m1x1, g_ram[0x97] ? 700 : 10)
BRIDGE_DYN(HleHand_EntryMenuStep, EntryMenuStep_m1x1, (g_ram[0x88] == 1 || (g_ram[0x26] & 0x80)) ? 1000 : 15)
BRIDGE(HleHand_DrawRider, DrawRider_m1x1, 200)
BRIDGE(HleHand_CrashLand, CrashLand_m1x1, 150)
/* c/hand/s4/objects.c, part 16 */
BRIDGE(HleHand_RiderLandCheck, RiderLandCheck_m1x1, 150)
BRIDGE(HleHand_BoostOnTile, BoostOnTile_m1x1, 170)
BRIDGE(HleHand_SlowTileCheck, SlowTileCheck_m1x1, 180)
BRIDGE(HleHand_LandFlat, LandFlat_m1x1, 160)
/* c/hand/s4/objects.c, part 17 */
BRIDGE(HleHand_CpuReleaseButtons, CpuReleaseButtons_m1x1, 25)
BRIDGE(HleHand_CpuThrottle, CpuThrottle_m1x1, 35)
BRIDGE(HleHand_CpuReleaseDpad, CpuReleaseDpad_m1x1, 15)
BRIDGE(HleHand_CpuSteer, CpuSteer_m1x1, 45)
BRIDGE_DYN(HleHand_QueuePlaceBadgeHud, QueuePlaceBadgeHud_m1x1, g_ram[0x086E] ? 1400 : 10)
/* per slot X..0: about 300 units with an active rider drawn, 45 without */
static uint32 draw_all_riders_cost(void)
{
    uint32 c = 20;
    for (int i = (int)(X & 0xFF); i >= 0 && i < 8; i--)
        c += g_ram[0x01DE + g_ram[0x0950 + i]] ? 300 : 45;
    return c;
}
BRIDGE_DYN(HleHand_DrawAllRiders, DrawAllRiders_m1x1, draw_all_riders_cost())
BRIDGE_DYN(HleHand_ShowCourseRecord, ShowCourseRecord_m1x1, g_ram[0x084C] ? 1700 : 15)
/* c/hand/s4/objects.c, part 18 */
BRIDGE(HleHand_DrawStartFlags, DrawStartFlags_m1x1, 420)
BRIDGE(HleHand_RaceStartLights, RaceStartLights_m1x1, 450)
/* c/hand/s4/objects.c, part 19 */
BRIDGE_DYN(HleHand_CrashRecovery, CrashRecovery_m1x1, ((g_ram[0x63] & 3) == g_ram[0x0998]) ? 700 : 15)
BRIDGE_DYN(HleHand_AimRatio, AimRatio_m1x1, (A & 0xFF) ? 120 + 25 * (A & 0xFF) : 15)
/* c/hand/s4/objects.c, part 20 */
BRIDGE_DYN(HleHand_SpeedGauge, SpeedGauge_m1x1, (g_ram[0x092D] && !(g_ram[0x7F] & 1)) ? 260 : 15)
BRIDGE(HleHand_FallingObjectStep, FallingObjectStep_m1x1, 130)
BRIDGE(HleHand_RiderRunOut, RiderRunOut_m1x1, 50)
/* c/hand/s4/objects.c, part 21 */
BRIDGE(HleHand_Mul16, Mul16_m1x1, 230)
/* Mul16b ($80:B9FA) passes difftest but is not hooked: with it the jump
 * contest run diverges within a few frames whatever its cost, and the
 * difftest cannot check multiplier reads (the harness serves them from a
 * table). Left recompiled until the difference is found. */
/* c/hand/s4/objects.c, part 22 */
BRIDGE_DYN(HleHand_TitleScrollStep, TitleScrollStep_m1x1, g_ram[0x8A] ? 40 : 1500)
BRIDGE(HleHand_StartEndScreen, StartEndScreen_m1x1, 3600)
BRIDGE(HleHand_LoadHdmaScene, LoadHdmaScene_m1x1, 2500)
/* c/hand/s4/objects.c, part 23 */
BRIDGE_DYN(HleHand_ShowCoinRank, ShowCoinRank_m1x1, (!(g_ram[0x63] & 1) && g_ram[0x76] == 3) ? 1300 : 15)
/* BonusCourseFrame ($80:D81B) passes difftest but is not hooked: it is the
 * bonus course's whole frame and no regression run reaches that course,
 * so its cost could not be checked. */
/* c/hand/s4/objects.c, part 24 */
BRIDGE(HleHand_ScaleJumpCount, ScaleJumpCount_m1x1, 200)
BRIDGE(HleHand_BsxFindFile, BsxFindFile_m0x0, 600)
/* c/hand/s4/objects.c, part 25 */
BRIDGE(HleHand_ShowItemPoints, ShowItemPoints_m1x1, 1650)
BRIDGE(HleHand_UpdateLanes, UpdateLanes_m1x1, 520)
BRIDGE(HleHand_LanePieceTables, LanePieceTables_m1x1, 15)
BRIDGE(HleHand_LanePieceTablesLong, LanePieceTablesLong_m1x1, 36)
BRIDGE(HleHand_LanePieceAt, LanePieceAt_m1x1, 58)
BRIDGE(HleHand_RiderPieceAt, RiderPieceAt_m1x1, 63)
BRIDGE(HleHand_IsPieceFlat, IsPieceFlat_m1x1, 10)
BRIDGE(HleHand_RiderOnFlat, RiderOnFlat_m1x1, 77)
BRIDGE(HleHand_HeightAbove, HeightAbove_m1x1, 18)
BRIDGE(HleHand_GroundHeightAbove, GroundHeightAbove_m1x1, 21)
BRIDGE(HleHand_PlayerColumnCheck, PlayerColumnCheck_m1x1, 60)
BRIDGE(HleHand_PlayerColumnCheckB, PlayerColumnCheckB_m1x1, 60)
BRIDGE(HleHand_CpuSteerUp, CpuSteerUp_m1x1, 150)
BRIDGE(HleHand_CpuSteerDown, CpuSteerDown_m1x1, 150)
BRIDGE(HleHand_QueueResultsTiles, QueueResultsTiles_m1x1, 1280)
/* about 650 units for each flagged player's 44-byte marker */
BRIDGE_DYN(HleHand_QueuePlayerMarkers, QueuePlayerMarkers_m1x1,
           30 + 650 * ((g_ram[0x1E8D] != 0) + (g_ram[0x1E8E] != 0) +
                       (g_ram[0x1E8F] != 0) + (g_ram[0x1E90] != 0)))
BRIDGE(HleHand_CourseColumn, CourseColumn_m1x1, 280)
BRIDGE(HleHand_CourseColumnB, CourseColumnB_m1x1, 280)
BRIDGE(HleHand_BallHit, BallHit_m1x1, 50)
BRIDGE(HleHand_FillLaneColumn, FillLaneColumn_m1x1, 490)
BRIDGE(HleHand_QueueMapTile, QueueMapTile_m1x1, 660)
BRIDGE(HleHand_QueueBlinkBanner, QueueBlinkBanner_m1x1, 600)
BRIDGE(HleHand_QueueFiveDigits, QueueFiveDigits_m1x1, 760)
BRIDGE_DYN(HleHand_QueueDigitsAt, QueueDigitsAt_m1x1, 620 + 20 * (Y & 0xFF))
BRIDGE(HleHand_ShowDigitDifference, ShowDigitDifference_m1x1, 920)
BRIDGE_DYN(HleHand_EndScreenScore, EndScreenScore_m1x1, g_ram[0x88] == 1 ? 820 : 15)
BRIDGE_DYN(HleHand_EndScreenCode, EndScreenCode_m1x1,
           20 + (g_ram[0x88] == 1 ? 680 : 0) +
           ((g_ram[0x88] <= 1 && (g_ram[0x26] & 0x80)) ? 900 : 0))
BRIDGE(HleHand_CpuLaneDecision, CpuLaneDecision_m1x1, 150)
BRIDGE(HleHand_PlayerStop, PlayerStop_m1x1, 40)
BRIDGE(HleHand_PlayerHalt, PlayerHalt_m1x1, 30)
BRIDGE(HleHand_PlayerStopB, PlayerStopB_m1x1, 40)
BRIDGE(HleHand_PlayerHaltB, PlayerHaltB_m1x1, 30)
BRIDGE(HleHand_ObjectFollowPath, ObjectFollowPath_m1x1, 1900)
BRIDGE(HleHand_SetRiderPair, SetRiderPair_m1x1, 18)
BRIDGE(HleHand_RiderLanePieceAt, RiderLanePieceAt_m1x1, 66)
BRIDGE(HleHand_EmptyHook, EmptyHook_m1x1, 5)
BRIDGE(HleHand_ReadRiderPieces, ReadRiderPieces_m1x1, 1120)
BRIDGE_DYN(HleHand_FlattenPlayerPiece, FlattenPlayerPiece_m1x1,
           ((X & 0xFF) == 0 && !g_ram[0x01EE] && !g_ram[0x84]) ? 110 : 10)
BRIDGE_DYN(HleHand_UseArrowPiece, UseArrowPiece_m1x1,
           165 + (((X & 0xFF) == 0 && !g_ram[0x01EE]) ? 300 : 10))
BRIDGE_DYN(HleHand_CollectPickupPiece, CollectPickupPiece_m1x1,
           ((X & 0xFF) == 0 && !g_ram[0x01EE]) ? 330 : 15)
BRIDGE(HleHand_HeightBelow, HeightBelow_m1x1, 15)
BRIDGE(HleHand_RiderPairHeight, RiderPairHeight_m1x1, 40)
BRIDGE(HleHand_PlayerSpeedBoost, PlayerSpeedBoost_m1x1, 70)
BRIDGE(HleHand_Mode0A, Mode0A_m1x1, 120)
BRIDGE(HleHand_SetJumpBase, SetJumpBase_m1x1, 12)
BRIDGE(HleHand_RiderDropStep, RiderDropStep_m1x1, 60)
BRIDGE_DYN(HleHand_RiderLanding, RiderLanding_m1x1, g_ram[0x01EE + (X & 0xFF)] ? 90 : 8)
BRIDGE(HleHand_DrawShadowsAndMarkers, DrawShadowsAndMarkers_m1x1, 1200)
BRIDGE(HleHand_StartModeWithSound, StartModeWithSound_m1x1, 130)
/* eight riders' move, landing, land check and tick, then the scrolls */
BRIDGE_DYN(HleHand_RaceMoveStep, RaceMoveStep_m1x1, g_ram[0x92] ? 2500 : 8)
BRIDGE(HleHand_ScrollByPlayerSpeedLong, ScrollByPlayerSpeedLong_m1x1, 45)
BRIDGE(HleHand_PlaceRiderShadowLong, PlaceRiderShadowLong_m1x1, 65)
BRIDGE(HleHand_AddScoreLong, AddScoreLong_m1x1, 210)
BRIDGE(HleHand_RiderJumpBase, RiderJumpBase_m1x1, 50)
BRIDGE(HleHand_CpuAvoidHurdle, CpuAvoidHurdle_m1x1, 150)
BRIDGE(HleHand_PlaceRival, PlaceRival_m1x1, 350)
BRIDGE(HleHand_SlotRiderShadow, SlotRiderShadow_m1x1, 110)
BRIDGE(HleHand_AddStadiumBonus, AddStadiumBonus_m1x1, 270)
/* CpuRidersStep: about 450 units for each active CPU rider on the ground
 * (steer, throttle, hurdles, lane checks), 60 in the air, 20 idle */
static uint32 cpu_riders_cost(void)
{
    uint32 c = 10;
    for (int x = 1; x < 8; x++) {
        if (g_ram[0x01DE + x] &&
            !(g_ram[0x01CE + x] | g_ram[0x01D6 + x] | g_ram[0x199E + x]))
            c += g_ram[0x01EE + x] ? 60 : 450;
        else
            c += 20;
    }
    return c;
}
BRIDGE_DYN(HleHand_CpuRidersStep, CpuRidersStep_m1x1, cpu_riders_cost())
BRIDGE(HleHand_SpawnRivals, SpawnRivals_m1x1, 420)
BRIDGE_DYN(HleHand_CrossFinish, CrossFinish_m1x1,
           ((X & 0xFF) == 0 && !g_ram[0x03AC] && !g_ram[0x84]) ? 450 : 10)
BRIDGE(HleHand_ByteTable84BA, ByteTable84BA_m1x1, 20)
BRIDGE_DYN(HleHand_LaunchBall, LaunchBall_m1x1, g_ram[0x092C] ? 500 : 8)
BRIDGE(HleHand_StepObjectX8, StepObjectX8_m1x1, 88)
BRIDGE(HleHand_MarkerObjectStep, MarkerObjectStep_m1x1, 260)
BRIDGE(HleHand_QueueBonusDigitsAt, QueueBonusDigitsAt_m1x1, 1310)
static uint32 marker_objects_cost(void)
{
    uint32 c = 20 + ((!(g_ram[0x63] & 1) && g_ram[0x76] == 3) ? 1300 : 15);
    for (int x = 0; x <= 6; x += 2)
        if (g_ram[0x1530 + x]) c += 280;
    return c;
}
BRIDGE_DYN(HleHand_MarkerObjects, MarkerObjects_m1x1, marker_objects_cost())
BRIDGE_DYN(HleHand_RaceFrameStep, RaceFrameStep_m1x1,
           1130 + ((!g_ram[0x0CD7] && !g_ram[0x1923] && g_ram[0x76] < 9) ? 1560 : 0))
BRIDGE(HleHand_EmptyStep9828, EmptyStep9828_m1x1, 6)
BRIDGE(HleHand_QueueFontDigits, QueueFontDigits_m1x1, 850)
BRIDGE(HleHand_SpawnObject, SpawnObject_m1x1, 200)
BRIDGE(HleHand_FindObjectSlot, FindObjectSlot_m1x1, 60)
BRIDGE(HleHand_QueueTimeDigits, QueueTimeDigits_m1x1, 850)
BRIDGE_DYN(HleHand_EndScreenTimes, EndScreenTimes_m1x1, g_ram[0x88] == 1 ? 1050 : 12)
BRIDGE(HleHand_FrameSequenceA, FrameSequenceA_m1x1, 40)
BRIDGE(HleHand_FrameSequenceB, FrameSequenceB_m1x1, 40)
BRIDGE_DYN(HleHand_DriftUpObject, DriftUpObject_m1x1, (g_ram[0x63] & 3) ? 20 : 80)
BRIDGE(HleHand_ReleaseBalloons, ReleaseBalloons_m1x1, 50)
BRIDGE_DYN(HleHand_EndScreenRecord, EndScreenRecord_m1x1,
           20 + (g_ram[0x88] == 1 ? 680 : 0) +
           ((g_ram[0x88] <= 1 && (g_ram[0x26] & 0x80)) ? 1600 : 0))
BRIDGE(HleHand_MoveObjectX8, MoveObjectX8_m1x1, 76)
BRIDGE(HleHand_ConfettiStep, ConfettiStep_m1x1, 250)
BRIDGE(HleHand_EmptyStep9554, EmptyStep9554_m1x1, 6)
BRIDGE_DYN(HleHand_DrawAllRidersLong, DrawAllRidersLong_m1x1, 15 + draw_all_riders_cost())
BRIDGE(HleHand_BuildScoreDigitsLong, BuildScoreDigitsLong_m0x1, 240)
BRIDGE(HleHand_FlagRidersAtLine, FlagRidersAtLine_m1x1, 140)
/* even frames: the ball check; frames 1 mod 4: line flags + speed gauge */
static uint32 ball_or_gauge_cost(void)
{
    if (!(g_ram[0x63] & 1)) return 60;
    if (g_ram[0x63] & 2) return 10;
    return 160 + ((g_ram[0x092D] && !(g_ram[0x7F] & 1)) ? 260 : 15);
}
BRIDGE_AFTER(HleHand_BallOrGauge, BallOrGauge_m1x1, ball_or_gauge_cost())
BRIDGE_DYN(HleHand_ResetCoinCounts, ResetCoinCounts_m1x1,
           60 + ((!(g_ram[0x63] & 1) && g_ram[0x76] == 3) ? 1300 : 15))
BRIDGE_DYN(HleHand_JumpResultTotal, JumpResultTotal_m1x1,
           15 + draw_all_riders_cost() + (g_ram[0x8C] == 1 ? 1900 : 10))
BRIDGE(HleHand_ConfettiAll, ConfettiAll_m1x1, 120)
BRIDGE_DYN(HleHand_PlayerCrashStep, PlayerCrashStep_m1x1, g_ram[0x01CE] ? 120 : 8)
BRIDGE_DYN(HleHand_PlayerCrashStepB, PlayerCrashStepB_m1x1, g_ram[0x01CE] ? 120 : 8)
BRIDGE(HleHand_WalkToStartLane, WalkToStartLane_m1x1, 30)
BRIDGE(HleHand_RaceClockStep, RaceClockStep_m1x1, 1355)
BRIDGE(HleHand_QueueFontTime, QueueFontTime_m1x1, 1000)
BRIDGE_DYN(HleHand_BonusCountUp, BonusCountUp_m1x1,
           (g_ram[0x21] | g_ram[0x26]) ? 3900 : (g_ram[0x88] == 1 ? 1350 : 12))
BRIDGE(HleHand_ResultsPlacePoints, ResultsPlacePoints_m0x1, 2700)
BRIDGE(HleHand_ResultsJumpPoints, ResultsJumpPoints_m0x1, 2400)
BRIDGE_DYN(HleHand_RaceTimer, RaceTimer_m1x1, g_ram[0x92] ? 1500 : 8)
BRIDGE_DYN(HleHand_BrakeOnTurn, BrakeOnTurn_m1x1, (X & 0xFF) == g_ram[0x7F] ? 45 : 8)
/* about 60 units of checks, the accelerate or brake call and the lean */
BRIDGE(HleHand_RiderThrottle, RiderThrottle_m1x1, 150)
static uint32 riders_throttle_cost(void)
{
    uint32 c = 25;
    for (int x = 0; x <= (X & 0xFF) && x < 8; x++)
        c += g_ram[0x01DE + x] ? 165 : 10;
    return c;
}
BRIDGE_DYN(HleHand_RidersThrottle, RidersThrottle_m1x1, riders_throttle_cost())
/* object steps and draws dispatched per active slot */
static uint32 update_objects_cost(void)
{
    uint32 c = 30;
    for (int x = 0; x <= 14; x += 2)
        c += g_ram[0x02EF + x] ? 350 : 12;
    return c;
}
BRIDGE_DYN(HleHand_UpdateObjects, UpdateObjects_m1x1, update_objects_cost())
/* the results (objects, and the points lines on their timer frames) or
 * the course records list (a line every $20 frames) */
static uint32 end_screens_cost(void)
{
    if (g_ram[0x7D]) {
        uint16 t = (uint16)(g_ram[0x8A] | g_ram[0x8B] << 8);
        return 60 + update_objects_cost() +
               (t == 0x04ED ? 2700 : t == 0x0475 ? 2400 : 0);
    }
    return g_ram[0x88] == 1 ? 1500 : 30;
}
BRIDGE_DYN(HleHand_BroadcastEndScreens, BroadcastEndScreens_m1x1, end_screens_cost())
BRIDGE_DYN(HleHand_StadiumBonusBanner, StadiumBonusBanner_m1x1,
           g_ram[0xD4] == 1 ? 1250 : g_ram[0xD4] == 0x7F ? 1400 : 15)
BRIDGE_DYN(HleHand_SpeedReadout, SpeedReadout_m1x1, (g_ram[0x63] & 1) ? 900 : 8)
/* engine sound, lights, twenty jump steps, speed readout and scroll */
BRIDGE_DYN(HleHand_JumpContestFrame, JumpContestFrame_m1x1,
           1100 + ((g_ram[0x63] & 1) ? 900 : 8) + (g_ram[0x00C7] == 2 ? 180 : 0))
BRIDGE(HleHand_QueueBonusDigitsLong, QueueBonusDigitsLong_m1x1, 1310)
BRIDGE(HleHand_QueueBestTimeLong, QueueBestTimeLong_m1x1, 1325)
BRIDGE_DYN(HleHand_BestTimeBlink, BestTimeBlink_m1x1,
           !g_ram[0xCB] ? 8 : ((g_ram[0xCB] - 1) & 0x0F) ? (g_ram[0xCB] == 1 ? 1350 : 20) : 900)
           g_ram[0x88] ? 15 : (g_ram[0xBA] ? 2400 : 200))
BRIDGE_DYN(HleHand_DemoPadDisplay, DemoPadDisplay_m1x1,
           200 + (g_ram[0x1FFFF] >= 6 ? 1450 : 15))
BRIDGE(HleHand_JumpHudNumbers, JumpHudNumbers_m1x1, 1300)
BRIDGE(HleHand_SetupJumpContest, SetupJumpContest_m1x1, 9000)
BRIDGE(HleHand_ShowLastDistance, ShowLastDistance_m1x1, 40)
BRIDGE(HleHand_JumpResultDistance, JumpResultDistance_m1x1, 750)
/* four score builds and queues, two distance displays (recompiled) */
BRIDGE(HleHand_JumpResultBoard, JumpResultBoard_m1x1, 4500)
BRIDGE_DYN(HleHand_JumpResultBoardStep, JumpResultBoardStep_m1x1, 4560 + draw_all_riders_cost())
BRIDGE(HleHand_ShowJumpCountLong, ShowJumpCountLong_m1x1, 3010)
BRIDGE(HleHand_ShowBonusPointsLong, ShowBonusPointsLong_m1x1, 2910)
BRIDGE_DYN(HleHand_JumpResultSkip, JumpResultSkip_m1x1,
           (g_ram[0x21] | g_ram[0x26]) ? 4000 : 10)
BRIDGE_DYN(HleHand_JumpContestMode, JumpContestMode_m1x1,
           120 + ((g_ram[0x21] | g_ram[0x26]) ? 4000 : 10))
BRIDGE(HleHand_ParkJumpRider, ParkJumpRider_m1x1, 12)
BRIDGE(HleHand_RiderThrottleLong, RiderThrottleLong_m1x1, 165)
BRIDGE(HleHand_AddSpeedToXLong, AddSpeedToXLong_m1x1, 45)
BRIDGE(HleHand_DrawRiderShadowPairLong, DrawRiderShadowPairLong_m1x1, 70)
BRIDGE_DYN(HleHand_JumpRiderStep, JumpRiderStep_m1x1, 520 + draw_all_riders_cost())
BRIDGE_DYN(HleHand_JumpRunStart, JumpRunStart_m1x1, 800 + draw_all_riders_cost())
BRIDGE(HleHand_MoveAndScroll, MoveAndScroll_m1x1, 100)
/* the track streamer stays recompiled and counts its own cycles */
BRIDGE_DYN(HleHand_JumpRun, JumpRun_m1x1, 1050 + draw_all_riders_cost())
BRIDGE(HleHand_ChangeModeLong, ChangeModeLong_m1x1, 30)
BRIDGE(HleHand_LeaveJumpContest, LeaveJumpContest_m1x1, 40)
BRIDGE(HleHand_JumpRunSteps, JumpRunSteps_m1x1, 660)
BRIDGE(HleHand_JumpRunStepsMode, JumpRunStepsMode_m1x1, 670)
/* VramQueueTilesLong / QueueTileRowsLong ($80:B2D6 / $80:B431) are
 * verified but unhooked: trampolines whose cost is the callee's. */
/* c/hand/s4/objects.c, part 26 */
/* UnpackToWram / UnpackToLowRam ($82:8BFE / $82:8BD8) pass difftest but
 * are not hooked: an unpack runs for several frames' worth of cycles and
 * the original takes NMIs part-way through it, which one bridged call
 * cannot; with them hooked the race run diverges whatever the cost. The C
 * counts the original's cycles (g_unpack_cycles) for when the bridge can
 * yield. */
/* c/hand/s4/objects.c, part 27 */
BRIDGE_DYN(HleHand_AimVelocity, AimVelocity_m1x1, (A & 0xFF) ? 150 + 25 * (A & 0xFF) : 45)
BRIDGE(HleHand_StartBonusTry, StartBonusTry_m1x1, 2600)
BRIDGE_DYN(HleHand_JumpResultCount, JumpResultCount_m1x1, 340 + (g_ram[0x8C] == 1 ? 3060 : 20))
BRIDGE_DYN(HleHand_JumpResultBonus, JumpResultBonus_m1x1, 340 + (g_ram[0x8C] == 1 ? 2960 : 20))
BRIDGE(HleHand_JumpResultWait, JumpResultWait_m1x1, 360)
BRIDGE(HleHand_StartPlayerSelect, StartPlayerSelect_m1x1, 5700)
BRIDGE_DYN(HleHand_JumpResultTally, JumpResultTally_m1x1, 360 + ((g_ram[0xCF] == 1 && g_ram[0xCE] >= 3) ? 300 : 0))
BRIDGE(HleHand_ResetRaceBoard, ResetRaceBoard_m1x1, 4600)
/* c/hand/s4/objects.c, part 28 */
BRIDGE(HleHand_StoreEntryNumber, StoreEntryNumber_m1x1, 820)
BRIDGE(HleHand_LoadPackNumber, LoadPackNumber_m1x1, 1370)
/* c/hand/s4/objects.c, part 29 */
BRIDGE(HleHand_StartBonusCourse, StartBonusCourse_m1x1, 7700)
/* c/hand/s4/objects.c, part 30 */
BRIDGE(HleHand_ShowEntryCode, ShowEntryCode_m1x1, 7000)
BRIDGE_DYN(HleHand_SteerObjectStep, SteerObjectStep_m1x1, (A & 0xFF) ? 330 + 25 * (A & 0xFF) : 220)
/* c/hand/s4/flow.c */
BRIDGE(HleHand_Mode02_SegmentCard, Mode02_SegmentCard_m1x1, 30)
/* c/hand/s4/flow.c, part 2 */
BRIDGE(HleHand_Mode11, Mode11_m1x1, 30)
BRIDGE(HleHand_Mode0F_EndScreen, Mode0F_EndScreen_m1x1, 190)
/* c/hand/s4/flow.c, part 3 */
BRIDGE(HleHand_DemoInputStep, DemoInputStep_m1x1, 60)
BRIDGE_DYN(HleHand_TitlePushStart, TitlePushStart_m1x1, g_ram[0x1FFFF] >= 6 ? 1450 : 15)
/* c/hand/s4/flow.c, part 4 */
BRIDGE(HleHand_OverheatStep, OverheatStep_m1x1, 60)
/* c/hand/s4/flow.c, part 5 */
BRIDGE(HleHand_ItemFlashState, ItemFlashState_m1x1, 30)
/* Stadium 4 race column loader (objects.c) */
BRIDGE(HleHand_NextColumn, NextColumn_m1x1, 15)
BRIDGE(HleHand_NextColumnLong, NextColumnLong_m1x1, 26)
BRIDGE(HleHand_RaceRowList, RaceRowList_m1x1, 23)
/* ~38 cycles while the builder column $0279 is not at the player column
 * $0266, ~1050 when it loads one. */
BRIDGE_DYN(HleHand_LoadRaceColumn, LoadRaceColumn_m1x1, g_ram[0x0279] != g_ram[0x0266] ? 38 : 1050)
/* Stadium 4 course builder and piece scripts (objects.c) */
BRIDGE(HleHand_CourseRowList, CourseRowList_m1x1, 18)
/* ~33 cycles while the builder column $0279 is not at the player column
 * $0266, ~900 when it builds one. */
BRIDGE_DYN(HleHand_BuildCourseColumn, BuildCourseColumn_m1x1, g_ram[0x0279] != g_ram[0x0266] ? 33 : 900)
BRIDGE(HleHand_RunRiderPieces, RunRiderPieces_m1x1, 70)
/* Stadium 4 race frame loops (objects.c) */
static uint32 throttle_all_cost4(void)
{
    uint32 c = 25;
    if (!g_ram[0x92]) return 20;
    for (int x = 0; x < 8; x++) c += g_ram[0x01DE + x] ? 165 : 10;
    return c;
}
static uint32 rider_steps_cost4(void)
{
    static const uint16 step[6] = { 5, 50, 30, 60, 25, 45 };
    uint32 c = 5;
    for (int x = 0; x < 8; x++) {
        uint8 st = g_ram[0x01D6 + x];
        c += 60 + (st < 6 ? step[st] : 0);
    }
    return c;
}
static uint32 ground_heights_cost4(int last)
{
    uint32 c = 5;
    for (int x = 0; x <= last && x < 8; x++) c += g_ram[0x01EE + x] ? 9 : 32;
    return c;
}
static uint32 piece_actions_cost4(void)
{
    uint32 c = 8;
    for (int x = 0; x < 8; x++) c += g_ram[0x0226 + x] ? 54 : 9;
    return c;
}
BRIDGE_DYN(HleHand_ThrottleAll, ThrottleAll_m1x1, throttle_all_cost4())
BRIDGE(HleHand_RiderStateStep, RiderStateStep_m1x1, 20)
BRIDGE_DYN(HleHand_RiderStepAll, RiderStepAll_m1x1, rider_steps_cost4())
BRIDGE_DYN(HleHand_SortAndDrawRiders, SortAndDrawRiders_m1x1, 705 + draw_all_riders_cost())
BRIDGE(HleHand_SpinAllWheels, SpinAllWheels_m1x1, 152)
BRIDGE(HleHand_RunAllRiderPieces, RunAllRiderPieces_m1x1, 600)
BRIDGE_DYN(HleHand_RiderGroundHeightsFrom, RiderGroundHeightsFrom_m1x1, ground_heights_cost4(X & 0xFF))
BRIDGE_DYN(HleHand_RiderGroundHeights, RiderGroundHeights_m1x1, 2 + ground_heights_cost4(7))
BRIDGE_DYN(HleHand_RunPieceActions, RunPieceActions_m1x1, piece_actions_cost4())
BRIDGE(HleHand_TurboMeterJmp, TurboMeterJmp_m1x1, 242)
/* Stadium 4 course best and bonus course builder (objects.c) */
BRIDGE(HleHand_ShowCourseBest, ShowCourseBest_m1x1, 1350)
BRIDGE(HleHand_BonusRowList, BonusRowList_m1x1, 18)
BRIDGE_DYN(HleHand_BuildBonusColumn, BuildBonusColumn_m1x1, g_ram[0x0279] != g_ram[0x0266] ? 40 : 900)
/* carried over from Stadium 1 (identical or moved-operand code, c/hand/s4/from_s1.c) */
BRIDGE(HleHand_DmaFillZero, DmaFillZero_m0x1, 70)
BRIDGE(HleHand_ReadJoypads, ReadJoypads_m1x1, 150)
BRIDGE(HleHand_NmiFlushVramQueue, NmiFlushVramQueue_m1x1, 400)
BRIDGE(HleHand_StepLfsr, StepLfsr_m1x1, 80)
BRIDGE(HleHand_Mode00_IntroWait, Mode00_IntroWait_m1x1, 60)
BRIDGE(HleHand_Mode0E_TitleWait, Mode0E_TitleWait_m1x1, 40)
BRIDGE(HleHand_FinishModeSetup, FinishModeSetup_m1x1, 40)
BRIDGE(HleHand_StepObjectX, StepObjectX_m1x1, 80)
BRIDGE(HleHand_ComputePlaces, ComputePlaces_m1x1, 1500)
BRIDGE(HleHand_ApplyVelocity, ApplyVelocity_m1x1, 40)
BRIDGE(HleHand_ClearWords1530, ClearWords1530_m1x1, 40)
BRIDGE(HleHand_ResetMinuteFrame, ResetMinuteFrame_m1x1, 30)
BRIDGE(HleHand_Nop_BD39, Nop_BD39_m1x1, 20)
BRIDGE(HleHand_Tick08CC, Tick08CC_m1x1, 40)
BRIDGE(HleHand_DivideAX, DivideAX_m1x1, 100)
BRIDGE(HleHand_BlankQueueRow, BlankQueueRow_m1x1, 100)
BRIDGE(HleHand_BlankQueueRowPal2, BlankQueueRowPal2_m1x1, 80)
BRIDGE(HleHand_BeginFadeIn, BeginFadeIn_m1x1, 40)
BRIDGE(HleHand_SendSpeedToApu, SendSpeedToApu_m1x1, 40)
BRIDGE(HleHand_RiderShadowY, RiderShadowY_m1x1, 40)
BRIDGE(HleHand_Add32, Add32_m1x1, 40)
BRIDGE(HleHand_DispatchBy85, DispatchBy85_m1x1, 40)
BRIDGE(HleHand_StartMode01, StartMode01_m1x1, 40)
BRIDGE(HleHand_BsxReadSerialByte, BsxReadSerialByte_m1x1, 80)
BRIDGE(HleHand_GrowBar087E, GrowBar087E_m1x1, 80)
BRIDGE(HleHand_StepColourCycle, StepColourCycle_m1x1, 80)
BRIDGE(HleHand_MergeBsxFlags, MergeBsxFlags_m1x1, 80)
BRIDGE(HleHand_LaneFromY, LaneFromY_m1x1, 80)
BRIDGE(HleHand_MarkFastRider, MarkFastRider_m1x1, 80)
BRIDGE(HleHand_DrawRiderShadowPair, DrawRiderShadowPair_m1x1, 60)
BRIDGE(HleHand_ScrollRivalsRelative, ScrollRivalsRelative_m1x1, 200)
BRIDGE(HleHand_RiderLaneBand, RiderLaneBand_m1x1, 80)
BRIDGE(HleHand_SetupSpriteRow15, SetupSpriteRow15_m1x1, 400)
BRIDGE(HleHand_RiderHeightStep, RiderHeightStep_m1x1, 60)
BRIDGE(HleHand_ObjectFacing, ObjectFacing_m1x1, 120)
BRIDGE(HleHand_AddScore, AddScore_m1x1, 200)
BRIDGE(HleHand_ApplyVelocityX, ApplyVelocityX_m1x1, 40)
BRIDGE(HleHand_SpawnPlayerSpark, SpawnPlayerSpark_m1x1, 80)
BRIDGE(HleHand_StepSpark, StepSpark_m1x1, 80)

/* The Gen B sample upload ($82:8593) from the hand-written player select,
 * through the framework's native transfer (the running sound driver's own
 * handshake, live): the original streams and the roster's added ones. The
 * boot-time uploads keep the original routine (the sound CPU's boot ROM is
 * listening then, not the driver). */
extern bool RtlUploadSpcImageFromDpLive(CpuState *cpu);
static void spc_upload_dp(void)
{
    cpu_write8(&g_cpu, 0x00, 0x1978, 0);
    to_cpu(&g_cpu, 0);
    RtlUploadSpcImageFromDpLive(&g_cpu);
    from_cpu(&g_cpu);
}
static void spc_set_dp(uint32_t stream)
{
    uint16_t d = D;
    cpu_write8(&g_cpu, 0x00, (uint16_t)(d + 0), (uint8_t)stream);
    cpu_write8(&g_cpu, 0x00, (uint16_t)(d + 1), (uint8_t)(stream >> 8));
    cpu_write8(&g_cpu, 0x00, (uint16_t)(d + 2), (uint8_t)(stream >> 16));
}
void SpcUploadB_m1x1(void)
{
    uint8_t x = (uint8_t)X;
    uint32_t stream = cpu_read8(&g_cpu, 0x82, (uint16_t)(0x857E + x)) |
                      cpu_read8(&g_cpu, 0x82, (uint16_t)(0x8585 + x)) << 8 |
                      (uint32_t)cpu_read8(&g_cpu, 0x82, (uint16_t)(0x858C + x)) << 16;
    cpu_write8(&g_cpu, 0x00, 0x2140, 0xFF);
    spc_set_dp(stream);
    spc_upload_dp();
    S++; S++; S++;                  /* RTL: drop the return frame the caller pushed */
}
void SpcUploadFrom(uint32_t stream)
{
    cpu_write8(&g_cpu, 0x00, 0x2140, 0xFF);
    spc_set_dp(stream);
    spc_upload_dp();
}
#endif
