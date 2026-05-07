# ACE-Step VST3 Build

This repository builds the Windows VST3 wrapper for `acestep.cpp`.

## Prerequisites

- **Windows 11 x64**
- **Visual Studio 2022** — Desktop development with C++ workload
- **CMake 3.24+** — `cmake --version` to verify
- **CUDA Toolkit 13.2.1** (or latest) — required only for the real-backend build
  - Download: https://developer.nvidia.com/cuda-downloads (Windows x64 local installer)
  - Verify: `nvcc --version` (expect `release 13.2`)
- **Vulkan SDK** — required only for the Vulkan GGML backend build
  - Download: https://vulkan.lunarg.com/sdk/home#windows

> **Stub host-load validation does not require CUDA Toolkit or Vulkan SDK.**
> Use `ACE-Step-Plugin\scripts\build-stub-vst3.ps1` to build the stub plugin on
> any Windows 11 machine with VS 2022 and CMake.

End users do not need CUDA Toolkit or Vulkan SDK. Those SDKs are build-time
requirements only; the built VST3 bundle ships the GGML backend DLLs beside the
plugin binary.

### Real Backend Validation Status

**Current build baseline:** commit `9ca3ce1a518c`, validated 2026-05-07 on
Windows 11 Pro Insider Preview 10.0.26300 x64.

| Check | Result |
|---|---|
| Bundle DLL present (`ACE-Step.vst3`) | PASS — 13,638,656 bytes |
| `ggml.dll` present | PASS — 360,960 bytes |
| `ggml-base.dll` present | PASS — 1,067,520 bytes |
| `ggml-cpu.dll` present | PASS — 1,393,152 bytes |
| `ggml-cuda.dll` present | PASS — 163,799,040 bytes |
| `ggml-vulkan.dll` present | PASS — 75,120,640 bytes |
| No direct CUDA imports from plugin binary | PASS — only `ggml.dll` from the above list |

`dumpbin /dependents ACE-Step.vst3` shows `ggml.dll` and standard Windows
system DLLs only. No `cudart64_*.dll`, `nvcuda.dll`, or `cublas64_*.dll`
appear as direct plugin imports.

Prior real-backend validation was run on Windows 11 with Visual Studio 2022
Professional, CUDA Toolkit 13.2.1, and Vulkan SDK 1.4.341.1.

| Tool | Result |
|---|---|
| `cmake` | Found at `C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe` |
| `dumpbin` | Found through the Visual Studio developer environment. |
| `nvcc` | `release 13.2, V13.2.78` |
| `glslc` | Found at `C:\VulkanSDK\1.4.341.1\Bin\glslc.exe` |
| `ninja` | `1.12.1` from Visual Studio 2022 |

**dumpbin Availability:**

`dumpbin.exe` is installed with Visual Studio 2022 Professional (version 17.14.18)
but not added to `PATH`. It can be used via:

- **Explicit path:**
  ```
  "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe"
  ```

