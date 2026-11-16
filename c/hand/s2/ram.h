/*
 * ram.h - Stadium 2's (Gen A) work RAM by name, for the address operands of
 * the hand-written C: rd8(DP(GAME_MODE)), wr8(ABSX(RIDER_SPEED_HI), A).
 * The names are applied by tools/ramname.py (which also writes them back as
 * hex for tools/port_hand.py); an address without a name here stays hex.
 * Only addresses whose use is understood are named; the meaning of each is
 * taken from the routines that read and write it (see their comments).
 *
 * The same names as c/hand/s4/ram.h: the addresses were paired through
 * the routines the two engines share (Gen A's rider arrays sit 6 bytes
 * below Gen B's).
 *
 * The layout is Stadium 1's (c/hand/s1/ram.h): the names that the routines
 * shared by the two dumps use sit at the same addresses in both.
 *
 * Direct-page names apply where D = 0 (the game runs with D = 0 except in a
 * few DMA and register routines, which ramname.py leaves alone).
 */
#ifndef HAND_S2_RAM_H
#define HAND_S2_RAM_H

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
#define GAME_MODE       0x0076  /* index of the mode's script ($80:B65D) */
#define MODE_STEP       0x0077  /* position in the mode's handler list */
#define COURSE_SLOT     0x007C  /* course counter in the broadcast schedule */
#define FINAL_RACE      0x007D  /* the final of the course (second race) */
#define RIDER_TURN      0x007F  /* [+$8] frame & 7: the rider whose turn it is */
#define MODE_TIMER      0x0088  /* word: the current mode's frame timer */
#define MODE_HOLD       0x0050  /* RunGameModeScript: setup waits while set */
#define RACE_STARTED    0x008E  /* the run has begun (countdown over) */
#define SCROLL_X        0x0094  /* word: the track's scroll position */
#define CLOCK_MINUTE    0x00AF  /* broadcast minute, copied from $7F:FFFF */
#define COUNTDOWN       0x00BB  /* the start countdown's step (1-4) */
#define NMI_FRAME_READY 0x081A  /* set by the NMI: the main loop runs a frame */
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
#define RIDER_SCRIPT    0x0170  /* [+$8] running piece action (0 none; $14/$15 jump landing, $18 boost ground) */
#define RIDER_INPUT     0x0178  /* [+$8] d-pad and buttons this frame (bit 6 turbo); CPU and demo riders write it */
#define RIDER_STEP      0x0180  /* [+$8] pixels moved this frame (speed high + fraction carry) */
#define RIDER_WHEEL     0x0188  /* [+$8] wheel animation counter (counts down by the step) */
#define RIDER_X         0x0190  /* [+$8] track position, low byte */
#define RIDER_PAGE      0x0198  /* [+$8] track position, high byte (screen page) */
#define RIDER_POSE      0x01A0  /* [+$8] pose for the lean table ($80:BA92) */
#define RIDER_STEER     0x01A8  /* [+$8] lane steering +1 / -1 ($FF) */
#define RIDER_Y         0x01B0  /* [+$8] screen height */
#define RIDER_SPEED_LO  0x01B8  /* [+$8] speed, fraction */
#define RIDER_SPEED_HI  0x01C0  /* [+$8] speed, pixels per frame */
#define RIDER_BUMPED    0x01C8  /* [+$8] knocked back by a rear bump ($FF) */
#define RIDER_TIMER     0x01D0  /* [+$8] pose / wave timer at the start and finish */
#define RIDER_ACTIVE    0x01D8  /* [+$8] racing on screen */
#define RIDER_LEAN      0x01E0  /* [+$8] lean angle (6 = upright) */
#define RIDER_AIR       0x01E8  /* [+$8] in the air (0 = on the ground) */
#define RIDER_LANE      0x01F0  /* [+$8] lane position (sorted for the ranking) */
#define RIDER_HEIGHT    0x01F8  /* [+$8] jump height */
#define RIDER_LANDING   0x0200  /* [+$8] landing kind (2 = flat) */
#define RIDER_JUMP      0x0208  /* [+$8] jump state (a rival placed mid-jump: 1 or 3) */
#define RIDER_TILE_Y    0x0210  /* [+$8] height of the track piece under the wheel */
#define RIDER_TILE      0x0218  /* [+$8] type of the track piece under the wheel */
#define RIDER_TARGET_LO 0x0228  /* [+$8] CPU rider's target speed, fraction */
#define RIDER_TARGET_HI 0x0230  /* [+$8] CPU rider's target speed, high */
#define RIDER_SLOW_AT   0x0240  /* [+$8] speed threshold for the slow mark */
#define RIDER_DISTANCE  0x0248  /* [+$8] distance into the piece script */
#define RIDER_COLUMN    0x0260  /* [+$8] track column under the rider (0-63) */
#define RIDER_JUMP_BASE 0x0268  /* [+$8] jump height base */
#define RIDER_ON_RAMP   0x08BC  /* [+$8] 2 = on a ramp */
#define RIDER_SLOW      0x08F4  /* [+$8] marked slow (below RIDER_SLOW_AT) */
#define RIDER_RUNOUT    0x090C  /* [+$8] run-out column past the finish */
#define RIDER_BAND      0x0958  /* [+$8] which band of the track the rider is in */
#define RIDER_WANT_LEAN 0x0971  /* [+$8] CPU rider's wanted lean */
#define RIDER_LANE_NUM  0x08A4  /* [+$8] lane 5..1 from the lane position */

