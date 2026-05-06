# Host Compatibility Matrix

## Real Backend Bundle Validation Status

**Tasks 3.7 and 12.3 Prerequisites:**

| Tool | Status | Location |
|------|--------|----------|
| `cmake` | ✅ FOUND | Visual Studio 2022 |
| `dumpbin` | ✅ FOUND | Visual Studio 2022 MSVC Tools |
| `nvcc` | ❌ MISSING | Requires CUDA Toolkit 12.8 |
| `glslc` | ❌ MISSING | Requires Vulkan SDK |

**Blocker:** Real backend build and dependency validation cannot proceed without CUDA Toolkit 12.8
and Vulkan SDK installation. Tasks 3.7 and 12.3 remain incomplete until:

1. CUDA Toolkit 12.8 is installed (provides `nvcc`)
2. Vulkan SDK is installed (provides `glslc`)
3. Real backend build succeeds with `-DACESTEP_ENABLE_ACESTEP_CPP=ON`
4. Bundle contains CPU, CUDA, and Vulkan GGML backend DLL siblings
5. `dumpbin /dependents` output is captured and validated

## Expected Bundle Contents (Pending Validation)

When real backend build succeeds, the VST3 bundle should contain:

- `ACE-Step.vst3/Contents/x86_64-win/ACE-Step.vst3` — Plugin DLL
- `ACE-Step.vst3/Contents/x86_64-win/ggml-*.dll` — CPU backend DLLs
- `ACE-Step.vst3/Contents/x86_64-win/ggml-*-cuda.dll` — CUDA backend DLLs
- `ACE-Step.vst3/Contents/x86_64-win/ggml-*-vulkan.dll` — Vulkan backend DLLs

Exact DLL filenames will be documented after successful build.

## Plugin Dependencies (Pending dumpbin Validation)

`dumpbin /dependents` output will be recorded here after real backend build succeeds.

## DAW Host Compatibility

To be tested and documented after real backend validation completes.
