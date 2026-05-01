# Plan: ACE-Step JUCE VST3 Plugin (Windows 11)

A standalone sibling repo containing a JUCE-based VST3 plugin that captures
context-aware reference audio from the host, runs ACE-Step generation on a
background thread via the user-provided `acestep.cpp` (GGML) backend, and
exposes generated `.wav` files via drag-and-drop into the DAW timeline.

User confirmed: `ServeurpersoCom/acestep.cpp` (MIT) · models shipped upstream
(`Serveurperso/ACE-Step-1.5-GGUF` on HF) · sibling repo · VST3 only · WAV-only
v1 · 60s reference capture · all VST3-capable DAWs.

### Upstream reality check (decisions baked in below)

- `acestep-core` is a `STATIC` CMake library target — linkable.
- BUT: all headers live in `src/*.h` and are exposed via `PRIVATE` include
  dirs. No public C/C++ API surface, no `extern "C"` shim. Closest reference
  implementations: `tools/ace-synth.cpp` (offline pipeline) and
  `tools/ace-server.cpp` (HTTP).
- GGML backends are loaded **dynamically at runtime** via
  `ggml_backend_load_all()` — `ggml-cpu.dll`, `ggml-cuda.dll`,
  `ggml-vulkan.dll` are sibling files of the executable. We must ship those
  inside the VST3 bundle and steer the loader to find them.
- Total weights ~7.7 GB (LM 4.2 + text-enc 0.75 + DiT 2.4 + VAE 0.32). Lazy
  load only when user clicks Generate; show download/scan UX up front.
- Generation is **offline batch** (text + lyrics → stereo 48 kHz WAV/MP3).
  Not real-time. Our plugin is a "generate-and-drop" workflow, NOT a live
  effect.

### Architecture decision: two viable integration modes

We will support BOTH and ship Mode A as v1, with Mode B as a clean fallback
behind a CMake option, because Mode A carries real upstream-coupling risk.

- **Mode A — In-process static link** (preferred, lower latency, single
  binary): plugin links `acestep-core` directly. Requires either
  (a) adding `target_include_directories(acestep-core PUBLIC src)` to the
  upstream CMake (trivial PR), or (b) carrying a tiny patch in our
  submodule that promotes the include dir + adds an `extern "C"` shim
  header `src/acestep_capi.h` calling into the existing pipeline classes.
- **Mode B — Sidecar `ace-server.exe`** (fallback, zero upstream coupling):
  plugin spawns `ace-server` as a child process bound to a random
  localhost port, talks JSON over httplib-compatible REST. Survives
  upstream API churn; trivially supportable for any future ACE updates.

Default v1 ships Mode A. Mode B selectable via `ACESTEP_PLUGIN_MODE=server`
CMake option for users hitting integration breakage.

### Resolved decisions (May 2026)

- **Integration mode:** Mode A (in-process static link) is v1 default.
  Mode B retained as `-DACESTEP_PLUGIN_MODE=server` build option only.
- **Upstream PR:** authorized — open a PR against
  `ServeurpersoCom/acestep.cpp` promoting `acestep-core` include dirs to
  `PUBLIC` and adding a minimal `src/acestep_capi.h` `extern "C"` shim.
  Until merged, carry the same change as `patches/0001-public-headers.patch`
  applied via CMake `execute_process(git apply ...)` at configure time
  (idempotent — checks `git apply --check` first).
- **GPU backend:** **single universal bundle** — CPU + Vulkan + CUDA
  backends all built as `MODULE_LIBRARY` (DL mode) and shipped inside the
  VST3. Mirrors upstream's `buildall.cmd`. Runtime loader (GGML) picks the
  best available backend on the user's box. Distributable ~400 MB; users
  with no GPU silently fall back to CPU. One installer, no SKU split.
