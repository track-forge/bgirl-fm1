#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
./build.sh --host
app/build/host/bgirl_host app/tests/basic.txt app/build/host
cc -std=c99 -O2 -Wall -Wextra -Werror -ffp-contract=off -Ibase/include \
  tests/electro_test.c app/dsp/electro.c app/dsp/vocoder.c -lm -o app/build/host/electro_test
app/build/host/electro_test
app/build/host/bgirl_host app/tests/electro.txt app/build/host
