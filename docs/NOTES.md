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

## Open questions
* What is the byte at `$81:800C`? It is 0 in S1 and 1 in the S2 rebroadcast
  dump, which is otherwise S1. It is in a data block (no direct code reference).
* What is the second dispatcher at `$80:8A2B` (`JMP ($0000)` from a list
  pointer at `$02`)? Its targets need the lists under `$80:B683`.
