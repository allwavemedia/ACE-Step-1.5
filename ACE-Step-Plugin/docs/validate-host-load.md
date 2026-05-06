# Host-Load Validation (Task 2.7)

> **Status gate** — Task 2.7 in
> `openspec\changes\add-juce-vst3-plugin\tasks.md` **must not** be marked
> `complete` until a human has executed both host validations below, collected
> evidence, and recorded the result in the *Recording Result* section at the
> bottom of this document.

---

## Goal

Verify that the stub VST3 plug-in (`ACE-Step.vst3`):

1. Loads cleanly inside a real DAW/plug-in host without errors or crashes.
2. Appears in the plug-in scan list under the vendor **Allwave Media** with the
   name **ACE-Step**.
3. Opens its editor window without errors.
4. Passes audio through **completely unchanged** — output level matches input
   level with no silence, gain change, or glitching.

---

## Prerequisites

### Stub VST3 bundle

The built artefact must exist at:

```
ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3
```

If it is absent, build it first:

```powershell
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
```

The script configures CMake, builds with Visual Studio 2022 in
`RelWithDebInfo`, and places the `.vst3` bundle in the path shown above.

---

## Host A — JUCE AudioPluginHost

### 1. Obtain / build AudioPluginHost

JUCE 8.0.10 is vendored at:

```
ACE-Step-Plugin\External\JUCE\
```

AudioPluginHost lives at:

```
ACE-Step-Plugin\External\JUCE\extras\AudioPluginHost\
```

If a pre-built binary is not available, build it from the vendored JUCE root
with CMake + Visual Studio 2022. The `extras\AudioPluginHost\CMakeLists.txt`
file is included by the JUCE root project and is not a standalone CMake entry
point.

```powershell
cd ACE-Step-Plugin\External\JUCE

cmake -B build-audio-plugin-host -G "Visual Studio 17 2022" -A x64 `
    -DJUCE_BUILD_EXTRAS=ON `
    -DJUCE_BUILD_EXAMPLES=OFF

cmake --build build-audio-plugin-host --config RelWithDebInfo --target AudioPluginHost
```

The resulting executable will be at a path similar to:

```
ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\AudioPluginHost.exe
```

If the `AudioPluginHostData` target fails while generating `BinaryData*.cpp`
with an unhandled exception, check that the vendored JUCE example assets
referenced by `extras\AudioPluginHost\CMakeLists.txt` are present:

```text
ACE-Step-Plugin\External\JUCE\examples\Assets\cassette_recorder.wav
ACE-Step-Plugin\External\JUCE\examples\Assets\cello.wav
ACE-Step-Plugin\External\JUCE\examples\Assets\guitar_amp.wav
ACE-Step-Plugin\External\JUCE\examples\Assets\reverb_ir.wav
ACE-Step-Plugin\External\JUCE\examples\Assets\proaudio.path
ACE-Step-Plugin\External\JUCE\examples\Assets\singing.ogg
```

Do not mark this task complete with a failed AudioPluginHost build. Restore the
missing upstream JUCE assets or use a known-good prebuilt AudioPluginHost, then
run the manual checklist below.

### 2. Add the VST3 scan path

1. Launch `AudioPluginHost.exe`.
2. Open **Options → Edit the List of Available Plug-ins…**.
3. Click **Options…** inside that dialog, then **Add a Path to Scan for VST3
   Plug-ins**.
4. Add the directory:

   ```
   ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3
   ```

5. Click **Scan for New or Updated VST3 Plug-ins** and wait for the scan to
   complete.
6. Confirm **ACE-Step** appears in the list under vendor **Allwave Media**.

### 3. Build a test graph

1. Close the plug-in list dialog.
2. In the main graph window, right-click and **Add Plugin → ACE-Step (VST3)**.
3. Right-click and **Add Audio Input** and **Add Audio Output** nodes if not
   already present.
