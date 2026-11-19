#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
cut_out=${1:-local/cut-rollout/after/native}
mkdir -p "$cut_out/ppm" felucca/build/host
cc -O1 -w -ffp-contract=off -DFELUCCA_VERSION='"1.8.1"' \
 -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip \
 -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/build/gen \
 -Ifelucca/firmware/src -Ifelucca/tests \
 -o felucca/build/host/cut_render tests/cut-render.c -lm
felucca/build/host/cut_render "$cut_out"
python3 felucca/tests/ui_render.py "$cut_out"
