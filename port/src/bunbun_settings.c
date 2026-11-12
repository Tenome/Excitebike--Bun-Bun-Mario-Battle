/*
 * bunbun_settings.c - the port's own settings, bunbun.ini beside the exe.
 *
 * Written by the launcher's Mods page (src/launcher_mods.c) and read by the
 * game (stadium, courses, flow, roster, sprite swaps, sound folder, custom
 * courses). One "key = value" per line; '#' starts a comment. The old
 * environment variables (BUNBUN_STADIUM, BUNBUN_COURSES, ...) still win when
 * they are set, for scripted runs.
 */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bunbun_settings.h"

#define MAX_KEYS 128
static char s_key[MAX_KEYS][64];
static char s_val[MAX_KEYS][512];
static int s_n, s_loaded;

static void path_of(char *out, size_t cap)
{
    const char *base = SDL_GetBasePath();
    SDL_snprintf(out, cap, "%sbunbun.ini", base ? base : "");
}

static void trim(char *s)
{
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\r' || s[n - 1] == '\n' || s[n - 1] == ' ' || s[n - 1] == '\t'))
        s[--n] = 0;
    size_t i = 0;
    while (s[i] == ' ' || s[i] == '\t') i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
}

void bunbun_settings_load(void)
{
    s_loaded = 1;
    s_n = 0;
    char p[1100];
    path_of(p, sizeof p);
    FILE *f = fopen(p, "r");
    if (!f)
        return;
    char line[700];
    while (fgets(line, sizeof line, f) && s_n < MAX_KEYS) {
        char *h = strchr(line, '#');
        if (h && (h == line || h[-1] == ' ' || h[-1] == '\t')) *h = 0;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char k[700], v[700];
        SDL_snprintf(k, sizeof k, "%s", line);
        SDL_snprintf(v, sizeof v, "%s", eq + 1);
        trim(k); trim(v);
        if (!k[0]) continue;
        SDL_snprintf(s_key[s_n], sizeof s_key[0], "%s", k);
        SDL_snprintf(s_val[s_n], sizeof s_val[0], "%s", v);
        s_n++;
    }
    fclose(f);
}

static int find(const char *key)
{
    if (!s_loaded) bunbun_settings_load();
    for (int i = 0; i < s_n; i++)
        if (strcmp(s_key[i], key) == 0)
            return i;
    return -1;
}

const char *bunbun_setting(const char *key, const char *def)
{
    int i = find(key);
    return i >= 0 ? s_val[i] : def;
}

int bunbun_setting_int(const char *key, int def)
{
    const char *v = bunbun_setting(key, NULL);
    return v && *v ? atoi(v) : def;
}

void bunbun_setting_set(const char *key, const char *value)
{
    int i = find(key);
    if (i < 0) {
        if (s_n >= MAX_KEYS) return;
        i = s_n++;
        SDL_snprintf(s_key[i], sizeof s_key[0], "%s", key);
    }
    SDL_snprintf(s_val[i], sizeof s_val[0], "%s", value ? value : "");
}

int bunbun_settings_save(void)
{
    char p[1100];
    path_of(p, sizeof p);
    FILE *f = fopen(p, "w");
    if (!f)
        return 0;
    fprintf(f, "# Bun Bun Mario Battle port settings (edited by the launcher's Mods page)\n");
    for (int i = 0; i < s_n; i++)
        fprintf(f, "%s = %s\n", s_key[i], s_val[i]);
    fclose(f);
    return 1;
}

/* An environment variable overrides the setting (scripted runs). */
int bunbun_option(const char *env, const char *key, int def)
{
    const char *e = getenv(env);
    if (e && *e)
        return atoi(e);
    return bunbun_setting_int(key, def);
}
