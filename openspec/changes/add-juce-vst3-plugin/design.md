## Context

ACE-Step 1.5 currently exposes generation through Python entry points, a Gradio UI, and API server flows. The proposed plugin lives under `ACE-Step-Plugin/` and brings ACE-Step into Windows DAWs as an offline generate-and-drop VST3 workflow. It must pass audio through unchanged during normal playback, capture routed host audio only when armed, run generation outside the audio thread, and expose generated WAV files for timeline import.

The plugin will use JUCE for the VST3 shell and UI, MSVC 2022 with C++17 for the Windows build, and `ServeurpersoCom/acestep.cpp` for GGML inference. The selected v1 integration is in-process static linking against `acestep-core`, with a build-time sidecar server fallback retained for upstream API churn. GGML CPU, CUDA, and Vulkan backends are bundled as runtime-loaded DLL siblings inside the VST3 bundle.

## Goals / Non-Goals

**Goals:**
- Create a JUCE VST3 project that builds a pass-through Windows plugin and can be loaded by VST3-capable DAWs.
- Keep plugin scanning fast by lazy-loading models only on generation request.
- Keep the audio callback real-time-safe: no allocation, file I/O, logging, locks, or string work in `processBlock`.
- Capture up to 60 seconds of host-routed stereo reference audio and snapshot it for offline ACE-Step generation.
- Run GGML generation on a single background worker with progress, cancellation, and UI-safe completion callbacks.
- Download or detect the four required GGUF model files, with resumable verified downloads.
- Export generated 24-bit WAV files through preview, Save As, and external file drag-and-drop.
- Export MIDI files for generated or derived musical structure when the engine can provide note/event data.
- Produce separated stem WAV assets for generated songs and expose them through the same asset workflow as full-mix WAV files.
- Provide a preset browser for reusable prompts, lyrics, generation parameters, scheduler choices, reference settings, and export preferences.
- Keep plugin-owned behavior consistent across supported VST3 DAWs and provide fallbacks or documentation for host-controlled differences.

**Non-Goals:**
- Real-time synthesis or live audio effects.
- MIDI CC automation, AAX, AU, Standalone, macOS, or Linux builds.
- Shipping GGUF model weights inside the plugin binary or installer.
- Guaranteeing behavior the plugin cannot control inside every DAW host, such as timeline insertion policy, file drag interpretation, sandbox prompts, or host-specific plugin scanning implementation.

## Decisions

### Track the JUCE plugin tree in-repo for now

The plugin lives in `ACE-Step-Plugin/` inside this repository for the current implementation phase. This keeps the C++ build graph, vendored external source, VST3 bundle outputs, and large model/runtime artifacts separate from the ACE-Step Python package while still allowing the plugin code to be tracked by the parent repository.

Alternatives considered: keeping the plugin in a separate sibling repository would reduce repository size but would require a separate remote and PR flow. Mixing C++ plugin code into the Python package would make cross-language dependency boundaries harder to review and risks introducing build artifacts into the Python project.

### Ship Mode A as default with Mode B fallback

Mode A links `acestep-core` in-process for lower overhead and a single plugin-hosted generation path. Because upstream currently exposes headers privately and has no C API, the plugin will carry the include promotion and minimal C API shim as a guarded patch until an upstream PR lands. All upstream-specific types remain behind `Source/Engine/AceStepCApi.h/.cpp`.

Mode B builds and bundles `ace-server.exe` behind `-DACESTEP_PLUGIN_MODE=server`. It is not the default path, but it gives users and maintainers an escape hatch when upstream internal APIs churn.

Alternatives considered: only using the sidecar server would lower coupling but add process lifecycle and HTTP failure modes to every v1 generation. Only using in-process linking would remove the fallback needed for a fast-moving upstream dependency.

### Bundle GGML backends as dynamic runtime modules

The Windows v1 bundle will build GGML with CPU, CUDA, and Vulkan backends in dynamic loading mode and copy `ggml-*.dll` files next to the plugin binary inside `Contents/x86_64-win/`. Runtime initialization calls the GGML loader with the bundle directory so the host process working directory does not influence backend discovery.

Alternatives considered: separate CPU/CUDA/Vulkan installers would reduce individual package size but create support fragmentation. Static backend linking would make host runtime conflicts and optional GPU support harder to manage.

### Use a real-time-safe reference ring

Reference capture uses a preallocated stereo ring backed by `juce::AbstractFifo`, with an atomic armed flag and drop-oldest overflow behavior. The audio callback writes only when armed. Snapshots are made from the message thread into immutable heap buffers handed to the worker.

Alternatives considered: writing capture directly to disk or locking a shared buffer would violate real-time safety in DAW hosts.

