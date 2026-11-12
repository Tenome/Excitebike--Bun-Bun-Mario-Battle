/*
 * bunbun_data.c - the game's own data folder.
 *
 * The port is built from the ROM dumps the player supplies (code generation
 * reads them), and then runs on its own: tools/import_roms.py copies the
 * verified dumps into data/ beside each executable once, and the game reads
 * everything it needs from there. No ROM path has to be given again, and
 * moving or deleting the original files does not matter.
 *
 *   data/s1.sfc   Stadium 1 build (Gen A; the Stadium 1 and 2 courses)
 *   data/s3.sfc   Stadium 3 build (Gen B; Luigi's Gen B rider set)
 *   data/s4.sfc   Stadium 4 build (Gen B base; Yoshi, Peach)
 *
 * Which dumps an executable needs is in port/README / docs/MODDING.md. The
 * files are for the owner's private use and are never part of a release.
 */
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>
#include "bunbun_data.h"

static const struct { const char *name; const char *sha256; } kDumps[] = {
    { "s1.sfc", "a22163ddd3d07f01101e885297568d1863db0639e5b1b97fc84dff253dee2089" },
    { "s3.sfc", "072e053c3fa74fc954b0b1b6e11bdee7219f6323be97abf869f7da3c11ca9247" },
    { "s4.sfc", "d689aacff0aa4b6357ff23de592980b286464be774f6bcdeea7da07d39de00ea" },
};

const char *bunbun_data_dir(void)
{
    static char dir[1024];
    if (!dir[0]) {
        const char *env = getenv("BUNBUN_DATA");
        const char *base = SDL_GetBasePath();
        if (env && *env)
            SDL_snprintf(dir, sizeof dir, "%s", env);
        else
            SDL_snprintf(dir, sizeof dir, "%sdata", base ? base : "");
    }
    return dir;
}

int bunbun_data_path(const char *name, char *out, size_t cap)
{
    SDL_snprintf(out, cap, "%s/%s", bunbun_data_dir(), name);
    return SDL_GetPathInfo(out, NULL) ? 1 : 0;
}

/* A 1 MB dump from data/ (malloc'd, caller frees), or NULL. The import tool
 * verified it; here only the size is checked. */
uint8_t *bunbun_data_load_dump(const char *name)
{
    char p[1100];
    if (!bunbun_data_path(name, p, sizeof p))
        return NULL;
    size_t n = 0;
    uint8_t *d = SDL_LoadFile(p, &n);
    if (d && n == 0x100000)
        return d;
    if (d) {
        SDL_Log("[bunbun] %s: %u bytes, expected a 1 MB dump", p, (unsigned)n);
        SDL_free(d);
    }
    return NULL;
}

int bunbun_data_known(const char *name)
{
    for (size_t i = 0; i < sizeof kDumps / sizeof kDumps[0]; i++)
        if (strcmp(kDumps[i].name, name) == 0)
            return 1;
    return 0;
}

/* LoROM read from a 1 MB dump. */
uint8_t bunbun_lorom(const uint8_t *rom, uint32_t a)
{
    return rom[((a >> 16) & 0x1F) * 0x8000u + (a & 0x7FFFu)];
}
