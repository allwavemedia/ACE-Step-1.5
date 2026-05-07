# ACE-Step VST3 Remaining Validation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the remaining evidence-gated OpenSpec tasks for `add-juce-vst3-plugin` without marking any validation task complete before real host, bundle, backend, and error-state evidence exists.

**Architecture:** The implementation code for MIDI, stems, presets, host notes, and release documentation is already present. Remaining work is validation-first: collect reproducible evidence, record exact versions/paths/commits/results in the plugin docs, then update OpenSpec checkboxes only when each gate is satisfied. Any unavailable host, backend, model, GPU, or SDK prerequisite is recorded as a blocker instead of converted into a pass/fail result.

**Tech Stack:** Windows VST3, JUCE 8.0.10, CMake + Visual Studio 2022, PowerShell, REAPER v7.71/x64, OpenSpec CLI, `dumpbin`, CUDA Toolkit/GGML backends when real-engine validation is available.

---

## Current State

- Worktree: `A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-add-juce-vst3-plugin-validation`
- Branch: `feat/add-juce-vst3-plugin-8-3-onwards`
- Draft PR: `https://github.com/allwavemedia/ACE-Step-1.5/pull/2`
- Latest validation evidence commit when this plan was written: `95f2e449 docs(plugin): record Reaper validation evidence`
- OpenSpec progress: 64/72 tasks complete.
- Partial REAPER evidence already recorded:
  - REAPER v7.71/x64 discovered `VST3: ACE-Step (Allwave Media)`.
  - ReaScript inserted ACE-Step on a track and opened the FX UI.
  - Offline accessor pass-through produced identical enabled/bypassed peak and RMS values.

## Remaining OpenSpec Gates

| Task | Status | Completion condition |
|---|---|---|
| 2.7 | Open | JUCE AudioPluginHost and Reaper both load the VST3 and prove unchanged pass-through with recorded versions, bundle path, build commit, tester, and notes. |
| 3.7 | Open | Real backend bundle dependency evidence shows expected plugin and GGML runtime DLL relationships with `dumpbin /dependents` and bundle contents. |
| 6.9 | Open | Real-engine smoke test runs turbo generation with model files, GGML backends, and suitable hardware. |
| 11.3 | Open | Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig matrix cells are validated for scan/load, pass-through, editor, capture, generation state, exports, gated MIDI/stems, and presets. |
| 12.1 | Open | DAW scan time is measured and remains fast because models are not loaded during plugin construction/scan. |
| 12.3 | Open | Real VST3 bundle contains plugin DLL plus CPU, CUDA, and Vulkan GGML backend DLL siblings. |
| 12.4 | Open | External drag/drop is validated in every supported DAW, with Save As fallback recorded where drag/drop differs. |
| 12.5 | Open | Missing model, checksum mismatch, OOM/backend/generation/cancel errors, MIDI/stem unavailable/failure states, preset load failure, and host compatibility errors surface in the editor without crashing the host. |

## File Structure

Validation execution should modify only documentation/OpenSpec artifacts unless a validation script is intentionally promoted from the session workspace into the repository.

- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
  - Owns DAW matrix cells, host-owned notes, and per-host validation records.
- Modify: `ACE-Step-Plugin\docs\validate-host-load.md`
  - Owns task 2.7 AudioPluginHost/Reaper checklist and final combined host-load record.
