/* Hand-written bodies for code the recompiler could not translate, and the
 * port's deliberate replacements (hle_func directives in recomp/bank*.cfg).
 *
 * Fill this in with real behaviour, never with a value invented to get past
 * a gap. Each replacement documents where it departs from the original.
 */
#include "cpu_state.h"
#include "common_cpu_infra.h"

/*
 * $80:9DE0  EndScreenStart  (RTS routine, entered M1X1 from the $80:99CD table)
 *
 * Original (Stadium 1, $81:800C = 0):
 *     LDA $21 / AND #$10 / BEQ done          ; P1 Start newly pressed?
 *     LDA $81800C / BNE to_mode12
 *     SEI / STZ NMITIMEN / BS-X MMC writes / LDA #$FE / STA APUIO0
 *     JML $10:5C3C                           ; into the BS-X BIOS
 *   to_mode12:
 *     LDA #$12 / STA $76 / STZ $75 / STZ $77 / DEC $1923
 *   done:
 *     RTS
 *
 * There is no BIOS in the port, so the BIOS jump would run into open bus.
 * This replacement always takes the to_mode12 path, the one Stadium 2 and 4
 * ship with ($81:800C = 1), and also rewinds the broadcast clock to minute 0
 * (see below). Everything else matches the original.
 */
RecompReturn HleEndScreenStart(CpuState *cpu)
{
    uint8 pressed = cpu_read8(cpu, 0x00, (uint16)(cpu->D + 0x21));
    uint8 a = (uint8)(pressed & 0x10);
    uint32 cyc = 3 + 2 + 2;                 /* LDA dp, AND #, BEQ */
    if (a == 0) {
        cpu_write_a8(cpu, a);
        cpu->_flag_Z = 1;
        cpu->_flag_N = 0;
        cyc += 1 + 6;                       /* branch taken, RTS */
    } else {
        /* Port addition: restart the broadcast hour (minute 0, frame 0) so
         * mode $12 -> $0D replays the intro instead of idling at minute 60,
         * past the end of the schedule. */
        cpu_write8(cpu, 0x7F, 0xFFFF, 0x00);
        cpu_write8(cpu, 0x7F, 0xFFFA, 0x00);
        cpu_write8(cpu, 0x7F, 0xFFFB, 0x00);
        cpu_write8(cpu, 0x00, (uint16)(cpu->D + 0x76), 0x12);
        cpu_write8(cpu, 0x00, (uint16)(cpu->D + 0x75), 0x00);
        cpu_write8(cpu, 0x00, (uint16)(cpu->D + 0x77), 0x00);
        uint8 v = (uint8)(cpu_read8(cpu, cpu->DB, 0x1923) - 1);
        cpu_write8(cpu, cpu->DB, 0x1923, v);
        cpu_write_a8(cpu, 0x12);
        cpu->_flag_Z = (v == 0);
        cpu->_flag_N = (v & 0x80) ? 1 : 0;
        /* LDA long, BNE, LDA #, STA dp, STZ dp x2, DEC abs, RTS */
        cyc += 5 + 2 + 2 + 3 + 3 + 3 + 6 + 6;
    }
    cpu_mirrors_to_p(cpu);
    cpu->S = (uint16)(cpu->S + 2);          /* the RTS frame */
    cpu->cycles += cyc;
    cpu->master_cycles += (uint64_t)cyc * 8;
    return RECOMP_RETURN_NORMAL;
}
