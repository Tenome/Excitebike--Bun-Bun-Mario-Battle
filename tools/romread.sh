#!/usr/bin/env bash
# Collect ROM read maps (CPU/DMA) with a locally instrumented snesrecomp build
# (SNESRECOMP_ROMREAD_MAP, see docs/NOTES.md), one session per broadcast minute.
#   tools/romread.sh EXE ROM OUTDIR [JOBS] [FRAMES]
set -u
EXE=$1; ROM=$2; OUT=$3; JOBS=${4:-2}; FRAMES=${5:-8000}
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "$OUT"
one() {
  local m=$1 d="$OUT/m$1"
  mkdir -p "$d"
  python "$HERE/randplay.py" "$m" "$FRAMES" "$d/script.txt" --minute "$m"
  ( cd "$d" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SNESRECOMP_ROMREAD_MAP="$d/romread.bin" \
      timeout 1800 "$EXE" --no-launcher --rom "$ROM" --script "$d/script.txt" > "$d/log.txt" 2>&1 )
}
for m in $(seq 0 59); do
  while [ "$(jobs -rp | wc -l)" -ge "$JOBS" ]; do wait -n; done
  one $m &
done
wait
echo done
