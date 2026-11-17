# Clarity wire/control contract

This tranche adds one common parameter, `P_VRANGE`, immediately before `P_E0`.
Existing vocoder IDs, including raw `P_VREL`, stay fixed. Engine IDs shift by one;
P_COUNT becomes127. `VRANGE=0` means legacy REL: coefficient
`1/(44.1*(REL+1))`, unchanged. `VRANGE=1` means logarithmic5–500ms over REL0–127:
`ms=5*100^(REL/127)`, release coefficient `1/(44.1*ms)`. These are approximate
one-pole e-folding times, not time to silence or Ableton envelope equivalence.
Display rounds milliseconds to nearest integer; DSP retains table precision.
Lookup tables are generated offline and shared by authored presets and DSP.

B6S1 stays a3840-byte object with frozen126-field defaults unchanged. Writer
advertises127 parameters. Reader accepts126 or127; old126 records gain
VRANGE=0, engine fields and motion/lock engine IDs shift by one on decode. New
records use an accessor over the frozen defaults with VRANGE default0. Old
firmware rejects127-count records rather than misreading engine fields. Two-pass
validation and FULL atomicity remain. No unrelated project format is migrated.

Sound presets already map common/engine fields by stored count. Missing VRANGE
therefore uses0. Old REL values are not rewritten. Engine MIDI-learn codes are
already relative to ML_E0 and remain stable; common IDs do not move. Locks/motion
can now explicitly address REL (and bands/wet/trim/color), while VRANGE remains a
non-automatable patch setting. A lock's REL interpretation follows that setting.
Editor descriptor/DUMP counts follow firmware; clients must re-read descriptors.

BANDS identities0/1 retain8/12 coefficients and gain8. Identity2 adds24. Switching
filterbank modes explicitly clears filter/envelope/Freeze state before processing
and captures the new bank on the next Freeze activation. STOP/load/panic reset
through the existing routing owner. No allocation/I/O/math generation in render.
Existing startup/electro loader remains unchanged. New auditions are explicit,
stopped project replacements; no saved-slot deletion or autosave override.

### Preset and editor downgrade protection

Log-mode sound presets use versions9/10 (normal/grid); older firmware rejects
those versions. Legacy-mode presets retain existing versions/count mapping.
Identity-aware VOCALS fingerprints remain at byte134 in either mode. New imports
validate the discriminator/count, and preserve both vocal identity and drum-grid
semantics. No old sound preset is rewritten just by reading it.

BGIRL-only descriptor formats14/15 mean logarithmic/legacy vocoder milliseconds;
the pinned engine descriptor API is not changed. Editor formatting understands
both; RANGE changes trigger descriptor reloads, including panel/MIDI changes.
The127th parameter fills the last spare MIDI-learn common code; engine codes
remain120–127. No existing learned mapping is cleared or moved.
