/*
 * launcher_mods.c - the port's entries on the launcher's Mods page.
 *
 * The framework launcher (also opened in game with Ctrl+L / Select+L3) shows
 * mod packages with switches, choices and file pickers. This provider puts
 * the port's own settings there, in front of the framework's .snesmod
 * packages:
 *
 *   Bun Bun Mario Battle
 *     Game            game flow, Stadium, course set, expanded roster
 *     Sprite swaps    a PNG rider sheet per character (tools/assets.py layout)
 *     Music & sound   a folder of replacement samples (tools/sound.py names)
 *     Custom courses  a course file per course (tools/editor/course_editor.html)
 *   one package per folder in mods/ (on/off; off = a "disabled" file in it)
 *
 * Choices are kept in bunbun.ini (src/bunbun_settings.c); the game reads them
 * at start (a change made in game restarts it through a save state).
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "recomp_launcher.h"
#include "bunbun_settings.h"

#define PKG "bunbun"

static RecompLauncherCModProvider g_parent, g_prov;
static char g_error[256];

/* ---- our features ------------------------------------------------------------ */
typedef struct { const char *value, *label; } Choice;
typedef struct {
    const char *id, *label, *desc, *key, *def;
    int type;                       /* RECOMP_MOD_OPTION_* */
    const Choice *choices;
    int nchoices;
} Opt;
typedef struct { const char *id, *label, *desc, *key, *patterns, *what, *format; } Res;
typedef struct {
    const char *id, *name, *desc, *group;
    const Opt *opts; int nopts;
    const Res *res; int nres;
} Feature;

