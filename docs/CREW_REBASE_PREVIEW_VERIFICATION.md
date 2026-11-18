# CREW HOME — rebase and authorized tailnet preview

2026-11-18. Doug explicitly requested rebasing PR29 onto current main and replacing
`https://openclaw-fedora.tail549b77.ts.net:8448/`. This authorizes that existing
private preview only, not a merge, public release/Funnel, device build or flash.

## Integration

- Remote main verified at `5a107f6ce772b9dac4bafdf1198aa4895e5238ed` (merged PR28,
  including the subsequent Mega Jam listening refinements).
- Previous PR head275867d retained in a local backup branch before rebase.
- Only conflict: appended blocks in `tests/run_tests.sh`. Kept **both** Mega Jam
  capacity/voice/persistence/native-WASM checks and CREW publication/render/audio
  preservation checks. Range-diff shows the six other replayed commits unchanged; the runtime commit
  differs only in that test-runner context.
- No changes to main's Mega Jam arrangement, DSP, dependency pins or user formats.
  CREW now accompanies main's six active demo tracks, including T6 BG KICK.
- Browser test commit `47a69ee04563f47948f175e1a72160d34b5ae11d` removes the old
  assumption that default T6 is empty. It explicitly clears only T6's pattern via
  SAVE→TOOLS→CLEAR PAT/confirmation, verifies it is unmuted and inactive while the
  other tracks remain audibly playing, then continues normal controls/persistence.
  Production demo data is not changed to satisfy the test.

## Rebased verification

The pre-rebase report is historical: [CREW_HOME_VERIFICATION.md](CREW_HOME_VERIFICATION.md).
Fresh integration commands:

```sh
source /home/dev/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
# Actual Chromium, using the normal local test server:
node tests/crew-browser.cjs
node tests/mega-browser.cjs
node tests/six-browser.cjs
node tests/browser-smoke.cjs
node tests/dance-browser.cjs
node tests/vocoder-browser.cjs
node tests/clarity-browser.cjs
```

Rebased web build and full host suite **PASS (exit0)**:109 unchanged audio goldens,
88200 heavy-song /176400 clarity /661500 Mega native-WASM frames identical. Both
Mega Jam and CREW checks execute; CREW's512000-frame audio/RNG comparison and1M
snapshot stress pass. All seven Chromium suites above **PASS**, no page errors;
CREW includes six identities, the explicit cleared-track isolation, live notes,
mute/return, saved HOME choices, overlays, LARGE/palette/style and mobile.

[Rebased CREW browser receipt](img/crew-rebase/results.json) ·
[T1](img/crew-rebase/track-1.png) · [T5 FM6 PAD](img/crew-rebase/track-5.png) ·
[T6 BG KICK](img/crew-rebase/track-6.png).

Final exact-head hosted-CI states are recorded in PR29's live receipt; do not
transfer a green verdict from an earlier head. Local receipts:
`/tmp/bgirl-crew-rebase-{build,tests,browser}.log` and corresponding suite logs.
Rebased WASM952989B, SHA256
`ab21349b4ac9f448b7e6453ab459ffc66e434f92bc5064a6e4385a8cebb219ac`.
This is an integration artifact, not a target-fit or performance claim.

## Preview ownership and rollback

Use a clean detached checkout of the final remote head; rebuild and freeze its
static browser assets in a SHA-named ignored directory. `build.json` records exact
commit/source/license and WASM hash; retain a per-file SHA256 manifest. Preserve
archived listening files from the prior immutable preview unchanged, with no new
rendering. The active service must serve this frozen directory, not the mutable
implementation checkout.

Only change the existing `bgirl-signature-local.service` runtime drop-in, keeping
its loopback8786 listener and **all** Tailscale Serve/Funnel settings unchanged.
Before replacement the artifact was Mega Jamb71dbc8, WASM SHA256
`b5ab47626dd914a7edb583198351717d3cd2c2226c633dac90e191e938e54b5d`.
It remains retained for rollback. Saved service/drop-in/routing/build receipts and
an exact restore script live in ignored `local/crew/rebase-preview/rollback/`.

After restarting, verify served metadata, downloaded WASM/all-file hashes, unchanged
routing and real HTTPS gesture/AudioWorklet/controls for CREW and Mega Jam. Restore
the old drop-in/artifact if acceptance fails. Deployment outcome/exact deployed
head are recorded in PR29's receipt and notebook, without a post-deployment code
commit that would silently advance the PR beyond the served revision.

Existing browser saves are not cleared or migrated. Saved HOME choice/music win;
if needed, HOME hold→DISPLAY→HOME→CREW and explicitly Load electro demo to adopt
current main's arrangement. Human art/seam/listening approval and all physical
LCD/DMA/SPI/atomic/linker/audio-deadline/MIDI gates remain separate.
