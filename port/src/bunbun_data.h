#ifndef BUNBUN_DATA_H
#define BUNBUN_DATA_H
#include <stddef.h>
#include <stdint.h>

/* data/ beside the executable (BUNBUN_DATA overrides). */
const char *bunbun_data_dir(void);
/* Full path of data/<name>; returns 1 when the file exists. */
int bunbun_data_path(const char *name, char *out, size_t cap);
/* data/<name> as a 1 MB buffer (SDL_free it), or NULL. */
uint8_t *bunbun_data_load_dump(const char *name);
int bunbun_data_known(const char *name);
/* LoROM read from a 1 MB dump. */
uint8_t bunbun_lorom(const uint8_t *rom, uint32_t a);

#endif
