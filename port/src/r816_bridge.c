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
