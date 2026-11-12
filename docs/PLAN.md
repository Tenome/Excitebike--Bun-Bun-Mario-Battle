# Project plan

Goal: disassemble, decompile and recompile *Excitebike: Bun Bun Mario Battle*
(Satellaview, 1997) into a native PC port. Then merge the four Stadium
broadcasts into one game.

## What we are working with

| Version | File in `orig/` | Source dump | Engine |
|---|---|---|---|
| Stadium 1 | `s1.bs` | Stadium 1 (6-30). The (6-29) dump is the same except for one header date byte. | Gen A |
| Stadium 2 | `s2.bs` | Stadium 2 (5-18) | Gen A |
| Stadium 2 rebroadcast | `s2r.bs` | Stadium 2 (7-6). It is Stadium 1 with 6 bytes changed: title digit, date, checksum, and `$81:800C` 0→1. | Gen A |
| Stadium 3 | `s3.bs` | Stadium 3 (11-5) | Gen B |
| Stadium 3 rebroadcast | `s3r.bs` | Stadium 3 (2-8). 11,377 bytes differ from Stadium 4, so it is built from the Stadium 4 build. | Gen B |
| Stadium 3 bad dump | `s3b.bs` | Stadium 3 (1-11) [b]. Its header is corrupt. Reference only. | Gen B |
| Stadium 4 | `s4.bs` | Stadium 4 (2-18). The (2-17) dump differs by one date byte. | Gen B |

All images are 1 MB LoROM memory-pack images with a BS header at `$7FB0`.
The code runs at `$80:8000`. The vectors at `$FFE4..$FFFF` point to small
`JML` stubs at `$80:FFA0`.

* **Gen A** (Stadiums 1–2): about 24 KB of code in banks `$80-$81`.
* **Gen B** (Stadiums 3–4): about 32 KB of code in banks `$80-$82`.

The rest of the 1 MB is graphics, tracks, music and other data.

## Key technical facts so far

* Every Satellaview hardware access sits in one library at
  `$81:F80C-$81:FAB2` (Gen A). It uses the satellite receiver registers
  `$2188-$2197` for the broadcast clock and the SoundLink time channel.
* At boot, `$81:FA81` probes the receiver by writing `$AA`/`$55` to
  `$2188` and reading them back. With no receiver it stores
  `$7FFFE1 = 1` and the reset code skips the satellite setup. **So the game
  does not need the BS-X BIOS or the satellite to boot.** It can run as a
  plain LoROM cartridge, which snesrecomp supports.
* SoundLink (live narration and voice) came over the radio stream and is not
  in the dumps. The port will not have it unless we recreate it.

## Toolchain

| Tool | Where | Use |
|---|---|---|
| `tools/bsdis.py` (ours) | repo | Recursive 65816 disassembler that knows the BS-X layout. Tracks M/X, recognises pointer tables, reports BS-X register use. Writes `report.txt`, `listing.asm` and `code.json`. |
| `tools/mkcfg.py` (ours) | repo | Turns `code.json` into snesrecomp `bank*.cfg` seeds. |
| snesrecomp CLI | `E:\Claude - SNES Decomp Disassembly\snesrecomp-cli-windows-x86_64` | 65816 → C (`generate`). |
| snesrecomp source | `E:\...\tools\snesrecomp` | Docs, runner, interpreter, co-sim, snesref. |
| snes2asm | `E:\...\tools\snes2asm` (patched for the BS header) | WLA-DX rebuild project, asset decoders (tiles, palettes, BRR). For the byte-matching rebuild and asset extraction. |
| DiztinGUIsh | `E:\...\DiztinGUIsh` | Interactive GUI labelling, if useful. |
| Ghidra 12.2 | `E:\Claude Typing of the Dead\ghidra_12.2_PUBLIC` | Optional second opinion on hard routines. |

## Phases

### Phase 1: map the code (in progress)
1. Find all code with `bsdis.py`, starting from the vectors and adding pointer
   tables as they are recognised. ✅ First pass done for every version.
2. Resolve the last indirect dispatchers. Example: the two-level script
   table at `$80:8A2B` (`$B683` → lists of handler pointers).
3. Confirm code coverage at run time: run the ROM in the snesrecomp
   interpreter or a reference emulator through `snesref` and log every
   executed PC. Add the missing roots. ✅ Stadium 1: 83% executed, nothing
   missed (see NOTES). Stadiums 2–4 in progress.
4. Name routines and RAM in `config/<ver>/names.txt` and a RAM map.

### Phase 2: byte-matching disassembly
1. Have `bsdis.py` write assembler source (ca65 or WLA-DX) for code and data.
   ✅ `tools/mkasm.py` writes ca65 source per bank; `tools/build.py <ver>`
   disassembles, assembles, links and compares.
2. Rebuild each version byte-identically. ✅ All seven dumps match
   (2026-11-11). CI (ByteMatch) still to do; the source is generated into
   `build/` because it carries ROM data.
3. Split the data into assets: graphics (4bpp tiles), palettes, tilemaps,
   tracks, text and SPC700 music. snes2asm's decoders help here.

### Phase 3: recompile to C and boot natively
1. Generate C with `snesrecomp generate --cfg-roots` from our cfg seeds. ✅
   First run: 542 function variants, 533 native and 9 left to the interpreter.
2. Scaffold a full game project with snesrecomp's `setup_project`: CMake,
   runner, SDL3 and the launcher. Get it to the title screen. ✅ `port/`
   boots Stadium 1 natively through the intro, title and into a race
   (2026-11-10). Native and interpreter-only builds render the same scenes;
   frame timing drifts by a few frames (not yet co-simulated).
