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
#include "bunbun_embedded.h"

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

/* A 1 MB dump (SDL_malloc'd, caller SDL_frees), or NULL: the copy built
 * into the executable when there is one, else data/<name>. The import tool
 * verified the files; here only the size is checked. */
uint8_t *bunbun_data_load_dump(const char *name)
{
    const BunbunEmbeddedDump *e = bunbun_embedded(name);
    if (e && e->size == 0x100000) {
        uint8_t *d = SDL_malloc(e->size);
        if (d)
            SDL_memcpy(d, e->data, e->size);
        return d;
    }
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

const BunbunEmbeddedDump *bunbun_embedded(const char *name)
{
    for (const BunbunEmbeddedDump *e = kBunbunEmbedded; e->name; e++)
        if (strcmp(e->name, name) == 0)
            return e;
    return NULL;
}

/* rom.cfg beside the executable names the ROM the launcher opens on. */
static int rom_cfg_points_at(const char *want)
{
    char cfg[1100];
    const char *base = SDL_GetBasePath();
    SDL_snprintf(cfg, sizeof cfg, "%srom.cfg", base ? base : "");
    size_t n = 0;
    char *txt = SDL_LoadFile(cfg, &n);
    int ok = 0;
    if (txt) {
        while (n > 0 && (txt[n - 1] == '\n' || txt[n - 1] == '\r' || txt[n - 1] == ' '))
            txt[--n] = 0;
        ok = SDL_strcmp(txt, want) == 0;
        SDL_free(txt);
    }
    if (!ok) {
        SDL_IOStream *io = SDL_IOFromFile(cfg, "wb");
        if (io) {
            SDL_WriteIO(io, want, SDL_strlen(want));
            SDL_WriteIO(io, "\n", 1);
            SDL_CloseIO(io);
        }
    }
    return 1;
}

int bunbun_embedded_prepare(const char *rom_file)
{
    const BunbunEmbeddedDump *e = bunbun_embedded(rom_file);
    if (!e)
        return 0;
    /* The host boots from a file: keep data/<rom_file> equal to the built-in
     * dump (written on the first launch, or if it was removed or changed). */
    char path[1100];
    int have = bunbun_data_path(rom_file, path, sizeof path);
    int same = 0;
    if (have) {
        size_t n = 0;
        uint8_t *d = SDL_LoadFile(path, &n);
        same = d && n == e->size && SDL_memcmp(d, e->data, n) == 0;
        SDL_free(d);
    }
    if (!same) {
        SDL_CreateDirectory(bunbun_data_dir());
        SDL_IOStream *io = SDL_IOFromFile(path, "wb");
        if (!io || SDL_WriteIO(io, e->data, e->size) != e->size) {
            SDL_Log("[bunbun] cannot write %s: %s", path, SDL_GetError());
            if (io)
                SDL_CloseIO(io);
            return 0;
        }
        SDL_CloseIO(io);
        SDL_Log("[bunbun] wrote the built-in %s to %s", rom_file, path);
    }
    return rom_cfg_points_at(path);
}
