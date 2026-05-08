# Local Open-Source Audio-to-MIDI Options for a Windows JUCE VST3 Sidecar

## Executive summary

For **ACE-Step VST3 on Windows 11**, the strongest **v1** choice is **Spotify Basic Pitch** running in a **local helper sidecar**, not inside the VST3 audio callback. It is the cleanest fit because it is explicitly documented for **Windows**, installs directly from **PyPI**, defaults to **ONNX on Windows**, generates a **MIDI file directly from audio**, can save note-event CSVs for debugging, and exposes an API for **reusing a loaded model** across jobs so the sidecar can stay warm. Its core tradeoff is that it is **instrument-agnostic but works best on one instrument at a time**, so it is ideal for approximate first-pass symbolic export but not the best ceiling for dense multi-instrument mixtures. citeturn5view0turn12view0turn12view3turn27search5turn14view0

The strongest **fallback / upgrade path** is the **MT3 family through `mt3-infer`**, specifically **`mr_mt3`** first and then, if needed later, **`yourmt3`** for heavier multi-instrument work. This is the best route if you want a backend that can later be swapped from “approximate whole-audio transcription” to “higher-quality note-event generation” without changing your plugin-side contract. `mt3-infer` provides a **single local CLI and Python API** for **MR-MT3**, **MT3-PyTorch**, and **YourMT3**; the package is small, checkpoints auto-download, and the documented model sizes are **176 MB** for MR-MT3, **176 MB** for MT3-PyTorch, and **536 MB** for YourMT3. The main downside is deployment complexity: this stack is substantially heavier than Basic Pitch and is best treated as an optional high-quality backend in the sidecar. citeturn7view0turn7view2turn7view3turn25search2turn25search1turn25search8

If you want a **native/C++ trajectory**, the most relevant open-source derivatives are **NeuralNote** and **basicpitch.cpp**. NeuralNote is a **JUCE-based plugin/standalone app** that already uses **Basic Pitch internally**, has **Windows installers**, and explicitly says its transcription code lives in `Lib/Model` for reuse. `basicpitch.cpp` is a **C++20 ONNXRuntime/libremidi** implementation of Basic Pitch with a CLI app; it is especially interesting architecturally, but its README says it was **only tested on Linux**. These are not the easiest way to ship v1, but they are the best code references if you later want to reduce Python sidecar dependency or embed more of the pipeline in native code. citeturn24view0turn30view0

For **specialized sub-tasks**, the picture is different. **Omnizart** remains one of the broadest open-source MIR toolkits because it covers **pitched music transcription, vocal melody, vocal contour, drum events, chords, and beat**, but its latest PyPI release is from **2021** and the project has documented **Windows installation issues**. **madmom** and **librosa** still matter, but primarily as **feature-extraction and post-processing tools** rather than full modern polyphonic audio-to-MIDI engines: madmom is still strong for **beats, downbeats, chords, onset pipelines, and some piano-note tooling**, and librosa is still a dependable building block for **pYIN melody/F0**, **onsets**, and **beat tracking**. citeturn6view3turn31view0turn31view1turn15search4turn16search0turn16search4turn17view0turn17view1turn19search0turn19search1turn19search2turn19search3turn22search3

## Candidate matrix

I ranked these candidates for your specific constraints: **Windows 11**, **JUCE 8/C++17 host**, **strictly non-blocking VST3 callback**, **local-only execution**, and **“usable MIDI export” first, backend-replaceable architecture second**.

