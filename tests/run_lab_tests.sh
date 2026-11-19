#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
lab_out=${1:-local/lab-rollout/after/lab}
mkdir -p "$lab_out/ppm" felucca/build/host
cc -O1 -w -ffp-contract=off -DFELUCCA_VERSION='"1.8.1"' \
 -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip \
 -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/build/gen \
 -Ifelucca/firmware/src -Ifelucca/tests \
 -o felucca/build/host/lab_render tests/lab-render.c -lm
felucca/build/host/lab_render "$lab_out"
python3 felucca/tests/ui_render.py "$lab_out"
