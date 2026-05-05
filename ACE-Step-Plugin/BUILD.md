# ACE-Step VST3 Build

This repository builds the Windows VST3 wrapper for `acestep.cpp`.

## Prerequisites

- Windows 11 x64
- Visual Studio 2022 with the Desktop C++ workload
- CMake 3.24 or newer
- Git with submodule support
- CUDA Toolkit for the CUDA GGML backend
- Vulkan SDK for the Vulkan GGML backend

End users do not need CUDA Toolkit or Vulkan SDK. Those SDKs are build-time
requirements only; the built VST3 bundle ships the GGML backend DLLs beside the
plugin binary.

## First Checkout

```powershell
git submodule update --init --recursive
```

The checked-in submodule gitlinks pin JUCE to tag `8.0.10` and `acestep.cpp`
to commit `6e0237bb4a2c94479a8c636e1116e1e3c30c9f45`.

## Configure and Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DACESTEP_PLUGIN_MODE=static
cmake --build build --config RelWithDebInfo --target AceStepPlugin_VST3 --parallel
```

Use `-DACESTEP_PLUGIN_MODE=server` to build the sidecar fallback path. The v1
default is the in-process static-link path.

## Expected Build Output

The VST3 bundle should contain:

- `AceStepPlugin.vst3/Contents/x86_64-win/AceStepPlugin.vst3`
- `AceStepPlugin.vst3/Contents/x86_64-win/ggml-*.dll`
- `ace-server.exe` only when `ACESTEP_PLUGIN_MODE=server`

## Smoke Verification

1. Open JUCE `AudioPluginHost.exe`.
2. Scan the built VST3 bundle.
3. Insert `ACE-Step` on a stereo audio track.
4. Confirm the stub editor opens and audio passes through unchanged.
5. Run `dumpbin /dependents` on the plugin DLL and confirm no unexpected
   runtime DLLs are required by the host process.
