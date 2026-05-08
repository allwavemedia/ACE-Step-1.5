## 1. Plugin Project Skeleton

- [x] 1.1 Create `ACE-Step-Plugin/` project with `Source/`, `Resources/`, `External/`, `cmake/`, `patches/`, and `.vscode/` directories.
- [x] 1.2 Add `.gitignore` entries for build outputs, model directories, generated bundles, and `*.gguf` files.
- [x] 1.3 Add pinned vendored JUCE and `ServeurpersoCom/acestep.cpp` external source, including recursive `ggml` source.
- [x] 1.4 Add `BUILD.md` with Windows prerequisites for MSVC 2022, CMake, CUDA Toolkit, Vulkan SDK, JUCE AudioPluginHost, and expected build commands.

## 2. CMake and JUCE Plugin Shell

- [x] 2.1 Add root `CMakeLists.txt` with project options, C++17 settings, JUCE setup, `ACESTEP_PLUGIN_MODE`, and VST3 target creation.
- [x] 2.2 Add `cmake/CompilerWarnings.cmake` with the MSVC `/W4`, `/MP`, `/utf-8`, and `/EHsc` baseline.
- [x] 2.3 Add `cmake/BundleBackends.cmake` to copy `ggml-*.dll` files into `Contents/x86_64-win/` after VST3 build.
- [x] 2.4 Add minimal `Source/PluginProcessor.h` and `Source/PluginProcessor.cpp` that configure stereo-in/stereo-out and pass audio through unchanged.
- [x] 2.5 Add minimal `Source/PluginEditor.h` and `Source/PluginEditor.cpp` with a stub editor that opens reliably in JUCE AudioPluginHost.
- [x] 2.6 Add `.vscode/settings.json`, `tasks.json`, and `launch.json` for MSVC kit selection, RelWithDebInfo VST3 build, and AudioPluginHost debugging.
- [ ] 2.7 Verify the initial VST3 bundle loads in JUCE AudioPluginHost and Reaper with unchanged audio pass-through.
  **Reaper:** PASS for current PR-head automated validation (2026-05-07). REAPER v7.71/x64 scanned and loaded `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3`, opened the FX UI (at 448 ms), and passed offline pass-through: `baseline_peak=0.249969482`, `enabled_peak=0.249969482`, `peak_diff=0.000000000`, `rms_diff=0.000000000`. Evidence is recorded in `ACE-Step-Plugin\docs\validate-host-load.md` and `ACE-Step-Plugin\docs\host-compatibility-matrix.md`.
  **AudioPluginHost:** Build blockers resolved 2026-05-07. The `extras/Build/` CMake infrastructure and the missing harfbuzz `OT/Var/VARC/` files have been restored from the official JUCE 8.0.10 GitHub release. A developer with Visual Studio 2022 and CMake can now build AudioPluginHost from the vendored tree using `python ACE-Step-Plugin\scripts\generate-stub-juce-assets.py` followed by the CMake commands in `ACE-Step-Plugin\docs\audiopluginhost-blocker-investigation.md`. Pass-through validation must still be executed and evidence recorded before this task can be marked complete.
  **Task 2.7 status:** Reaper evidence is current (2026-05-07). AudioPluginHost build is now unblocked; pass-through evidence still pending.

## 3. ACE-Step C++ Backend Integration

