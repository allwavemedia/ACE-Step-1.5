# Local Stem Separation Options for a Windows JUCE VST3 Sidecar

## Executive assessment

For a Windows 11 JUCE 8.0.10 VST3 that must keep the audio callback pass-through-only and non-blocking, the strongest local open-source separation options as of May 7, 2026 split into two groups.

The first group is the **current quality leaders**, which are the **BS-RoFormer / MelBand-RoFormer family** and their derivative community checkpoints. The published BS-RoFormer system won the Sound Demixing Challenge 2023 music-source-separation track, and the smaller MUSDB-only version reported state-of-the-art average SDR without extra data. Mel-Band RoFormer was proposed as an improvement over BS-RoFormer and reported better separation for vocals, drums, and other on MUSDB18HQ. In today’s practical open ecosystem, the most compelling multi-stem example is **BS-RoFormer-SW**, a 6-stem model used by MVSEP for vocals, bass, drums, guitar, piano, and other; MVSEP’s published quality table reports especially strong scores for instrumental, bass, drums, and vocals. citeturn36search3turn17search6turn18search1turn29search6

The second group is the **best official, stable shipping options**, led by **Demucs v4 HTDemucs**. Demucs is still the cleanest “ship this in v1” choice because it has an official MIT-licensed codebase, explicit Windows documentation, a straightforward CLI, clear GPU/CPU behavior, and a predictable 4-stem output contract. Its 6-stem variant adds guitar and piano, but the official README explicitly warns that piano quality is not great. citeturn38view2turn16view4turn9view0

For your plugin specifically, the core engineering decision is not “which model is best in the abstract,” but “which model gives the best balance of quality, licensing clarity, packaging stability, and deterministic file export in a helper process.” On that criterion, my recommendation is:

**Primary v1 implementation:** **Demucs `htdemucs_ft`** in a local helper sidecar.  
**Fallback implementation:** **SCNet Large** if you need a lighter, more CPU-tolerant 4-stem fallback with an official MIT repo and much smaller checkpoint footprint.  
**Quality-upgrade path after v1:** A pinned **BS-RoFormer-SW-class** 6-stem workflow, once you freeze a specific checkpoint and clear the weight-license/provenance questions that still exist around some community model distributions. citeturn38view2turn16view5turn36search0turn23search3turn18search1turn8search9turn25view2

## Ranked candidates

The table below ranks candidates by **practical suitability for a local Windows sidecar** for ACE-Step, not only by raw benchmark quality.

