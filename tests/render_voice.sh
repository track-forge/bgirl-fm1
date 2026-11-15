#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p app/build/voice
cc -std=c99 -O2 -ffp-contract=off -Ibase/include tests/voice_render.c app/dsp/vocoder.c -lm -o app/build/voice/render
for pitch in 110 164.8138 220; do
  app/build/voice/render "$pitch" > app/build/voice/voice.raw
  ffmpeg -hide_banner -loglevel error -y -f s16le -ar 44100 -ac 1 -i app/build/voice/voice.raw "app/build/voice/vocoder-$pitch.wav"
done
