# ACE-Step VST3 Post-Merge Remaining Development Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the remaining evidence-gated `add-juce-vst3-plugin` OpenSpec tasks after the PR #2 merge without marking any validation gate complete before current, truthful evidence exists.

**Architecture:** The remaining work is validation and evidence management, not feature expansion. Keep production code unchanged unless a validation run exposes a real bug; in that case, stop this plan, write a narrow bug-fix plan, and return here only after the fix is merged.

**Tech Stack:** Windows 11, PowerShell, Visual Studio 2022 CMake environment, JUCE AudioPluginHost, Reaper x64, VST3 bundle at `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3`, OpenSpec task ledger under `openspec/changes/add-juce-vst3-plugin/tasks.md`.

---

## Current State

Open OpenSpec gates:

| Task | Current state | Required evidence before checking |
|---|---|---|
| `2.7` | Unchecked | Current bundle loads in JUCE AudioPluginHost and Reaper, with unchanged audio pass-through measured in both hosts. |
| `11.3` | Unchecked | Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig matrix cells validated for scan/load, pass-through, editor layout, capture controls, generation UI state, WAV export, gated MIDI/stem states, and preset browsing. |
| `12.4` | Unchecked | External drag/drop and Save As fallback validated across the same DAW matrix. |
| `12.5` | Unchecked | Error states surface in the editor without crashing the host: missing model, checksum mismatch, out-of-memory, backend-load failure, cancellation, generation failure, MIDI unavailable, stem failure, preset load failure, and host compatibility errors. |

Known blockers recorded in the merged repo:

| Blocker | Evidence location |
|---|---|
| AudioPluginHost pass-through has not been measured. | `ACE-Step-Plugin/docs/validate-host-load.md` |
| Current PR-head Reaper host rerun remains pending. | `ACE-Step-Plugin/docs/validate-host-load.md`, `ACE-Step-Plugin/docs/host-compatibility-matrix.md` |
| FL Studio, Cubase, Studio One, Ableton Live, and Bitwig were not installed at checked paths. | `ACE-Step-Plugin/docs/host-compatibility-matrix.md` |
| Model root exists with the four expected GGUF files, but destructive/manual error-state validation has not been executed. | `openspec/changes/add-juce-vst3-plugin/tasks.md` |

## File Responsibilities

| File | Responsibility |
|---|---|
| `ACE-Step-Plugin/docs/validate-host-load.md` | Task `2.7` host-load and pass-through evidence for JUCE AudioPluginHost and Reaper. |
| `ACE-Step-Plugin/docs/host-compatibility-matrix.md` | Task `11.3` and `12.4` host matrix, host availability, per-host validation cells, and host-owned behavior notes. |
| `openspec/changes/add-juce-vst3-plugin/tasks.md` | Source-of-truth OpenSpec checklist. Check a task only after its evidence document proves the gate. |
| `docs/superpowers/handoffs/handoff-2026-05-07-add-juce-vst3-plugin-remaining-validation.md` | End-of-session handoff with exact pass/blocker state and next action. |

Production code files are intentionally out of scope for this plan.

## Validation Rules

- Do not mark `2.7`, `11.3`, `12.4`, or `12.5` complete while any required evidence is missing.
- Record `BLOCKED - host not installed at checked path`, `BLOCKED - no generated WAV asset available in this validation run`, or another complete blocker sentence instead of `PASS` when host software, generated assets, a safe destructive path, or hardware prerequisites are unavailable.
- Do not destructively mutate `%LOCALAPPDATA%\AceStepPlugin\models` directly. Use a copied model root for checksum, missing-file, and backend-load validation.
- If validation exposes a plugin crash, stop and capture the crash details before any cleanup.
- Commit evidence-only updates separately from any future code fix.

---

### Task 1: Establish Fresh Validation Baseline

**Files:**
- Read: `openspec/changes/add-juce-vst3-plugin/tasks.md`
- Read: `ACE-Step-Plugin/docs/validate-host-load.md`
- Read: `ACE-Step-Plugin/docs/host-compatibility-matrix.md`
- Modify: `ACE-Step-Plugin/docs/host-compatibility-matrix.md`
- Modify: `openspec/changes/add-juce-vst3-plugin/tasks.md`

- [ ] **Step 1: Create an isolated validation worktree**

Run from the main checkout:

```powershell
cd A:\Repos\ACE-Step-1.5-allwavemedia
git fetch origin main --prune
git worktree add A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-post-merge-vst3-validation -b agents/post-merge-vst3-validation origin/main
cd A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-post-merge-vst3-validation
git status --short --branch
```

