# Modding the port: the `mods/` folder

The port loads replacement graphics, palettes, sound samples and patches
from plain files in a folder. No packaging or rebuild is needed.

```
mods/                       beside the executable (or set BUNBUN_MODS=<folder>)
  my-mod/                   one folder per mod, applied in name order
    disabled                (optional) an empty file with this name skips the mod
    *.ips                   IPS patches against the ROM image
    gfx/BBAAAA_4bpp_..._nN.png
    pal/BBAAAA_....txt
    sound/BBAAAA_....wav
    rom/BBAAAA_....bin
    courses/courseN.bin
```

`BBAAAA` is the SNES address of the data in the ROM (for example `868000` is
`$86:8000`). Only the first six characters of a file name matter to the
loader; the rest is free text. The tools below name every file correctly.

Everything is applied to the ROM image in memory when the game starts. The
ROM file on disk is never changed. Graphics, palettes and samples are read by
the game from ROM, so a replacement shows up everywhere the original is used.
Later mods overwrite earlier ones where they touch the same bytes.

## Getting the originals to edit

From the repository, with your own ROM in `orig/`:

```bash
python tools/assets.py export s1 my-mod
python tools/sound.py export s1 my-mod
```

This writes every known graphics block, palette and sample into `my-mod/`.
Delete what you do not change, edit the rest, and copy the folder into
`mods/`.

* The graphics list (`build/s1/assets.json`) is made from what the game
  actually uploads to the PPU. Recreate it with
  `python tools/assets.py catalog s1 '<dmalog>/*/dma.txt'`, where the logs come
  from runs of the port with `BUNBUN_DMALOG=<file>` (one line per distinct
  DMA, with the PPU state that tells what it is).
* `python tools/assets.py check s1 my-mod` re-encodes the exported sheets as
  the port does and compares them with the ROM. An unedited export gives zero
  differences.

## Graphics: `gfx/`

* `BBAAAA_<bpp>bpp_<use>_n<tiles>.png`: a sheet of 8x8 tiles, read left to
  right and top to bottom. `<bpp>` is 2, 4 or 8 bits per pixel. `<use>` says
  where the game puts them (`obj` for sprites, `bg1`-`bg3` for backgrounds).
  `n<tiles>` is the number of real tiles; padding at the end of the last row
  is ignored.
* Keep the image size. Draw with the palette in the PNG (an indexed PNG): the
  pixel's palette index is what the game stores, 0 is transparent.
* If your editor saves RGB, that works too. Each pixel is matched to the
  nearest colour in `<same name>.pal.txt` beside the PNG (written by the
  export). Fully transparent pixels become index 0.
* The colours in an exported sheet are a guide only. On screen the game picks
  the sub-palette per tile (tilemaps) or per sprite (attributes).
* Sprite sheets are 8 tiles wide: the rider animation frames are streamed to
  VRAM as pairs of 8-tile rows, so each 8x2-tile band is one frame strip.

## Palettes: `pal/`

`BBAAAA_cNNN.txt`: one colour per line as `RRGGBB` hex, written to ROM as
SNES colours (5 bits per channel). `cNNN` is the first CGRAM colour the game
loads it to. Lines starting with `#` are comments.

## Sound: `sound/`

`BBAAAA_dNN_<n>b.wav`: one BRR sample. `dNN` is its sample directory number
and `<n>b` its length in 9-byte BRR blocks (16 samples each).

* Any WAV works (any rate, mono or stereo, 8/16/32-bit). It is mixed to mono,
  stretched or squeezed to the original number of samples, and encoded to
  BRR. The loop and end flags and the loop point stay as they were, so a
  looping instrument still loops at the same place.
* The length is fixed because the sound driver's memory layout is. A sound
  much longer than the original gets played faster; trim it first for the
  best result.
* The exported WAVs are written at 16 kHz for listening. The real playback
  rate depends on the note the music plays.
* `BBAAAA_....brr`: raw BRR data of exactly the original size, if you encode
  it yourself.

## Raw data: `rom/` and IPS

* `rom/BBAAAA_....bin` copies the file's bytes to that address. The asset
  export puts tilemaps here (`..._tilemap_bgN.bin`, 2 bytes per tile entry).
* `*.ips` in a mod's folder is applied first, before `rom/`, `gfx/`, `pal/`
  and `sound/`. Offsets in IPS files are file offsets in the 1 MB image.

## Courses: `courses/` and the course editor

`courses/courseN.bin` replaces course `N` when the game loads it. The file is
the expanded track: one byte per column, as the game keeps it in its track
buffer (`$7F:4000`). Up to 4088 columns.

`N` is the course table index. In the Gen A build (Stadiums 1-2) 0-3 are
Stadium 1 courses 1-4 and 4-7 are Stadium 2; in the Gen B build 0-3 are
Stadium 3 and 4-7 Stadium 4. 8 and 9 are the special courses. If several mods
have the same course, the last one in name order wins.

To edit, open `tools/editor/course_editor.html` in a browser (it runs
offline, nothing is uploaded). Open your ROM to load its courses (or a course
`.bin`), paint columns with a code, insert, delete, copy and paste, and save
`courseN.bin`. Column codes: `40` plain ground in Gen A (`80` in Gen B),
`41`-`4C` ground variants, `00`-`3F` obstacles and events (`09` is a wide
element that covers six columns). Code names you type are kept in the
browser. `python tools/course.py s1 --dump` writes the original courses as
`build/s1/courseN.bin` in the same format.

## Debugging

* The game log lists every file applied (`mods: ...`).
* `BUNBUN_MODS_DUMP=<file>` writes the patched image after loading, for
  checking a mod with other tools. Do not distribute it: it is the game.