static const Choice kFlow[] = {
    { "menu", "Play without the broadcast clock" },
    { "broadcast", "Original broadcast timing" },
};
#ifdef BUNBUN_GENB
static const Choice kStadium[] = { { "4", "Stadium 4" }, { "3", "Stadium 3" } };
static const Choice kCourses[] = {
    { "own", "The Stadium's own courses" },
    { "1", "Stadium 1 courses (needs data/s1.sfc)" },
    { "2", "Stadium 2 courses (needs data/s1.sfc)" },
};
static const Choice kOnOff[] = { { "1", "On" }, { "0", "Off" } };
static const Choice kRivals[] = {
    { "all", "Seven CPU rivals in every race" },
    { "final", "Only in the final (original)" },
};
static const Choice kPace[] = {
    { "normal", "Normal (original)" },
    { "easy", "Easy: rivals 15% slower" },
    { "hard", "Hard: rivals 15% faster" },
};
static const Choice kWide[] = {
    { "off", "Off (4:3, the original picture)" },
    { "16:9", "16:9: races wide, other screens pillarboxed" },
};
static const Opt kGameOpts[] = {
    { "flow", "Game flow", "Without the clock the port moves on as soon as a screen is done; the original waits for the satellite broadcast's schedule.", "game.flow", "menu", RECOMP_MOD_OPTION_CHOICE, kFlow, 2 },
    { "stadium", "Stadium", "Which Stadium's rules, schedule and end screen to play.", "game.stadium", "4", RECOMP_MOD_OPTION_CHOICE, kStadium, 2 },
    { "courses", "Courses", "Play the Stadium 1 or 2 courses in this engine.", "game.courses", "own", RECOMP_MOD_OPTION_CHOICE, kCourses, 3 },
    { "luigi", "Expanded roster", "Add Luigi (from data/s3.sfc) as a sixth character.", "game.luigi", "1", RECOMP_MOD_OPTION_CHOICE, kOnOff, 2 },
    { "widescreen", "Widescreen", "Show more of the track to both sides in races. Takes effect when the game restarts.", "display.widescreen", "off", RECOMP_MOD_OPTION_CHOICE, kWide, 2 },
    { "pace", "Rival pace", "The target speed every CPU rival accelerates toward.", "game.pace", "normal", RECOMP_MOD_OPTION_CHOICE, kPace, 3 },
    { "rivals", "CPU rivals", "The original runs the course alone and meets the seven rivals only in the final; this fills every race with them.", "game.rivals", "all", RECOMP_MOD_OPTION_CHOICE, kRivals, 2 },
};
static const Res kSprites[] = {
    { "mario", "Mario rider sheet", "PNG, 8 tiles wide, 4 bpp (tools/assets.py export).", "sprites.mario", "*.png", "PNG image", "file" },
    { "yoshi", "Yoshi rider sheet", "", "sprites.yoshi", "*.png", "PNG image", "file" },
    { "kinopio", "Kinopio rider sheet", "", "sprites.kinopio", "*.png", "PNG image", "file" },
    { "wario", "Wario rider sheet", "", "sprites.wario", "*.png", "PNG image", "file" },
    { "peach", "Peach rider sheet", "", "sprites.peach", "*.png", "PNG image", "file" },
    { "luigi", "Luigi rider sheet", "", "sprites.luigi", "*.png", "PNG image", "file" },
};
#else
static const Choice kStadium[] = { { "1", "Stadium 1" }, { "2", "Stadium 2" } };
static const Opt kGameOpts[] = {
    { "flow", "Game flow", "Without the clock the port moves on as soon as a screen is done; the original waits for the satellite broadcast's schedule.", "game.flow", "menu", RECOMP_MOD_OPTION_CHOICE, kFlow, 2 },
    { "stadium", "Stadium", "Which Stadium's courses and rules to play.", "game.stadium", "1", RECOMP_MOD_OPTION_CHOICE, kStadium, 2 },
};
static const Res kSprites[] = {
    { "mario", "Mario rider sheet", "PNG, 8 tiles wide, 4 bpp (tools/assets.py export).", "sprites.mario", "*.png", "PNG image", "file" },
    { "luigi", "Luigi rider sheet", "", "sprites.luigi", "*.png", "PNG image", "file" },
    { "kinopio", "Kinopio rider sheet", "", "sprites.kinopio", "*.png", "PNG image", "file" },
    { "wario", "Wario rider sheet", "", "sprites.wario", "*.png", "PNG image", "file" },
};
#endif
static const Res kSound[] = {
    { "folder", "Sample folder", "WAV files named like tools/sound.py export (BBAAAA_....wav); each replaces that sample.", "sound.folder", "", "", "directory" },
};
static const Res kCourseFiles[] = {
    { "c0", "Course 1", "courseN.bin from the course editor.", "course.0", "*.bin", "Course file", "file" },
    { "c1", "Course 2", "", "course.1", "*.bin", "Course file", "file" },
    { "c2", "Course 3", "", "course.2", "*.bin", "Course file", "file" },
    { "c3", "Course 4", "", "course.3", "*.bin", "Course file", "file" },
    { "c4", "Course 5", "", "course.4", "*.bin", "Course file", "file" },
    { "c5", "Course 6", "", "course.5", "*.bin", "Course file", "file" },
    { "c6", "Course 7", "", "course.6", "*.bin", "Course file", "file" },
    { "c7", "Course 8", "", "course.7", "*.bin", "Course file", "file" },
};
#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))
static const Feature kFeatures[] = {
    { "game", "Game", "How the game flows, which Stadium and courses, and the roster.", "Game", kGameOpts, N(kGameOpts), NULL, 0 },
    { "sprites", "Sprite swaps", "Replace a character's rider sprites with your own sheet.", "Graphics", NULL, 0, kSprites, N(kSprites) },
    { "sound", "Music & sound", "Replace instrument, voice and effect samples.", "Audio", NULL, 0, kSound, N(kSound) },
    { "courses", "Custom courses", "Play your own courses (make them with tools/editor/course_editor.html).", "Courses", NULL, 0, kCourseFiles, N(kCourseFiles) },
};
#define NFEAT N(kFeatures)

static int feature_on(int i)
{
    char k[96];
    SDL_snprintf(k, sizeof k, "feature.%s", kFeatures[i].id);
    return bunbun_setting_int(k, 1);
}

