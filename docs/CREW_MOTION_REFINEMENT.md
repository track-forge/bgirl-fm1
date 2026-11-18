# CREW animation refinement — 2026-11-18

Doug requested a larger T1 groundwork silhouette, worm footage restricted to
seconds1–4, forward/backward frame playback, and an update to the existing private
preview. HOME-hold/menu navigation remains explicitly deferred.

## Changes

- Re-extract WINDMILL from its original authorized MP4 (same12 source poses).
  Replace the470px all-footage union square with a364px constant-scale square,
  following the horizontal silhouette center and keeping one common floor anchor.
  This increases linear size29.1%, preserves aspect ratio, and contains every
  selected source silhouette with padding. No per-pose zoom or limb clipping.
  The same bank is shared by T1 and T6, so both receive the improvement.
- Re-extract WORM using24fps frames24,31,37,44,50,57,63,70,76,83,89,96: nearest-frame
  samples over1.000–4.000s inclusive. Discard the rest of the footage from playback;
  use the selected interval's bounds, not the discarded lead-in/tail.
- All CREW dancers walk0..11,10..1,0 again. No repeated endpoint holds or11→0 hard
  cut. One complete forward/back cycle spans the existing four-beat phrase.
  Twelve stored masks per bank remain sufficient; all22 steps fit the normal
  display cadence at the default demo tempo. High tempos may skip current poses
  under the unchanged15Hz redraw cap; there is no backlog/replay.
- STEP/TURN and WAVE-ISH masks, original FLOORWORK/ARP DANCE, audio producer/DSP,
  demo music, settings and project formats are unchanged.

## Reproduction and checks

```sh
python3 tools/convert_crew.py local/crew/originals /tmp/crew-repeat
python3 tests/test_convert_crew.py
./build.sh --web
./tests/run_tests.sh
node tests/crew-browser.cjs
node tests/dance-browser.cjs
```

Recipes/provenance/hashes and source/crop bounds are committed under
[assets/crew](../assets/crew/README.md). Offline repeated conversion must reproduce
all24 generated files byte-for-byte. Native tests exhaust all256 phase bins,
forward/reverse endpoints and wrap, then compare2904 exact full-stage compositions
across six tracks/palettes/styles. Existing inactive/hidden/rate-cap checks and
512000-frame audio/RNG preservation continue to apply. Converter tests verify the
1–4s recipe, crop containment, empty pixel borders, unchanged neighboring masks,
header/bin hashes and22-step preview metadata.

## Preview integration

Only the previously authorized8448 tailnet preview is refreshed, using a clean
exact-remote-head build and frozen SHA-named artifact. Preserve all37 archived
listening files unchanged; verify every served payload hash and unchanged complete
Serve/Funnel routing. Snapshot the currenta30c38d service drop-in/artifact for
rollback separately from the prior38a429/Mega rollback records. Exercise CREW on
the actual HTTPS AudioWorklet preview; compare screenshots against the new masks.
Exact head, build hash, test outcomes, CI and rollback location are recorded in
PR29 and the handoff notebook. No merge/device build/flash or public release.
