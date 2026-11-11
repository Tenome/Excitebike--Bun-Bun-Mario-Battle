# Excitebike: Bun Bun Mario Battle — decomp and native port

A research project. We disassemble, decompile and recompile the Satellaview
game *Excitebike: Bun Bun Mario Battle* (1997, Stadiums 1–4) into a native
PC program. Later we merge all four Stadiums into one game.

No ROM data is included. Put your own dumps in `orig/` (see
[docs/PLAN.md](docs/PLAN.md) for the expected file names).

* [docs/PLAN.md](docs/PLAN.md): phases, toolchain and the state of each step
* [docs/NOTES.md](docs/NOTES.md): research findings: memory map, BS-X library, routines
* `tools/bsdis.py`: BS-aware recursive 65816 disassembler
* `tools/mkcfg.py`: turns the disassembler's output into snesrecomp cfg seeds
* `config/<ver>/names.txt`: routine names
* `c/`: hand-decompiled C

## Quick start

```bash
python tools/bsdis.py orig/s1.bs --out build/s1        # report.txt, listing.asm, code.json
python tools/mkcfg.py build/s1/code.json build/s1/cfg --names config/s1/names.txt
snesrecomp.exe generate --rom s1.sfc --cfg-dir build/s1/cfg --out-dir build/s1/gen --cfg-roots
```

## Native port (`port/`)

A snesrecomp game project for Stadium 1, with the framework and launcher as
submodules (`git submodule update --init --recursive`).

```bash
python tools/bsdis.py orig/s1.bs --out build/s1
python tools/mkcfg.py build/s1/code.json port/recomp --names config/s1/names.txt --into
cp orig/s1.bs port/s1.sfc && (cd port && bash tools/regen.sh --cfg-roots)
cmake -S port -B port/build -G "Visual Studio 18 2026" -A x64
cmake --build port/build --config Release --parallel
port/build/Release/BunbunS1SNESRecomp.exe --no-launcher --rom port/s1.sfc
```

Coverage: `tools/covrun.sh` runs random-input sessions with snesrecomp's
coverage capture; `tools/covcmp.py` checks them against the static map.

### Port options (environment variables)

| Variable | Effect |
|---|---|
| `BUNBUN_STADIUM=1\|2` | Gen A executable: play Stadium 1 or 2 |
| `BUNBUN_STADIUM=3\|4` | Gen B executable: play Stadium 3 or 4 |
| `BUNBUN_SKIPWAIT=1` | Gen A: Start skips the title wait and the course cards (advances the broadcast clock one minute per press) |
| `BUNBUN_GENA_ROM=<path>` + `BUNBUN_COURSES=1\|2` | Gen B executable: use Stadium 1 or 2 courses, read from your Stadium 1 dump |