- Modify: `ACE-Step-Plugin\BUILD.md`
  - Owns release validation commands and troubleshooting notes if validation discovers missing prerequisite steps.
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`
  - Owns final OpenSpec checkbox state and concise evidence notes.
- Optional create: `ACE-Step-Plugin\docs\release-validation-record.md`
  - Use only if the matrix/doc pages become too large; otherwise keep evidence in existing docs.
- Optional create: `ACE-Step-Plugin\scripts\validate-reaper-smoke.ps1`
  - Promote the session-proven Reaper smoke automation only if repeated Reaper automation is needed by future reviewers.

Do not commit session artifacts under `C:\Users\ldoby\.copilot\session-state\...`; cite them in docs only when they are evidence from the current validation run.

---

### Task 1: Preflight and Evidence Ledger

**Files:**
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Confirm branch, cleanliness, and current OpenSpec state**

Run:

```powershell
cd A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-add-juce-vst3-plugin-validation
git --no-pager branch --show-current
git --no-pager status --short
git --no-pager log -1 --oneline
openspec instructions apply --change "add-juce-vst3-plugin" --json
openspec validate add-juce-vst3-plugin --strict
```

Expected:

```text
feat/add-juce-vst3-plugin-8-3-onwards
Change 'add-juce-vst3-plugin' is valid
```

If `git status --short` shows user changes, inspect them and do not overwrite them. If OpenSpec validation fails, stop and fix the spec/doc inconsistency before running host validation.

- [ ] **Step 2: Build the current stub VST3**

Run:

```powershell
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
Test-Path "ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3"
Test-Path "ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3\Contents\x86_64-win\ACE-Step.vst3"
```

Expected:

```text
SUCCESS: Stub VST3 built successfully.
True
True
```

- [ ] **Step 3: Record the validation run identity**

Collect:

```powershell
$commit = git rev-parse --short=12 HEAD
$os = Get-ComputerInfo -Property OsName,OsVersion,OsBuildNumber
$bundle = Resolve-Path "ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3"
"Commit: $commit"
"OS: $($os.OsName) $($os.OsVersion) build $($os.OsBuildNumber)"
"Bundle: $bundle"
```

Add or update a short “Current validation status” section in `ACE-Step-Plugin\docs\host-compatibility-matrix.md` using this shape:

```markdown
## Current validation status

Validation run:

| Field | Value |
|---|---|
| Build commit | Value printed by `$commit = git rev-parse --short=12 HEAD` |
| OS | Values printed by `Get-ComputerInfo -Property OsName,OsVersion,OsBuildNumber` |
| Stub bundle | `ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Result | Evidence collection in progress; OpenSpec validation gates remain unchecked until the records below pass. |
```

- [ ] **Step 4: Validate docs/spec after the ledger update**

Run:

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
```

Expected:

```text
Change 'add-juce-vst3-plugin' is valid
```

- [ ] **Step 5: Commit preflight evidence if documentation changed**

Run:

```powershell
git add -- ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): update validation run ledger" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

Expected: one docs commit is pushed, or no commit is created if the ledger already contains the exact current run identity.

---

### Task 2: Complete Task 2.7 Host-Load Validation

**Files:**
- Modify: `ACE-Step-Plugin\docs\validate-host-load.md`
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Verify AudioPluginHost prerequisites**

Run:

```powershell
$assets = @(
  "ACE-Step-Plugin\External\JUCE\examples\Assets\cassette_recorder.wav",
  "ACE-Step-Plugin\External\JUCE\examples\Assets\cello.wav",
  "ACE-Step-Plugin\External\JUCE\examples\Assets\guitar_amp.wav",
  "ACE-Step-Plugin\External\JUCE\examples\Assets\reverb_ir.wav",
  "ACE-Step-Plugin\External\JUCE\examples\Assets\proaudio.path",
  "ACE-Step-Plugin\External\JUCE\examples\Assets\singing.ogg"
)
foreach ($asset in $assets) {
  if (Test-Path $asset) { "FOUND`t$asset" } else { "MISSING`t$asset" }
}
```

Expected for a buildable AudioPluginHost:

```text
FOUND   ACE-Step-Plugin\External\JUCE\examples\Assets\cassette_recorder.wav
FOUND   ACE-Step-Plugin\External\JUCE\examples\Assets\cello.wav
FOUND   ACE-Step-Plugin\External\JUCE\examples\Assets\guitar_amp.wav
FOUND   ACE-Step-Plugin\External\JUCE\examples\Assets\reverb_ir.wav
FOUND   ACE-Step-Plugin\External\JUCE\examples\Assets\proaudio.path
FOUND   ACE-Step-Plugin\External\JUCE\examples\Assets\singing.ogg
```

If any asset is missing, do not mark task 2.7 complete. Restore the missing files from the pinned JUCE 8.0.10 source used by the repository or use a known-good prebuilt `AudioPluginHost.exe`. Record the exact missing assets in `ACE-Step-Plugin\docs\validate-host-load.md`.

- [ ] **Step 2: Build AudioPluginHost from the JUCE root**

Run:

```powershell
cd ACE-Step-Plugin\External\JUCE
cmake -B build-audio-plugin-host -G "Visual Studio 17 2022" -A x64 `
  -DJUCE_BUILD_EXTRAS=ON `
  -DJUCE_BUILD_EXAMPLES=OFF
cmake --build build-audio-plugin-host --config RelWithDebInfo --target AudioPluginHost --parallel
cd ..\..\..
Test-Path "ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\AudioPluginHost.exe"
```

