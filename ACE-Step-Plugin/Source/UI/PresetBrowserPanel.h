#pragma once

#include "../Presets/PresetTypes.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

namespace acestep_plugin
{

/** Compact preset browser controls for listing and managing saved presets. */
class PresetBrowserPanel final : public juce::Component
{
public:
    using ActionCallback = std::function<void()>;

    PresetBrowserPanel();

    void setPresets(const std::vector<GenerationPreset>& presets);
    void setFilterText(const juce::String& filterText);
    void setRenameText(const juce::String& name);
    int getPresetCount() const;
    juce::String getSelectedPresetId() const;
    juce::String getRenameText() const;

    void setOnSave(ActionCallback callback);
    void setOnLoad(ActionCallback callback);
    void setOnRename(ActionCallback callback);
    void setOnDelete(ActionCallback callback);

    void resized() override;

private:
    void applyFilter();

    juce::Label presetHeadingLabel;
    juce::TextEditor presetFilterBox;
    juce::TextEditor presetRenameBox;
    juce::ComboBox presetListBox;
    juce::TextButton presetSaveButton { "Save Preset" };
    juce::TextButton presetLoadButton { "Load" };
    juce::TextButton presetRenameButton { "Rename" };
    juce::TextButton presetDeleteButton { "Delete" };
    std::vector<GenerationPreset> allPresets;
    std::vector<juce::String> presetIds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserPanel)
};

} // namespace acestep_plugin
