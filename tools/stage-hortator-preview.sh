#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Overlay the trimmed upstream shell on a fresh, separate tailnet review directory.
set -eu
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 BGIRL_WEB_BUILD NEW_OUTPUT_DIRECTORY" >&2
    exit 2
fi
studio=$1
output=$2
test -s "$studio/play/felucca.wasm"
cd "$(dirname "$0")/.."
mkdir "$output"
cp felucca/web/hortator/*.js felucca/web/hortator/*.css felucca/web/hortator/index.html "$output/"
mkdir "$output/fonts"
cp felucca/assets/fonts/Michroma-Regular.ttf felucca/assets/fonts/OFL-Michroma.txt "$output/fonts/"
cp LICENSE "$output/"
cp -R felucca/LICENSES "$studio/play" "$output/"
node tools/capture-crew-lcd.mjs "$studio/play/felucca.wasm" > "$output/crew-lcd"
gzip -n "$output/crew-lcd"
git rev-parse HEAD > "$output/build-sha.txt"