| Candidate | Best use | License | Model size or package weight | Runtime stack | Windows and sidecar fit | Overall fit |
|---|---|---|---|---|---|---|
| **Basic Pitch** | Approximate **polyphonic note transcription** for single-part or relatively clean generated material; includes **pitch bends** | Apache-2.0 | Official docs emphasize “lightweight”; the browser TF.js weight shard in `basic-pitch-ts` is **725 KB**; the Python package ships multiple serializations and chooses **ONNX on Windows** by default. citeturn5view0turn9view0turn11view0 | Python CLI/API; TensorFlow, CoreML, TFLite, **ONNX** runtimes; TypeScript sibling repo available. citeturn5view0turn9view0 | **Best Windows fit** in this set: README explicitly lists **Windows**, supports Python 3.7–3.11, and says Windows defaults to **ONNX**. Direct WAV→MIDI CLI and reusable in-process model API make it ideal for a helper sidecar. citeturn5view0turn12view0turn12view3 | **Primary recommendation** |
| **MT3 family via `mt3-infer`** | Higher-ceiling **polyphonic / multi-instrument** transcription; future backend upgrade path | `mt3-infer` MIT; upstream family includes Apache-2.0 for Magenta MT3, MIT for MR-MT3, GPL-3.0 repo for original YourMT3 code, with YourMT3+ checkpoints also published on HF. citeturn5view6turn5view2turn25search6turn27search3turn27search7 | `mt3-infer` package ~**8 MB** source; with downloaded models ~**882 MB**. Documented model sizes: **MR-MT3 176 MB**, **MT3-PyTorch 176 MB**, **YourMT3 536 MB**. citeturn7view3turn7view0turn28search10 | Python CLI/API with PyTorch/TensorFlow/JAX isolation; cached model loading; auto-download checkpoints. citeturn7view0turn7view2turn7view3 | Good sidecar fit, but much heavier than Basic Pitch. Best treated as an **optional high-quality backend**, not the first thing to ship. citeturn7view0turn26view3 | **Fallback recommendation** |
| **Basic Pitch derivatives** | Native integration references and non-Python deployments | NeuralNote Apache-2.0; `basicpitch.cpp` MIT. citeturn24view0turn30view0 | NeuralNote reuses Basic Pitch internals; `basicpitch.cpp` compiles an ORT model into binaries and uses libremidi. citeturn24view0turn30view0 | **NeuralNote** uses **RTNeural + ONNXRuntime**; **basicpitch.cpp** uses **ONNXRuntime + Eigen + libremidi** and includes CLI/WASM demos. citeturn24view0turn30view0 | Excellent for **architectural borrowing**. NeuralNote has **Windows installers** and reusable transcription code; `basicpitch.cpp` is promising but readme says **only tested on Linux**. citeturn24view0turn30view0 | Strong long-term native path |
| **Omnizart** | Broadest open-source MIR toolbox: **music, drum, vocal melody, vocal contour, chord, beat** | Project docs say TensorFlow-based open-source library; PyPI latest release **0.5.0** from **Dec. 2021**. citeturn15search10turn15search4 | Checkpoint sizes are not summarized in the docs I reviewed. | Python/TensorFlow CLI/API; returns `pretty_midi.PrettyMIDI` for music/drum transcription. citeturn31view0turn31view1 | Broad functionality is excellent, but the project is older and has **known Windows install issues**, including a chord-path dependency problem noted by users. citeturn6view3turn31view0turn31view1turn16search0turn16search4 | Good toolbox, weaker Windows choice |
| **Piano Transcription Inference** | Best if ACE-Step export is effectively **solo piano** or piano-like rendered material | Not surfaced in the retrieved README excerpt, but project is public and ties to the ByteDance piano-transcription work. | Checkpoint auto-download from Zenodo; size not summarized in retrieved README. citeturn6view2 | Python/PyTorch | Usage is straightforward and it supports **CPU or CUDA**, but README says **Linux and Mac** are supported and **Windows has not been tested**. citeturn6view2turn6view0 | Very good specialist, not ideal general v1 |
| **ADTOF-PyTorch** | Specialist **drum-only** transcription to MIDI | The README page I reviewed did not surface a license, so verify terms before product use. citeturn36view0turn37view0 | Minimal-dependency PyTorch port; bundled weights; size not summarized in README. citeturn36view0 | Python/PyTorch + librosa + pretty_midi | Strong drum-focused sidecar candidate with direct CLI/API and an MDBDrums++ comparison, but it is narrowly scoped to drums and needs license verification before shipping. citeturn36view0 | Excellent specialist auxiliary backend |
| **madmom + librosa pipeline** | **Beat, onset, chord, monophonic melody/F0**, and deterministic post-processing around other AMT engines | madmom code BSD; pretrained madmom models/data are released separately with **CC BY-NC-SA 4.0**; librosa ISC. citeturn18search5turn18search12turn22search0turn22search6 | Small library dependencies, not giant checkpoints. | Python signal-processing stack | Not a modern end-to-end general polyphonic WAV→MIDI winner, but still extremely useful for **tempo maps, onset correction, chord labels, monophonic pYIN melody, and QA**. madmom’s Windows story is weak and historically source-based. citeturn17view0turn17view1turn17view2turn19search2turn19search3turn22search1 | Keep as supporting tools, not primary AMT |

