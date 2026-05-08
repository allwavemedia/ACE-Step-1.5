# Minimal Single-Scroll VST3 UI for ACE-Step

I did not have access to internal ACE-Step design docs in this chat, so the proposal below treats your ACE-Step context as the product brief and grounds the recommendations in current public VST3/JUCE guidance and current AI music tool patterns.

The strongest architectural constraint is host reliability, not visual polish. Steinberg’s VST3 guidance notes that not all VST3 hosts support live plug-in resizing, VST3 simple plug-ins typically rely on the host for preset management, and JUCE reminds developers that an editor may be absent, closed, and recreated at any time, so the processor must not depend on the editor existing. That combination strongly favors a fixed initial editor size, a single vertical scroll layout, and processor-owned async job state rather than editor-owned state. citeturn18view0turn7view0turn13view0

Current AI music tools converge on a compact core generation loop: text prompt, optional input audio/reference, model selection, output length, and a single Create/Generate action. Stable Audio explicitly describes text prompt and input audio as the two core inputs of its interface, with model selection, input strength, prompt strength, seed, and generate as supporting controls. Udio’s beginner flow is even slimmer: describe the song, choose a length, and click Create. Prompt guidance in both ecosystems emphasizes genre, mood, tempo/BPM, and instrumentation as the most useful scaffold for the prompt itself. citeturn4view5turn4view6turn4view10turn12view5turn23view0turn23view1

The output side of the workflow is also clear in current tools. Stable Audio’s preview panel centers playback, reuse, and download actions; Udio Sessions emphasizes takes/snapshots; and Suno exposes take lanes, saved versions, multitrack export, stem extraction, and MIDI extraction from stems. That pattern supports a recovery-tier ACE-Step UI where the result history is not a side feature but the workspace’s second primary area, immediately below generation. citeturn11view2turn12view0turn12view2turn4view7turn4view8

For long-running operations, the UI should prefer inline progress with clear text and cancellation, and optionally mirror that progress to the host when supported. Steinberg’s `IProgress` exists specifically so plug-ins can report UI-triggered background tasks to the host, while Microsoft’s UI guidance recommends determinate progress when duration is known and textual explanation alongside the indicator. Steinberg also recommends requesting the host to open the editor instead of showing blocking alerts during load flows. citeturn21view0turn21view1turn6view3turn7view1

## Research basis

The best recovery UI for ACE-Step is therefore **one fixed-size, single-column, single-scroll editor** with three priorities in this order: configure generation, watch/cancel work, and manage generated assets. It should avoid tabs, wizard steps, and resizable multi-pane layouts in the first pass, because those increase host-surface complexity without adding much functional value for the first working release. That recommendation follows directly from the host/editor lifecycle constraints above and from the fact that current AI music tools already prove that prompt-plus-reference workflows can be productive without a full DAW-style canvas. citeturn18view0turn13view0turn4view5turn4view10

It is also important to separate **UI commands** from **host-automatable parameters**. JUCE documents that hosts may call parameter changes at any time, including during audio processing, and that implementations must be efficient and avoid locking. Steinberg also requires proper parameter automation cooperation, and JUCE provides discrete/boolean metadata specifically so hosts can show stepped values correctly. That means operations like **Generate**, **Cancel**, **Save As**, **Download Model**, **Export MIDI**, and **Export Stems** should remain explicit UI commands, not automatable VST parameters. citeturn13view1turn15view0turn4view1

## Minimum control set

For the first recovery UI, I would keep the always-visible surface area deliberately small. The required controls are those needed to complete one full loop from idea to file: enter prompt, set length, optionally capture/import reference audio, confirm the model is ready, start generation, watch/cancel progress, preview the result, and export it. Controls like seed and prompt-guidance strength are still useful, but they belong in a small collapsed **Advanced** row rather than the main surface, because current tools treat them as supporting controls rather than the primary entry point. citeturn4view5turn4view6turn4view10turn23view0

