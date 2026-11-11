#!/usr/bin/env bash
# Screenshot what the broadcast shows at each minute (0..59).
#   tools/minuteshots.sh EXE ROM OUTDIR [JOBS] [FRAMES_AFTER]
# For minute m: boot, poke $7F:FFFF = m, hold B (throttle) with periodic Start/A,
# and save the frame FRAMES_AFTER frames later as OUTDIR/mNN.ppm.
set -u
EXE=$1; ROM=$2; OUT=$3; JOBS=${4:-4}; AFTER=${5:-1500}
mkdir -p "$OUT"
one() {
  local m=$1 d="$OUT/m$1"
  mkdir -p "$d"
  {
    echo "turbo on"; echo "wait 60"
    printf 'poke 1FFFF %02X\npoke 1FFFA 0000\n' "$m"
    for i in $(seq 1 $((AFTER / 200 + 1))); do echo "press b 150"; echo "press start 3"; echo "wait 20"; echo "press a 3"; echo "wait 20"; done
    echo "quit"
  } > "$d/script.txt"
  ( cd "$d" && SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SNESRECOMP_SCREENSHOT="$OUT/m$(printf %02d "$m").ppm" \
      SNESRECOMP_SCREENSHOT_FRAME=$((AFTER + 60)) timeout 900 "$EXE" --no-launcher --rom "$ROM" --script "$d/script.txt" > "$d/log.txt" 2>&1 )
}
for m in $(seq 0 59); do
  while [ "$(jobs -rp | wc -l)" -ge "$JOBS" ]; do wait -n; done
  one $m &
done
wait
