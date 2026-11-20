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
SOURCE = ROOT / 'assets/branding/bgirl-lcd-transparent.png'
SOURCE_SHA = 'ecc8904703b2b8ee16ca059f57b8e75827b88515fcfa64992e276a611d875ea1'
WIDTH = 216


def package(out=None, preview=None):
    data = SOURCE.read_bytes()
    if hashlib.sha256(data).hexdigest() != SOURCE_SHA:
        raise ValueError('wordmark changed: review artwork/provenance before repackaging')
    with Image.open(SOURCE) as source:
        assert source.size == (1679, 448), 'review changed LCD master'
        # Trim by actual alpha, never by RGB or a baked background color.
        logo = source.convert('RGBA')
        logo = logo.crop(logo.getchannel('A').getbbox())
        logo.thumbnail((WIDTH, 216), Image.Resampling.LANCZOS)
    # Reserve index 0 for clear pixels; quantize only visible RGB. Quantizing
    # RGBA directly can merge clear/opaque colors and lose background holes.
    mask = logo.getchannel('A').point(lambda a: 255 if a >= 128 else 0)
    bounds = mask.getbbox()
    logo, mask = logo.crop(bounds), mask.crop(bounds)
    width, height = logo.size
    visible = [rgb for rgb, a in zip(logo.convert('RGB').getdata(), mask.getdata()) if a]
    samples = Image.new('RGB', (len(visible), 1))
    samples.putdata(visible)
    quant = samples.quantize(colors=15, method=Image.Quantize.FASTOCTREE)
    rgb = quant.getpalette()[:45]
    palette = [0, 0, 0, 0]
    for k in range(0, 45, 3):
        palette.extend(rgb[k:k+3] + [255])
    cursor = iter(quant.tobytes())
    indices = bytes(next(cursor)+1 if a else 0 for a in mask.getdata())
    colors = [((palette[k] >> 3) << 11) | ((palette[k+1] >> 2) << 5) | (palette[k+2] >> 3)
              for k in range(0, 64, 4)]
    alpha = palette[3:64:4]
    assert max(indices) < 16
    indices += b'\0' if len(indices) % 2 else b''
    packed = bytes((indices[k] << 4) | indices[k+1] for k in range(0, len(indices), 2))
    lines = ['/* Generated from the selected chrome wordmark by tools/package_logo.py.',
             ' * Artwork is NOT GPL: see assets/branding/README.md for provenance/use terms.',
             f' * Source SHA-256: {SOURCE_SHA} */', '#pragma once',
             f'#define BG_LOGO_W {width}u', f'#define BG_LOGO_H {height}u',
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
        result = Image.new('RGBA', (width, height))
        result.putdata([((colors[i] >> 11)*255//31, ((colors[i] >> 5)&63)*255//63,
                         (colors[i]&31)*255//31, alpha[i]) for i in indices[:width*height]])
        result.save(preview)
    print(f'splash logo: {width}x{height}, {len(packed)+48} B indexed RGB565 + alpha; source {SOURCE_SHA}')
    return packed, colors


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out')
    parser.add_argument('--preview')
    args = parser.parse_args()
    package(args.out, args.preview)
