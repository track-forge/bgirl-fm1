# BGIRL FM1 wordmark

The acid-lime graffiti wordmark was selected by Doug Smith for the BGIRL FM1
browser-preview branding in [issue #16](https://github.com/track-forge/bgirl-fm1/issues/16).

## Files

- `bgirl-fm1-wordmark-source.jpg` is the selected 1024×768 source artwork.
- `bgirl-fm1-wordmark.png` is a source-preserving 788×509 RGBA crop with the
  connected off-white background removed for responsive dark/light placement.

Source post:
https://www.moltazine.com/post/7d51db4e-f52a-46a9-8a39-d3c8e5b7362e

Source SHA-256:
`032efc2813a0b0278663193fe20810e0c96b3e57671c203730aa924cbdabe265`

Transparent derivative SHA-256:
`0d1b4f0b96c54cce8d5dc4693e93e1eb1c8202dce3209f653c83332fdf0c2d0a`

## Transparent-background conversion

ImageMagick 7.1.1-47 removed only the connected off-white exterior and then
trimmed the transparent canvas. The lettering, interior white highlights,
palette, splatters and proportions remain raster pixels from the selected source.

```sh
magick bgirl-fm1-wordmark-source.jpg \
  -bordercolor white -border 1 -alpha on -channel RGBA \
  -fuzz 8% -fill none -draw 'alpha 0,0 floodfill' \
  -shave 1x1 -trim +repage bgirl-fm1-wordmark.png
```

The PNG was checked over both near-black and off-white backgrounds. It is raster
artwork, not a vector source.

## Provenance and use terms

The source was generated with the pinned **Qwen Image 2.1** evaluation workflow
recorded by the canonical Moltazine post. The workflow/model terms identify
research/evaluation use and require a separate commercial license for commercial
use. These image files are **not** licensed under this repository's GPL-3.0-only
code license, and this repository does not label them CC0 or make a separate
commercial-rights claim.

Before using the mark as a commercial production brand, review the applicable
workflow/model terms and obtain any required license. Its use here is the selected
project-preview artwork with explicit provenance and limitations.
