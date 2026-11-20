#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Pack the selected chrome wordmark as a bounded, 4-bit RGB565 splash image.
Artwork has separate use terms: assets/branding/README.md (not GPL artwork).
"""
import argparse
import hashlib
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'assets/branding/chrome-source.jpg'
SOURCE_SHA = 'db3d2a32353e25522d8e5117b3caeb2f75115eb0cd6ce163efe424492c7b36cb'
WIDTH = 224


def package(out=None, preview=None):
    data = SOURCE.read_bytes()
    if hashlib.sha256(data).hexdigest() != SOURCE_SHA:
        raise ValueError('wordmark changed: review artwork/provenance before repackaging')
    with Image.open(SOURCE) as source:
        assert source.size == (1952, 544), 'review crop coordinates for changed master'
        # Separate FM1 stays separate, enlarged below the name for LCD readability.
        # Coordinates are in the unchanged 1952x544 master, not a stock reference.
        name = source.convert('RGBA')
        name.paste((17, 10, 28, 255), (1648, 30, 1790, 145))
        name = name.resize((WIDTH, 62), Image.Resampling.LANCZOS)
        fm1 = source.crop((1650, 40, 1788, 138)).convert('RGBA')
        fm1.thumbnail((42, 30), Image.Resampling.LANCZOS)
        height = 104
        logo = Image.new('RGBA', (WIDTH, height), (17, 10, 28, 255))
        logo.paste(name, (0, 0))
        logo.paste(fm1, ((WIDTH - fm1.width)//2, 72))
    # Retain the existing indexed RGBA interface; this selected master is opaque.
    indexed = logo.quantize(colors=16, method=Image.Quantize.FASTOCTREE, dither=Image.Dither.NONE)
    palette = indexed.getpalette('RGBA')[:64]
    colors = [((palette[k] >> 3) << 11) | ((palette[k+1] >> 2) << 5) | (palette[k+2] >> 3)
              for k in range(0, 64, 4)]
    alpha = palette[3:64:4]
    indices = indexed.tobytes()
    assert len(indices) % 2 == 0 and max(indices) < 16
    packed = bytes((indices[k] << 4) | indices[k+1] for k in range(0, len(indices), 2))
    lines = ['/* Generated from the selected chrome wordmark by tools/package_logo.py.',
             ' * Artwork is NOT GPL: see assets/branding/README.md for provenance/use terms.',
             f' * Source SHA-256: {SOURCE_SHA} */', '#pragma once',
             f'#define BG_LOGO_W {WIDTH}u', f'#define BG_LOGO_H {height}u',
             'static const uint16_t BG_LOGO_PALETTE[16] = {',
             '    '+', '.join(f'0x{c:04x}' for c in colors)+'};',
             'static const uint8_t BG_LOGO_ALPHA[16] = {',
             '    '+', '.join(f'0x{a:02x}' for a in alpha)+'};',
             f'static const uint8_t BG_LOGO_PIXELS[{len(packed)}] = {{']
    for k in range(0, len(packed), 24):
        lines.append('    '+', '.join(f'0x{v:02x}' for v in packed[k:k+24])+',')
    lines += ['};', '']
    dest = Path(out) if out else ROOT / 'felucca/build/gen/bgirl_logo.h'
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text('\n'.join(lines))
    if preview:
        # Preview the exact RGB565 values, not the pre-quantization source.
        result = Image.new('RGBA', indexed.size)
        result.putdata([((colors[i] >> 11)*255//31, ((colors[i] >> 5)&63)*255//63,
                         (colors[i]&31)*255//31, alpha[i]) for i in indices])
        result.save(preview)
    print(f'splash logo: {WIDTH}x{height}, {len(packed)+48} B indexed RGB565 + alpha; source {SOURCE_SHA}')
    return packed, colors


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out')
    parser.add_argument('--preview')
    args = parser.parse_args()
    package(args.out, args.preview)