| Rank | Candidate | License status | Typical model size | Runtime stack and Windows fit | Stem groups | GPU and CPU feasibility | Local invocation | Bottom line |
|---|---|---|---:|---|---|---|---|---|
| **A** | **Demucs `htdemucs_ft`** | MIT code. citeturn38view2turn0search1 | `htdemucs_ft` is stored as four 84.1 MB shards; the Demucs v4 folder shown in a UVR mirror is 643 MB total with multiple model variants. citeturn9view0 | PyTorch CLI and Python package. Explicit Windows guidance says to use `python.exe`; on Windows Demucs relies on FFmpeg because torchaudio format support is limited. citeturn38view2 | 4 stems: vocals, drums, bass, other. citeturn38view2 | NVIDIA GPU: at least 3 GB VRAM, about 7 GB with defaults; CPU mode is supported and roughly 1.5× track duration. citeturn16view5 | Official CLI is simple and deterministic. citeturn38view2 | Best official shipping choice for v1. Clear license, stable outputs, easiest multi-stem story. |
| **B** | **BS-RoFormer-SW-class 6-stem workflow** | Code is MIT in upstream implementations such as lucidrains and `bs-roformer-infer`, but model-weight licensing metadata is inconsistent across community distributions and should be frozen and reviewed checkpoint-by-checkpoint. citeturn24search5turn5view5turn8search9turn25view2 | Example 6-stem SW checkpoint is 699 MB. citeturn8search5 | PyTorch-based; inference wrappers exist, including `bs-roformer-infer`. Not as standardized as Demucs. citeturn13search2turn33search3 | 6 stems: vocals, bass, drums, guitar, piano, other. citeturn18search1turn29search6 | Quality-first choice, but heavier and more community-fragmented than Demucs. CPU use is possible in principle, but the practical sweet spot is GPU/offline batch. citeturn18search1turn29search6 | Typically run through dedicated inference wrappers or MSST-compatible scripts. citeturn13search2turn29search6 | Best current multi-stem quality path, but higher packaging and governance risk for a first commercial plugin release. |
| **C** | **SCNet Large / XL family** | MIT. citeturn5view3 | Official large checkpoint example in UVR resources is 169 MB. citeturn23search3 | Official PyTorch repo with direct inference command. Less polished than Demucs, but much more compact. citeturn5view3 | 4 stems. Official paper and repo target standard MSS. citeturn36search0turn5view3 | Paper reports 9.0 dB SDR on MUSDB18-HQ without extra data, and CPU inference time only 48% of HT Demucs. citeturn36search0 | Official CLI-style inference exists. citeturn5view3 | Excellent fallback if you want smaller weights and better CPU behavior, with less ecosystem sprawl than UVR models. |
| **D** | **MDX23C / UVR ecosystem** | UVR GUI is MIT; `audio-separator` is MIT; some model packs are MIT-tagged, but not every derivative repo cleanly exposes license metadata. citeturn5view2turn15view3turn25view0turn22search0 | Example `MDX23C-8KFFT-InstVoc_HQ_2.ckpt` is 448 MB; `MDX23C_D1581.ckpt` is 183 MB in model listings. citeturn25view0turn29search4 | Very Windows-friendly in practice because the ecosystem heavily uses ONNX and UVR-style wrappers. `audio-separator` supports CPU/GPU installs and a Python API. Microsoft and ONNX Runtime support CPU, CUDA, WinML, and DirectML-style acceleration paths on Windows. citeturn7view0turn33search0turn33search5turn33search9 | Mostly strongest for 2-stem vocals/instrumental, but specialized derivatives exist for drum sub-stems, dereverb, center extraction, and more. citeturn20view1turn29search4turn37view0 | Good balance of speed and practicality; usually easier to deploy than research repos. citeturn7view0turn20view1 | `audio-separator` and dedicated UVR/MDX CLIs make this easy to automate. citeturn7view0turn20view0 | Great ecosystem for 2-stem and specialized operations. Less compelling for clean, official 4-stem shipping than Demucs. |
| **E** | **MelBand-RoFormer vocal/instrumental variants** | Quality is excellent, but weight licensing is scattered across community checkpoints and mirrors; verify each checkpoint before bundling. citeturn8search2turn19search0turn25view2 | Kimberley Jensen checkpoint example is 913 MB; some community variants are even larger. citeturn8search2turn8search10 | PyTorch; direct inference repo exists from Kimberley Jensen. Can be wrapped through `audio-separator`. citeturn33search1turn7view0 | Usually 2 stems: vocals and instrumental. Some specialized karaoke/lead-back-vocal variants exist in the ecosystem. citeturn33search1turn19search10turn19search13 | Very strong audio quality for vocals/instrumental; heavier than Spleeter/Open-Unmix and less standardized than Demucs. citeturn19search0turn19search9 | Direct Python inference or wrapper-based use. citeturn33search1turn7view0 | Use this when your main deliverable is vocal/instrumental export, not general 4-stem music decomposition. |
| **F** | **Open-Unmix `umxhq`** | MIT code and MIT `umxhq` weights; note that the default `umxl` weights are non-commercial. citeturn15view0turn28view0turn14view0 | `umxhq` weights total 143.6 MB; default `umxl` weights are 452.5 MB total. citeturn28view0turn10search11 | PyTorch package and CLI. Stable and well documented, but older architecture and offline only. citeturn14view0 | 4 stems: vocals, drums, bass, other. citeturn14view0 | Offline only; BiLSTM design is not suitable for real-time use. Quality is materially behind Demucs and modern RoFormers. citeturn14view0turn16view3 | Straightforward CLI and Python API. citeturn14view0 | Good safe baseline, but not the model family I would choose for a new shipping plugin. |
| **G** | **Spleeter** | MIT. citeturn15view1 | Public release assets were 73.1 MB for 2 stems, 146.3 MB for 4 stems, and 182.8 MB for 5 stems. citeturn26search6 | TensorFlow. Still installable on PyPI, but Windows CLI still has a known workaround and the classic models are bandwidth-limited to 11 kHz, with 16 kHz variants available. citeturn26search2turn15view1turn14view1 | 2, 4, or 5 stems, including piano in 5-stem mode. citeturn14view1turn15view1 | Extremely fast on GPU; Deezer reports up to 100× faster than real time for 4 stems on GPU. citeturn15view1turn26search1 | Easy CLI, but use `python -m spleeter` on Windows when needed. citeturn15view1turn14view1 | Still relevant as a fast legacy baseline, not as a best-quality 2026 recommendation. |

