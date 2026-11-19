#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
mkdir -p felucca/build/host
# The full root runner already executes six_codec_test through upstream
# project_test.c; do not repeat its 20,000-record matrix here.
cc -O1 -w -ffp-contract=off -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen -o felucca/build/host/song_snapshot_test tests/song_snapshot_test.c -lm
felucca/build/host/song_snapshot_test
python3 tools/song-snapshots/generate_layout.py --check
node tools/song-snapshots/schema.mjs --check
node tests/song-snapshot.mjs
node tools/song-snapshots/song.mjs validate tmp/song-fixtures/export.bgirl.json
node tools/song-snapshots/song.mjs inspect tmp/song-fixtures/agent-edited.bgirl.json
node tools/song-snapshots/song.mjs diff tmp/song-fixtures/export.bgirl.json tmp/song-fixtures/agent-edited.bgirl.json
