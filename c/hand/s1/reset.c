/*
 * reset.c - Stadium 1's Reset handler ($80:FFA5 -> $80:8000): the boot and
 * the main loop. Reset never returns, so difftest checks it in three
 * fragments (fragments.txt): the boot up to the APU wait, the boot after
 * it, and one main loop frame after the NMI wait. Not in the port build:
 * the port runs the recompiled main loop, which yields at the NMI wait.
 */
#include "runtime816.h"
#include "ram.h"

void BsxDetect_m1x1(void);
void BsxOpenTimeChannel_m1x1(void);
void InitPpuRegisters_m1x1(void);
void SpcUpload_m1x1(void);
void InitFadeState_m1x1(void);
void BsxPollStream_m1x1(void);
void StepLfsr_m1x1(void);
void ReadJoypads_m1x1(void);
void FadeStep_m1x1(void);
void TickTimers_m1x1(void);
void BroadcastEventInject_m1x1(void);
void ClearOamBuffer_m1x1(void);
void RunGameModeScript_m1x1(void);
void LaneFromY_m1x1(void);
void RaceBookkeeping_m1x1(void);
void RiderPaletteWords_m1x1(void);
void PackOamHighTable_m1x1(void);
void DrawClock_m1x1(void);

/* $80:8000-$80:807A: native mode, D = 0, stack $1FFF; BS-X detection (the
 * time channel when $7F:FFE1 is clear); broadcast clock minute 2, frame 2,
 * $7F:FFF9 = $13, $7F:FFFB = 5; DB = 0; NMI, DMA and APU ports off, forced
 * blank, the PPU registers. */
R816_EXPORT void ResetBoot1(void)
{
    fi = 1;
    fc = 0;                                 /* CLC; XCE (native mode) */
    rep(0x30);
    lda16(0x0000); D = A; nz16(D);
    lda16(0x1FFF); S = A;
    sep(0x30);
    push8(0x80); push16(0x8012); BsxDetect_m1x1();
    lda8(rd8(LONG(NO_RECEIVER)));
    if (fz) { push8(0x80); push16(0x801C); BsxOpenTimeChannel_m1x1(); }
    lda8(0x13); wr8(LONG(BCAST_HOUR), 0x13);
    lda8(0x02); wr8(LONG(BCAST_MINUTE), 0x02); wr8(LONG(BCAST_FRAME), 0x02);
    lda8(0x05); wr8(LONG(BCAST_FRAME_HI), 0x05);
    lda8(0x1E); wr8(ABS(0x1E84), 0x1E);
    wr8(ABS(0x1E85), 0);
    sep(0x20);
    lda8(0x00); push8(0x00); DB = pull8(); nz8(DB);
    wr8(ABS(NMITIMEN), 0);
    wr8(ABS(HDMAEN), 0);
    wr8(ABS(MDMAEN), 0);
    wr8(ABS(APUIO0), 0);
    wr8(ABS(APUIO1), 0);
    wr8(ABS(APUIO2), 0);
    wr8(ABS(APUIO3), 0);
    lda8(0x80); wr8(ABS(INIDISP), 0x80);
    push8(0x80); push16(0x805E); InitPpuRegisters_m1x1();
    lda8(0x00);
    wr8(ABS(0x1E1C), 0);
    wr8(DP(0xF2), 0);
    wr8(DP(0xF4), 0);
    lda8(0x00);
    wr8(LONG(0x7FFFE3), 0);
    wr8(LONG(0x7FFFE4), 0);
}

/* $80:8072: the first SPC upload (set 0), then NMI off. */
R816_EXPORT void ResetBoot1Spc(void)
{
    ldx8(0x00);
    push8(0x80); push16(0x8077); SpcUpload_m1x1();
    wr8(ABS(NMITIMEN), 0);
}

/* $80:807B: wait until all four APU ports read 0. */
static void wait_apu_idle1(void)
{
    do {
        lda8(rd8(ABS(APUIO0)));
        ora8(rd8(ABS(APUIO1)));
        ora8(rd8(ABS(APUIO2)));
        ora8(rd8(ABS(APUIO3)));
    } while (!fz);
}

/* $80:8089-$80:80E9: the second SPC upload, the fade state; DB = $80; IRQ
 * timer at V $D0 / H $98, FastROM, NMI on; APU port 0 = $FC; clock and
 * game state defaults; game mode $0D. ($80:80EB-$80:80FD, skipped by the
 * BRA, is a leftover debug start: $1542 = 1, mode 4, $7D = 1, TM = $17.) */
