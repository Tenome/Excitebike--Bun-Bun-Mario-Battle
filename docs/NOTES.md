# Research notes

Addresses are Stadium 1 (Gen A) unless stated otherwise.

## ROM layout
* 1 MB LoROM. Code and data are addressed at `$80-$9F:8000-FFFF`.
* BS header at `$80:FFB0`. Maker `01`. The title is half-width katakana
  "EXｴｷｻｲﾄﾊﾞｲｸ ﾌﾞﾝﾌﾞﾝﾏﾘｵ1" plus the Stadium digit. Map byte `$30`, type `$00`.
  The date bytes at `$FFD6` are the only difference between same-content dumps.
* Native vectors: COP and BRK → `$FFA0` (`RTI`). NMI → `$FFA1`
  (`JML $80:85FF`). IRQ → `$FFA9` (`JML $80:BBDA`). Reset → `$FFA5`
  (`JML $80:8000`). Gen B: NMI `$80:8256`, IRQ `$80:B898`.

## Engine generations
| | Code | Banks | Functions found by bsdis (first pass) |
|---|---|---|---|
| Gen A: S1, S2 | ~28.4 KB | `$80-$81` | 407 (current) |
| Gen B: S3, S4 | ~37.3 KB | `$80-$82` | 532 (current) |

S1 and S2 code is the same apart from a small insertion that shifts later
addresses. S3 and S4 also share their code. Most of the byte differences are
data.

Function matching (`tools/xmatch.py`, `tools/fdiff.py`):

| Pair | Same | Changed | Only in A | Only in B |
|---|---|---|---|---|
| S1 ↔ S2 | 387 | 18 | 2 | 1 |
| S3 ↔ S4 | 507 | 23 | 2 | 2 |
| S2 ↔ S3 | 106 | 204 | 96 | 222 |

S1 ↔ S2: apart from address shifts, the real changes are two end-screen menu
handlers (`$80:9AF9`, `$80:9B4C`: extra sound and redraw calls) and a few
constants (for example `$80:9E46` timer `$0708` → `$0E10`). The episodes differ
in data, not in engine. Gen B (S3/S4) is a substantial rewrite with a third
code bank.

## Satellaview (BS-X) usage
* The only receiver library is `$81:F80C-$81:FAB2`. It uses `$2188-$2197`.
* `$81:FA68` `BsxDetect` → `$7FFFE1` (1 = no receiver).
* `$81:F80C` `BsxOpenTimeChannel` (only runs when a receiver is found).
* `$81:F8BB` is the per-frame poll of the stream (`$2190`/`$2192`/`$2193`),
  called from `$80:810A`. It copies the satellite clock (minute into `$7FFFFF`,
  hour into `$7FFFF9`, ...) and, with no data, ticks the clock itself
  (`$81:F959`, see "Broadcast clock").
* `$80:A34B` / `$80:A387` bit-bang `$2198` (clock/data lines, MSB first):
  reads 16 words from the base unit's serial EEPROM into RAM, called from the
  `$80:A0BC` tree (only when a receiver is present). Probably the player's
  Satellaview profile (name), used for display.
* `$7F:FFE0-$7F:FFFF` is a handshake area: `$FFE1`, `$FFE3/4`, `$FFF7/8`
  (channel id), `$FFF9-$FFFB`, `$FFFF`.

## Kernel routines
| Address | Name | Notes |
|---|---|---|
| `$80:8000` | Reset | |
| `$80:8369` | InitPpuRegisters | |
| `$80:8539` | DmaKick | Called with D=`$4300`, so `STA $02` writes A1T0. |
| `$80:8541` | DmaCopyToWram | DMA through `$2180`. |
| `$80:8562` | DmaFillZero | Mode 7 multiply 1×0 → DMA from `$2134`. |
| `$80:85FF` | Nmi | Shadow PPU registers at `$1903-$190F`. Scroll registers at DP `$8C`, `$90-$9F`. |
| `$80:876B` | NmiVramUpload | Queued VRAM DMA. |
| `$80:8979` | NmiOamUpload | |
| `$80:89ED` | RunGameModeScript | Pointer table `$80:B65D` (19 modes) and a two-level script table `$80:B683`. |
| `$80:BBDA` | Irq | H/V IRQ at line 6 (`VTIME`): HUD split. |