| Area | Control | Priority | Recovery-tier behavior |
|---|---|---:|---|
| Global header | **Preset dropdown** | Required | Load factory/user presets for generation settings |
| Global header | **Save preset** | Required | Save current form as user preset; show dirty state when edited |
| Global header | **Model selector + status pill** | Required | Shows Ready / Missing / Downloading / Error |
| Generation | **Prompt text area** | Required | Multiline; placeholder suggests genre, mood, instruments, BPM |
| Generation | **Duration dropdown** | Required | Simple small set such as 8s / 16s / 32s / 60s / 120s |
| Generation | **Generate** | Required | Primary CTA; enabled only when model is ready and prompt is valid |
| Generation | **Cancel** | Required | Appears only during active jobs; same location as Generate secondary action |
| Reference | **Capture from input** | Required | Record/capture reference audio from host input path |
| Reference | **Import / drop audio** | Required | File dialog plus drag/drop target |
| Reference | **Reference preview strip** | Required when occupied | Mini waveform, name, duration, Replace, Remove |
| Reference | **Reference influence slider** | Conditional | Visible only when a reference exists |
| Progress | **Progress bar + stage text** | Required | Determinate when possible; indeterminate with stage text otherwise |
| History | **Result tiles** | Required | Latest-first generated asset history within same scroll |
| Result tile | **Play / Stop preview** | Required | Tile-local lightweight preview |
| Result tile | **Save As WAV…** | Required | Never hidden behind a secondary screen |
| Result tile | **Drag WAV** | Required | Convenience action; not sole export path |
| Result tile | **MIDI…** | Required | Direct export if ready; otherwise launch extraction job |
| Result tile | **Stems…** | Required | Direct export if ready; otherwise launch extraction job |
| Result tile | **Use as Reference** | Recommended | Fast iteration control, borrowed from current AI workflows |
| Advanced | **Seed lock / seed value** | Recommended | Hidden under disclosure |
| Advanced | **Prompt guidance / prompt strength** | Recommended | Hidden under disclosure; especially useful for reference workflows |
| Diagnostics | **Copy diagnostics** | Required | One-click support bundle text |
| Diagnostics | **Open log folder / reveal cache** | Required | Fast recovery path for failures |

Deliberately **excluded** from the first recovery UI: timeline editing, lyric editor, tag auto-complete chips, embedded stem mixer, elaborate browser/search views, collaborative sharing, account/billing surfaces, and any mandatory pop-out panels. Those are good later-layer features, but they are not needed to make ACE-Step functionally complete in the first single-scroll pass. citeturn12view0turn12view2

## Text wireframe

The safest recovery layout is a **fixed initial editor size** with vertical scrolling only, no horizontal scrolling, and no dependency on live resize. A practical starting target would be something in the neighborhood of **760 × 920 px**, content-scaled by the host/OS, with the understanding that some hosts may not support live resizing consistently. citeturn18view0turn10view2