/* ---- folders in mods/ --------------------------------------------------------- */
#define MAX_FOLDERS 64
static char g_folder[MAX_FOLDERS][128];
static int g_nfolders;

static SDL_EnumerationResult SDLCALL collect(void *ud, const char *dir, const char *name)
{
    (void)ud;
    char p[1200];
    SDL_PathInfo info;
    SDL_snprintf(p, sizeof p, "%s/%s", dir, name);
    if (g_nfolders < MAX_FOLDERS && SDL_GetPathInfo(p, &info) && info.type == SDL_PATHTYPE_DIRECTORY &&
        SDL_strcasecmp(name, "preloaded") != 0)
        SDL_snprintf(g_folder[g_nfolders++], sizeof g_folder[0], "%s", name);
    return SDL_ENUM_CONTINUE;
}

static void mods_root(char *out, size_t cap)
{
    const char *env = getenv("BUNBUN_MODS");
    const char *base = SDL_GetBasePath();
    if (env && *env) SDL_snprintf(out, cap, "%s", env);
    else SDL_snprintf(out, cap, "%smods", base ? base : "");
}

static void scan_folders(void)
{
    char root[1024];
    g_nfolders = 0;
    mods_root(root, sizeof root);
    SDL_EnumerateDirectory(root, collect, NULL);
}

static int folder_enabled(int i)
{
    char root[1024], p[1300];
    mods_root(root, sizeof root);
    SDL_snprintf(p, sizeof p, "%s/%s/disabled", root, g_folder[i]);
    return !SDL_GetPathInfo(p, NULL);
}

