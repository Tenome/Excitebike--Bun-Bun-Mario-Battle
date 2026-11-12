/*
 * mods_folder.c - load replacement graphics, palettes and data from a plain
 * folder, no packaging needed.
 *
 * Layout (docs/MODDING.md):
 *   mods/                      beside the executable, or BUNBUN_MODS=<dir>
 *     <any name>/              one mod per folder, applied in name order;
 *                              a file named "disabled" inside skips it
 *       *.ips                  IPS patches against the ROM image
 *       rom/BBAAAA.bin         raw bytes placed at SNES address $BB:AAAA
 *       gfx/BBAAAA_Nbpp*.png   8x8 tiles (N = 2, 4 or 8), left to right, top
 *                              to bottom, encoded to SNES planar tiles at
 *                              $BB:AAAA; "_n<count>" in the name limits it
 *                              to that many tiles. Indexed PNGs use their pixel
 *                              indices; RGB(A) PNGs are matched to the
 *                              nearest colour of BBAAAA*.pal.txt beside them
 *                              (written by tools/assets.py export), else by
 *                              brightness. Fully transparent pixels = 0.
 *       pal/BBAAAA*.txt        palette: one RRGGBB hex colour per line,
 *                              stored as SNES BGR555 words at $BB:AAAA
 *       sound/BBAAAA*.wav      replaces the BRR sample starting at $BB:AAAA
 *                              (names from tools/sound.py export): any WAV
 *                              is mixed to mono, resampled to the original
 *                              length and BRR-encoded; the loop/end flags of
 *                              each block are kept. sound/BBAAAA*.brr: raw
 *                              BRR of exactly the original size.
 *
 * Everything is applied to the ROM image in memory once, after the ROM has
 * been verified; the file on disk is never changed. The graphics and
 * palettes are uploaded by the game's own DMA from ROM, so replacing the
 * bytes there replaces them everywhere they are used. tools/assets.py export
 * writes every graphics block in this form, ready to edit.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snes/cart.h"
#include "snes/snes.h"
#include "common_cpu_infra.h"

static uint8_t *s_rom;
static size_t s_rom_size;
static int s_changed;
static size_t s_gfx_limit;      /* bytes a sheet may write (0: no limit) */

/* LoROM: $BB:AAAA (AAAA >= $8000) -> file offset. */
static long rom_offset(uint32_t snes)
{
    if ((snes & 0xFFFF) < 0x8000)
        return -1;
    long off = (long)(((snes >> 16) & 0x7F) << 15 | (snes & 0x7FFF));
    return off < (long)s_rom_size ? off : -1;
}

static void put_bytes(uint32_t snes, const uint8_t *data, size_t n, const char *what)
{
    long off = rom_offset(snes);
    if (off < 0 || (size_t)off + n > s_rom_size) {
        SDL_Log("mods: %s: address $%06X out of range", what, (unsigned)snes);
        return;
    }
    memcpy(s_rom + off, data, n);
    s_changed++;
    SDL_Log("mods: %s -> $%06X (%u bytes)", what, (unsigned)snes, (unsigned)n);
}

static uint8_t *read_file(const char *path, size_t *n)
{
    size_t len = 0;
    void *p = SDL_LoadFile(path, &len);
    *n = len;
    return p;
}

