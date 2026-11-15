/*
 * ram.h - Stadium 3's (Gen B) work RAM by name, for the address operands of
 * the hand-written C: rd8(DP(GAME_MODE)), wr8(ABSX(RIDER_SPEED_HI), A).
 * The names are applied by tools/ramname.py (which also writes them back as
 * hex for tools/port_hand.py); an address without a name here stays hex.
 * Only addresses whose use is understood are named; the meaning of each is
 * taken from the routines that read and write it (see their comments).
 *
 * The layout is Stadium 4's (c/hand/s4/ram.h): the names that the routines
 * shared by the two dumps use sit at the same addresses in both.
 *
 * Direct-page names apply where D = 0 (the game runs with D = 0 except in a
 * few DMA and register routines, which ramname.py leaves alone).
 */
#ifndef HAND_S3_RAM_H
#define HAND_S3_RAM_H

#include "hand/snes_regs.h"
#include "hand/long_ram.h"

/* ---- joypads (ReadJoypads): hi = B Y Select Start Up Down Left Right,
 * lo = A X L R in the top nibble; "press" = newly pressed this frame ---- */
#define P1_HELD_HI      0x0020
#define P1_PRESS_HI     0x0021
#define P2_HELD_HI      0x0022
#define P2_PRESS_HI     0x0023
#define P1_HELD_LO      0x0024
#define P2_HELD_LO      0x0025
#define P1_PRESS_LO     0x0026
#define P2_PRESS_LO     0x0027
#define P1_PREV_HI      0x0028
#define P2_PREV_HI      0x0029
#define P1_PREV_LO      0x002A
#define P2_PREV_LO      0x002B

/* ---- game flow ---- */
#define FRAME_COUNT     0x0063  /* word: +1 per game frame (main loop) */
#define MODE_ACTIVE     0x0075  /* RunGameModeScript: 0 = run the mode's init */
#define GAME_MODE       0x0076  /* index of the mode's script ($80:B52B) */
#define MODE_STEP       0x0077  /* position in the mode's handler list */
#define COURSE_SLOT     0x007C  /* course counter in the broadcast schedule */
#define FINAL_RACE      0x007D  /* the final of the course (second race) */
#define RIDER_TURN      0x007F  /* frame & 7: the rider whose turn it is */
#define MODE_TIMER      0x0088  /* word: the current mode's frame timer */
#define MODE_HOLD       0x0050  /* RunGameModeScript: setup waits while set */
#define RACE_STARTED    0x0092  /* the run has begun (countdown over) */
#define SCROLL_X        0x009C  /* word: the track's scroll position */
#define CLOCK_MINUTE    0x00B7  /* broadcast minute, copied from $7F:FFFF */
#define COUNTDOWN       0x00C3  /* the start countdown's step (1-4) */
#define NMI_FRAME_READY 0x0817  /* set by the NMI: the main loop runs a frame */
#define FADE_STATE      0x1923  /* +1 fading in, -1 fading out, 0 still */
#define DEBUG_PAUSE     0x1917  /* debug build: pause toggle */
#define PALETTE_DIRTY   0x1915  /* upload the palette buffer in the NMI */

/* ---- PPU register shadows (copied to the PPU in the NMI) ---- */
#define SH_INIDISP      0x1903
#define SH_TM           0x1905
#define SH_TS           0x1906
#define SH_TMW          0x190A
#define SH_TSW          0x190B
#define SH_CGWSEL       0x190C
#define SH_CGADSUB      0x190D
#define SH_NMITIMEN     0x190E
#define SH_HDMAEN       0x190F
#define PALETTE_BUF     0x1C00  /* $0200 bytes: CGRAM image */

/* ---- riders: parallel arrays, stride 8, index X = rider 0-7 (0 = the
 * player); the name is the address of rider 0's entry ---- */
