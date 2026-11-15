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
#define RIDER_TURN      0x007F  /* [+$8] frame & 7: the rider whose turn it is */
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
#define PALETTE_BUF     0x1C00  /* [+$200] $0200 bytes: CGRAM image */

/* ---- riders: parallel arrays, stride 8, index X = rider 0-7 (0 = the
 * player); the name is the address of rider 0's entry ---- */
#define RIDER_SCRIPT    0x0176  /* [+$8] running piece action (0 none; $14/$15 jump landing, $18 boost ground) */
#define RIDER_INPUT     0x017E  /* [+$8] d-pad and buttons this frame (bit 6 turbo); CPU and demo riders write it */
#define RIDER_STEP      0x0186  /* [+$8] pixels moved this frame (speed high + fraction carry) */
#define RIDER_WHEEL     0x018E  /* [+$8] wheel animation counter (counts down by the step) */
#define RIDER_X         0x0196  /* [+$8] track position, low byte */
#define RIDER_PAGE      0x019E  /* [+$8] track position, high byte (screen page) */
#define RIDER_POSE      0x01A6  /* [+$8] pose for the lean table ($80:BA92) */
#define RIDER_STEER     0x01AE  /* [+$8] lane steering +1 / -1 ($FF) */
#define RIDER_Y         0x01B6  /* [+$8] screen height */
#define RIDER_SPEED_LO  0x01BE  /* [+$8] speed, fraction */
#define RIDER_SPEED_HI  0x01C6  /* [+$8] speed, pixels per frame */
#define RIDER_BUMPED    0x01CE  /* [+$8] knocked back by a rear bump ($FF) */
#define RIDER_TIMER     0x01D6  /* [+$8] pose / wave timer at the start and finish */
#define RIDER_ACTIVE    0x01DE  /* [+$8] racing on screen */
#define RIDER_LEAN      0x01E6  /* [+$8] lean angle (6 = upright) */
#define RIDER_AIR       0x01EE  /* [+$8] in the air (0 = on the ground) */
#define RIDER_LANE      0x01F6  /* [+$8] lane position (sorted for the ranking) */
#define RIDER_HEIGHT    0x01FE  /* [+$8] jump height */
#define RIDER_LANDING   0x0206  /* [+$8] landing kind (2 = flat) */
#define RIDER_JUMP      0x020E  /* [+$8] jump state (a rival placed mid-jump: 1 or 3) */
#define RIDER_TILE_Y    0x0216  /* [+$8] height of the track piece under the wheel */
#define RIDER_TILE      0x021E  /* [+$8] type of the track piece under the wheel */
#define RIDER_TARGET_LO 0x022E  /* [+$8] CPU rider's target speed, fraction */
#define RIDER_TARGET_HI 0x0236  /* [+$8] CPU rider's target speed, high */
#define RIDER_SLOW_AT   0x0246  /* [+$8] speed threshold for the slow mark */
#define RIDER_DISTANCE  0x024E  /* [+$8] distance into the piece script */
#define RIDER_RETIRED   0x025E  /* [+$8] left the screen, retired */
#define RIDER_COLUMN    0x0266  /* [+$8] track column under the rider (0-63) */
#define RIDER_JUMP_BASE 0x026E  /* [+$8] jump height base */
#define RIDER_ON_RAMP   0x08BC  /* [+$8] 2 = on a ramp */
#define RIDER_SLOW      0x08F4  /* [+$8] marked slow (below RIDER_SLOW_AT) */
#define RIDER_RUNOUT    0x090C  /* [+$8] run-out column past the finish */
#define RIDER_X_FRAC    0x0914  /* [+$8] track position fraction */
#define RIDER_BAND      0x0958  /* [+$8] which band of the track the rider is in */
#define RIDER_WANT_LEAN 0x0971  /* [+$8] CPU rider's wanted lean */
#define RIDER_LANE_NUM  0x08A4  /* [+$8] lane 5..1 from the lane position */

