/*
 * Course loading for the Gen B engine, with optional Gen A courses.
 *
 * Gen B's course loader ($81:A0F4) clears the track buffer $7F:4000-$4FFF,
 * picks a course by X = (($81:800C * 4) + $7C) * 2 (or $10 for the special
 * course when $0CD7 != 0), and expands the course bytes ($81:8C62 starts,
 * $81:8C76 lengths, $81:8C8A data; run flag bit $80) into the buffer, then
 * runs a race-only random pass at $81:A18B and returns.
 *
 * HleCourseLoad replaces the lookup and expansion ($81:A130-$81:A18A) and
 * then continues at $81:A18B, so the random pass and the exit are the
 * original code. With no override it reproduces the original exactly.
 *
 * Override (merge experiment): BUNBUN_GENA_ROM=<path to the Stadium 1 dump>
 * and BUNBUN_COURSES=1|2 load that Gen A ROM at startup, verify its CRC32,
 * and use its Stadium 1 or 2 courses for courses 1-4. Gen A courses use run
 * flag $40 (ground $40); they are converted on load by moving codes
 * $40-$7F to $80-$BF. Nothing ROM-derived is stored in this repository.
 */
#include <stdio.h>
#include <stdlib.h>
#include "bunbun_data.h"
#include "bunbun_settings.h"
#include <SDL3/SDL.h>
#include <string.h>

#include "cpu_state.h"
#include "common_cpu_infra.h"

#define GENA_CRC32 0x6e6d087cu      /* Stadium 1 (6-30) dump, see rom_identity.txt */

static uint8_t *g_gena;               /* converted Gen A course bytes, 8 courses */
static uint32_t g_gena_off[8], g_gena_len[8];
static int g_gena_set = -1;           /* 0 = Stadium 1, 1 = Stadium 2, -1 = off */

static uint32_t crc32_buf(const uint8_t *p, size_t n)
{
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; i++) {
        c ^= p[i];
        for (int k = 0; k < 8; k++)
            c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return ~c;
}

static uint8_t lorom(const uint8_t *rom, uint32_t a)
{
    return rom[((a >> 16) & 0x1F) * 0x8000u + (a & 0x7FFFu)];
}

void BunbunCoursesInit(void)
{
    const char *path = getenv("BUNBUN_GENA_ROM");
    char setbuf[8];
    SDL_snprintf(setbuf, sizeof setbuf, "%d", bunbun_option("BUNBUN_COURSES", "game.courses", 0));
    const char *set = setbuf;              /* launcher: Game > Courses ("own" = 0) */
    if (!set || (atoi(set) != 1 && atoi(set) != 2))
        return;
    uint8_t *rom = NULL;
    size_t n = 0;
    if (path) {                             /* an explicit dump wins */
        rom = SDL_LoadFile(path, &n);
        if (!rom) { fprintf(stderr, "[bunbun] cannot open BUNBUN_GENA_ROM '%s'\n", path); return; }
    } else {                                /* built in, else data/s1.sfc */
        rom = bunbun_data_load_dump("s1.sfc");
        n = rom ? 0x100000 : 0;
        if (!rom)
            return;
    }
    if (n != 0x100000 || crc32_buf(rom, n) != GENA_CRC32) {
        fprintf(stderr, "[bunbun] the Stadium 1 dump is not the expected one\n");
        SDL_free(rom);
        return;
    }
    /* Gen A tables: starts $81:8C59, lengths $81:8C6D, data $81:8C81. */
    uint32_t total = 0;
    for (int i = 0; i < 8; i++) {
        g_gena_off[i] = total;
        g_gena_len[i] = lorom(rom, 0x818C6Du + 2 * i) | lorom(rom, 0x818C6Eu + 2 * i) << 8;
        total += g_gena_len[i];
    }
    g_gena = (uint8_t *)malloc(total);
    for (int i = 0; i < 8; i++) {
        uint32_t s = lorom(rom, 0x818C59u + 2 * i) | lorom(rom, 0x818C5Au + 2 * i) << 8;
        uint8_t *dst = g_gena + g_gena_off[i];
        for (uint32_t k = 0; k < g_gena_len[i]; k++)
            dst[k] = lorom(rom, 0x818C81u + s + k);
        /* walk the stream so only code bytes are remapped, never run counts */
        for (uint32_t k = 0; k < g_gena_len[i]; k++) {
            uint8_t b = dst[k];
            if (b & 0x40) {               /* run: code, then count */
                dst[k] = (uint8_t)((b & 0x3F) | 0x80);
                k++;
            }
        }
    }
    SDL_free(rom);
    g_gena_set = atoi(set) - 1;
    fprintf(stderr, "[bunbun] using Stadium %d courses from the Gen A ROM\n", g_gena_set + 1);
}

