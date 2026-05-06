#include "PresetBrowserPanel.h"

namespace acestep_plugin
{

PresetBrowserPanel::PresetBrowserPanel()
{
    presetHeadingLabel.setText("Presets", juce::dontSendNotification);
    presetHeadingLabel.setJustificationType(juce::Justification::centredLeft);
    presetHeadingLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    presetHeadingLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));

    presetFilterBox.setTextToShowWhenEmpty("Filter presets", juce::Colours::grey);
    presetFilterBox.onTextChange = [this] { applyFilter(); };

    addAndMakeVisible(presetHeadingLabel);
    addAndMakeVisible(presetFilterBox);
    addAndMakeVisible(presetListBox);
    addAndMakeVisible(presetSaveButton);
    addAndMakeVisible(presetLoadButton);
    addAndMakeVisible(presetRenameButton);
    addAndMakeVisible(presetDeleteButton);
}

void PresetBrowserPanel::setPresets(const std::vector<GenerationPreset>& presets)
{
    allPresets = presets;
    applyFilter();
}

void PresetBrowserPanel::setFilterText(const juce::String& filterText)
{
    presetFilterBox.setText(filterText, false);
    applyFilter();
}

void PresetBrowserPanel::applyFilter()
{
    presetListBox.clear(juce::dontSendNotification);
    presetIds.clear();

    const auto filter = presetFilterBox.getText().trim();
    int itemId = 1;
    for (const auto& preset : allPresets)
    {
        const auto displayName = preset.name.isNotEmpty() ? preset.name : preset.id;
        if (filter.isNotEmpty() && !displayName.containsIgnoreCase(filter))
            continue;

        presetListBox.addItem(displayName, itemId++);
        presetIds.push_back(preset.id);
    }

    if (!presetIds.empty())
        presetListBox.setSelectedItemIndex(0, juce::dontSendNotification);
}

int PresetBrowserPanel::getPresetCount() const
{
    return presetListBox.getNumItems();
}

juce::String PresetBrowserPanel::getSelectedPresetId() const
{
    const auto selectedIndex = presetListBox.getSelectedItemIndex();
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(presetIds.size()))
        return {};

    return presetIds[static_cast<size_t>(selectedIndex)];
}

void PresetBrowserPanel::setOnSave(ActionCallback callback)
{
    presetSaveButton.onClick = std::move(callback);
}

void PresetBrowserPanel::setOnLoad(ActionCallback callback)
{
    presetLoadButton.onClick = std::move(callback);
}

void PresetBrowserPanel::setOnRename(ActionCallback callback)
{
    presetRenameButton.onClick = std::move(callback);
}

void PresetBrowserPanel::setOnDelete(ActionCallback callback)
{
    presetDeleteButton.onClick = std::move(callback);
}

void PresetBrowserPanel::resized()
{
    auto bounds = getLocalBounds();
    presetHeadingLabel.setBounds(bounds.removeFromTop(22));
    presetFilterBox.setBounds(bounds.removeFromTop(24));

    auto row = bounds.removeFromTop(28);
    presetListBox.setBounds(row.removeFromLeft(170));
    presetLoadButton.setBounds(row.removeFromLeft(56).reduced(4, 0));
    presetSaveButton.setBounds(row.removeFromLeft(96).reduced(4, 0));
    presetRenameButton.setBounds(row.removeFromLeft(74).reduced(4, 0));
    presetDeleteButton.setBounds(row.removeFromLeft(70).reduced(4, 0));
}

} // namespace acestep_plugin