Expected status:

```text
## agents/post-merge-vst3-validation...origin/main
```

- [ ] **Step 2: Confirm the OpenSpec gates are still unchecked**

Run:

```powershell
Select-String -Path openspec\changes\add-juce-vst3-plugin\tasks.md `
  -Pattern '^- \[[ xX]\] (2\.7|11\.3|12\.4|12\.5)'
```

Expected result: all four matching lines begin with `- [ ]`.

- [ ] **Step 3: Capture current commit and bundle identity**

Run:

```powershell
$commit = git rev-parse --short=12 HEAD
$bundle = 'C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3'
$dll = Join-Path $bundle 'Contents\x86_64-win\ACE-Step.vst3'
Write-Host "Commit: $commit"
Get-Item -LiteralPath $dll | Select-Object FullName,Length,LastWriteTime
```

Expected result: the bundle DLL exists and prints its path, byte length, and timestamp.

- [ ] **Step 4: Rebuild and validate the real bundle**

Run in a Visual Studio 2022 developer PowerShell:

```powershell
cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 && cmake --build C:\b\ace-ninja --target AceStepPlugin_VST3 --parallel'
powershell -ExecutionPolicy Bypass -File ACE-Step-Plugin\scripts\validate-bundle.ps1 -BuildDir C:\b\ace-ninja -Config RelWithDebInfo
```

Expected result:

```text
ACE-Step.vst3 bundle validation passed
```

- [ ] **Step 5: Record baseline evidence without closing gates**

Update `ACE-Step-Plugin/docs/host-compatibility-matrix.md` under `## Current validation status` so the validation run table uses the current commit, bundle size, timestamp, and this result sentence:

```markdown
| Result | Current post-merge real bundle rebuilt successfully and `scripts\validate-bundle.ps1 -BuildDir C:\b\ace-ninja -Config RelWithDebInfo` passed in a VS 2022 developer environment. Host pass-through, interactive UI/export validation, DAW matrix validation, and destructive/manual error-state validation remain pending unless recorded below. |
```

Update `openspec/changes/add-juce-vst3-plugin/tasks.md` under task `2.7` with this evidence line, replacing the stale current PR-head wording:

```markdown
  **Current post-merge preflight:** PASS for rebuild and bundle validation at the current `origin/main` commit. `cmake --build C:\b\ace-ninja --target AceStepPlugin_VST3 --parallel` succeeded in a VS 2022 developer environment, and `scripts\validate-bundle.ps1 -BuildDir C:\b\ace-ninja -Config RelWithDebInfo` confirmed the required plugin/backend DLLs and no direct CUDA imports from the plugin binary. Current AudioPluginHost pass-through and Reaper host reruns remain pending until recorded in the validation docs.
```

- [ ] **Step 6: Commit the baseline evidence**

Run:

```powershell
git diff --check -- ACE-Step-Plugin/docs/host-compatibility-matrix.md openspec/changes/add-juce-vst3-plugin/tasks.md
git add ACE-Step-Plugin/docs/host-compatibility-matrix.md openspec/changes/add-juce-vst3-plugin/tasks.md
git commit -m "docs(plugin): record post-merge validation baseline"
```

Expected result: one docs-only commit.

---

### Task 2: Complete Task 2.7 Host Pass-Through Evidence

**Files:**
- Modify: `ACE-Step-Plugin/docs/validate-host-load.md`
- Modify: `ACE-Step-Plugin/docs/host-compatibility-matrix.md`
- Modify: `openspec/changes/add-juce-vst3-plugin/tasks.md`

- [ ] **Step 1: Verify AudioPluginHost and Reaper binaries**

Run:

```powershell
$paths = @(
  'ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\AudioPluginHost.exe',
  'C:\Program Files\REAPER (x64)\reaper.exe',
  'C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3'
)
foreach ($path in $paths) {
  "{0}`t{1}" -f (Test-Path -LiteralPath $path), $path
}
```

Expected result: all three lines begin with `True`.

- [ ] **Step 2: Measure AudioPluginHost pass-through**

Run the manual checklist in `ACE-Step-Plugin/docs/validate-host-load.md` sections `Host A - JUCE AudioPluginHost` and `Pass Criteria`.

Record the measured input/output level in dBFS or peak/RMS form. The pass criterion is:

```text
Output level matches input level; no silence; no unexpected gain change; no audible glitching over at least 5 seconds.
```

If AudioPluginHost cannot provide a reliable measurement path, record:

```text
AudioPluginHost pass-through: BLOCKED - AudioPluginHost graph did not provide a reliable level-matched measurement path in this validation environment.
```

Do not mark task `2.7` complete with this blocked result.

- [ ] **Step 3: Re-run Reaper scan/load and pass-through on the current commit**

Use the existing Reaper automation if present in the repository or prior evidence folder. If no committed automation exists, run the same ReaScript procedure documented in `ACE-Step-Plugin/docs/host-compatibility-matrix.md`: insert the VST3, open the FX UI, measure bypassed and enabled output peak/RMS with a 48 kHz stereo sine WAV.

Required result text for a pass:

```text
Reaper current post-merge validation: PASS - scan/load, FX UI-open, and offline pass-through measured identical peak/RMS with ACE-Step bypassed and enabled.
```

Required result text for a failure:

```text
Reaper current post-merge validation: FAIL - scan/load, FX UI-open, or offline pass-through differed. Do not mark task 2.7 complete.
```

- [ ] **Step 4: Update host-load documentation**

Append a completed section to `ACE-Step-Plugin/docs/validate-host-load.md` after the existing Reaper validation records. Generate the section header and fixed fields with this command, then edit only the two result lines to match the observed PASS, FAIL, or BLOCKED outcome from Steps 2 and 3:

```powershell
$commit = git rev-parse --short=12 HEAD
@"
### Post-Merge Host Pass-Through Validation Record

| Field | Value |
|---|---|
| Build commit | `$commit` |
| Bundle path | ``C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3`` |
| AudioPluginHost result | BLOCKED - AudioPluginHost pass-through result has not been edited from this generated starter line. |
| Reaper result | BLOCKED - Reaper pass-through result has not been edited from this generated starter line. |
| Tester | Codex CLI plus manual host validation |

Task 2.7 remains unchecked unless both AudioPluginHost and Reaper pass current post-merge pass-through validation.
"@
```

- [ ] **Step 5: Check task `2.7` only if both hosts passed**

If both hosts pass, replace task `2.7` in `openspec/changes/add-juce-vst3-plugin/tasks.md` with:

```markdown
- [x] 2.7 Verify the initial VST3 bundle loads in JUCE AudioPluginHost and Reaper with unchanged audio pass-through.
  **Post-merge evidence:** PASS. Current real bundle at `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` loaded in JUCE AudioPluginHost and Reaper, and both hosts measured unchanged audio pass-through. Evidence is recorded in `ACE-Step-Plugin\docs\validate-host-load.md`.
```

If either host is blocked or failed, leave task `2.7` unchecked and add one evidence line explaining the blocker.

- [ ] **Step 6: Commit task `2.7` evidence**

Run:

```powershell
git diff --check -- ACE-Step-Plugin/docs/validate-host-load.md ACE-Step-Plugin/docs/host-compatibility-matrix.md openspec/changes/add-juce-vst3-plugin/tasks.md
git add ACE-Step-Plugin/docs/validate-host-load.md ACE-Step-Plugin/docs/host-compatibility-matrix.md openspec/changes/add-juce-vst3-plugin/tasks.md
git commit -m "docs(plugin): record host pass-through validation"
```

Expected result: one docs-only commit.

---

### Task 3: Complete Tasks 11.3 and 12.4 Host Matrix Evidence

**Files:**
- Modify: `ACE-Step-Plugin/docs/host-compatibility-matrix.md`
- Modify: `openspec/changes/add-juce-vst3-plugin/tasks.md`

- [ ] **Step 1: Check supported DAW installation paths**

Run:

```powershell
$hosts = [ordered]@{
  'Reaper x64' = 'C:\Program Files\REAPER (x64)\reaper.exe'
  'FL Studio 21' = 'C:\Program Files\Image-Line\FL Studio 21\FL64.exe'
  'Cubase 13' = 'C:\Program Files\Steinberg\Cubase 13\Cubase13.exe'
  'Studio One 6' = 'C:\Program Files\PreSonus\Studio One 6\Studio One.exe'
  'Ableton Live 12 Suite' = 'C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe'
  'Bitwig Studio' = 'C:\Program Files\Bitwig Studio\Bitwig Studio.exe'
}
$hosts.GetEnumerator() | ForEach-Object {
  [pscustomobject]@{ Host = $_.Key; Exists = Test-Path -LiteralPath $_.Value; Path = $_.Value }
} | Format-Table -AutoSize
```

Expected result: Reaper is installed; any missing host remains `BLOCKED - host not installed at checked path`.

- [ ] **Step 2: Validate Reaper interactive matrix cells**

In Reaper, validate these cells against the current bundle:

```text
scan/load
pass-through
editor layout
capture controls
generation UI state
WAV Save As
WAV drag/drop
MIDI gated unavailable state
stem gated unavailable state
preset browsing
```

Use this result rule:

```text
PASS means observed in the current validation run.
FAIL means observed broken with exact reproduction details.
BLOCKED means not runnable in the current validation environment with exact reason.
```

- [ ] **Step 3: Validate every installed non-Reaper host**

For each installed host from Step 1, validate the same matrix cells. Use host-specific notes for:

```text
external drag insertion location
file import prompts
project media copy policy
browser, arranger, pool, launcher, or playlist placement
```

If a host is missing, leave all cells as:

```text
BLOCKED - host not installed at checked path
```

- [ ] **Step 4: Update the compatibility matrix**

Update the top table in `ACE-Step-Plugin/docs/host-compatibility-matrix.md` so each cell says one of:

```text
PASS - observed in current post-merge validation run
FAIL - scan/load failed with exact host error recorded below
BLOCKED - host not installed at checked path
BLOCKED - no generated WAV asset available in this validation run
Pending manual validation
```

Do not leave `Pending manual validation` for any cell that was actually exercised.

- [ ] **Step 5: Check tasks `11.3` and `12.4` only if every required host is complete or scope changes**

If all supported hosts are validated successfully, replace the task lines with:

```markdown
- [x] 11.3 Verify scan/load, pass-through, editor layout, capture controls, generation UI state, WAV export, MIDI export when available, stem export, and preset browsing across Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig.
  Evidence: all supported DAW matrix cells passed for the current post-merge bundle. Results are recorded in `ACE-Step-Plugin\docs\host-compatibility-matrix.md`.