#define RIDER_DIST      0x085E  /* [+$10] words, 2X: distance to the player */
#define RANK_ORDER      0x0950  /* [+$8] 8 bytes: rider numbers by lane position */
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
#define DIGIT_ROW       0x03D0  /* [+$20] the upper row of DIGIT_BUF (the lower row is +$10) */
#define TRACK_ROW_PTR   0x0276  /* the track column's VRAM row pointer */
#define TRACK_VRAM_COL  0x0277  /* the track column's VRAM column (wraps at $20) */
#define TRACK_VRAM_HALF 0x0278  /* the track column's nametable half (^ 4) */
#define END_STEP        0x0450  /* end screen: its step (table $80:98C8) */
#define ENTRY_CURSOR    0x044F  /* end screen: the digit being entered */
#define JUMP_LAST_DIST  0x1567  /* word: the last jump distance (half-units) */
#define JUMP_COUNT      0x1569  /* word: the jump count */
#define JUMP_RANK       0x156B  /* the jump's rank ($FF none) */
#define JUMP_BEST_SCORE 0x156D  /* word: the best score */

/* ---- sound commands: four channels, one per APU port. The game appends a
 * command to channel n's ring; the NMI sends it to APUIO<n> and waits for
 * the port to echo it before it sends the next. Only channel 0 is ever
 * queued (QueueSound); rings 1-3 are only cleared. ---- */
#define SND_WRITE       0x0065  /* + channel: the ring's write index (0-63) */
#define SND_READ        0x0069  /* + channel: the ring's read index (0-63) */
#define SND_PENDING     0x006D  /* + channel: the command sent, waiting for the echo */
#define SND_LAST        0x0071  /* the last command queued */
#define SND_RING        0x1400  /* [+$100] 4 rings of 64 commands, channel n at + $40 * n */

/* ---- more direct page ---- */
#define RANDOM_LFSR     0x002C  /* 12 bytes: the shift register StepLfsr turns (a random source) */
#define RIDER_LEAN_TIMER 0x0045  /* [+$8] X = rider: frames until the next lean step (the finish wave timer) */
#define RACE_END        0x0084  /* the run is over (2 = the time ran out; even and nonzero ends the mode) */
#define SUB_STEP        0x0085  /* a screen's own step: the DispatchBy85 tables, the 20 jump steps */
#define UPLOAD_SRC      0x0086  /* word: the NMI animation tile upload source ($0200 bytes; Gen B bank $9B to VRAM $5A00, Gen A $9E to $4A00) */
#define LIGHTS_TIMER    0x008E  /* how long the countdown lights stay up */
#define HDMA_ZOOM       0x0095  /* the IRQ builds the HDMA line table from the scroll */
#define PARALLAX_ON     0x0096  /* the IRQ builds the parallax lines (ParallaxLines) */
#define BONUS_SIGN      0x0097  /* the bonus course sign is up */
#define BG1_VSCROLL     0x009E  /* word: BG1 vertical scroll */
#define ZOOM            0x00A2  /* word: the jump and bonus screens' zoom ($0100 = 1:1) */
#define COURSE_COLUMN   0x00AE  /* the course column being loaded into the track */
#define BONUS_SECONDS   0x00B4  /* the bonus course clock, seconds left */
#define BONUS_TICK      0x00B5  /* frames into the bonus clock's second */
#define JUMP_BOARD_STEP 0x00CD  /* jump contest result board: its step */

