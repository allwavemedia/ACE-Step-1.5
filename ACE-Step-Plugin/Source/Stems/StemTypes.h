#pragma once

#include <juce_core/juce_core.h>

#include <vector>

namespace acestep_plugin
{

/** Whether stem generation/export can be trusted for a generated asset. */
enum class StemCapabilityState
{
    unavailable,
    available
};

/** Stem groups supported by the plugin-owned asset model. */
enum class StemGroup
{
    fullMix,
    vocals,
    drums,
    bass,
    other
};

/** Metadata for one full-mix or separated stem WAV asset. */
struct StemAsset
{
    StemGroup group = StemGroup::fullMix;
    juce::String outputPath;
    bool success = false;
    juce::String errorMessage;
};

} // namespace acestep_plugin