R816_EXPORT void ResetBoot2(void)
{
    ldx8(0x01);
    push8(0x80); push16(0x808E); SpcUpload_m1x1();
    push8(0x80); push16(0x8092); InitFadeState_m1x1();
    lda8(0x80); push8(0x80); DB = pull8(); nz8(DB);
    rep(0x20);
    lda16(0x00D0); wr16(ABS(VTIMEL), A);
    lda16(0x0098); wr16(ABS(HTIMEL), A);
    sep(0x20);
    lda8(0x01); wr8(ABS(MEMSEL), 0x01);
    lda8(0x81); wr8(ABS(NMITIMEN), 0x81);
    wr8(ABS(SH_NMITIMEN), 0x81);
    wr8(ABS(APUIO1), 0);
    wr8(ABS(APUIO2), 0);
    wr8(ABS(APUIO3), 0);
    lda8(0xFC); wr8(ABS(APUIO0), 0xFC);
    wr8(ABS(BONUS_TRIES), 0);
    wr8(ABS(BONUS_BEST), 0);
    wr8(ABS(0x1556), 0);
    lda8(0x04); wr8(ABS(RECORD_TIME), 0x04);
    wr8(ABS(RECORD_TIME + 0x01), 0);
    wr8(ABS(RECORD_TIME + 0x02), 0);
    lda8(0x04);
    wr8(ABS(0x083D), 0x04);
    wr8(ABS(0x0840), 0x04);
    wr8(ABS(0x0843), 0x04);
    wr8(ABS(0x0846), 0x04);
    lda8(0x0D); wr8(ABS(GAME_MODE), 0x0D);
}

/* $80:8103-$80:819C: one main loop frame after NMI ($081A set): minute to
 * $B7, the BS-X stream, the random shift register, the joypads, IRQs on.
 * In the debug build ($81:800E) pad 2's L/R step the broadcast minute
 * (restarting it) and Select / Start toggle a pause ($1917) that skips the
 * game logic. Then a fade step, or the frame counter $63, timers, broadcast
 * events, the OAM buffer and the game mode script; the race's lanes and
 * bookkeeping when $90; rider palettes, the OAM high table, the clock;
 * $081A cleared. */
R816_EXPORT void MainLoopFrame1(void)
{
    lda8(rd8(LONG(BCAST_MINUTE)));
    wr8(ABS(0x00B7), (u8)A);
    push8(0x80); push16(0x810D); BsxPollStream_m1x1();
    push16(0x8110); StepLfsr_m1x1();
    push8(0x80); push16(0x8114); ReadJoypads_m1x1();
    fi = 0;
    lda8(rd8(LONG(DEBUG_ENABLED)));
    if (!fz) {
        lda8(rd8(DP(P2_PRESS_LO)));
        and8(0xC0);
        if (!fz) {
            lda8(asl8((u8)A));
            lda8(rd8(LONG(BCAST_MINUTE)));
            if (fc) {
                lda8(dec8((u8)A));
                if (fn) lda8(0x00);
            } else {
                lda8(inc8((u8)A));
            }
            wr8(LONG(BCAST_MINUTE), (u8)A);
            wr8(ABS(0x00B0), inc8(rd8(ABS(0x00B0))));
            lda8(0x01); wr8(ABS(0x1E84), 0x01);
            rep(0x20);
            lda16(0x0000); wr16(LONG(BCAST_FRAME), 0);
            sep(0x20);
        }
        lda8(rd8(DP(P1_PRESS_LO)));
        and8(0x20);
        if (!fz) wr8(ABS(DEBUG_PAUSE), inc8(rd8(ABS(DEBUG_PAUSE))));
        lda8(rd8(DP(P1_PRESS_LO)));
        and8(0x10);
        if (fz) {
            lda8(rd8(ABS(DEBUG_PAUSE)));
            and8(0x01);
            wr8(ABS(DEBUG_PAUSE), (u8)A);
            if (!fz) goto paused;
        }
    }
    lda8(rd8(ABS(FADE_STATE)));
    if (!fz) {
        push8(0x80); push16(0x8171); FadeStep_m1x1();
        goto tail;
    }
    rep(0x20);
    wr16(DP(FRAME_COUNT), inc16(rd16(DP(FRAME_COUNT))));
    sep(0x20);
    push16(0x817C); TickTimers_m1x1();
    push16(0x817F); BroadcastEventInject_m1x1();
    push16(0x8182); ClearOamBuffer_m1x1();
    push16(0x8185); RunGameModeScript_m1x1();
paused:
    lda8(rd8(DP(VSCROLL_ZOOM)));
    if (!fz) {
        push16(0x818C); LaneFromY_m1x1();
        push16(0x818F); RaceBookkeeping_m1x1();
    }
tail:
    push16(0x8192); RiderPaletteWords_m1x1();
    push16(0x8195); PackOamHighTable_m1x1();
    push16(0x8198); DrawClock_m1x1();
    wr8(ABS(NMI_FRAME_READY), 0);
}

/* $80:FFA5 -> $80:8000 Reset: the boot, then a main loop frame after each
 * NMI. */
/* The boot alone, and one turn of the main loop (a frame if the NMI has
 * flagged 081A): the port's frame-loop native mode calls these once and
 * once per frame instead of the endless loop (port/src/native_driver.c). */
R816_EXPORT void ResetBootAll(void)
{
    ResetBoot1();
    ResetBoot1Spc();
    wait_apu_idle1();
    ResetBoot2();
}

R816_EXPORT void MainLoopTurn(void)
{
    lda8(rd8(ABS(NMI_FRAME_READY)));
    if (!fz)
        MainLoopFrame1();
}

void ResetVector_m1x1(void)
{
    ResetBootAll();
    for (;;) {
        do lda8(rd8(ABS(NMI_FRAME_READY))); while (fz);
        MainLoopFrame1();
    }
}
