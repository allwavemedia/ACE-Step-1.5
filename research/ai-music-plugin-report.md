# Validation Strategy for a Windows JUCE VST3 AI Music Generation Plugin with Local Sidecars

## Executive synthesis

The safest and highest-yield validation stack for this plugin, as of May 7, 2026, is a **four-layer stack**: Steinberg’s **Validator** and **VST3 Plug-in Test Host** for standards compliance, **JUCE AudioPluginHost** for JUCE-wrapper smoke validation, **REAPER** for tolerant real-world routing and project behaviors, and **two locally installed production hosts** chosen to maximize behavioral diversity. In practice, the strongest pair is usually **one Steinberg-family host** such as Cubase and **one non-Steinberg host** such as Studio One, Ableton Live, or FL Studio. Steinberg’s validator is designed for build-server integration and can run custom tests; the VST3 Plug-in Test Host includes plug-in unit tests; REAPER officially supports VST3 and non-destructive rendering; Cubase exposes a plug-in manager and blocklist; Studio One has a startup scanner and blocklist reset; Ableton uses an explicit VST3 system-folder activation flow; and FL Studio loads VST3 plug-ins through its wrapper, which exposes compatibility and routing options. citeturn10view0turn10view1turn32view0turn16view0turn17view1turn33view0turn17view5turn17view3turn17view6turn17view8

For **Windows installation during validation**, the most reliable rule is: **place the validation build in the global system VST3 folder** rather than relying on the newer per-user VST3 location or on symlinks. Steinberg documents both the user and global scan locations and also notes that some DAWs may not scan the newer user location or may fail to resolve symbolic links; Ableton and Studio One both document use of the dedicated VST3 system folder on Windows. That means a build that “works in the validator” can still fail host discovery unless it is copied into `C:\Program Files\Common Files\VST3` for host validation. citeturn20view0turn14view3turn20view1turn13view4turn17view5turn17view7

The single most important design constraint in your brief is also the most important validation rule: **never run destructive tests against the only copy of the model files**. Microsoft’s documentation gives you all the primitives needed to enforce that rule safely on Windows: **Get-FileHash** for a SHA-256 manifest, **robocopy** for controlled copies with metadata, **icacls** for permission save/restore, and **VHDX** for disposable isolated volumes, including **differencing disks** for large model sets. For file commits, JUCE and Windows both provide safe replacement patterns: JUCE’s **TemporaryFile** and **File::replaceWithData/replaceWithText** explicitly avoid harming the original target if the write fails, and Windows **ReplaceFile** can atomically swap a file while also creating a backup of the original. citeturn12view15turn35view2turn36view2turn13view0turn11view5turn26view0turn34view0

For the sidecar architecture specifically, JUCE’s **ChildProcessCoordinator** and **ChildProcessWorker** give you a documented coordinator/worker pattern with message passing, ping-based liveness detection, and a `handleConnectionLost()` callback when the worker dies or disconnects. That makes it possible to write **real** crash and disconnect validation rather than merely mocking failures. JUCE also documents that `setProcessing()` can be called from the real-time audio thread and must be lock-free and avoid allocation, while `AudioProcessorValueTreeState::copyState()` is thread-safe but not real-time safe. Taken together, that means your validation strategy should explicitly prove that generation, exports, downloads, and sidecar restart logic never stall the audio thread or perform state serialization from the callback path. citeturn31view0turn31view1turn31view3turn10view2turn27view0

## Host validation checklist

The checklist below is organized around the VST3 and JUCE behaviors that hosts actually exercise: discovery in the correct VST3 path, instantiate/load, editor lifecycle, pass-through and processing stability, state/preset round-trips, and recovery after external process failure. VST3 hosts save and load plug-in state via `getState`/`setState`; JUCE exposes that through `AudioProcessor::getStateInformation()` and `setStateInformation()`; VST3 preset files use the `.vstpreset` format and are host-managed. citeturn10view3turn39view0turn29view0turn29view1

### Core checklist to run in every host