#define RIDER_DIST      0x085E  /* [+$10] words, 2X: distance to the player */
#define RANK_ORDER      0x0950  /* [+$8] 8 bytes: rider numbers by lane position */
#define ENGINE_HEAT     0x0990  /* the player's engine temperature */

/* ---- timers, scrolling, the HUD ---- */
#define TIMER_A         0x008A  /* a mode's second timer (the start countdown's beeps) */
#define SCROLL_HALF     0x008C  /* BG1 nametable half, flips on each scroll carry */
#define BG_SCROLL       0x0092  /* BG1 horizontal scroll (+= the player's step) */
#define SCROLL_PAGE     0x0095  /* scroll pages passed (high byte of SCROLL_X) */
#define OVERHEATED      0x0060  /* the engine is overheated (odd) */
#define PLAYER_PLACE    0x086E  /* the player's place (8 minus the riders beaten) */
#define COIN_COUNT      0x0CDF  /* coins collected (5 win the turbo) */
#define TURBO_ON        0x0CE0  /* the coin turbo is running */
#define DIGIT_BUF       0x03D2  /* number staging: the size byte, then two rows of tiles */
#define DIGIT_ROW       0x03D3  /* [+$20] the upper row of DIGIT_BUF (the lower row is +$10) */
#define TRACK_ROW_PTR   0x0270  /* the track column's VRAM row pointer */
#define TRACK_VRAM_COL  0x0271  /* the track column's VRAM column (wraps at $20) */
#define TRACK_VRAM_HALF 0x0272  /* the track column's nametable half (^ 4) */
#define END_STEP        0x0453  /* end screen: its step */
#define ENTRY_CURSOR    0x0452  /* end screen: the digit being entered */

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
#define COURSE_COLUMN   0x00A6  /* the course column being loaded into the track */

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
#define PIC_SCROLL      0x009E  /* word: the title and credits picture's scroll (also the intro zoom to $0100) */
#define PLACE_BONUS     0x00B2  /* the place bonus, counted down into the score on the results */

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
#define PLAYER_CHAR     0x1542  /* the player's character (roster index) */
#define ANIM_UPLOAD_ON  0x1E0C  /* the NMI uploads the animation strip at $1E0A */
#define COIN_COMBO      0x1540  /* coins in a row (up to 5): adds the combo bonus */
#define MARKER_COUNT    0x1541  /* coin markers started (up to 5) */

/* ---- effect slots (sparks, dust, confetti; Y = slot 0/2/4/6) and the race clock ---- */
#define EFFECT_X        0x0100  /* [+$8] Y = slot: X position */
#define EFFECT_Y        0x0108  /* [+$8] Y = slot: Y position */
#define EFFECT_KIND     0x0110  /* [+$8] Y = slot: kind or step (0 = free) */
#define EFFECT_FRAME    0x0118  /* [+$8] Y = slot: animation frame or tile */
#define EFFECT_TIMER    0x0120  /* [+$8] Y = slot: frames until the next step */
#define RACE_MIN        0x03B1  /* the race clock: minutes */
#define RACE_SEC        0x03B2  /* seconds */
#define RACE_FRAMES     0x03B3  /* frames (shown as hundredths) */
#define FINISH_CROSSED  0x03AF  /* the player crossed the finish (once per lap) */
#define ROW_TYPE        0x0277  /* the row type of the course entry being built (9 = the finish row); the bonus builders use it as their course index */
#define TRACK_READ_POS  0x0275  /* word: the builder's read position in TRACK_BUF (a little ahead of the player) */

