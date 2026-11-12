# Progress reports (ByteMatch)

`tools/report.py` writes objdiff-format reports to `progress/<version>/report.json`;
`.github/workflows/report.yml` publishes them on every push to master as the
artifacts `<version>_report`, which [ByteMatch](https://bytematch.dev/Tenome/Excitebike--Bun-Bun-Mario-Battle)
reads. The ROM dumps never leave the local machine: the reports are generated
locally and committed, and contain only routine names, addresses, sizes and
percentages.

| Artifact | What it measures | How a routine counts |
|---|---|---|
| `s1_report`, `s2_report`, `s3_report`, `s4_report` | the ca65 disassembly | 100 % when `tools/build.py <ver>` rebuilds the dump byte for byte (the report refuses to be written otherwise) |
| `s1_c_report`, `s4_c_report` | the decompilation into C | 100 % hand-written readable C (`c/hand/<ver>/`, difftest-checked); 50 % mechanically lifted C passing `tools/difftest816.py`; 0 % otherwise |

Regenerate after a change (with the dumps in `orig/`):

```bash
for v in s1 s2 s3 s4; do python tools/report.py $v; done
python tools/report.py s1 --c
python tools/report.py s4 --c
```