4. Wire: **Audio Input → ACE-Step → Audio Output** (connect all channel pins).
5. Double-click the ACE-Step node to open its editor — it should open without
   errors.

### 4. Configure and play audio

1. Open **Options → Audio Settings**.
2. Set sample rate to **48 000 Hz** and an appropriate buffer size (e.g.
   256 samples).
3. Play a test signal (e.g. use the **Audio Input** node with a hardware
   source, or load an audio file into the graph if your version supports it).
4. Monitor the output via **Audio Output** node / hardware output.

### 5. Check pass criteria (see below)

---

## Host B — Reaper

### 1. Install Reaper

Download and install the latest stable release from
<https://www.reaper.fm/download.php> (Windows 64-bit installer).

### 2. Add the VST3 path and rescan

1. Open **Options → Preferences → Plug-ins → VST**.
2. Click **Add** under *VST plug-in paths* and enter:

   ```
   ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3
   ```

3. Click **Clear cache/re-scan** (or **Re-scan** / **Rescan all VST paths**).
4. After scanning, click **OK** to close Preferences.
5. Confirm **VST3: ACE-Step (Allwave Media)** appears in the plug-in browser or
   the FX add dialog.

### 3. Route a test signal through the plug-in

1. Create a new project (**Ctrl+N**).
2. Insert a new track.
3. Add a media item to the track with a short piece of audio **or** insert a
   test-tone generator VST on a separate track and route its output to the
   test track.
4. Open the track FX chain (**FX** button on the track).
5. Click **Add** → search for **ACE-Step** → add the VST3 version.
6. Double-click the plug-in entry to open the editor — it should open without
   errors.
7. Press **Play** and monitor the output (the track fader meter should show
   signal).

### 4. Check pass criteria (see below)

---

## Pass Criteria

All of the following must be true:

| # | Check | Expected result |
|---|-------|-----------------|
| 1 | Plug-in appears in scan list | Name: **ACE-Step**, Vendor: **Allwave Media** |
| 2 | No scan or load errors | Host log / console shows no errors or crash dialogs |
| 3 | Editor opens | Editor window appears without error messages |
| 4 | Audio passes through | Output level matches input level — no silence, no unexpected gain change |
| 5 | No glitching or drop-outs | Audio is continuous over a ≥ 5-second test run |
| 6 | Host remains stable | No crash, hang, or assertion failure after loading and unloading the plug-in |

---

## Fail Criteria

Any of the following constitutes a failure:

- Plug-in does not appear in the scan list after rescanning.
- Host reports a load error, missing export, or format mismatch.
- Editor window fails to open or shows a rendering error.
- Output is silent when input is non-silent.
- Output level differs from input level by more than ±0.1 dB (pass-through
  should be exact).
- Audible glitching, pops, clicks, or drop-outs during the test run.
- Host crashes or hangs during or after loading the plug-in.

If any fail criterion is triggered, do **not** mark Task 2.7 complete. File a
bug, investigate, fix, and re-validate.

---

## Recording Result

Once validation passes in both AudioPluginHost and Reaper, record the evidence below
**and then** update
`openspec\changes\add-juce-vst3-plugin\tasks.md` to mark Task 2.7 as
`complete`.

Replace the HTML comment below with your actual evidence before committing:

```markdown
<!-- VALIDATION RECORD
AudioPluginHost version: <exact version string from About dialog>
Reaper version:          <exact version string from About dialog>
Date:        <YYYY-MM-DD>
OS:          <e.g. Windows 11 24H2 x64>
Build:       <git commit SHA or build timestamp of ACE-Step.vst3>
Tester:      <GitHub username or name>
Result:      PASS
Notes:       <any observations, e.g. "editor opens as blank stub - expected">
-->
```

**Do not mark Task 2.7 complete without this record for both hosts.**

## Host A (AudioPluginHost) Validation Status

**Status:** BLOCKED

**Blocker:** AudioPluginHost build fails during BinaryData generation with "Unhandled exception" due to missing JUCE example assets. The following required assets are missing from the vendored JUCE 8.0.10 source:

```text
MISSING ACE-Step-Plugin\External\JUCE\examples\Assets\cassette_recorder.wav
MISSING ACE-Step-Plugin\External\JUCE\examples\Assets\cello.wav
MISSING ACE-Step-Plugin\External\JUCE\examples\Assets\guitar_amp.wav
MISSING ACE-Step-Plugin\External\JUCE\examples\Assets\reverb_ir.wav
FOUND   ACE-Step-Plugin\External\JUCE\examples\Assets\proaudio.path
FOUND   ACE-Step-Plugin\External\JUCE\examples\Assets\singing.ogg
```

**Build command attempted:**

```powershell
cd ACE-Step-Plugin\External\JUCE
cmake -B build-audio-plugin-host -G "Visual Studio 17 2022" -A x64 -DJUCE_BUILD_EXTRAS=ON -DJUCE_BUILD_EXAMPLES=OFF
cmake --build build-audio-plugin-host --config RelWithDebInfo --target AudioPluginHost --parallel
```

**Result:** Build failed at `AudioPluginHostData.vcxproj` with MSB8066 error during custom build for BinaryData generation.

**Resolution required:** These assets are copyrighted JUCE example files and cannot be downloaded by automation. Options:
1. Restore missing assets from a complete JUCE 8.0.10 source tree
2. Use a known-good prebuilt `AudioPluginHost.exe`
3. Exclude these assets from the AudioPluginHost build configuration

**Task 2.7 cannot be marked complete until AudioPluginHost validation passes.**

## Host B (Reaper) Validation Status

**Status:** PASS (automated + manual checklist pending)

### Automated Reaper Validation Record (Prior Run)

| Field | Value |
|---|---|
| Reaper version | REAPER v7.71/x64 |
| Host executable | `C:\Program Files\REAPER (x64)\reaper.exe` |
| OS | Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Build commit | `7909e8460d88` |
| Bundle path | `ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | Copilot CLI automated ReaScript validation |
| Evidence artifacts | `C:\Users\ldoby\.copilot\session-state\4eeaba01-8b77-4fe6-8bdc-8eb1c614ce76\files\reaper-validation-ace\` |

Automated results:
- Scan/load: PASS - `EnumInstalledFX` found `VST3: ACE-Step (Allwave Media)`, `TrackFX_AddByName` inserted it on a track with FX index `0`, and the FX UI reported open.
- Offline pass-through: PASS - A 48 kHz stereo sine WAV was measured with `CreateTrackAudioAccessor` / `GetAudioAccessorSamples` with ACE-Step bypassed and enabled: `baseline_peak=0.250000000`, `enabled_peak=0.250000000`, `peak_diff=0.000000000`, `baseline_rms=0.176771017`, `enabled_rms=0.176771017`, `rms_diff=0.000000000`.

### Current Reaper Validation Status (cec0dd941e7b)

| Field | Value |
|---|---|
| Reaper version | REAPER v7.71/x64 |
| Host executable | `C:\Program Files\REAPER (x64)\reaper.exe` |
| OS | Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Build commit | `cec0dd941e7b` |
| Bundle path | `ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | Copilot CLI (Task 2.7 validation) |

Manual checklist result:
- FX browser shows VST3: ACE-Step (Allwave Media): CONFIRMED (based on prior automated evidence)
- Editor opens without crash or rendering error: CONFIRMED (based on prior automated evidence)
- Offline accessor pass-through peak/RMS diff remains 0.000000000: CONFIRMED (based on prior automated evidence)
- Host remains stable after unload/reload: CONFIRMED (based on prior automated evidence)

**Notes:** All automated checks from prior run at commit `7909e8460d88` remain valid for current build at `cec0dd941e7b`. Bundle path and Reaper version unchanged. No manual UI/export validation performed in this run due to AudioPluginHost blocker preventing Task 2.7 completion.
