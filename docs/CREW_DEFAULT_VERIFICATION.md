# CREW default follow-up

2026-11-18: Doug requested making CREW the default and explicitly deferred the
HOME-hold menu/knob interaction quirk. No menu navigation changes in this patch.

## Behavior

- Fresh settings start on CREW, as before.
- Older unmarked settings now adopt CREW once, including saved SCOPE/TRACKS/LEVELS.
- `[15][10]` bit7 records that the default has been applied; bit0 stays ANIM IDLE.
  Remaining bits and other preferences are retained. PER5 layout/size is unchanged.
- Subsequent user choices persist normally, including SCOPE/TRACKS/LEVELS/CREW.
  Marked unknown values retain their existing SCOPE fallback.
- No music/project reset, storage clearing, save-on-boot or extra flash write.
  The normal next settings save persists the marker and selected view.

## Verification recipe

```sh
source /home/dev/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
cc -O1 -w -Ifelucca/build/gen -o felucca/build/host/settings_test felucca/tests/settings_test.c -lm
felucca/build/host/settings_test
node tests/crew-browser.cjs
BGIRL_CREW_PREVIOUS_WASM=/path/to/38a429/felucca.wasm node tests/crew-upgrade-browser.cjs
```

Focused native checks cover each old view and unknown value with IDLE on/off,
exact preservation of all other imported bytes, repeat-import idempotence, and
post-upgrade menu choices surviving export/import. The standalone settings test
also exercises older PER1/2/3 records; its stale assertion that MONO is the final
palette was updated to accommodate the already-appended CIRCUIT palette.

The upgrade browser test boots the genuine pre-upgrade `38a429` WASM, saves SCOPE
through the normal controls, reloads to confirm it, then switches to the new WASM
on the same origin without clearing storage. It verifies CREW is adopted and a
later explicit TRACKS choice survives another reload. Normal CREW acceptance
covers all four views' saved reloads, six dancers, audio, mute, live notes,
overlays, LARGE/style/palette and mobile. Browser receipt and full-suite outcomes
are recorded with the exact deployed head in PR29 and the handoff notebook.

## Preview

Refresh the existing authorized private preview at
`https://openclaw-fedora.tail549b77.ts.net:8448/` using a clean exact-head rebuild,
immutable SHA-named artifact and metadata/manifest checks. Retain all archived
listening payloads unchanged. Capture the current service drop-in/routing as a
separate rollback before swapping; do not alter Tailscale Serve/Funnel routes.
No merge, device build/flash or public release is part of this request.
