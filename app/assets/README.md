# Original speech modulator

Text: **“B girl. Move your feet. Rock the beat.”** Written for this project;
no sampled record, voice clone or reference recording used.

Generated 2026-11-15 with Moltazine/Crucible `qwen3-tts-customvoice-1.7b`,
preset `Ryan`, English, seed 41, instruction: “Speak clearly and rhythmically,
dry studio speech, no music.” Job `ce4b8172-e27c-4843-ac0a-d013a5706a63`.
This job is not a build dependency. Generation is synthetic, not a real performer.
The original text and generated asset are dedicated to CC0-1.0 to the extent
we hold rights; model/runtime licensing is separate from generated output.
No third-party performance rights claimed.

Source: mono PCM signed 16-bit, 24000 Hz, 2.856875 seconds. Converted locally:

```sh
ffmpeg -i source.wav -ac 1 -ar 22050 -c:a pcm_s16le app/assets/phrase.wav
python3 app/assets/embed.py
```

Committed `phrase.wav`: 62995 frames, 22050 Hz, mono signed 16-bit, 125990 PCM
bytes (WAV adds header). `phrase.h` is a deterministic embedding of that PCM;
only `vocoder.c` includes it, once, as read-only data. Playback at 44100 Hz uses
each sample twice. No decoder, filesystem or allocation in the render path.
Original unconverted download remains local under `tmp/bgirl-voice/source.wav`.
No API key or private download URL is included here.
