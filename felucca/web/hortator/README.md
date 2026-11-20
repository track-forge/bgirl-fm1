# Hortator review baseline

Imported from [DEADACTIVE's Hortator](https://github.com/deadactive/hortator/tree/44c9ad4aa57bebf63f412bd552e57143c30b4065/web/sim)
at `44c9ad4aa57bebf63f412bd552e57143c30b4065`. The first import commit is an
unmodified copy of the entire `web/sim` directory. This checkpoint retains the
scroll reveal, original panel, and all of How to play. It excludes Hear
it go, Install, and counters. The panel now runs BGIRL’s existing studio WASM and AudioWorklet with disposable flash. The silent CREW reel is captured offline from that same WASM. The jam guide describes BGIRL’s six-track workflow and vocoder.

Build the pinned checkout with its `tools/build_sim.sh` and BGIRL with
`./build.sh --web`, then run from the BGIRL root:

```sh
tools/stage-hortator-preview.sh /absolute/hortator /absolute/bgirl-fm1/felucca/build/emu /absolute/new-preview
```

Serve the new directory over HTTPS or loopback for AudioWorklet. The homepage does not access browser saves. BGIRL studio at `/play/` retains its existing storage. Upstream attribution and notices accompany the preview.
This staging step is not part of the production Pages workflow.
