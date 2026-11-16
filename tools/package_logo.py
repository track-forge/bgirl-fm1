#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Pack the existing PR19 wordmark as a bounded, 4-bit RGB565 splash image.
Artwork has separate use terms: assets/branding/README.md (not GPL artwork).
"""
import argparse
import hashlib
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'assets/branding/bgirl-fm1-wordmark.png'
SOURCE_SHA = '0d1b4f0b96c54cce8d5dc4693e93e1eb1c8202dce3209f653c83332fdf0c2d0a'
WIDTH = 224
BONE = (216, 210, 191, 255)


def package(out=None, preview=None):
    data = SOURCE.read_bytes()
    if hashlib.sha256(data).hexdigest() != SOURCE_SHA:
        raise ValueError('wordmark changed: review artwork/provenance before repackaging')
    with Image.open(SOURCE) as source:
        height = round(source.height * WIDTH / source.width)
        logo = source.convert('RGBA').resize((WIDTH, height), Image.Resampling.LANCZOS)
    # An opaque bone plate retains the original black outlines on dark palettes.
    rgb = Image.alpha_composite(Image.new('RGBA', logo.size, BONE), logo).convert('RGB')
    indexed = rgb.quantize(colors=16, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    palette = indexed.getpalette()[:48]
    colors = [((palette[k] >> 3) << 11) | ((palette[k+1] >> 2) << 5) | (palette[k+2] >> 3)
              for k in range(0, 48, 3)]
    indices = indexed.tobytes()
    assert len(indices) % 2 == 0 and max(indices) < 16
    packed = bytes((indices[k] << 4) | indices[k+1] for k in range(0, len(indices), 2))
    lines = ['/* Generated from the PR19 wordmark by tools/package_logo.py.',
             ' * Artwork is NOT GPL: see assets/branding/README.md for provenance/use terms.',
             f' * Source SHA-256: {SOURCE_SHA} */', '#pragma once',
             f'#define BG_LOGO_W {WIDTH}u', f'#define BG_LOGO_H {height}u',
             'static const uint16_t BG_LOGO_PALETTE[16] = {',
             '    '+', '.join(f'0x{c:04x}' for c in colors)+'};',
             f'static const uint8_t BG_LOGO_PIXELS[{len(packed)}] = {{']
    for k in range(0, len(packed), 24):
        lines.append('    '+', '.join(f'0x{v:02x}' for v in packed[k:k+24])+',')
    lines += ['};', '']
    dest = Path(out) if out else ROOT / 'felucca/build/gen/bgirl_logo.h'
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text('\n'.join(lines))
    if preview:
        # Preview the exact RGB565 values, not the pre-quantization source.
        result = Image.new('RGB', indexed.size)
        result.putdata([((colors[i] >> 11)*255//31, ((colors[i] >> 5)&63)*255//63,
                         (colors[i]&31)*255//31) for i in indices])
        result.save(preview)
    print(f'splash logo: {WIDTH}x{height}, {len(packed)+32} B indexed RGB565; source {SOURCE_SHA}')
    return packed, colors


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out')
    parser.add_argument('--preview')
    args = parser.parse_args()
    package(args.out, args.preview)
