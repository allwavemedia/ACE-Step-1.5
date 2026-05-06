#pragma once

#include "PresetStore.h"

#include <functional>
#include <optional>

namespace acestep_plugin
{

/** UI-facing preset browser state that never starts generation during load. */
class PresetBrowserModel final
{
public:
    using GenerationSubmitCallback = std::function<void(const GenerationRequest&)>;

    explicit PresetBrowserModel(juce::File presetDirectory);

    PresetListResult refresh();
    PresetOperationResult savePreset(const GenerationPreset& preset);
    PresetLoadResult loadPreset(const juce::String& presetId);
    PresetOperationResult renamePreset(const juce::String& presetId, const juce::String& newName);
    PresetOperationResult deletePreset(const juce::String& presetId);

    void setGenerationSubmitCallback(GenerationSubmitCallback callback);
    const std::optional<GenerationPreset>& getCurrentPreset() const noexcept;
    const std::optional<GenerationRequest>& getCurrentRequest() const noexcept;
    const std::vector<GenerationPreset>& getPresets() const noexcept;

private:
    PresetStore store;
    std::vector<GenerationPreset> presets;
    std::optional<GenerationPreset> currentPreset;
    std::optional<GenerationRequest> currentRequest;
    GenerationSubmitCallback generationSubmitCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserModel)
};

} // namespace acestep_plugin