```text
┌──────────────────────────────────────────────────────────────────────────────┐
│ ACE-Step                                                [Preset: Init ▼] [Save…] │
│ [Model: Medium v1 ▼ | Ready]                Status: Idle                     │
└──────────────────────────────────────────────────────────────────────────────┘

Generation
┌──────────────────────────────────────────────────────────────────────────────┐
│ Prompt                                                                      │
│ [ A warm neo-soul instrumental at 92 BPM with Rhodes, dusty MPC drums,     ]│
│ [ electric bass, intimate mood, subtle tape texture                         ]│
│                                                                             │
│ Hint: genre • mood • key instruments • BPM                                 │
│                                                                             │
│ [Duration: 32s ▼]   [Advanced ▸]                              [Generate]    │
│                                                             [Cancel hidden] │
└──────────────────────────────────────────────────────────────────────────────┘

Reference audio
┌──────────────────────────────────────────────────────────────────────────────┐
│ [Capture from input]   [Import / Drop audio]   [Clear]                      │
│                                                                              │
│ Empty state: "No reference selected. Capture or drop a short WAV."          │
│                                                                              │
│ Occupied state:                                                             │
│ [mini waveform] tap_loop_01.wav • 00:12.4                 [Replace] [Remove]│
│ Reference influence [────────●──────] 55%                                   │
└──────────────────────────────────────────────────────────────────────────────┘

Active job
┌──────────────────────────────────────────────────────────────────────────────┐
│ Hidden when idle                                                             │
│ [██████████────────] 54%  Generating audio… Stage 2/4  ETA 00:28  [Cancel] │
│ Inline subtext: "Do not close project. You may close this editor."          │
└──────────────────────────────────────────────────────────────────────────────┘

Generated assets
┌──────────────────────────────────────────────────────────────────────────────┐
│ Result A   [Ready]   00:32   3:52 PM                                        │
│ [Play] [mini waveform preview.............................................] │
│ Prompt: warm neo-soul instrumental at 92 BPM…                               │
│ Meta: Model Medium v1 • Ref tap_loop_01.wav • Preset Dusty Soul             │
│ [Save As WAV…] [Drag WAV] [MIDI…] [Stems…] [Use as Reference] [⋯]           │
└──────────────────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────────────┐
│ Result B   [Extracting MIDI]   00:32   3:51 PM                               │
│ [Play] [mini waveform preview.............................................] │
│ Prompt: warm neo-soul instrumental at 92 BPM…                               │
│ Meta: Model Medium v1 • No ref • Preset Init                                │
│ [Save As WAV…] [Drag WAV] [MIDI…disabled/spinner] [Stems…] [Use as Ref] [⋯]│
└──────────────────────────────────────────────────────────────────────────────┘

Model setup
┌──────────────────────────────────────────────────────────────────────────────┐
│ Ready state:                                                                 │
│ Model path: /ACE-Step/Models/Medium-v1                                       │
│ Cache path: /ACE-Step/Cache                                                  │
│                                                                              │
│ Missing state:                                                               │
│ Medium v1 not installed                                                      │
│ Download size 3.4 GB • Estimated disk required 5.1 GB                       │
│ [Download] [Choose Folder…]                                                  │
│                                                                              │
│ Downloading state:                                                           │
│ [███████─────────] 41% • 28 MB/s • ETA 01:42                    [Cancel]     │
│                                                                              │
│ Error state:                                                                 │
│ "Insufficient disk space in selected folder."  [Choose Folder…] [Retry]     │
└──────────────────────────────────────────────────────────────────────────────┘

Diagnostics
┌──────────────────────────────────────────────────────────────────────────────┐
│ [▸ Diagnostics]                                                              │
│ Engine 0.9.x • Model checksum • Last job ID • Host name/version if present   │
│ [Copy diagnostics] [Open log folder] [Reveal cache]                          │
└──────────────────────────────────────────────────────────────────────────────┘
```

The key recovery principle in this wireframe is that **nothing important is hidden behind a mode switch**. Generate, progress/cancel, results, export, model readiness, presets, and diagnostics all remain in one scrollable continuum. That lowers both cognitive overhead and host-surface risk.

## Interaction flow and state table

The interaction flow should be straightforward. On open, ACE-Step checks model/cache status in the background and restores the last non-binary state from the processor. If the selected model is missing, the model setup block expands and **Generate** stays disabled. If the model is ready, the user enters a prompt, optionally captures or imports a reference clip, and clicks **Generate**. During generation, the form becomes read-only, progress is shown inline and optionally via VST3 `IProgress`, and **Cancel** remains available. On completion, the newest asset tile is inserted at the top of history, ready for preview and export. If the user requests stems or MIDI, that export may be a second async job, because current AI music tools often derive MIDI from stems rather than treating it as an automatically present artifact. Preset loading should swap the form state only; it should not erase the existing generated-asset history. citeturn21view1turn4view7turn4view8turn12view0turn13view0

