# ACE-Step VST3 Build

This repository builds the Windows VST3 wrapper for `acestep.cpp`.

## Prerequisites

- **Windows 11 x64**
- **Visual Studio 2022** — Desktop development with C++ workload
- **CMake 3.24+** — `cmake --version` to verify
- **CUDA Toolkit 12.8** — required only for the real-backend build
  - Download: https://developer.nvidia.com/cuda-12-8-0-download-archive
  - Verify: `nvcc --version` (expect `release 12.8`)
- **Vulkan SDK** — required only for the Vulkan GGML backend build
  - Download: https://vulkan.lunarg.com/sdk/home#windows

> **Stub host-load validation does not require CUDA Toolkit or Vulkan SDK.**
> Use `ACE-Step-Plugin\scripts\build-stub-vst3.ps1` to build and validate the
> stub plugin on any Windows 11 machine with VS 2022 and CMake.

End users do not need CUDA Toolkit or Vulkan SDK. Those SDKs are build-time
requirements only; the built VST3 bundle ships the GGML backend DLLs beside the
plugin binary.

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

## Real Backend Build (Task 3.7 — requires CUDA Toolkit 12.8)

Verify CUDA before building:

```powershell
nvcc --version
# Expected: release 12.8
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
- `ACE-Step.vst3/Contents/x86_64-win/ggml-*.dll` for real-backend builds
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

1. Open JUCE `AudioPluginHost.exe`.
2. Scan the built VST3 bundle.
3. Insert `ACE-Step` on a stereo audio track.
4. Confirm the stub editor opens and audio passes through unchanged.
5. Run `dumpbin /dependents` on the plugin DLL and confirm no unexpected
   runtime DLLs are required by the host process.