| Area | What to validate | Pass criteria |
|---|---|---|
| Scan and discovery | Plugin appears after rescan from the global VST3 folder; no duplicate ghost entries; no silent disappear/reappear across rescans | Plugin is discoverable consistently across two consecutive rescans |
| Load and instantiate | Insert on empty track; insert on armed track; duplicate instance; remove and reinsert; save project; reopen project | No crash, no blocklist placement unless intentionally faulted, no orphaned sidecar |
| Editor lifecycle | Open/close editor repeatedly; dock/undock if host supports it; reopen after host transport changes; check keyboard focus | Editor remains responsive; no blank UI; no focus traps; no leaked windows |
| DPI and resize | Test 100%, 150%, 200% Windows scaling; host resize if enabled; min/max size limits | Correct scaling, no clipped controls, no resize glitches |
| Idle pass-through | With generation idle, verify silence pass-through or intended dry behavior; confirm no spurious audio/MIDI output | No unintended audio, MIDI, CPU spikes, or latency changes while idle |
| Playback stability | Start/stop transport while editor open and closed; change sample rate and block size between sessions | Instance stays stable and reloads correctly |
| Prompt-to-WAV generation | Generate while stopped; generate during playback; queue second request while first is active; retry after success | Host remains stable; sidecar status visible; final WAV valid and complete |
| Reference capture | Capture from real input and from a file-based source path; cancel mid-capture; retry after cancel | Input captured correctly or canceled cleanly with no stale capture state |
| WAV Save As | Save generated file to writable folder, long-path folder, and existing filename prompt flow | Saved file opens correctly; no partial file promoted as final |
| Drag and drop | Drag generated WAV to desktop and, where supported, into DAW/browser or OS target | Drag operation yields the expected final file or degrades gracefully |
| MIDI export | Export MIDI after successful generation; re-import into host; compare note count/basic timing sanity | Valid MIDI file produced and importable |
| Stem export | Export stems; verify expected stem count, naming, and channel format; re-import stems | Every expected stem is valid and synchronized |
| Presets and state | Save/load host preset; save/load project state; duplicate track with instance; reopen session; test default preset recall | Round-trip state is deterministic and does not corrupt generation settings |
| Multi-instance behavior | Run two or more instances with different prompts/models/output destinations | No state bleed, no shared-output collision, sidecars mapped correctly |
| Recovery | Force sidecar loss; reload preset after failure; reopen project after failed generation | Host survives; plugin reports actionable state; retry path works |

JUCE documents that the host may call `setScaleFactor()` and may resize the editor when `setResizable()` or resize limits are enabled, so scaling and resize are not cosmetic-only checks; they are part of host compatibility. JUCE also documents safe, thread-aware state serialization behavior through `AudioProcessorValueTreeState`, which is precisely why preset/state round-trip and recovery need to be validated outside the audio callback path. citeturn28view0turn28view3turn27view0

### Host-specific additions

**Steinberg layer:** run the plugin through both the **Validator** and **VST3 Plug-in Test Host** on every candidate build. In Cubase, confirm visibility in the **VST Plug-in Manager** and verify behavior when intentionally blocklisted, because Cubase explicitly distinguishes loaded plug-ins from plug-ins withheld for stability reasons. citeturn10view0turn10view1turn17view1turn33view0

**REAPER:** treat REAPER as the “routing and persistence stress host.” REAPER officially supports VST3, multichannel routing, and non-destructive rendering, so it is a good place to validate track duplication, reload behavior, render/export interaction, and multi-instance sidecar behavior under project save/load. citeturn16view0turn16view1

**Studio One:** explicitly validate startup scan, the default VST3 path, and **Reset blocklist** handling. Studio One documents both the default VST3 location and the blocklist reset path, so it is ideal for “discovery after failure” tests. citeturn17view3turn17view5turn17view2

**Ableton Live:** explicitly validate VST3 source activation and that presets/libraries are not co-mingled with the VST3 folder. Ableton documents both the dedicated VST3 system folder and the requirement that only valid plug-ins live there, which matters when your plugin also manages large models and generated artifacts. citeturn17view6turn17view7

**FL Studio:** validate wrapper-hosted behavior, especially editor windowing, drag/drop behavior, and routing. FL Studio documents that VST3 plug-ins are loaded in the Fruity Wrapper, which is exactly the sort of host layer that can expose UI and compatibility edge cases missed elsewhere. citeturn13view5turn17view8

