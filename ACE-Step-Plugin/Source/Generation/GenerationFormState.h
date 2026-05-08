#pragma once

#include "../Engine/GenerationRequest.h"

namespace acestep_plugin
{

/** Transient UI state backing the generation form.
 *
 *  Holds the user-editable fields and provides a validate/convert helper pair
 *  for handing off to the generation coordinator.
 */
struct GenerationFormState
{
    /** Text description of the desired music style and mood. */
    juce::String prompt;

    /** Optional lyrics / vocal lines to embed in the output. */
    juce::String lyrics;

    /** Target output duration in seconds.  Default: 30. */
    float durationSeconds = 30.0f;

    /** RNG seed for the diffusion sampler.  -1 = random. */
    int seed = -1;

    /** Classifier-free guidance scale.  Default: 7.0. */
    float cfgScale = 7.0f;

    /** RNG seed for the language model planner.  -1 = random. */
    int lmSeed = -1;

    /** Diffusion scheduler name, e.g. "euler" or "dpmpp_2m". */
    juce::String scheduler { "euler" };

    /** Whether to use a captured reference audio clip (resolved by coordinator). */
    bool useCapturedReference = false;

    /** Convert the form state into a GenerationRequest for the engine.
     *
     *  @param outputPath  Absolute path where the output WAV should be written.
     *  @return            A populated GenerationRequest.  useCapturedReference
     *                     is intentionally not mapped here; that is handled by
     *                     the Task-5 coordinator.
     */
    GenerationRequest toRequest(const juce::String& outputPath) const;

    /** Return a non-empty error string when the state is invalid, or empty when valid.
     *
     *  Checks: non-empty trimmed prompt and durationSeconds >= 1.0f.
     */
    juce::String validate() const;
};

inline GenerationRequest GenerationFormState::toRequest(const juce::String& outputPath) const
{
    GenerationRequest req;
    req.prompt = prompt;
    req.lyrics = lyrics;
    req.durationSeconds = durationSeconds;
    req.seed = seed;
    req.cfgScale = cfgScale;
    req.lmSeed = lmSeed;
    req.scheduler = scheduler;
    req.outputPath = outputPath;
    return req;
}

inline juce::String GenerationFormState::validate() const
{
    if (prompt.trim().isEmpty())
        return "Prompt must not be empty.";

    if (durationSeconds < 1.0f)
        return "Duration must be at least 1 second.";

    return {};
}

} // namespace acestep_plugin
