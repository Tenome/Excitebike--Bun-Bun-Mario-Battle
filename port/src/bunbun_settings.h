#ifndef BUNBUN_SETTINGS_H
#define BUNBUN_SETTINGS_H

/* bunbun.ini beside the executable (see bunbun_settings.c). */
void bunbun_settings_load(void);
int bunbun_settings_save(void);
const char *bunbun_setting(const char *key, const char *def);
int bunbun_setting_int(const char *key, int def);
void bunbun_setting_set(const char *key, const char *value);
/* Environment variable `env` when set, else setting `key`, else def. */
int bunbun_option(const char *env, const char *key, int def);

#endif