## Destructive test matrix

All destructive cases below should run **only against a disposable workspace**: either a copied model/output sandbox or a mounted child VHDX. Use **SHA-256 manifests** before and after each run, **ACL save/restore** around permission tests, and **small VHDX volumes** plus filler files for disk-full tests. Windows identifies access-denied as error **5** and disk-full as error **112**, so your logs should preserve those raw codes alongside user-facing messages. Sidecar crash injection can use a test-only crash RPC if you have one, but an external kill via **Stop-Process** or **taskkill** is also a real failure injection. citeturn12view15turn35view2turn36view2turn24view3turn24view1turn12view0turn25view0turn38view0turn38view1

| Case | Real failure injection | Expected result | Required evidence |
|---|---|---|---|
| Missing models | Delete or rename the required model file or model directory **in the disposable workspace only** | Plugin detects absence before inference, keeps host stable, shows actionable remediation, does not mutate baseline | Before/after hash manifest, plugin log, sidecar log, screenshot |
| Wrong-size models | Truncate cloned model file so name/path remain correct but size is wrong | Loader rejects model as invalid; no crash; no fallback to undefined behavior | File size proof, hash delta, loader error log |
| Corrupted models | Flip bytes in header and in a later chunk of the cloned file | Corruption is detected predictably; current good model remains unaffected | Binary diff note, loader error, no baseline drift |
| Failed downloads | Kill download process, sever test network path, or terminate downloader sidecar mid-transfer | Partial download remains quarantined; no promotion into active model slot | Temp-file listing, final slot unchanged, logs |
| Checksum mismatch | Complete download successfully, then tamper with staged file or manifest before verification | Install is rejected after transfer; old active model remains live | Expected vs actual SHA-256, status message, active-model verification |
| Backend load failure | Remove required runtime/backend DLL from sidecar sandbox or point backend selector to an invalid target in test build | Sidecar reports backend initialization failure without host crash | Missing dependency proof, sidecar stderr/stdout, host/plugin logs |
| Sidecar launch failure | Configure sidecar executable path to nonexistent file or remove executable from sandbox | Plugin reports sidecar unavailable; host remains stable; retry after fix succeeds | Launch command evidence, error text, retry proof |
| Sidecar crash before generation | Start sidecar, then force-stop it before a job starts | Plugin transitions to disconnected/ready-to-retry state; no hung UI | Process stop proof, `handleConnectionLost`-style log, UI screenshot |
| Sidecar crash during generation | Start generation, then force-stop sidecar PID | In-flight job fails cleanly; no partial final export promoted; retry path works | PID kill proof, partial temp artifact listing, retry success |
| User cancellation | Cancel download, generation, capture, MIDI export, and stem export from the real UI | Operation aborts predictably; temp artifacts cleaned or quarantined; plugin remains reusable | Screen recording, temp/output folder contents, logs |
| Generation failure | Use test-only backend error route, invalid internal precondition, or force sidecar failure after job acceptance | Job fails with explicit state and no deadlock; host transport unaffected | Job ID, state transition log, host playback continuity |
| MIDI export failure | Deny write access to MIDI target or force disk full on MIDI destination | No corrupt/zero-byte MIDI promoted to final target; plugin remains usable | Folder ACL evidence or VHDX space proof, output folder snapshot |
| Stem export failure | Fill destination after first stem, or deny write on subsequent stem names | Either all stems are atomic as a set, or partial stems are clearly quarantined and surfaced as failure | Stem count before/after, naming list, logs |
| Preset corruption | Truncate or mangle `.vstpreset` and any plugin-specific preset/state blob | Load is rejected safely; current instance state is preserved until validation succeeds | Corrupted preset sample, load log, state-before/state-after screenshot |
| Disk full | Run output/model staging on small VHDX and consume remaining space with a filler file | Final write fails with no silent truncation; active model/output from previous good state survives | Volume size, filler file proof, raw Windows error 112 |
| Permission denied | Use `icacls /deny` on target directory/file after saving ACLs for restoration | Operation fails with clear error; no hidden fallback to another directory | ACL save file, deny command/output, raw Windows error 5 |
| Preset/state compatibility after failure | Save project/preset after a failed destructive case, then reload | Reloaded instance is sane, or failure state is explicit and recoverable | Saved project, reload screenshot, logs |

