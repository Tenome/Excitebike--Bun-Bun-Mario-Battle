/*
 * Stadium 1 video initialisation, hand-written.
 */
#include "hand/hand.h"

void DmaFillZero_m0x1(void);
void DmaKick_m0x1(void);

/* $80:84A8 ForceBlank (M=1, X=1, RTL): screen off, HDMA off. */
void ForceBlank_m1x1(void)
{
    lda8(0x80);
    wr8(ABS(0x2100), 0x80);                            /* INIDISP: forced blank */
    wr8(ABS(0x0819), 0x80);
    wr8(ABS(0x420C), 0);                               /* HDMAEN */
    wr8(ABS(0x190F), 0);                               /* its shadow */
    retl();
}

/* Fill `bytes` VRAM bytes of one half (low or high byte of each word) from a
 * fixed source byte at $00:0000, through channel 0 (D = $4300). */
static void vram_fill_half(u16 vram, int wide_stz, u8 bbad, u16 bytes)
{
    if (wide_stz)
        wr16(ABS(0x2115), 0);                          /* STZ VMAIN in 16-bit mode: also $2116 */
    else
        wr8(ABS(0x2115), 0);                           /* VMAIN = 0: increment after low byte */
    wr16(ABS(0x2116), vram);
    wr16(DP(0x00), (u16)(bbad << 8 | 0x08));           /* DMAP0 = $08 (fixed source) */
    wr16(DP(0x02), 0);                                 /* source $00:0000 */
    wr16(DP(0x04), 0);
    wr16(DP(0x05), bytes);
    wr8(ABS(0x420B), 0x01);
}

/*
 * $80:84B7 ClearTilemaps (M=1, X=1, RTL). Blanks two tilemap areas: VRAM
 * $0000-$0FFF and $1800-$1BFF get tile $FC with attribute 0. A fixed-source
 * DMA writes the byte at $0000 into every low byte ($FC), then every high
 * byte ($00).
 */
void ClearTilemaps_m1x1(void)
{
    push16(D);
    lda8(0xFC);
    wr8(DP(0x00), 0xFC);                               /* source byte for the low halves */
    rep(0x20);
    A = 0x4300; D = 0x4300;
    Y = 1;

    vram_fill_half(0x0000, 0, 0x18, 0x1000);        /* low bytes: $FC */
    /* the original then sets DAS0 again and re-addresses VRAM */
    wr16(DP(0x05), 0x1000);
    wr16(ABS(0x2116), 0x0000);
    wr16(ABS(0x0000), 0);                              /* source byte: $00 */
    X = 0x80; wr8(ABS(0x2115), 0x80);
    X = 0x19; wr8(DP(0x01), 0x19);                     /* BBAD0 = VMDATAH */
    wr16(DP(0x05), 0x1000);
    wr8(ABS(0x420B), 0x01);                            /* high bytes: $00 */

    X = 0xFC; wr8(ABS(0x0000), 0xFC);
    vram_fill_half(0x1800, 1, 0x18, 0x0800);
    wr16(DP(0x05), 0x0800);
    wr16(ABS(0x2116), 0x1800);
    wr16(ABS(0x0000), 0);
    X = 0x80; wr8(ABS(0x2115), 0x80);
    X = 0x19; wr8(DP(0x01), 0x19);
    wr16(DP(0x05), 0x0800);
    wr8(ABS(0x420B), 0x01);
    A = 0x0800;
    sep(0x20);
    D = pull16();
    nz16(D);
    retl();
}

/*
 * $80:8369 InitPpuRegisters (M=1, X=1, RTL), once from Reset.
 * Screen off; clear $7E:2000-$7F:FFDF and $00:0000-$1E7F with DMA; reset the
 * VRAM queue; set the PPU registers (written through D = $2100) and their
 * shadows; set up HDMA channels 6 and 7 (BG1/BG2 horizontal scroll tables at
 * $80:849E and $80:8482); upload the font ($88:8000, $1000 bytes) to VRAM $1000.
 */
