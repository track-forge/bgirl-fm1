#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
python3 tools/carrier_bank.py --check
for name in sh_style_test sh_style_native; do
 cc -O2 -w -ffp-contract=off -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen -o "felucca/build/host/$name" "tests/$name.c" -lm
done
felucca/build/host/sh_style_test
felucca/build/host/sh_style_native felucca/build/host/sh-style-native.f32
node tests/sh-style-wasm.mjs
