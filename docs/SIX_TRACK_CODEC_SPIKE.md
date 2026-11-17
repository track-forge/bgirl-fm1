# Six-track save-capacity decision

2026-11-17 · host/browser, not device validation.
Baseline `4a5f1fbc414a32cd907b2b215060dca503004e6c`; plan head `27f805d`.
Before source edits: root web build, complete host suite and actual Chromium
smoke passed (108 unchanged audio goldens). Emscripten 6.0.10, Node 24.22.1,
GCC 15.3.1. Logs: `/tmp/bgirl-six-baseline-{build,tests,browser}.log`.

## Decision

Keep four project slots, 4096-byte sectors, 256-byte payload offset, 3840-byte
objects and the existing CRC/A-B/header-last storage implementation unchanged.
A full compressed object is a legitimate, explicit **PROJECT FULL** failure.
No sample, bank, OTA or settings region is borrowed. No physical-layout change.

A first exact-field/sparse-zero-parameter sketch extrapolated from the baseline
showed that naive six-track expansion could exceed capacity even without dense
steps. Adopt JIANT's sparse-map/conditional-patch ideas instead, with two material
BGIRL differences: frozen codec defaults and a distinct clean-break format.

Implemented measurements (`tests/six_track_test.c`, including header/checksum):

| Fixture | Bytes needed | Fits 3840 |
| --- | ---: | --- |
| Empty normal reset palette | 734 | yes |
| Existing startup electro/vocal demo, T5/T6 empty | 1107 | yes |
| Six FM6 factory voices, empty steps | 1236 | yes |
| Six FM6 + all 128 shared motion events | 1748 | yes |
| New six-track 64-step sparse electro audition | 1358 | yes |
| All six ×64 steps ×4 notes, ratchet/nudge/hits/accents/chance +128 motion | 5972 | **no** |
| Same dense fixture, every track parameter nondefault | 6686 | **no** |

This is a fixture budget, not a promise that every musically dense state fits.
Dormant notes and flags count; no fields are discarded to meet the limit.
Exact 3839/3840/3841-byte boundaries are tested, not inferred from an estimate.

## Format contract: B6S1

Magic `0x31533642` (wire bytes `B6S1`), version1, exact six-track/126-parameter/
27-global counts. A fixed 20-byte header includes declared used length and
selected track. Explicit LE16 globals; 12 name bytes; per-track engine/preset,
conditional complete 128-byte FM6 patch, eight vocal-identity bytes, MIDI channel,
parameter bitmap/biased byte values, and 64-bit step bitmap. Steps retain all
four notes when dormant note bytes exist; optional fields retain hits/accents,
chance, and all high flag bits (including ratchet and signed nudge).

The chain is explicitly 16 ×(six slot bytes + repeat), with a four-byte head.
Motion has a four-byte head and count ×(LE16 place, parameter/lock, signed value).
128 events remain shared. Reserved/padding bytes are zero. Final FNV checksum
covers all 3836 preceding bytes, including unused space; NOR also retains its CRC.
No native `project_t`, chain or motion struct is copied onto the wire.

`project_defaults.h` freezes all omission defaults and the INIT FM6 patch for
this format. Future engine/preset default edits cannot redefine old saved sounds.
Parameter/global counts are compile-time guarded; incompatible schema changes
need a new format, not a regenerated defaults header.

The writer validates and measures **before** touching its output. Decoder validates
in a first pass before applying to its caller in a second pass. Its largest local
objects are chain116 + motion516 bytes; GCC -O2 reports 752 bytes stack for
`proj_decode`, 128 for `proj_pack`, 48 for `project_capture` (host only, not target).
The existing static decoded project scratch is 7284 bytes, serialized scratch3840.
Undo uses native per-track snapshots and is independent of packing capacity.

Manual save/rename/export return failure rather than SAVED/truncated output.
The PROJECT card shows bytes/3840 or FULL; full project saves are rejected before
name entry. Autosave reports FULL once and blocks retries for the unchanged state;
editing that state permits another attempt. The live project and previous RAM/NOR
copy survive failure. Existing interrupted-write tests retain the committed copy.

Legacy projects/autosaves are **not migrated**. Readers reject every old magic
before state mutation; old autosaves leave the fresh stopped startup song and
show OLD FORMAT. Old slots remain in NOR until explicit overwrite. PER5 settings
preserve older preferences/favorites but clear incompatible two-bit-track learned
bindings; new bindings retain all16 slots and use a 17-bit record split across
existing low bytes and a new high-bit bitmap. Presets, samples and banks are not
reset. See [verification](SIX_TRACK_VERIFICATION.md) for controls and audition.