- [x] 12.4 Verify external drag-and-drop in Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig, with Save As as fallback.
  Evidence: external drag/drop and Save As fallback behavior passed across the supported DAW matrix. Results and host-owned differences are recorded in `ACE-Step-Plugin\docs\host-compatibility-matrix.md`.
```

If any required host is missing, failed, or blocked, leave both tasks unchecked and add blocker evidence under each task.

- [ ] **Step 6: Commit DAW matrix evidence**

Run:

```powershell
git diff --check -- ACE-Step-Plugin/docs/host-compatibility-matrix.md openspec/changes/add-juce-vst3-plugin/tasks.md
git add ACE-Step-Plugin/docs/host-compatibility-matrix.md openspec/changes/add-juce-vst3-plugin/tasks.md
git commit -m "docs(plugin): record DAW matrix validation evidence"
```

Expected result: one docs-only commit.

---

### Task 4: Complete Task 12.5 Error-State Evidence

**Files:**
- Modify: `ACE-Step-Plugin/docs/host-compatibility-matrix.md`
- Modify: `openspec/changes/add-juce-vst3-plugin/tasks.md`

- [ ] **Step 1: Prepare a safe copied model root**

Run:

```powershell
$source = Join-Path $env:LOCALAPPDATA 'AceStepPlugin\models'
$target = Join-Path $env:TEMP 'AceStepPlugin-model-validation-copy'
if (Test-Path -LiteralPath $target) {
  Remove-Item -LiteralPath $target -Recurse -Force
}
Copy-Item -LiteralPath $source -Destination $target -Recurse
Get-ChildItem -LiteralPath $target | Select-Object Name,Length | Format-Table -AutoSize
```

Expected result: the copied root contains:

```text
acestep-5Hz-lm-4B-Q5_K_M.gguf
acestep-v15-turbo-Q5_K_M.gguf
Qwen3-Embedding-0.6B-Q8_0.gguf
vae-BF16.gguf
```

- [ ] **Step 2: Validate missing-model behavior**

In the copied model root, rename one model file:

```powershell
Rename-Item -LiteralPath "$env:TEMP\AceStepPlugin-model-validation-copy\vae-BF16.gguf" -NewName "vae-BF16.gguf.missing-test"
```

Run the plugin with the copied model root only if the UI or environment supports selecting that alternate root. Record one result:

```text
Missing model: PASS - editor reports missing model and host remains stable.
Missing model: BLOCKED - plugin does not support selecting an alternate model root for safe missing-model validation.
Missing model: FAIL - host crashed or editor failed to surface the error.
```

Restore the copied file:

```powershell
Rename-Item -LiteralPath "$env:TEMP\AceStepPlugin-model-validation-copy\vae-BF16.gguf.missing-test" -NewName "vae-BF16.gguf"
```

- [ ] **Step 3: Validate checksum mismatch behavior**

Corrupt only the copied model file:

```powershell
Add-Content -LiteralPath "$env:TEMP\AceStepPlugin-model-validation-copy\vae-BF16.gguf" -Value "checksum-mismatch-validation"
```

Run the plugin with the copied model root only if the UI or environment supports selecting that alternate root. Record one result:

```text
Checksum mismatch: PASS - editor reports checksum mismatch and host remains stable.
Checksum mismatch: BLOCKED - plugin does not support selecting an alternate model root for safe checksum validation.
Checksum mismatch: FAIL - host crashed or editor failed to surface the error.
```

- [ ] **Step 4: Validate backend-load failure behavior**

Copy the VST3 bundle to a temporary validation bundle and remove one backend DLL only from the copy:

```powershell
$bundleCopy = Join-Path $env:TEMP 'ACE-Step-backend-failure-test.vst3'
if (Test-Path -LiteralPath $bundleCopy) {
  Remove-Item -LiteralPath $bundleCopy -Recurse -Force
}
Copy-Item -LiteralPath 'C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3' -Destination $bundleCopy -Recurse
Remove-Item -LiteralPath (Join-Path $bundleCopy 'Contents\x86_64-win\ggml-cuda.dll') -Force
```

Load the copied bundle in Reaper or AudioPluginHost. Record one result:

```text
Backend-load failure: PASS - editor reports backend-load failure and host remains stable.
Backend-load failure: FAIL - host crashed or editor failed to surface the error.
```

- [ ] **Step 5: Validate cancellation and generation failure**

Run a normal generation with valid copied models, then cancel it from the editor. Run an intentionally invalid generation request using unsupported inputs only if the UI exposes such a path without code changes. Record:

```text
Cancellation: PASS - editor returns to stable non-generating state and host remains stable.
Generation failure: PASS - editor reports generation failure and host remains stable.
Generation failure: BLOCKED - no safe user-facing invalid request path exists without changing production code.
```

- [ ] **Step 6: Validate MIDI, stem, and preset error states**

Use the current editor UI:

```text
MIDI unavailable state: PASS when MIDI export is visibly unavailable for assets without reliable MIDI event data.
Stem unavailable state: PASS when stem export is visibly unavailable until backend stem metadata/output support exists.
Preset load failure: PASS when loading an invalid preset JSON reports an error without starting generation or crashing the host.
```

Create an invalid preset file under the plugin preset directory only after backing up the directory. If the preset path cannot be found from the UI or docs, record:

```text
Preset load failure: BLOCKED - preset storage path was not discoverable from committed docs or UI in this validation run.
```

- [ ] **Step 7: Validate out-of-memory only with a safe bounded trigger**

Do not attempt an unbounded OOM crash. Record one result:

```text
Out-of-memory: PASS - a safe bounded memory failure path surfaced in the editor and host remained stable.
Out-of-memory: BLOCKED - no safe bounded OOM trigger was available in this shared Windows validation environment.
```

- [ ] **Step 8: Add an error-state validation section**

Append this section to `ACE-Step-Plugin/docs/host-compatibility-matrix.md`:

```markdown
## Error-state validation record

