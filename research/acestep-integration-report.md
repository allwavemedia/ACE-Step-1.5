# ACE-Step and acestep.cpp Research for a JUCE VST3 Plugin

## Executive assessment

As of May 7, 2026, the strongest, highest-confidence conclusion is that **prompt-to-WAV generation is ready to expose in a JUCE plugin using vendored acestep.cpp**, but **native stem-oriented features are not yet clean enough to be treated as release-grade primary workflows**. The current public code/docs show a capable native C++ core, multiple CLIs, and HTTP server surfaces for LM enrichment, synthesis, audio understanding, and latent encode/decode. They also show real native task modes for `cover`, `cover-nofsq`, `repaint`, `lego`, `extract`, and `complete`. However, the public materials are also internally inconsistent in a few important places, including HTTP shape, model-name formatting, and whether some model-selection fields are formally part of `AceRequest`. That makes a **plugin-owned typed boundary** safer than exposing raw JSON one-for-one. citeturn8view0turn15view0turn15view1turn31view0turn37view0turn45view0turn45view3turn38view4

For **v1**, I recommend this split:

- **WAV generation:** use **native acestep.cpp APIs in a background worker** if you already have a working vendored integration.
- **Process isolation option:** keep a **local helper sidecar** available as a fallback or “safe mode,” especially for crash containment and multi-instance sharing.
- **Stem outputs:** use **external post-processing tools** for release-grade stem separation, and gate ACE-Step native stem modes behind an **experimental** flag until you validate them on your vendored commit and chosen models.
- **MIDI export:** use **external AMT/transcription**, not ACE-Step’s native tokens/latents, because I did not find a documented native note/onset/event/MIDI surface in either acestep.cpp or the official ACE-Step inference docs I inspected. citeturn37view0turn39view5turn20view0turn20view2turn23search0turn42search0turn42search1

## Codebase state and exposed surfaces

The public ecosystem currently has three relevant code/doc surfaces:

The first is **`ServeurpersoCom/acestep.cpp`**, which is the richest technical source I found for the native C++ backend. Its build system creates a shared core library plus `ace-lm`, `ace-synth`, `ace-server`, `ace-understand`, `neural-codec`, `mp3-codec`, and `quantize`. It also exposes current C++ headers for requests, LM generation, model-store management, synth jobs, and audio buffers. citeturn8view0turn15view0turn15view1turn31view0

The second is the **official `ace-step/acestep.cpp` fork**, which is public and active, but it appears to lag upstream in places. Its docs still show older shapes and wording in some areas, although they clearly document the native creative task modes and task compatibility tables. citeturn33view0turn35view0

The third is **`ace-step/acestep.vst3`**, the official VST3 fork/repo. Its README documents a JUCE VST3 plugin plus an HTTP/server workflow, but it still describes an older, simpler HTTP surface with synchronous `/lm`, `/synth`, `/synth?wav=1`, and `/understand`, while the newer upstream architecture docs describe an asynchronous queued server with `/job`, `/vae`, `/logs`, and multipart mixed results. In practice, that means you should **pin your vendored commit first**, and let the plugin talk to *that* exact boundary only. Do not code against the public docs as if they were fully stable across repos. citeturn28view0turn38view3turn38view0

At the native C++ layer, the exposed building blocks are clear enough to support an in-process plugin worker:

- `request_init`, `request_parse`, `request_parse_json`, `request_to_json`, `request_resolve_seed`
- `ace_lm_default_params`, `ace_lm_load`, `ace_lm_generate`, `ace_lm_free`
- `store_create`, `store_free`, typed `store_require_*` accessors, and VRAM eviction policies
- `ace_synth_default_params`, `ace_synth_job_run_dit`, `ace_synth_job_get_latent`, `ace_synth_job_run_vae`, `ace_audio_free`, `ace_synth_free` citeturn46view0turn15view1turn31view0turn15view0

One especially important architectural point for plugin design is that the newer server already serializes heavy work through **one worker thread and one FIFO queue**, while `ModelStore` serializes model load/unload decisions behind a mutex and explicitly tries to avoid duplicate LM residency. That is a strong signal that your plugin should also use **a single backend service object per process or per sidecar**, rather than per-plugin-instance model stacks. citeturn38view0turn39view2turn31view0

## Capability matrix