### What is new enough to watch closely

Two newer developments are worth monitoring, even though I would not make either your first plugin release target.

**BS PolarFormer** is a very new 2-stem entrant. A recent ONNX conversion publishes a 201 MB FP32 model and a 103 MB FP16 model, both under MIT metadata, and describes the model as a PoPE-based variant of the BSRoformer family. MVSEP published a May 2, 2026 result for “BS PolarFormer Vocals (2026.05),” which is notable because it signals active movement beyond the original RoFormer family. This is attractive for Windows because ONNX plays well with Windows accelerators, but it is too new for me to call it the safest v1 choice. citeturn31view0turn30search2turn33search0turn33search5

**MVSep Mega 53 Stems** appeared in the `Music-Source-Separation-Training` releases in April 2026. The release describes a BS-RoFormer-based 53-stem model with a long list of instrument classes, and a public Hugging Face folder shows a 4.11 GB collection of per-stem checkpoints. That is exciting for future fine-grained export from generated music, but it is still far too operationally heavy and immature for a pragmatic first plugin implementation. citeturn32search0turn34search2turn34search0

## Recommendation and rationale

### Primary v1 implementation

I recommend **Demucs `htdemucs_ft` in a local helper sidecar** as the first shipping implementation.

The reason is not that Demucs is the newest or the absolute leaderboard winner in every scenario. It is that Demucs is the best balance of what matters for your plugin: official MIT licensing, explicit Windows instructions, simple local CLI operation, standard 4-stem outputs, predictable output locations, and known memory/performance knobs such as `--segment`, `-d cpu`, and `--float32`. It writes exactly the stem file set you expect for export workflows, and its helper-process deployment model is straightforward. citeturn38view2turn16view5turn9view0

For ACE-Step, that means you can define a stable contract immediately:

- input: one generated WAV;
- job: asynchronous helper-process separation;
- output: `vocals.wav`, `drums.wav`, `bass.wav`, `other.wav`;
- UX: Save As and drag/drop simply expose those produced files.

That contract is stable enough to ship, test, and support. It also leaves room to add a “high-detail” model tier later without changing the plugin’s file/result abstraction. The main drawback is that the official 6-stem Demucs variant is explicitly experimental and weak on piano, so I would not make `htdemucs_6s` your default unless guitar/piano export is mandatory on day one. citeturn16view4turn38view2

### Fallback implementation

I recommend **SCNet Large** as the fallback implementation.

SCNet’s official repo is MIT, its checkpoint is compact relative to Demucs and RoFormer community weights, and the paper explicitly positions it as lower-compute while still competitive, reporting CPU inference time at only 48% of HT Demucs. If a user’s machine struggles with Demucs, or if you want a lighter recovery path for non-NVIDIA systems, SCNet is the strongest fallback I found that still keeps a real 4-stem story. citeturn5view3turn36search0turn23search3

### Quality-first upgrade path