### Destructive-case acceptance criteria

A destructive test should be considered **passing** only if **all** of the following are true:

1. The **host does not crash or hang**.
2. The plugin surfaces a **specific, user-actionable state** rather than generic failure.
3. The **current known-good model** remains usable if the failing action involved model setup, download, validation, or replacement.
4. No **partial final artifact** is promoted into the user-visible final path.
5. The plugin can either **retry immediately** or can recover after editor reopen/project reopen/sidecar restart without reinstantiation of the entire host session.

Those rules are not just process niceties; they are aligned with the way VST3 hosts persist state and with JUCE’s safe file-replacement patterns. VST3 preset/state is host-mediated, and JUCE’s safe replacement APIs exist specifically to reduce the chance of corrupted or unfinished files after failure. citeturn10view3turn39view0turn26view0turn11view5

## Safe backup and restore process

The best practical process is a **baseline–workspace–promotion** model.

### Baseline rule

Create one **immutable baseline model store** outside all normal validation paths. Then generate a manifest of every model file with **relative path, byte size, and SHA-256**. Use **Get-FileHash** to compute hashes and store the manifest with the plugin build ID and model pack version. Never point destructive tests at this baseline directly. citeturn12view15

### Workspace rule

For each test run, create a **fresh disposable workspace** containing:

- the candidate plugin build,
- a disposable model copy or mounted child VHDX,
- a run-specific output folder,
- a dedicated sidecar working directory,
- per-run logs.

For ordinary-sized model sets, a metadata-preserving copy with **robocopy** is the simple choice. For very large model sets, use **differencing VHDX** so each destructive case gets a copy-on-write layer instead of a full duplicate. Microsoft documents both `robocopy` copy semantics and `New-VHD -Differencing`. citeturn12view16turn35view2turn13view0

### Permission test rule

Before any permission-denied case, save ACLs with **`icacls /save`**, apply the deny, run the case, then restore with **`/restore`**. This turns permission testing from a risky manual exercise into a reversible, auditable action. citeturn36view2turn36view0

### Disk-full rule

For disk-full validation, place the candidate model staging area or export target on a **small attached VHDX**, then consume remaining space using a filler file created with **`fsutil file createnew`**. This produces a real Windows disk-full path rather than a simulated exception. Mounting/attaching VHDs is also officially supported in Windows admin tooling. citeturn24view1turn24view3turn24view2

### Promotion rule for models and outputs

Never write directly into the active model slot or final output file path. Instead:

1. Write into a sibling temporary file or staging path.
2. Flush the stream.
3. Verify file size and SHA-256 if applicable.
4. Atomically promote into the final location.
5. Optionally create a backup when replacing an existing final file.

JUCE’s **TemporaryFile**, **File::replaceWithData**, and **File::replaceWithText** are explicitly designed to avoid harming the original if the write fails, and `FileOutputStream::flush()` ensures buffered data is written before commit. On Windows, **ReplaceFile** can replace a file while also creating a backup copy of the original. citeturn11view5turn11view4turn26view0turn26view3turn11view6turn34view0

### Restore rule

After each destructive run:

- detach/delete the workspace VHDX or delete the copied workspace,
- restore ACLs if they were changed,
- recompute the baseline manifest for spot-check or full verification,
- confirm the current production model store hash still matches the original manifest.

If the active production slot ever differs from the baseline unexpectedly, treat that as a **validation failure of the harness itself**, not of the plugin. citeturn12view15turn36view2

### Sample run structure

```text
validation/
  2026-05-07/
    build-1.4.0+sha.abc123/
      manifests/
        baseline-models.sha256.csv
        post-run-models.sha256.csv
      hosts/
        reaper/
          TC-MODEL-CHK-001/
            logs/
            screenshots/
            sidecar/
            outputs/
        cubase/
        studio-one/
```

This structure is a recommendation, but it directly supports the artifact separation and auditability that OpenSpec and the Windows/JUCE primitives make possible. citeturn22view0turn22view2

## Automated and manual validation split

