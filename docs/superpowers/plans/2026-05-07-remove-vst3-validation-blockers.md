# Remove VST3 Validation Blockers Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove or formally resolve the remaining ACE-Step VST3 validation blockers so the OpenSpec tasks can be completed with current-build evidence.

**Architecture:** The blockers are not model-generation defects; they are evidence and environment gates around host validation. The plan keeps plugin behavior unchanged, adds evidence only where validation passes, and requires an explicit scope decision where commercial hosts are unavailable.

**Tech Stack:** Windows 11, CMake, Visual Studio 2022, JUCE AudioPluginHost, Reaper, PowerShell 5.1, ACE-Step VST3 C++ plugin.

---

## Files and Responsibilities

- `openspec\changes\add-juce-vst3-plugin\tasks.md`: source-of-truth checklist for tasks 2.7, 11.3, 12.4, and 12.5.
- `ACE-Step-Plugin\docs\validate-host-load.md`: detailed AudioPluginHost and Reaper validation evidence for task 2.7.
- `ACE-Step-Plugin\docs\host-compatibility-matrix.md`: DAW matrix evidence and host-owned behavior notes for tasks 11.3 and 12.4.
- `ACE-Step-Plugin\docs\audiopluginhost-blocker-investigation.md`: historical root-cause analysis for the missing JUCE example asset build blocker.
- `ACE-Step-Plugin\BUILD.md`: build and bundle validation commands, SDK versions, model manifest, and smoke evidence.
- `ACE-Step-Plugin\scripts\generate-stub-juce-assets.py`: build-only workaround for missing JUCE AudioPluginHost WAV assets.
- `ACE-Step-Plugin\scripts\build-stub-vst3.ps1`: reproducible stub VST3 build path for host-load validation without CUDA or Vulkan.
- `ACE-Step-Plugin\scripts\validate-bundle.ps1`: real-backend bundle validation for required DLLs and direct CUDA import checks.

## Current Blockers

1. `2.7`: AudioPluginHost load/editor has partial current evidence, but AudioPluginHost pass-through remains pending; Reaper current-build rerun is also pending.
2. `11.3`: Reaper has partial automated evidence, but full interactive UI/export checks remain pending; FL Studio, Cubase, Studio One, Ableton Live, and Bitwig are blocked because they are not installed at the checked paths.
3. `12.4`: Reaper drag/drop remains pending; non-Reaper host drag/drop is blocked by the same host availability issue.
4. `12.5`: Error-state surfacing is pending for the current build, including destructive/manual cases such as checksum mismatch, backend-load failure, cancellation, and host compatibility errors.
5. Historical AudioPluginHost build blocker: missing JUCE example WAV assets. This is already unblocked for ACE-Step validation by `ACE-Step-Plugin\scripts\generate-stub-juce-assets.py`; do not treat AudioPluginHost demo-plugin audio quality as a blocker for ACE-Step task 2.7.

---

### Task 1: Establish Current Build Baseline

**Files:**
- Read: `ACE-Step-Plugin\BUILD.md`
- Read: `ACE-Step-Plugin\scripts\build-stub-vst3.ps1`
- Read: `ACE-Step-Plugin\scripts\validate-bundle.ps1`
- Modify after evidence is collected: `ACE-Step-Plugin\BUILD.md`

- [ ] **Step 1: Capture the current commit and working tree state**

Run:

```powershell
git --no-pager status --short
git rev-parse --short=12 HEAD
```

Expected:

```text
git status prints no modified, deleted, or untracked files before validation starts.
git rev-parse prints one 12-character commit id for the build under validation.
```

- [ ] **Step 2: Build the stub VST3 bundle**

Run:

```powershell
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
```

Expected:

```text
SUCCESS: Stub VST3 built successfully.
Bundle path: ...\ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3
```

- [ ] **Step 3: Validate the real-backend bundle when the real build tree exists**

Run:

```powershell
.\ACE-Step-Plugin\scripts\validate-bundle.ps1 -BuildDir C:\b\ace-ninja -Config RelWithDebInfo
```

Expected:

```text
[PASS] Plugin binary exists: ACE-Step.vst3
[PASS] Required DLL present: ggml.dll
[PASS] Required DLL present: ggml-base.dll
[PASS] Required DLL present: ggml-cpu.dll
[PASS] Required DLL present: ggml-cuda.dll
[PASS] Required DLL present: ggml-vulkan.dll
[PASS] Bundle structure is correct and no direct CUDA imports found.
```

If `C:\b\ace-ninja` is absent, configure the real build before validating:

```powershell
cmake -S ACE-Step-Plugin -B C:\b\ace-ninja -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=ON -DACESTEP_BUILD_TESTS=OFF -DACESTEP_PLUGIN_MODE=static
cmake --build C:\b\ace-ninja --config RelWithDebInfo --target AceStepPlugin_VST3 --parallel
.\ACE-Step-Plugin\scripts\validate-bundle.ps1 -BuildDir C:\b\ace-ninja -Config RelWithDebInfo
```

- [ ] **Step 4: Record baseline evidence in BUILD.md**

Modify `ACE-Step-Plugin\BUILD.md` under `Real Backend Validation Status` so the table includes the actual commit id from Step 1, the real bundle path, and the successful `validate-bundle.ps1` result.

- [ ] **Step 5: Commit the baseline evidence**

Run:

```powershell
git add ACE-Step-Plugin\BUILD.md
git commit -m "docs: record current vst3 build baseline" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
```

Expected:

```text
[agents/stage-commit-remove-blockers <commit>] docs: record current vst3 build baseline
```

---

### Task 2: Complete AudioPluginHost Pass-Through Evidence

**Files:**
- Read: `ACE-Step-Plugin\docs\audiopluginhost-blocker-investigation.md`
- Read: `ACE-Step-Plugin\docs\validate-host-load.md`
- Modify: `ACE-Step-Plugin\docs\validate-host-load.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Generate local build-only AudioPluginHost stub assets**

Run:

```powershell
python ACE-Step-Plugin\scripts\generate-stub-juce-assets.py
```

Expected:

```text
Stub asset generation complete.
Note: These are minimal silent WAV files for build-only.
```

- [ ] **Step 2: Build JUCE AudioPluginHost from the vendored JUCE tree**

Run:

```powershell
Push-Location ACE-Step-Plugin\External\JUCE
cmake -B build-audio-plugin-host -G "Visual Studio 17 2022" -A x64 -DJUCE_BUILD_EXTRAS=ON -DJUCE_BUILD_EXAMPLES=OFF
cmake --build build-audio-plugin-host --config RelWithDebInfo --target AudioPluginHost
Pop-Location
```

Expected:

```text
AudioPluginHost.exe exists under ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\
```

- [ ] **Step 3: Launch AudioPluginHost and scan the current ACE-Step VST3**

Run:

```powershell
& ".\ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\AudioPluginHost.exe"
```

Manual validation:

```text
Open Options -> Edit the List of Available Plug-ins...
Add this VST3 scan path:
ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3
Scan for new or updated VST3 plug-ins.
Confirm ACE-Step appears with vendor Allwave Media.
```

- [ ] **Step 4: Validate AudioPluginHost editor and pass-through**

Manual validation:

```text
Create graph: Audio Input -> ACE-Step (VST3) -> Audio Output.
Open the ACE-Step editor window and confirm it opens without errors.
Set Audio Settings to 48000 Hz and a stable buffer size such as 256 samples.
Send a non-silent stereo signal into Audio Input for at least 5 seconds.
Confirm output remains non-silent, continuous, and level-matched with no audible gain change, silence, pops, clicks, or drop-outs.
Unload and reload the ACE-Step node once; confirm AudioPluginHost remains stable.
```

- [ ] **Step 5: Record AudioPluginHost evidence**

Modify `ACE-Step-Plugin\docs\validate-host-load.md` in the `Host A (AudioPluginHost) Validation Status` section:

```markdown
**Status:** PASS

### Current AudioPluginHost validation

| Field | Value |
|---|---|
| Host executable | `ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\AudioPluginHost.exe` |
| OS | Record the exact Windows version from `systeminfo` or Settings -> System -> About |
| Build commit | Record the 12-character commit id from `git rev-parse --short=12 HEAD` |
| Bundle path | `ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | Record the tester name or GitHub username |
| Result | PASS |

AudioPluginHost scan/load, editor open, pass-through, reload stability, and 5-second continuous audio checks passed for the current build. The generated silent JUCE assets were used only to satisfy AudioPluginHost BinaryData build inputs; ACE-Step VST3 validation did not depend on AudioPluginHost demo-plugin audio quality.
```

- [ ] **Step 6: Update OpenSpec task 2.7 AudioPluginHost status**

Modify `openspec\changes\add-juce-vst3-plugin\tasks.md` under task `2.7` so `AudioPluginHost` says `PASS` and references `ACE-Step-Plugin\docs\validate-host-load.md`.

- [ ] **Step 7: Commit AudioPluginHost evidence**

Run:

```powershell
git add ACE-Step-Plugin\docs\validate-host-load.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs: record audiopluginhost pass-through evidence" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
```

Expected:

```text
[agents/stage-commit-remove-blockers <commit>] docs: record audiopluginhost pass-through evidence
```

---

### Task 3: Complete Current Reaper Evidence