Expected:

```text
True
```

- [ ] **Step 3: Run the AudioPluginHost manual checklist**

Use `ACE-Step-Plugin\docs\validate-host-load.md`, Host A.

Record these exact fields:

```markdown
AudioPluginHost version: value read from the AudioPluginHost About dialog
Bundle path: ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3
Build commit: output of `git rev-parse --short=12 HEAD`
Result:
- Scan list shows ACE-Step / Allwave Media: PASS
- Editor opens with no error dialog: PASS
- Audio input -> ACE-Step -> audio output pass-through for at least 5 seconds: PASS
- Host remains stable after unload/reload: PASS
Notes: exact observed host-owned prompts, or the literal value `none`
```

If any row fails, record `Result: FAIL` plus the exact host message. Do not update the OpenSpec checkbox.

- [ ] **Step 4: Re-run or confirm Reaper host-load evidence**

Use the existing Reaper automation evidence as a baseline, then perform the manual Reaper checklist in `ACE-Step-Plugin\docs\validate-host-load.md`, Host B.

Run to confirm Reaper version:

```powershell
(Get-Item "C:\Program Files\REAPER (x64)\reaper.exe").VersionInfo | Select-Object ProductVersion,FileVersion
```

Expected:

```text
ProductVersion FileVersion
-------------- -----------
7.71           7.71
```

Manual checks to record:

```markdown
Reaper version: REAPER v7.71/x64
Bundle path: ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3
Build commit: output of `git rev-parse --short=12 HEAD`
Result:
- FX browser shows VST3: ACE-Step (Allwave Media): PASS
- Editor opens without crash or rendering error: PASS
- Offline accessor pass-through peak/RMS diff remains 0.000000000: PASS
- Host remains stable after unload/reload: PASS
Notes: Existing automated evidence is stored under C:\Users\ldoby\.copilot\session-state\4eeaba01-8b77-4fe6-8bdc-8eb1c614ce76\files\reaper-validation-ace\
```

- [ ] **Step 5: Update task 2.7 only when both hosts pass**

In `openspec\changes\add-juce-vst3-plugin\tasks.md`, change:

```markdown
- [ ] 2.7 Verify the initial VST3 bundle loads in JUCE AudioPluginHost and Reaper with unchanged audio pass-through.
```

to:

```markdown
- [x] 2.7 Verify the initial VST3 bundle loads in JUCE AudioPluginHost and Reaper with unchanged audio pass-through.
  Evidence: AudioPluginHost version recorded in `ACE-Step-Plugin\docs\validate-host-load.md` and REAPER v7.71/x64 loaded `ACE-Step.vst3` from `ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3`, displayed `ACE-Step` / `Allwave Media`, opened the editor, and passed unchanged audio through. Full host records are in `ACE-Step-Plugin\docs\validate-host-load.md`.
```

- [ ] **Step 6: Validate and commit task 2.7 evidence**

Run:

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\docs\validate-host-load.md ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): record host-load validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

Expected:

```text
Change 'add-juce-vst3-plugin' is valid
```

---

### Task 3: Complete Tasks 3.7 and 12.3 Real Bundle Dependency Validation

**Files:**
- Modify: `ACE-Step-Plugin\BUILD.md`
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Confirm required toolchain availability**

Run:

```powershell
where.exe cmake
where.exe dumpbin
where.exe nvcc
where.exe glslc
```

Expected:

```text
cmake path is printed
dumpbin path is printed
nvcc path is printed for CUDA Toolkit validation
glslc path is printed for Vulkan SDK validation
```

If `dumpbin`, `nvcc`, or `glslc` is missing, record the missing tool in `ACE-Step-Plugin\BUILD.md` and leave tasks 3.7 and 12.3 unchecked.

- [ ] **Step 2: Configure and build the real backend VST3 bundle**

Run:

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-real" -G "Visual Studio 17 2022" -A x64 `
  -DACESTEP_ENABLE_ACESTEP_CPP=ON `
  -DACESTEP_BUILD_REAL_SMOKE_TEST=ON `
  -DACESTEP_BUILD_TESTS=OFF