If your product roadmap eventually demands better stem fidelity and more specialized export groups, the most interesting upgrade path is a **pinned BS-RoFormer-SW-class 6-stem model** or a related MSST-based RoFormer checkpoint set. That class is where the open local ecosystem looks strongest in 2026 for broad multi-stem quality. The caution is operational rather than sonic: checkpoint provenance, license metadata, and wrapper maturity are still much less tidy than Demucs. citeturn18search1turn29search6turn31view0turn8search9turn25view2

## Exact local commands and APIs

### Demucs

Install on Windows:

```powershell
python.exe -m pip install -U demucs
```

Separate all four stems with the fine-tuned v4 model:

```powershell
python.exe -m demucs -n htdemucs_ft "C:\ACE-Step\jobs\mix.wav"
```

Force CPU if needed:

```powershell
python.exe -m demucs -n htdemucs_ft -d cpu "C:\ACE-Step\jobs\mix.wav"
```

Demucs writes stems under `separated\htdemucs_ft\<track-name>\` and produces `drums.wav`, `bass.wav`, `other.wav`, and `vocals.wav`. On Windows, the project explicitly says to replace `python3` with `python.exe`. citeturn38view2turn38view1

If you want karaoke mode only:

```powershell
python.exe -m demucs --two-stems=vocals "C:\ACE-Step\jobs\mix.wav"
```

That still performs full separation under the hood, so it is not a true low-compute shortcut. citeturn38view2

### SCNet

Official inference command:

```powershell
python -m scnet.inference --input_dir C:\ACE-Step\jobs\in --output_dir C:\ACE-Step\jobs\out --checkpoint_path C:\models\scnet-large.ckpt
```

The official repo documents direct inference this way after downloading the model checkpoint. citeturn5view3

### Audio Separator wrapper for UVR, MDX23C, Demucs, and RoFormer checkpoints

Install CPU-only:

```powershell
pip install "audio-separator[cpu]"
```

Install GPU-enabled:

```powershell
pip install "audio-separator[gpu]"
```

Example using a strong BS-RoFormer vocal/instrumental checkpoint:

```powershell
audio-separator C:\ACE-Step\jobs\mix.wav --model_filename model_bs_roformer_ep_317_sdr_12.9755.ckpt
```

List available models:

```powershell
audio-separator --list_models
```

Filter models by stem type:

```powershell
audio-separator -l --list_filter=drums
```

The package auto-downloads the specified model on first use and has a Python API as well. Its examples also show chunking support for long files and ensembling of multiple models. citeturn7view0turn12view0turn12view4

Python API:

```python
from audio_separator.separator import Separator