## RAM so far
* DP `$75` / `$76` / `$77`: script mode flag, mode index, script step.
* `$081A`: skip NMI uploads.
* `$1903-$190F`: PPU shadow registers (INIDISP, TM, TS, TMW, TSW, CGWSEL,
  CGADSUB, NMITIMEN, HDMAEN).
* `$1E12`, `$1543`: VRAM upload source.

## Each build holds two Stadiums
The course loader (`$81:A01B` Gen A, `$81:A11E` area Gen B) indexes
`($81:800C * 4 + course) * 2` into a start table and a length table, then
expands the course bytes into the track buffer at `$7F:4000`:

| Gen | starts | lengths | data | courses |
|---|---|---|---|---|
| A (S1/S2) | `$81:8C59` | `$81:8C6D` | `$81:8C81` | 8 (+2 special, picked when `$0CD7` != 0) |
| B (S3/S4) | `$81:8C62` | `$81:8C76` | `$81:8C8A` | 8 (+2 special) |

The course data is byte-identical in S1 and S2, and in S3, S4 and the S3
rebroadcast. So `$81:800C` selects the Stadium: Gen A 0 = Stadium 1,
1 = Stadium 2; Gen B 0 = Stadium 3, 1 = Stadium 4. The 1998 rebroadcast dumps
are one build per generation with this byte flipped (S2 7-6 = S1 build + 1;
S3 2-8 = S4 build + 0). The S1 (1998) and S4 dumps together hold all four
Stadiums. The port selects the Stadium with `BUNBUN_STADIUM=1|2`
(`port/src/game_rtl.c`, patches the byte in memory after verification);
verified: Stadium 2 courses appear in the S1 build.

## A cross-episode unlock in BS-X memory
`$80:916B` (a setup step of the player select, mode 1) counts how many of the
seven bytes at `$11:5719-$571F` are non-zero. On a Satellaview that address
is BS-X unit memory; here it reads open bus. With two or more set, or in
Stadium 2 (`$81:800C` = 1), it sets `$1E8B` = 2, which picks the second
player-select handler (`$80:8DE6` instead of `$80:8D2F`, table `$80:8D2B`).
Confirmed in the port: the second variant adds Wario to the wheel (Mario,
Luigi, Toad, Wario; Stadium 1 normally has three). `BUNBUN_UNLOCK=1` grants
it at the frame boundary when mode 1 starts (`port/src/game_rtl.c`), and
Wario races in Stadium 1.

## BS-X player data the game reads (`$11:5716-$5734`)
Both generations read a block the BS-X BIOS maintains (open bus without it):
* `$11:5719-$571F`: episode flags. Gen A: two or more set → Wario unlock
  (above); `$80:A41C` also ORs pairs of them into the end-screen digits
  `$0856-$0858`.
* Gen B `$80:A070`: `$11:5730-$5734` combined as a 5-digit number (weights
  10000, 1000, 100, 10, 1, via `$80:B98E`) into `$0850-$0855`.
* Gen B `$80:A129`: `$11:572C-$572E` each split into tens and units into
  `$0854-$0859` (looks like a date: year, month, day).
A port could fill this block from a save file to recreate the player's BS-X
profile.

## Race state layout (first look)
Like the NES Excitebike, state is kept in parallel arrays indexed by X:
* `$0200-$026F`: 14 fields, stride 8, X = rider 0..7 (races have 8 riders).
* `$0279-$0389`: 18 fields, stride 16, X = object slot 0..15. The object
  update in bank `$81` dispatches on type through `JSR ($AAD1,X)` and friends
  (`$81:970D/973C/9748`).
* `$7F:4000`: expanded course buffer (from `$81:A01B`), `$7F:6000` column index.
* `$08A4`: current course (0..3); `$7C`: course counter in the schedule.

### Course format
`tools/course.py` decodes every course. One byte per track column:

* bit `RUN` set (`$40` in Gen A, `$80` in Gen B): `[b][n]`, `b` repeated `n`
  columns. The plain ground code is `$40` (A) / `$80` (B); other run codes are
  terrain variants (A `$41-$4C`, B `$81-$8A`).