cmake --build "ACE-Step-Plugin\build-real" --config RelWithDebInfo --parallel
```

Expected:

```text
ACE-Step.vst3 is built under ACE-Step-Plugin\build-real\AceStepPlugin_artefacts\RelWithDebInfo\VST3
AceStepRealSmokeTest.exe is built under ACE-Step-Plugin\build-real\RelWithDebInfo
```

- [ ] **Step 3: Validate real bundle contents**

Run:

```powershell
.\ACE-Step-Plugin\scripts\validate-bundle.ps1
Get-ChildItem "ACE-Step-Plugin\build-real\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3\Contents\x86_64-win" | Select-Object Name,Length
```

Expected bundle directory includes:

```text
ACE-Step.vst3
ggml*.dll entries required by CPU backend
ggml*.dll entries required by CUDA backend
ggml*.dll entries required by Vulkan backend
```

If the real build intentionally produces different exact DLL names, record the exact filenames and the CMake options that selected them. Do not mark 12.3 complete unless CPU, CUDA, and Vulkan backend DLL siblings are present in the bundle.

- [ ] **Step 4: Record `dumpbin /dependents` output**

Run:

```powershell
$dll = "ACE-Step-Plugin\build-real\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3\Contents\x86_64-win\ACE-Step.vst3"
dumpbin /dependents $dll
```

Expected:

```text
Dump of file ...\ACE-Step.vst3
File Type: DLL
Image has the following dependencies:
```

Record the full dependency list in `ACE-Step-Plugin\BUILD.md` or `ACE-Step-Plugin\docs\host-compatibility-matrix.md`.

- [ ] **Step 5: Update OpenSpec tasks 3.7 and 12.3**

Only after Steps 2-4 pass, update `openspec\changes\add-juce-vst3-plugin\tasks.md`:

```markdown
- [x] 3.7 Verify `dumpbin /dependents` and bundle contents show expected plugin and GGML runtime DLL relationships.
  Evidence: real backend build at the commit printed by `git rev-parse --short=12 HEAD` produced `ACE-Step.vst3`; `dumpbin /dependents` and `validate-bundle.ps1` output are recorded in `ACE-Step-Plugin\BUILD.md` or `ACE-Step-Plugin\docs\host-compatibility-matrix.md`.
...
- [x] 12.3 Verify the VST3 bundle contains the plugin DLL plus CPU, CUDA, and Vulkan GGML backend DLL siblings.
  Evidence: bundle directory `ACE-Step-Plugin\build-real\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3\Contents\x86_64-win` contains `ACE-Step.vst3` plus recorded CPU/CUDA/Vulkan GGML backend DLL siblings.
```

- [ ] **Step 6: Validate and commit dependency evidence**

Run:

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\BUILD.md ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): record real bundle dependency validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

---

### Task 4: Complete Task 6.9 Real-Engine Smoke Validation

**Files:**
- Modify: `ACE-Step-Plugin\BUILD.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Confirm model files exist and pass manifest validation**

Run:

```powershell
$modelRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models"
Test-Path $modelRoot
Get-ChildItem $modelRoot -Recurse | Select-Object FullName,Length
```

Expected:

```text
True
Required GGUF model files are present with non-zero sizes.
```

If model files are absent, use the plugin first-run setup UI or documented downloader flow. Record absent files in `ACE-Step-Plugin\BUILD.md` and leave task 6.9 unchecked.

- [ ] **Step 2: Confirm real smoke executable exists**

Run:

```powershell
Test-Path "ACE-Step-Plugin\build-real\RelWithDebInfo\AceStepRealSmokeTest.exe"
```

Expected:

```text
True
```

If it is missing, complete Task 3’s real build first.

- [ ] **Step 3: Run the real-engine smoke test**

Run:

```powershell
.\ACE-Step-Plugin\build-real\RelWithDebInfo\AceStepRealSmokeTest.exe
```

Expected:

```text
Model loading starts from the local model root.
Turbo generation starts.
WAV output path is printed.
Smoke test exits with code 0.
```

Capture the output WAV path and verify it exists:

```powershell
$smokeWav = Read-Host "Paste the WAV output path printed by AceStepRealSmokeTest.exe"
Test-Path $smokeWav
Get-Item $smokeWav | Select-Object FullName,Length
```

Expected: `True` and a non-zero byte length.

- [ ] **Step 4: Update OpenSpec task 6.9**

Only after Step 3 exits with code 0 and the output WAV exists, update:

```markdown
- [x] 6.9 Run a real-engine smoke test with turbo generation once model files and GGML backends are available.
  Evidence: `ACE-Step-Plugin\build-real\RelWithDebInfo\AceStepRealSmokeTest.exe` exited 0 using the OS/GPU details recorded in `ACE-Step-Plugin\BUILD.md`; local models were read from `%LOCALAPPDATA%\AceStepPlugin\models`; the generated WAV path and byte length are recorded in the same evidence note.
```

- [ ] **Step 5: Validate and commit real-engine smoke evidence**

Run:

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\BUILD.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): record real engine smoke validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

---

### Task 5: Complete Task 12.1 DAW Scan-Time Validation

**Files:**
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Measure isolated Reaper scan time**

Use an isolated Reaper profile and the stub or real VST3 bundle path. Record `script-start` and `scan-found` timestamps from the ReaScript smoke log.

Run:

```powershell
$evidence = "C:\Users\ldoby\.copilot\session-state\4eeaba01-8b77-4fe6-8bdc-8eb1c614ce76\files\reaper-validation-ace\reaper-ace-smoke.log"
Get-Content $evidence | Select-String -Pattern "script-start|scan-found|result=PASS"
```

Expected existing partial evidence:

```text
[000000 ms] script-start version=7.71/x64 ...
[000493 ms] scan-found index=15 name=VST3: ACE-Step (Allwave Media) ...
[001185 ms] result=PASS ...
```

If a fresh run is needed, reuse the isolated Reaper automation approach already recorded in `ACE-Step-Plugin\docs\host-compatibility-matrix.md`.

- [ ] **Step 2: Confirm plugin construction does not load models**

Inspect the processor/editor construction path:

```powershell
Select-String -Path "ACE-Step-Plugin\Source\PluginProcessor.cpp","ACE-Step-Plugin\Source\PluginEditor.cpp","ACE-Step-Plugin\Source\Engine\AceStepEngine.cpp" -Pattern "loadModels|generate|GGUF|models|ThreadPool|prepareToPlay|processBlock" -Context 2,2
```

Expected evidence:

```text
Plugin construction does not call loadModels() or generate().
Model readiness/loading is driven by explicit model management or generation flow, not VST3 scan.
processBlock remains pass-through/capture-only.
```

- [ ] **Step 3: Update OpenSpec task 12.1**

Only after Step 1 has scan timing and Step 2 confirms no construction-time model load, update:

```markdown
- [x] 12.1 Verify DAW scan time stays fast because GGUF models are not loaded during plugin construction or scan.
  Evidence: REAPER v7.71/x64 isolated scan found `VST3: ACE-Step (Allwave Media)` at the measured millisecond value recorded in `reaper-ace-smoke.log`; inspection of `PluginProcessor`/`PluginEditor` construction paths found no `loadModels()` or generation calls during scan.
```

- [ ] **Step 4: Validate and commit scan-time evidence**

Run:

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): record DAW scan-time validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

---

### Task 6: Complete Tasks 11.3 and 12.4 DAW Matrix and Drag/Drop Validation

**Files:**
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Check host installation paths**

Run:

```powershell
$hosts = [ordered]@{
  "Reaper x64" = "C:\Program Files\REAPER (x64)\reaper.exe"
  "FL Studio 21" = "C:\Program Files\Image-Line\FL Studio 21\FL64.exe"
  "Cubase 13" = "C:\Program Files\Steinberg\Cubase 13\Cubase13.exe"
  "Studio One 6" = "C:\Program Files\PreSonus\Studio One 6\Studio One.exe"
  "Ableton Live 12 Suite" = "C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe"
  "Bitwig Studio" = "C:\Program Files\Bitwig Studio\Bitwig Studio.exe"
}
foreach ($entry in $hosts.GetEnumerator()) {
  if (Test-Path $entry.Value) {
    $version = (Get-Item $entry.Value).VersionInfo.ProductVersion
    "FOUND`t$($entry.Key)`t$version`t$($entry.Value)"
  } else {
    "MISSING`t$($entry.Key)`t$($entry.Value)"
  }
}
```

Expected before full completion:

```text
FOUND lines for all six supported DAWs.
```

If any DAW is missing, record the missing host in `ACE-Step-Plugin\docs\host-compatibility-matrix.md` and leave tasks 11.3 and 12.4 unchecked.

- [ ] **Step 2: Prepare generated assets for export validation**

Use the real-engine smoke output from Task 4 for WAV Save As and drag/drop validation. If real-engine output is not available, do not mark WAV export or drag/drop cells complete.

Confirm:

```powershell
$generatedWav = Read-Host "Paste the real-engine generated WAV path from Task 4"
Test-Path $generatedWav
Get-Item $generatedWav | Select-Object FullName,Length
```

Expected: `True` and non-zero byte length.

- [ ] **Step 3: Validate each DAW with the same matrix**

For each host in this order: Reaper, FL Studio, Cubase, Studio One, Ableton Live, Bitwig.

Record this exact per-host row in `ACE-Step-Plugin\docs\host-compatibility-matrix.md`:

```markdown
### Reaper validation record

