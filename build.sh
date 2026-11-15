#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")"
[ "${1:---web}" = --web ] && [ "$#" -le 1 ] || { echo 'Usage: ./build.sh [--web] (browser only)' >&2; exit 2; }
python3 tools/generate.py
exec sh felucca/web/emu/build.sh
