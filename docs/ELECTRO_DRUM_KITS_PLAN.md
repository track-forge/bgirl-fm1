# Three electro kits for BGIRL

A beat programmed once should tell three different stories. Make **BG BREAK** the fresh-start drum sound: dry, wiry, Cybotron-leaning electro where the bass plays half the rhythm. **808 RAW** is the recognizable drum machine; **COLD WIRE** is the kit that sounds least like one. These are complete eight-lane kits on one DRUM track, not eight new tracks or three EQ settings on one kick.

## Voice the kit as an ensemble

Keep the existing key, MIDI and sequencer lane roles. The columns are voicing targets, not a prescription for a particular synthesis algorithm.

| Lane | BG BREAK | 808 RAW | COLD WIRE |
| --- | --- | --- | --- |
| Kick | Short, chesty punch; clear the next bass note | Rounded pitch drop with an exposed, ringing low tail | Hard, compact knock; little sub sustain |
| Snare | Narrow dry crack, enough body to stand alone | Tuned shell under snappy noise | Struck sheet metal; accents expose the clang |
| Clap | Slightly spread around the snare; stack without burying its attack | Recognizable multi-burst handclap and tail | Clipped noise burst, not a room tail |
| Closed hat | Tight tick for syncopated sixteenths | Wiry metallic chatter | Thin, bright pulse; quiet hits stay restrained |
| Open hat | Quick intake of breath | Sizzling open metal, cut by the closed hat | Splash of inharmonic metal with an abrupt choke |
| Tom | Short tuned knock between bass notes | Hollow conga | Pitched FM tom that answers the bass |
| Rim | Dry woody knock strong enough to carry an offbeat | Hard rim/clave click | High metallic ping, distinct from the snare |
| Bell | Short metallic punctuation, below the clap in the mix | Unmistakable two-tone cowbell | Glassy zap; accents change timbre, not just level |

## Implementation route

1. Establish a listening baseline with Felucca's existing 80 and TIGHT kits. Compare the same kick, clap, hats and bell phrase against [fm1-x0x's modeled 808](https://github.com/charlesvestal/fm1-x0x/blob/main/firmware/src/dsp/drum808.c) (derived from 8W8). Reuse the voices that make RAW more convincing, not the fork's entire machine. Its 909 is a separate hybrid with sampled cymbals; a 909 kit is not a deliverable. [Hortator](https://github.com/DeadActive/hortator) is a reference for pitched percussion and expressive models, not a request to port its sequencer. Record exact source revision, paths, authors and component notices for anything copied or adapted.
2. Build RAW as the reference, BREAK around the snare/clap stack, then WIRE around the tom/bass exchange. Start in felucca/firmware/src/drum_voice.c (voices and kit recipes) and eng_drum.c (selection, note mapping and presets). Reuse existing knobs, accent and choke behavior; introduce voice DSP only where the existing models cannot make the sound. Do not require parameter locks, external FX or a second track to hear a kit's identity.
3. Expose all three via normal DRUM selection. Append stable identifiers rather than renumbering existing choices; verify selector limits and preset/project round trips. Select BREAK in BGIRL's fresh initialization, not in TRK_DEF or frozen project defaults. Restoring an autosave must restore its own sound. Preserve Mega Jam's T1 TIGHT/T6 BG KICK split and existing songs.

## How to know it works

- Render one two-bar broken beat at the same tempo, timing, velocities and mix settings for all three kits. Include a snare alone, clap alone, stacked backbeat, open-to-closed hat and accented tom/bell answer. Supply level-matched clips, first drums alone and then with the same bassline. Give the reviewer the pattern or reproduction command. BREAK should win on the backbeat, RAW on space and boom, WIRE on the pitched reply. If only the kick decay gives the kit away, revisit the other seven lanes.
- Exercise live keys, MIDI and sequenced hits; switch kits on the same pattern, save/reload each, and load an existing project unchanged. Compare dense accented stacks with the baseline for clipping, tail cutoff, stuck voices and render cost under the shared eight-voice limit. Keep allocation and I/O out of render.
- Build the web target and run the repo host tests plus focused drum and browser playback checks. Report what was measured versus what still needs Doug's ears. No device build/flash or hardware performance claim.
