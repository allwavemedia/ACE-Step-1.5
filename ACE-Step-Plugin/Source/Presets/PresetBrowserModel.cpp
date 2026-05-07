#include "PresetBrowserModel.h"

namespace acestep_plugin
{

PresetBrowserModel::PresetBrowserModel(juce::File presetDirectory)
    : store(std::move(presetDirectory))
{
}

PresetListResult PresetBrowserModel::refresh()
{
    auto result = store.list();
    if (result.success)
        presets = result.presets;

    return result;
}

PresetOperationResult PresetBrowserModel::savePreset(const GenerationPreset& preset)
{
    const auto result = store.save(preset);
    if (result.success)
        refresh();

    return result;
}

PresetLoadResult PresetBrowserModel::loadPreset(const juce::String& presetId)
{
    auto result = store.load(presetId);
    if (result.success)
    {
        currentPreset = result.preset;
        currentRequest = result.preset.request;
    }

    return result;
}

PresetOperationResult PresetBrowserModel::renamePreset(
    const juce::String& presetId, const juce::String& newName)
{
    const auto result = store.rename(presetId, newName);
    if (result.success)
        refresh();

    return result;
}

PresetOperationResult PresetBrowserModel::deletePreset(const juce::String& presetId)
{
    const auto result = store.deletePreset(presetId);
    if (result.success)
        refresh();

    return result;
}

void PresetBrowserModel::setGenerationSubmitCallback(GenerationSubmitCallback callback)
{
    generationSubmitCallback = std::move(callback);
}

const std::optional<GenerationPreset>& PresetBrowserModel::getCurrentPreset() const noexcept
{
    return currentPreset;
}

const std::optional<GenerationRequest>& PresetBrowserModel::getCurrentRequest() const noexcept
{
    return currentRequest;
}

const std::vector<GenerationPreset>& PresetBrowserModel::getPresets() const noexcept
{
    return presets;
}

} // namespace acestep_plugin
