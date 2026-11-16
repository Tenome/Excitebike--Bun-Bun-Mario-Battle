/*
 * reset.c - Stadium 4's Reset handler ($80:FFA5 -> $80:8000): the boot and
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
void SpcUploadB_m1x1(void);
void ResetSoundState_m1x1(void);
void BsxPollStream_m1x1(void);
void StepLfsr_m1x1(void);
void ReadJoypads_m1x1(void);
void StepFade_m1x1(void);
void TickTimers_m1x1(void);
void PlaceMinuteMarker_m1x1(void);
void HideAllSprites_m1x1(void);
void RunGameModeScript_m1x1(void);
void LaneFromY_m1x1(void);
void RaceFrameStep_m1x1(void);
void RiderSpriteSets_m1x1(void);
void PackSpriteSizes_m1x1(void);
void DrawClockB_m1x1(void);

/* $80:8000-$80:8077: native mode, D = 0, stack $1FFF; BS-X detection (the
 * time channel when $7F:FFE1 is clear); broadcast clock minute 2, frame 2,
 * $7F:FFF9 = $13, $7F:FFFB = 0; DB = 0; NMI, DMA and APU ports off, forced
 * blank, the PPU registers. */
R816_EXPORT void ResetBoot4a(void)
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
    lda8(0x00); wr8(LONG(BCAST_FRAME_HI), 0x00);
    lda8(0x1E); wr8(ABS(CLOCK_TIMER), 0x1E);
    wr8(ABS(CLOCK_COLON), 0);
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

/* $80:8072: the first SPC upload (set 0). */
R816_EXPORT void ResetBoot4aSpc(void)
{
    ldx8(0x00);
    push8(0x80); push16(0x8077); SpcUploadB_m1x1();
}

/* $80:8078: NMI off until all four APU ports read 0. */
static void wait_apu_idle4(void)
{
    do {
        wr8(ABS(NMITIMEN), 0);
        lda8(rd8(ABS(APUIO0)));
        ora8(rd8(ABS(APUIO1)));
        ora8(rd8(ABS(APUIO2)));
        ora8(rd8(ABS(APUIO3)));
    } while (!fz);
}

/* $80:8089-$80:80E9: the second SPC upload (set 2), the sound state; DB = $80; IRQ
 * timer at V $D0 / H $98, FastROM, NMI on; APU port 0 = $FC; clock and
 * game state defaults; game mode $0D. */
R816_EXPORT void ResetBoot4b(void)
{
    ldx8(0x02);
    push8(0x80); push16(0x808E); SpcUploadB_m1x1();
    push8(0x80); push16(0x8092); ResetSoundState_m1x1();
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
    wr8(ABS(CLOCK_ON), 0);
    lda8(0x04); wr8(ABS(RECORD_TIME), 0x04);
    wr8(ABS(RECORD_TIME + 0x01), 0);
    wr8(ABS(RECORD_TIME + 0x02), 0);
    lda8(0x04);
    wr8(ABS(COURSE_BEST), 0x04);
    wr8(ABS(COURSE_BEST + 0x03), 0x04);
    wr8(ABS(COURSE_BEST + 0x06), 0x04);
    wr8(ABS(COURSE_BEST + 0x09), 0x04);
    lda8(0x0D); wr8(ABS(GAME_MODE), 0x0D);
}

/* $80:8108-$80:81A6: one main loop frame after NMI ($0817 set): minute to
 * $BF, the BS-X stream, the random shift register, the joypads, IRQs on.
 * In the debug build ($81:800E) pad 2's L/R step the broadcast minute
 * (restarting it) and Select / Start toggle a pause ($1917) that skips the
 * game logic. Then a fade step, or the frame counter $63, timers, the minute
 * marker, hidden sprites and the game mode script; the race's lanes and
 * frame step when $94; rider sprite sets, sprite sizes, the clock;
 * $0817 cleared. */
R816_EXPORT void MainLoopFrame4(void)
{
    lda8(rd8(LONG(BCAST_MINUTE)));
    wr8(ABS(0x00BF), (u8)A);
    push8(0x80); push16(0x8112); BsxPollStream_m1x1();
    push16(0x8115); StepLfsr_m1x1();
    push8(0x80); push16(0x8119); ReadJoypads_m1x1();
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
            wr8(ABS(MINUTE_TICKS), inc8(rd8(ABS(MINUTE_TICKS))));
            lda8(0x01); wr8(ABS(CLOCK_TIMER), 0x01);
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
        push8(0x80); push16(0x8176); StepFade_m1x1();
        goto tail;
    }
    rep(0x20);
    wr16(DP(FRAME_COUNT), inc16(rd16(DP(FRAME_COUNT))));
    sep(0x20);
    push8(0x80); push16(0x8182); TickTimers_m1x1();
    push8(0x80); push16(0x8186); PlaceMinuteMarker_m1x1();
    push8(0x80); push16(0x818A); HideAllSprites_m1x1();
    push16(0x818D); RunGameModeScript_m1x1();
paused:
    lda8(rd8(DP(VSCROLL_ZOOM)));
    if (!fz) {
        push16(0x8194); LaneFromY_m1x1();
        push16(0x8197); RaceFrameStep_m1x1();
    }
tail:
    push8(0x80); push16(0x819B); RiderSpriteSets_m1x1();
    push8(0x80); push16(0x819F); PackSpriteSizes_m1x1();
    push16(0x81A2); DrawClockB_m1x1();
    wr8(ABS(NMI_FRAME_READY), 0);
}

/* $80:FFA5 -> $80:8000 Reset: the boot, then a main loop frame after each
 * NMI. */
/* The boot alone, and one turn of the main loop (a frame if the NMI has
 * flagged 0817): the port's frame-loop native mode calls these once and
 * once per frame instead of the endless loop (port/src/native_driver.c). */
R816_EXPORT void ResetBootAll(void)
{
    ResetBoot4a();
    ResetBoot4aSpc();
    wait_apu_idle4();
    ResetBoot4b();
}

R816_EXPORT void MainLoopTurn(void)
{
    lda8(rd8(ABS(NMI_FRAME_READY)));
    if (!fz)
        MainLoopFrame4();
}

void ResetVector_m1x1(void)
{
    ResetBootAll();
    for (;;) {
        do lda8(rd8(ABS(NMI_FRAME_READY))); while (fz);
        MainLoopFrame4();
    }
}