#define RIDER_SCRIPT    0x0176  /* running piece action (0 none; $14/$15 jump landing, $18 boost ground) */
#define RIDER_INPUT     0x017E  /* d-pad and buttons this frame (bit 6 turbo); CPU and demo riders write it */
#define RIDER_STEP      0x0186  /* pixels moved this frame (speed high + fraction carry) */
#define RIDER_WHEEL     0x018E  /* wheel animation counter (counts down by the step) */
#define RIDER_X         0x0196  /* track position, low byte */
#define RIDER_PAGE      0x019E  /* track position, high byte (screen page) */
#define RIDER_POSE      0x01A6  /* pose for the lean table ($80:BA92) */
#define RIDER_STEER     0x01AE  /* lane steering +1 / -1 ($FF) */
#define RIDER_Y         0x01B6  /* screen height */
#define RIDER_SPEED_LO  0x01BE  /* speed, fraction */
#define RIDER_SPEED_HI  0x01C6  /* speed, pixels per frame */
#define RIDER_BUMPED    0x01CE  /* knocked back by a rear bump ($FF) */
#define RIDER_TIMER     0x01D6  /* pose / wave timer at the start and finish */
#define RIDER_ACTIVE    0x01DE  /* racing on screen */
#define RIDER_LEAN      0x01E6  /* lean angle (6 = upright) */
#define RIDER_AIR       0x01EE  /* in the air (0 = on the ground) */
#define RIDER_LANE      0x01F6  /* lane position (sorted for the ranking) */
#define RIDER_HEIGHT    0x01FE  /* jump height */
#define RIDER_LANDING   0x0206  /* landing kind (2 = flat) */
#define RIDER_JUMP      0x020E  /* jump state (a rival placed mid-jump: 1 or 3) */
#define RIDER_TILE_Y    0x0216  /* height of the track piece under the wheel */
#define RIDER_TILE      0x021E  /* type of the track piece under the wheel */
#define RIDER_TARGET_LO 0x022E  /* CPU rider's target speed, fraction */
#define RIDER_TARGET_HI 0x0236  /* CPU rider's target speed, high */
#define RIDER_SLOW_AT   0x0246  /* speed threshold for the slow mark */
#define RIDER_DISTANCE  0x024E  /* distance into the piece script */
#define RIDER_RETIRED   0x025E  /* left the screen, retired */
#define RIDER_COLUMN    0x0266  /* track column under the rider (0-63) */
#define RIDER_JUMP_BASE 0x026E  /* jump height base */
#define RIDER_ON_RAMP   0x08BC  /* 2 = on a ramp */
#define RIDER_SLOW      0x08F4  /* marked slow (below RIDER_SLOW_AT) */
#define RIDER_RUNOUT    0x090C  /* run-out column past the finish */
#define RIDER_X_FRAC    0x0914  /* track position fraction */
#define RIDER_BAND      0x0958  /* which band of the track the rider is in */
#define RIDER_WANT_LEAN 0x0971  /* CPU rider's wanted lean */
#define RIDER_LANE_NUM  0x08A4  /* lane 5..1 from the lane position */

#define RIDER_DIST      0x085E  /* words, 2X: distance to the player */
#define RANK_ORDER      0x0950  /* 8 bytes: rider numbers by lane position */
#define ENGINE_HEAT     0x0990  /* the player's engine temperature */

/* ---- timers, scrolling, the HUD ---- */
#define TIMER_A         0x008A  /* a mode's second timer (the start countdown's beeps) */
#define TIMER_B         0x008C  /* a mode's third timer (the waving pose) */
#define SCROLL_HALF     0x0090  /* BG1 nametable half, flips on each scroll carry */
#define BG_SCROLL       0x009A  /* BG1 horizontal scroll (+= the player's step) */
#define SCROLL_PAGE     0x009D  /* scroll pages passed (high byte of SCROLL_X) */
#define OVERHEATED      0x0060  /* the engine is overheated (odd) */
#define JUMP_STATE      0x00C7  /* jump contest: 1 the drop, 2 the run-out */
#define PLAYER_PLACE    0x086E  /* the player's place (8 minus the riders beaten) */
#define COIN_COUNT      0x0CDF  /* coins collected (5 win the turbo) */
#define TURBO_ON        0x0CE0  /* the coin turbo is running */
#define ENGINE_PITCH    0x198A  /* engine sound pitch, sent to APU port 2 */
#define DIGIT_BUF       0x03CF  /* number staging: the size byte, then two rows of tiles */
#define DIGIT_ROW       0x03D0  /* the upper row of DIGIT_BUF (the lower row is +$10) */
#define TRACK_ROW_PTR   0x0276  /* the track column's VRAM row pointer */
#define TRACK_VRAM_COL  0x0277  /* the track column's VRAM column (wraps at $20) */
#define TRACK_VRAM_HALF 0x0278  /* the track column's nametable half (^ 4) */
#define END_STEP        0x0450  /* end screen: its step (table $80:98C8) */
#define ENTRY_CURSOR    0x044F  /* end screen: the digit being entered */
#define JUMP_LAST_DIST  0x1567  /* word: the last jump distance (half-units) */
#define JUMP_COUNT      0x1569  /* word: the jump count */
#define JUMP_RANK       0x156B  /* the jump's rank ($FF none) */
#define JUMP_BEST_SCORE 0x156D  /* word: the best score */

/* ---- sprites ---- */
#define OAM_BUF         0x1600  /* $0200 bytes: the OAM low table image */
#define OAM_SIZE_BUF    0x1820  /* 128 size bytes, packed into the high table */
#define SPRITES_FROZEN  0x1958  /* leave the sprites where they are */

/* ---- objects (the jump contest's and the bonus screens' actors):
 * 16-byte arrays of 8 word slots, index X = 2 * slot (14-0); SpawnObject
 * fills a free one ---- */
#define OBJ_X           0x027F  /* word: X position */
#define OBJ_Y           0x028F  /* word: Y position */
#define OBJ_VX          0x029F  /* word: horizontal velocity */
#define OBJ_VY          0x02AF  /* word: vertical velocity */
#define OBJ_ACTIVE      0x02EF  /* slot in use */
#define OBJ_TYPE        0x02FF  /* type: picks its init routine and script */
#define OBJ_STATE       0x030F
#define OBJ_STEP        0x031F
#define OBJ_RANDOM      0x032F  /* a random count */
#define OBJ_FRAME       0x034F
#define OBJ_DELAY       0x036F
#define OBJ_COUNTER     0x037F
#define OBJ_SCRIPT      0x038F  /* word: the type's script */
#define OBJ_TARGET_X    0x039F  /* word: the point an object steers to */
#define OBJ_TARGET_Y    0x03A1
#define OBJ_CURRENT     0x03A3  /* the slot being run */

#endif
