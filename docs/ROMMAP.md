# ROM map (Stadium 1 build, Gen A)

How the 1 MB image is used. Built from three sources:

* the static code map (`tools/bsdis.py`),
* run-time read maps from a locally instrumented snesrecomp build
  (`SNESRECOMP_ROMREAD_MAP`, `tools/romread.sh`, one session per broadcast
  minute, CPU and DMA reads kept apart), merged by `tools/romregions.py`,
* parsers for the formats we understand (`tools/spcblocks.py`, the course
  tables).

Byte totals over 120 one-minute sessions (60 as Stadium 1, 60 as Stadium 2 on
the same build): code 28,368; CPU-read data 101,090; DMA-read data 507,648;
padding 45,840; never read 365,630. Stadium 2 adds only its course data
(about 1.8 KB in `$80/$81`); graphics and music are shared by both Stadiums.

## Bank by bank

| Bank | Contents | Evidence |
|---|---|---|
| `$80` | code (22 KB), tables read by the CPU, BS header and vectors at `$FFB0-$FFFF` | static map, reads |
| `$81` | code (6 KB, incl. the BS-X library `$F80C-$FAB2`), build stamp `$8000-$800F`, course tables `$8C59/$8C6D/$8C81` | static map, reads |
| `$82-$84` | graphics / tilemaps sent by DMA | DMA reads |
| `$85` | sound: credits music upload stream (block 5, `$85:8000-$C21E`) | `spcblocks.py` |
| `$86-$8C` | graphics / tilemaps by DMA (`$8B` also has 4.4 KB of CPU-read data) | DMA reads |
| `$8D` | not read as Stadium 1 (tile-like data) | — |
| `$8E-$8F` | sound driver and samples (block 0, `$8E:8000-$8F:ADE2`, driver entry ARAM `$0500`) | `spcblocks.py` |
| `$90-$93` | four song sets (blocks 1-4, each loaded at ARAM `$C360`) | `spcblocks.py` |
| `$94-$96` | zero-filled 4 KB blocks starting with `$AA`; never read | contents |
| tails of `$8F-$93` | after each sound stream: mostly `$FF` with scattered cleared bits, more `$AA` blocks | contents |
| `$97-$9C`, `$9E` | graphics / tilemaps by DMA | DMA reads |
| `$9D` | not read as Stadium 1 (tile-like data) | — |
| `$9F` | graphics by DMA (`$9F:8000-$EFFF`) | DMA reads |

### The never-read areas

The `.bs` files are dumps of the whole 1 MB memory pack, not just the game.
The never-read areas look like stale pack contents: `$FF` (erased flash) with
scattered cleared bits after the end of each sound stream, 4 KB blocks with an
`$AA` marker, and tile-like data in `$8D` and `$9D`. None of it is referenced by
code or read in 120 sessions; the port and the merge can ignore it. (The
credits music in `$85` is the exception: it is read only on the credits path,
which the original Stadium 1 never reaches.)

## Known structures

| Address | What | Notes |
|---|---|---|
| `$80:88CE/D4/DA` | sound upload table (lo/hi/bank, 6 entries) | read by `SpcUpload` `$80:88E0` |
| `$80:9727` | free-run end minutes per course | 11, 23, 35, 47 |
| `$80:A5A8` | final end minutes per course | 18, 30, 42, 54 |
| `$80:B65D` | game mode handler table (19) | `RunGameModeScript` |
| `$80:B683` | per-mode setup lists | two-level dispatch |
| `$81:8000` | build stamp (BCD), `$800C` Stadium select, `$800E` debug flag | |
| `$81:8C59` / `$8C6D` / `$8C81` | course start offsets / lengths / data | 8 courses + 2 special |
| `$80:FFB0` | Satellaview header | title, dates, map byte `$30` |

## Next

* Identify the DMA'd assets (tiles, tilemaps, palettes) per scene; extract them
  with snes2asm's decoders.
* Decode the course byte format and the CPU-read tables in `$80/$81/$8B`.
