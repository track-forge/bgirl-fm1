# STAGE / LAB / CUT — first-screen review

Implementation in progress on the existing draft PR36. Refs #35.

## Baseline and screen inventory

Baseline runtime/main: `c7116c5451bcfca9325ce71a71f7356ab1bd1b2c`;
planning head: `7d2a0fe73f9bcc2e28b2eae69b66c3b1c60fb107`.
Remote main unchanged at implementation start. Pinned submodules initialized;
engine/BSP revisions unchanged. Browser/host only, no device build.

| Family | Existing bindings / visual data | First milestone |
| --- | --- | --- |
| HOME / CREW | Engine's four knobs; selected-track atomic activity and existing 96px masks | STAGE, performance-first, readouts below |
| HOME SCOPE/TRACKS/LEVELS | Scope snapshot / track status / banked levels | Existing composition retained |
| FM6 OPERATOR / OPERATOR2 | OP/coarse/fine/level; OP/mode/detune/velocity; actual six-op patch routing | LAB, routing-first, readouts below |
| ENV | ATK/DEC/SUS/REL, parameter-derived envelope | Shared LAB-layout cross-check |
| Drum STEP | Step cursor/lane editing and events, accents, mutes, 16-step banks of64 | CUT, event-first, readouts below |
| Melodic STEP/DETAIL, OP ENV and remaining sound/FX pages | Existing parameter bindings/graphs | Fallback unchanged; shared typography audited |
| MIXER/CLOCK, lists, layer maps, naming, dialogs, count-in | Existing owners/overlays | Existing composition; transition checks |

Original geometry: header0–23, cards28–71 (LARGE tall28–131), graph76–197
(LARGE strip136–197), footer202–239. Layout identity is not persisted.

Baseline commands (logs under ignored `local/stage-lab-cut/`):

```sh
source /home/dev/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
# Native comprehensive renderer: same HAL includes as host tests
cc -O1 -w -DFELUCCA_VERSION='"1.8.1"' \
 -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip \
 -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/build/gen \
 -Ifelucca/firmware/src -Ifelucca/tests \
 -o local/stage-lab-cut/ui-baseline felucca/tests/ui_render.c -lm
local/stage-lab-cut/ui-baseline local/stage-lab-cut/before
python3 felucca/tests/ui_render.py local/stage-lab-cut/before
```

Baseline native comprehensive renderer **already fails**: 4044 layout findings,
zero ink/neutrality/alignment failures. Preserve this finding ledger for comparison;
do not treat a newly green subset as a comprehensive pass. Focused four-screen
matrix baseline: zero findings/ink/neutrality/alignment failures. Native WASM baseline
956577 bytes. Canvas59520 bytes. Audio baseline111 goldens unchanged (full suite
receipt to follow). Focused renderer reuses upstream hooks and scene setup through
`BGIRL_RENDER_LIBRARY`; no copied firmware renderer.