| Field | Value |
|---|---|
| Host version | value from the host About dialog |
| Host executable | executable path verified by Step 1 |
| Build commit | output of `git rev-parse --short=12 HEAD` |
| Bundle path | `ACE-Step-Plugin\build-real\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | name or GitHub username of the tester running the host |

| Check | Result | Notes |
|---|---|---|
| Scan/load | PASS | exact scan dialog/cache notes from the host |
| Pass-through | PASS | test signal name and duration used for the host |
| Editor layout | PASS | Prompt/lyrics/model/capture/generation/history/preset sections visible without clipping. |
| Capture controls | PASS | Arm, Clear, source label, and meter update without blocking audio. |
| Generation UI state | PASS | Missing-model/ready/generating/cancel states match the current environment. |
| WAV Save As | PASS | Saved copy path exists with non-zero byte length. |
| WAV drag/drop | PASS or FALLBACK | exact drop target behavior; if fallback, Save As path used |
| MIDI gated unavailable state | PASS | MIDI control shows unavailable/N/A and does not start a drag without a MIDI path. |
| Stem gated unavailable state | PASS | Stem controls remain unavailable unless successful stem WAVs exist. |
| Preset browsing | PASS | Save, filter/list, load-without-generation, rename, delete verified in host. |
```

If a host behaves differently for drag/drop, use `Result: FALLBACK` for the drag/drop row and record the host-owned behavior plus the Save As success path.

- [ ] **Step 4: Update the top-level matrix**

After each host record passes, replace that host’s `Pending manual validation` cells in the top matrix with `PASS`, `FALLBACK`, or a concise note that points to its detailed record. Do not update cells for hosts that have not been run.

- [ ] **Step 5: Update OpenSpec tasks 11.3 and 12.4 only after all six hosts pass**

When every host record is complete, update:

```markdown
- [x] 11.3 Verify scan/load, pass-through, editor layout, capture controls, generation UI state, WAV export, MIDI export when available, stem export, and preset browsing across Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig.
  Evidence: all six host records in `ACE-Step-Plugin\docs\host-compatibility-matrix.md` include exact host version, executable path, bundle path, build commit, tester, pass/fallback results, and notes.
...
- [x] 12.4 Verify external drag-and-drop in Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig, with Save As as fallback.
  Evidence: each host record in `ACE-Step-Plugin\docs\host-compatibility-matrix.md` records WAV drag/drop behavior and Save As fallback outcome.
```

- [ ] **Step 6: Validate and commit host matrix evidence**

Run:

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): record DAW compatibility validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

---

### Task 7: Complete Task 12.5 Error-State Host Validation

**Files:**
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `ACE-Step-Plugin\BUILD.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Create a Reaper error-state validation table**

Add this section to `ACE-Step-Plugin\docs\host-compatibility-matrix.md`:

```markdown
## Error-state validation record