## Candidate notes and exact invocation

### Basic Pitch

Basic Pitch is the most deployment-friendly open-source AMT package in this set. The official README says it supports **polyphonic instruments**, is **instrument-agnostic**, outputs **MIDI complete with pitch bends**, and “works best on one instrument at a time.” It explicitly documents **Windows** support and says the default runtime on Windows is **ONNX**. It also exposes a CLI, a `predict()` API, and a pattern for loading the model once and reusing it across multiple calls. citeturn5view0turn12view0turn12view3

**CLI**
```bash
pip install basic-pitch
basic-pitch .\out .\input.wav
basic-pitch .\out .\input.wav --save-note-events --save-model-outputs
```
The CLI writes a MIDI transcription to the output directory and can also emit CSV note events and raw NPZ model outputs. citeturn12view0

**Python API**
```python
from basic_pitch.inference import predict, Model
from basic_pitch import ICASSP_2022_MODEL_PATH

# one-shot
model_output, midi_data, note_events = predict(r"input.wav")
midi_data.write(r"output.mid")

# warm model for repeated sidecar jobs
basic_pitch_model = Model(ICASSP_2022_MODEL_PATH)
model_output, midi_data, note_events = predict(r"input.wav", basic_pitch_model)
midi_data.write(r"output.mid")
```
The official docs recommend loading the model yourself in loops to avoid repeated slow model loads. citeturn12view0turn12view3

### MT3 family through `mt3-infer`

The official Magenta **MT3** repository is important research code, but its own README points users to the **Colab notebook** for transcription and says training is not easily supported. For production-style local inference in 2026, `mt3-infer` is the practical open-source entrypoint because it normalizes **MR-MT3**, **MT3-PyTorch**, and **YourMT3** behind one CLI/API. citeturn5view2turn7view3

The major algorithmic tradeoffs are clear. The original **MT3** paper positioned MT3 as a state-of-the-art multi-task, multi-track transcription framework for multi-instrument AMT; **MR-MT3** was introduced specifically to reduce **instrument leakage**; and **YourMT3+** reported competitiveness with or superiority to existing transcription models across ten public datasets while adding direct **vocal transcription** capability. `mt3-infer` then exposes deployment-oriented variants with documented weights and benchmarked speed on an RTX 4090. citeturn25search8turn25search2turn25search1turn7view0

**CLI**
```bash
pip install mt3-infer
mt3-infer download mr_mt3
mt3-infer transcribe input.wav -o output.mid -m mr_mt3

# alternatives
mt3-infer transcribe input.wav -o output.mid -m mt3_pytorch
mt3-infer transcribe input.wav -o output.mid -m yourmt3
```
The project also documents `mt3-infer download --all` and `--device cuda`. citeturn6view6turn6view7turn26view3

**Python API**
```python
from mt3_infer import load_model

model = load_model("mr_mt3")      # cached for reuse
midi = model.transcribe(audio, sr=16000)
midi.save("output.mid")
```
The package also documents a one-line `transcribe(audio, sr=16000)` path and persistent model loading via `load_model()`. citeturn7view0turn7view2

