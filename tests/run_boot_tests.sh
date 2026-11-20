#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
mkdir -p felucca/build/host
cc -O1 -w -ffp-contract=off -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen -o felucca/build/host/boot_render tests/boot-render.c -lm
felucca/build/host/boot_render "$@"
