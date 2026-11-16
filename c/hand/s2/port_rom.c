/*
 * port_rom.c - the port hooks (port.h) as the original behaves: nothing.
 * Linked by tools/difftest816.py; the port answers them itself.
 */
#include "port.h"

int port_end_screen_start(void) { return 0; }
int port_bsx_exit(u32 site) { (void)site; return 0; }
void port_course_loaded(void) { }
void port_ws_clip_rider(void) { }
int port_ws_spark_margin(void) { return 0; }
void port_ws_spark_fix(void) { }