/* The leading 6 hex digits of a file name, or -1. */
static long name_address(const char *name)
{
    char hex[7];
    for (int i = 0; i < 6; i++) {
        char c = name[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
            return -1;
        hex[i] = c;
    }
    hex[6] = 0;
    return strtol(hex, NULL, 16);
}

static int ends_with(const char *s, const char *suffix)
{
    size_t a = strlen(s), b = strlen(suffix);
    return a >= b && SDL_strcasecmp(s + a - b, suffix) == 0;
}

/* ---- IPS ----------------------------------------------------------------- */
static void apply_ips(const char *path, const char *name)
{
    size_t n;
    uint8_t *p = read_file(path, &n);
    if (!p)
        return;
    if (n < 8 || memcmp(p, "PATCH", 5) != 0) {
        SDL_Log("mods: %s is not an IPS patch", name);
        SDL_free(p);
        return;
    }
    size_t i = 5;
    int records = 0;
    while (i + 3 <= n && memcmp(p + i, "EOF", 3) != 0) {
        if (i + 5 > n) break;
        uint32_t off = (uint32_t)p[i] << 16 | p[i + 1] << 8 | p[i + 2];
        uint32_t len = (uint32_t)p[i + 3] << 8 | p[i + 4];
        i += 5;
        if (len) {
            if (i + len > n) break;
            if (off + len <= s_rom_size) memcpy(s_rom + off, p + i, len);
            i += len;
        } else {                                    /* RLE record */
            if (i + 3 > n) break;
            uint32_t rl = (uint32_t)p[i] << 8 | p[i + 1];
            if (off + rl <= s_rom_size) memset(s_rom + off, p[i + 2], rl);
            i += 3;
        }
        records++;
    }
    s_changed++;
    SDL_Log("mods: %s applied (%d records)", name, records);
    SDL_free(p);
}

/* ---- palettes ------------------------------------------------------------ */
typedef struct { int n; uint8_t rgb[256][3]; } Pal;

static int load_pal_txt(const char *path, Pal *pal)
{
    size_t n;
    char *p = (char *)read_file(path, &n);
    pal->n = 0;
    if (!p)
        return 0;
    char *s = p, *end = p + n;
    while (s < end && pal->n < 256) {
        while (s < end && (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n' || *s == '#'))
            if (*s++ == '#') while (s < end && *s != '\n') s++;   /* comment */
        if (s >= end) break;
        char *q;
        unsigned long v = strtoul(s, &q, 16);
        if (q == s) { while (s < end && *s != '\n') s++; continue; }
        pal->rgb[pal->n][0] = (uint8_t)(v >> 16);
        pal->rgb[pal->n][1] = (uint8_t)(v >> 8);
        pal->rgb[pal->n][2] = (uint8_t)v;
        pal->n++;
        s = q;
        while (s < end && *s != '\n') s++;
    }
    SDL_free(p);
    return pal->n;
}

static void apply_pal(const char *path, const char *name, uint32_t addr)
{
    Pal pal;
    if (!load_pal_txt(path, &pal))
        return;
    uint8_t words[512];
    for (int i = 0; i < pal.n; i++) {
        unsigned r = pal.rgb[i][0] >> 3, g = pal.rgb[i][1] >> 3, b = pal.rgb[i][2] >> 3;
        unsigned w = r | g << 5 | b << 10;
        words[i * 2] = (uint8_t)w;
        words[i * 2 + 1] = (uint8_t)(w >> 8);
    }
    put_bytes(addr, words, (size_t)pal.n * 2, name);
}

/* ---- tile sheets ----------------------------------------------------------- */
static int nearest(const Pal *pal, int levels, uint8_t r, uint8_t g, uint8_t b)
{
    if (pal->n <= 0) {                              /* brightness ramp */
        int y = (r * 30 + g * 59 + b * 11) / 100;
        return y * levels / 256;
    }
    int best = 0, bestd = 1 << 30;
    for (int i = 0; i < pal->n && i < levels; i++) {
        int dr = r - pal->rgb[i][0], dg = g - pal->rgb[i][1], db = b - pal->rgb[i][2];
        int d = dr * dr + dg * dg + db * db;
        if (d < bestd) { bestd = d; best = i; }
    }
    return best;
}

/* Encode one 8x8 tile of indices into SNES planar format (2/4/8 bpp). */
static void encode_tile(const uint8_t px[64], int bpp, uint8_t *out)
{
    memset(out, 0, (size_t)bpp * 8);
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++) {
            int v = px[y * 8 + x], bit = 7 - x;
            for (int pl = 0; pl < bpp; pl++)
                if (v >> pl & 1)
                    out[(pl >> 1) * 16 + y * 2 + (pl & 1)] |= (uint8_t)(1 << bit);
        }
}

static void apply_gfx(const char *dir, const char *path, const char *name, uint32_t addr)
{
    const char *b = strstr(name, "bpp");
    int bpp = (b && b > name && (b[-1] == '2' || b[-1] == '4' || b[-1] == '8')) ? b[-1] - '0' : 4;
    SDL_Surface *img = SDL_LoadPNG(path);
    if (!img) {
        SDL_Log("mods: %s: %s", name, SDL_GetError());
        return;
    }
    int w = img->w / 8, h = img->h / 8, levels = 1 << bpp;
    SDL_Palette *ipal = SDL_GetSurfacePalette(img);
    int indexed = ipal && SDL_BYTESPERPIXEL(img->format) == 1;
    Pal pal = { 0 };
    SDL_Surface *rgba = NULL;
    if (!indexed) {
        char pp[1024];                              /* <name>.pal.txt beside it */
        SDL_snprintf(pp, sizeof pp, "%s/%.*s.pal.txt", dir, (int)(strlen(name) - 4), name);
        load_pal_txt(pp, &pal);
        rgba = SDL_ConvertSurface(img, SDL_PIXELFORMAT_RGBA32);
        if (!rgba) { SDL_DestroySurface(img); return; }
    }
    size_t tsize = (size_t)bpp * 8, total = (size_t)w * h * tsize;
    if (s_gfx_limit && total > s_gfx_limit) total = s_gfx_limit;
    const char *nt = strstr(name, "_n");             /* _n<tiles>: real tile count */
    if (nt && nt[2] >= '0' && nt[2] <= '9') {
        size_t want = (size_t)strtoul(nt + 2, NULL, 10) * tsize;
        if (want < total) total = want;
    }
    uint8_t *out = SDL_malloc(total ? total : 1);
    for (int ty = 0; ty < h; ty++)
        for (int tx = 0; tx < w; tx++) {
            uint8_t px[64];
            for (int y = 0; y < 8; y++)
                for (int x = 0; x < 8; x++) {
                    int sx = tx * 8 + x, sy = ty * 8 + y, v;
                    if (indexed) {
                        v = ((uint8_t *)img->pixels)[sy * img->pitch + sx];
                    } else {
                        uint8_t *q = (uint8_t *)rgba->pixels + sy * rgba->pitch + sx * 4;
                        v = q[3] < 128 ? 0 : nearest(&pal, levels, q[0], q[1], q[2]);
                    }
                    px[y * 8 + x] = (uint8_t)(v & (levels - 1));
                }
            size_t at = ((size_t)ty * w + tx) * tsize;
            if (at + tsize <= total)
                encode_tile(px, bpp, out + at);
        }
    put_bytes(addr, out, total, name);
    SDL_free(out);
    if (rgba) SDL_DestroySurface(rgba);
    SDL_DestroySurface(img);
}


/* ---- sound samples (BRR) ---------------------------------------------------- */
static int brr_clamp(int v) { return v < -32768 ? -32768 : v > 32767 ? 32767 : v; }

/* Decode one 16-sample block with the given header, as the S-DSP does
 * (15-bit internal samples). Returns the squared error against want[]. */
static long long brr_try(const uint8_t *nibs, int shift, int filt, int *p1, int *p2,
                         const int16_t *want, int16_t *got)
{
    long long err = 0;
    int a = *p1, b = *p2;
    for (int i = 0; i < 16; i++) {
        int n = (i & 1) ? (nibs[i >> 1] & 15) : (nibs[i >> 1] >> 4);
        if (n >= 8) n -= 16;
        int v = shift <= 12 ? (n << shift) >> 1 : (n < 0 ? -2048 : 0);
        if (filt == 1) v += a + (-a >> 4);
        else if (filt == 2) v += 2 * a + ((-3 * a) >> 5) - b + (b >> 4);
        else if (filt == 3) v += 2 * a + ((-13 * a) >> 6) - b + ((3 * b) >> 4);
        v = brr_clamp(v);
        v = (int16_t)(v << 1) >> 1;                 /* 15-bit wrap */
        b = a; a = v;
        int out = v * 2;
        if (got) got[i] = (int16_t)out;
        long long d = (long long)out - want[i];
        err += d * d;
    }
    *p1 = a; *p2 = b;
    return err;
}

/* Choose the nibbles for one block: for each candidate shift/filter, quantise
 * sample by sample against the running predictor. */
static void brr_encode_block(const int16_t *want, int *p1, int *p2, int force_f0,
                             uint8_t flags, uint8_t *out9)
{
    long long best = -1;
    uint8_t bestblk[9];
    int bp1 = 0, bp2 = 0;
    for (int filt = 0; filt < (force_f0 ? 1 : 4); filt++)
        for (int shift = 0; shift <= 12; shift++) {
            uint8_t nib[8] = { 0 };
            int a = *p1, b = *p2;
            for (int i = 0; i < 16; i++) {
                int pred = 0;
                if (filt == 1) pred = a + (-a >> 4);
                else if (filt == 2) pred = 2 * a + ((-3 * a) >> 5) - b + (b >> 4);
                else if (filt == 3) pred = 2 * a + ((-13 * a) >> 6) - b + ((3 * b) >> 4);
                int target = want[i] / 2 - pred;
                int n = shift ? (int)SDL_lround((double)target * 2 / (1 << shift)) : target * 2;
                if (n > 7) n = 7;
                if (n < -8) n = -8;
                int v = brr_clamp(pred + ((n << shift) >> 1));
                v = (int16_t)(v << 1) >> 1;
                b = a; a = v;
                nib[i >> 1] |= (uint8_t)((n & 15) << ((i & 1) ? 0 : 4));
            }
            int t1 = *p1, t2 = *p2;
            long long e = brr_try(nib, shift, filt, &t1, &t2, want, NULL);
            if (best < 0 || e < best) {
                best = e;
                bestblk[0] = (uint8_t)(shift << 4 | filt << 2 | (flags & 3));
                memcpy(bestblk + 1, nib, 8);
                bp1 = t1; bp2 = t2;
            }
        }
    memcpy(out9, bestblk, 9);
    *p1 = bp1; *p2 = bp2;
}

/* Number of 9-byte BRR blocks from ROM offset off up to the end flag. */
static int brr_length(long off)
{
    for (int n = 1; n < 8192; n++) {
        long h = off + (long)(n - 1) * 9;
        if (h + 9 > (long)s_rom_size) return 0;
        if (s_rom[h] & 1) return n;
    }
    return 0;
}

static void apply_sound(const char *path, const char *name, uint32_t addr)
{
    long off = rom_offset(addr);
    int blocks = off < 0 ? 0 : brr_length(off);
    if (!blocks) {
        SDL_Log("mods: %s: no BRR sample at $%06X", name, (unsigned)addr);
        return;
    }
    size_t need = (size_t)blocks * 9;
    if (ends_with(name, ".brr")) {
        size_t n;
        uint8_t *p = read_file(path, &n);
        if (!p) return;
        if (n != need)
            SDL_Log("mods: %s: %u bytes, the slot holds %u; not used", name, (unsigned)n, (unsigned)need);
        else
            put_bytes(addr, p, n, name);
        SDL_free(p);
        return;
    }
    SDL_AudioSpec spec;
    Uint8 *buf = NULL;
    Uint32 len = 0;
    if (!SDL_LoadWAV(path, &spec, &buf, &len)) {
        SDL_Log("mods: %s: %s", name, SDL_GetError());
        return;
    }
    SDL_AudioSpec want = { SDL_AUDIO_S16, 1, spec.freq };
    Uint8 *mono = NULL;
    int mlen = 0;
    if (!SDL_ConvertAudioSamples(&spec, buf, (int)len, &want, &mono, &mlen) || mlen < 2) {
        SDL_Log("mods: %s: %s", name, SDL_GetError());
        SDL_free(buf);
        return;
    }
    SDL_free(buf);
    const int16_t *src = (const int16_t *)mono;
    int srcn = mlen / 2, dstn = blocks * 16;
    int16_t *pcm = SDL_malloc(sizeof(int16_t) * dstn);
    for (int i = 0; i < dstn; i++) {               /* linear resample to fit */
        double t = dstn > 1 ? (double)i * (srcn - 1) / (dstn - 1) : 0;
        int k = (int)t;
        double f = t - k;
        int v = (int)(src[k] * (1 - f) + (k + 1 < srcn ? src[k + 1] : src[k]) * f);
        pcm[i] = (int16_t)brr_clamp(v);
    }
    SDL_free(mono);
    uint8_t *out = SDL_malloc(need);
    int p1 = 0, p2 = 0;
    for (int b = 0; b < blocks; b++) {
        uint8_t flags = s_rom[off + (long)b * 9] & 3;  /* keep loop/end flags */
        brr_encode_block(pcm + b * 16, &p1, &p2, b == 0, flags, out + b * 9);
    }
    put_bytes(addr, out, need, name);
    SDL_Log("mods: %s: %d source samples fitted to %d (%d BRR blocks)", name, srcn, dstn, blocks);
    SDL_free(out);
    SDL_free(pcm);
}

/* ---- folder walk --------------------------------------------------------- */
typedef struct { char **v; int n, cap; } Names;

static SDL_EnumerationResult SDLCALL collect(void *ud, const char *dir, const char *name)
{
    (void)dir;
    Names *l = ud;
    if (l->n == l->cap) {
        l->cap = l->cap ? l->cap * 2 : 32;
        l->v = SDL_realloc(l->v, sizeof(char *) * l->cap);
    }
    l->v[l->n++] = SDL_strdup(name);
    return SDL_ENUM_CONTINUE;
}

static int cmp_names(const void *a, const void *b)
{
    return SDL_strcasecmp(*(char *const *)a, *(char *const *)b);
}

static Names list_dir(const char *dir)
{
    Names l = { 0 };
    SDL_EnumerateDirectory(dir, collect, &l);
    if (l.n) qsort(l.v, l.n, sizeof(char *), cmp_names);
    return l;
}

static void free_names(Names *l)
{
    for (int i = 0; i < l->n; i++) SDL_free(l->v[i]);
    SDL_free(l->v);
}

static int is_dir(const char *path)
{
    SDL_PathInfo info;
    return SDL_GetPathInfo(path, &info) && info.type == SDL_PATHTYPE_DIRECTORY;
}

static void apply_dir(const char *dir, int kind);
static void apply_sub(const char *mod, const char *sub, int kind)
{
    char dir[1024];
    SDL_snprintf(dir, sizeof dir, "%s/%s", mod, sub);
    apply_dir(dir, kind);
}

static void apply_dir(const char *dir, int kind)
{
    if (!is_dir(dir))
        return;
    Names l = list_dir(dir);
    for (int i = 0; i < l.n; i++) {
        const char *name = l.v[i];
        long addr = name_address(name);
        char path[1200];
        SDL_snprintf(path, sizeof path, "%s/%s", dir, name);
        if (addr < 0)
            continue;
        if (kind == 0 && ends_with(name, ".bin")) {
            size_t n;
            uint8_t *p = read_file(path, &n);
            if (p) { put_bytes((uint32_t)addr, p, n, name); SDL_free(p); }
        } else if (kind == 1 && ends_with(name, ".png")) {
            apply_gfx(dir, path, name, (uint32_t)addr);
        } else if (kind == 2 && ends_with(name, ".txt") && !ends_with(name, ".pal.txt")) {
            apply_pal(path, name, (uint32_t)addr);
        } else if (kind == 3 && (ends_with(name, ".wav") || ends_with(name, ".brr"))) {
            apply_sound(path, name, (uint32_t)addr);
        }
    }
    free_names(&l);
}

static void apply_mod(const char *mod)
{
    char p[1100];
    SDL_snprintf(p, sizeof p, "%s/disabled", mod);
    if (SDL_GetPathInfo(p, NULL))
        return;
    SDL_Log("mods: loading %s", mod);
    Names l = list_dir(mod);
    for (int i = 0; i < l.n; i++)
        if (ends_with(l.v[i], ".ips")) {
            SDL_snprintf(p, sizeof p, "%s/%s", mod, l.v[i]);
            apply_ips(p, l.v[i]);
        }
    free_names(&l);
    apply_sub(mod, "rom", 0);
    apply_sub(mod, "gfx", 1);
    apply_sub(mod, "pal", 2);
    apply_sub(mod, "sound", 3);
}

/* Called from GameInitialize (applying twice gives the same image). Returns
 * the number of changes made. */
/* ---- the launcher's Mods page (bunbun.ini) ------------------------------------- */
#include "bunbun_settings.h"
typedef struct { const char *key; uint32_t addr; } RiderSet;
#ifdef BUNBUN_GENB
static const RiderSet kRiderSets[] = {
    { "sprites.mario", 0x99C000 }, { "sprites.yoshi", 0x9A8000 }, { "sprites.kinopio", 0x9AC000 },
    { "sprites.wario", 0x9B8000 }, { "sprites.peach", 0x98C000 }, { "sprites.luigi", 0xA08000 },
};
#else
static const RiderSet kRiderSets[] = {
    { "sprites.mario", 0x868000 }, { "sprites.luigi", 0x86C000 },
    { "sprites.kinopio", 0x878000 }, { "sprites.wario", 0x87C000 },
};
#endif

static void apply_launcher_settings(void)
{
    if (bunbun_setting_int("feature.sprites", 1)) {
        for (size_t i = 0; i < sizeof kRiderSets / sizeof kRiderSets[0]; i++) {
            const char *path = bunbun_setting(kRiderSets[i].key, "");
            if (!path[0] || !SDL_GetPathInfo(path, NULL))
                continue;
            char dir[1024];
            SDL_snprintf(dir, sizeof dir, "%s", path);
            char *slash = SDL_strrchr(dir, '/'), *bslash = SDL_strrchr(dir, '\\');
            char *cut = slash > bslash ? slash : bslash;
            const char *name = cut ? cut + 1 : path;
            char nm[512];
            SDL_snprintf(nm, sizeof nm, "%s", name);
            if (cut) *cut = 0; else dir[0] = 0;
            s_gfx_limit = 0x4000;                       /* one 16 KB rider set */
            apply_gfx(dir, path, nm, kRiderSets[i].addr);
            s_gfx_limit = 0;
        }
    }
    if (bunbun_setting_int("feature.sound", 1)) {
        const char *dir = bunbun_setting("sound.folder", "");
        if (dir[0])
            apply_dir(dir, 3);
    }
}

int BunbunModsFolderApply(void)
{
    if (!g_snes || !g_snes->cart || !g_snes->cart->rom)
        return 0;
    s_changed = 0;
    s_rom = g_snes->cart->rom;
    s_rom_size = g_snes->cart->romSize;
    char root[1024];
    const char *env = getenv("BUNBUN_MODS");
    if (env && *env)
        SDL_snprintf(root, sizeof root, "%s", env);
    else
        SDL_snprintf(root, sizeof root, "%smods", SDL_GetBasePath() ? SDL_GetBasePath() : "");
    if (is_dir(root)) {
        Names l = list_dir(root);
        for (int i = 0; i < l.n; i++) {
            char mod[1100];
            SDL_snprintf(mod, sizeof mod, "%s/%s", root, l.v[i]);
            if (is_dir(mod) && SDL_strcasecmp(l.v[i], "preloaded") != 0)
                apply_mod(mod);
        }
        free_names(&l);
    }
    apply_launcher_settings();
    if (s_changed) {
        SDL_Log("mods: %d change(s) applied from %s", s_changed, root);
        const char *dump = getenv("BUNBUN_MODS_DUMP");  /* debugging: the patched image */
        if (dump && *dump)
            SDL_SaveFile(dump, s_rom, s_rom_size);
    }
    return s_changed;
}

/* ---- custom courses ---------------------------------------------------------- */
/* Called by the LoadCourse bridge: if some enabled mod has courses/courseN.bin
 * (the last one in name order wins), copy it over the track buffer (the rest
 * is zero-filled, like the original) and return 1. */
int BunbunCustomCourse(int index, uint8_t *buf, int cap)
{
    if (bunbun_setting_int("feature.courses", 1)) {
        char key[32];
        SDL_snprintf(key, sizeof key, "course.%d", index);
        const char *path = bunbun_setting(key, "");
        size_t n = 0;
        uint8_t *d = path[0] ? read_file(path, &n) : NULL;
        if (d) {
            if (n > (size_t)cap - 8) n = (size_t)cap - 8;
            memset(buf, 0, (size_t)cap);
            memcpy(buf, d, n);
            SDL_free(d);
            SDL_Log("mods: course %d from %s (%u columns)", index, path, (unsigned)n);
            return 1;
        }
    }
    char root[1024];
    const char *env = getenv("BUNBUN_MODS");
    if (env && *env)
        SDL_snprintf(root, sizeof root, "%s", env);
    else
        SDL_snprintf(root, sizeof root, "%smods", SDL_GetBasePath() ? SDL_GetBasePath() : "");
    if (!is_dir(root))
        return 0;
    Names l = list_dir(root);
    uint8_t *best = NULL;
    size_t bestn = 0;
    char bestname[1200] = "";
    for (int i = 0; i < l.n; i++) {
        char mod[1100], p[1200];
        SDL_snprintf(mod, sizeof mod, "%s/%s", root, l.v[i]);
        SDL_snprintf(p, sizeof p, "%s/disabled", mod);
        if (!is_dir(mod) || SDL_GetPathInfo(p, NULL))
            continue;
        SDL_snprintf(p, sizeof p, "%s/courses/course%d.bin", mod, index);
        size_t n;
        uint8_t *d = SDL_GetPathInfo(p, NULL) ? read_file(p, &n) : NULL;
        if (d) {
            SDL_free(best);
            best = d;
            bestn = n;
            SDL_snprintf(bestname, sizeof bestname, "%s", p);
        }
    }
    free_names(&l);
    if (!best)
        return 0;
    if (bestn > (size_t)cap - 8)
        bestn = (size_t)cap - 8;                    /* keep a zero tail */
    memset(buf, 0, (size_t)cap);
    memcpy(buf, best, bestn);
    SDL_free(best);
    SDL_Log("mods: course %d from %s (%u columns)", index, bestname, (unsigned)bestn);
    return 1;
}
