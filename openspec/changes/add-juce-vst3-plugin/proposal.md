## Why

ACE-Step can generate full songs, but it currently has no DAW-native workflow for producers who want to capture musical context from a host session, generate offline with ACE-Step, and drag the resulting audio directly back into the timeline. A Windows VST3 plugin closes that workflow gap while keeping generation out of the real-time audio path.

## What Changes

- Add an `ACE-Step-Plugin/` project tree for a JUCE-based Windows 11 VST3 plugin.
- Provide a minimal VST3 pass-through plugin skeleton with CMake, JUCE, VS Code, and MSVC 2022 wiring.
- Integrate `ServeurpersoCom/acestep.cpp` as the primary GGML backend through an in-process `acestep-core` static link, with a sidecar `ace-server.exe` build option as fallback.
- Ship CPU, Vulkan, and CUDA GGML backend DLLs inside a single universal VST3 bundle and load them from the bundle at runtime.
- Add a first-run model setup flow that downloads or detects the ACE-Step 1.5 GGUF model files.
- Add host-routed reference audio capture, background offline generation, progress/cancel handling, and WAV-only output.
- Add generated asset history with waveform preview, playback, Save As, external file drag-and-drop, MIDI export, and stem export into VST3-capable DAWs.
- Add a preset browser for reusable generation settings and workflow templates.
- Add a host compatibility target for consistent plugin-owned behavior across supported VST3 DAWs, with host-controlled differences handled by fallbacks or documented limitations.
- Exclude live real-time generation, AAX/AU/Standalone formats, and non-Windows builds from v1.

## Capabilities

### New Capabilities
- `juce-vst3-plugin-shell`: Creates and builds the JUCE VST3 plugin project, including CMake integration, VS Code tooling, pass-through audio behavior, and bundled GGML backend DLLs.
- `acestep-cpp-integration`: Loads ACE-Step GGUF models and invokes the `acestep.cpp` generation pipeline through a contained engine API, with sidecar server fallback available at build time.
- `reference-audio-capture`: Captures up to 60 seconds of host-routed stereo audio using a real-time-safe buffer and provides immutable snapshots for generation.
- `model-management`: Detects local GGUF model files and downloads missing models with resumable, verified transfers inside the plugin UI.
- `background-generation-workflow`: Runs ACE-Step generation on a single background worker with progress, cancellation, error reporting, resampling, and temporary WAV output.
- `generated-wav-export`: Presents generated WAV assets with waveform preview, playback, history, Save As, and external file drag-and-drop into DAW timelines.
- `midi-export`: Exports generated or derived musical structure as MIDI files when ACE-Step metadata or analysis makes note/event extraction available.
- `stem-separation`: Produces and manages separated stem WAV assets for generated songs in addition to the full mix.
- `preset-browsing`: Provides browsing, saving, loading, and organizing reusable generation presets.
- `host-behavior-compatibility`: Defines consistent plugin-owned behavior and validation expectations across supported VST3 DAWs.

### Modified Capabilities
- None.

## Impact

- Adds a new plugin project layout, build system, JUCE plugin source files, CMake helper modules, patch files for upstream `acestep.cpp`, and Windows build documentation.
- Introduces vendored external source dependencies on JUCE and `ServeurpersoCom/acestep.cpp` plus runtime GGML backend DLL bundling.
- Requires MSVC 2022, CMake, CUDA Toolkit, and Vulkan SDK on the build machine for the universal Windows bundle.
- Requires approximately 7.7 GB of ACE-Step GGUF model storage under `%LOCALAPPDATA%\AceStepPlugin\models\` unless models are manually placed there.
- Keeps DAW plugin scanning fast by lazy-loading models only when generation is requested and keeping all GGML inference off the audio thread.
