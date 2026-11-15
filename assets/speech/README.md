# BGIRL reference speech v1

Four original short texts, generated 2026-11-15 through the discovered Moltazine CLI
workflow `qwen3-tts-voicedesign-1.7b`, one-credit quote/job (four jobs). No real-person voice,
reference audio or cloning. Not pre-vocoded. Source FLACs retained unchanged;
mono PCM16 WAVs decoded with ffmpeg are the reproducible offline packaging inputs.

`manifest.json`: stable local bank/phrase IDs, original transcripts, job/output
identities, exact workflow/model revisions and parameters, rights basis/caveats,
SHA256 of both retained source formats. Output IDs are Crucible job-output IDs,
not a claim that user-upload asset records were created. No signed URLs/tokens.
Private CLI receipts remain outside this repo. No social posts were created.

`packaged.json`: exact runtime byte count/checksum/rate/samples and trim boundaries.
Generated original contributions are offered under CC0-1.0 to the extent rights
exist; this is not a warranty of exclusive rights in machine-generated output.
Model metadata declares Apache-2.0. No model/custom-node source is copied here;
workflow metadata retains a custom-node license-file caveat (see manifest).
CC0 text: https://creativecommons.org/publicdomain/zero/1.0/legalcode

## Offline reproduction

`python3 tools/package_speech.py` verifies WAV hashes, trims at 1% peak with25ms
pre-roll/50ms tail, peak normalizes to23000 PCM units, linearly resamples to22050Hz,
applies5ms endpoint fades, pads phrases to even sample counts, and encodes one IMA
stream using the pinned Felucca `sampleio.py`. It writes `felucca/build/gen/` header
and binary plus the measured packaging manifest. No numpy/network/service.
`./build.sh --web` calls it automatically. Stored dry WAVs, not TTS regeneration,
are the reproducibility boundary: seeded VoiceDesign is stochastic.

Bundled SLICE SRC5 (SPEECH), chromatic C4..D#4, wraps over four phrases; original
pitch regardless of key. Existing sequencer and decoder, no sample-slot upload.
Speech-bank version1 IDs/order never silently reassigned. User sample slots and
factory bank/source numbers0..4 remain unchanged.

## Listening

`assets/listening/` holds five9s stereo lossless firmware renders using these same
phrases, existing sequencer and ANALOG carrier. Dry/8/12/consonants/insert-chorus.
No loudness normalization; compare conservatively. Manifest includes peaks/RMS,
checksums, rates and sizes. Reproduce with `tests/render_vocoder.sh`.
**Neither source quality nor word intelligibility has human approval yet.**
