#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
out=${1:-tmp/electro-kits}
mkdir -p "$out"
cc -O2 -w -ffp-contract=off -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip \
    -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen \
    -o "$out/electro-kits" tests/electro_kits.c -lm
"$out/electro-kits" "$out"
python3 tools/match_electro_kits.py "$out"
