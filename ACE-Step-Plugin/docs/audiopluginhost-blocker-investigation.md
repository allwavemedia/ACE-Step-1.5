# AudioPluginHost Validation Blocker Investigation

**Investigation Date:** 2025-06-12  
**Worktree:** `agents-add-juce-vst3-plugin-validation`  
**Branch:** `feat/add-juce-vst3-plugin-8-3-onwards`  
**Task:** OpenSpec 2.7 - Validate with AudioPluginHost  
**Todo ID:** `unblock-audiopluginhost-validation`

---

## Executive Summary

AudioPluginHost validation for Task 2.7 is blocked by **missing JUCE example assets** required for BinaryData generation during the AudioPluginHost build. These assets are copyrighted JUCE example files and cannot be downloaded automatically.

**Root cause:**
- JUCE AudioPluginHost includes demo plugins (DSPModulePluginDemo, SamplerPluginDemo) that use embedded binary resources
- The CMake build requires 4 WAV files that are missing from the vendored JUCE 8.0.10 source
- Build fails with MSB8066 error during BinaryData generation

**Impact:**
- AudioPluginHost cannot be built from vendored JUCE source
- Task 2.7 validation checklist cannot be completed
- OpenSpec change cannot progress without AudioPluginHost validation evidence

---

## Missing Assets Analysis

### Required Files

The following assets are referenced in `ACE-Step-Plugin\External\JUCE\extras\AudioPluginHost\CMakeLists.txt` line 50-56:

| File | Status | Purpose | Used By |
|------|--------|---------|---------|
| `cassette_recorder.wav` | ❌ MISSING | Demo audio sample | Unknown (not found in source grep) |
| `cello.wav` | ❌ MISSING | Sampler demo sample | `SamplerPluginDemo.h` line 2115 |
| `guitar_amp.wav` | ❌ MISSING | Convolution IR | `DSPModulePluginDemo.h` line 1105 |
| `reverb_ir.wav` | ❌ MISSING | Convolution IR | `DSPModulePluginDemo.h` line 1106 |
| `proaudio.path` | ✅ FOUND | Pro Audio path metadata | InternalPlugins BinaryData lookup |
| `singing.ogg` | ✅ FOUND | Demo audio sample | InternalPlugins BinaryData lookup |

### BinaryData Usage Flow

1. **Build-time:** `juce_add_binary_data` CMake function (line 50-56 in `CMakeLists.txt`) embeds these files into `BinaryData.cpp/.h`
2. **Compile-time:** `InternalPlugins.cpp` (line 48-56) implements `createAssetInputStream()` that looks up resources by filename from `BinaryData::namedResourceList`
3. **Runtime:** Demo plugins call `createAssetInputStream("cello.wav")` or `loadImpulseResponse(cabinet, "guitar_amp.wav")` to load embedded assets

### Why Assets Are Required

The AudioPluginHost source code at `Source/Plugins/InternalPlugins.cpp` includes demo plugins from `../../../../examples/Plugins/`:

- **DSPModulePluginDemo.h** - ConvolutionProcessor constructor (lines 1105-1106) loads `guitar_amp.wav` and `reverb_ir.wav` as impulse responses
- **SamplerPluginDemo.h** - SamplerProcessor constructor (line 2115) loads `cello.wav` for the sampler

These demo plugins are **mandatory** in AudioPluginHost's internal plugin list (lines 61-70 of InternalPlugins.cpp reference them). They cannot be excluded without modifying the source code.

---

## Attempted Build Evidence

### Build Command

```powershell
cd ACE-Step-Plugin\External\JUCE
cmake -B build-audio-plugin-host -G "Visual Studio 17 2022" -A x64 `
    -DJUCE_BUILD_EXTRAS=ON `
    -DJUCE_BUILD_EXAMPLES=OFF
cmake --build build-audio-plugin-host --config RelWithDebInfo --target AudioPluginHost
```

### Build Result

**Failed** at `AudioPluginHostData.vcxproj` with MSB8066 error during custom build step for BinaryData generation. The Projucer BinaryBuilder tool throws an unhandled exception when it cannot find the referenced WAV files.

### Source Investigation

- **No local AudioPluginHost.exe found** in worktree or build directories
- **No BinaryData workaround** found in vendored JUCE configuration
- **CMake has no option** to skip BinaryData or demo plugins

---

## Potential Solutions

### Option 1: Restore Missing JUCE Assets (Recommended for Real Validation)

**Action:** Obtain the missing WAV files from a complete JUCE 8.0.10 source tree.