static int folder_set(int i, int on)
{
    char root[1024], p[1300];
    mods_root(root, sizeof root);
    SDL_snprintf(p, sizeof p, "%s/%s/disabled", root, g_folder[i]);
    if (on)
        return SDL_RemovePath(p) || !SDL_GetPathInfo(p, NULL);
    FILE *f = fopen(p, "w");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static int folder_index(const char *package_id)
{
    if (strncmp(package_id, "folder.", 7) != 0) return -1;
    for (int i = 0; i < g_nfolders; i++)
        if (strcmp(g_folder[i], package_id + 7) == 0)
            return i;
    return -1;
}

/* ---- the provider ------------------------------------------------------------- */
static int ours(const char *package_id) { return package_id && strcmp(package_id, PKG) == 0; }
static int our_count(void) { return NFEAT + g_nfolders; }
static int feat_by_id(const char *id)
{
    for (int i = 0; i < NFEAT; i++)
        if (strcmp(kFeatures[i].id, id) == 0)
            return i;
    return -1;
}
#define T(f, ...) SDL_snprintf(out->f, sizeof out->f, __VA_ARGS__)

static int PackageCount(void *c)
{
    scan_folders();
    return 1 + g_nfolders + (g_parent.package_count ? g_parent.package_count(c) : 0);
}

static int PackageGet(void *c, int i, RecompLauncherCModPackage *out)
{
    if (i == 0) {
        memset(out, 0, sizeof *out);
        T(id, PKG); T(version, "1"); T(name, "Bun Bun Mario Battle");
        T(author, "port"); T(description, "The port's own options: game flow, Stadium, roster, sprite swaps, sounds and custom courses.");
        out->enabled = 1; out->option_count = 0;
        return 1;
    }
    if (i - 1 < g_nfolders) {
        int k = i - 1;
        memset(out, 0, sizeof *out);
        T(id, "folder.%s", g_folder[k]); T(version, "folder"); T(name, "%s", g_folder[k]);
        T(author, "mods/ folder"); T(description, "Files from mods/%s (graphics, palettes, sounds, courses, patches; docs/MODDING.md).", g_folder[k]);
        out->enabled = folder_enabled(k);
        return 1;
    }
    return g_parent.package_get ? g_parent.package_get(c, i - 1 - g_nfolders, out) : 0;
}

static int FeatureCount(void *c)
{
    scan_folders();
    return our_count() + (g_parent.feature_count ? g_parent.feature_count(c) : 0);
}

static int FeatureGet(void *c, int i, RecompLauncherCModFeature *out)
{
    if (i < NFEAT) {
        const Feature *f = &kFeatures[i];
        memset(out, 0, sizeof *out);
        T(id, "%s", f->id); T(package_id, PKG); T(package_version, "1");
        T(package_name, "Bun Bun Mario Battle"); T(name, "%s", f->name); T(author, "port");
        T(description, "%s", f->desc); T(group, "%s", f->group);
        out->enabled = feature_on(i);
        T(status, "%s", out->enabled ? "On" : "Off");
        out->option_count = f->nopts;
        out->quick = i == 0;
        return 1;
    }
    if (i - NFEAT < g_nfolders) {
        int k = i - NFEAT;
        memset(out, 0, sizeof *out);
        T(id, "main"); T(package_id, "folder.%s", g_folder[k]); T(package_version, "folder");
        T(package_name, "%s", g_folder[k]); T(name, "%s", g_folder[k]); T(author, "mods/ folder");
        T(description, "Files from mods/%s.", g_folder[k]); T(group, "Mods folder");
        out->enabled = folder_enabled(k);
        T(status, "%s", out->enabled ? "On" : "Off (a 'disabled' file is in the folder)");
        return 1;
    }
    return g_parent.feature_get ? g_parent.feature_get(c, i - our_count(), out) : 0;
}

static int FeatureOptionGet(void *c, const char *p, const char *f, int i, RecompLauncherCModOption *out)
{
    if (!ours(p)) {
        if (folder_index(p) >= 0) return 0;
        return g_parent.feature_option_get ? g_parent.feature_option_get(c, p, f, i, out) : 0;
    }
    int fi = feat_by_id(f);
    if (fi < 0 || i < 0 || i >= kFeatures[fi].nopts) return 0;
    const Opt *o = &kFeatures[fi].opts[i];
    memset(out, 0, sizeof *out);
    T(id, "%s", o->id); T(label, "%s", o->label); T(description, "%s", o->desc);
    T(value, "%s", bunbun_setting(o->key, o->def)); T(default_value, "%s", o->def);
    out->type = o->type; out->choice_count = o->nchoices;
    return 1;
}

static int FeatureChoiceGet(void *c, const char *p, const char *f, const char *opt, int i, RecompLauncherCModChoice *out)
{
    if (!ours(p))
        return g_parent.feature_choice_get && folder_index(p) < 0 ? g_parent.feature_choice_get(c, p, f, opt, i, out) : 0;
    int fi = feat_by_id(f);
    if (fi < 0) return 0;
    for (int k = 0; k < kFeatures[fi].nopts; k++) {
        const Opt *o = &kFeatures[fi].opts[k];
        if (strcmp(o->id, opt) == 0 && i >= 0 && i < o->nchoices) {
            memset(out, 0, sizeof *out);
            T(value, "%s", o->choices[i].value); T(label, "%s", o->choices[i].label);
            return 1;
        }
    }
    return 0;
}

static int FeatureEnable(void *c, const char *p, const char *f, int e)
{
    int k = folder_index(p);
    if (k >= 0) return folder_set(k, e);
    if (!ours(p)) return g_parent.feature_enable ? g_parent.feature_enable(c, p, f, e) : 0;
    int fi = feat_by_id(f);
    if (fi < 0) return 0;
    char key[96];
    SDL_snprintf(key, sizeof key, "feature.%s", f);
    bunbun_setting_set(key, e ? "1" : "0");
    return bunbun_settings_save();
}

static int FeatureSetOption(void *c, const char *p, const char *f, const char *opt, const char *v)
{
    if (!ours(p))
        return g_parent.feature_set_option && folder_index(p) < 0 ? g_parent.feature_set_option(c, p, f, opt, v) : 0;
    int fi = feat_by_id(f);
    if (fi < 0 || !opt) return 0;
    for (int k = 0; k < kFeatures[fi].nopts; k++) {
        const Opt *o = &kFeatures[fi].opts[k];
        if (strcmp(o->id, opt) == 0) {
            bunbun_setting_set(o->key, v);
            return bunbun_settings_save();
        }
    }
    return 0;
}

static int ResourceCount(void *c, const char *p, const char *f)
{
    if (!ours(p))
        return g_parent.feature_resource_count && folder_index(p) < 0 ? g_parent.feature_resource_count(c, p, f) : 0;
    int fi = feat_by_id(f);
    return fi < 0 ? 0 : kFeatures[fi].nres;
}

static int ResourceGet(void *c, const char *p, const char *f, int i, RecompLauncherCModResource *out)
{
    if (!ours(p))
        return g_parent.feature_resource_get && folder_index(p) < 0 ? g_parent.feature_resource_get(c, p, f, i, out) : 0;
    int fi = feat_by_id(f);
    if (fi < 0 || i < 0 || i >= kFeatures[fi].nres) return 0;
    const Res *r = &kFeatures[fi].res[i];
    memset(out, 0, sizeof *out);
    T(id, "%s", r->id); T(label, "%s", r->label); T(description, "%s", r->desc);
    const char *path = bunbun_setting(r->key, "");
    T(path, "%s", path);
    int exists = path[0] && SDL_GetPathInfo(path, NULL);
    T(status, "%s", !path[0] ? "Not set (original)" : exists ? "Selected" : "File not found");
    T(file_patterns, "%s", r->patterns); T(file_description, "%s", r->what); T(format, "%s", r->format);
    out->required = 0;
    out->verified = exists;
    return 1;
}

static int ResourceSetPath(void *c, const char *p, const char *f, const char *rid, const char *path)
{
    if (!ours(p))
        return g_parent.feature_resource_set_path && folder_index(p) < 0 ? g_parent.feature_resource_set_path(c, p, f, rid, path) : 0;
    int fi = feat_by_id(f);
    if (fi < 0) return 0;
    for (int k = 0; k < kFeatures[fi].nres; k++)
        if (strcmp(kFeatures[fi].res[k].id, rid) == 0) {
            bunbun_setting_set(kFeatures[fi].res[k].key, path ? path : "");
            return bunbun_settings_save();
        }
    return 0;
}

static int SetEnabled(void *c, const char *p, int e)
{
    int k = folder_index(p);
    if (k >= 0) return folder_set(k, e);
    if (ours(p)) return 1;
    return g_parent.set_enabled ? g_parent.set_enabled(c, p, e) : 0;
}

static int Commit(void *c, const char *image)
{
    bunbun_settings_save();
    return g_parent.commit ? g_parent.commit(c, image) : 1;
}

static const char *LastError(void *c)
{
    if (g_error[0]) return g_error;
    return g_parent.last_error ? g_parent.last_error(c) : "";
}

const RecompLauncherCModProvider *BunbunModsProvider(const RecompLauncherCModProvider *parent)
{
    g_parent = parent ? *parent : (RecompLauncherCModProvider){ 0 };
    g_prov = g_parent;
    g_prov.package_count = PackageCount;
    g_prov.package_get = PackageGet;
    g_prov.set_enabled = SetEnabled;
    g_prov.commit = Commit;
    g_prov.last_error = LastError;
    g_prov.feature_count = FeatureCount;
    g_prov.feature_get = FeatureGet;
    g_prov.feature_option_get = FeatureOptionGet;
    g_prov.feature_choice_get = FeatureChoiceGet;
    g_prov.feature_enable = FeatureEnable;
    g_prov.feature_set_option = FeatureSetOption;
    g_prov.feature_resource_count = ResourceCount;
    g_prov.feature_resource_get = ResourceGet;
    g_prov.feature_resource_set_path = ResourceSetPath;
    g_prov.feature_set_quick = NULL;
    /* the parent's commit plans may run on a worker; ours are file writes only */
    g_prov.try_commit = NULL;
    g_prov.preparation_revision = NULL;
    g_prov.commit_with_progress = NULL;
    g_prov.commit_worker_safe = 0;
    return &g_prov;
}
