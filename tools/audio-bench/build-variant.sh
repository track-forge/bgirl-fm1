#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
test -f felucca/build/gen/felucca_tables.h || {
    echo 'Generate assets first: BGIRL_HARDWARE_ASSETS=1 python3 tools/generate.py' >&2
    exit 1
}
"${CC:-cc}" -O1 -ffp-contract=off \
    -Ifelucca/web/emu -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip \
    -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen \
    tools/audio-bench/variant.c -lm -o felucca/build/bench-variant