| State | Visible cues | Allowed actions | Disabled actions | Exit condition |
|---|---|---|---|---|
| Startup checking | Header/status shows **Checking model and cache…** | Scroll history; open diagnostics | Generate; model-sensitive exports | Integrity check completes |
| Model missing | Model block expanded; status pill **Missing**; inline size/disk text | Download; choose folder; edit prompt; load preset | Generate | Download/install succeeds or user selects another ready model |
| Model downloading | Determinate bar if bytes known; **Cancel** visible | Cancel download; edit prompt | Generate; model switch | Download completes or fails |
| Idle invalid form | Empty/invalid prompt hint; Generate disabled | Type prompt; load preset; import reference | Generate | Prompt becomes valid and model is ready |
| Ready to generate | Generate enabled; status **Idle** | Generate; capture/import reference; adjust advanced settings; load/save preset | Cancel | User starts a job |
| Recording reference | Reference card shows recording timer/levels | Stop; discard | Generate; preset load; model switch | Capture stops or is discarded |
| Generating | Form locked; progress text and bar; Cancel visible | Cancel; preview older completed assets; copy diagnostics | Prompt edit; duration/model/reference changes; preset load/save | Job completes, fails, or cancellation resolves |
| Canceling | Status **Canceling…**; controls remain locked | Wait; copy diagnostics | New generate; edits to active form | Backend confirms cancellation or late result is discarded |
| Result ready | New top tile with **Ready** badge | Play; Save As WAV; Drag WAV; MIDI; Stems; Use as Reference; Generate again | N/A | User exports, reuses, or deletes |
| Exporting stems/MIDI | Tile-local spinner/badge such as **Extracting MIDI** | Cancel export if backend supports it; continue using other ready tiles | Re-trigger same export on same tile | Export completes or fails |
| Recoverable error | Inline banner attached to failing area, not modal | Retry; reveal path; copy diagnostics; choose folder/relink/remove | Only the failing command | User retries successfully or dismisses/removes |
| Missing cached asset on restore | Tile badge **Missing file** | Locate…; Remove; use diagnostics | Preview; Drag WAV; Save As on missing file | Asset is relinked or removed |
| Preset dirty | Preset bar shows unsaved marker | Save preset; save project; keep editing | N/A | Saved or reverted |

A few interaction details matter enough to make explicit:

When the editor closes during a running job, the job should continue under processor ownership and the UI should reattach cleanly when reopened, because JUCE explicitly warns that editors may be deleted and recreated independently of processor lifetime. citeturn13view0

Generated asset history should be **latest first** and behave more like a compact “takes” list than a file browser. Udio’s Sessions and Suno’s take-lane/version patterns both reinforce that users iterate by comparing recent candidates, not by navigating a deep folder tree in the critical creation loop. citeturn12view0turn12view2

## Accessibility and host-validation notes

Keyboard and assistive support should be treated as part of the recovery UI, not deferred polish. W3C guidance requires visible focus, sufficient target size or spacing, alternative methods for drag-dependent operations, and semantic status messages; JUCE’s accessibility APIs support focus control and user announcements. In practical terms, every core action in ACE-Step should be keyboard reachable in a sensible top-to-bottom order; every icon button should have an explicit text-accessible name; every drag action must have a button equivalent such as **Save As WAV…** or **Import / Drop audio** plus **Browse…**; and generation/export status changes should be surfaced as readable text and accessibility announcements such as “Generation started,” “Generation canceled,” and “MIDI export failed.” citeturn17search6turn17search3turn17search1turn17search7turn6view5

For usability, keep primary controls big and boring. The prompt box should accept plain prose; the helper text should suggest the prompt ingredients that current tools document as most productive, namely genre, mood, key instruments, and BPM. Error text should explain both **what failed** and **what to do next**, for example: “Model not found. Choose folder or download again,” not just “Error 17.” Progress should be determinate whenever bytes or steps are known; otherwise use an indeterminate animation plus a stage label. citeturn23view0turn12view5turn6view3

