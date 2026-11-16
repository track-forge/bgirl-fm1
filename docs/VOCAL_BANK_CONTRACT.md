# Optional vocal bank contract (issue #13)

Written before implementation, based on main c3edd75. Host/browser only.

- Append SLICE SRC 6 `VOCALS`; 0..5, especially factory SPEECH, never move.
  EDIT 1: SRC, VIEW, START, BANK. VIEW 0 = WHOLE (up to six originals);
  1..6 = packed parent VIEW indices in sorted source-ID order; display labels
  may retain an original group number (the public G6-only bank uses VIEW 1).
  START is an entry
  offset. Bank metadata maps compact display names to source/chop IDs.
  C4 + ROOT selects START; subsequent chromatic notes wrap within that view,
  including negative offsets. OCT +/- changes the usual keyboard octave;
  all entries are reachable without needing all 46 in one AUTO table.
- Forward original-speed mono one-shots, no PTCH/REV/DCAY/LOOP. Note-off
  (even before first render) never truncates; one voice, newest trigger wins,
  128-frame anti-click handover. Existing ADSR/TONE/effects remain available.
  Tempo changes trigger spacing only. Use existing keys, MIDI and steps.
- Reuse the SLICE IMA decoder and captured source/bounds. A source or VIEW
  change lets an existing shot finish its captured entry; the next trigger
  uses the new view. Routing/Freeze reset follows existing block contracts.
  Missing bank/invalid view: next trigger replaces the voice with silence,
  never a sine or last valid entry. STOP/panic stop voice and pending triggers.
- Optional immutable compiled bank, offline prepared, no render-time I/O or
  allocation. One ADPCM stream with explicit start/end/checkpoint per entry;
  plus 128 grid checkpoints for the inherited read-only waveform display;
  no 16-segment/32-onset limit abuse. Whole files and supplied chopped files
  are encoded independently into the stream (their fades may differ).
- Project parameters already preserve SRC/VIEW/START/ROOT and sequencer notes.
  Use the 32 bytes immediately before FUNB's three MIDI bytes for four 8-byte
  content fingerprints. Zero/mismatch means missing, not permission to adopt
  another bank. Vocal projects write FUNC (same 3840-byte extent/layout) so
  older firmware refuses SRC 6 instead of clamping to SPEECH. Non-vocal projects
  still write FUNB. Old FUN1..FUNB remain readable; factory paths ignore IDs.
  A VOCALS preset uses version 6 and eight spare packed bytes (134..141);
  older firmware refuses it. Old records have zero. Preserve the expected fingerprint even while unavailable. Explicitly
  editing SRC to VOCALS adopts the installed bank; restored projects/presets,
  source automation and triggers do not. No new parameters, count/layout/address/flash extent changes.
- Default build packages the checked-in G6 original and six chops authorized by
  Doug for public distribution on 2026-11-16. Other supplied recordings remain
  private and require an explicit bank directory; those banks/artifacts stay
  ignored and out of CI/Pages. Default builds repackage G6 afresh. The original
  speech fixture remains an explicit offline test option. No hot loading or
  new slicing editor. Group labels participate in the bank fingerprint.
- Packaging validates full inventory/manifest/bounds and hashes; stable sorted
  parent IDs and slice_index order, no filesystem-order IDs, retrimming or
  detection. Stereo averaged to mono; deterministic 44.1k->22.05k pair-average
  low-pass/decimation (22.05k unchanged; existing 24k fixture linearly resampled),
  fixed -3dB headroom (float overshoots up to 1.4 permitted), original
  tails preserved, 3ms edge fades on originals only (chops already faded).
  Exact offsets, durations, input SHA256 and full bank SHA256 in metadata.

No warping (#12), dry overlay (#8), hardware build/flash, release, merge or
manual public deployment. Browser signal checks do not establish listening
approval or physical-device timing/RAM fit.