**Files:**
- Modify: `ACE-Step-Plugin\docs\validate-host-load.md`
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Verify Reaper is installed**

Run:

```powershell
Test-Path "C:\Program Files\REAPER (x64)\reaper.exe"
```

Expected:

```text
True
```

- [ ] **Step 2: Scan and load the current ACE-Step VST3 in Reaper**

Manual validation:

```text
Open Reaper.
Open Options -> Preferences -> Plug-ins -> VST.
Add this VST3 path:
ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3
Run Clear cache/re-scan.
Confirm VST3: ACE-Step (Allwave Media) appears.
Create a track and insert ACE-Step as an FX.
Open the ACE-Step FX UI and confirm it opens.
```

- [ ] **Step 3: Validate Reaper pass-through**

Manual validation:

```text
Create or import a 48 kHz stereo sine WAV with a known peak below 0 dBFS.
Render or monitor the track once with ACE-Step bypassed and once enabled.
Confirm enabled output is non-silent, continuous, and level-matched to the bypassed baseline.
If using a measurement script, record peak_diff and rms_diff; both must be 0.000000000 for an exact pass-through render.
```

- [ ] **Step 4: Validate Reaper interactive UI/export cells**

Manual validation:

```text
Open the editor and inspect layout for clipped labels or unusable controls.
Toggle capture Arm and Clear; confirm the controls respond and do not interrupt audio.
Open the generation UI with models missing and with models present; confirm state changes are visible and host remains stable.
Generate or use an existing generated WAV asset; validate Save As writes a WAV to a user-selected folder.
Drag the generated WAV from the tile to Reaper; confirm Reaper accepts it as media or document the exact host-owned fallback behavior.
Confirm MIDI export is visibly unavailable unless note/event data is present.
Confirm stem export is visibly unavailable unless stem WAV metadata is present.
Save and reload a preset; confirm editor state updates without auto-starting generation.
```

- [ ] **Step 5: Record Reaper evidence**

Modify `ACE-Step-Plugin\docs\validate-host-load.md` under `Reaper validation record` and `ACE-Step-Plugin\docs\host-compatibility-matrix.md` under the Reaper row with the actual current commit id, Reaper version, bundle path, tester, and PASS/PARTIAL result for each validated cell.

- [ ] **Step 6: Update OpenSpec tasks that Reaper fully unblocks**

If AudioPluginHost also passed in Task 2, mark `2.7` complete. Keep `11.3` and `12.4` incomplete unless every required host is validated or scope is changed in Task 4.

- [ ] **Step 7: Commit Reaper evidence**

Run:

```powershell
git add ACE-Step-Plugin\docs\validate-host-load.md ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs: record current reaper host evidence" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
```

Expected:

```text
[agents/stage-commit-remove-blockers <commit>] docs: record current reaper host evidence
```

---

### Task 4: Resolve Non-Reaper Host Availability Blockers

**Files:**
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Check every required host path**

Run:

```powershell
@(
  "C:\Program Files\Image-Line\FL Studio 21\FL64.exe",
  "C:\Program Files\Steinberg\Cubase 13\Cubase13.exe",
  "C:\Program Files\PreSonus\Studio One 6\Studio One.exe",
  "C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe",
  "C:\Program Files\Bitwig Studio\Bitwig Studio.exe"
) | ForEach-Object { [pscustomobject]@{ Path = $_; Installed = Test-Path $_ } } | Format-Table -AutoSize
```

Expected in the current environment:

```text
Installed is False for each non-Reaper host unless the machine has changed since the last matrix update.
```

- [ ] **Step 2: Choose the unblock path**

Use one of these two paths; do not mix them in one commit:

```text
Path A: Install or access each missing DAW, then validate every matrix cell and record exact evidence.
Path B: Get explicit OpenSpec/product approval to narrow v1 release validation scope to installed hosts and list the non-installed DAWs as known external validation gaps.
```

- [ ] **Step 3A: If using Path A, validate each installed DAW**

Manual validation per host:

```text
Add the ACE-Step VST3 scan path.
Clear cache or rescan.
Confirm ACE-Step appears as a VST3 from Allwave Media.
Insert ACE-Step on a stereo audio track.
Open the editor.
Run at least 5 seconds of pass-through audio.
Validate capture controls, generation UI state, WAV Save As, WAV drag/drop, MIDI unavailable state, stem unavailable state, and preset browsing.
Record host version, OS, build commit, bundle path, tester, result, and host-owned differences.
```

- [ ] **Step 3B: If using Path B, update scope explicitly**

Modify `openspec\changes\add-juce-vst3-plugin\tasks.md` so tasks `11.3` and `12.4` state that non-installed commercial DAWs are accepted external validation gaps for v1, while Reaper and AudioPluginHost remain required local release gates.

