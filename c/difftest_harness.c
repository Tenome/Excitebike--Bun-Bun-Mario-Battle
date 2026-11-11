/* difftest_harness.c - DLL entry points for tools/difftest816.py. */
#include <setjmp.h>
#include <string.h>
#include "runtime816.h"

typedef struct { u32 pc; u8 m, x; void (*fn)(void); const char *name; } LiftedEntry;
extern const LiftedEntry lifted_table[];
extern const unsigned lifted_count;

jmp_buf r816_bail;
long r816_ticks, r816_limit = 2000000;

#define HWLOG_MAX 65536
static u32 hwlog_addr[HWLOG_MAX];
static u8 hwlog_val[HWLOG_MAX];
static int hwlog_n;
static u8 hw_table[0x4000];          /* read values for $2000-$5FFF */

u8 hw_read(u16 reg) { return (reg >= 0x2000 && reg < 0x6000) ? hw_table[reg - 0x2000] : 0; }
void hw_write(u16 reg, u8 v) { if (hwlog_n < HWLOG_MAX) { hwlog_addr[hwlog_n] = reg; hwlog_val[hwlog_n++] = v; } }
void open_write(u32 a, u8 v) { if (hwlog_n < HWLOG_MAX) { hwlog_addr[hwlog_n] = 0x80000000u | a; hwlog_val[hwlog_n++] = v; } }

R816_EXPORT void h_set_rom(const u8 *p, u32 n) { rom = p; rom_size = n; }
R816_EXPORT u8 *h_wram(void) { return wram; }
R816_EXPORT u8 *h_hw_table(void) { return hw_table; }
R816_EXPORT unsigned h_count(void) { return lifted_count; }
R816_EXPORT u32 h_entry(unsigned i, u8 *m, u8 *x) { *m = lifted_table[i].m; *x = lifted_table[i].x; return lifted_table[i].pc; }
R816_EXPORT const char *h_name(unsigned i) { return lifted_table[i].name; }
R816_EXPORT int h_hwlog(u32 **addr, u8 **val) { *addr = hwlog_addr; *val = hwlog_val; return hwlog_n; }

R816_EXPORT void h_set_regs(u16 a, u16 x, u16 y, u16 s, u16 d, u8 db, u8 p)
{
    A = a; X = x; Y = y; S = s; D = d; DB = db; setP(p);
}

R816_EXPORT void h_get_regs(u16 *out)
{
    out[0] = A; out[1] = X; out[2] = Y; out[3] = S; out[4] = D; out[5] = DB; out[6] = getP();
}

/* Call entry i as JSR (long = 0) or JSL (long = 1) with the same sentinel
 * return frame the interpreter pushes. Returns 0 ok, 1 loop guard, 2 unknown target. */
R816_EXPORT int h_run(unsigned i, int long_call, long limit)
{
    hwlog_n = 0;
    r816_unresolved = 0;
    r816_ticks = 0;
    r816_limit = limit;
    PB = (u8)(lifted_table[i].pc >> 16);
    if (long_call) push8(0xFF);
    push16(0xFFFE);
    if (setjmp(r816_bail)) return 1;
    lifted_table[i].fn();
    return r816_unresolved ? 2 : 0;
}