For your plugin, **`mr_mt3`** is the best fallback starting point because it keeps the checkpoint at **176 MB**, improves leakage relative to raw MT3, and is positioned as the speed-optimized variant in `mt3-infer`. If you later need a heavier research mode for more complex multi-instrument material, **`yourmt3`** is the next step. citeturn7view0turn25search2turn25search1

### Omnizart

Omnizart is still uniquely broad: the docs and README say it can transcribe **pitched instruments**, **vocal melody**, **frame-level vocal contour**, **drum events**, **chords**, and **beat**. For music and drum transcription, the documented API returns `pretty_midi.PrettyMIDI`. That breadth makes it a useful research toolbox and a decent side-backend for extracting non-note symbolic structure such as **beat** or **chord** labels. citeturn6view3turn31view0turn31view1

**CLI**
```bash
pip install omnizart
omnizart download-checkpoints
omnizart music transcribe input.wav
omnizart drum transcribe input.wav
omnizart chord transcribe input.wav
```
These are the commands shown in the project README. citeturn6view3

**Python API**
```python
from omnizart.music.app import MusicTranscription

midi = MusicTranscription().transcribe("input.wav", output="./out")
```
The docs say `transcribe()` writes a MIDI file and returns a `pretty_midi.PrettyMIDI` object. citeturn31view0

I would not choose Omnizart as your primary Windows v1 because the retrieved evidence shows **latest PyPI release in 2021** and **known Windows installation issues**, including a chord-path dependency problem noted in Windows discussions. It is better as a research utility or an optional offline analysis backend than as the first production sidecar target. citeturn15search4turn16search0turn16search4

### Basic Pitch derivatives relevant to JUCE and native code

**NeuralNote** is the most relevant derivative for your use case because it is already a **JUCE audio plugin / standalone app** and explicitly states it uses **Basic Pitch internally**, running parts of the pipeline through **RTNeural** and **ONNXRuntime**. The README says the transcription code is in `Lib/Model` and invites reuse. It also ships **Windows installers**. That makes it less of a sidecar candidate and more of a **native integration reference implementation**. citeturn24view0

**basicpitch.cpp** is a separate C++ implementation that uses **ONNXRuntime**, **Eigen**, and **libremidi**, and includes a command-line app. It even points to the vendored Basic Pitch ONNX model path. For architectural experimentation it is excellent, but its README says it was **only tested on Linux**, so I would not make it your first Windows deliverable. citeturn30view0

**CLI for `basicpitch.cpp`**
```bash
./build/build-cli/basicpitch input.wav ./midi-out-cpp
```
That command is shown in the readme, which reports writing `clip.mid` to the output directory. citeturn30view0

### Piano-only specialist

If the ACE-Step render path ever narrows to **solo piano** or piano-like generated content, **Piano Transcription Inference** stays attractive because its API is simple and it supports both **CPU** and **CUDA** invocation. The README shows direct WAV/MP3→MIDI usage and a programmatic `PianoTranscription(...).transcribe(audio, 'out.mid')` path. The downside is that the project explicitly says **Windows has not been tested**. citeturn6view2turn6view0

**CLI**
```bash
pip install piano_transcription_inference
python3 example.py --audio_path="input.wav" --output_midi_path="output.mid"
```
For GPU mode, the project example appends `--cuda`. citeturn6view2

### Drum specialist

If you want a **dedicated drum backend**, **ADTOF-PyTorch** is the most interesting current specialist I found. Its README positions it as a PyTorch port of ADTOF with **minimal dependencies**, direct **audio→MIDI** commands, and a **programmatic API**. It also includes a quantitative comparison to the original ADTOF on **MDBDrums++**. The caveat is that the repository page I retrieved did **not clearly surface a license**, so I would verify that before bundling it in a product. citeturn36view0turn35search8

**CLI**
```bash
adtof --audio input.wav --out output.mid \
  --thresholds 0.22,0.24,0.32,0.22,0.30 --fps 100 --device cuda
```

**Python API**
```python
from adtof_pytorch import transcribe_to_midi
transcribe_to_midi("input.wav", "output.mid")
```
Both are documented in the README. citeturn36view0

### madmom and librosa as support libraries