void InitPpuRegisters_m1x1(void)
{
    push8(0x80); push16(0x836C); ForceBlank_m1x1();
    rep(0x20);
    wr16(DP(0x00), 0xE000); A = 0x2000; Y = 0x7E;
    push8(0x80); push16(0x837C); DmaFillZero_m0x1();  /* $7E:2000, $E000 bytes */
    wr16(DP(0x00), 0xFFE0); A = 0x0000; Y = 0x7F;
    push8(0x80); push16(0x838A); DmaFillZero_m0x1();  /* $7F:0000, $FFE0 bytes */
    wr16(DP(0x00), 0x1E80); A = 0x0000; Y = 0x00;
    push8(0x80); push16(0x8398); DmaFillZero_m0x1();  /* $00:0000, $1E80 bytes */
    A = 0x0002;
    wr16(LONG(0x7F0000), 0x0002);                      /* VRAM queue empty */

    push16(D);
    A = 0x2100; D = 0x2100;                            /* DP = PPU registers */
    sep(0x20);
    static const struct { u8 reg; u8 val; u16 shadow; } ppu[] = {
        {0x01, 0x01, 0},      {0x05, 0x09, 0x1904}, {0x06, 0x00, 0},
        {0x07, 0x01, 0x1900}, {0x08, 0x09, 0},      {0x0A, 0x00, 0},
        {0x09, 0x19, 0},      {0x0B, 0x44, 0},      {0x0C, 0x01, 0},
        {0x2C, 0x16, 0x1905}, {0x2D, 0x00, 0},      {0x2E, 0x00, 0},
        {0x2F, 0x00, 0},      {0x23, 0x00, 0},      {0x24, 0x00, 0},
        {0x25, 0x00, 0},      {0x30, 0x00, 0},      {0x31, 0x00, 0},
        {0x32, 0xE0, 0},      {0x33, 0x00, 0},
    };
    /* OBSEL, BGMODE, MOSAIC, BG1SC..BG4SC, BG12NBA, BG34NBA, TM/TS/TMW/TSW,
     * window settings, CGWSEL, CGADSUB, COLDATA, SETINI */
    for (unsigned i = 0; i < sizeof ppu / sizeof ppu[0]; i++) {
        if (ppu[i].val) SET_A8(ppu[i].val);
        wr8(DP(ppu[i].reg), ppu[i].val);
        if (ppu[i].shadow) wr8(ABS(ppu[i].shadow), ppu[i].val);
    }
    static const u16 zero_shadows[] = {0x1907, 0x1908, 0x1909, 0x190A, 0x190B, 0x190C, 0x190D,
                                       0x190F, 0x1910, 0x1911};
    for (unsigned i = 0; i < sizeof zero_shadows / sizeof zero_shadows[0]; i++)
        wr8(ABS(zero_shadows[i]), 0);
    wr8(ABS(0x1912), 0x20);
    wr8(ABS(0x1913), 0x40);
    wr8(ABS(0x1914), 0x80);
    wr8(ABS(0x420C), 0);                               /* HDMAEN off */
    wr8(ABS(0x190F), 0);
    static const u8 hdma[2][6] = {                     /* ch7, ch6: DMAP BBAD A1TL A1TH A1B DASB */
        {0x42, 0x0F, 0x82, 0x84, 0x80, 0x00},          /* indirect, BG2HOFS, table $80:8482 */
        {0x42, 0x0D, 0x9E, 0x84, 0x80, 0x00},          /* indirect, BG1HOFS, table $80:849E */
    };
    for (int ch = 0; ch < 2; ch++) {
        u16 base = ch == 0 ? 0x4370 : 0x4360;
        static const u8 off[6] = {0, 1, 2, 3, 4, 7};
        for (int k = 0; k < 6; k++) {
            SET_A8(hdma[ch][k]);
            wr8(ABS(base + off[k]), hdma[ch][k]);
        }
    }
    rep(0x20);
    A = 0x4300; D = 0x4300;
    Y = 0x80; wr8(ABS(0x2115), 0x80);
    A = 0x1801; wr16(DP(0x00), 0x1801);                /* channel 0 -> VMDATA */
    wr16(ABS(0x2116), 0x1000);
    wr16(DP(0x05), 0x1000);
    A = 0x8000; X = 0x88; Y = 0x01;
    push16(0x847D); DmaKick_m0x1();                    /* font: $88:8000 -> VRAM $1000 */
    D = pull16();
    nz16(D);
    sep(0x20);
    retl();
}
