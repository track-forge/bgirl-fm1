# Upstream provenance

Scaffold prepared 2026-11-15.

- Platform: https://github.com/charlesvestal/fm1-base
- Pinned base: `c8e05997ce970875038156d2bbd69e712eac1543`
- Nested https://github.com/hugelton/felucca-bsp pin: `18eadadb8c364abd026accd2b1ea1c14ca347f06`
- `app/app.c`, `app/app.h`, `app/fm1.toml`, `app/words.tsv`, and
  `app/tests/basic.txt` are unchanged copies from that base's `template/`.
- Upstream MPL-2.0 text is retained in `LICENSE.upstream`; preserve copied-file headers.
  No blanket license decision for new original project code has been made.
- Review `base/LICENSING.md` for SDK, font, radio and browser dependencies before
  distribution; they do not all share one license.

Copied config still refers to the upstream template's author/site/package identity.
This is baseline provenance, not BGIRL release metadata. Rebrand and review identity
in milestone 1 before generating anything intended for distribution. Generated
`firmware.json` is ignored until that review; do not publish it as our catalog entry.

Build/test wrappers and documentation are project setup; no upstream release/site
workflow, generated assets, firmware binary, or electro engine was copied in.

## Scaffold verification (2026-11-15)

`./tests/run_tests.sh` passed in this independent repository (exit 0), including
notes/MIDI, save/reboot, audio peak, touch/pressure, screen-reader messages and
simulated Bluetooth. The five copied files were byte-compared against the pinned
template. Shell syntax and Git whitespace checks passed. Browser and device builds
were not attempted. Upstream incomplete-release-metadata warnings are expected at
this unchanged-template stage; they are not evidence of releasable BGIRL firmware.
