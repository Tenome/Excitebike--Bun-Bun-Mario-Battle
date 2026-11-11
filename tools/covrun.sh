#!/usr/bin/env bash
# Run N random-input coverage sessions of a snesrecomp build in parallel.
#   tools/covrun.sh EXE ROM OUTDIR FIRST_SEED COUNT FRAMES [JOBS] [--p2]
# Each session writes OUTDIR/<seed>/cap.json(+.jsonl) and a log.
set -u
EXE=$1; ROM=$2; OUT=$3; FIRST=$4; COUNT=$5; FRAMES=$6; JOBS=${7:-4}; P2=${8:-}
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "$OUT"
run_one() {
  local seed=$1 d="$OUT/$1"
  mkdir -p "$d"
  python "$HERE/randplay.py" "$seed" "$FRAMES" "$d/script.txt" $P2
  ( cd "$d" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SNESRECOMP_TIER2_CAPTURE=1 \
      SNESRECOMP_TIER2_MANIFEST="$d/cap.json" SNESRECOMP_TIER2_JOURNAL="$d/cap.jsonl" \
      timeout 3600 "$EXE" --no-launcher --rom "$ROM" --script "$d/script.txt" > "$d/log.txt" 2>&1 )
  echo "seed $seed rc=$? $(grep -o 'exit: .*' "$d/log.txt" | head -1)"
}
seed=$FIRST
end=$((FIRST + COUNT))
while [ $seed -lt $end ]; do
  while [ "$(jobs -rp | wc -l)" -ge "$JOBS" ]; do wait -n; done
  run_one $seed &
  seed=$((seed + 1))
done
wait