`madmom` is still valuable for **beats**, **downbeats**, **chords**, **onsets**, and some **piano-note** tools. Its docs expose modules for `features.beats`, `features.chords`, `features.notes`, `features.onsets`, and MIDI I/O, and the project still ships bundled programs such as **DBNBeatTracker** and **CNNChordRecognition**. The package itself is easy to install with `pip install madmom`, but its Windows wiki says Windows is **not officially supported** and historically required source builds. Also, madmom’s source code and its pretrained model/data assets use **different licenses**, which is a real product concern. citeturn5view5turn17view0turn17view1turn17view2turn18search5turn18search12

**Useful madmom commands**
```bash
pip install madmom
DBNBeatTracker single input.wav -o beats.txt
CNNChordRecognition single input.wav -o chords.txt
```
These bundled tools are documented in the package and source tree. citeturn20search5turn19search4turn19search5

`librosa` is not an end-to-end AMT engine, but it is still an excellent deterministic toolkit for **pYIN melody/F0** (`librosa.pyin`), **onset detection** (`librosa.onset.onset_detect`), and **beat tracking** (`librosa.beat.beat_track`). If you want a transparent baseline for **monophonic melody export** or for **post-transcription beat/onset cleanup**, librosa is still one of the most useful supporting libraries available. citeturn19search2turn22search1turn19search3turn22search3

## Recommendation and rationale

### Primary v1 implementation

Use **Basic Pitch official Python package** in a **local sidecar** as your shipping v1. That recommendation is driven less by leaderboard ambition and more by your actual product constraints: **Windows support is explicit**, the sidecar can rely on a **documented CLI and API**, ONNX is the **default Windows runtime**, MIDI export is built in, and the model can be **kept warm** between jobs. Those traits materially lower integration risk for a JUCE VST3 that must keep the audio callback pass-through-only and non-blocking. citeturn5view0turn12view0turn12view3

The other reason Basic Pitch is the right v1 is that your initial requirement is only **“functional MIDI export”** from **generated WAV**. Generated content is usually cleaner and more internally consistent than arbitrary live recordings, so a lightweight instrument-agnostic model that already handles **polyphony** and **pitch bends** is a very pragmatic place to start. The architecture can still preserve an internal note-event format so you can later swap in a different backend without changing the plugin/UI contract. The official docs even let you export **CSV note events** alongside MIDI, which is useful for debugging and for gradually moving toward a backend-defined note-event layer. citeturn5view0turn12view0

### Fallback implementation

Use **`mt3-infer` with `mr_mt3`** as the fallback implementation. This gives you a stronger algorithmic family, a cleaner path to later upgrades, and a maintained inference wrapper that is much more deployment-friendly than the original Magenta MT3 repo. MR-MT3 is the right fallback member because it specifically targets **instrument leakage**, keeps the checkpoint at **176 MB**, and is documented by `mt3-infer` as the **speed-optimized** option. citeturn25search2turn7view0turn7view3

I would *not* make `yourmt3` your first fallback despite its stronger research positioning, because the documented **536 MB** checkpoint and heavier architecture make it a worse operational fit for a Windows helper you want to keep simple. I would reserve `yourmt3` for a later “high-quality transcription mode” or for backend experimentation after the Basic Pitch path is stable. citeturn7view0turn25search1

## Local sidecar integration sketch

The safest architecture is a **strict out-of-process worker** with a **stable internal result schema**:

```text
ACE-Step VST3
  ├─ audio callback: pass-through only
  ├─ background thread: writes generated WAV, enqueues job
  └─ UI/control thread: polls worker status, imports result MIDI

Local sidecar worker
  ├─ warm model pool
  │   ├─ Basic Pitch Model(ICASSP_2022_MODEL_PATH)
  │   └─ optional mt3_infer.load_model("mr_mt3")
  ├─ job API
  │   ├─ input_wav
  │   ├─ output_mid
  │   ├─ backend
  │   ├─ settings
  │   └─ correlation_id
  └─ result bundle
      ├─ output.mid
      ├─ note_events.json
      ├─ optional note_events.csv
      └─ diagnostics.json
```