| Error state | Host | Stimulus | Expected editor result | Host stability result | Evidence |
|---|---|---|---|---|---|
```

- [ ] **Step 2: Validate missing-model state**

Stimulus:

```powershell
$modelRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models"
$backupRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models.validation-backup"
if (Test-Path $backupRoot) { Remove-Item $backupRoot -Recurse -Force }
if (Test-Path $modelRoot) { Rename-Item $modelRoot $backupRoot }
```

Run the plugin in Reaper. Expected editor result: model setup/missing-model UI appears, generation is unavailable, host does not crash.

Restore:

```powershell
if (Test-Path $backupRoot) { Rename-Item $backupRoot $modelRoot }
```

Record:

```markdown
| Missing model | REAPER v7.71/x64 | `%LOCALAPPDATA%\AceStepPlugin\models` temporarily renamed | Missing model/setup state displayed; generation not started | PASS | screenshot or log path captured during the validation run |
```

- [ ] **Step 3: Validate checksum mismatch state**

Stimulus:

```powershell
$modelRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models"
$target = Get-ChildItem $modelRoot -Recurse -File | Select-Object -First 1
$backup = "$($target.FullName).validation-backup"
Copy-Item $target.FullName $backup -Force
Set-Content -Path $target.FullName -Value "invalid model bytes for checksum validation" -Encoding ASCII
```

Run the plugin in Reaper. Expected editor result: checksum/validation error is displayed, model is not marked ready, host does not crash.

Restore:

```powershell
Move-Item $backup $target.FullName -Force
```

Record:

```markdown
| Checksum mismatch | REAPER v7.71/x64 | One model file replaced with invalid ASCII bytes, then restored | Checksum/validation error displayed; model not ready | PASS | screenshot or log path captured during the validation run |
```

- [ ] **Step 4: Validate backend-load failure**

Stimulus:

```powershell
$bundleBin = "ACE-Step-Plugin\build-real\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3\Contents\x86_64-win"
$backend = Get-ChildItem $bundleBin -Filter "ggml*.dll" | Select-Object -First 1
$backup = "$($backend.FullName).validation-backup"
Rename-Item $backend.FullName $backup
```

Run the plugin in Reaper. Expected editor result: backend-load failure is displayed, generation is unavailable or fails cleanly, host does not crash.

Restore:

```powershell
Rename-Item $backup $backend.FullName
```

Record:

```markdown
| Backend-load failure | REAPER v7.71/x64 | One bundled GGML backend DLL temporarily renamed | Backend-load error displayed | PASS | screenshot or log path captured during the validation run |
```

- [ ] **Step 5: Validate cancellation and generation failure**

Cancellation stimulus:

1. Use valid models/backends.
2. Start a generation with a prompt such as `short electronic pop loop, bright drums, synth bass`.
3. Click Cancel while progress is active.

Expected editor result: cancellation status appears, no stale generated asset is added, host stays responsive.

Generation failure stimulus:

1. Use valid models/backends.
2. Temporarily set the generation output directory to a path that cannot be written by the plugin if the UI exposes a path selector; otherwise use the backend-load failure stimulus from Step 4 as the generation failure evidence.

Record:

```markdown
| Cancellation | REAPER v7.71/x64 | Cancel clicked during active generation | Canceled status displayed; no crash | PASS | screenshot or log path captured during the validation run |
| Generation failure | REAPER v7.71/x64 | Backend unavailable during generation start | Generation failure status displayed; no crash | PASS | screenshot or log path captured during the validation run |
```

- [ ] **Step 6: Validate MIDI/stem unavailable and preset load failure**

MIDI/stem unavailable stimulus:

1. Open the plugin after a normal full-mix result or with no eligible MIDI/stem metadata.
2. Confirm MIDI button shows unavailable/N/A and does not start file drag.
3. Confirm stem controls remain unavailable unless successful stem WAVs are present.

Preset load failure stimulus:

```powershell
$presetDir = Join-Path ([Environment]::GetFolderPath("ApplicationData")) "ACE-Step\Presets"
New-Item -ItemType Directory -Force -Path $presetDir | Out-Null
$badPreset = Join-Path $presetDir "invalid-validation-preset.json"
Set-Content -Path $badPreset -Value "{ invalid json" -Encoding ASCII
```

Run the plugin in Reaper and attempt to load the invalid preset. Expected editor result: preset load error is displayed, existing editor state is preserved, host does not crash.

Cleanup:

```powershell
Remove-Item $badPreset -Force
```

Record:

```markdown
| MIDI unavailable state | REAPER v7.71/x64 | Asset without note/event data | MIDI unavailable/N/A state displayed; no invalid drag | PASS | screenshot or log path captured during the validation run |
| Stem unavailable/failure state | REAPER v7.71/x64 | No successful stem WAV outputs available | Stem unavailable state displayed; no invalid export | PASS | screenshot or log path captured during the validation run |
| Preset load failure | REAPER v7.71/x64 | Invalid JSON preset loaded | Preset load error displayed; host stable | PASS | screenshot or log path captured during the validation run |
```

- [ ] **Step 7: Validate out-of-memory state only on suitable hardware**

Run this only on a validation machine where inducing OOM will not disrupt other workloads.

Stimulus:

1. Use the real backend with valid models.
2. Select the largest duration and highest memory mode exposed by the UI.
3. Run generation while monitoring GPU memory.

Expected editor result: out-of-memory error is surfaced and host remains stable.

If the available hardware cannot safely induce OOM, record:

```markdown
| Out-of-memory | Not executed | Available validation hardware could not safely induce OOM without risking unrelated workloads | Blocked; requires low-memory GPU validation machine | Not run | GPU model and installed RAM from `Get-CimInstance Win32_VideoController` and `Get-CimInstance Win32_ComputerSystem` |
```

Do not mark task 12.5 complete while OOM is blocked.

- [ ] **Step 8: Update OpenSpec task 12.5**

Only when every required error state has PASS evidence, update:

```markdown
- [x] 12.5 Verify missing model, checksum mismatch, out-of-memory, backend-load failure, cancellation, generation failure, MIDI unavailable state, stem failure, preset load failure, and host compatibility errors surface in the editor without crashing the host.
  Evidence: `ACE-Step-Plugin\docs\host-compatibility-matrix.md` records each error stimulus, expected editor result, host stability result, and evidence artifact.
