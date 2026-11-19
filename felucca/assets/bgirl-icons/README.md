# Original BGIRL section icons

Three original monochrome geometric designs, GPL-3.0-only like this project.
No third-party icon or artist's drawing was traced/imported.

- **STAGE:** a simple three-point crown, per Doug's direction. This is a section
  glyph, not a crown added to any dancer or animation.
- **LAB:** two offset operator boxes and a right-angle modulation connection.
- **CUT:** a six-cell rhythmic step grid.

Each has separately authored12/16/24px SVG geometry. The12px crown in particular
uses half-pixel positions and a distinct stroke/point arrangement for clear valleys;
its source is not a blind downscale of the24px file. SVGs contain monochrome
strokes only; firmware theme tokens supply all color.

`tools/bgirl_sections.py` reads this restricted SVG vocabulary at build time.
`tools/gen_aa_icons.py` appends ICON_BG_STAGE/LAB/CUT **after all existing IDs**,
then emits the usual AI*_DATA/IDX/INK/OFF and shared Huffman code. The existing
`icon_cell`, `cv_icon_in` and `cv_alpha_hc` render the generated cells. No runtime
SVG interpreter, extra framebuffer, or separate icon rendering API.

`inherited-alpha-sha256.json` pins all150 previously drawn icon cells from
iteration-one77a2756: IDs and decoded4-bit alpha remain unchanged even though the
shared compressed stream changes. `tests/test_studio_icons.py` checks those hashes,
all9 new SVG/decoder compositions, nonempty/distinct silhouettes and padding.
The native icon sheet produced by `tests/stage-render.c` uses the real firmware
renderer, not the source rasterizer's preview.

## Vinyl beat icon

`bg_vinyl-{12,16,24}.svg` is an original, separately fitted concentric vinyl/label
mark. It appends a new icon ID; inherited TEMPO/TAP and the three section IDs stay
unchanged. Only the header beat marker switches to vinyl. The existing stationary
beat/header behavior is preserved; this is not a spinning animation or an audio
metronome change. Build-time circle strokes are packed through the same decoder.
