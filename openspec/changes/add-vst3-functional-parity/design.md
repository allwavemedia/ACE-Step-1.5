# VST3 Functional Parity Design

## Current Gap

The current editor wires only capture, model status, a meter, and presets. Standalone components for generation results and export exist, but they are not reachable in the running VST3.

## Runtime Boundary

The audio callback remains pass-through-only. Heavy work runs through a lazy per-host-process sidecar launched only from message/background threads after explicit user action, never during host scan, editor creation, or `processBlock`.

## Single Scroll Editor

The editor contains:

1. Model/tool setup status and download progress.
2. Generation form: prompt, lyrics, duration, seed, CFG scale, LM seed, scheduler, and use-captured-reference toggle.
3. Capture controls: Arm, Clear, and meter.
4. Generate/Cancel controls and progress/errors.
5. Generated asset history.
6. Per-asset preview, WAV Save As, WAV drag/drop, MIDI export, MIDI drag/drop, stem export, and stem drag/drop.
7. Preset browser.
8. Diagnostics panel.

## Sidecar Orchestration

The plugin starts the helper by absolute `CreateProcessW` path, assigns it to a kill-on-close job object, and communicates over versioned JSON messages on Windows named pipes. Large artifacts live in per-job directories under LocalAppData/temp. Results are trusted only after the plugin verifies schema version, request ID, byte size, SHA-256, expected artifact count, and file readability.

## MIDI Export

V1 uses Basic Pitch official Python package in the sidecar and emits both a rendered `.mid` file and backend-neutral `note_events.json`. MT3/MR-MT3 via `mt3-infer` is the first fallback/upgrade path. The architecture allows replacing audio-to-MIDI with backend-derived note events later.

## Stem Export

V1 uses Demucs `htdemucs_ft` in the sidecar and exports `standard-4` stem WAVs: vocals, drums, bass, and other. SCNet Large is the first fallback. Native ACE-Step stem modes stay experimental until validated against release-grade outputs.

## Error Handling

Every user-visible failure maps to explicit editor text. Failures must not create success-shaped asset tiles. Partial stem success may preserve successful full-mix output while marking failed stems.

## Validation

Validation covers Steinberg Validator, VST3 Plug-in Test Host, pluginval, AudioPluginHost, Reaper, and at least two locally installed VST3 production hosts. Host validation uses `C:\Program Files\Common Files\VST3`. Destructive validation uses disposable workspaces or VHDX layers, SHA-256 manifests, ACL save/restore, disk-full volumes, atomic promotion, and real failure injection.