- [x] 3.1 Add `cmake/AcestepIntegration.cmake` to configure GGML options, add `External/acestep_cpp` with `EXCLUDE_FROM_ALL`, and link only required targets.
- [x] 3.2 Add guarded patch application for `patches/0001-public-headers.patch` using a check-before-apply CMake flow.
- [x] 3.3 Create `patches/0001-public-headers.patch` to promote required `acestep-core` include directories and add a minimal `src/acestep_capi.h` shim.
- [x] 3.4 Add `Source/Engine/AceStepCApi.h` and `.cpp` as the only plugin-owned boundary that includes upstream `acestep.cpp` internals.
- [x] 3.5 Implement bundle-local GGML backend initialization that points the loader at the VST3 `Contents/x86_64-win/` directory.
- [x] 3.6 Add `ACESTEP_PLUGIN_MODE=server` support that builds `ace-server` and bundles it as a sidecar only when selected.
- [x] 3.7 Verify `dumpbin /dependents` and bundle contents show expected plugin and GGML runtime DLL relationships.
  Evidence: real backend VST3 built successfully in `C:\b\ace-ninja` using Visual Studio 2022, CUDA Toolkit 13.2.1, Vulkan SDK 1.4.341.1, Ninja 1.12.1, and `-DCMAKE_CUDA_ARCHITECTURES=89-real` for the local RTX 4090 validation machine. `scripts\validate-bundle.ps1 -BuildDir C:\b\ace-ninja -Config RelWithDebInfo` passed. Raw `dumpbin /dependents` output shows `ACE-Step.vst3` imports `ggml.dll` without direct CUDA runtime imports; `ggml-cuda.dll` imports `ggml-base.dll` and `cublas64_13.dll`; `ggml-vulkan.dll` imports `ggml-base.dll` and `vulkan-1.dll`. Full evidence is recorded in `ACE-Step-Plugin\BUILD.md`.

## 4. Reference Audio Capture

- [x] 4.1 Add `ReferenceAudioBuffer` with preallocated stereo capacity for at least 60 seconds at 48 kHz and `juce::AbstractFifo` coordination.
- [x] 4.2 Wire `prepareToPlay` to allocate/reset capture state without doing work in the constructor.
- [x] 4.3 Add atomic capture arm and clear controls on the processor side.
- [x] 4.4 Update `processBlock` to write host input into the reference buffer only when armed, with no locks, allocation, file I/O, logging, or string work.
- [x] 4.5 Implement drop-oldest overflow behavior and immutable snapshot creation for worker consumption.
- [x] 4.6 Add editor controls for Arm, Clear, capture source label, and atomic peak/VU display.
- [x] 4.7 Add focused tests for round-trip capture, overflow retention, disabled capture, and snapshot immutability.

## 5. Model Management

