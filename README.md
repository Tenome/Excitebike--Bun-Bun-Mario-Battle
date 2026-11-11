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