* `$09`: also stamps five more `$09` ahead (a wide element; later bytes overwrite).
* `$00-$3F` otherwise: one obstacle or event column. These codes are shared by
  both generations.

Courses expand to 315-393 columns (Gen A) and 338-416 (Gen B). The second
special course is identical in both generations except for the ground code, so
Gen A courses should convert to the Gen B engine by moving `$40-$7F` to
`$80-$BF`. That is the cheapest route to running all 16 courses in one engine.
Gen B also randomises some columns at race start (`$81:A18B`, mode 3).

## Build stamp and debug flags (`$81:8000`)
* `$81:8000-$8005`: BCD build timestamp YY MM DD hh mm ss.
  S1 dump `98-03-23 13:40:41`, S2 `97-05-02 17:37:15`, S3 `97-10-23 20:53:10`,
  S4 `98-03-19 15:49:25`. The S1 dump is a 1998 rebuild, not the May 1997 original.
* `$81:800C`: Stadium selector (see above). 0 in S1/S3, 1 in S2/S4/S2-rebroadcast.
* `$81:800E`: debug flag, read by the main loop (`$80:8116`). When set: P2 A/X
  step `$7FFFFF` down/up, P1 L toggles pause (`$1917`), P1 R advances one frame.

## Main loop (`$80:80FE`)
Wait for NMI (`$081A`), poll BS stream, `$80:B4AE`, `ReadJoypads`, optional debug
keys, then (unless paused) `$80:B498`, `$80:823C`, `$80:B226`,
`RunGameModeScript`, scroll helpers when `$90` is set, `$80:B538`, `$80:85A6`
(joypad consumers), `$80:819F`, clear `$081A`.

## Broadcast clock and schedule
The game is one broadcast hour. `$7FFFF9` = hour, `$7FFFFF` = minute (0..59),
`$7FFFFA` (16-bit) = frame counter within the minute. With a receiver,
`BsxPollStream` copies the satellite clock into `$7FFFF0-$7FFFF3/$F9/$FF`.
Without one (`$81:F959`) `$7FFFFA` counts frames and every $0E4C (3660) frames
the minute goes up by one. Reset seeds hour 19, minute 2.

* minute < 3/4: intro / rules text ("はじめに"), title with "しばらくおまちください".
* minute 4: the race segment opens (modes `$0C`/`$0E` check `$7FFFFF >= 4`).
* During a race (mode 3) a change of minute matched against `$80:82A7`
  (minutes 8, 9, 20, 21, 32, 33, 44, 45) drops an item onto the track
  (`$80:823C` BroadcastEventInject): the course's item code from `$80:82AF`
  ($2A-$2D) goes on the next plain-ground column ahead of the player
  (`$0275` + 3), never right after obstacles $17/$19/$1B/$03/$04. These are the
  broadcast's timed bonus items.
* minute 60 (`$3C`): end of broadcast. Unless hour is `$11`, the game writes
  `$80` to BS-X register `$2197`, which on hardware hands control back to the
  BIOS.
* `$80:9DE0` (end screen, Start pressed): in Stadium 1 (`$81:800C` = 0) the
  game maps the BIOS back in (BS-X MMC writes to `$03/$04/$07/$08:5000` = 0,
  `$0E:5000` = `$80`) and does `JML $10:5C3C` into the BS-X BIOS. With no BIOS
  this runs into garbage, so a port must replace it (return to title or a
  Stadium select). Stadium 2 (`$81:800C` = 1) goes to mode `$12` instead.

Schedule tables (Stadium 1), indexed by the course number `$7C` (0..3):

| Table | Values | Meaning |
|---|---|---|
| `$80:9727` | 11, 23, 35, 47 | free run ends (minute >= value → mode 2, next segment) |
| `$80:A5A8` | 18, 30, 42, 54 | final ends; after course 4 → mode 9 |
| `$80:E023` | 57 (`$39`) | → mode `$10` (ending) |

Seen on screen (port build, one boot per minute, `tools/minuteshots.sh`):

