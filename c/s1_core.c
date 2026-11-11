/*
 * Excitebike: Bun Bun Mario Battle - Stadium 1 (Gen A engine)
 * Hand-decompiled core routines.
 *
 * Every function cites its original address. Hardware goes through
 * hw_w8/hw_w16/hw_r8 so a port can route it to its own PPU/DMA model.
 * WRAM is wram[0x20000]; direct page 0 and bank $00 low RAM are wram[0..0x1FFF].
 *
 * Status: hand-lifted from the bsdis.py listing. Not yet verified against
 * the original by a difftest harness (see docs/PLAN.md phase 4).
 */
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

extern u8 wram[0x20000];
extern void hw_w8(u16 reg, u8 v);
extern void hw_w16(u16 reg, u16 v);     /* low byte to reg, high byte to reg+1 */
extern u8 hw_r8(u16 reg);

/* ---- SNES registers used here ---------------------------------------- */
enum {
    INIDISP = 0x2100, M7A = 0x211B, M7B = 0x211C, WMADDL = 0x2181, WMADDH = 0x2183,
    APUIO0 = 0x2140, NMITIMEN = 0x4200, MDMAEN = 0x420B, HDMAEN = 0x420C,
    DMAP0 = 0x4300, A1TL0 = 0x4302, A1B0 = 0x4304, DASL0 = 0x4305,
    /* BS-X satellite receiver (bus B) */
    BSX_2188 = 0x2188, BSX_2189 = 0x2189, BSX_218E = 0x218E, BSX_218F = 0x218F,
    BSX_2191 = 0x2191, BSX_2192 = 0x2192, BSX_2194 = 0x2194,
};

/* ---- RAM -------------------------------------------------------------- */
#define DP(a)        (wram[(a) & 0xFF])                 /* D = $0000 */
#define DP16(a)      ((u16)(wram[(a)] | wram[(a) + 1] << 8))
#define RAM8(a)      (wram[(a)])
#define RAM16(a)     ((u16)(wram[(a)] | wram[(a) + 1] << 8))
#define SET16(a, v)  (wram[(a)] = (u8)(v), wram[(a) + 1] = (u8)((v) >> 8))
/* $7F:FFE0-$7F:FFFF: BS-X handshake area at the very top of WRAM */
#define BSRAM(a)     (wram[0x10000 + ((a) & 0xFFFF)])

#define bs_no_receiver   BSRAM(0xFFE1)   /* 1 = no satellite receiver found */
#define bs_stream_id     0xFFF7          /* 16-bit, last opened channel id  */

/* ======================================================================
 * DMA helpers
 * ====================================================================== */

/*
 * $80:8539  DmaKick  (M=0, X=1; called with D = $4300)
 *   STA $02 / STX $04 / STY MDMAEN / RTS
 * With the direct page pointing at the DMA registers, $02/$04 are channel 0's
 * A1T0/A1B0.  The caller has already set DMAP0/BBAD0 ($00 = $1801 for VRAM)
 * and DAS0 ($05).  A = source address, X = source bank, Y = channel mask.
 */
void DmaKick(u16 src, u8 bank, u8 channels)
{
    hw_w16(A1TL0, src);
    hw_w8(A1B0, bank);
    hw_w8(MDMAEN, channels);
}

/*
 * $80:8541  DmaCopyToWram  (M=0, X=1, JSL)
 *   A = WRAM destination (low 16 bits), Y = WRAM bank bit ($2183)
 *   $02/$03 = source address, $04 = source bank, $00/$01 = byte count.
 * Channel 0 writes through WMDATA ($2180): DMAP=$00, BBAD=$80.
 */
void DmaCopyToWram(u16 dst, u8 dst_bank)
{
    hw_w16(WMADDL, dst);
    hw_w8(WMADDH, dst_bank);
    hw_w16(DMAP0, 0x8000);              /* DMAP0=$00, BBAD0=$80 (WMDATA) */
    hw_w16(A1TL0, DP16(0x02));
    hw_w8(A1B0, DP(0x04));
    hw_w16(DASL0, DP16(0x00));
    hw_w8(MDMAEN, 0x01);
}

