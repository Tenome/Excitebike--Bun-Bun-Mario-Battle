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
/* Not bridged: RunGameModeScript (c/hand/s1/modes.c). Its reset path makes
 * non-tail calls into recompiled routines, and a recompiled routine may yield
 * to the frame scheduler mid-call (a long one like $80:B873 does), which hand
 * C cannot resume. Rule: bridge only routines whose calls stay inside
 * hand-written code. */
#endif