separator = Separator()
separator.load_model()   # defaults to a MelBand RoFormer checkpoint if unspecified
output_files = separator.separate("audio1.wav")
print(output_files)
```

That exact minimal pattern is documented in the project README. citeturn7view0

### MDX23C reference implementation

The public MVSEP MDX23 repo documents this inference command:

```powershell
python inference.py --input_audio C:\ACE-Step\jobs\mix.wav --output_folder C:\ACE-Step\jobs\results
```

Add `--cpu` for CPU mode, or `--only_vocals` when only vocal/instrumental export is needed. The repo documents that CPU mode can be very slow and that `--large_gpu` expects about 11 GB of free GPU memory. citeturn20view1

### Open-Unmix

CLI:

```powershell
umx C:\ACE-Step\jobs\mix.wav
```

Python one-liner API:

```python
from openunmix.predict import separate
estimates = separate(audio)
```

Open-Unmix ships as a PyPI package, supports pre-trained models, and separates into vocals, drums, bass, and other. citeturn14view0

### Spleeter

On Windows, use the documented workaround form if the `spleeter` shortcut misbehaves:

```powershell
python -m spleeter separate -o C:\ACE-Step\jobs\out -p spleeter:4stems C:\ACE-Step\jobs\mix.wav
```

For five stems:

```powershell
python -m spleeter separate -o C:\ACE-Step\jobs\out -p spleeter:5stems C:\ACE-Step\jobs\mix.wav
```

Spleeter documents 2-, 4-, and 5-stem models, and notes that the classic models are bandwidth-limited to 11 kHz with 16 kHz variants available. citeturn14view1turn15view1

### Kimberley Jensen Mel-Band RoFormer

The repo documents this direct inference form:

```powershell
python inference.py --config_path configs/config_vocals_mel_band_roformer.yaml --model_path melbandroformer.ckpt --input_folder C:\ACE-Step\jobs\in --store_dir C:\ACE-Step\jobs\out
```

That produces a vocals file and an instrumental file for each WAV in the input folder. citeturn33search1

### BS PolarFormer

The published ONNX conversion documents this local inference command:

```powershell
python run_onnx_inference.py C:\ACE-Step\jobs\mix.wav --output_dir C:\ACE-Step\jobs\out
```

Use `--fp16` for the smaller ONNX model when appropriate. citeturn31view0

## Sidecar integration sketch

The cleanest architecture for ACE-Step is a **file-oriented background worker** with a very small control channel.

The VST3 should never attempt separation from the audio callback. Instead, the plugin UI or message thread writes a **job manifest** that includes the input WAV path, chosen model ID, output folder, and desired export format. The helper process picks up the job, runs separation asynchronously, writes all stems to a job-specific folder, and updates a status file or lightweight IPC endpoint. Because the output of every system here is already a real file tree, your Save As and drag/drop UX can be built as plain file exposure rather than a second serialization layer.

A practical Windows layout looks like this:

`%LocalAppData%\ACE-Step\StemJobs\<job-id>\input\mix.wav`  
`%LocalAppData%\ACE-Step\StemJobs\<job-id>\output\vocals.wav`  
`%LocalAppData%\ACE-Step\StemJobs\<job-id>\output\drums.wav`  
`%LocalAppData%\ACE-Step\StemJobs\<job-id>\output\bass.wav`  
`%LocalAppData%\ACE-Step\StemJobs\<job-id>\output\other.wav`  
`%LocalAppData%\ACE-Step\StemJobs\<job-id>\job.json`  
`%LocalAppData%\ACE-Step\StemJobs\<job-id>\status.json`

For control-plane communication, I would keep it simple. On Windows, **named pipes** are a good fit if you want zero firewall edge cases; a loopback-only HTTP control port is also reasonable. The helper can expose commands such as `submit`, `status`, `cancel`, and `reveal-output-folder`. The plugin never streams audio to the sidecar in real time; it passes file paths only. That sharply reduces synchronization bugs and keeps the real-time path isolated.

A good v1 helper contract is:

- one input WAV per job;
- one model ID per job;
- one immutable output folder per job;
- explicit completion state;
- physical stem WAVs only after atomic finalization.

That last point matters. Write outputs to a temp directory first, then atomically rename or promote them into the final output folder only when all expected stems and manifests are present. This prevents half-finished folders from appearing in Save As or drag/drop.

If you want the most maintainable implementation path, a **frozen Python sidecar** is the path of least resistance because the best current open-source separation stacks are overwhelmingly Python-first. A C++-native inference sidecar becomes more attractive only if you standardize on ONNX-based models later.

## Test assets and objective acceptance criteria

### Test assets

For **ground-truth functional and regression testing**, I would use three asset buckets.

Use a short set of **MUSDB18-HQ excerpts** for standard 4-stem validation, because Demucs, Open-Unmix, and the broader literature/tools explicitly target MUSDB or MUSDB18-HQ-style 4-stem separation. citeturn14view0turn16view5turn36search0

Use an **internal synthetic pack rendered from ACE-Step or a DAW session** for plugin-specific testing. This should include tracks where you know the exact source stems because you rendered them yourself: vocal-led pop, instrumental EDM, sparse piano ballad, guitar-driven rock, dense cinematic hybrid, fully instrumental synthwave, and at least one track with no vocals at all. Those synthetic references are especially important because your plugin is targeting generated music, not only commercial mixed masters.

Use an **edge-case pack** with very short clips, silence, mono input, clipped input, 48 kHz files, very dense percussion, and unusually long files. This validates the file-handling, padding, resampling, and chunking parts of the helper rather than the model itself.

### Acceptance criteria for functional stem export

For a first shipping build, I would define “functional stem export” this way.

A job must complete asynchronously without blocking playback or the UI controls that are unrelated to the export job. The helper may take seconds or minutes; the plugin must remain responsive throughout.

For every completed job, the helper must produce the **documented full set of stems** for the chosen model tier. For the primary Demucs tier, that means exactly four WAV files with stable names. For any alternative tier, the stem set must be documented and versioned.

Each output WAV should match the input’s duration closely enough for drag/drop editing. A practical tolerance is **sample-accurate where possible**, with any padding trimmed so the final files match the source length or differ by no more than a few milliseconds of codec or windowing edge behavior.

Each output file should be a valid WAV, readable by standard DAWs, with no NaNs, no zero-byte files, and no silent stem unless the source content truly warrants it. The sidecar should also emit a manifest that records the model ID, checkpoint hash, sample rate, channel count, output filenames, and completion timestamp.

A useful reconstruction check is to sum the exported stems and compare them to the input mixture. Because some systems can rescale or clip-protect outputs, I would not make this a strict bitwise requirement. Instead, I would use an automated regression threshold such as:

- duration match within tolerance;
- summed stems strongly correlate with the original mix;
- no stem exceeds allowed clipping threshold;
- no stem is unexpectedly empty.

For the plugin UX, Save As and drag/drop are successful only if the file paths remain valid after the helper exits and the target DAW or file explorer can ingest them immediately.

## Risks and mitigations

The biggest risk is **model-weight provenance and licensing**, especially in the RoFormer and UVR community ecosystem. The code licenses are often clear, but checkpoint metadata is not always uniform across mirrors and derivative training efforts. The mitigation is to pin exact checkpoints, record hashes, keep a NOTICE file, and do not bundle any weight until a human review signs off on the specific file you ship. citeturn8search9turn25view2turn5view2

The second risk is **Windows packaging drift**. Python, CUDA, ONNX Runtime, FFmpeg, and model-download behavior can all change over time. The mitigation is to freeze the helper into a versioned environment, avoid “latest” dependencies, and run a startup self-test that validates FFmpeg presence, model path accessibility, and accelerator availability before the first real export. Demucs and `audio-separator` both document dependency and accelerator caveats that justify this. citeturn38view2turn7view0

The third risk is **GPU variability**. Demucs is comfortable on CPU but slower; RoFormer-class models can be much more demanding. ONNX-based paths are attractive on Windows because ONNX Runtime supports CPU and CUDA, while Microsoft documents Windows AI acceleration paths around DirectML/WinML. The mitigation is to implement capability-based model selection in the helper: if no suitable accelerator is present, prefer Demucs or SCNet over the heaviest RoFormer checkpoints. citeturn16view5turn33search0turn33search5turn33search9

The fourth risk is **output inconsistency between model families**. Some models are true 4-stem, some 2-stem, some 5-stem or 6-stem, and some “multi-stem” challenge models are really specialized target models. The mitigation is to make the plugin’s export contract versioned and explicit. Do not pretend every model produces the same file schema. Instead, define per-tier schemas such as `standard-4`, `karaoke-2`, and `extended-6`.

The fifth risk is **over-optimizing for absolute leaderboard quality too early**. The current best open local quality often lives in community-trained RoFormer checkpoints, but that comes with operational cost. The mitigation is to ship a stable v1 on Demucs, then add an “experimental high-detail” tier later once you have production telemetry and a frozen tested checkpoint set.

## Open questions and limitations

A few things remain less settled than I would like.

I have high confidence in the **quality direction** of the current RoFormer ecosystem, but lower confidence in the **long-term packaging standard** for community checkpoints. The open ecosystem still does not center on one clean canonical inference stack the way Demucs does.

I also found strong evidence for **BS PolarFormer** and the new **53-stem BS-RoFormer-based release**, but both are too fresh for me to recommend as your first shipping implementation without a separate hardening cycle. citeturn31view0turn34search2turn34search0

Finally, the exact CLI shape for every newer community wrapper is not yet standardized enough that I would build your production contract around it. That is another reason the Demucs-first approach is the safer v1 decision.