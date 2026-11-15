# bgirl-fm1

**Hype classic electro in a box.** An experimental M-VAVE FM-1 instrument:
raw machine drums, rubbery mono bass, and a sample-fed robot voice.

## Start here

- [Initial build plan and handoff](docs/INITIAL_BUILD_PLAN.md) — scope, sequence, acceptance criteria.
- [Contributor instructions](AGENTS.md) — working boundaries and verification.
- [Upstream provenance](docs/UPSTREAM.md) — pinned sources and licensing.

## Local development

Requires Git, Bash, Python 3.11+ (tomllib), a C compiler (`cc`), and standard host development headers.

```sh
git clone --recurse-submodules https://github.com/track-forge/bgirl-fm1.git
cd bgirl-fm1
git submodule update --init --recursive
./tests/run_tests.sh
```

The test runner builds the host app and runs the copied upstream checks. WAV and
screen captures go to `app/build/host/` (ignored). Run `./build.sh --host` to build only.
`./build.sh --web` requires Emscripten on PATH; browser setup and verification are the next work.

**Current status:** buildable upstream template, not the electro instrument yet.
`app/` retains upstream TEMPLATE identity, settings, Bluetooth simulation, and tests
intentionally so the baseline is comparable. No custom DSP or sequencing exists.
The wrapper only enables host/browser builds. Device identity, storage layout,
SDK/toolchain, packages, releases, and flashing are not configured or verified.
No auto-publishing workflows are installed.
