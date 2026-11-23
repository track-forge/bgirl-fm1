#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
BGIRL_HARDWARE_ASSETS=1 python3 tools/generate.py
cd felucca
mkdir -p build/host/miami-embedded build/host/miami-slot
for mode in embedded slot; do
    flag=
    [ "$mode" != slot ] || flag=-DBGIRL_MIAMI_SLOT
    out=build/host/miami-$mode
    cc -O1 -w -ffp-contract=off $flag -Ifirmware/hal -Ifirmware/bsp/chip \
        -Ifirmware/bsp/boards/fm1 -Ifirmware/src -Ibuild/gen \
        -o "$out/test" ../tests/vocal_bank_test.c -lm
    "$out/test" "$out" build/gen/miami.slot
done
for file in decoded.pcm vocal-0.wav vocal-1.wav vocal-2.wav vocal-3.wav; do
    cmp "build/host/miami-embedded/$file" "build/host/miami-slot/$file"
done
echo "Miami XIP PCM and four dry/vocoded renders match the embedded hardware oracle byte for byte"
