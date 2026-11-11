/*
 * runtime816.h - the machine model used by mechanically lifted 65816 code
 * (tools/lift816.py output in c/auto/<ver>/).
 *
 * The lifted C keeps the original's registers, flags and stack, so it can be
 * diffed instruction-for-instruction against the original (tools/difftest816.py)
 * before it is rewritten by hand into readable code.
 *
 * Memory: wram[] (128 KB), rom[] (LoROM), and hardware registers through
 * hw_read()/hw_write(), which the host provides (the difftest harness logs
 * writes; a port routes them to its PPU/APU).
 */
#ifndef RUNTIME816_H
#define RUNTIME816_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#ifdef _WIN32
#define R816_EXPORT __declspec(dllexport)
#else
#define R816_EXPORT
#endif

/* ---- machine state (defined in runtime816.c) ---------------------------- */
extern u16 A, X, Y, S, D;
extern u8 DB, PB;
extern u8 fn, fz, fc, fv, fd, fi;     /* N Z C V D I flags (0/1) */
extern u8 fm, fx;                     /* width flags: 1 = 8-bit (tracked for PHP/PLP) */
extern u8 wram[0x20000];
extern const u8 *rom;
extern u32 rom_size;
extern int r816_unresolved;           /* set when lifted code reaches an unknown target */

u8 hw_read(u16 reg);
void hw_write(u16 reg, u8 v);
void open_write(u32 addr, u8 v);

/* ---- memory ------------------------------------------------------------- */
static inline u8 rd8(u32 a)
{
    a &= 0xFFFFFF;
    u8 b = (u8)(a >> 16);
    u16 o = (u16)a;
    if (b == 0x7E || b == 0x7F) return wram[((u32)(b - 0x7E) << 16) | o];
    u8 lb = b & 0x7F;
    if (lb < 0x40) {
        if (o < 0x2000) return wram[o];
        if (o < 0x6000) return hw_read(o);
        if (o < 0x8000) return 0;
        return rom[(((u32)lb << 15) | (o & 0x7FFF)) % rom_size];
    }
    if (o >= 0x8000) return rom[((((u32)lb & 0x3F) << 15) | (o & 0x7FFF)) % rom_size];
    return 0;
}

static inline void wr8(u32 a, u8 v)
{
    a &= 0xFFFFFF;
    u8 b = (u8)(a >> 16);
    u16 o = (u16)a;
    if (b == 0x7E || b == 0x7F) { wram[((u32)(b - 0x7E) << 16) | o] = v; return; }
    u8 lb = b & 0x7F;
    if (lb < 0x40 && o < 0x2000) wram[o] = v;
    else if (lb < 0x40 && o < 0x6000) hw_write(o, v);
    else open_write(a, v);
}

static inline u16 rd16(u32 a) { return (u16)(rd8(a) | rd8(a + 1) << 8); }
static inline void wr16(u32 a, u16 v) { wr8(a, (u8)v); wr8(a + 1, (u8)(v >> 8)); }

/* ---- effective addresses ------------------------------------------------- */
#define DP(o)      ((u32)(u16)(D + (o)))
#define DPX(o)     ((u32)(u16)(D + (o) + X))
#define DPY(o)     ((u32)(u16)(D + (o) + Y))
#define IDP(o)     (((u32)DB << 16) + rd16(DP(o)))
#define IDPX(o)    (((u32)DB << 16) + rd16(DPX(o)))
#define IDPY(o)    ((((u32)DB << 16) + rd16(DP(o)) + Y) & 0xFFFFFF)
#define ILDP(o)    ((u32)rd16(DP(o)) | (u32)rd8((u16)(D + (o) + 2)) << 16)
#define ILDPY(o)   ((ILDP(o) + Y) & 0xFFFFFF)
#define SR(o)      ((u32)(u16)(S + (o)))
#define ISRY(o)    ((((u32)DB << 16) + rd16(SR(o)) + Y) & 0xFFFFFF)
#define ABS(o)     (((u32)DB << 16) | (o))
#define ABSX(o)    ((((u32)DB << 16) + (o) + X) & 0xFFFFFF)
#define ABSY(o)    ((((u32)DB << 16) + (o) + Y) & 0xFFFFFF)
#define LONG(a)    ((u32)(a))
#define LONGX(a)   (((u32)(a) + X) & 0xFFFFFF)

