#pragma once

#include "../Stems/StemTypes.h"

#include <juce_core/juce_core.h>

#include <optional>
#include <vector>

namespace acestep_plugin
{

/** All parameters needed to produce one piece of music.
 *
 *  Field defaults are chosen so a caller only needs to set prompt and outputPath
 *  for a minimal generation; all other fields are optional or have sensible
 *  defaults that mirror the ACE-Step CLI defaults.
 */
struct GenerationRequest
{
    /** Text description of the desired music style and mood. */
    juce::String prompt;

    /** Optional lyrics / vocal lines to embed in the output. */
    juce::String lyrics;

    /** Target output duration in seconds. Default: 30. */
    float durationSeconds = 30.0f;

    /** RNG seed for the diffusion sampler.  -1 = random. */
    int seed = -1;

    /** Classifier-free guidance scale.  Default: 7.0. */
    float cfgScale = 7.0f;

    /** RNG seed for the language model planner.  -1 = random. */
    int lmSeed = -1;

    /** Diffusion scheduler name, e.g. "euler" or "dpmpp". */
    juce::String scheduler { "euler" };

    /** Optional path to a captured reference audio WAV file. */
    std::optional<juce::String> referenceAudioPath;

    /** Whether stem outputs should be requested when the backend supports them. */
    bool stemsEnabled = false;

    /** Stem groups requested by the user; empty means full mix only. */
    std::vector<StemGroup> requestedStemGroups;

    /** Absolute path where the output WAV should be written. */
    juce::String outputPath;
};

/** Outcome of a completed generation attempt. */
struct GenerationResult
{
    bool success = false;

    /** Absolute path to the written WAV file on success; empty otherwise. */
    juce::String outputPath;

    /** Human-readable error description on failure; empty on success. */
    juce::String errorMessage;
};

} // namespace acestep_plugin
