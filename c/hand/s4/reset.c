/*
 * reset.c - Stadium 4's Reset handler ($80:FFA5 -> $80:8000): the boot and
 * the main loop. Reset never returns, so difftest checks it in three
 * fragments (fragments.txt): the boot up to the APU wait, the boot after
 * it, and one main loop frame after the NMI wait. Not in the port build:
 * the port runs the recompiled main loop, which yields at the NMI wait.
 */
#include "runtime816.h"

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
    lda8(rd8(LONG(0x7FFFE1)));
    if (fz) { push8(0x80); push16(0x801C); BsxOpenTimeChannel_m1x1(); }
    lda8(0x13); wr8(LONG(0x7FFFF9), 0x13);
    lda8(0x02); wr8(LONG(0x7FFFFF), 0x02); wr8(LONG(0x7FFFFA), 0x02);
    lda8(0x00); wr8(LONG(0x7FFFFB), 0x00);
    lda8(0x1E); wr8(ABS(0x1E84), 0x1E);
    wr8(ABS(0x1E85), 0);
    sep(0x20);
    lda8(0x00); push8(0x00); DB = pull8(); nz8(DB);
    wr8(ABS(0x4200), 0);
    wr8(ABS(0x420C), 0);
    wr8(ABS(0x420B), 0);
    wr8(ABS(0x2140), 0);
    wr8(ABS(0x2141), 0);
    wr8(ABS(0x2142), 0);
    wr8(ABS(0x2143), 0);
    lda8(0x80); wr8(ABS(0x2100), 0x80);
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
        wr8(ABS(0x4200), 0);
        lda8(rd8(ABS(0x2140)));
        ora8(rd8(ABS(0x2141)));
        ora8(rd8(ABS(0x2142)));
        ora8(rd8(ABS(0x2143)));
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
    lda16(0x00D0); wr16(ABS(0x4209), A);
    lda16(0x0098); wr16(ABS(0x4207), A);
    sep(0x20);
    lda8(0x01); wr8(ABS(0x420D), 0x01);
    lda8(0x81); wr8(ABS(0x4200), 0x81);
    wr8(ABS(0x190E), 0x81);
    wr8(ABS(0x2141), 0);
    wr8(ABS(0x2142), 0);
    wr8(ABS(0x2143), 0);
    lda8(0xFC); wr8(ABS(0x2140), 0xFC);
    wr8(ABS(0x1E87), 0);
    wr8(ABS(0x1E88), 0);
    wr8(ABS(0x1566), 0);
    lda8(0x04); wr8(ABS(0x083A), 0x04);
    wr8(ABS(0x083B), 0);
    wr8(ABS(0x083C), 0);
    lda8(0x04);
    wr8(ABS(0x083D), 0x04);
    wr8(ABS(0x0840), 0x04);
    wr8(ABS(0x0843), 0x04);
    wr8(ABS(0x0846), 0x04);
    lda8(0x0D); wr8(ABS(0x0076), 0x0D);
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
    lda8(rd8(LONG(0x7FFFFF)));
    wr8(ABS(0x00BF), (u8)A);
    push8(0x80); push16(0x8112); BsxPollStream_m1x1();
    push16(0x8115); StepLfsr_m1x1();
    push8(0x80); push16(0x8119); ReadJoypads_m1x1();
    fi = 0;
    lda8(rd8(LONG(0x81800E)));
    if (!fz) {
        lda8(rd8(DP(0x27)));
        and8(0xC0);
        if (!fz) {
            lda8(asl8((u8)A));
            lda8(rd8(LONG(0x7FFFFF)));
            if (fc) {
                lda8(dec8((u8)A));
                if (fn) lda8(0x00);
            } else {
                lda8(inc8((u8)A));
            }
            wr8(LONG(0x7FFFFF), (u8)A);
            wr8(ABS(0x00B8), inc8(rd8(ABS(0x00B8))));
            lda8(0x01); wr8(ABS(0x1E84), 0x01);
            rep(0x20);
            lda16(0x0000); wr16(LONG(0x7FFFFA), 0);
            sep(0x20);
        }
        lda8(rd8(DP(0x26)));
        and8(0x20);
        if (!fz) wr8(ABS(0x1917), inc8(rd8(ABS(0x1917))));
        lda8(rd8(DP(0x26)));
        and8(0x10);
        if (fz) {
            lda8(rd8(ABS(0x1917)));
            and8(0x01);
            wr8(ABS(0x1917), (u8)A);
            if (!fz) goto paused;
        }
    }
    lda8(rd8(ABS(0x1923)));
    if (!fz) {
        push8(0x80); push16(0x8176); StepFade_m1x1();
        goto tail;
    }
    rep(0x20);
    wr16(DP(0x63), inc16(rd16(DP(0x63))));
    sep(0x20);
    push8(0x80); push16(0x8182); TickTimers_m1x1();
    push8(0x80); push16(0x8186); PlaceMinuteMarker_m1x1();
    push8(0x80); push16(0x818A); HideAllSprites_m1x1();
    push16(0x818D); RunGameModeScript_m1x1();
paused:
    lda8(rd8(DP(0x94)));
    if (!fz) {
        push16(0x8194); LaneFromY_m1x1();
        push16(0x8197); RaceFrameStep_m1x1();
    }
tail:
    push8(0x80); push16(0x819B); RiderSpriteSets_m1x1();
    push8(0x80); push16(0x819F); PackSpriteSizes_m1x1();
    push16(0x81A2); DrawClockB_m1x1();
    wr8(ABS(0x0817), 0);
}

/* $80:FFA5 -> $80:8000 Reset: the boot, then a main loop frame after each
 * NMI. */
void ResetVector_m1x1(void)
{
    ResetBoot4a();
    ResetBoot4aSpc();
    wait_apu_idle4();
    ResetBoot4b();
    for (;;) {
        do lda8(rd8(ABS(0x0817))); while (fz);
        MainLoopFrame4();
    }
}