int BunbunCustomCourse(int index, uint8_t *buf, int cap);   /* src/mods_folder.c */

/* $81:A130 (M0X0, X = course index * 2, DB = $80, D = $0000). */
RecompReturn HleCourseLoad(CpuState *cpu)
{
    unsigned idx = (cpu->X & 0xFFFF) >> 1;
    const uint8_t *src;
    uint32_t len;
    if (g_gena && g_gena_set >= 0 && idx < 8) {
        unsigned c = (unsigned)g_gena_set * 4 + (idx & 3);
        src = g_gena + g_gena_off[c];
        len = g_gena_len[c];
    } else {
        static uint8_t buf[0x4000];
        uint32_t s = cpu_read16(cpu, 0x81, (uint16)(0x8C62 + 2 * idx));
        len = cpu_read16(cpu, 0x81, (uint16)(0x8C76 + 2 * idx));
        if (len > sizeof buf) len = sizeof buf;
        for (uint32_t k = 0; k < len; k++)
            buf[k] = cpu_read8(cpu, 0x81, (uint16)(0x8C8A + s + k));
        src = buf;
    }
    /* Expand exactly as $81:A148-$81:A189 does (run flag $80). */
    uint32_t x = 0, y = 0;
    int32_t n = (int32_t)len;
    uint8_t last_run = 0;
    while (n > 0) {
        uint8_t b = src[x];
        cpu_write8(cpu, 0x7F, (uint16)(0x4000 + y), b);
        if (b == 0x09)
            for (int k = 1; k <= 5; k++)
                cpu_write8(cpu, 0x7F, (uint16)(0x4000 + y + k), 0x09);
        if (b & 0x80) {
            last_run = b;
            n--;
            x++;
            unsigned cnt = src[x];
            for (unsigned k = 0; k < cnt; k++)
                cpu_write8(cpu, 0x7F, (uint16)(0x4000 + y + k), b);
            y += cnt - 1;
        }
        x++;
        y++;
        n--;
    }
    /* A custom course (launcher: Mods > Courses, or mods/<name>/courses/
     * courseN.bin; N = 0-3 Stadium 3, 4-7 Stadium 4, 8 the special course)
     * replaces the expanded buffer. A file made for the Gen A engine (ground
     * codes $40-$7F and none of $80-$BF) is moved to Gen B's codes. */
    {
        extern uint8_t g_ram[];
        uint8_t *track = &g_ram[0x14000];                       /* $7F:4000 */
        int n_a = 0, n_b = 0;
        if (BunbunCustomCourse(idx < 8 ? (int)idx : 8, track, 0x1000)) {
            for (int k = 0; k < 0x1000; k++) {
                n_a += track[k] >= 0x40 && track[k] < 0x80;
                n_b += track[k] >= 0x80 && track[k] < 0xC0;
            }
            if (n_a && !n_b)
                for (int k = 0; k < 0x1000; k++)
                    if (track[k] >= 0x40 && track[k] < 0x80)
                        track[k] = (uint8_t)(track[k] + 0x40);
        }
    }
    /* Leave the state the original leaves at $81:A18B: M=1 X=0, DB=$7F with
     * the caller's DB pushed (PHB at $81:A143), $00 = 0, $02 = last run byte. */
    cpu_write8(cpu, 0x00, (uint16)(cpu->D + 0x00), 0);
    cpu_write8(cpu, 0x00, (uint16)(cpu->D + 0x01), 0);
    if (last_run)
        cpu_write8(cpu, 0x00, (uint16)(cpu->D + 0x02), last_run);
    cpu_write8(cpu, 0x00, cpu->S, cpu->DB);
    cpu->S = (uint16)(cpu->S - 1);
    cpu->DB = 0x7F;
    cpu->X = (uint16)(x & 0xFFFF);
    cpu->Y = (uint16)(y & 0xFFFF);
    cpu->P = (uint8)((cpu->P | CPU_P_M) & ~CPU_P_X);
    cpu_p_to_mirrors(cpu);
    cpu->cycles += 40 + 30 * len;
    cpu->master_cycles += (uint64_t)(40 + 30 * len) * 8;
    return cpu_dispatch_pc_from(cpu, 0x81A18Bu, cpu->S, 0x81A130u);
}
