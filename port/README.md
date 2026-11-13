# Excitebike: Bun Bun Mario Battle — native port

A native PC version of the Satellaview game *Excitebike: Bun Bun Mario
Battle* (1997), recompiled from your own dumps with
[snesrecomp](https://github.com/RetroPortingToolKit/snesrecomp) and partly
replaced by hand-decompiled C. See the [project README](../README.md) for an
overview and screenshots.

> **You must legally own a copy of the game.** No ROM data is distributed with
> this project, in the repository or in any release. The recompiled C, the
> embedded dumps and the editor graphics are generated locally from your own
> copy and are never committed.

## Status

Playable from boot to the end screen in both engines: title, player select,
course preview, races, results, the final and the jump contest. On top of the
original: a six-character roster, CPU rivals in every race, a clock-free
flow with skip buttons, a launcher with a Mods page, sprite, sound and course
mods, and a course editor. 207 of the 532 Stadium 3–4 routines and 152 of
the 407 Stadium 1–2 routines run as hand-written C.

## ROM identity

| | Stadium 1–2 engine (Gen A) | Stadium 3–4 engine (Gen B) |
|---|---|---|
| Executable | `BunbunS1SNESRecomp` | `BunbunS4SNESRecomp` |
| Boots from | `s1.sfc` (Stadium 1) | `s4.sfc` (Stadium 4) |
| CRC32 | `6e6d087c` | `c747fc3b` |
| SHA-256 | `a22163dd...dee2089` | `d689aacf...de00ea` |
| Also uses | — | `s3.sfc` (Luigi), `s1.sfc` (Stadium 1–2 courses) |

Full digests: `rom_identity.txt`, `rom_identity_genb.txt` and
`port/tools/embed_roms.py`. LoROM, 1 MB, Japan. The regen scripts refuse any
other dump.

## Build

```sh
git submodule update --init --recursive
bash tools/regen.sh      --rom /path/to/s1.sfc --cfg-roots   # Gen A: src/gen
bash tools/regen_genb.sh --rom /path/to/s4.sfc --cfg-roots   # Gen B: src/gen-genb
cmake -S . -B build
cmake --build build --config Release
python tools/editor_graphics.py                              # editor graphics (optional)
```

The regen scripts verify the dump, generate the recompiled C
(`src/gen*/`), re-sync `recomp*/funcs.h` and embed your dumps
(`src/embed/`). Re-run them after changing anything under `recomp*/` (for
example a new `hle_func` line). `SNESRECOMP_ROM` can name the dump once for a
shell instead of `--rom`.

## Run

Start `build/genb/Release/BunbunS4SNESRecomp.exe` (the main game) or
`build/Release/BunbunS1SNESRecomp.exe`. The launcher opens first: settings,
controls and the Mods page; "Skip launcher on boot" goes straight into the
game. `--no-launcher` does the same from the command line, and
`--script <file>` plays back scripted input (the regression tests use it).

## No ROM needed after the build

`tools/regen.sh` and `tools/regen_genb.sh` also run `tools/embed_roms.py`,
which builds your dumps into the executable (`src/embed/`, git-ignored): the
Gen A exe carries `s1.sfc`, the Gen B exe `s4.sfc`, `s3.sfc` (Luigi) and
`s1.sfc` (Stadium 1-2 courses), found by their digests beside the `--rom`
you pass, in `$BUNBUN_DUMPS` or in a build tree's `data/`. The built exe then
runs from any folder with nothing supplied: on its first launch it writes its
own boot dump to `data/` and points `rom.cfg` at it (the framework host boots
from a file), and reads the other dumps from memory. `BUNBUN_NO_EMBED=1`
skips this, and the exe reads `data/` as before.

**An executable built this way contains the game. Keep it for yourself;
never share or upload it.**

The course editor is copied beside each exe as `editor/course_editor.html`
together with `editor_courses.js` (every course of both engines, decoded at
build time), so it opens straight onto the game's courses without a ROM.
Save a course as `courseN.bin` into `mods/<name>/courses/` (or pick it on
the Mods page) to play it.

## Which executable

* `build/genb/Release/BunbunS4SNESRecomp.exe` is the main game: the Gen B
  engine with all six characters (Mario, Yoshi, Kinopio, Wario, Peach,
  Luigi) on the player select. Built with the dumps embedded it needs
  nothing else; otherwise it reads `data/s4.sfc` and, for Luigi,
  `data/s3.sfc` (`python port/tools/import_roms.py <dumps>`).
  `BUNBUN_STADIUM=3|4` picks the Stadium, `BUNBUN_COURSES=1|2` plays the
  Stadium 1 or 2 courses in it (needs `data/s1.sfc`).
* `build/Release/BunbunS1SNESRecomp.exe` is the Stadium 1-2 engine (Gen A),
  kept for comparison and the original Stadium 1-2 experience.

## Settings and mods: the launcher

Starting an executable opens the launcher (tick "Skip launcher on boot" to go
straight in). **Ctrl+L** (or Select+L3 on a pad) opens it again over the
running game; changes that cannot apply live restart the game through a save
state, so you continue where you were.

* **Player 1 / Player 2**: keyboard or gamepad per player, and **Configure**
  for the buttons (stored in `keybinds.ini` / `config.ini`).
* **Settings**: display, audio, hotkeys, rewind.
* **Mods** (stored in `bunbun.ini`):
  * *Game*: play without the broadcast clock (default) or with the original
    timing; Stadium; Stadium 1/2 courses in the Gen B engine; the expanded
    roster (Luigi); *CPU rivals*: the original runs the free-run races
    alone and meets its seven CPU rivals only in the final, the default here
    puts all seven in every race (the engine has eight rider slots; more
    would need the race engine rewritten, which the decompilation is working
    towards); *Rival pace*: easy (rivals 15% slower), normal (the
    original) or hard (15% faster), applied to the target speed the rival
    AI ($80:BE66) picks.
  * *Widescreen*: 16:9 races (`src/widescreen_bunbun.c`). The track's
    right margin is filled by replaying the track streamer ($81:8A4D) a few
    columns ahead into the unused part of its 64-column tilemap; the bottom
    HUD's checkered flag is reflected into the margins; every other screen
    is pillarboxed. Riders in the margins are shown too: PlaceRiderShadow
    ($80:CE96) ends with a port hook that unhides a rider's sprites inside
    the wide view and sets each column's X9 bit. Restart to apply.
  * *Sprite swaps*: pick a PNG rider sheet per character (export the
    originals with `tools/assets.py export`).
  * *Music & sound*: pick a folder of replacement samples (WAV files named as
    `tools/sound.py export` names them).
  * *Custom courses*: pick a course file per course (make them with the
    course editor, `editor/course_editor.html` beside the executable).
  * one switch per folder in `mods/` (see `docs/MODDING.md`).

## Controls

Skipping (Game flow "Play without the broadcast clock", the default): Start
through the intro and title to PUSH START, then Start for the player select;
after choosing a character, A or Start skips the countdown; Start skips the
course preview and the results; A or Start skips cards and rules screens;
**Select ends a race** early and moves on. With "Original broadcast timing"
every screen waits for the broadcast schedule as on the Satellaview.

Keyboard defaults (change them in the launcher): arrows, Z = B, X = A,
A = Y, S = X, C/V = L/R, Enter = Start, Right Shift = Select. Gamepads
(Xbox, PlayStation, Switch Pro) are detected automatically. A player set to
gamepad does not read the keyboard. Hotkeys: Alt+Enter fullscreen, Tab turbo,
F1-F10 load state, Shift+F1-F10 save state, F12 rewind, P pause, Ctrl+L
launcher.

## Layout

| Path | What lives there |
|---|---|
| `src/main.c` | The host shim: title identity, launcher hooks, the built-in dumps |
| `src/game_rtl.c` | Frame driver, the clock-free flow and skip buttons |
| `src/roster.c` | The six-character roster and CPU rivals in every race |
| `src/courses.c`, `src/mods_folder.c` | Course loading, custom courses, sprite and sound mods |
| `src/launcher_mods.c`, `src/bunbun_settings.c` | The Mods page and `bunbun.ini` |
| `src/r816_bridge.c` | Puts the hand-written C (`../c/hand/`) in place of recompiled routines |
| `src/bunbun_data.c`, `src/bunbun_embedded*` | The dumps: built in, or from `data/` |
| `recomp/`, `recomp-genb/` | Analysis input per engine: `bank*.cfg` (`hle_func` lines name the hand-written routines) |
| `src/gen/`, `src/gen-genb/`, `src/embed/` | Generated from your dumps. Never committed |
| `tools/` | `regen*.sh`, `embed_roms.py`, `editor_graphics.py`, `import_roms.py` |
| `snesrecomp/`, `recomp-ui/` | Framework and launcher submodules |

## How the port works

The framework recompiles every routine of the game into C ahead of time and
runs it on a cycle-counted model of the SNES (PPU, APU, DMA, the BS-X
registers). Routines the decompilation has written by hand
(`../c/hand/s1`, `../c/hand/s4`) replace the recompiled ones: each passes
`../tools/difftest816.py` against the original code first, then
`src/r816_bridge.c` hooks it in with a cycle estimate so frame timing stays
the original's. Port features (roster, rivals, flow, mods) hook in at the
same seams, mostly through the hand-written routines, so the original
behaviour stays available behind an option. After every change the race and
jump-contest runs are compared frame by frame with the previous build.

## Multiplayer

Two players, one controller per port.

## License

This project's own source is under the license in `LICENSE`. The framework
carries its own terms — see `snesrecomp/LICENSE` and
`snesrecomp/THIRD_PARTY_ATTRIBUTION.md`. Neither covers the game data, which
is not distributed here.