/*
 * $80:8562  DmaFillZero  (M=0, X=1, JSL)
 *   A = destination address, Y = destination bank, $00/$01 = byte count.
 * Classic trick: set the Mode 7 multiplier to 1 x 0, so MPYL ($2134) reads 0,
 * then DMA B->A from $2134 into memory with the A-bus address incrementing.
 * Side effect: clears DP $02.
 */
void DmaFillZero(u16 dst, u8 bank)
{
    DP(0x02) = 0;
    hw_w16(A1TL0, dst);
    hw_w8(A1B0, bank);
    hw_w8(M7A, 0x01);                   /* M7A = $0001 (write-twice register) */
    hw_w8(M7A, 0x00);
    hw_w8(M7B, DP(0x02));               /* M7B = 0 -> product 0 */
    hw_w16(DMAP0, 0x3480);              /* DMAP0=$80 (B->A), BBAD0=$34 (MPYL) */
    hw_w16(DASL0, DP16(0x00));
    hw_w8(MDMAEN, 0x01);
}

/* ======================================================================
 * BS-X satellite library (bank $81, $F80C-$FAB2)
 * The only code that touches the satellite receiver.
 * ====================================================================== */

/*
 * $81:FA81  BsxProbeReceiver  (M=1, X=0)
 * Writes $AA and $55 to $2188 and checks they read back, then clears four
 * stream registers and ORs their read-back.  Returns true (Z clear in the
 * original) when the receiver is *absent* or misbehaves.
 */
static bool BsxProbeReceiver(void)
{
    hw_w8(BSX_2194, 0x01);
    hw_w8(BSX_2188, 0xAA);
    if (hw_r8(BSX_2188) != 0xAA) return true;
    hw_w8(BSX_2188, 0x55);
    if (hw_r8(BSX_2188) != 0x55) return true;
    hw_w8(BSX_2188, 0); hw_w8(BSX_2189, 0);
    hw_w8(BSX_218E, 0); hw_w8(BSX_218F, 0);
    return (hw_r8(BSX_2188) | hw_r8(BSX_2189) | hw_r8(BSX_218E) | hw_r8(BSX_218F)) != 0;
}

/*
 * $81:FA68  BsxDetect  (JSL from Reset)
 * $7FFFE1 = 1 when there is no usable receiver.  The rest of the game checks
 * this flag and skips all satellite / SoundLink features, so a stock SNES (or
 * a native port) runs the game without the BS-X BIOS.
 */
void BsxDetect(void)
{
    bs_no_receiver = 0;
    if (BsxProbeReceiver())
        bs_no_receiver = 1;
}

extern bool BsxFindChannel(void);       /* $81:F9C8: carry set = not found */

/*
 * $81:F80C  BsxOpenTimeChannel  (JSL from Reset when a receiver exists)
 * Looks up the broadcast channel and points stream 1 at it.  On failure,
 * stores the sentinel $5533 (and $AAEE to $218E), so later polls never match.
 * Returns carry: false = opened, true = failed.
 */
bool BsxOpenTimeChannel(void)
{
    BSRAM(0xFFFA) = 0; BSRAM(0xFFFB) = 0;
    SET16(0x13D3, 0x0101);
    SET16(0x13D5, 0x0800);
    if (BsxFindChannel()) {
        BSRAM(bs_stream_id) = 0x33; BSRAM(bs_stream_id + 1) = 0x55;
        hw_w16(BSX_218E, 0xAAEE);
        return true;
    }
    u16 ch = RAM16(0x13FE);
    BSRAM(bs_stream_id) = (u8)ch; BSRAM(bs_stream_id + 1) = (u8)(ch >> 8);
    hw_w16(BSX_218E, ch);
    hw_w16(BSX_2191, 0x0001);
    hw_w16(BSX_2192, 0x0001);           /* (after four NOPs) */
    return false;
}

