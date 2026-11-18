#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
CREW_INC='-Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen'
cc -O1 -w -ffp-contract=off -pthread $CREW_INC -o felucca/build/host/crew_test tests/crew_test.c -lm
felucca/build/host/crew_test
cc -O1 -w -ffp-contract=off -pthread -DBGIRL_CREW_TELEMETRY=0 $CREW_INC -o felucca/build/host/crew_control tests/crew_test.c -lm
felucca/build/host/crew_control 0 > felucca/build/host/crew-control.bin
for mode in 0 1 2; do
    felucca/build/host/crew_test "$mode" > "felucca/build/host/crew-$mode.bin"
    cmp felucca/build/host/crew-control.bin "felucca/build/host/crew-$mode.bin"
done
echo 'CREW audio/RNG preservation: disabled vs hidden/visible/covered, six-track playback +random arp, 512000 frames +4000 state snapshots identical PASS'
python3 tests/test_convert_crew.py