| Minutes | Segment |
|---|---|
| 0–2 | intro / rules text ("はじめに") |
| 3 | title, "しばらくおまちください" |
| 4–5 | practice with the controls overlay (Y / B) |
| 6, 17, 29, ... | "コース N フリー走行" card (2 min) |
| 7–10, 19–22, ... | free run (4 min) |
| 11–12, 23–24, ... | "コース N 決勝" card (2 min) |
| 13–16, 25–28, ... | final race (4 min) |
| 41–52 | course 4 (same pattern) |
| 53 | "Congratulations" podium (results) |
| 54–59 | bonus game: night stage with a "TIME" counter |
| 57 | end screen check (`$80:E023`, mode `$10`) |
| 60 | end of broadcast |

The 2-minute cards are where the SoundLink narration played.

Mode `$12` (`$80:9F68`) is a staff credits roll (cast: ピーチ姫, ワリオ, ...; then
St.GIGA staff, game staff, sound, programmers). Stadium 1 never shows it
because its end screen jumps into the BIOS first. The port's replacement for
`$80:9DE0` now reaches it. The credits' own Start handler (`$80:9FE0`) jumps
into the BIOS the same way; the port replaces it (`hle_func` at the mid-function
address, which snesrecomp treats as a hard boundary) with a console reset at
the next frame boundary (`g_port_reset_request` in `port/src/game_rtl.c`).
Verified: results → credits → final illustration → Start → intro with artwork.

Coverage runs can tune in at any minute by poking `$7F:FFFF` (script
`poke 1FFFF NN`); this is what the debug keys at `$81:800E` do too.

## Dispatch idioms the disassembler recognises
* `LDA tbl,Y / STA $00 / INY / LDA tbl,Y / STA $01 / JMP ($0000)` (word table)
* two-level: outer table of list pointers, `LDA ($02),Y` steps the list (`$80:8A2B`)
* split tables: low bytes at `tbl`, high bytes at `tbl+n` (`$80:BE3B`)
* `JMP (tbl,X)` / `JSR (tbl,X)`, tables end where the first handler starts

## Coverage status (Stadium 1)
Static map: 407 functions, 12,688 instructions. Runtime capture (8 random
sessions + a 60-minute sweep, about 1.1 M frames) executed 10,576 of them
(83.4%) and found **no** executed instruction outside the static map, apart
from the deliberate jump into the BS-X BIOS.

## Open questions
* What do the remaining 16.6% of static instructions belong to (2-player
  paths, bonus games, error paths)?

## snesrecomp integration notes
* Every `JMP ($0000)` dispatcher must be declared to the analyzer, otherwise it
  is an "unresolved IndirectGoto" and runs on the interpreter (which also
  bypasses `hle_func` replacements). `tools/mkcfg.py --into` now writes
  `indirect_dispatch <site> <n> ptrtail targets:...` for every table bsdis
  resolved, and `idx:X` for `JMP/JSR (abs,X)`. After this, 572 of 574 variants
  are native.
* MSVC treats `_ptr32`/`_ptr64` as keywords; the v2 emitter uses those names for
  temporaries. `port/CMakeLists.txt` renames them with compile definitions.
  (Upstream bug worth reporting.)

## Stadium 4 (Gen B) broadcast hour
Seen on screen (`tools/minuteshots.sh` on the interpreter-only S4 build):

| Minutes | Segment |
|---|---|
| 0–2 | intro text (now lists four event types: free run, final, jump contest, bonus game) |
| 3 | title "スタジアム4", "しばらくおまちください" |
| 4–5 | practice with the controls overlay |
| 6 | PLAYER SELECT ("SELECT YOUR FAVORITE PLAYER!", B to cancel) |
| 7–10 | course 1 free run, new "COURSE MAP" overview screen |
| 11 / 12–15 | course 1 final card / final race (HUD adds SPEED km/h) |
| 16 | JUMP CONTEST rules ("press any button to start") |
| 17–21 / 22 / 23–26 | course 2 free run / final card / final |
| 27 | jump contest |
| 28 / 29–30 | BONUS GAME rules (jump to collect Boos) / bonus game, TIME 30 |
| 31–35 / 36 / 37–40 | course 3 free run / final card / final |
| 41 | jump contest |
| 42–46 / 47 / 48–51 | course 4 free run / final card / final |
| 52–59 | jump contest rules (waits for a button) |

Gen B adds the player select, the course map, the jump contest and a speed
readout. For the merge, Gen B is the natural base engine.

## Graphics uploads per game mode (`$80:B873` → `$80:B88D`)

