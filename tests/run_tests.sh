#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Browser/host subset of Felucca's runner. Device OTA/package/disassembly tests
# require the separate JieLi SDK build and are NOT claimed by this runner.
set -eu
cd "$(dirname "$0")/../felucca"
mkdir -p build/host
HALI='-Ifirmware/hal -Ifirmware/bsp/chip -Ifirmware/bsp/boards/fm1 -Ifirmware/src'
for t in storage_test upreset_test input_test midi_uart_test project_test persistence_test ui_test midi_control_test midi_drift_test swing_test; do
    echo "== upstream $t"
    cc -O1 -w -ffp-contract=off $HALI -Ibuild/gen -o "build/host/$t" "tests/$t.c" -lm
    "build/host/$t"
done
cc -O2 -w -ffp-contract=off $HALI -Ibuild/gen -o build/host/regress tests/regress.c -lm
build/host/regress tests/golden.txt tests/cpu_baseline.txt
cc -O2 -w -ffp-contract=off $HALI -Ibuild/gen -o build/host/emu_native web/emu/native_check.c -lm
build/host/emu_native build/host/emu_native.f32
node web/emu/emu_test.mjs build/emu/felucca.wasm build/host/emu_native.f32
cc -O1 -w -ffp-contract=off $HALI -Ibuild/gen -o build/host/bgirl_test ../tests/bgirl_test.c -lm
build/host/bgirl_test
node web/test_backup.mjs