For host validation and DAW friendliness, I would treat the following as non-negotiable. First, keep long-running tasks out of the automatable parameter path; JUCE notes that parameter changes may arrive during audio processing and must avoid locking, so heavy commands such as model download, generate, Save As, and export should stay as UI commands. Second, expose only a small, stable set of host-meaningful parameters, and mark stepped/boolean ones correctly so generic host UIs and automation displays behave correctly. Third, support content scaling, survive editor reopen/close cycles, and avoid modal alerts on project load. Fourth, treat **Drag WAV** as a convenience only: JUCE supports external file drag, but Steinberg’s minimum host requirements do not make drag/drop a baseline requirement, so **Save As WAV…** must always remain available as the guaranteed export path. Finally, if the host supports it, right-click on any exported parameter should surface the host context menu, because Steinberg explicitly documents automation/MIDI-learn style context items as user value. citeturn13view1turn15view0turn10view2turn13view0turn7view1turn10view1turn8view0turn19search2turn19search7

Diagnostics are part of usability here, not just support engineering. A collapsed diagnostics section with **Copy diagnostics** and **Open log folder** is worth the space because `pluginval` exists specifically to validate plug-ins across platforms and its saved logs materially speed issue resolution. citeturn6view7turn6view8

## Acceptance criteria

| Area | Acceptance criterion |
|---|---|
| Layout | The editor opens in a fixed initial size and all required functions are reachable with **one vertical scroll** and **no horizontal scroll**. |
| Startup | On load, ACE-Step checks model/cache status without blocking the host. If the selected model is missing, **Generate** is disabled and the model section explains the fix inline. |
| Generation | A user can type a prompt, choose a duration, optionally capture/import reference audio, and start a prompt-to-WAV job from the main screen without opening any additional window. |
| Progress and cancel | While a job is running, the UI shows status text plus a progress indicator, and **Cancel** is visible in the same section. Canceling never leaves the UI in an ambiguous state. |
| Result history | Every successful generation creates a new top-of-list asset tile with preview and metadata. Prior results remain visible in the same history list unless explicitly removed. |
| WAV export | Every ready result tile has **Save As WAV…** and **Drag WAV**. If drag is unsupported or rejected by the target host, **Save As WAV…** still completes the task with no dead end. |
| MIDI and stems | Every ready result tile exposes **MIDI…** and **Stems…**. If those assets must be derived asynchronously, the tile shows a local loading state and later transitions to ready or error. |
| Presets | The preset bar supports load and save. Factory presets are read-only; user presets can be overwritten intentionally. A visible dirty indicator appears when the current form differs from the loaded preset. |
| State recall | Host project save/restore brings back the prompt, duration, selected model ID, reference metadata, selected preset, and asset-history metadata without requiring the editor to have stayed open. |
| Error handling | Errors are inline, attached to the relevant section, and recovery-oriented. They include at least one next-step action such as Retry, Choose Folder, Locate, Remove, or Copy diagnostics. |
| Editor lifecycle | If the editor is closed and reopened during a background task, the job state and progress reconnect cleanly. The processor never depends on the editor remaining alive. |
| Accessibility | All core actions are keyboard reachable; focus is always visible; drag-only actions have button alternatives; status changes are expressed as text and surfaced to assistive tech. |
| Scaling | The UI remains usable at common host/OS scale factors such as 100%, 125%, 150%, and 200%, with no clipped primary controls or hidden progress text. |
| Host friendliness | No modal alert blocks project load; lightweight host-visible parameters remain stable and properly typed; and host context menus are available on exposed parameters where supported. |
| Validation readiness | The plug-in can be scanned, opened, closed, state-saved, state-restored, and reopened under validation tooling without the UI entering a broken or unrecoverable state. |

The shortest summary of the recommendation is this: **keep ACE-Step’s first recovery UI to one fixed-size scrollable column with a top preset/model bar, one obvious generation form, one obvious progress/cancel strip, one obvious result-history section, and one small diagnostics footer.** That is the minimum surface that still covers prompt-to-WAV generation, reference capture, model readiness, export, presets, errors, and host-safe behavior.