- **Developer environment:** Activate the Visual Studio developer environment first:
  ```powershell
  cmd /c "call `"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat`" x64 && dumpbin /?"
  ```

**Bundle Evidence:**

The real VST3 bundle was built in `C:\b\ace-ninja` with
`-DACESTEP_ENABLE_ACESTEP_CPP=ON`, `-DACESTEP_BUILD_REAL_SMOKE_TEST=ON`, and
`-DCMAKE_CUDA_ARCHITECTURES=89-real` for local RTX 4090 validation. The bundle
directory `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3\Contents\x86_64-win`
contains:

- `ACE-Step.vst3`
- `ggml.dll`
- `ggml-base.dll`
- `ggml-cpu.dll`
- `ggml-cuda.dll`
- `ggml-vulkan.dll`

`scripts\validate-bundle.ps1 -BuildDir C:\b\ace-ninja -Config RelWithDebInfo`
passed and confirmed no direct CUDA imports from the plugin binary. Raw
`dumpbin /dependents` evidence showed:

- `ACE-Step.vst3` directly imports `ggml.dll`, but no CUDA runtime DLLs.
- `ggml.dll` imports `ggml-base.dll`.
- `ggml-cpu.dll` imports `ggml-base.dll`.
- `ggml-cuda.dll` imports `ggml-base.dll` and `cublas64_13.dll`.
- `ggml-vulkan.dll` imports `ggml-base.dll` and `vulkan-1.dll`.

## Vendored External Sources

JUCE and `acestep.cpp` are currently checked in under `External/` as vendored
source. No `git submodule update` step is required.

The vendored source pins JUCE to tag `8.0.10` and `acestep.cpp` to commit
`6e0237bb4a2c94479a8c636e1116e1e3c30c9f45`.

## Stub VST3 Build (Task 2.7 prep)

Builds the stub plugin only — no CUDA or Vulkan required.

```powershell
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
```

This script configures, builds, and verifies that the stub VST3 bundle exists.
Useful for host-load testing and CI on machines without GPU SDKs.

## Real Backend Build (Task 3.7 — requires CUDA Toolkit 13.2.1 or later)

Verify CUDA before building:

```powershell
nvcc --version
# Expected: release 13.2 (or newer)
```

Configure and build:

```powershell
cmake -S ACE-Step-Plugin -B ACE-Step-Plugin\build-real -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=ON -DACESTEP_BUILD_TESTS=OFF -DACESTEP_PLUGIN_MODE=static
cmake --build ACE-Step-Plugin\build-real --config RelWithDebInfo --target AceStepPlugin_VST3 --parallel
```

Use `-DACESTEP_PLUGIN_MODE=server` to build the sidecar fallback path. The v1
default is the in-process static-link path.

### Validate Real Backend Bundle

```powershell
.\ACE-Step-Plugin\scripts\validate-bundle.ps1 -BuildDir ACE-Step-Plugin\build-real
```

Expected output:

```text
[PASS] Bundle structure is correct and no direct CUDA imports found.
```

## Expected Build Output

The VST3 bundle should contain:

- `ACE-Step.vst3/Contents/x86_64-win/ACE-Step.vst3`
- `ACE-Step.vst3/Contents/x86_64-win/ggml.dll` plus `ggml-*.dll`
  for real-backend builds
- `ace-server.exe` only when `ACESTEP_PLUGIN_MODE=server`

## Model Manifest

The v1 turbo profile expects these model files under
`%LOCALAPPDATA%\AceStepPlugin\models\`:

| Filename | Expected size |
|---|---:|
| `acestep-5Hz-lm-4B-Q5_K_M.gguf` | 3,025,965,984 bytes |
| `acestep-v15-turbo-Q5_K_M.gguf` | 1,700,140,224 bytes |
| `Qwen3-Embedding-0.6B-Q8_0.gguf` | 784,144,960 bytes |
| `vae-BF16.gguf` | 337,420,928 bytes |

The plugin validates file sizes during discovery and SHA-256 during download.
The source of truth for URLs, hashes, and sizes is
`Source\Models\ModelDiscovery.cpp`.

## Feature Capability Notes

- **MIDI export:** unavailable by default. The current backend boundary exposes
  WAV output and FSQ audio tokens, not reliable note/onset events. The plugin
  includes a standards-compliant MIDI writer for future explicit note data.
- **Stem export:** unavailable by default unless a generated asset contains
  successful stem WAV metadata. The vendored backend has internal stem task
  modes, but the plugin-facing C API does not yet expose stem requests.
- **Preset storage:** presets are stored as one JSON file per preset under
  `%APPDATA%\ACE-Step\Presets` by default, with schema version `1`.
- **Host differences:** scan UX, drag/drop insertion location, media import
  prompts, and project media-copy behavior are host-owned. Save As is the
  plugin-owned fallback when drag/drop differs.

## Smoke Verification

Real-engine smoke validation was run from `C:\b\ace-ninja` after downloading
the four GGUF files into `%LOCALAPPDATA%\AceStepPlugin\models`. The smoke test
launched:

```text
C:\b\ace-ninja\ace-synth.exe --models "%LOCALAPPDATA%\AceStepPlugin\models" --request "%TEMP%\acestep_smoke_request.json"
```

Result: PASS. `AceStepRealSmokeTest.exe` produced
`%TEMP%\acestep_smoke_request0.wav` with size `967,724` bytes.

Host smoke validation:

1. Open JUCE `AudioPluginHost.exe`.
2. Scan the built VST3 bundle.
3. Insert `ACE-Step` on a stereo audio track.
4. Confirm the editor opens and audio passes through unchanged.
5. Run `dumpbin /dependents` on the plugin DLL and confirm no unexpected
   runtime DLLs are required by the host process.
