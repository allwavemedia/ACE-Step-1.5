# VST3 Functional Parity Research Decisions

## Source Reports

These decisions summarize local research only. They cite the checked-in reports below and do not add external citations beyond those reports.

- `research/acestep-integration-report.md`
- `research/ai-music-plugin-report.md`
- `research/audio-to-midi-report.md`
- `research/Local-Stem-Separation-report.md`
- `research/vst3-sidecar-report.md`
- `research/vst3-ui-report.md`

## Decision Summary

The v1 recovery architecture is sidecar-first for every heavyweight operation required for release-grade VST3 functional parity: ACE-Step generation, MIDI transcription, and stem separation. This intentionally supersedes the earlier in-process ACE-Step generation path for the user-facing release runtime contract. The in-process ACE-Step/acestep.cpp boundary may remain as a lower-level library surface or future optimization, but scan/load, editor construction, and `processBlock` stay stable by routing long-running model, tool, and file work through a lazy helper sidecar.

The research basis is:

- `research/acestep-integration-report.md` found ACE-Step prompt-to-WAV surfaces viable, native stem modes not release-grade for primary v1 stem export, and no documented native note/onset/event/MIDI API.
- `research/audio-to-midi-report.md` selected Basic Pitch as the Windows-friendly v1 transcription backend and `mt3-infer` with `mr_mt3` as the fallback path.
- `research/Local-Stem-Separation-report.md` selected Demucs `htdemucs_ft` as the stable v1 4-stem implementation and SCNet Large as the first fallback.
- `research/vst3-sidecar-report.md` selected a lazy per-host-process helper, Windows named-pipe control plane, staged file artifacts, absolute `CreateProcessW` launch, job-object cleanup, and private helper runtime packaging.
- `research/vst3-ui-report.md` selected one fixed-size, single-column, single-scroll editor with no tabs, wizards, or resize-dependent primary controls.
- `research/ai-music-plugin-report.md` selected disposable validation workspaces or VHDX layers, SHA-256 manifests, ACL save/restore, disk-full tests, host validation, and evidence records as release gates.

## MIDI Decision

| Field | Decision |
|---|---|
| Primary tool | Basic Pitch official Python package |
| Fallback tool | `mt3-infer` with `mr_mt3` |
| Invocation | `basic-pitch <output-dir> <input.wav> --save-note-events --save-model-outputs` |
| Warm API | `basic_pitch.inference.Model(ICASSP_2022_MODEL_PATH)` reused inside sidecar |
| Input | Generated full-mix WAV |
| Output | `output.mid`, `note_events.json`, optional `note_events.csv`, diagnostics manifest |
| Internal contract | Backend-neutral note-event schema plus rendered MIDI |
| Acceptance | Parseable MIDI imports into Reaper and at least one strict parser; note count and duration sanity pass; simple fixture note-onset F1 is measured with 50 ms tolerance |

### MIDI Feasibility Notes

Feasibility is based on `research/audio-to-midi-report.md`, which records Basic Pitch Windows support, CLI/API availability, note-event outputs, and warm model reuse. The sidecar must ship Basic Pitch inside a private embedded/frozen runtime and must never invoke user PATH Python. Shipping remains gated on packaging validation, a sidecar smoke test, strict parser validation, Reaper import validation, and host validation evidence.

## Stem Decision

| Field | Decision |
|---|---|
| Primary tool | Demucs `htdemucs_ft` |
| Fallback tool | SCNet Large |
| Invocation | `python.exe -m demucs -n htdemucs_ft <input.wav>` |
| Input | Generated full-mix WAV |
| Output | Stem WAV files staged under a job output directory |
| Stem groups | `standard-4`: vocals, drums, bass, other |
| Acceptance | Four valid WAVs exist; duration matches input within tolerance; no zero-byte outputs; summed stems strongly correlate with source; manifest records model ID/checkpoint hash/sample rate/channel count |

### Stem Feasibility Notes

Feasibility is based on `research/Local-Stem-Separation-report.md`, which records Demucs `htdemucs_ft` as the stable v1 4-stem path with a simple Windows CLI and SCNet Large as a lighter fallback. The sidecar must ship Demucs and dependencies inside a private embedded/frozen runtime and must never invoke user PATH Python. Shipping remains gated on packaged-runtime smoke validation, four-stem output validation, manifest/hash validation, Reaper stem import validation, and host validation evidence.

## Sidecar Decision

| Field | Decision |
|---|---|
| Lifetime | Lazy per-host-process helper |
| Control plane | Versioned JSON messages over Windows named pipes |
| Artifacts | Per-job file staging under LocalAppData/temp with result manifests |
| Launch | Absolute `CreateProcessW` path; no PATH or shell lookup |
| Cleanup | Job object with kill-on-close plus janitor cleanup of orphaned job roots |
| Bundle location | `ACE-Step.vst3\Contents\Resources\ACE-Step\helpers\` |
| Python | Private embedded/frozen runtime; never user PATH Python |

## UI and Validation Decisions

The release UI is one fixed-size, single-column, single-scroll workflow covering setup, generation, capture, progress/cancel, generated assets, WAV/MIDI/stem exports, presets, and diagnostics. All destructive validation must run in disposable workspaces or VHDX layers, never against the only known-good model copy. Validation evidence must include manifests, hashes, host/tool versions, sidecar runtime identity, raw Windows errors where applicable, and observed recovery behavior.
