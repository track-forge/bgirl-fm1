# SH-STYLE + current main: authorized tailnet preview

2026-11-18. Doug requested replacing the existing browser preview at
https://openclaw-fedora.tail549b77.ts.net:8448/ and checking/rebasing the stack first.
This supersedes the earlier no-preview-deployment boundary for this endpoint only.
No repository merge, public Pages/Funnel rollout, hardware capture/build/flash or
DAW change is authorized or performed.

## Stack integration

Main `a0a475d` includes merged Mega Jam PR28 and CREW PR29, including the final
ping-pong/crop improvements. Parent PR32's three commits were rebased onto that
main; range-diff shows all three unchanged. New parent head:
`e7f26ba281310978cf236ea123efeb2d526de103`.
Child PR33 was restacked on that parent. Only conflict was appended test-runner
blocks: retained **all Mega, CREW and SH-STYLE checks**. Translator, policy, source
snapshots, recipes/reports and engine/BSP pins are unchanged. Normal sound-loading
hook and browser additions auto-merged; no main demo/CREW features are dropped.
Old published parent/child heads were retained as local backup refs; pushes use
exact force-with-lease values, not unguarded force. PR33 remains stacked on PR32.

The initial report's old ancestry/CI/917035-byte artifact is historical, not proof
of this integration. Fresh local full host/WASM tests and actual Chromium SH-STYLE,
Mega, CREW, CREW settings upgrade, six-track and smoke suites cover the combined
build. Exact observed results and current-head hosted checks are recorded on PR33
and the notebook before final handoff; no old-head CI is relabeled as new.

## Preview mechanics / rollback

Keep the existing systemd user service `bgirl-signature-local.service`, Python
server bound only to127.0.0.1:8786, and the existing8448 tailnet proxy. Snapshot
its effective unit/drop-in, old build identity and complete Serve/Funnel JSON under
ignored `local/sh-style-preview/rollback/`. Do not alter other routes/services.
The previous d4ab9a1 artifact stays intact. Rollback restores the captured
`clarity-preview.conf`, daemon-reloads and restarts only this service.

Build a clean exact-PR-head source, freeze all runtime files in a SHA-named ignored
artifact directory, include build.json/source/license and a payload hash manifest,
and compare runtime bytes with the tested build. Retain archived listening files
byte-for-byte from the existing preview; no gallery regeneration or relabeling.
Repoint only the captured service drop-in and verify active status, exact HTTPS
metadata and downloaded payload hashes, unchanged routing, and actual HTTPS
AudioWorklet/controls/persistence on SH-STYLE plus Mega and CREW. Roll back if the
live interaction or identity fails.

## Normal demo

Open the tailnet URL and Switch on. Existing music/autosaves still win; use the
explicit **Load electro demo** button only if you want to replace unsaved music
with main's Mega Jam. CREW is main's default display; existing saved view choices
remain honored. SelectT1–T6 with ALGORITHM, then use SH-ISH's selected-track
loader or normal PRESETS→LIST→SH-ISH. ENV/LFO/EDIT and SAVE work normally.
No new collection preset is automatically installed into the current demo.

The sounds remain approximate: proving1-1 drops source pulse/stored chord; wire-PC1
(unknown human slot) drops source sub. Source selector bins/physical laws remain
unverified. Generated technical signal tests and this preview do not establish
Roland equivalence, acoustic calibration, listening approval or physical fit.