**Source options:**
1. Download official JUCE 8.0.10 from GitHub: https://github.com/juce-framework/JUCE/releases/tag/8.0.10
2. Extract only the missing assets from `examples/Assets/` directory
3. Copy them into `ACE-Step-Plugin\External\JUCE\examples\Assets\`

**Pros:**
- ✅ Enables full AudioPluginHost build with all demo plugins
- ✅ No code changes required
- ✅ Validates real AudioPluginHost behavior

**Cons:**
- ❌ Requires manual download of copyrighted JUCE assets
- ❌ Assets not tracked in Git (or requires LFS)
- ❌ Must document asset source and licensing

**Blocker:** These are copyrighted example assets included with JUCE. They cannot be automatically downloaded or redistributed. A developer must manually obtain them from the official JUCE distribution.

---

### Option 2: Use Pre-built AudioPluginHost Binary

**Action:** Obtain a known-good pre-built `AudioPluginHost.exe` from another source.

**Source options:**
1. Build AudioPluginHost on a machine with complete JUCE 8.0.10 source
2. Download pre-built binary from JUCE forum/community (if available)
3. Use AudioPluginHost from a JUCE installation that includes extras

**Pros:**
- ✅ No local build required
- ✅ No need to manage missing assets
- ✅ Can proceed with validation immediately

**Cons:**
- ❌ Binary provenance unclear
- ❌ Version mismatch risk (must match JUCE 8.0.10)
- ❌ Not reproducible from vendored source
- ❌ Does not validate vendored JUCE build capability

---

### Option 3: Patch AudioPluginHost to Exclude Demo Plugins (Complex, Not Recommended)

**Action:** Modify vendored JUCE source to remove demo plugin dependencies.

**Required changes:**
1. Edit `InternalPlugins.cpp` to comment out DSPModulePluginDemo and SamplerPluginDemo includes
2. Remove corresponding entries from `InternalPluginCache::create()` factory list
3. Edit `CMakeLists.txt` to remove BinaryData entries for missing WAV files
4. Create a patch file in `ACE-Step-Plugin\patches\` for these changes

**Pros:**
- ✅ Enables AudioPluginHost build from vendored source
- ✅ No external assets required

**Cons:**
- ❌ Modifies JUCE source code (violates minimal-invasive policy)
- ❌ Creates divergence from upstream JUCE behavior
- ❌ Demo plugins unavailable for testing internal plugin system
- ❌ Maintenance burden for future JUCE upgrades
- ❌ Not representative of standard AudioPluginHost

**Verdict:** This approach violates AGENTS.md minimal-scope policy and introduces unnecessary technical debt. Not recommended.

---

### Option 4: Generate Stub Audio Files (Workaround, Validation Limited)

**Action:** Create minimal-size stub WAV files to satisfy BinaryData build.

**Implementation:**
```python
# Example: Generate silent 1-second stereo WAV files
import wave
import struct

def create_stub_wav(filename, duration_sec=1.0, sample_rate=48000):
    num_samples = int(duration_sec * sample_rate)
    with wave.open(filename, 'w') as wav:
        wav.setnchannels(2)  # stereo
        wav.setsampwidth(2)  # 16-bit
        wav.setframerate(sample_rate)
        wav.writeframes(struct.pack('<h', 0) * num_samples * 2)

