#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Overlay the trimmed upstream shell on a fresh, separate tailnet review directory.
set -eu
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 HORTATOR_CHECKOUT BGIRL_WEB_BUILD NEW_OUTPUT_DIRECTORY" >&2
    exit 2
fi
hortator=$1
studio=$2
output=$3
pin=44c9ad4aa57bebf63f412bd552e57143c30b4065
test "$(git -C "$hortator" rev-parse HEAD)" = "$pin"
test -s "$studio/play/felucca.wasm"
cd "$(dirname "$0")/.."
mkdir "$output"
cp felucca/web/hortator/*.js felucca/web/hortator/*.css felucca/web/hortator/index.html "$output/"
mkdir "$output/fonts"
cp felucca/assets/fonts/Michroma-Regular.ttf felucca/assets/fonts/OFL-Michroma.txt "$output/fonts/"
cp "$hortator/LICENSE" "$hortator/LICENSING.md" "$output/"
cp -R "$hortator/LICENSES" "$studio/play" "$output/"
node tools/capture-crew-lcd.mjs "$studio/play/felucca.wasm" > "$output/crew-lcd"
gzip -n "$output/crew-lcd"
git rev-parse HEAD > "$output/build-sha.txt"