| Error state | Result |
|---|---|
| Missing model | BLOCKED - copied-model validation result has not been recorded. |
| Checksum mismatch | BLOCKED - copied-model validation result has not been recorded. |
| Out-of-memory | BLOCKED - no safe bounded OOM trigger was available in this shared Windows validation environment. |
| Backend-load failure | BLOCKED - copied-bundle validation result has not been recorded. |
| Cancellation | BLOCKED - cancellation validation result has not been recorded. |
| Generation failure | BLOCKED - generation-failure validation result has not been recorded. |
| MIDI unavailable state | BLOCKED - unavailable-state validation result has not been recorded. |
| Stem unavailable state | BLOCKED - unavailable-state validation result has not been recorded. |
| Preset load failure | BLOCKED - preset failure validation result has not been recorded. |
| Host compatibility error | BLOCKED - host compatibility error validation result has not been recorded. |

Task 12.5 remains unchecked unless all non-blocked states pass and every blocked state is explicitly accepted by the OpenSpec scope.
```

- [ ] **Step 9: Check task `12.5` only if evidence and scope support it**

If all required states pass or the OpenSpec scope explicitly accepts blocked unsafe cases, replace task `12.5` with:

```markdown
- [x] 12.5 Verify missing model, checksum mismatch, out-of-memory, backend-load failure, cancellation, generation failure, MIDI unavailable state, stem failure, preset load failure, and host compatibility errors surface in the editor without crashing the host.
  Evidence: current post-merge error-state validation passed for all non-blocked states, and any blocked unsafe cases were explicitly accepted by scope. Results are recorded in `ACE-Step-Plugin\docs\host-compatibility-matrix.md`.
