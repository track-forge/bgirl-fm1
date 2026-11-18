# Song snapshot utilities

No npm install, server or WASM runtime is required for these file commands:

```sh
node tools/song-snapshots/song.mjs validate song.bgirl.json
node tools/song-snapshots/song.mjs inspect song.bgirl.json
node tools/song-snapshots/song.mjs diff original.bgirl.json edited.bgirl.json
```

Validate/inspect return1 for invalid or explicitly unresolved resources. Diff
returns0 for valid comparisons, including changes;1 for invalid input. Metadata,
title and producer timestamps/build IDs are not musical differences.
The browser performs the final installed-bank check. No file is evaluated, sent
anywhere or rewritten by the CLI. Files over2MiB are rejected before reading.

The authoritative readable JSON contains stable parameter names, exact raw counts
and explicit numeric FM6 operator bytes. To edit only pan and one stored note:

```sh
node --input-type=module - original.bgirl.json edited.bgirl.json <<'JS'
import fs from 'node:fs';
const [src, dst] = process.argv.slice(2);
const song = JSON.parse(fs.readFileSync(src, 'utf8'));
song.song.tracks[5].parameters.P_PAN = 15;
song.song.tracks[4].steps[63].notes[0] = 57;
fs.writeFileSync(dst, JSON.stringify(song, null, 2) + '\n');
JS
node tools/song-snapshots/song.mjs validate edited.bgirl.json
node tools/song-snapshots/song.mjs diff original.bgirl.json edited.bgirl.json
```

That edits the stored note field even if currently a rest; to sound it, set that
step's `n=1`, `time=0`, `velocity=99` and `notes=[57,0,0,0]` too. Notes0..127 are
MIDI note numbers; track/step indexes are zero-based. Normal audition/import needs
no build. See [the full contract](../../docs/SONG_SNAPSHOTS.md) for FM6 OWN,
resource identity, checkpoints, bounds and factory-promotion separation.
