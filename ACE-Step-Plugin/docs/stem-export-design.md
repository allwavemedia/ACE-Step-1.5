# Stem Export Design

## Backend Capability Finding

Task 9.1 inspected the vendored `ServeurpersoCom/acestep.cpp` backend for explicit
stem generation or post-processing APIs. The backend source does contain stem-oriented
task modes:

- `src/task-types.h` defines `lego`, `extract`, and `complete` task identifiers plus
  track names including `vocals`, `drums`, `bass`, `guitar`, and related groups.
- `src/pipeline-synth.cpp` implements `run_lego`, `run_extract`, and `run_complete`.
  Comments describe `extract` as stem isolation from a full mix and warn that stem
  tasks require the base model because turbo output is incoherent.
- `docs/ARCHITECTURE.md` documents `task_type="extract"` as a full-mix-to-stem path
  requiring `--src-audio` and `track`.

The current plugin-facing C API remains narrower than that internal capability.
`src/acestep_capi.h` exposes `acestep_generate_wav` and an `acestep_generation_request`
with `prompt`, `lyrics`, model path, output WAV path, duration, seeds, CFG scale, and
scheduler only. It does not expose `task_type`, `track`, source-audio input, base-model
selection, or multi-output stem metadata.

**Conclusion:** ACE-Step stem functionality exists internally, but it is not yet
available through the plugin-owned backend boundary. Stem controls must remain gated
unavailable by default until the C API and background worker can explicitly request,
validate, and return stem WAV outputs.

## v1 Product Behavior

Generated assets may contain a full mix and zero or more stem WAV files. Stem controls
are disabled unless the generation path reports reliable stem outputs for that asset.
The plugin must not infer stems from FSQ tokens or expose empty placeholder files.

## Implementation Boundary

Stem metadata lives on generated assets. Stem production runs only on the background
worker, never inside `processBlock`. The next implementation slice should add explicit
request/capability types while keeping the default capability state unavailable.
