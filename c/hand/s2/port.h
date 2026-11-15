/*
 * port.h - hooks the native port answers in its native mode (the hand-written
 * C running the whole game, port/src/native_driver.c); the recompiled port
 * does the same in hle_funcs instead. c/hand/s1/port_rom.c answers as the
 * original does (nothing) for tools/difftest816.py.
 *
 *   port_end_screen_start  EndScreenStart ($80:9DE0) with Start pressed:
 *                          restart the broadcast hour and go to the credits
 *                          (1 = done, the routine's frame popped)
 *   port_bsx_exit          a JML into the BS-X BIOS at site: the port's exit
 *                          instead (1 = done, frame popped)
 *   port_course_loaded     LoadCourse has expanded a course into $7F:4000:
 *                          apply a custom course
 */
#ifndef HAND_S1_PORT_H
#define HAND_S1_PORT_H
#include "runtime816.h"
int port_end_screen_start(void);
int port_bsx_exit(u32 site);
void port_course_loaded(void);
#endif
