# BunBun S1 Recomp

A native recompilation of **BunBun S1** (JPN), built on
[snesrecomp](https://github.com/RetroPortingToolKit/snesrecomp).

_Add a short description here._



> **You must legally own a copy of the game.** No ROM data is distributed with
> this project, in the repository or in any release. The recompiled C is
> generated locally from your own copy and is never committed.

## Status

Scaffolded on 2026-11-11 — **not yet a working port.** The layout, build,
regeneration pipeline, CI, and packaging are wired up; the game does not run
until the host work in `src/game_rtl.c` is done. See
[Porting from here](#porting-from-here).

## ROM identity

| | |
|---|---|
| File | `s1.sfc` |
| Publisher | — |
| Developer | — |
| Year | — |
| Mapping | lorom |
| Region | JPN (Japan) |
| Coprocessor | unknown |
| CRC32 | `6e6d087c` |
| SHA-256 | `a22163ddd3d07f01101e885297568d1863db0639e5b1b97fc84dff253dee2089` |

`tools/regen.sh` refuses to run against anything else, so a mismatched
revision fails immediately instead of producing subtly wrong output.

## Build

```sh
git submodule update --init --recursive
bash tools/regen.sh --rom /path/to/s1.sfc
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The ROM does not have to live in the repository — keeping it on your own
drive is the better habit, and `SNESRECOMP_ROM` sets the path once for a
shell. Without either, `tools/regen.sh` finds `s1.sfc` at the repo root,
then the path the setup wizard recorded in `rom.cfg`; `.gitignore` blocks
both from ever being committed.

`tools/regen.sh` verifies the ROM, generates `src/gen/*.c`, and re-syncs
`recomp/funcs.h`. Re-run it whenever you change anything under `recomp/`.

## Run

```sh
./build/BunbunS1SNESRecomp                     # launcher picks the ROM
./build/BunbunS1SNESRecomp /path/to/s1.sfc # or name it and skip the launcher
```

With no ROM on the command line the build opens the recomp-ui launcher: a ROM
picker with a verification badge, plus display / audio / input settings. Your
choice is cached in `rom.cfg` beside the executable, so the next launch opens
on it and "Skip launcher on boot" makes it immediate. If the launcher is not
built in (`--no-recomp-ui`), the host falls back to a native file picker.

Either way the ROM is checked against the digests above before anything boots
— a dump that could not have produced this build is refused at the door rather
than mis-executing ten frames in.

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
    roster (Luigi).
  * *Sprite swaps*: pick a PNG rider sheet per character (export the
    originals with `tools/assets.py export`).
  * *Music & sound*: pick a folder of replacement samples (WAV files named as
    `tools/sound.py export` names them).
  * *Custom courses*: pick a course file per course (make them with
    `tools/editor/course_editor.html`).
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
| `recomp/` | Analysis input: `bank*.cfg`, `symbols.toml`, generated `funcs.h` |
| `rom_identity.txt` | ROM digests — read by the build, `tools/regen.sh` and CI |
| `src/` | Host code you own: `main.c`, `game_rtl.c` |
| `src/gen/` | Generated C. Never committed — regenerate locally |
| `snesrecomp/` | Framework submodule (owns `lib/recomp-net`, `lib/retcomm-rbengine`) |
| `tools/` | `regen.sh` — the ROM → C pipeline |
| `scripts/` | `package_release.sh` — player-facing zip |
| `framework_pins.txt` | Exact framework commits this project was scaffolded against |

## Porting from here

The scaffold stops where the game-specific work starts. In rough order:

1. **Make it boot.** `src/game_rtl.c` holds the frame driver. It starts on
   the framework's beam-aligned driver (`snesrecomp/runner/src/beam_frame_driver.h`):
   one frame per PPU field, interrupts taken where the beam latches them,
   the field rasterized from the raster journal. A title whose main loop it
   does not fit replaces those calls with its own driver.
2. **Name things.** Add entries to `recomp/symbols.toml` as you identify
   routines, then re-run `tools/regen.sh`. Set `emit = true` to promote one
   into ahead-of-time analysis; leave it false to keep it interpreted.
   Regen synchronizes the marked blocks in each bank cfg, creating missing
   configs. Only proven variants become AOT code. See `recomp/README.md`.
3. **Resolve dispatch misses.** After every run, deal with unresolved
   indirect targets before anything else — they are the reason a port
   diverges, and they are cheap to fix early.
4. **Never synthesise a result** to get past uncovered code, and never edit
   `src/gen/` by hand. Fix the config or the framework and regenerate.

## Multiplayer

Two players, one controller per port.

## License

This project's own source is under the license in `LICENSE`. The framework
carries its own terms — see `snesrecomp/LICENSE` and
`snesrecomp/THIRD_PARTY_ATTRIBUTION.md`. Neither covers the game data, which
is not distributed here.