| Capability | Native entry points | Output | Model constraints | Recommendation |
|---|---|---|---|---|
| Prompt → WAV full generation | `ace-lm` + `ace-synth`; newer server `/lm` + `/synth`; native `ace_lm_generate` + synth job API | WAV/MP3, stereo 48 kHz | LM + text encoder + DiT + VAE required for caption-only/simple prompting; DiT+VAE alone can still run direct synth/passthrough flows | **Ship in v1**. This is the cleanest, strongest native path. citeturn17view4turn36view0turn15view1turn15view0turn25search0 |
| Prompt formatting / inspiration without rendering | `lm_mode="inspire"` or `lm_mode="format"` via `ace-lm` or `/lm` | Enriched metadata/lyrics, no audio | LM only | Useful as a plugin “plan” step before render. citeturn9view0turn15view1turn37view0 |
| Direct code/latent decode to audio | `ace-synth`, `/synth`, latent decode via `/vae` | WAV/MP3 or raw latent decode path | Text encoder + DiT + VAE; `audio_codes` skips LM | Good for cached/regenerated renders. citeturn36view3turn38view3turn15view0 |
| Cover / remix from source audio | `task_type="cover"` and `task_type="cover-nofsq"` | Full rendered audio | Requires source audio; LM skipped | Viable as an advanced feature; not core v1 if your current plugin only targets prompt generation. citeturn37view0turn35view0 |
| Repainting / partial edit / outpainting | `task_type="repaint"` | Full rendered audio with region edit | Requires source audio; LM skipped; negative start / long end support outpainting | Viable, but secondary to v1 prompt generation. citeturn37view0turn11view1 |
| Add a track in context | `task_type="lego"` | Generated track in context of source | **Base-model family only** in official docs; c++ warns turbo is incoherent | **Experimental only**. Output semantics are not solid enough for a release promise. citeturn37view0turn39view5turn20view2turn23search0 |
| Stem extraction from source audio | `task_type="extract"` | Intended isolated stem WAV | **Base/XL-base safest**; official Python docs call this base-only; c++ warns turbo is incoherent | **Do not make this your primary shipping stem path yet**. Good experimental/native option. citeturn37view0turn39view5turn20view0turn23search0 |
| Stem completion / accompaniment from isolated source | `task_type="complete"` | **Full mix**, not an isolated stem | **Base/XL-base safest**; official Python docs call this base-only; c++ warns turbo is incoherent | Useful creatively, but not a straightforward “stem output” API. citeturn37view0turn39view5turn20view0turn23search0 |
| Audio understanding | `ace-understand`, `/understand` | Metadata + lyrics + source latents / codes pipeline outputs | LM + synth-side tokenizer/codec pieces | Useful for round-tripping existing audio into editable requests. citeturn17view0turn17view4turn38view3 |
| Native note/onset/event/MIDI export | No documented native surface found in inspected docs | N/A | N/A | Use external AMT for v1 MIDI. Native ACE-Step gives semantic codes and latents, not musical note events. citeturn17view4turn37view0turn15view0turn42search0turn42search1 |

## Entry points, task modes, and exact examples

### CLI entry points

Current acestep.cpp documentation and build files expose these primary binaries:

- `ace-lm` for metadata + lyrics + audio-code generation
- `ace-synth` for text/audio-conditioned synthesis
- `ace-server` for an HTTP job API
- `ace-understand` for reverse analysis
- `neural-codec` for VAE encode/decode
- `mp3-codec` for MP3/WAV conversion
- `quantize` for GGUF quantization workflows citeturn8view0turn17view1

A current **prompt-to-WAV** flow, using the newer model-registry style documented in upstream acestep.cpp, looks like this:

```bash
cat > request.json << 'EOF'
{
  "caption": "Upbeat pop rock with driving guitars and catchy hooks",
  "vocal_language": "en",
  "output_format": "wav16",
  "lm_model": "acestep-5Hz-lm-4B-Q8_0.gguf",
  "synth_model": "acestep-v15-turbo-Q8_0.gguf"
}
EOF

./ace-lm --models ./models --request ./request.json
./ace-synth --models ./models --request ./request0.json
```

That pattern is directly supported by the newer `ace-lm --models <dir> --request <json>` and `ace-synth --models <dir> --request <json...>` references, along with `output_format` values `mp3`, `wav16`, `wav24`, and `wav32`. citeturn37view0turn38view3

A current **stem-extract experiment** from a source mix looks like this:

```bash
cat > extract.json << 'EOF'
{
  "task_type": "extract",
  "track": "vocals",
  "output_format": "wav16",
  "synth_model": "acestep-v15-base-Q8_0.gguf"
}
EOF

./ace-synth --models ./models --src-audio ./mix.wav --request ./extract.json
```

This matches the documented `extract` task semantics: source audio required, track required, intended isolated stem output, and safest operation on base-model families rather than turbo. citeturn37view0turn39view5turn20view0turn23search0

A current **complete-from-stem** experiment looks like this:

```bash
cat > complete.json << 'EOF'
{
  "task_type": "complete",
  "track": "drums",
  "caption": "tight modern pop-rock rhythm section",
  "lyrics": "[Instrumental]",
  "output_format": "wav16",
  "synth_model": "acestep-v15-base-Q8_0.gguf"
}
EOF

./ace-synth --models ./models --src-audio ./vocals_only.wav --request ./complete.json
```

This is a creative accompaniment/full-mix workflow, not a native isolated-drum export. The docs explicitly describe `complete` as generating a **full mix from a single isolated stem**, not a temporal continuation or a stem-only output. citeturn37view0

### HTTP/API entry points

This is where the public docs currently diverge.

The **older VST3/official README surface** describes:

- `GET /props`
- `POST /lm`
- `POST /synth`
- `POST /synth?wav=1`
- `POST /understand` citeturn28view0

The **newer upstream architecture docs** describe an **async job model**:

- `POST /lm`
- `POST /synth`
- `POST /understand`
- `POST /vae`
- `GET /job?id=N`
- `GET /job?id=N&result=1`
- `POST /job?id=N&cancel=1`
- `GET /health`
- `GET /props`
- `GET /logs` citeturn38view3turn38view0

That newer server surface is the more capable one for plugin-style background work, because it already gives you queuing, cancellation, result polling, latent transport, and model discovery. A representative request chain is:

```bash
# submit LM planning
curl -X POST http://127.0.0.1:8080/lm \
  -H 'Content-Type: application/json' \
  -d '{"caption":"cinematic synthwave with female vocals","output_format":"wav16"}'

# poll job
curl "http://127.0.0.1:8080/job?id=1"

# fetch LM result
curl "http://127.0.0.1:8080/job?id=1&result=1"

# then submit /synth with the enriched AceRequest JSON returned by /lm
```

The important integration point is not the exact `curl` syntax; it is that the current upstream HTTP server wants you to think in **queued jobs** plus **result retrieval**, not synchronous request/response generation. citeturn38view3turn38view0

### Native C++ API surface

The native C++ pieces I found are sufficient to build a plugin-owned service layer:

- `AceRequest` request struct plus helpers in `request.h`
- `AceLm` load/generate/free API in `pipeline-lm.h`
- `ModelStore` ownership/eviction/sharing in `model-store.h`
- `AceSynthJob` two-phase synthesis with cancel callbacks and latent retrieval in `pipeline-synth.h` citeturn46view0turn15view1turn31view0turn15view0

One especially useful output for future-proofing is `ace_synth_job_get_latent`, which exposes a flat `[T_latent * 64]` latent buffer matching the `/vae` wire format. That gives you a native place to build caching or offline re-decoding later, even if v1 only ships WAV output. citeturn15view0turn38view0

## Native stem and MIDI findings

The native task modes that matter for stem-related plugin goals are real:

- `extract` is intended to isolate a stem from a mixed source.
- `lego` is intended to generate a track in the context of a backing source.
- `complete` is intended to generate a full mix from an isolated stem input.
- `cover`, `cover-nofsq`, and `repaint` are source-audio-conditioned edit/remix flows rather than stem APIs. citeturn37view0turn35view0turn20view0turn20view2

The strongest caution is with **model compatibility**. Official ACE-Step 1.5 docs and model-zoo tables are stricter than some aquestep.cpp wording. Official Python docs label `lego`, `extract`, and `complete` as **base model only**, and the official model zoo shows support for those tasks on `acestep-v15-base` and `acestep-v15-xl-base`, while SFT and turbo variants are shown as unsupported for those modes. By contrast, the c++ docs often say “base/SFT” and the synth code appears to warn specifically when turbo is used. The safest practical interpretation for a plugin is: **treat stem-related native tasks as base/XL-base only until validated on your exact vendored commit**. citeturn20view0turn20view2turn23search0turn39view5

On whether native ACE-Step can produce usable stem WAVs, the answer is **partly yes, partly not yet trustworthy enough for a product promise**:

