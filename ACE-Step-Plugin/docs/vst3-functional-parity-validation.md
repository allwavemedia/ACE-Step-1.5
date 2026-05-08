# VST3 Functional Parity Validation Notes

This runbook records validation gates for the sidecar-first VST3 functional parity recovery plan. It is documentation-only until implementation creates the concrete harness and evidence records.

## Research Evidence Basis

Feasibility for the selected local toolchain is based on checked-in research reports:

- Basic Pitch and MT3/MR-MT3: `research/audio-to-midi-report.md`
- Demucs and SCNet: `research/Local-Stem-Separation-report.md`
- Sidecar IPC, cleanup, and packaging: `research/vst3-sidecar-report.md`
- Fixed single-scroll UI: `research/vst3-ui-report.md`
- Disposable validation workspaces and host matrix: `research/ai-music-plugin-report.md`
- ACE-Step runtime boundary and native limitations: `research/acestep-integration-report.md`

## Runtime Packaging Gate

- The helper ships under `ACE-Step.vst3\Contents\Resources\ACE-Step\helpers\`.
- Python-backed tools use a private embedded/frozen runtime owned by the helper.
- Validation must prove the plugin never invokes user PATH Python for Basic Pitch, Demucs, MT3/MR-MT3, or SCNet.
- The helper is launched by absolute `CreateProcessW` path and supervised by a kill-on-close job object.
- A smoke test must verify helper startup, named-pipe handshake, tool discovery, model/checkpoint identity, and clean shutdown from the packaged bundle layout.

## MIDI Export Gate

- Run Basic Pitch from the packaged helper against a generated full-mix WAV.
- Produce `output.mid`, `note_events.json`, optional `note_events.csv`, and a diagnostics manifest.
- Verify the MIDI is parseable by at least one strict parser and imports into Reaper.
- Verify note count and duration sanity.
- For a simple fixture, measure note-onset F1 with 50 ms tolerance.
- Verify unavailable Basic Pitch and fallback MT3/MR-MT3 states produce explicit editor text and diagnostics without host instability.

## Stem Export Gate

- Run Demucs `htdemucs_ft` from the packaged helper against a generated full-mix WAV.
- Produce the `standard-4` stem set: vocals, drums, bass, and other.
- Verify all four files are valid WAVs, non-zero-byte, readable, and duration-matched to the input within tolerance.
- Verify summed stems strongly correlate with the source mix.
- Verify the manifest records model ID, checkpoint hash, sample rate, channel count, output filenames, and completion timestamp.
- Verify unavailable Demucs and fallback SCNet states produce explicit editor text and diagnostics without host instability.

## Disposable Workspace Rule

Destructive tests must run only in disposable workspaces or VHDX layers. They must not mutate the only known-good model store or user-saved exports. Each run records baseline and post-run SHA-256 manifests, output listings, logs, screenshots or screen recordings where relevant, raw Windows error codes, and observed recovery behavior.

## Host Validation Gate

A candidate implementation is not complete until it passes:

- Steinberg Validator
- VST3 Plug-in Test Host
- pluginval
- JUCE AudioPluginHost
- Reaper
- At least two locally installed VST3 production hosts

Host validation installs the candidate to `C:\Program Files\Common Files\VST3` and covers scan/load, pass-through, editor layout, generation, cancellation, WAV export, MIDI export, stem export, presets, diagnostics, sidecar failure, and recovery after project reload.