The important design decision is to make the sidecar’s *true product output* a backend-neutral **note-event structure**, with MIDI merely one rendering target. That lines up well with both Basic Pitch and `mt3-infer`: Basic Pitch can emit MIDI plus note events/CSV, while `mt3-infer` returns MIDI objects and supports persistent loaded models. This gives you a clean future path where the transcription step can later be replaced by **backend/model-derived note events** rather than by audio-to-MIDI inference, without refactoring the plugin boundary. citeturn12view0turn12view3turn7view0turn7view2

For the Windows implementation, I would package the sidecar as one of these:

- a **Python-embedded executable** with pinned dependencies and cached checkpoints;
- a **small RPC/CLI worker** that takes file paths and emits a JSON status/result manifest;
- or, later, a **native helper** derived from NeuralNote/basicpitch.cpp code if you decide Python packaging is the main operational pain point. NeuralNote is especially relevant here because it already solved the “JUCE + non-real-time transcription + MIDI export” problem in open source. citeturn24view0turn30view0

The job contract should be **file-based and atomic**. Write the generated WAV, invoke the sidecar with an output directory, and only publish/import the `.mid` once the worker has fully completed and written a manifest. This avoids partial-file races and keeps the VST side simple. For repeated jobs, keep the sidecar alive and **reuse the loaded model** rather than launching a fresh Python process for every render. Basic Pitch and `mt3-infer` both document patterns that support this. citeturn12view3turn7view2

## Test assets and acceptance criteria

For a serious “functional MIDI export” gate, I would use **two classes of assets**: **public reference datasets** and **ACE-Step synthetic fixtures**. Public datasets ensure your sidecar is not overfit to only one generated texture family; ACE-Step fixtures ensure your actual product path is validated against known source MIDI. This combination is much better than relying only on in-the-wild songs. citeturn32search0turn32search2turn32search3turn33search1turn33search2turn33search3

A compact but representative validation set would include:

- **MAESTRO** excerpts for piano note accuracy and timing; Magenta describes it as about **200 hours** of aligned piano audio/MIDI with roughly **3 ms** alignment. citeturn32search0turn32search15
- **Slakh2100** mixes and stems for multi-instrument transcription; Slakh is specifically described as a dataset of **multi-track audio and aligned MIDI** for source separation and **multi-instrument automatic transcription**, with **2100** tracks in Slakh2100. citeturn32search2turn32search7turn32search12
- **GuitarSet** or selected **MedleyDB** stems for polyphonic but cleaner instrument-focused cases; GuitarSet is explicitly a dataset for **guitar transcription**, and MedleyDB was curated to support **melody extraction** from multitrack recordings. citeturn32search1turn32search6turn33search3
- **MIR-ST500** for singing transcription or melodic vocal tests; the MIR-ST500 repo describes it as a **500-song** singing transcription dataset. citeturn32search4turn32search14
- **Groove MIDI Dataset**, **Expanded Groove**, **IDMT-SMT-Drums**, or **ENST-Drums** for drum cases. Groove is an aligned expressive drum dataset; Expanded Groove increases coverage to **444 hours** of examples with ground-truth MIDI; IDMT-SMT-Drums and ENST-Drums are classic automatic drum transcription datasets. citeturn32search3turn32search13turn33search1turn33search2
- **ACE-Step rendered fixtures** where you render a known MIDI pattern through your own generator and compare the exported MIDI against the source pattern after deterministic normalization. This last class is product-specific and should be your main release gate.

For objective note-level checks, I would use **`mir_eval`** conventions: note-onset matching with **±50 ms** tolerance, and onset+offset note F-measures where pitch, onset, and offset must all be sufficiently close. Those are standard MIR evaluation primitives and are exactly the right baseline for deciding whether your MIDI export is “usable” rather than “musicologically perfect.” citeturn34search0turn34search6turn34search11

The practical **product acceptance criteria** I would use for v1 are:

