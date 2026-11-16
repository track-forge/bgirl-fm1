# Public G6 demo recording

Doug explicitly authorized this recording and its six supplied chops to ship in
the public repository and browser/Pages build on 2026-11-16. This authorization
is scoped to G6, not the other five recordings supplied during issue #13.
Audio is supplied material; the firmware's GPL notice is not a new claim of
recording authorship or an invented CC0 grant.

- Source: supplied `T75_S008_candidate_float.wav`, 44.1 kHz float stereo.
- Six supplied transient chops: `slices/T75_S008/`, unchanged including their
  existing 3 ms edge fades. No new detection, trimming or speech generation.
- `slices/manifest.json`: the G6-only subset of the supplied slicing manifest.
  The historical reel name records slicing provenance; no reel is needed to build.
- Reproduction: from repository root, run `./build.sh --web`, or
  `python3 tools/package_vocals.py --input-dir assets/vocals/g6 --manifest
  assets/vocals/g6/slices/manifest.json --group-id 6 --output local/public-g6`
  (join the last command onto one line).
- Existing deterministic packaging: mono, 22.05 kHz, fixed headroom, IMA4;
  checkpointed whole + chops. `bank.json` records exact source hashes/bounds.
- Runtime mapping: WHOLE/W01 = this original, G6/C01..C06 = these chops. Compact
  VIEW index 1 carries the G6 label. The full private bank uses VIEW 6 for G6;
  bank fingerprints prevent silent cross-bank project substitution.

Public rendering includes the exact two-bar G6 chop rhythm and octave-down
GRAIN / CLOUD PAD carrier. No other supplied recordings are required or included.