The cleanest split is:

### Automate

Automate everything that is **deterministic, machine-verifiable, and does not depend on subjective host UI affordances**.

That includes:

- Steinberg **Validator** execution on every CI build.
- Custom validator tests for instantiate/load/state round-trip if you expose validator test hooks.
- Sidecar handshake, ping timeout, launch failure, disconnect, and restart contract tests.
- Model staging tests: missing file, wrong size, corrupted bytes, checksum mismatch, canceled download.
- File-system fault tests: disk full, permission denied, long path, existing file replacement.
- Export integrity: WAV header validity, MIDI file readability, expected stem count and naming.
- Baseline protection checks: manifest equality before/after destructive runs.

Steinberg explicitly supports validator use in automated build pipelines and allows you to plug in your own tests; the VST3 Plug-in Test Host adds another standards-oriented validation surface. citeturn10view0turn10view1turn32view0turn14view2

### Keep manual

Keep manual everything that is **host-UI dependent, workflow dependent, or partially subjective**.

That includes:

- whether the plugin is discoverable in each DAW’s actual scan UI,
- editor behavior under real docking/focus/DPI conditions,
- drag/drop behavior into OS and DAW targets,
- reference capture using real audio routing,
- generation during transport changes and project interaction,
- user-facing failure text and recovery affordances,
- long-session behavior with multiple plugin instances.

This split is especially important because **VST compliance is not the same thing as DAW interoperability**. Steinberg’s own FAQ notes that a plugin may be visible in the VST3 Plug-in Test Host but still not appear in a DAW because of scan-location or symlink behavior. citeturn14view1

### Recommended release gate

A candidate build should not ship until it has:

- passed **CI automation**,
- passed the **destructive matrix** on an isolated workspace,
- passed the **manual host checklist** in JUCE AudioPluginHost, REAPER, and two local production hosts,
- produced a complete evidence package.

## Evidence template for docs and OpenSpec

OpenSpec’s stable value here is not the exact slash-command alias, but the **artifact model**: each change carries a **proposal**, **design**, **tasks**, and **spec deltas**, and verification checks **completeness, correctness, and coherence** before archival. That maps very well to validation evidence for a plugin with host variance, file staging, and sidecar failures. citeturn22view0turn22view1turn30view0

### Evidence record template

```md
# Validation Evidence Record

## Header
- Case ID:
- Capability:
- Build version:
- Plugin binary hash:
- Sidecar build hash:
- Date/time:
- Tester:
- Host:
- Host version:
- OS build:
- Audio device / driver:
- Sample rate / buffer size:
- VST3 install path:
- Model baseline manifest ID:
- Workspace path or VHDX path:

## Preconditions
- Baseline manifest verified: Yes/No
- ACL snapshot captured: Yes/No
- Output folder empty or isolated: Yes/No
- Sidecar logging enabled: Yes/No

## Injection
- Injection type:
- Exact method used:
- Command or manual action:
- Real failure or simulation:
- Why this is representative:

## Steps
1.
2.
3.

## Expected result
- Host stability:
- Plugin UX:
- Sidecar state:
- File-system state:
- Recovery behavior:

## Observed result
- Host behavior:
- Plugin behavior:
- Sidecar behavior:
- Final artifact state:
- Current active model preserved: Yes/No

## Evidence
- Screenshot(s):
- Screen recording:
- Host log:
- Plugin log:
- Sidecar stdout/stderr:
- Windows event/error code:
- Output file list:
- Output hashes:
- Before/after manifest diff:

## Verdict
- Pass / Fail / Blocked
- Severity if failed:
- Defect ID:
- Notes:
```

This record format is intentionally evidence-heavy because OpenSpec verification is designed to reason about completeness, correctness, and coherence; without concrete artifacts, those checks are weak. citeturn30view0

### OpenSpec mapping

Use the following mapping for the change set that introduces or formalizes validation:

| OpenSpec artifact | What to store |
|---|---|
| `proposal.md` | Scope of validation, host matrix, destructive testing mandate, non-corruption requirement for model files |
| `design.md` | Sidecar state machine, model-store staging/promote rules, backup/restore algorithm, Windows error taxonomy, evidence capture design |
| `tasks.md` | Host checklist cases and destructive test IDs as checkbox tasks |
| `specs/<capability>/spec.md` | Normative requirements and scenarios for generation, export, presets, model setup/download, and failure recovery |
| archived change folder | Final evidence bundle links, pass/fail summary, defects discovered, approved exceptions |