/* ---- flags ---------------------------------------------------------------- */
static inline void nz8(u8 v) { fn = v >> 7; fz = v == 0; }
static inline void nz16(u16 v) { fn = v >> 15; fz = v == 0; }
static inline u8 getP(void)
{
    return (u8)(fc | fz << 1 | fi << 2 | fd << 3 | fx << 4 | fm << 5 | fv << 6 | fn << 7);
}
static inline void setP(u8 p)
{
    fc = p & 1; fz = (p >> 1) & 1; fi = (p >> 2) & 1; fd = (p >> 3) & 1;
    fx = (p >> 4) & 1; fm = (p >> 5) & 1; fv = (p >> 6) & 1; fn = p >> 7;
    if (fx) { X &= 0xFF; Y &= 0xFF; }
}
static inline void rep(u8 v) { setP(getP() & (u8)~v); }
static inline void sep(u8 v) { setP(getP() | v); }

/* ---- stack ----------------------------------------------------------------- */
static inline void push8(u8 v) { wr8(S, v); S--; }
static inline u8 pull8(void) { S++; return rd8(S); }
static inline void push16(u16 v) { push8((u8)(v >> 8)); push8((u8)v); }
static inline u16 pull16(void) { u8 lo = pull8(); return (u16)(lo | pull8() << 8); }

/* ---- accumulator / index loads ------------------------------------------- */
#define SET_A8(v)  (A = (u16)((A & 0xFF00) | (u8)(v)))
static inline void lda8(u8 v) { SET_A8(v); nz8(v); }
static inline void lda16(u16 v) { A = v; nz16(v); }
static inline void ldx8(u8 v) { X = v; nz8(v); }
static inline void ldx16(u16 v) { X = v; nz16(v); }
static inline void ldy8(u8 v) { Y = v; nz8(v); }
static inline void ldy16(u16 v) { Y = v; nz16(v); }

/* ---- logic ------------------------------------------------------------------ */
static inline void ora8(u8 v) { lda8((u8)(A | v)); }
static inline void ora16(u16 v) { lda16((u16)(A | v)); }
static inline void and8(u8 v) { lda8((u8)(A & v)); }
static inline void and16(u16 v) { lda16((u16)(A & v)); }
static inline void eor8(u8 v) { lda8((u8)(A ^ v)); }
static inline void eor16(u16 v) { lda16((u16)(A ^ v)); }
static inline void bit8(u8 v, int imm) { fz = ((u8)A & v) == 0; if (!imm) { fn = v >> 7; fv = (v >> 6) & 1; } }
static inline void bit16(u16 v, int imm) { fz = (A & v) == 0; if (!imm) { fn = v >> 15; fv = (v >> 14) & 1; } }

/* ---- arithmetic -------------------------------------------------------------- */
void adc8(u8 v);
void adc16(u16 v);
void sbc8(u8 v);
void sbc16(u16 v);
static inline void cmp8(u8 r, u8 v) { fc = r >= v; nz8((u8)(r - v)); }
static inline void cmp16(u16 r, u16 v) { fc = r >= v; nz16((u16)(r - v)); }

/* ---- shifts and inc/dec (value in, result out, flags set) ------------------- */
static inline u8 asl8(u8 v) { fc = v >> 7; v <<= 1; nz8(v); return v; }
static inline u16 asl16(u16 v) { fc = v >> 15; v <<= 1; nz16(v); return v; }
static inline u8 lsr8(u8 v) { fc = v & 1; v >>= 1; nz8(v); return v; }
static inline u16 lsr16(u16 v) { fc = v & 1; v >>= 1; nz16(v); return v; }
static inline u8 rol8(u8 v) { u8 c = fc; fc = v >> 7; v = (u8)(v << 1 | c); nz8(v); return v; }
static inline u16 rol16(u16 v) { u16 c = fc; fc = v >> 15; v = (u16)(v << 1 | c); nz16(v); return v; }
static inline u8 ror8(u8 v) { u8 c = fc; fc = v & 1; v = (u8)(v >> 1 | c << 7); nz8(v); return v; }
static inline u16 ror16(u16 v) { u16 c = fc; fc = v & 1; v = (u16)(v >> 1 | c << 15); nz16(v); return v; }
static inline u8 inc8(u8 v) { v++; nz8(v); return v; }
static inline u16 inc16(u16 v) { v++; nz16(v); return v; }
static inline u8 dec8(u8 v) { v--; nz8(v); return v; }
static inline u16 dec16(u16 v) { v--; nz16(v); return v; }
static inline u8 tsb8(u8 v) { fz = ((u8)A & v) == 0; return (u8)(v | A); }
static inline u16 tsb16(u16 v) { fz = (A & v) == 0; return (u16)(v | A); }
static inline u8 trb8(u8 v) { fz = ((u8)A & v) == 0; return (u8)(v & ~A); }
static inline u16 trb16(u16 v) { fz = (A & v) == 0; return (u16)(v & ~A); }

/* ---- misc ------------------------------------------------------------------ */
void mvn(u8 dst, u8 src);
void mvp(u8 dst, u8 src);
void r816_unknown_target(u32 site, u32 value);

#endif
