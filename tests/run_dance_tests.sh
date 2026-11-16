#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
DANCE_INC='-Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen'
cc -O1 -w -ffp-contract=off $DANCE_INC -o felucca/build/host/dance_test tests/dance_test.c -lm
felucca/build/host/dance_test
cc -O1 -w -ffp-contract=off -DBGIRL_DANCE_TELEMETRY=0 $DANCE_INC -o felucca/build/host/dance_control tests/dance_test.c -lm
for mode in -1 0 1; do
    felucca/build/host/dance_test "$mode" > "felucca/build/host/dance-$mode.bin"
done
felucca/build/host/dance_control -1 > felucca/build/host/dance-control.bin
for mode in -1 0 1; do
    cmp felucca/build/host/dance-control.bin "felucca/build/host/dance-$mode.bin"
done
echo 'dance preservation: 512000 stereo frames +4000 RNG/note snapshots identical: disabled hook vs hidden/CRISP/GHOST PASS'
cc -O2 -pthread -o felucca/build/host/dance_snapshot_test tests/dance_snapshot_test.c
felucca/build/host/dance_snapshot_test
python3 tests/test_convert_dance.py