3. Replace the BS-X library with a C function (`hle_func`). It reports "no
   receiver" or returns a fake clock, so time-gated content can be unlocked.
4. Compare against an independent emulator (snesref plus a libretro core)
   frame by frame, and fix the differences.

### Phase 4: readable decompilation
0. ✅ Pipeline (2026-11-11): `tools/lift816.py` lifts every function variant to
   faithful C over `c/runtime816.h` (`c/auto/<ver>/`, regenerated, not
   committed); `tools/difftest816.py` checks each one against the original in
   `tools/cpu816.py` from real WRAM snapshots (WRAM, registers, flags, ordered
   hardware writes). Stadium 1, all 409 variants, 8 cases each from real
   WRAM snapshots: 3,005 pass, 256 skipped (the original waits on hardware),
   11 fail. Every failure is a jump table indexed out of range by a random
   input (for example `$80:E834` takes its index in A), not a lifting error;
   the lifted code falls back to dispatch-by-address for pointer values
   outside a table. Stadium 4 (539 variants): 4,004 pass, 306 skipped, 2 fail
   (one function, same out-of-range cause).
0b. ✅ Hand-written replacements: `c/hand/<ver>/*.c` + `hand.txt`; the lifter
   leaves those bodies out and the difftest checks them (`--hand-only`).
   Eight so far (DMA helpers, QueueSound, ReadJoypads, LoadCourse,
   BsxPollStream, RunGameModeScript) pass. `tools/liveout.py` finds which
   registers and flags callers read after each call; `hand.txt` compares
   exactly those (`mem+A,X,Y`), so a readable routine may drop flag results
   nobody reads but nothing that is observable.
0c. ✅ Decompiled code in the port: `port/src/r816_bridge.c` binds the
   runtime816 machine model to snesrecomp's CPU and bus and wraps hand
   routines as `hle_func`s. Gen A now runs five hand-written routines
   (QueueSound, DmaCopyToWram, DmaFillZero, LoadCourse, BsxPollStream);
   verified by play-through to a race and an identical track buffer.
1. Hand-lift routines to readable C, kept in `c/`. `c/s1_core.c` is the start.
2. Use the same approach as the NES Excitebike port: test each routine
   against the recompiled or interpreted original. Compare RAM and the order
   of hardware register writes.
3. Turn raw RAM into named structs: riders, track, camera, items.

### Phase 5: merge the Stadiums
0. ✅ Found that each build already holds two Stadiums behind `$81:800C`; the
   port's `BUNBUN_STADIUM=1|2` selects Stadium 1 or 2 from one build. Stadiums
   3–4 need the Gen B build (S4 dump).
0b. ✅ Second executable for the Gen B engine (`BunbunS4SNESRecomp`,
   `BUNBUN_STADIUM=3|4`), verified at the player select (Luigi vs Yoshi).
0c. ✅ First cross-generation run: the Gen B course loader is replaced in C
   (`port/src/courses.c`); with `BUNBUN_GENA_ROM=<s1 dump>` and
   `BUNBUN_COURSES=1|2` the Gen B engine plays the Stadium 1 or 2 courses
   (converted on load, run flag `$40` → `$80`). Runs without errors; the
   terrain-code mapping is not yet checked tile by tile. Without the override
   the replacement builds a track buffer identical to the original (checked
   against the interpreter-only build: 353 bytes, 0 differences).
1. Match routines across versions by instruction signature, ignoring
   addresses. This gives the exact code differences A↔A, B↔B and A↔B.
2. Use the Gen B engine (Stadium 4) as the base. Load each Stadium's data
   (tracks, characters, graphics, rules) as a content pack.
3. Add a Stadium select and a character roster that combines all of them
   (Mario, Luigi, Toad, Wario, Peach, Yoshi).
4. Port extras later: widescreen, more riders, higher frame rate, mods.

## Repository rules
* `orig/` (ROMs) and `build/` (all generated output, including C made by
  snesrecomp) are git-ignored and never committed.
* Edit the tools and `config/`, never the generated files.
* Push only with the owner's OK.

## Port features (goals for the PC version)

Goals: every character from all four Stadiums, a GUI level editor, sound and
sprite replacement from a plain folder, more riders per race, and other
quality-of-life improvements.

| Feature | Status |
|---|---|
| Runs without the ROM | ✅ `port/tools/import_roms.py` copies the dumps into `data/` beside the exes once; the exe starts from there. |
| Mods folder (`docs/MODDING.md`) | ✅ `port/src/mods_folder.c`: IPS, raw `rom/` overlays, `gfx/` PNG tile sheets, `pal/` palettes, `sound/` WAV→BRR samples, `courses/` custom courses. Verified in a race (recoloured rider set; WAV replacement ~38 dB SNR). |
| Asset export | ✅ `tools/assets.py` (catalogue from the port's DMA log, PNG export, round-trip check), `tools/sound.py` (every BRR sample to WAV). |
| Course editor | ✅ first version: `tools/editor/course_editor.html` (offline page: opens the ROM or a course file, paint/insert/delete/copy, undo, saves `courseN.bin`). Next: name the column codes from in-game experiments, draw the obstacles with the game's own tiles. |
| Full roster | ✅ All six characters in the Gen B exe: Mario, Yoshi, Kinopio, Wario, Peach and Luigi, selectable on a six-portrait PLAYER SELECT (c/hand/s4/select.c, port/src/roster.c). Luigi's rider set and voice samples come from data/s3.sfc into an extended 4 MB image; he rides on Yoshi's engine index, the roster tracks the real character ($15FE). Next: let Luigi and Yoshi meet in one race (decompile the remaining index users: collision voices $80:C776, results $81:9926, $80:924C). |
| More riders per race | Not started. Races run 8 riders with 4 streamed sprite slots. |