At each mode's setup, `$80:B88D` walks a list from `$80:B9B2` (word index per
mode `$76`) into 7-byte entries at `$80:B9D8`: VRAM word address, byte count,
source address, source bank; `$FFFF` ends a list. Each entry is one DMA
through DmaKick. Stadium 1 build:

| Mode | Uploads (source+size → VRAM) |
|---|---|
| 00 | `88:8000+1000→1000` `84:8000+8000→4000` `82:8000+3800→2400` `82:F000+1000→3800` `89:A000+0800→0800` |
| 01 | `88:8000+1000→1000` `99:8000+8000→4000` `9F:8000+4000→2000` `88:D800+0800→0000` `88:F800+0800→0800` |
| 02 | `9C:8000+8000→4000` `88:E800+0800→0800` `88:8000+1000→1000` |
| 03 | `88:8000+1000→1000` `83:8000+8000→4000` `82:8000+2800→2400` `82:F000+1000→3800` `89:8000+1000→0000` `89:9000+1000→0800` |
| 04 | `88:8000+1000→1000` `8C:D000+3000→4000` `98:8000+4000→2000` `88:C800+0800→0000` `88:D000+0800→0800` `8A:F000+0800→1B00` |
| 05, 06 | `88:8000+1000→1000` `84:8000+2000→4000` |
| 09 | `88:8000+1000→1000` `9E:8000+8000→4000` `82:C000+4000→2000` `89:E000+1000→0800` `89:F000+1000→0000` |
| 0A | `82:C000+4000→2000` `8A:A000+4000→4000` `88:8000+1000→1000` `8A:E000+0800→0000` `8A:E800+0800→0800` |
| 0C | `88:8000+1000→1000` `83:8000+8000→4000` `8B:8000+2000→2000` `82:F000+1000→3800` `89:8000+1000→0000` `89:9000+1000→0800` |
| 0D | `9F:C000+3000→1000` `9A:8000+8000→4000` `89:A800+0800→0800` `8B:E000+0800→2800` |
| 0F | `88:8000+1000→1000` `82:F000+1000→3800` `97:8000+8000→4000` `89:C000+1000→0800` `89:D000+0800→0000` |
| 10 | `8B:A000+2000→1000` `8B:C000+2000→2800` `9B:8000+8000→4000` `88:E000+0800→0000` |
| 11 | `8D:8000+8000→4000` `8C:B000+0800→0000` |
| 12 | `8C:8000+3000→1000` `8F:B000+0800→3000` `9D:8000+8000→4000` `88:F000+0800→0000` |

(07, 08, 0B, 0E: only the font `88:8000`.) The 2 KB uploads to VRAM
`$0000`/`$0800` are tilemaps. Channels 6 and 7 run indirect HDMA from tables
at `$84:8482`/`$84:9E0D` (Gen A): the "DMA-read" parts of banks `$82-$84`
that are not in this list are HDMA scroll data.

## Rider sprites by character

`$1542` is the player's character. Rider animation frames stream to four
sprite slots each frame (NmiFixedUploads: sources `$1E12-$1E18`, banks
`$1543-$1546`). `$80:B5D6` fills the banks from `$80:B608 + character * 4`
and `$80:B55B` the offsets from `$80:B618 + character * 8`:

| `$1542` | slot 0 (player) | slot 1 | slot 2 | slot 3 |
|---|---|---|---|---|
| 0 | `86:8000` | `86:C000` | `87:8000` | `87:C000` |
| 1 | `86:C000` | `86:8000` | `87:8000` | `87:C000` |
| 2 | `87:8000` | `86:8000` | `86:C000` | `87:C000` |
| 3 | `87:C000` | `86:C000` | `86:8000` | `87:8000` |

So each character's frames are one 16 KB set (frame `f` at
`set + (f & $FE) * $200 + (f & 1) * $100`, two 8-tile rows of `$100` bytes),
and the race always shows the four characters, the player's in slot 0.

## Sound samples

The driver sets the sample directory to `$5C00` (DSP `$5D` = `$5C`, ARAM
`$0557`). `tools/sound.py list s1` maps every directory entry back to its ROM
range (each upload stream is contiguous in the file). The driver block
(`$8E:8000`) holds the shared instruments and two long samples at ARAM
`$4100` and `$31F0`; each song set (`$90`-`$93`) adds entries `$20`-`$24` at
ARAM `$C360`+; the credits set (`$85`) has its own instruments.

