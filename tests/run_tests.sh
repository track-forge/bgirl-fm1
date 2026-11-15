#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
./build.sh --host
app/build/host/bgirl_host app/tests/basic.txt app/build/host