- **Model distribution:** **in-plugin downloader** is the v1 default.
  Built on `juce::URL` with HTTP `Range:` resume support, SHA-256 verify
  per file, parallel chunks (4), pause/resume UI. Manual drop into
  `%LOCALAPPDATA%\AceStepPlugin\models\` remains supported as a power-user
  escape hatch (auto-detected on startup).

---

## Phase 1 — Project Skeleton & Build (Step 1 of user's iterative workflow)

1. Create sibling repo `ACE-Step-Plugin/` with this top-level layout:
   - `CMakeLists.txt` (root)
   - `Source/` (plugin C++)
   - `Resources/` (icons, default plugin presets)
   - `External/JUCE/` (submodule, pinned tag)
   - `External/acestep_cpp/` (submodule of `ServeurpersoCom/acestep.cpp`,
     `--recurse-submodules` to pull its `ggml/` submodule)
   - `cmake/` (helpers: AcestepIntegration.cmake, BundleBackends.cmake,
     CompilerWarnings.cmake)
   - `patches/` (small guarded patches against acestep.cpp if upstream
     PR for public-headers hasn't landed)
   - `.vscode/` (CMake Tools + clangd configs)
   - `.gitignore` for `build/`, `Models/`, `*.gguf`
2. JUCE: pinned tag via git submodule (FetchContent risks recursive cmake
   re-fetch on clean). `juce_add_plugin(AceStepPlugin FORMATS VST3
   COMPANY_NAME "..." PLUGIN_MANUFACTURER_CODE Acst PLUGIN_CODE Acsg
   IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
   PRODUCT_NAME "ACE-Step")`. Channel config: stereo-in/stereo-out only.
3. Integrate acestep.cpp via `add_subdirectory(External/acestep_cpp
   EXCLUDE_FROM_ALL)`. Link only `acestep-core`. The upstream CMake also
   declares `ace-synth`, `ace-server`, `ace-lm`, `ace-understand`,
   `quantize`, `mp3-codec` as executable targets — leave them in
   EXCLUDE_FROM_ALL (they won't build unless we list them in our default
   target). For Mode B (sidecar) builds, add `ace-server` as a dependency
   and copy it into the VST3 bundle's resource dir.
4. Promote the private `src/` include dir for our plugin target via either:
   - **Preferred:** open upstream PR adding
     `target_include_directories(acestep-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)`.
   - **Until merged:** in our `cmake/AcestepIntegration.cmake`, after
     `add_subdirectory`, call
     `target_include_directories(AceStepPlugin PRIVATE
     "${CMAKE_SOURCE_DIR}/External/acestep_cpp/src"
     "${CMAKE_SOURCE_DIR}/External/acestep_cpp")`.
   - Add a thin C++ wrapper `Source/Engine/AceStepCApi.h/.cpp` that owns
     the only knowledge of upstream internal types — keeps blast radius
     contained when upstream churns.
5. Backend DLL bundling (`cmake/BundleBackends.cmake`):
   - After acestep build, glob `${CMAKE_BINARY_DIR}/External/acestep_cpp/ggml-*.dll`
   - Copy them into the VST3 bundle's
     `Contents/x86_64-win/` directory next to `AceStepPlugin.vst3` binary.
   - At runtime, call `ggml_backend_load_all_from_path(<bundle dir>)`
     during first-generation init so GGML finds them inside the bundle
     instead of the host process's working directory.
6. GGML build options for Windows v1 (universal bundle):
   `GGML_AVX2=ON`, `GGML_FMA=ON`, `GGML_OPENMP=OFF` (avoid clashing with
   the DAW's OMP runtime), `GGML_BACKEND_DL=ON` (DL mode — backends as
   `MODULE_LIBRARY`), `GGML_CUDA=ON`, `GGML_VULKAN=ON`, `GGML_CPU=ON`.
   Build all three as separate DLLs that ship inside the VST3 bundle;
   GGML's runtime loader picks the best available on the user's machine
   (CUDA on NVIDIA, Vulkan on AMD/Intel, CPU fallback otherwise).
   Requires CUDA Toolkit + Vulkan SDK installed on the build machine
   (documented in `BUILD.md`); end users need no SDKs.
7. Toolchain: MSVC 2022, C++17 (matches upstream — JUCE supports 17), `/W4
   /MP /utf-8 /EHsc`, `JUCE_REPORT_APP_USAGE=0`,
   `JUCE_USE_CURL=1` (needed by the model downloader for HTTPS to Hugging
   Face — links system libcurl on Windows via vcpkg or the JUCE-bundled
   curl on platforms that provide it),
   `JUCE_DISPLAY_SPLASH_SCREEN=0` (set per JUCE license terms).
8. VS Code wiring: `.vscode/settings.json` selects MSVC kit + clangd LSP,
   `tasks.json` builds `AceStepPlugin_VST3` in `RelWithDebInfo`,
   `launch.json` attaches MSVC debugger to JUCE `AudioPluginHost.exe` with
   the plugin auto-loaded.

**Verification:** `.vst3` bundle contains plugin DLL + `ggml-*.dll`
backend siblings. Loads in `AudioPluginHost.exe` and Reaper. Stub editor
shows; audio passes through unchanged. `dumpbin /dependents` confirms no
unexpected runtime DLLs leaked into host process.

## Phase 2 — Context-Aware Reference Audio Capture

7. Add a `ReferenceAudioBuffer` class wrapping `juce::AbstractFifo` for
   lock-free SPSC handoff from `processBlock` (producer) to the inference
   thread (consumer). Capacity: 60 s × 48 kHz × 2 ch × float32 ≈ 22 MB,
   allocated once at `prepareToPlay`. Use `std::atomic<bool> capturing`
   to gate writes — default OFF so idle plugins have zero overhead.
8. Inside `processBlock`: when `capturing.load(std::memory_order_acquire)` is
   true, copy the input block into the FIFO (drop oldest on overflow → ring).
   No locks, no allocations, no `juce::String`, no logging on this path.
9. The "context awareness" is **emergent, not explicit**: JUCE/VST3 has no API
   to know whether the plugin sits on a track, bus, or master — the host
   simply routes audio in. Whatever the host sends us IS the reference.
   Document this clearly; expose a "Capture Source" label that just reads
   "Host input (current routing)".
10. UI controls: `Arm` toggle (sets `capturing`), `Clear` button (resets
    FIFO via a message-thread→audio-thread atomic command), live VU meter
    fed by an atomic peak value updated in `processBlock`.
11. Snapshot API: `std::shared_ptr<juce::AudioBuffer<float>>
    snapshotReference()` called from the message thread when user clicks
    "Generate" — drains FIFO into a heap buffer that is then handed (by
    `shared_ptr`) to the inference thread. Producer keeps writing into a
    fresh region; snapshot is immutable.

**Verification:** Unit test feeds known sine into FIFO from one thread,
drains from another, asserts sample-accurate round-trip with no data race
(run under `-fsanitize=thread` on a WSL Linux build configuration).
Manual test in Reaper: place on track / bus / master, arm, observe meter.

## Phase 3 — Background Inference Thread

12. `AceStepEngine` lives in `Source/Engine/`. Owns:
    - `acestep::ModelStore` (from `src/model-store.h`) — loads the 4 GGUFs
      (LM, text-enc, DiT, VAE) discovered under user's `Models/` dir
      (default: `%LOCALAPPDATA%\AceStepPlugin\models\`).
    - `acestep::PipelineLm` and `acestep::PipelineSynth` — mirroring
      `tools/ace-synth.cpp::main()`. Use that file as the canonical
      reference for call ordering and request shape.
    - GGML backend init: `ggml_backend_load_all_from_path()` pointed at
      the VST3 bundle's `Contents/x86_64-win/` dir (Phase 1 step 5).
13. Public engine API (only surface visible to rest of plugin — keeps
    upstream churn contained behind `Source/Engine/AceStepCApi.h`):
    - `bool isReady() const noexcept`
    - `juce::Result loadModels(const juce::File& modelsDir)`
    - `std::future<GenerationResult> generate(GenerationRequest)`
    - `void cancelAll()`
14. `juce::ThreadPool` with **1 worker** (GGML pipeline is single-context;
    serializing avoids contention and OOM). Cancellation polled inside a
    progress callback passed to `PipelineSynth` (DiT denoising is the
    long phase — 50 steps SFT, 8 steps turbo).
15. `GenerationRequest` mirrors upstream `acestep::Request` (see
    `src/request.h` + `docs/ARCHITECTURE.md`). v1 UI-exposed fields:
    prompt (caption), lyrics, duration_seconds, seed, cfg_scale, lm_seed,
    scheduler (turbo|sft), reference_audio (optional `shared_ptr<
    AudioBuffer>` from Phase 2 — fed via the **cover/repaint** task type,
    upstream's audio-conditioned pathway).
16. Resampling: capture is at host SR (typically 44.1/48 k). ACE expects
    44.1 kHz. Resample on the worker thread (`juce::ResamplingAudioSource`
    or r8brain-free), never on the audio thread.
17. Progress: worker invokes `std::function<void(int step, int total,
    const char* phase)>` callback that pushes a `ProgressMsg` into a
    lock-free FIFO; `juce::Timer` (30 Hz) on message thread drains and
    updates progress bar + phase label ("LM planning…",
    "DiT step 12/50…", "VAE decode…").
18. Output: upstream `wav.h` writes 24-bit PCM to
    `%TEMP%\AceStepPlugin\<uuid>\out.wav`. Notify UI via
    `juce::MessageManager::callAsync`.
19. Lifecycle: `AceStepEngine` lives on the `AudioProcessor` (survives
    editor close/reopen). On destruction: `cancelAll()`, join with hard
    timeout, force-terminate if GGML hangs.
20. Memory policy: ~7.7 GB resident with all models. Default behavior
    matches upstream `--keep-loaded=false` — unload after each generation.
    Expose "Keep models loaded" toggle for power users (≥32 GB RAM).

**Verification:** Mock `AceStepEngine` returns 5 s sine after 2 s sleep:
UI stays responsive, processBlock continues with zero xruns under Reaper's
performance meter, cancellation aborts cleanly. Real-engine smoke test:
load turbo DiT (8 steps), generate 30 s "ambient piano" clip, confirm
WAV plays back correctly.

## Phase 4 — Drag-and-Drop Export UI

18. `AceStepEditor` inherits `juce::AudioProcessorEditor` and is itself a
    `juce::DragAndDropContainer`. A child `GeneratedAssetTile` initiates
    an **external file drag** via `performExternalDragDropOfFiles({path},
    /*canMove=*/false)` from its `mouseDrag` handler.
19. Tile shows: waveform thumbnail (`juce::AudioThumbnail`), filename,
    duration, transport (play/stop using `juce::AudioTransportSource`
    routed through a dedicated preview output, NOT the host's audio path).
20. Lifecycle: temp WAVs live until plugin instance is destroyed, then
    cleaned up. Add an explicit "Save As…" menu item using
    `juce::FileChooser` for users who want to keep them outside `%TEMP%`.
21. Multi-asset history: keep last N=8 generations as tiles in a scrollable
    `juce::Viewport`. Each independently draggable.
22. First-run UX: if `loadModels()` returns "no models found", editor shows
    a setup panel with a **"Download models (~7.7 GB)" button**. The
    downloader uses `juce::URL` against `huggingface.co/Serveurperso/
    ACE-Step-1.5-GGUF/resolve/main/<file>` with HTTP `Range:` resume,
    SHA-256 verification per file (manifest shipped inside plugin),
    pause/resume controls, and per-file progress bars. Files land in
    `%LOCALAPPDATA%\AceStepPlugin\models\`. If the user already placed
    GGUFs there manually, the downloader is skipped (auto-detection on
    plugin load).

**Verification:** Drag tile from plugin window → audio item appears at
playhead. Test in **Reaper, FL Studio, Cubase, Studio One, Ableton Live,
Bitwig** (broad VST3 host coverage per user's ask).

---

## Cross-Cutting Concerns

- **Threading model summary:** audio thread (host) → ring buffer → message
  thread (snapshot trigger) → thread pool worker (GGML inference) →
  message thread (UI update). Zero locks on the audio path.
- **Real-time safety:** no `new`/`delete`, no `std::string`, no `juce::File`
  I/O, no logging, no `std::mutex` inside `processBlock`. Enforce via
  code review checklist + `juce::ScopedNoDenormals` + tracy/`rtsan` profile
  build.
- **Model loading:** lazy-load GGUF on first generation request (not in
  constructor) so plugin scan in DAWs stays fast (<100 ms).
- **Error surfaces:** missing model file, OOM during generation, GGML
  assertion failures — all caught in worker, surfaced as red banner in UI.
- **Out of scope for v1:** MIDI export, stem separation, AAX/AU/Standalone,
  macOS/Linux builds, MPS/Metal backend, real-time generation (we are
  offline-trigger only), preset browser, MIDI CC automation of params.

## Relevant References (this repo, for behavior parity)

- `acestep/inference.py` — generation parameter schema to mirror in
  `GenerationRequest` (CFG scale, scheduler, seed, duration semantics)
- `acestep/handler.py` — `AceStepHandler` orchestration shape; the C++
  `AceStepEngine` should mirror its public surface
- `acestep/audio_utils.py` — reference audio preprocessing (resampling,
  loudness normalization) the C++ side must replicate bit-equivalently
- `acestep/gpu_config.py` — informs GGML backend selection logic on Windows
- `acestep/api_server.py` — fallback embedded-server option if acestep.cpp
  proves unviable mid-project

## Confirmed Decisions (no open blockers — ready to execute Step 1)

1. ✅ **Mode A** (in-process static link to `acestep-core`) is v1 default;
   Mode B (sidecar `ace-server.exe`) preserved as
   `-DACESTEP_PLUGIN_MODE=server` build option.
2. ✅ **Upstream PR authorized** — promote `acestep-core` include dirs to
   `PUBLIC` and add `src/acestep_capi.h` `extern "C"` shim. Carry the
   same change as `patches/0001-public-headers.patch` until merged.
3. ✅ **Universal bundle** — CPU + Vulkan + CUDA backends as DL modules
   (`GGML_BACKEND_DL=ON`), runtime selection. Single ~400 MB installer.
4. ✅ **In-plugin model downloader** with resume + SHA-256 verify.
   Manual GGUF placement remains supported as an escape hatch.

## Deliverables for Step 1 (next message)

When you say "go", I will produce, as concrete files for the new
`ACE-Step-Plugin/` sibling repo:

- `CMakeLists.txt` (root) — project, options, JUCE/acestep wiring
- `cmake/AcestepIntegration.cmake` — submodule + patch + include promotion
- `cmake/BundleBackends.cmake` — copy `ggml-*.dll` into VST3 bundle
- `cmake/CompilerWarnings.cmake` — MSVC `/W4` baseline
- `patches/0001-public-headers.patch` — upstream diff applied at configure
- `.gitmodules` — pinned JUCE + acestep.cpp submodules
- `.gitignore`
- `.vscode/settings.json`, `tasks.json`, `launch.json`
- `Source/PluginProcessor.{h,cpp}` — minimal stub passing audio through
- `Source/PluginEditor.{h,cpp}` — stub editor
- `BUILD.md` — build prereqs (CUDA Toolkit, Vulkan SDK, MSVC 2022)