## Characters across the Stadiums

Racer names (Gen A: 29-byte tile records at `$80:A5F9`, font M=`$76` A=`$5A`
R=`$7B` I=`$72` O=`$78`...; Gen B: 15-byte records at `$80:A598`, Stadium 4
at +`$4B`, the same font moved down by `$50`) and the sprite set tables:

| Build | Char | Name | Rider set (16 KB) |
|---|---|---|---|
| Gen A (S1/S2), `$1542` | 0 | MARIO | `$86:8000` |
| | 1 | LUIGI | `$86:C000` |
| | 2 | KINOPIO (Toad) | `$87:8000` |
| | 3 | WARIO | `$87:C000` |
| Gen B (S3/S4), `$154F` | 0 | MARIO | `$99:C000` (97% the same bytes as Gen A Mario) |
| | 1 | LUIGI (S3) / YOSHI (S4) | `$9A:8000`: Luigi in the S3 dump (92% like Gen A Luigi), Yoshi in the S4 dump |
| | 2 | KINOPIO | `$9A:C000` (redrawn) |
| | 3 | WARIO | `$9B:8000` (88% like Gen A Wario) |
| | 4 | PEACH | `$98:C000` |

Gen B sets the slot banks at `$82:8889` from `$82:88C8` (or `$82:88DC` when
`$1E91` != 0; they differ only in slot 3) and the offsets from `$82:88F0` /
`$82:8918`, 4 bytes and 8 bytes per character; the streaming
(`$80:83D2`, banks `$1550-$1553`) is the same as Gen A's. All sets share one
format, so the full roster (Mario, Luigi, Toad, Wario, Peach, Yoshi) exists
across the S1 and S3/S4 dumps. A merged roster in the Gen B engine needs a
sixth set placed in free ROM, longer per-character tables (banks, offsets,
names, the select wheel) and the sprite palettes per character.

## Course column codes (from in-game tests)

Each code `00`-`3F` was placed alone at column 31 of Stadium 1 course 1 in a
custom course (`mods/.../courses/course0.bin`) and the race was captured
(`projects/codes`, 63 runs). What appears:

| Codes | Object |
|---|---|
| `01` `05` `06` `07` `08` `0A` `0B` `14` `21` `23` | ramps of different sizes and slopes (`14` a huge wall ramp, `21` a slope, `23` a long down slope) |
| `09` | start/finish gate with the LAP sign (the loader writes it to six columns) |
| `0C` `0D` | puddles (upper / lower rows) |
| `0E` | crystal spike |
| `02`-`04` `11`-`13` `15`-`1C` `1F` `20` `22` | posts, poles, blocks and barriers in different rows |
| `1D` `1E` | barrier with a ball |
| `0F` `10` `28` `29` | item boxes |
| `24`-`27` | coin in row 1-4 |
| `2A`-`2D` | the items the broadcast drops in (`$80:82AF`) |
| `2E`-`3F` | nothing visible, race unchanged |

The run codes `41`-`4F` (8-column runs) draw like plain ground but change
the riding (4 to 56 of 63 frames differ): surfaces with effects, not yet
measured. The editor (`tools/editor/course_editor.html`) carries these as
default names.

## Rider palettes and voices (Gen A)

* `$80:BDAF + character * 8 + slot`: OBJ palette of each rider slot (OAM
  attribute bits via `$0B`, written by `$80:B17B`). Player's own slot: Mario
  2, Luigi 3, Toad 4, Wario 1. Race sprite colours come from the palette
  block loaded at CGRAM 128 (`$88:B700` in practice/race).
* `$80:CB19 + character * 8 + rider`: crash voice sound per rider (`$37-$3A`).
  Gen B has the same table at `$80:C8E9` (+`$50` for Stadium 4, +`$28` when
  `$1E91` is set). Gen B's palette table is not found yet.
* A character swap mod (Peach/Yoshi into a Gen A slot) needs the 16 KB rider
  set, that slot's 16 colours in each OBJ palette block, and the name record
  (Gen B name tiles + `$50`).
