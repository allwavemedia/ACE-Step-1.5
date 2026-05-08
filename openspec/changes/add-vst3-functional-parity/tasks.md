## 1. Supersession Baseline and Scope

- [ ] 1.1 Audit the current VST3 runtime editor and record which generation/export surfaces are actually reachable.
- [ ] 1.2 Confirm pass-through, capture, model status, and preset behavior remain the non-regression baseline.
- [ ] 1.3 Map superseded `add-juce-vst3-plugin` functional claims to this change so implementation work has one source of truth.

## 2. Runtime Boundary and Sidecar Contract

- [x] 2.1 Define the lazy per-host-process sidecar lifecycle, including launch only after explicit user action.
- [x] 2.2 Start the helper by absolute `CreateProcessW` path and attach it to a kill-on-close Windows job object.
- [x] 2.3 Define versioned JSON request/response schemas over Windows named pipes.
- [x] 2.4 Verify no sidecar launch, model load, file I/O, or IPC occurs during host scan, editor creation, or `processBlock`.

## 3. Artifact Workspace and Result Verification

- [x] 3.1 Create per-job artifact directories under LocalAppData/temp with deterministic cleanup rules.
- [x] 3.2 Require schema version, request ID, byte size, SHA-256, expected artifact count, and readability checks before promoting results.
- [x] 3.3 Prevent partial or malformed outputs from creating success-shaped generated asset tiles.
- [ ] 3.4 Preserve successful full-mix output when later MIDI or stem work partially fails.

## 4. Single-Scroll Editor Shell

- [x] 4.1 Replace hidden or unreachable generation/export surfaces with one fixed-size, single-scroll workflow.
- [x] 4.2 Add visible sections for setup, generation form, capture, progress, generated assets, exports, presets, and diagnostics.
- [x] 4.3 Keep the editor usable in constrained VST3 host windows without tabs, wizard steps, or resize-dependent controls.
- [x] 4.4 Add accessibility labels and deterministic layout behavior for host validation.

## 5. Model and Tool Setup

- [ ] 5.1 Show ACE-Step model status, required sidecar tool status, and download/install progress in the editor.
- [ ] 5.2 Validate required local ACE-Step assets before enabling generation.
- [ ] 5.3 Validate Basic Pitch, Demucs, and fallback tool availability before enabling dependent exports.
- [ ] 5.4 Surface missing, corrupt, incompatible, or insufficient-disk setup failures with explicit recovery text.

## 6. Prompt-to-WAV Generation Workflow

- [x] 6.1 Add prompt, lyrics, duration, seed, CFG scale, LM seed, scheduler, and captured-reference controls.
- [ ] 6.2 Wire Generate and Cancel controls to sidecar orchestration with progress phases and user-visible errors.
- [ ] 6.3 Ensure generation runs off the audio thread and does not destabilize pass-through audio.
- [x] 6.4 Persist completed WAV artifacts into generated asset history only after verification succeeds.

## 7. Capture Integration

- [ ] 7.1 Keep Arm, Clear, and meter controls visible in the single-scroll editor.
- [ ] 7.2 Connect the use-captured-reference toggle to immutable capture snapshots for generation requests.
- [ ] 7.3 Verify capture controls remain safe while generation, MIDI export, or stem export is running.
- [ ] 7.4 Preserve the existing audio callback real-time-safety guarantees.

## 8. Generated Asset History and Preview

- [ ] 8.1 Present successful full-mix generations as asset tiles with metadata, progress history, and error state separation.
- [ ] 8.2 Add per-asset preview playback that does not alter host pass-through processing.
- [ ] 8.3 Keep multiple generated assets available in a scrollable history.
- [ ] 8.4 Clean plugin-owned temporary assets without deleting user-saved exports.

## 9. WAV Export

- [ ] 9.1 Implement per-asset WAV Save As with copy semantics and clear overwrite/error handling.
- [ ] 9.2 Implement WAV external drag/drop from each asset tile.
- [ ] 9.3 Verify WAV export against unreadable source, denied destination, existing file, and disk-full states.
- [ ] 9.4 Document host-owned differences in drag/drop timeline insertion behavior.

## 10. MIDI Export

- [ ] 10.1 Integrate Basic Pitch official Python package in the sidecar for v1 audio-to-MIDI export.
- [ ] 10.2 Emit both rendered `.mid` files and backend-neutral `note_events.json`.
- [ ] 10.3 Add MT3/MR-MT3 through `mt3-infer` as the first fallback/upgrade path.
- [ ] 10.4 Add per-asset MIDI Save As, MIDI drag/drop, progress, cancellation, and explicit failure states.
- [ ] 10.5 Validate MIDI note timing, empty-output handling, corrupt input WAV handling, and unavailable-tool messaging.

## 11. Stem Export

- [ ] 11.1 Integrate Demucs `htdemucs_ft` in the sidecar for v1 stem separation.
- [ ] 11.2 Export `standard-4` stem WAVs: vocals, drums, bass, and other.
- [ ] 11.3 Keep SCNet Large as the first fallback path and keep native ACE-Step stem modes experimental until validated.
- [ ] 11.4 Add per-stem Save As, drag/drop, preview, progress, cancellation, and partial-failure handling.
- [ ] 11.5 Validate missing stem, corrupt stem, partial success, and disk-full export behavior.

## 12. Presets and Diagnostics

- [ ] 12.1 Keep preset browsing reachable in the single-scroll editor.
- [ ] 12.2 Ensure presets cover prompt, lyrics, duration, seeds, CFG scale, scheduler, reference usage, MIDI options, and stem options.
- [ ] 12.3 Add a diagnostics panel for model/tool paths, sidecar state, last request IDs, artifact paths, and validation failures.
- [ ] 12.4 Surface every user-visible failure as explicit editor text instead of generic failure banners.

## 13. Destructive Validation

- [ ] 13.1 Build disposable validation workspaces or VHDX layers for destructive model, sidecar, generation, MIDI, stem, preset, and host error states.
- [ ] 13.2 Use SHA-256 manifests, ACL save/restore, disk-full volumes, atomic promotion, and real failure injection.
- [ ] 13.3 Verify failures never crash or hang the host and never promote partial final artifacts.
- [ ] 13.4 Record evidence for each destructive scenario with expected and observed recovery behavior.

## 14. Host and Release Validation

- [ ] 14.1 Run Steinberg Validator, VST3 Plug-in Test Host, pluginval, and JUCE AudioPluginHost validation.
- [ ] 14.2 Validate Reaper plus at least two locally installed VST3 production hosts.
- [ ] 14.3 Use `C:\Program Files\Common Files\VST3` for host validation installs.
- [ ] 14.4 Verify scan/load, pass-through, editor layout, generation, cancellation, WAV export, MIDI export, stem export, presets, diagnostics, and host error states.
- [ ] 14.5 Update release notes and compatibility documentation with validated hosts, known host-owned differences, and remaining limitations.