### Serialize generation on one worker

ACE-Step generation runs through a `juce::ThreadPool` with one worker. This avoids multiple GGML contexts contending for VRAM/RAM and keeps cancellation/progress state simple. The `AceStepEngine` lives on the processor so generation can survive editor close/reopen.

Alternatives considered: allowing multiple concurrent generations would look faster in the UI but increases out-of-memory risk and complicates GGML backend ownership.

### Download models in-plugin but keep manual placement

The plugin detects model files under `%LOCALAPPDATA%\AceStepPlugin\models\`. If required files are missing, the editor presents a downloader using `juce::URL` with HTTP Range resume, four parallel chunks, and SHA-256 verification from a shipped manifest. Manual placement remains supported for offline or advanced users.

Alternatives considered: requiring manual downloads would reduce plugin code but create a poor first-run experience. Bundling models would make distribution too large and inflexible.

### Treat MIDI, stems, and presets as v1 product workflows

MIDI export, stem separation, and preset browsing are v1 workflows rather than future backlog items. MIDI export will be offered only when the generation result includes reliable note/event data or when an implemented analysis path can derive it; otherwise the UI must explain that MIDI is unavailable for that asset. Stem separation will produce full-mix plus per-stem WAV assets and expose them through the asset history/export surface. Presets will be stored as plugin-owned JSON documents so users can reuse prompt, lyrics, generation parameters, scheduler, reference, and export preferences without coupling them to a DAW-specific preset format.

Alternatives considered: deferring these features would keep v1 narrower but would miss the requested DAW workflow completeness. Relying only on DAW preset mechanisms would make presets less portable and harder to use across hosts.

### Normalize host behavior where the plugin owns it

The plugin will target consistent behavior across supported VST3-capable DAWs for scanning, editor layout, pass-through processing, capture controls, generation state, asset history, preview, Save As, and export initiation. Where hosts control behavior, such as external drag insertion semantics, file import prompts, or timeline placement rules, the plugin will provide a fallback path and document known limitations.

Alternatives considered: promising identical behavior in every DAW is not technically enforceable because hosts own parts of VST3 loading, UI embedding, drag/drop, and timeline import behavior.

## Risks / Trade-offs

- Upstream `acestep.cpp` internal API changes -> isolate all direct calls in `AceStepCApi` and retain Mode B server fallback.
- DAW host instability from GPU/runtime DLL conflicts -> use GGML dynamic backend loading from the VST3 bundle and avoid OpenMP in the plugin build.
- Plugin scan timeout from heavyweight initialization -> defer GGUF loading and backend initialization until the first generation request.
- Out-of-memory failures on consumer systems -> serialize generation, default to unload-after-generate, expose a keep-loaded toggle only for power users, and report worker failures in the UI.
- Reference capture bugs causing audio glitches -> keep capture off by default, preallocate in `prepareToPlay`, and enforce no locks or allocation in `processBlock`.
- Hugging Face download interruption or corruption -> support Range resume, pause/resume, per-file progress, and SHA-256 verification before marking models usable.
- Host-specific external drag behavior -> validate with Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig, and keep Save As as a fallback export path.
- MIDI extraction may be unavailable for some generations -> gate MIDI export per asset and surface a clear unavailable state instead of producing misleading MIDI.
- Stem separation increases generation time and disk usage -> show stem progress separately and allow users to disable stem output per request or preset.
- Preset compatibility can drift as parameters evolve -> version preset files and migrate known schema versions on load.
- Host behavior cannot be fully controlled by the plugin -> define supported host expectations, verify plugin-owned behavior, and document host-owned differences.

## Migration Plan

This is a new plugin project, so there is no in-place migration for existing ACE-Step users. Implementation starts with the buildable pass-through plugin skeleton, then layers in capture, engine integration, model management, generation workflow, and export UI. Rollback for each phase is to keep the prior buildable plugin state and gate incomplete paths behind build options or UI-disabled states.

## Open Questions

- Exact JUCE tag and `acestep.cpp` commit pin must be selected during implementation and recorded in the vendored-source notes in `session-handoff.md`.
- Exact GGUF manifest filenames and SHA-256 values must be verified against the
  `Serveurperso/ACE-Step-1.5-GGUF` Hugging Face repository before downloader
  implementation.
- The available `acestep.cpp` outputs for reliable MIDI event data and stem generation must be verified before choosing whether MIDI/stems come directly from backend metadata, model output, or post-processing analysis.
- The upstream PR shape for `acestep-core` public headers and the C API shim must be validated against the current `acestep.cpp` source before creating `patches/0001-public-headers.patch`.
