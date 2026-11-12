/*
 * bunbun_embedded.h - ROM dumps built into the executable.
 *
 * port/tools/embed_roms.py (run by tools/regen*.sh) writes the player's own
 * dumps into port/src/embed/<target>/bunbun_embedded.c, and the build links
 * it in; without it src/bunbun_embedded_none.c supplies an empty list and the
 * port reads data/ as before. An executable built with the dumps in it must
 * not be shared: it contains the game.
 */
#ifndef BUNBUN_EMBEDDED_H
#define BUNBUN_EMBEDDED_H

#include <stddef.h>

typedef struct BunbunEmbeddedDump {
    const char *name;               /* "s1.sfc", "s3.sfc", "s4.sfc" */
    const unsigned char *data;
    size_t size;
} BunbunEmbeddedDump;

/* Terminated by a NULL name. */
extern const BunbunEmbeddedDump kBunbunEmbedded[];

/* The embedded dump called `name`, or NULL. */
const BunbunEmbeddedDump *bunbun_embedded(const char *name);

/* Before the host starts: when the boot dump `rom_file` is built in, make
 * sure data/<rom_file> holds it and rom.cfg names it, so the launcher opens
 * on it with nothing supplied. Returns 1 when the built-in dump is used. */
int bunbun_embedded_prepare(const char *rom_file);

#endif
