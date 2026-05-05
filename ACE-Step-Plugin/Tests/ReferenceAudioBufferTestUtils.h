#pragma once

#include "../Source/Models/GeneratedAssetHistory.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

/** Create a test GeneratedAsset with deterministic values. */
inline GeneratedAsset makeTestAsset(int index = 0)
{
    GeneratedAsset a;
    a.id = "test-asset-" + juce::String(index);
    a.outputPath = "/tmp/test-asset-" + juce::String(index) + ".wav";
    a.durationSeconds = 30.0f + static_cast<float>(index);
    a.timestamp = juce::Time(2025, 0, 1, 0, 0, index, 0);
    return a;
}

} // namespace acestep_plugin