create_stub_wav('cassette_recorder.wav')
create_stub_wav('cello.wav')
create_stub_wav('guitar_amp.wav', duration_sec=0.5)  # IR can be shorter
create_stub_wav('reverb_ir.wav', duration_sec=0.5)
```

**Pros:**
- ✅ Enables AudioPluginHost build
- ✅ No copyrighted content
- ✅ Reproducible from script

**Cons:**
- ❌ Demo plugins will not produce useful audio output
- ❌ Cannot validate demo plugin functionality
- ❌ Still represents modified AudioPluginHost behavior
- ⚠️ Acceptable **only** if Task 2.7 validation focuses on ACE-Step VST3 load/pass-through, not demo plugin quality

**Verdict:** Viable workaround for **unblocking Task 2.7** if the validation goal is limited to:
- ACE-Step VST3 scan/load in AudioPluginHost
- ACE-Step editor opens without crash
- ACE-Step audio pass-through validation

This does not enable validation of AudioPluginHost's demo plugins, but those are not Task 2.7 requirements.

---

## Recommended Next Steps

### Immediate (Unblock Task 2.7)

**Recommended: Option 4 (Stub Files) for Minimal Unblocking**

1. **Create stub WAV generation script:**
   - Add `ACE-Step-Plugin\scripts\generate-stub-juce-assets.py`
   - Generate minimal silent WAV files for missing assets
   - Document that these are stub files for build-only

2. **Update documentation:**
   - Add note in `validate-host-load.md` explaining stub asset workaround
   - Clarify that AudioPluginHost validation focuses on ACE-Step VST3 load, not demo plugins

3. **Build AudioPluginHost:**
   ```powershell
   python ACE-Step-Plugin\scripts\generate-stub-juce-assets.py
   cd ACE-Step-Plugin\External\JUCE
   cmake -B build-audio-plugin-host -G "Visual Studio 17 2022" -A x64 -DJUCE_BUILD_EXTRAS=ON -DJUCE_BUILD_EXAMPLES=OFF
   cmake --build build-audio-plugin-host --config RelWithDebInfo --target AudioPluginHost
   ```

4. **Run validation checklist** from `validate-host-load.md`:
   - Add VST3 scan path in AudioPluginHost
   - Load ACE-Step VST3 plugin
   - Verify editor opens
   - Verify audio pass-through
   - Record evidence

5. **Mark Task 2.7 complete** once validation passes

**Effort:** Low (1-2 hours)  
**Risk:** Low (stub files only affect demo plugins, not ACE-Step validation)  
**Blockers:** None

---

### Long-term (Full AudioPluginHost Validation)

**Optional: Option 1 (Real Assets) for Complete Validation**

If future validation requires testing AudioPluginHost demo plugins:

1. **Manual asset restoration:**
   - Developer downloads official JUCE 8.0.10 from GitHub
   - Extracts `examples/Assets/` directory
   - Copies missing WAV files to `ACE-Step-Plugin\External\JUCE\examples\Assets\`

2. **Document asset source:**
   - Add `ACE-Step-Plugin\External\JUCE\examples\Assets\README.md`
   - Reference JUCE 8.0.10 release URL
   - Note assets are copyrighted by Raw Material Software Limited
   - Clarify assets are excluded from repo, must be restored manually

3. **Update BUILD.md:**
   - Add prerequisite step for JUCE asset restoration before AudioPluginHost build

**Effort:** Medium (manual download + documentation)  
**Risk:** Low (restores standard JUCE behavior)  
**Blockers:** Requires access to official JUCE 8.0.10 distribution

---

## Validation Scope for Task 2.7

### What Task 2.7 Requires

From `openspec\changes\add-juce-vst3-plugin\tasks.md` line 16:

> 2.7 Verify the initial VST3 bundle loads in JUCE AudioPluginHost and Reaper with unchanged audio pass-through.

### What Is Actually Being Validated

- ✅ ACE-Step VST3 scan/load in AudioPluginHost
- ✅ ACE-Step editor opens without crash
- ✅ ACE-Step audio pass-through (input → output unchanged)
- ❌ AudioPluginHost demo plugin functionality (NOT required for Task 2.7)

### Conclusion

**Stub audio files (Option 4) are sufficient** to unblock Task 2.7 validation because:
1. Task 2.7 validates **ACE-Step VST3 behavior**, not AudioPluginHost demo plugins
2. Demo plugin audio quality does not affect ACE-Step scan/load/pass-through tests
3. AudioPluginHost GUI and plugin routing work identically with stub or real assets

The missing assets are a **JUCE AudioPluginHost build blocker**, not an **ACE-Step validation blocker**.

---

## Files Changed

None yet (investigation only). Proposed next step: create stub asset generation script.

---

## References

- **Task:** `openspec\changes\add-juce-vst3-plugin\tasks.md` line 16 (Task 2.7)
- **Validation docs:** `ACE-Step-Plugin\docs\validate-host-load.md` lines 233-264 (AudioPluginHost blocker section)
- **JUCE AudioPluginHost:**
  - `ACE-Step-Plugin\External\JUCE\extras\AudioPluginHost\CMakeLists.txt` (lines 50-56: BinaryData sources)
  - `ACE-Step-Plugin\External\JUCE\extras\AudioPluginHost\Source\Plugins\InternalPlugins.cpp` (lines 48-56: BinaryData lookup)
- **Demo plugins:**
  - `ACE-Step-Plugin\External\JUCE\examples\Plugins\DSPModulePluginDemo.h` (lines 1105-1106: guitar_amp.wav, reverb_ir.wav)
  - `ACE-Step-Plugin\External\JUCE\examples\Plugins\SamplerPluginDemo.h` (line 2115: cello.wav)
- **JUCE 8.0.10:** https://github.com/juce-framework/JUCE/releases/tag/8.0.10