```

- [ ] **Step 9: Validate and commit error-state evidence**

Run:

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\docs\host-compatibility-matrix.md ACE-Step-Plugin\BUILD.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): record error-state validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

---

### Task 8: Final Release-Readiness Closure

**Files:**
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`
- Modify: `ACE-Step-Plugin\BUILD.md`
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`

- [ ] **Step 1: Run full plugin validation commands**

Run:

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-codex-tests" -G "Visual Studio 17 2022" -A x64 `
  -DACESTEP_ENABLE_ACESTEP_CPP=OFF `
  -DACESTEP_BUILD_TESTS=ON
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
.\ACE-Step-Plugin\build-codex-tests\AceStepPluginTests_artefacts\RelWithDebInfo\AceStepPluginTests.exe
ctest --test-dir "ACE-Step-Plugin\build-codex-tests" -C RelWithDebInfo --output-on-failure
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
openspec validate add-juce-vst3-plugin --strict
```

Expected:

```text
AceStepPluginTests exits 0.
ctest exits 0.
Stub VST3 build exits 0.
Change 'add-juce-vst3-plugin' is valid.
```

- [ ] **Step 2: Confirm all OpenSpec tasks are either complete or truthfully blocked**

Run:

```powershell
Select-String -Path "openspec\changes\add-juce-vst3-plugin\tasks.md" -Pattern "^- \[ \]"
```

Expected before archive:

```text
No unchecked tasks remain.
```

If unchecked tasks remain, do not archive. Each unchecked task must have a recorded blocker and the PR must stay draft.

- [ ] **Step 3: Request final PR review only after implementation and validation evidence are complete**

Run:

```powershell
gh pr view 2 --repo allwavemedia/ACE-Step-1.5 --json isDraft,headRefName,baseRefName,mergeable
```

Expected:

```json
{"isDraft":true,"headRefName":"feat/add-juce-vst3-plugin-8-3-onwards","baseRefName":"main","mergeable":"MERGEABLE"}
```

When all tasks are complete and validation commands pass, request review:

```powershell
gh pr ready 2 --repo allwavemedia/ACE-Step-1.5
gh pr review 2 --repo allwavemedia/ACE-Step-1.5 --comment --body "Ready for final review. OpenSpec add-juce-vst3-plugin tasks are complete with validation evidence recorded in plugin docs."
```

- [ ] **Step 4: Final commit for release-readiness notes**

If final docs changed, run:

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\BUILD.md ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): complete VST3 release validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

Expected: final docs commit pushed to PR #2.

---

## Self-Review Notes

Spec coverage:

- Task 2 covers OpenSpec 2.7.
- Task 3 covers OpenSpec 3.7 and 12.3.
- Task 4 covers OpenSpec 6.9.
- Task 5 covers OpenSpec 12.1.
- Task 6 covers OpenSpec 11.3 and 12.4.
- Task 7 covers OpenSpec 12.5.
- Task 8 covers final validation, PR readiness, and archive readiness.

Placeholder scan:

- The plan contains no unresolved placeholders. Values that can only be known during validation are described by the exact command, dialog, screenshot, or log source that produces them.
- The plan explicitly says not to mark tasks complete when a host/tool/backend/model/GPU prerequisite is missing.

Type and name consistency:

- Paths consistently use the validation worktree and Windows path separators.
- OpenSpec task numbers and file names match the current `openspec\changes\add-juce-vst3-plugin\tasks.md`.
- Validation docs remain the source of evidence; OpenSpec checkboxes remain the source of task completion state.