/* ---- more globals ---- */
#define RIDER_PIECE_ACTION 0x0220  /* [+$8] X = rider: the pending piece action (PieceAction runs it) */
#define EFFECT_STEP     0x0128  /* [+$8] Y = slot: the effect's step or second X */
#define LAPS_LEFT       0x093F  /* laps left (one off at each lap; nonzero at the finish picks the sound) */
#define FINISH_POSE     0x091C  /* the player's walk or wave pose at the finish (0, 1, 3, 4) */
#define SELECT_TURN     0x154B  /* player select: the carousel's turn step (+2 / -2, 0 still) */

/* ---- more effect and rider fields ---- */
#define EFFECT_VX       0x0138  /* [+$8] Y = slot: horizontal speed */
#define EFFECT_VY       0x0140  /* [+$8] Y = slot: vertical speed */

/* ---- riders' height motion, crash and ball ---- */
#define RIDER_COUNTDOWN 0x08CC  /* [+$8] X = rider: a timer counted down every frame (bounce, overheat end) */
#define RIDER_HEIGHT_LO 0x08E4  /* [+$8] X = rider: height motion, fraction ($08EC:$08E4 += RIDER_FALL_SPEED) */
#define RIDER_HEIGHT_HI 0x08EC  /* [+$8] X = rider: height motion, whole part */
#define RIDER_FALL_SPEED 0x08FC  /* [+$8] X = rider: the height step added every frame */
#define RIDER_BOUNCE_LO 0x08D4  /* [+$8] X = rider: the crash bounce speed, fraction (copied to RIDER_HEIGHT_LO) */
#define RIDER_BOUNCE_HI 0x08DC  /* [+$8] X = rider: the crash bounce speed, whole part */
#define CRASH_PHASE     0x0998  /* crash recovery runs every 4 frames on this countdown */
#define TURBO_WARN      0x00A2  /* the overheat warning sound is playing */
#define COLOUR_TIMER    0x0882  /* frames to the colour cycle's next step (14) */
#define BAR_LENGTH      0x087E  /* the credits bar's length (to 15) */
#define BALL_LANE       0x097C  /* the ball's lane position (the player uses it when BALL_OUT) */
#define BALL_OUT        0x08C7  /* the ball is out on the track */

/* ---- score, coin markers, records ---- */
#define SCORE_DIGITS    0x0CE5  /* [+$6] the score, six decimal digits (index 5 = units) */
#define SCORE_ADD       0x09AC  /* [+$6] six digits waiting to be added to the score (AddScore clears them) */
#define MARKER_X        0x1500  /* [+$8] X = 2 * coin marker: X position (word with MARKER_X + 1) */
#define MARKER_Y        0x1508  /* [+$8] X = 2 * marker: Y position */
#define MARKER_VX       0x1510  /* [+$8] X = 2 * marker: velocity */
#define MARKER_VY       0x1518  /* [+$8] X = 2 * marker: vertical speed, +1 a frame (Gen B retires the marker once it turns positive) */
#define MARKER_FRAC     0x1520  /* [+$8] X = 2 * marker: position fraction */
#define MARKER_ON       0x1530  /* [+$8] X = 2 * marker: in use */
#define MARKER_AGE      0x1538  /* [+$8] X = 2 * marker: word timer, counts up (the oldest is reused) */
#define PIECES_AHEAD    0x0884  /* [+$20] 4 per rider (0884 + 4X): the heights of the four pieces ahead */
#define SCROLL_FRAC     0x0994  /* the scroll's fraction (the player's speed * 16 adds into it) */
#define DEMO_PAD        0x098E  /* the attract demo's pad input for the player this frame */
#define RECORD_TIME     0x083A  /* [+$3] the course record (minutes, seconds, hundredths) */
#define RECORD_COURSE   0x084C  /* the course the record belongs to (course - 1; set means a record to show) */

/* ---- bonus tries ---- */
#define BONUS_TRIES     0x1E87  /* bonus tries banked: a win without rivals adds one, a bonus run takes 10 */
#define BONUS_BEST      0x1E88  /* the best bonus count */

