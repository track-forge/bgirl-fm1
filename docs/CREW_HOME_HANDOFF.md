# Copy/paste handoff — implement CREW HOME

Implement `docs/CREW_HOME_IMPLEMENTATION_PLAN.md` in
https://github.com/track-forge/bgirl-fm1 on a new feature branch/worktree.

Goal: make the b-girls the instrument's HOME experience, one independently reacting
dancer per track. **Current main has SIX tracks**, not the four mentioned in the
initial brainstorm. Preserve the eight-voice budget and normal instrument controls.

Read `AGENTS.md`, README, the CREW plan, six-track contracts/verification, and existing
ARP DANCE implementation/provenance before editing. Start from fresh verified main.
If the planning PR is unmerged, bring its two documents onto your implementation
branch without discarding newer main changes; do not implement on an obsolete
four-track baseline. Inspect status and leave other agents' worktrees untouched.

Follow the plan's five stages: reproduce baseline; convert and review supplied art
and six-person layouts; implement bounded general track-activity telemetry;
integrate CREW as a fresh-settings HOME default with preserved preferences and
controls; verify and publish an implementation PR. You may make the documented
layout fallback decision from actual-size evidence. Do not silently reduce the
crew to four or label unreviewed art as approved.

All FOUR new public video references and Doug's public-use authorization are in
the plan's source table. Download/inspect each original, preserve provenance and
hashes, convert offline to deterministic named silhouette banks, and include
contact sheets/loop-seam previews. The second source's move is unnamed; inspect it
rather than guessing. Existing FLOORWORK is a separate asset and must remain
available to ARP DANCE. No new video/image generation is requested.

Use musical phase for underlying movement and accepted track activity for bounded
accents/settling. Support sequencer, arp and live keyboard/MIDI; coalesce chords and
rapid notes rather than speeding animation with voice count. Define stopped live
play, sustained/one-shot sounds, mute/solo, voice stealing, deferred engine starts,
project reset and panic behavior. Do not claim an activity display is a sound meter.
Keep audio output/RNG/sequencing unchanged and rendering out of the audio path.

Add CREW without renumbering existing HOME views; preserve saved choices, six-track
selection, GLO banks, HOME knobs, HOME/MIXER/CLOCK/menu behavior and editing pages.
Keep ARP DANCE's existing CRISP/GHOST trigger contract. No project-format changes.

Deliver scoped commits, reproducible conversions, actual 240px/browser art review,
focused regressions and full applicable build/tests, audio-preservation comparison,
size/redraw measurements, README and `docs/CREW_HOME_VERIFICATION.md`, and a review
PR with exact tested revision and demo recipe. Distinguish automated acceptance,
Doug's pending visual approval, and untested physical-device behavior. Document
unusable reference clips or genuine blockers precisely, without silently dropping
sources or relaxing tests.

Do not merge, build/flash hardware, change pins/storage identity, publish releases,
run paid generation or manually deploy previews. Existing normal PR CI is fine.
Do not absorb unrelated Mega Jam/demo or vocoder work into this feature.