/* ---- race globals ---- */
#define LAP             0x0928  /* the lap number (Gen B also shows it as the stadium badge) */
#define ATTRACT_DEMO    0x0CD7  /* the attract demo or a practice run (no pad, the special course) */
#define OVERHEAT_LOCK   0x0940  /* overheated: set with OVERHEATED, the player cools down standing */
#define GAUGE_ON        0x092D  /* the player's throttle flag: the speed gauge is on */
#define TOTAL_TIME      0x084D  /* [+$3] 3 bytes: the broadcast's total time (minutes, seconds, hundredths) */
#define BONUS_RESULT    0x09C5  /* the bonus course's landing points */
#define TRACK_ROWS      0x09D3  /* [+$300] six 128-byte track tables (tile, height, flags, coin value ...; X = column) */
#define TURBO_HEAT_FRAC 0x098F  /* the turbo heat's fraction (ENGINE_HEAT is the whole part) */
#define PACE_FRAMES     0x0995  /* frames counted for the pace band */
#define PACE_DIST       0x0996  /* distance counted for the pace band (every $3C picks one) */
#define GROUND_LINE     0x0CD5  /* the ground line the riders' heights are measured from ($A0) */
#define COLOUR_CYCLE    0x0880  /* the colour cycle's index into its list ($FF stopped) */

/* ---- direct page, part 3 ---- */
#define RIDER_HOLD      0x003D  /* [+$8] X = rider: frames the rider is held (start-line delay X * 8 + $1C, $0A after a crash) */
#define PIC_SCROLL      0x00A6  /* word: the title and credits picture's scroll (also the intro zoom to $0100) */
#define CLOCK_HIDE      0x00CB  /* the race clock is not redrawn (the best-time blink) */
#define PLACE_BONUS     0x00BA  /* the place bonus, counted down into the score on the results */
#define COIN_TAKEN      0x00B2  /* a coin was taken: redraw the map cells behind it */
#define WAVE_FRAME      0x00CE  /* the waving player's pose frame on the jump result board */

/* ---- race globals, part 2 ---- */
#define RIDER_PIECE_LEAN 0x08B4  /* [+$8] X = rider: the lean the piece under it asks for (0 while landing) */
#define RIDER_LANDED    0x08AC  /* [+$8] X = rider: settled after a landing (2 after a crash landing) */
#define PLAYER_GROUND   0x09C4  /* the track height under the player (follows TRACK_ROWS' height row) */
#define COURSE_ENDED    0x099C  /* the course's end tile was reached (once) */
#define BUILD_COLUMN    0x09C3  /* the next column of the course's lane data to build */
#define DEMO_INDEX      0x0CE1  /* the attract demo's input script position */
#define DEMO_TIME       0x0CE3  /* frames left on the demo input entry ($F8 or more ends the demo) */
#define ENTRY_CODE      0x0850  /* [+$C] the end screen entry code (packed digits, flags and check bytes up to $085B) */

/* ---- $15xx-$1Exx globals ---- */
#define SPRITE_FRAME    0x1961  /* [+$4] X = rider sprite slot (0-3): the animation frame it shows */
#define PLAYER_CHAR     0x154F  /* the player's character (roster index) */
#define RACE_VARIANT    0x1E91  /* the race variant picked at random on the player select (rider sets, palettes) */
#define ANIM_UPLOAD_ON  0x1E0C  /* the NMI uploads the animation strip at $1E0A */
#define COIN_COMBO      0x1540  /* coins in a row (up to 5): adds the combo bonus */
#define MARKER_COUNT    0x1541  /* coin markers started (up to 5) */
#define COIN_TOTAL      0x1542  /* coins collected (the 100 and 200 milestones) */
#define ENTRY_DIGITS    0x1570  /* [+$3] 3 bytes: the three digits entered on the end screen */
#define SELECT_CHOSEN   0x198B  /* player select: the choice is made */
#define SELECT_SLIDING  0x198D  /* player select: the portrait transition is running */
#define SELECT_ROW      0x198E  /* player select: the row the front portrait slides to */
#define BONUS_END       0x1990  /* the bonus course ended (1 the track's end, 2 the time ran out) */
#define JUMP_RUN_STEP   0x197F  /* jump contest: the run's step (2 = riding) */
#define JUMP_RUN_FRAMES 0x1981  /* word: jump contest: frames into the run (picks the pose and the band) */