- **Yes for `extract`**, in the sense that the documented task is “isolate a specific stem from a mixed source.” That can be applied to any source audio, including ACE-Step-generated audio after the fact. citeturn37view0turn20view0
- **Maybe for `lego`**, but the newer c++ docs explicitly caution that the output may behave “analogous to stem generation” while the “output mix vs isolated stem is model-dependent and unverified in this codebase.” That is not a strong enough guarantee for a release-grade stem feature. citeturn37view0
- **No for `complete`** if your goal is “output the missing stem as stem WAV,” because the current docs describe a **full-mix regeneration conditioned on the input stem**, not an isolated accompaniment stem. citeturn37view0

On MIDI, the native ACE-Step surfaces I found expose:

- LM-generated **5 Hz audio semantic codes**
- VAE/DiT **latents**
- “understand” outputs like caption, lyrics, BPM, key, duration, and language
- LM debug token/logit dumps for text-side inspection citeturn17view4turn37view0turn15view0turn15view1

I did **not** find a documented native API for **notes, onsets, MIDI events, note probabilities, pianorolls, or aligned symbolic output**. Since the exposed `audio_codes` are semantic tokens at 5 Hz, they are not a direct note-event representation. That makes them potentially useful for caching, similarity, or regeneration, but **not a good primary v1 MIDI export substrate**. For production MIDI, purpose-built transcription remains the safer path. Spotify’s Basic Pitch still exposes direct MIDI output plus raw note-event CSV and programmatic `note_events`, which is a much better fit for plugin MIDI export than ACE-Step’s internal semantic tokens. citeturn17view4turn37view0turn42search0turn42search1

## Recommended plugin boundary design

The safest plugin boundary is a **typed, background-only service facade** that hides current codebase inconsistencies and keeps all heavy work off the audio thread.

A good shape for v1 is:

- **`GenerateRequest`**: caption, lyrics, metadata, output format, model choice, seed, batch counts
- **`EditRequest`**: task type, source audio reference, optional reference/timbre audio, repaint region, track, caption/lyrics
- **`UnderstandRequest`**: source audio reference, model choice
- **`BackendJob`**: queued/running/done/failed/cancelled, warnings, enriched request JSON, output WAV path, optional audio buffer, optional latent blob, optional `audio_codes`
- **`submit()` / `cancel()` / `poll()` / `collect()`** methods
- **One process-wide backend singleton** with a dedicated worker pool or, even safer, one serial worker matching the server’s own concurrency assumptions. citeturn38view0turn39view2turn31view0

For **prompt-to-WAV**, I recommend **native in-process execution first** if your vendored library already smoke-tests successfully. The reason is simple: you already have C++ integration, and the public native APIs expose exactly the LM and synth stages you need. That avoids HTTP encoding/decoding, reduces operational complexity, and gives you easier access to seeds, latents, cancellation callbacks, and result objects. citeturn15view1turn15view0turn8view0

For **process isolation**, I still recommend keeping a **local sidecar option** available. GGML backends, GPU driver interactions, and long-running heavy generation are all valid reasons to isolate from the DAW process. A sidecar is also a better fit if you want **one shared backend across multiple plugin instances**, centralized model discovery via `/props`, or job-level observability and cancellation via the async server endpoints. citeturn38view3turn38view0turn31view0

For **stems and MIDI**, I would not make ACE-Step native modes the primary v1 contract:

- **Stems:** use an external separator for shipping quality and deterministic expectations.
- **MIDI:** use external transcription.
- **Native `extract`/`lego`/`complete`:** expose only behind an “experimental” or “labs” feature group, and only when the selected model is base/XL-base. citeturn20view0turn20view2turn23search0turn42search0turn43search0turn44search0

If you want **one concrete recommendation by output type**, it is this:

- **WAV:** native acestep.cpp
- **MIDI:** external AMT/transcription
- **Stems:** external separator first, ACE-Step-native stems as experimental secondary path citeturn15view0turn42search0turn43search0turn44search0

## Required models, assets, validation notes, and failure surfacing

For a plugin that supports **caption/prompt → WAV**, the practical minimum asset set is:

