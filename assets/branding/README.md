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

`bgirl-lcd-transparent.png` is the unchanged dedicated 1679×448 RGBA LCD
master supplied for Doug's PR48 boot repair (README/browser still use chrome).
SHA-256: `ecc8904703b2b8ee16ca059f57b8e75827b88515fcfa64992e276a611d875ea1`.
Coordinator Qwen Image 2.1 edit job `c4abae5d-3640-46c8-ad35-9b2815605504`;
receipt records background removal: `magick original.jpg -alpha on -fuzz 12%
-transparent white -trim +repage transparent.png`. Original/prompt/receipt are
retained by the coordinator; no generation was performed in this repair.
The simplified B intentionally has no lower counter; no additional art edits.
The separate artwork-rights limitations above apply to this derivative too.

`python3 tools/package_logo.py --preview .powderline/boot.png` verifies this
master hash, crops to alpha bounds, proportionally downsamples to216px wide,
thresholds alpha at128, and trims the resulting visible bounds. It reserves a
clear index and quantizes only visible colors to15 RGB565 colors using octree,
retaining the small mint FM1 and record center. No background RGB is baked in.
The216×58 packed asset is6,264 pixel bytes +32 palette bytes +16 alpha bytes =
**6,312 bytes**, down5,384 from the rejected chrome boot (11,696), and9,976
from the old16,288-byte asset. No browser/README logo pixels are changed.

The existing native renderer already blends indexed alpha against `T_BG`;
clear pixels now use that path. Its centered32-row strips,64-byte local cache,
existing canvas, boot timer and audio/control behavior are unchanged. GREY/MONO
retain existing conversion. This is static resource evidence, not target fit.

[Native-size and browser review evidence](../../docs/CHROME_BRANDING_VERIFICATION.md).
Human visual approval, hardware verification and licensing remain pending.
