#pragma once

#include "../Engine/GenerationRequest.h"

#include <juce_core/juce_core.h>

#include <vector>

namespace acestep_plugin
{

/** User-saved generation settings with versioned JSON persistence. */
struct GenerationPreset
{
    int schemaVersion = 1;
    juce::String id;
    juce::String name;
    GenerationRequest request;
    bool midiExportRequested = false;
    bool stemExportRequested = false;
};

/** Basic success/failure result for preset write operations. */
struct PresetOperationResult
{
    bool success = false;
    juce::String errorMessage;
};

/** Result returned when loading a single preset. */
struct PresetLoadResult
{
    bool success = false;
    GenerationPreset preset;
    juce::String errorMessage;
};

/** Result returned when listing available presets. */
struct PresetListResult
{
    bool success = false;
    std::vector<GenerationPreset> presets;
    juce::String errorMessage;
};

} // namespace acestep_plugin