| Asset type | Required for | Typical file examples |
|---|---|---|
| LM GGUF | Caption-only prompting, metadata/lyrics/code generation, understand | `acestep-5Hz-lm-0.6B-*`, `1.7B-*`, or `4B-*` |
| Text encoder GGUF | All synth work | `Qwen3-Embedding-0.6B-*.gguf` |
| DiT GGUF | All synth work | `acestep-v15-turbo-*`, `acestep-v15-base-*`, `acestep-v15-xl-base-*`, etc. |
| VAE GGUF | All encode/decode/render work | `vae-BF16.gguf` |
| Optional adapter directory | LoRA/adapters | safetensors or PEFT-style adapter folders | citeturn25search0turn37view0turn26search0 |

A key implementation convenience is that GGUF conversion currently bundles tokenizer/config-style runtime dependencies into the GGUF package itself, so you do **not** need to manage extra tokenizer files at runtime the way the original Python checkpoints do. citeturn28view0turn25search0

For **stem-related native experiments**, include an additional **base or XL-base DiT**. The official model zoo is the cleanest source here: `acestep-v15-base` and `acestep-v15-xl-base` are the safest documented choices for `extract`, `lego`, and `complete`. citeturn23search0

### Validation notes

Your v1 validation matrix should explicitly separate these cases:

- **Prompt-only caption → LM → synth → WAV**
- **Prompt + lyrics → LM → synth → WAV**
- **Passthrough `audio_codes` → synth only**
- **Cover / repaint with source audio**
- **Understand → editable request JSON**
- **Experimental `extract` / `lego` / `complete` on base models only** citeturn36view0turn37view0turn17view4

Also validate against the public doc inconsistencies before freezing your boundary. The current public materials conflict on at least three implementation details:

- whether `synth_model` / `lm_model` are part of `AceRequest`
- whether request-side model names should omit `.gguf`
- whether current server HTTP is synchronous or async-job-based citeturn46view0turn37view0turn45view0turn45view3turn28view0turn38view3

That is the strongest reason to keep the plugin boundary **strongly typed and normalized internally** instead of letting UI code compose raw request JSON directly.

### Failure modes and how the plugin should surface them

**Missing models or disabled pipelines.** The newer server docs say endpoints whose pipeline has no models in the registry return `501`, and `/props` is the source of truth for available models. Native load calls can also return `NULL` or fail when required paths are absent. Surface this as a **setup error** with a specific missing-asset list and a one-click “rescan models” action. citeturn39view0turn38view4turn39view4

**Invalid task input.** The synth path explicitly errors when a task requires source context but there is no source audio or codes, and it errors when repaint region end is not greater than start. Surface these as **preflight validation errors** before you enqueue. citeturn39view4

**Model/task mismatch.** The synth code warns that `lego`, `extract`, and `complete` on turbo are incoherent, while official ACE-Step 1.5 docs describe those tasks as base-only. Surface this as a **hard UI gate**, not a passive warning. If turbo is selected, disable those task modes. citeturn39view5turn20view0turn20view2turn23search0

**Queueing and cancellation.** The newer server is serial FIFO, cancellable, and retains only a limited pool of completed jobs. Surface states as **Queued / Running / Cancelling / Done / Failed**, and let users cancel long renders. If you implement in-process, mirror the same state machine. citeturn38view0turn39view2

**Oversize latent/audio payloads.** The newer server docs cap latent payload length at `T <= 15000` and return `413` if exceeded. Surface this as **duration/model limit exceeded** and recommend shorter durations or chunked workflows. citeturn39view3

**VRAM / backend failures.** The docs make clear that VAE chunk size and memory policy materially affect runtime footprint. Surface OOM-like failures as **resource errors** with suggested remediations: shorter duration, lower batch count, smaller LM, turbo model, or smaller VAE chunk. citeturn38view3turn39view0turn31view0

## Open questions and limitations

I did not inspect your vendored commit directly, so the final compiled signatures should come from **your checked-in headers**, not from public repo prose. That matters because the public materials are currently inconsistent in a few places. citeturn46view0turn15view0turn37view0

The biggest unresolved public-documentation question is **stem-mode maturity**. The official ACE-Step 1.5 docs are stricter than some aquestep.cpp wording, and the c++ docs themselves explicitly describe `lego` stem behavior as unverified. That is why I recommend **base/XL-base only** plus **experimental UI gating** for native stem tasks. citeturn37view0turn20view0turn20view2turn23search0

I also did not find a documented native note/onset/event API in the c++ or official ACE-Step inference docs I inspected. If such a surface exists in your vendored branch, it is not prominent in the public docs I reviewed. Based on the documented surfaces, **external MIDI transcription remains the safer and more practical v1 path**. citeturn17view4turn15view0turn42search0turn42search1