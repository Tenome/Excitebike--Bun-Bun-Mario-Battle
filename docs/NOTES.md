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
| Gen A: S1, S2 | ~24.4 KB | `$80-$81` | 356 |
| Gen B: S3, S4 | ~32.3 KB | `$80-$82` | 471–474 |

S1 and S2 code is the same apart from a small insertion that shifts later
addresses. S3 and S4 also share their code. Most of the byte differences are
data.

## Satellaview (BS-X) usage
* The only receiver library is `$81:F80C-$81:FAB2`. It uses `$2188-$2197`.
* `$81:FA68` `BsxDetect` → `$7FFFE1` (1 = no receiver).
* `$81:F80C` `BsxOpenTimeChannel` (only runs when a receiver is found).
* `$81:F8BB` is the per-frame poll of the stream (`$2190`/`$2192`/`$2193`),
  called from `$80:810A`. It compares a received byte with `$7FFFFF`
  (the Stadium/episode number?) and calls `$81:F999` when it changes.
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

## Build stamp and debug flags (`$81:8000`)
* `$81:8000-$8005`: BCD build timestamp YY MM DD hh mm ss.
  S1 dump `98-03-23 13:40:41`, S2 `97-05-02 17:37:15`, S3 `97-10-23 20:53:10`,
  S4 `98-03-19 15:49:25`. The S1 dump is a 1998 rebuild, not the May 1997 original.
* `$81:800C`: mode flag read by the menus and race setup (`$80:916B`, `$80:B715`,
  ...). 0 in S1/S3, 1 in S2/S4/S2-rebroadcast. Probably "second-week episode".
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
* During a race (mode 3) a change of minute matched against `$80:82A7` (8
  values) injects an event into the track buffer at `$7F:4000` (`$80:823C`).
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

The 2-minute cards are where the SoundLink narration played.

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
