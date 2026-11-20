# B-GIRL FM1 chrome wordmark

Doug selected this pink reflective metal / lavender bevel / mint shutter-shades
raster in [issue #47](https://github.com/track-forge/bgirl-fm1/issues/47), superseding
the graffiti selection in #16. FM1 remains separate from the L; the B contains the
simplified vinyl/tonearm and the R the crown cutout.

## Source and derivatives

- `chrome-source.jpg`: unchanged 1952×544 selected local master, SHA-256
  `db3d2a32353e25522d8e5117b3caeb2f75115eb0cd6ce163efe424492c7b36cb`.
- `bgirl-fm1-wordmark.png`: lossless RGB decode of that JPEG for README/browser;
  original aspect ratio, no transparency or vector-master claim.
- `bgirl-fm1-wordmark-source.jpg`: historical #16 source, not the active selection.

Canonical post: https://www.moltazine.com/post/9a54af0a-0d30-464e-8163-36170694ac26

Doug designed the original wordmark and directed edits. Glade produced the selected
raster with Qwen Image 2.1 two-reference edit, job
`61964d66-384b-4551-b80c-d3224a64f4c8`, using the refined shutter logo as base and
Doug's supplied 1980s chrome material reference. No new generation was performed;
the unrelated stock/material reference is not redistributed.

## Separate artwork rights — unresolved

Qwen workflow/model terms identify research/evaluation use and require separate
commercial licensing. The supplied chrome reference's reuse terms are not
established here. These images are **not** GPL-3.0-only artwork, CC0, or cleared
for commercial reuse by this integration. Human licensing resolution remains
required before commercial brand/distribution use. Upstream code/component
notices remain intact. No package/release or hardware approval is implied.

## Native boot derivative

`python3 tools/package_logo.py --preview .powderline/boot.png` verifies the master
hash and reproducibly generates `felucca/build/gen/bgirl_logo.h`. The main name
is Lanczos-resized proportionally to 224×62 (rounded from 62.43). FM1 is removed
from its tiny original location and placed separately below at up to42×30,
retaining its aspect ratio. Exact source rectangles are recorded in the tool.
The name, record and crown are neither stretched nor cropped. The original
near-black purple background is retained rather than claiming transparency.

The224×104 derivative uses16 colors, RGB565 plus alpha, two4-bit indices/byte:
11,648 pixel bytes +32 palette bytes +16 alpha bytes = **11,696 bytes**, down
4,592 bytes from the previous16,288. Firmware uses the unchanged32-row existing
canvas path and64 bytes of local color cache; no extra framebuffer, allocation,
boot timer/delay or audio/control changes. This is source/static cost evidence,
not a device linker-map, RAM high-water, timing or physical readability result.
GREY/MONO still use the existing grayscale conversion.

[Native-size and browser review evidence](../../docs/CHROME_BRANDING_VERIFICATION.md).
Human visual approval, actual hardware build/flash and licensing remain pending.
