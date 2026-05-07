#pragma once

#include "PresetTypes.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

/** Stores generation presets as one JSON file per preset. */
class PresetStore final
{
public:
    explicit PresetStore(juce::File presetDirectory);

    static juce::File getDefaultPresetDirectory();

    PresetOperationResult save(const GenerationPreset& preset) const;
    PresetLoadResult load(const juce::String& presetId) const;
    PresetListResult list() const;
    PresetOperationResult rename(const juce::String& presetId, const juce::String& newName) const;
    PresetOperationResult deletePreset(const juce::String& presetId) const;

private:
    juce::File getPresetFile(const juce::String& presetId) const;
    PresetOperationResult validatePresetId(const juce::String& presetId) const;

    juce::File directory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetStore)
};

} // namespace acestep_plugin
