# FLOORWORK — one move, twelve poses

Source: [stevedore's supplied Moltazine post](https://www.moltazine.com/post/a559b395-af6b-49c0-b6e9-08b11a202269).
Reported provenance: Qwen Image 2.1 still → MiniMax H3 first-frame animation,
without a loop constraint. Those are the post's claims, not independently
verified job settings. No generation was performed for this feature.

**Permission:** Doug Smith explicitly authorized public inclusion of the twelve
derived masks in the GPL-3.0-only repository on 2026-11-16. This records the
applicable supplied-art authorization separately from the public post's provenance;
public availability alone was not treated as permission. Derived masks, generated
previews and converter are GPL-3.0-only. The original MP4 is not committed.

## Reproduce / replace

Obtain the MP4 from the source post and verify its SHA-256:
`acb274cd20f88d92a624453eb4b2eb4d1f3d7e2f3c687988e39475b207fd3d51`.
It is 180,213 bytes, 608×352, 24fps, 96 frames / 4 seconds. Audio is ignored.
Use **FFmpeg/ffprobe 7.1.5** and **Pillow 12.1.1**; the converter rejects other
FFmpeg/Pillow versions. These tools are only needed for offline art work, not
instrument builds, CI, or runtime.

```sh
python3 tools/convert_dance.py /path/to/source.mp4 assets/dance
# Independent repeat: every generated file must match byte for byte.
python3 tools/convert_dance.py /path/to/source.mp4 /tmp/dance-repeat
for f in /tmp/dance-repeat/*; do cmp "$f" "assets/dance/${f##*/}"; done
./build.sh --web
./tests/run_tests.sh
```

For another authorized move (not part of this milestone):

```sh
python3 tools/convert_dance.py /path/to/new.mp4 /tmp/new-dance \
  --sha256 NEW_SOURCE_SHA256 --source-url https://example.org/new-source \
  --rights 'Applicable derivative redistribution permission / license' \
  --indices 0,8,16,24,32,40,48,56,64,72,80,88
```

Review all outputs and the seam, then replace generated files in `assets/dance/`.
The new input must have twelve distinct in-range frame indices. Keep the manifest,
contact sheets and permission record with the art. There is no runtime pack loader.

## Framing, sampling and size decision

- Fixed union of dark bounds **over all 96 frames**: `[144,14,459,324)`.
- One square stage: left132, top−3, side339 source pixels. Padding is deliberate,
  not a square center crop; it retains all hands/feet, aspect ratio and a common
  floor reference (source y324 → output y93 at96 / y62 at64). No per-pose centering.
- Segment gray `<96`, simplify with one **3×3 median**, LANCZOS-resample, threshold
  `>=128`. No invisible hand edits, hair painting, inbetweens or paid rerenders.
- Frames **0,8,16,24,32,40,48,56,64,72,80,88**, source-order, no duplicate endpoint.
- Both comparisons contain twelve distinct masks. **96×96 chosen**: better hand/
  forearm negative space and hair/limb definition than64 at the actual1× scale.
  192×192 cannot fit the existing122px graph with controls/header/footer intact.
  The centered96px stage fits at x72..167, graph-local y22..117 (screen98..193).
  LARGE retains this stage, with bigger card labels rather than a62px title strip.
- **11→0 is an intentional hard cut** from bent-leg floorwork to the upright-leg
  starting pose. The source was not generated as a loop. It is a rhythmic pose-bank
  jump, not a seamless film loop; ghost makes the departing11 visible under0.
  This is accepted for the bounded MVP, with the discontinuity explicitly exposed
  for art review. No claim of human seam approval or continuous choreography.

[96 contact sheet](contact-96.png) · [64 comparison](contact-64.png) ·
[96 ghost seam:10→11,11→0,0→1](seam-ghost-96.png).
Open **preview.html** directly in a browser (standalone, no credentials/server)
for intended panel scale, manual poses/seam, CRISP/GHOST and500/125ms-per-pose loops.
This offline preview's timer is not used by the instrument.

## Binary contract

Row-major, **MSB first**, one bit per pixel, 1=filled, no dithering; row stride12 at96
or8 at64. Twelve concatenated masks,1152 or512 bytes each; no file header. Manifest
records source/recipe/toolchain, anchor and hashes. `bgirl_dance.h` is the exact96
bank as `uint8_t[12][1152]`, checked at C compile time and against the binary by tests.
Runtime consumes only that committed header, no MP4, network, decoding or API.

- Shipping bank: **13,824 bytes**;64 comparison: **6,144 bytes**. Both raw files are
  review/conversion artifacts; only the96 C bank is linked. Ghost reuses the bank.
- No RGB asset bank or additional draw canvas. Existing240×124×2 canvas is reused;
  each changed graph transfers240×122×2 = **58,560 pixel bytes**, plus LCD commands
  and independently changed header/cards/footer. Tiny masks do not imply cheap SPI.
