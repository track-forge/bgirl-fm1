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
cat tests/golden.txt ../tests/signature-golden.txt > build/host/bgirl-golden.txt
build/host/regress build/host/bgirl-golden.txt tests/cpu_baseline.txt
cc -O2 -w -ffp-contract=off $HALI -Ibuild/gen -o build/host/emu_native web/emu/native_check.c -lm
build/host/emu_native build/host/emu_native.f32
node web/emu/emu_test.mjs build/emu/felucca.wasm build/host/emu_native.f32
cc -O1 -w -ffp-contract=off $HALI -Ibuild/gen -o build/host/bgirl_test ../tests/bgirl_test.c -lm
build/host/bgirl_test
node web/test_backup.mjs
cc -O1 -w -ffp-contract=off $HALI -Ibuild/gen -o build/host/vocoder_test ../tests/vocoder_test.c -lm
build/host/vocoder_test
cc -O2 -w -ffp-contract=off $HALI -Ibuild/gen -pthread -o build/host/vocoder_dual_test ../tests/vocoder_dual_test.c -lm
build/host/vocoder_dual_test

# Startup vocoder promotion must leave the disabled build's original demo intact.
cc -O1 -w -ffp-contract=off -DBGIRL_VOCODER=0 $HALI -Ibuild/gen -o build/host/bgirl_disabled_test ../tests/bgirl_test.c -lm
build/host/bgirl_disabled_test

python3 ../tests/test_package_vocals.py
cc -O1 -w -ffp-contract=off $HALI -Ibuild/gen -o build/host/vocal_bank_test ../tests/vocal_bank_test.c -lm
build/host/vocal_bank_test
cc -O1 -w -ffp-contract=off -DBGIRL_VOCODER=0 $HALI -Ibuild/gen -o build/host/vocal_bank_disabled_test ../tests/vocal_bank_test.c -lm
build/host/vocal_bank_disabled_test

cd ..
./tests/run_dance_tests.sh

# Battle Circuit settings/ID and framebuffer compatibility (host only).
cc -O1 -w -ffp-contract=off -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen -o felucca/build/host/battle_ui_test tests/battle-ui-test.c -lm
felucca/build/host/battle_ui_test
python3 felucca/tools/gen_ui_palettes.py --report

python3 tests/test_package_logo.py

# Signature bank behavioral contracts and serial/threaded hat dispatch.
python3 tools/signature_bank.py --check
cc -O1 -w -ffp-contract=off -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen -o felucca/build/host/signature_test tests/signature_test.c -lm
felucca/build/host/signature_test
cc -O2 -w -ffp-contract=off -pthread -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen -o felucca/build/host/signature_dual_test tests/signature_dual_test.c -lm
felucca/build/host/signature_dual_test
