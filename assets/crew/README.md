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

Twelve source-order poses at0,10,...110 (0–4.583333s); terminal footage is not added
as a duplicate endpoint. Gray<96;3×3 median before resampling; fixed **all124-frame
union bounds**, aspect-preserving square,24px source padding, common floor anchor;
LANCZOS→>=128, MSB-first row-major. No per-pose recentering. All resulting masks
retain an empty pixel border. Some source frames in the upright clips/windmill
already reach source y0: output padding avoids *additional* clipping, but does not
reconstruct anything outside the original video. Hair/fingers merge at1-bit scale.

96px fits the existing122px HOME graph with four engine cards, actual header/footer
and a track/move/state label. Source motion reads as windmill, stepping/turning,
arm-sweep/sway and prone worm; not motion-capture fidelity or human art approval.
Floorwork remains the unchanged independent fifth pack from ARP DANCE. Stable
T1–T6 assignments: WINDMILL, STEP/TURN, WAVE-ISH, WORM, FLOORWORK, WINDMILL. A track
label supplies identity even when packs are shared. No miniature crew is rendered.

Each bank13,824B; four new banks55,296B linked once per clip, not per track. Existing
FLOORWORK remains13,824B and is shared with ARP DANCE. No framebuffer added. Every
changed graph still transfers58,560 RGB565 bytes, before commands/other UI regions.

Every last→first seam is an **intentional hard cut**, not a seamless-loop claim.
The per-pack static seam PNG also exposes11→0; standalone playback has manual
steps/seam,120BPM/four-beat phrasing and idle/mute/accent treatments at actual240px.
Its timer/chrome are review aids, not firmware timing/fonts or stored choreography.
Doug's subjective art/seam approval remains pending.