/* ======================================================================
 * Reset  $80:8000 (opening only; the main loop follows at $80:8057+)
 * ====================================================================== */
extern void InitPpuRegisters(void);     /* $80:8369 */

void Reset_Begin(void)
{
    /* SEI / CLC / XCE / D=$0000 / S=$1FFF / SEP #$30 */
    BsxDetect();                                    /* JSL $81FA68 */
    if (!bs_no_receiver)
        BsxOpenTimeChannel();                       /* JSL $81F80C */
    BSRAM(0xFFF9) = 0x13;
    BSRAM(0xFFFF) = 0x02;
    BSRAM(0xFFFA) = 0x02;
    BSRAM(0xFFFB) = 0x05;
    RAM8(0x1E84) = 0x1E;
    RAM8(0x1E85) = 0;
    /* DB = $00 */
    hw_w8(NMITIMEN, 0); hw_w8(HDMAEN, 0); hw_w8(MDMAEN, 0);
    for (int i = 0; i < 4; i++) hw_w8(APUIO0 + i, 0);
    hw_w8(INIDISP, 0x80);                           /* forced blank */
    InitPpuRegisters();                             /* JSL $808369 */
    RAM8(0x1E1C) = 0;
    DP(0xF2) = 0; DP(0xF4) = 0;
    BSRAM(0xFFE3) = 0; BSRAM(0xFFE4) = 0;
    /* ... continues at $80:8074: JSL $8088E0 with X=0, APU handshake loop */
}

/* ======================================================================
 * NMI  $80:85FF (first half: scroll registers)
 * ====================================================================== */
extern void NmiVramUpload(void);        /* $80:876B: queued VRAM DMA (D=$4300) */
extern void NmiOamUpload(void);         /* $80:8979 */

void Nmi_Scroll(void)
{
    hw_w16(0x4209, 0x0006);             /* VTIME = 6: IRQ line for the HUD split */
    (void)hw_r8(0x4210);                /* acknowledge NMI */
    hw_w8(INIDISP, 0x80);
    hw_w8(HDMAEN, 0);
    if (RAM8(0x081A) == 0) {            /* $081A != 0: skip uploads this frame */
        NmiVramUpload();
        NmiOamUpload();
        /* BG1: X = $92 (lo) / $8C (hi), Y = $93 */
        hw_w8(0x210D, DP(0x92)); hw_w8(0x210D, DP(0x8C));
        hw_w8(0x210E, DP(0x93));
        if (DP(0x90)) hw_w8(0x210E, 0);
        else          hw_w8(0x210E, (DP(0x8C) & 0x02) >> 1);
        hw_w8(0x210F, DP(0x98)); hw_w8(0x210F, DP(0x99));   /* BG2 */
        hw_w8(0x2110, DP(0x9A)); hw_w8(0x2110, DP(0x9B));
        hw_w8(0x2111, DP(0x9C)); hw_w8(0x2111, DP(0x9D));   /* BG3 */
        hw_w8(0x2112, DP(0x9E)); hw_w8(0x2112, DP(0x9F));
    }
    /* NMI + auto-joypad, plus H/V IRQ when $91 != 0 */
    u8 nmitimen = DP(0x91) ? 0xB1 : 0x81;
    hw_w8(NMITIMEN, nmitimen);
    RAM8(0x190E) = nmitimen;
    hw_w8(INIDISP, RAM8(0x1903));       /* shadow registers $1903-$190F */
    hw_w8(0x212C, RAM8(0x1905));        /* TM */
    hw_w8(0x212D, RAM8(0x1906));        /* TS */
    hw_w8(HDMAEN, RAM8(0x190F));
    hw_w8(0x212E, RAM8(0x190A));        /* TMW */
    hw_w8(0x212F, RAM8(0x190B));        /* TSW */
    hw_w8(0x2130, RAM8(0x190C));        /* CGWSEL */
    hw_w8(0x2131, RAM8(0x190D));        /* CGADSUB */
    /* ... continues at $80:86D2: APU sound-command handshake via $6D/$69 */
}