OpenSpec’s generated planning artifacts are explicitly proposal, design, tasks, and specs, and `/opsx:verify` is explicitly meant to validate completeness, correctness, and coherence before archive. citeturn22view0turn22view2turn30view0

### Suggested spec-language examples

For this plugin, the most valuable normative requirements are:

- **The plugin MUST NOT mutate the only known-good model copy during setup, download, validation, or replacement.**
- **A candidate model MUST be staged and validated before promotion into the active slot.**
- **If sidecar connection is lost during an operation, the host MUST remain stable and the plugin MUST surface a recoverable state.**
- **A failed export MUST NOT produce a user-visible final artifact unless the artifact is complete and valid.**
- **A corrupted preset or state load MUST NOT overwrite the current runtime state until validation succeeds.**

Those are recommendations, but they align directly with VST3 state/preset semantics and JUCE’s safe file replacement patterns. citeturn10view3turn39view0turn26view0turn11view5

## Risk mitigations

The highest-value mitigations are the following.

**Use the global VST3 system folder for validation builds.** This removes a large class of false negatives caused by hosts that do not scan the user VST3 location or do not resolve symlinks reliably. citeturn20view0turn14view3turn13view4turn17view5

**Adopt a two-slot or three-slot model-store policy.** Keep a known-good active slot, stage candidates separately, and never replace active until validation succeeds. If storage allows, keep a previous-known-good backup as well. JUCE and Windows both provide safe replacement primitives that make this practical. citeturn11view5turn26view0turn34view0

**Treat model validation as a file-integrity operation, not just a load attempt.** Verify size and SHA-256 before promotion, and capture manifest diffs before and after every destructive run. citeturn12view15

**Isolate destructive tests physically, not just logically.** A copied workspace or differencing VHDX is safer than “promising not to touch production files.” Windows documents the necessary VHDX tooling directly. citeturn13view0turn24view3

**Log and surface native Windows error codes.** Permission errors and disk-full errors should retain their raw code paths so evidence and debugging are fast and unambiguous. Windows documents `ERROR_ACCESS_DENIED` as **5** and `ERROR_DISK_FULL` as **112**. citeturn12view0turn25view0

**Validate sidecar failure as a first-class state machine.** JUCE’s coordinator/worker model already exposes liveness timeouts and lost-connection callbacks, so your test cases should prove the correctness of those transitions instead of treating sidecar loss as an undefined exception. citeturn31view0turn31view1turn31view3

**Keep all non-audio work off the real-time path.** VST3 documents that `setProcessing()` may arrive on the real-time thread and must be lock-free with no allocations, while JUCE documents that `copyState()` is not real-time safe. Validation should therefore explicitly prove that generation, state save/load, downloads, and export orchestration do not stall pass-through audio. citeturn10view2turn27view0

**Do not let preset corruption become silent state corruption.** Because hosts persist VST3 preset/state through formal state APIs and `.vstpreset` files, negative-preset tests should be release-blocking if malformed loads can poison runtime state. citeturn10view3turn39view0turn29view0turn29view1

## Open questions and limitations

This research supports a **strong validation strategy** and identifies the official behaviors you can lean on for VST3 compliance, host discovery, sidecar orchestration, safe file handling, Windows fault injection, and OpenSpec evidence structure. What it does **not** establish from official sources is a standardized, vendor-supported **headless automation API for each production DAW**; for most teams, that means the reliable default is **CI automation for validator/integration harnesses plus manual DAW validation for the actual production hosts**. citeturn10view0turn10view1turn14view1

The practical implication is straightforward: for this plugin class, **compliance automation + isolated destructive harness + manual host acceptance** is the right split, and it is the split most consistent with the documented capabilities of Steinberg’s tools, JUCE’s process/file APIs, Windows’ admin/file-system tooling, and OpenSpec’s verification model. citeturn10view0turn31view0turn11view5turn30view0