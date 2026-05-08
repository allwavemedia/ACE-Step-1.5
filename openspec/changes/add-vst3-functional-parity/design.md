# VST3 Functional Parity Design

## Current Gap

The current editor wires only capture, model status, a meter, and presets. Standalone components for generation results and export exist, but they are not reachable in the running VST3. This change makes the VST3 release path functionally complete for prompt-to-WAV generation, WAV export, MIDI export, stem export, presets, diagnostics, and host-safe failure recovery.

## Architecture Mode Decision

The v1 recovery runtime contract is **sidecar-first** for all heavyweight work: ACE-Step generation, Basic Pitch/MT3 MIDI transcription, and Demucs/SCNet stem separation. This intentionally supersedes the previous in-process ACE-Step generation path for release-grade VST3 functional parity because host scan/load, editor creation, and `processBlock` must stay lightweight and stable.

The in-process ACE-Step/acestep.cpp boundary may remain as a lower-level library surface or future optimization, but it is not the release contract for user-facing v1 functional parity. The plugin boundary is a typed job facade that submits work to the lazy helper, validates staged result manifests, and imports only verified artifacts.

## Runtime Boundary

The audio callback remains pass-through-only. The plugin must not launch helpers, load models, perform filesystem work, open IPC, run model discovery, or wait on background jobs during host scan, construction, editor creation, `prepareToPlay`, or `processBlock`.

Heavy work starts only after explicit user action on a message/background thread. The plugin starts a lazy per-host-process helper by absolute `CreateProcessW` path from `ACE-Step.vst3\Contents\Resources\ACE-Step\helpers\`, attaches it to a Windows job object with kill-on-close, and communicates with versioned JSON request/response envelopes over Windows named pipes. The helper uses a private embedded/frozen runtime for Python-based tools and never resolves `python.exe` from user PATH.

## Single-Scroll Editor

The editor is a fixed-size, single-column, single-scroll workflow with no tabs, wizard steps, horizontal scrolling, or resize-dependent primary controls. It contains:

1. ACE-Step model status plus Basic Pitch, Demucs, MT3/MR-MT3, and SCNet tool status.
2. Download/install/provisioning progress for packaged or staged local assets.
3. Generation form: prompt, lyrics, duration, seed, CFG scale, LM seed, scheduler, and use-captured-reference toggle.
4. Capture controls: Arm, Clear, meter, and immutable captured-reference snapshot state.
5. Generate/Cancel controls with phase, progress, and explicit errors.
6. Generated full-mix asset history with metadata and preview.
7. Per-asset WAV Save As, WAV drag/drop, MIDI export, MIDI drag/drop, stem export, stem drag/drop, and per-export progress/error state.
8. Preset browser and dirty-state/save controls.
9. Diagnostics panel with sidecar state, model/tool paths, last request IDs, artifact roots, manifest validation failures, and copyable support text.

## Sidecar Orchestration and Artifacts

The control plane uses versioned JSON envelopes on Windows named pipes for hello, job submit, progress, completion, failure, cancellation, and shutdown. Each message includes protocol version, request ID, stable job kind, and stable error codes.

Large artifacts are never streamed through the control pipe. Each job writes to a per-job staging directory under LocalAppData or a temp root. The sidecar publishes a result manifest only after all expected files are complete. The plugin trusts results only after validating schema version, request ID, byte size, SHA-256, expected artifact count, file readability, and capability-specific sanity checks. Partial or malformed results must not create success-shaped generated asset tiles.

Cleanup uses job-object kill-on-close for the helper process tree and a janitor pass that removes orphaned job roots older than a conservative TTL. Successful full-mix WAVs remain available when later MIDI or stem jobs fail, but failed dependent exports stay marked with explicit failure text.

## MIDI Export

V1 MIDI export uses the Basic Pitch official Python package inside the sidecar. The default invocation is:

```powershell
basic-pitch <output-dir> <input.wav> --save-note-events --save-model-outputs
```

For repeated jobs, the sidecar keeps a warm `basic_pitch.inference.Model(ICASSP_2022_MODEL_PATH)` instance. The input is the generated full-mix WAV. The output bundle contains `output.mid`, `note_events.json`, optional `note_events.csv`, optional model-output diagnostics, and a diagnostics manifest. The internal plugin contract is backend-neutral note events plus rendered MIDI so the backend can change without changing the UI contract.

`mt3-infer` with `mr_mt3` is the first fallback/upgrade path. MT3/MR-MT3 remains behind the same note-event and rendered-MIDI contract. MIDI acceptance requires a parseable MIDI file that imports into Reaper and at least one strict parser, note count and duration sanity checks, and simple fixture note-onset F1 measured with 50 ms tolerance.

## Stem Export

V1 stem export uses Demucs `htdemucs_ft` inside the sidecar. The default invocation is:

```powershell
python.exe -m demucs -n htdemucs_ft <input.wav>
```

The `python.exe` here is the sidecar's private embedded/frozen runtime, not user PATH Python. The input is the generated full-mix WAV. The output is the `standard-4` stem group staged under the job output directory: `vocals.wav`, `drums.wav`, `bass.wav`, and `other.wav`.

SCNet Large is the first fallback path. Native ACE-Step stem modes remain experimental until validated against release-grade outputs and model compatibility. Stem acceptance requires four valid WAVs, duration matching the source within tolerance, no zero-byte outputs, strong summed-stem correlation with the source, and a manifest recording model ID, checkpoint hash, sample rate, channel count, filenames, and completion timestamp.

## Error Handling

Every user-visible failure maps to explicit editor text and diagnostics. Common classes include missing models/tools, sidecar unavailable, tool unavailable, checksum failure, invalid output, timeout, cancellation, disk full, permission denied, and sidecar crash. The main UI shows short recovery-oriented text; diagnostics include request ID, helper version, paths, hashes, exit codes, and raw Windows errors where applicable.

A failed export must not create a final artifact unless the artifact is complete and valid. Corrupt manifests, wrong request IDs, missing files, zero-byte files, unreadable files, and hash mismatches are rejected before promotion. Preset/state corruption must not overwrite current runtime state until validation succeeds.

## Validation

Validation covers Steinberg Validator, VST3 Plug-in Test Host, pluginval, JUCE AudioPluginHost, Reaper, and at least two locally installed VST3 production hosts. Host validation installs to `C:\Program Files\Common Files\VST3`.

Destructive validation uses disposable workspaces or VHDX layers and never targets the only known-good model copy. The validation harness records SHA-256 manifests before and after runs, uses ACL save/restore for permission tests, uses real disk-full volumes, validates atomic promotion, and captures evidence for sidecar launch failure, sidecar crash, cancellation, MIDI failure, stem failure, corrupt output, corrupt presets, and host recovery. Basic Pitch and Demucs are not considered implementation-complete until packaged-runtime smoke tests, tool availability checks, artifact validation, Reaper import checks, and host validation evidence pass.
