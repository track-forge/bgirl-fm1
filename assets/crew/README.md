# Selected-track CREW packs

Four supplied public videos, creator **stevedore**, all converted; no new generation.
Doug Smith authorized supplied information/videos for public repository sprite work
on2026-11-17. Derived masks, headers, previews and converter use GPL-3.0-only.
Public availability is not the permission basis. Post captions report Qwen Image2.1
stills / MiniMax H3 motion references; those job/model claims are not independently
verified. The original MP4s remain ignored in `local/crew/originals`.

| Clip / source | Original bytes | Preview / contact sheet |
| --- | ---: | --- |
| [Windmill](https://www.moltazine.com/post/795f4882-e0a3-4fec-9de4-4a136c213ebb) |345878|[playback](windmill/preview.html) · [poses](windmill/contact-96.png)|
| [Reference02: step/turn](https://www.moltazine.com/post/2f43ab03-2793-4b31-8ffb-be56f6a2bc0d) |340435|[playback](reference-02/preview.html) · [poses](reference-02/contact-96.png)|
| [Wave-ish (provisional)](https://www.moltazine.com/post/c90acbbd-1cd3-4176-b170-ea9c8f74eb9d) |347962|[playback](wave-ish/preview.html) · [poses](wave-ish/contact-96.png)|
| [Worm](https://www.moltazine.com/post/d0e1e91c-9280-42fc-9d1d-86ecc730d481) |437886|[playback](worm/preview.html) · [poses](worm/contact-96.png)|

All originals: MP4,608×352,24/1fps,124 decoded frames,5.166667s, stream time base
1/12288. Hashes, full attribution, source URLs and frame indices are in
[sources.json](sources.json) and each manifest. Source captions say5 seconds;
actual decoded duration is recorded separately. Public media resolver:
`https://www.moltazine.com/api/v1/posts/<post-id>/media`. Resolve/read via the
supported Moltazine post API/CLI; no credential or expiring URL is committed.

## Recipe / independent repeat

FFmpeg/ffprobe7.1.5 and Pillow12.1.1, enforced by the converter. Network is used
only to retrieve the supplied originals; conversion, builds, tests and runtime
consume local/committed files offline. No paid generation or publication API call.

```sh
python3 tools/convert_crew.py local/crew/originals /tmp/crew-repeat
# Compare every generated file with its same path under assets/crew.
python3 tests/test_convert_crew.py
```

Individual replacement example (requires explicit source hash and permission):

```sh
python3 tools/convert_dance.py SOURCE.mp4 OUTPUT --clip authorized-name --sizes 96 \
  --sha256 SOURCE_SHA --source-url PUBLIC_POST_URL --rights 'Authorized derivative redistribution record' \
  --indices 0,10,20,30,40,50,60,70,80,90,100,110
```

## Current framing and loop (Doug's 2026-11-18 feedback)

Twelve source-order masks per pack, played **0→11→1→0** (22 distinct steps per
four-beat cycle, no repeated endpoint holds). All CREW tracks use this forward/back
walk, including the shared FLOORWORK bank; the separate ARP DANCE animation and
its masks remain unchanged. Static ghosts now show10→11,11→10 and1→0; standalone
review playback uses the same22-step/four-beat mapping as firmware. Direction
reverses at the ends rather than jumping11→0; this is not a motion-smoothness claim.

- **T1 WINDMILL (also shared by T6):** re-extracted from the original MP4, same
  indices0,10,...110. The old470px square reserved the entire sideways travel of
  the source. Follow each pose's horizontal bounding-box center with a **constant
  364px square / fixed floor anchor** and16px source padding. This makes the dancer
  **470/364 =1.291× larger linearly**, without per-pose zoom, stretching or clipping
  any selected source silhouette. Crop offsets/bounds are in the manifest.
- **T4 WORM:** only frames24,31,37,44,50,57,63,70,76,83,89,96 from the original24fps
  video: **1.000–4.000s inclusive**, twelve evenly spaced nearest-frame samples.
  Bounds use these selected poses, not the discarded lead-in/tail.
- STEP/TURN and WAVE-ISH masks are byte-identical to the initial conversion;
  only their review playback/turnaround metadata changed.

Recipes live in[sources.json](sources.json); `convert_crew.py` supplies ping-pong
review metadata. Gray<96,3×3 median before resampling; aspect-preserving square,
common floor anchor, LANCZOS→>=128, MSB-first row-major. All masks retain an empty
pixel border. Non-windmill clips use a fixed union crop,24px source padding.
Some original frames already meet source y0; conversion never reconstructs
missing footage. Hair/fingers still merge at1-bit scale.

96px fits the existing122px HOME graph with four engine cards, actual header/footer
and a track/move/state label. Source motion reads as windmill, stepping/turning,
arm-sweep/sway and prone worm; not motion-capture fidelity or human art approval.
Floorwork remains the unchanged independent fifth pack from ARP DANCE. Stable
T1–T6 assignments: WINDMILL, STEP/TURN, WAVE-ISH, WORM, FLOORWORK, WINDMILL. A track
label supplies identity even when packs are shared. No miniature crew is rendered.

Each bank13,824B; four new banks55,296B linked once per clip, not per track. Existing
FLOORWORK remains13,824B and is shared with ARP DANCE. No framebuffer added. Every
changed graph still transfers58,560 RGB565 bytes, before commands/other UI regions.

The converter and runtime tests check both turnaround directions, repeatable
assets, pixel borders, exact selected-track compositions and unchanged audio/RNG.
Subjective review of this new crop/loop remains with Doug.
