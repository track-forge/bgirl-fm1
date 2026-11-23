#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Stage the Pages homepage and Studio, optionally with a pinned installer bundle.
set -eu
if [ "$#" -ne 2 ] && [ "$#" -ne 4 ]; then
    echo "Usage: $0 BGIRL_WEB_BUILD NEW_OUTPUT_DIRECTORY [BUNDLE_DIR VERSION]" >&2
    exit 2
fi
studio=$1
output=$2
test -s "$studio/play/felucca.wasm"
if [ "$#" -eq 4 ]; then
    bundle=$(cd "$3" && pwd)
    version=$4
    # Check exactly the two expected files, not arbitrary names from the manifest.
    python3 - "$bundle" "$version" <<'PYTHON'
import hashlib
import re
import sys
from pathlib import Path

bundle, version = Path(sys.argv[1]), sys.argv[2]
if not re.fullmatch(r"v[0-9]+[.][0-9]+[.][0-9]+(?:-[A-Za-z0-9.-]+)?", version):
    sys.exit(f"Invalid installer version: {version}")
expected = (bundle / "SHA256SUMS").read_text().splitlines()
actual = [
    f"{hashlib.sha256((bundle / name).read_bytes()).hexdigest()}  {name}"
    for name in (f"felucca-{version}.fwsc", "miami.slot")
]
if actual != expected:
    sys.exit("Installer bundle does not match SHA256SUMS")
PYTHON
fi
cd "$(dirname "$0")/.."
mkdir "$output"
cp felucca/web/hortator/*.js felucca/web/hortator/*.css "$output/"
homepage=felucca/web/hortator/index.html
if [ "$#" -eq 4 ]; then
    python3 felucca/web/make_site.py "$bundle/felucca-$version.fwsc" "$version" "$output" \
        --miami "$bundle/miami.slot" --installer-only
    cp "$homepage" "$output/index.html"
else
    sed '/<!-- BGIRL-INSTALL-BEGIN -->/,/<!-- BGIRL-INSTALL-END -->/d' "$homepage" > "$output/index.html"
fi
mkdir "$output/fonts"
cp felucca/assets/fonts/Michroma-Regular.ttf felucca/assets/fonts/OFL-Michroma.txt "$output/fonts/"
cp LICENSE "$output/"
cp -R felucca/LICENSES "$studio/play" "$output/"
node tools/capture-crew-lcd.mjs "$studio/play/felucca.wasm" > "$output/crew-lcd"
gzip -n "$output/crew-lcd"
git rev-parse HEAD > "$output/build-sha.txt"