/* ---- effect slots (sparks, dust, confetti; Y = slot 0/2/4/6) and the race clock ---- */
#define EFFECT_X        0x0106  /* [+$8] Y = slot: X position */
#define EFFECT_Y        0x010E  /* [+$8] Y = slot: Y position */
#define EFFECT_KIND     0x0116  /* [+$8] Y = slot: kind or step (0 = free) */
#define EFFECT_FRAME    0x011E  /* [+$8] Y = slot: animation frame or tile */
#define EFFECT_TIMER    0x0126  /* [+$8] Y = slot: frames until the next step */
#define RACE_MIN        0x03AE  /* the race clock: minutes */
#define RACE_SEC        0x03AF  /* seconds */
#define RACE_FRAMES     0x03B0  /* frames (shown as hundredths) */
#define FINISH_CROSSED  0x03AC  /* the player crossed the finish (once per lap) */
#define ROW_TYPE        0x027D  /* the row type of the course entry being built (9 = the finish row); the bonus builders use it as their course index */
#define TRACK_READ_POS  0x027B  /* word: the builder's read position in TRACK_BUF (a little ahead of the player) */

/* ---- more globals ---- */
#define RIDER_PIECE_ACTION 0x0226  /* [+$8] X = rider: the pending piece action (PieceAction runs it) */
#define EFFECT_STEP     0x012E  /* [+$8] Y = slot: the effect's step or second X */
#define LAPS_LEFT       0x093F  /* laps left (one off at each lap; nonzero at the finish picks the sound) */
#define FINISH_POSE     0x091C  /* the player's walk or wave pose at the finish (0, 1, 3, 4) */
#define SELECT_TURN     0x1559  /* player select: the carousel's turn step (+2 / -2, 0 still) */
#define SCORE_COPY      0x1549  /* [+$6] 6 digits: the race score kept for the results subtraction */

/* ---- more effect and rider fields ---- */
#define EFFECT_VX       0x013E  /* [+$8] Y = slot: horizontal speed */
#define EFFECT_VY       0x0146  /* [+$8] Y = slot: vertical speed */

/* ---- sprites ---- */
#define OAM_BUF         0x1600  /* [+$200] $0200 bytes: the OAM low table image */
#define OAM_SIZE_BUF    0x1820  /* [+$80] 128 size bytes, packed into the high table */
#define SPRITES_FROZEN  0x1958  /* leave the sprites where they are */

/* ---- objects (the jump contest's and the bonus screens' actors):
 * 16-byte arrays of 8 word slots, index X = 2 * slot (14-0); SpawnObject
 * fills a free one ---- */
#define OBJ_X           0x027F  /* [+$10] word: X position */
#define OBJ_Y           0x028F  /* [+$10] word: Y position */
#define OBJ_VX          0x029F  /* [+$10] word: horizontal velocity */
#define OBJ_VY          0x02AF  /* [+$10] word: vertical velocity */
#define OBJ_ACTIVE      0x02EF  /* [+$10] slot in use */
#define OBJ_TYPE        0x02FF  /* [+$10] type: picks its init routine and script */
#define OBJ_STATE       0x030F  /* [+$10] */
#define OBJ_STEP        0x031F  /* [+$10] */
#define OBJ_RANDOM      0x032F  /* [+$10] a random count */
#define OBJ_FRAME       0x034F  /* [+$10] */
#define OBJ_DELAY       0x036F  /* [+$10] */
#define OBJ_COUNTER     0x037F  /* [+$10] */
#define OBJ_SCRIPT      0x038F  /* [+$10] word: the type's script */
#define OBJ_TARGET_X    0x039F  /* word: the point an object steers to */
#define OBJ_TARGET_Y    0x03A1
#define OBJ_CURRENT     0x03A3  /* the slot being run */

#endif