- [x] 5.1 Define the model manifest with required GGUF filenames, expected sizes, Hugging Face URLs, and SHA-256 values.
- [x] 5.2 Implement local model discovery under `%LOCALAPPDATA%\AceStepPlugin\models\` with validation before ready state.
- [x] 5.3 Add first-run setup UI for missing models with destination path and approximate total download size.
- [x] 5.4 Implement resumable `juce::URL` downloads with HTTP Range support, four parallel chunks, pause/resume, and per-file progress.
- [x] 5.5 Verify downloaded files with SHA-256 before making them available to the engine.
- [x] 5.6 Add error handling for failed downloads, checksum mismatch, partial files, and insufficient disk space.

> Functional parity note: runtime validation showed the current VST3 editor exposes only pass-through, capture, model status, and preset surfaces. Full prompt-to-WAV generation, generated assets, MIDI export, and stem export are superseded by `openspec/changes/add-vst3-functional-parity/`.

## 6. Background Generation Engine

- [x] 6.1 Add `GenerationRequest` and `GenerationResult` types covering prompt, lyrics, duration, seed, CFG scale, LM seed, scheduler, optional reference audio, output path, and error state.
- [x] 6.2 Implement `AceStepEngine` ownership on the processor with `isReady`, `loadModels`, `generate`, and `cancelAll` methods.
- [x] 6.3 Mirror the `acestep.cpp` `ace-synth` ordering for model store, LM planning, synth pipeline, scheduler choice, and WAV output.
- [x] 6.4 Run generation through a `juce::ThreadPool` with one worker and prevent concurrent GGML jobs per plugin instance.
- [x] 6.5 Resample captured reference audio to the expected ACE-Step sample rate on the worker thread before audio-conditioned generation.
- [x] 6.6 Add progress messages for model loading, LM planning, DiT steps, VAE decode, WAV write, completion, cancellation, and failure.
- [x] 6.7 Add cancellation polling at supported pipeline points and join/cleanup behavior during processor destruction.
- [x] 6.8 Add mock-engine tests proving UI responsiveness, cancellation behavior, pass-through stability, and successful temporary WAV result handling.
- [x] 6.9 Run a real-engine smoke test with turbo generation once model files and GGML backends are available.
  Evidence: downloaded all four required GGUF files into `%LOCALAPPDATA%\AceStepPlugin\models` with manifest-matching sizes, then ran `C:\b\ace-ninja\AceStepRealSmokeTest_artefacts\RelWithDebInfo\AceStepRealSmokeTest.exe`. The test launched `C:\b\ace-ninja\ace-synth.exe` with the smoke request and produced `%TEMP%\acestep_smoke_request0.wav` at `967,724` bytes.

## 7. Generated WAV Export UI

- [x] 7.1 Add generated asset model state for the most recent eight successful generation results.
- [x] 7.2 Add `GeneratedAssetTile` with waveform thumbnail, filename, duration, play/stop controls, drag state, and Save As action.
- [x] 7.3 Add preview playback through a dedicated preview path that does not alter the host audio pass-through path.
- [x] 7.4 Implement external file drag via `performExternalDragDropOfFiles` with copy semantics for generated WAV paths.
- [x] 7.5 Add a scrollable history area for multiple generated assets.
- [x] 7.6 Clean up plugin-owned temporary generation directories when the plugin instance is destroyed while preserving user-saved copies.

## 8. MIDI Export

- [x] 8.1 Verify whether `acestep.cpp` exposes reliable note/event data or whether a post-processing analysis path is required for MIDI extraction.
  Evidence: vendored `acestep.cpp` exposes WAV generation through `acestep_generate_wav`; its request/API surface carries generated `audio_codes`/FSQ tokens, not note, onset, or MIDI event data. No MIDI API is present in `src/` or `docs/`, so plugin MIDI export must remain unavailable until a post-processing analysis path is added.
- [x] 8.2 Add per-asset MIDI availability state that disables MIDI export when reliable note/event data is unavailable.
- [x] 8.3 Implement standards-compliant `.mid` file writing for eligible generated assets.
- [x] 8.4 Add MIDI drag-and-drop and Save As export paths with copy semantics.
- [x] 8.5 Add tests for MIDI availability gating, file writing, unavailable-state handling, and export path creation.

## 9. Stem Separation

- [x] 9.1 Verify the selected ACE-Step or post-processing path for producing separated stem WAV files from generated songs.
  Evidence: vendored `acestep.cpp` has internal stem task modes (`lego`, `extract`, `complete`) and documented track names, with `extract` described as stem isolation from source audio. The current plugin-facing `acestep_capi.h` only exposes `acestep_generate_wav` with prompt/lyrics/models/output WAV/duration/seed/scheduler fields and does not expose `task_type`, `track`, source-audio input, base-model selection, or multi-output stem metadata, so plugin stem controls remain gated until that backend boundary is implemented.
- [x] 9.2 Add generation request options for enabling stem output and selecting available stem groups.
- [x] 9.3 Add worker progress and error reporting for stem output separately from full-mix generation.
- [x] 9.4 Represent full-mix and stem WAV files as grouped generated assets in history.
- [x] 9.5 Add independent preview, drag, Save As, and cleanup handling for each stem WAV.
- [x] 9.6 Add tests for successful stem grouping, partial stem failure, and individual stem export.

## 10. Preset Browser

- [x] 10.1 Define a versioned preset JSON schema for prompt, lyrics, duration, seeds, CFG scale, scheduler, reference options, stem options, MIDI options, and export preferences.
- [x] 10.2 Implement preset save, load, rename, delete, and schema migration services.
- [x] 10.3 Add preset browser UI with listing, search/filtering, load, rename, delete, and save-current actions.
- [x] 10.4 Ensure loading a preset updates editor fields without starting generation automatically.
- [x] 10.5 Add tests for preset persistence, load behavior, migration, invalid preset errors, and DAW-independent storage paths.

## 11. Host Behavior Compatibility

- [x] 11.1 Define the supported v1 DAW validation matrix and the plugin-owned behaviors expected to match across hosts.
- [x] 11.2 Add compatibility notes for host-controlled differences such as external drag insertion location, file import prompts, and scan behavior.
- [ ] 11.3 Verify scan/load, pass-through, editor layout, capture controls, generation UI state, WAV export, MIDI export when available, stem export, and preset browsing across Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig.
  **Reaper:** PASS for current PR-head automated validation (2026-05-07) for scan/load, FX UI-open, and offline pass-through. Remaining Reaper interactive UI/export checks and all other DAH validations still pending.
  **Current blocker:** FL Studio 21, Cubase 13, Studio One 6, Ableton Live 12 Suite, and Bitwig Studio are blocked because they are not installed. Do not mark this task complete until every required host is validated or the OpenSpec scope explicitly accepts the blockers.
  **AudioPluginHost:** Build blockers resolved 2026-05-07; see task 2.7 and `audiopluginhost-blocker-investigation.md`. Pass-through validation still pending.
- [x] 11.4 Add fallback documentation for hosts where external drag-and-drop differs from the common path.

## 12. Validation and Release Readiness

- [x] 12.1 Verify DAW scan time stays fast because GGUF models are not loaded during plugin construction or scan.
  Evidence: current real-bundle Reaper automation found `VST3: ACE-Step (Allwave Media)` within `318 ms` of ReaScript start for the scan/load probe and within `182 ms` for the pass-through probe. After all four GGUF model files were installed under `%LOCALAPPDATA%\AceStepPlugin\models`, a fresh isolated Reaper profile found the same real bundle within `199 ms` of ReaScript start (`wall_ms=17105`, including REAPER startup wait). No model load occurred during plugin construction or scan.
- [x] 12.2 Verify no allocation, logging, file I/O, mutex locking, or string construction occurs inside `processBlock` capture logic.
  Evidence: inspected `PluginProcessor::processBlock`; the audio callback only enters `ScopedNoDenormals`, ignores MIDI, calls `referenceAudioBuffer.push(buffer)`, and clears surplus output channels. `ReferenceAudioBuffer::push` uses atomics/preallocated storage and contains no allocation, logging, file I/O, mutex locking, or string construction on the audio thread.
- [x] 12.3 Verify the VST3 bundle contains the plugin DLL plus CPU, CUDA, and Vulkan GGML backend DLL siblings.
  Evidence: the real bundle directory `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3\Contents\x86_64-win` contains `ACE-Step.vst3`, `ggml.dll`, `ggml-base.dll`, `ggml-cpu.dll`, `ggml-cuda.dll`, and `ggml-vulkan.dll`; `scripts\validate-bundle.ps1` passed against that bundle.
- [ ] 12.4 Verify external drag-and-drop in Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig, with Save As as fallback.
  **Current blocker:** Reaper drag/drop remains pending manual validation, and FL Studio 21, Cubase 13, Studio One 6, Ableton Live 12 Suite, and Bitwig Studio are blocked because they are not installed at the checked paths. Do not mark this task complete until every required host is validated or the OpenSpec scope explicitly accepts the blockers.
- [ ] 12.5 Verify missing model, checksum mismatch, out-of-memory, backend-load failure, cancellation, generation failure, MIDI unavailable state, stem failure, preset load failure, and host compatibility errors surface in the editor without crashing the host.
  **Unit-test coverage (automated):** `ModelDiscovery_test.cpp` covers missing-model enumeration. `ModelChecksum_test.cpp` covers checksum-mismatch false return. `AceStepEngineMock_test.cpp` covers cancellation flag delivery and error-result path. `StemCapability_test.cpp` and `MidiFileWriter_test.cpp` cover gated-unavailable states.
  **Current blocker:** Destructive/manual error-state validation (e.g., GGUF file corruption or deliberate backend-load failure) has not been executed against the real runtime at commit `f58ab9e75975`. Do not mark this task complete until all non-blocked error states pass and any blocked unsafe cases are explicitly accepted by scope.
- [x] 12.6 Update build and troubleshooting documentation with final external source pins, SDK versions, model manifest details, MIDI/stem capability details, preset storage details, and known host limitations.