- the sidecar always emits a **parseable MIDI file** that opens in at least one strict parser and one DAW;
- for **single-line and simple harmonic ACE-Step fixtures**, note-onset F1 should clear a product gate you define using `mir_eval` with **50 ms** onset tolerance;
- the exported MIDI length should remain close to the source audio duration after normalization;
- the plugin must never block the audio callback while export jobs run;
- the sidecar must produce a **machine-readable diagnostics artifact** with note count, wall-clock runtime, backend name, and errors, so failures are actionable rather than silent.

Those threshold values are ultimately product choices, but the **metric definitions** should stay aligned with standard MIR evaluation primitives so you can compare your internal gates against published work later. citeturn34search0turn34search6

## Risks and mitigations

The biggest technical risk is **backend mismatch between “works on demos” and “works reliably in a shipped Windows plugin toolchain.”** Basic Pitch has the lowest risk here because it is explicit about Windows support and default ONNX usage, while MT3-family backends are more operationally complex. Mitigation: ship **one backend first**—Basic Pitch—and wrap every backend behind the same note-event interface and result manifest. citeturn5view0turn12view0turn7view3

The second risk is **callback safety**. Audio transcription pipelines are inherently offline or near-offline processes, and even related open-source projects like NeuralNote explicitly say real-time transcription is not currently practical because of **CQT latency**, model latency, and non-causal note-event creation. Mitigation: keep the callback pass-through-only, render audio to file, and do transcription only in the helper. citeturn24view0

The third risk is **quality inconsistency on dense or mixed material**. Basic Pitch’s own README says it works best on **one instrument at a time**, and official MT3 demo materials note that the original models were **not trained on singing**, while YourMT3+ specifically advertises improved direct vocal handling. Mitigation: expose **backend selection by content class** in the sidecar, even if the UI only exposes one mode at first. A simple internal rule could be “Basic Pitch default; MR-MT3 for dense multi-instrument mode; ADTOF for drum-only mode.” citeturn5view0turn25search3turn25search1turn36view0

The fourth risk is **licensing and maintenance drift**. Basic Pitch and NeuralNote are straightforward on licensing; Omnizart is older; madmom has split code/model licenses; and ADTOF-PyTorch needs an explicit license check before product embedding. Mitigation: use **Basic Pitch** for v1, keep optional backends as isolated sidecar modules, and verify every non-primary backend license before distributing it. citeturn5view0turn24view0turn15search4turn18search5turn18search12turn36view0

The final risk is **premature over-optimization**. Since your architecture should eventually allow replacement by backend/model-derived note events, the most valuable early work is not squeezing the last few percentage points from a research model; it is building a stable **transcription job contract**, **warm-model lifecycle**, **diagnostics**, and **MIDI import flow**. That architecture will continue to pay off even if the transcription algorithm changes later. The APIs exposed by Basic Pitch and `mt3-infer` already support this style of design. citeturn12view3turn7view2

## Open questions and limitations

Some details were straightforwardly documented, and some were not. In particular, the source material I reviewed did **not** provide a clean published checkpoint-size table for **Omnizart** or **Piano Transcription Inference**, and the repository page I reviewed for **ADTOF-PyTorch** did **not clearly surface a license**, so that backend should be treated as **license-verification-pending** before any shipping decision. citeturn31view0turn6view2turn36view0

I also found that the most practical way to deploy modern **MT3-family** models locally by May 2026 is **not** the original Magenta repo itself, but the newer **`mt3-infer`** wrapper. That is good news operationally, but it does mean any serious evaluation should be done against *your chosen wrapper/configuration*, not the abstract research name alone. citeturn5view2turn7view3

The final bottom line is simple: **ship Basic Pitch sidecar first, design the note-event contract so the backend is swappable, and keep `mt3-infer`/MR-MT3 ready as the first serious fallback.** That gives you the best balance of **Windows practicality, open-source compliance, reasonable quality, and future backend flexibility** for ACE-Step VST3. citeturn5view0turn12view0turn7view3turn25search2