```

If any required state is failed or blocked without scope acceptance, leave task `12.5` unchecked and record the blocker.

- [ ] **Step 10: Commit error-state evidence**

Run:

```powershell
git diff --check -- ACE-Step-Plugin/docs/host-compatibility-matrix.md openspec/changes/add-juce-vst3-plugin/tasks.md
git add ACE-Step-Plugin/docs/host-compatibility-matrix.md openspec/changes/add-juce-vst3-plugin/tasks.md
git commit -m "docs(plugin): record error-state validation evidence"
```

Expected result: one docs-only commit.

---

### Task 5: Finalize Remaining Development Handoff

**Files:**
- Create: `docs/superpowers/handoffs/handoff-2026-05-07-add-juce-vst3-plugin-remaining-validation.md`
- Modify: `openspec/changes/add-juce-vst3-plugin/tasks.md`

- [ ] **Step 1: Run final task ledger check**

Run:

```powershell
Select-String -Path openspec\changes\add-juce-vst3-plugin\tasks.md -Pattern '^- \[ \]'
git status --short --branch
```

Expected result: either no unchecked tasks remain, or each unchecked task has an adjacent evidence or blocker paragraph.

- [ ] **Step 2: Create the handoff**

Create `docs/superpowers/handoffs/handoff-2026-05-07-add-juce-vst3-plugin-remaining-validation.md` with this command, then edit the task and blocker lines to match the evidence gathered in Tasks 2 through 4:

```powershell
$branch = git rev-parse --abbrev-ref HEAD
$head = git rev-parse --short=12 HEAD
$status = git status --short --branch
@"
# ACE-Step VST3 Remaining Validation Handoff

## Repo State

- Branch: `$branch`
- Head: `$head`
- Working tree:
````text
$status
````

## Completed Evidence

- Task 2.7: BLOCKED - edit this line after host pass-through validation.
- Task 11.3: BLOCKED - edit this line after DAW matrix validation.
- Task 12.4: BLOCKED - edit this line after drag/drop validation.
- Task 12.5: BLOCKED - edit this line after error-state validation.

## Remaining Blockers

- Host availability: FL Studio, Cubase, Studio One, Ableton Live, and Bitwig are blocked until installed at checked paths or scope changes.
- AudioPluginHost pass-through: blocked until level-matched pass-through is measured.
- Reaper current host rerun: blocked until current post-merge Reaper scan/load and pass-through are measured.
- Error-state validation: blocked until safe copied-model and copied-bundle validation is executed.

## Commands Run

````powershell
git status --short --branch
git log --oneline -5
````

## Next Action

Continue only from the first unchecked OpenSpec task with a recorded blocker.
"@ | Set-Content -Path docs\superpowers\handoffs\handoff-2026-05-07-add-juce-vst3-plugin-remaining-validation.md -Encoding UTF8
```

- [ ] **Step 3: Validate documentation formatting**

Run:

```powershell
git diff --check -- docs/superpowers/handoffs openspec/changes/add-juce-vst3-plugin/tasks.md ACE-Step-Plugin/docs
```

Expected result: no whitespace errors.

- [ ] **Step 4: Commit the handoff**

Run:

```powershell
git add docs/superpowers/handoffs openspec/changes/add-juce-vst3-plugin/tasks.md ACE-Step-Plugin/docs
git commit -m "docs(plugin): hand off remaining validation state"
```

Expected result: one docs-only commit.

---

## Self-Review

- Spec coverage: task `2.7` maps to Task 2; tasks `11.3` and `12.4` map to Task 3; task `12.5` maps to Task 4; post-merge baseline and handoff map to Tasks 1 and 5.
- Placeholder scan: this plan avoids deferred markers and does not instruct implementers to add unspecified tests or error handling.
- Type consistency: this is a docs and validation plan. The same task IDs, bundle path, host paths, and evidence files are used throughout.
- Scope control: production code remains out of scope unless validation exposes a real bug, at which point this plan explicitly stops.