/* ---- Gen A player select and end screen ---- */
#define SELECT_WHEELS   0x1547  /* [+$4] the four select wheels' angles */
#define SELECT_DELAY    0x1554  /* frames left after the choice before the next segment */
#define SELECT_STEP     0x1E8B  /* the player select's step (2 = the second screen, with Wario) */
#define END_SCROLL      0x009A  /* the end screen's BG X scroll (moves in 2 a frame) */

/* ---- Gen A entry and coins ---- */
#define ENTRY_DIGITS    0x03A6  /* [+$4] the four digits entered on the end screen */
#define COIN_TAKEN      0x00AA  /* a coin was taken: its type, erased from the track next */

/* ---- direct page, part 4 ---- */
#define START_STATE     0x003C  /* the start sequence: 1 = begin the countdown, 2 = running ($B5 when $54 is set) */
#define RIDER_DONE      0x0056  /* [+$8] X = rider: stopped after the finish (3); skips the start walk while set */
#define FLASH_TIMER     0x005E  /* the item-hit / finish flash countdown ($1D at the finish) */
#define TURN_LATCH      0x0079  /* the player's last A/B bits at the finish (a new press speeds the run-out); 1 at a mode start */
#define VSCROLL_ZOOM    0x0090  /* the NMI takes BG1's vertical scroll from $9E/$9F (else the wobble) */

/* ---- race, finish and end screen ---- */
#define ROW_OFFSET      0x0278  /* the builder's offset in the current row's column list (0 = take the next course entry) */
#define PLAYER_LIFT     0x09C7  /* the player's landing lift (subtracted in HeightAbove; cleared on landing) */
#define PLAYER_TUMBLE   0x0999  /* the crashed player is tumbling ($FF stopped dead, 1 the tumble ended) */
#define START_POLES     0x0936  /* [+$8] the eight riders' start lanes, also the start-line flag poles' X */
#define LAP_SIGN        0x0929  /* which digit of the LAP sign the builder writes next */
#define RIDER_FINISHED  0x08C4  /* [+$8] X = rider: crossed the finish (the run-out and the wave) */
#define NEW_TIME_RECORD 0x085C  /* the broadcast's total time is a new record (saved) */
#define NEW_SCORE_RECORD 0x085D  /* the score is a new record (saved) */
#define END_NEXT_STEP   0x0454  /* end screen: the step to go to after the current one */
#define BALL_DUE        0x092C  /* the ball is due: rider 7's slot launches it once off screen */
#define COURSE_TIMES    0x081C  /* [+$1E] ten 3-byte times (minutes, seconds, hundredths), $FF ends */

/* ---- sprites ---- */
#define OAM_BUF         0x1600  /* [+$200] $0200 bytes: the OAM low table image */
#define OAM_SIZE_BUF    0x1820  /* [+$80] 128 size bytes, packed into the high table */
#define SPRITES_FROZEN  0x1958  /* leave the sprites where they are */

/* ---- objects (the jump contest's and the bonus screens' actors):
 * 16-byte arrays of 8 word slots, index X = 2 * slot (14-0); SpawnObject
 * fills a free one ---- */
#define OBJ_X           0x0279  /* [+$10] word: X position */
#define OBJ_Y           0x0289  /* [+$10] word: Y position */
#define OBJ_VX          0x0299  /* [+$10] word: horizontal velocity */
#define OBJ_VY          0x02A9  /* [+$10] word: vertical velocity */
#define OBJ_ACTIVE      0x02E9  /* [+$10] slot in use */
#define OBJ_TYPE        0x02F9  /* [+$10] type: picks its init routine and script */
#define OBJ_STATE       0x0309  /* [+$10] */
#define OBJ_STEP        0x0319  /* [+$10] */
#define OBJ_RANDOM      0x0329  /* [+$10] a random count */
#define OBJ_FRAME       0x0349  /* [+$10] */
#define OBJ_DELAY       0x0369  /* [+$10] */
#define OBJ_COUNTER     0x0379  /* [+$10] */
#define OBJ_SCRIPT      0x0389  /* [+$10] word: the type's script */
#define OBJ_TARGET_X    0x0399  /* word: the point an object steers to */
#define OBJ_TARGET_Y    0x039B
#define OBJ_CURRENT     0x039D  /* the slot being run */

#endif