Modify `ACE-Step-Plugin\docs\host-compatibility-matrix.md` so the non-installed DAW rows use this result text:

```text
BLOCKED - host not installed in validation environment; accepted external validation gap only if OpenSpec scope is narrowed for v1.
```

- [ ] **Step 4: Commit host-scope resolution**

Run:

```powershell
git add ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs: resolve host validation scope blockers" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
```

Expected:

```text
[agents/stage-commit-remove-blockers <commit>] docs: resolve host validation scope blockers
```

---

### Task 5: Complete Error-State Validation

**Files:**
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Validate missing-model behavior without deleting model files**

Run:

```powershell
$modelRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models"
$holdRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models.validation-hold"
if (Test-Path $holdRoot) { Remove-Item -Recurse -Force $holdRoot }
if (Test-Path $modelRoot) { Rename-Item $modelRoot $holdRoot }
```

Manual validation:

```text
Launch the VST3 in Reaper.
Open ACE-Step editor.
Confirm missing-model setup state appears and host does not crash.
Close Reaper.
```

Restore models:

```powershell
$modelRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models"
$holdRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models.validation-hold"
if (Test-Path $modelRoot) { Remove-Item -Recurse -Force $modelRoot }
if (Test-Path $holdRoot) { Rename-Item $holdRoot $modelRoot }
```

- [ ] **Step 2: Validate checksum mismatch behavior with a disposable copied model**

Run:

```powershell
$modelRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models"
$badRoot = Join-Path $env:TEMP "AceStepPlugin-bad-models"
if (Test-Path $badRoot) { Remove-Item -Recurse -Force $badRoot }
Copy-Item -Recurse $modelRoot $badRoot
$target = Join-Path $badRoot "vae-BF16.gguf"
$stream = [System.IO.File]::Open($target, [System.IO.FileMode]::Open, [System.IO.FileAccess]::ReadWrite)
$stream.Position = 0
$stream.WriteByte(0)
$stream.Dispose()
```

Manual validation:

```text
Point the plugin model setup flow at %TEMP%\AceStepPlugin-bad-models if the UI supports manual model directory selection.
If the current UI only supports %LOCALAPPDATA%\AceStepPlugin\models, move the real model directory aside using the Step 1 hold pattern, move %TEMP%\AceStepPlugin-bad-models into %LOCALAPPDATA%\AceStepPlugin\models, launch Reaper, verify checksum mismatch is surfaced in the editor, then restore the real directory.
```

- [ ] **Step 3: Validate cancellation and generation failure surfaces**

Manual validation:

```text
Launch the real bundle with valid models.
Start a short turbo generation.
Click Cancel during the DiT progress phase.
Confirm the editor reports cancellation and Reaper remains responsive.
Start a generation with intentionally invalid prompt input if the UI exposes validation, or temporarily point models to the checksum-mismatched directory from Step 2.
Confirm the editor reports failure without crashing the host.
```

- [ ] **Step 4: Validate gated unavailable states**

Manual validation:

```text
Open a generated full-mix WAV asset that has no reliable note/event metadata.
Confirm MIDI export is disabled and explains that MIDI is unavailable.
Open a generated asset with no stem WAV metadata.
Confirm stem export is disabled and explains that stems are unavailable.
Load an invalid preset JSON file through the preset flow.
Confirm the editor surfaces preset load failure and host remains stable.
```

- [ ] **Step 5: Record error-state evidence**

Modify `ACE-Step-Plugin\docs\host-compatibility-matrix.md` with a new `Error-state validation record` subsection containing the current commit id, host, bundle path, tester, and PASS/PARTIAL result for each validated error state.

Modify `openspec\changes\add-juce-vst3-plugin\tasks.md` under task `12.5` so it records which cases passed and which cases remain explicitly blocked or unsafe to run.

- [ ] **Step 6: Commit error-state evidence**

Run:

```powershell
git add ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs: record vst3 error-state validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
```

Expected:

```text
[agents/stage-commit-remove-blockers <commit>] docs: record vst3 error-state validation
```

---

## Self-Review

- Spec coverage: The plan addresses OpenSpec blockers in tasks 2.7, 11.3, 12.4, and 12.5, plus the historical AudioPluginHost asset blocker.
- Placeholder scan: No task uses `TBD`, `TODO`, `implement later`, or unexplained placeholders. Dynamic values are collected by explicit commands and recorded as command outputs.
- Type consistency: File paths, script names, task ids, and host names match the existing repository files and documentation.
- Scope control: The plan does not change plugin runtime behavior. It either records evidence for current behavior or requires an explicit scope decision for unavailable commercial hosts.
