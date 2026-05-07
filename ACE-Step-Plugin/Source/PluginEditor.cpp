#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "Engine/AceStepCApi.h"
#include "Models/ModelDiscovery.h"
#include "Models/ModelSetupState.h"

namespace
{

constexpr auto kEditorWidth = 520;
constexpr auto kEditorHeightBase = 322;
constexpr auto kEditorHeightWithSetup = 452;

juce::String formatDownloadSize(juce::int64 bytes)
{
    if (bytes <= 0)
        return "size unavailable";

    return juce::File::descriptionOfSizeInBytes(bytes);
}

} // namespace

AceStepAudioProcessorEditor::AceStepAudioProcessorEditor(AceStepAudioProcessor& processor)
    : AudioProcessorEditor(&processor),
      audioProcessor(processor),
      presetBrowserModel(acestep_plugin::PresetStore::getDefaultPresetDirectory())
{
    titleLabel.setText("ACE-Step", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    captureSourceLabel.setText("Capture Source: Host input (current routing)",
        juce::dontSendNotification);
    captureSourceLabel.setJustificationType(juce::Justification::centredLeft);
    captureSourceLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    captureSourceLabel.setFont(juce::FontOptions(15.0f));
    addAndMakeVisible(captureSourceLabel);

    armButton.setButtonText("Arm");
    armButton.onClick = [this] {
        audioProcessor.setReferenceCaptureEnabled(armButton.getToggleState());
    };
    addAndMakeVisible(armButton);

    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] {
        audioProcessor.requestReferenceClear();
        meterLevel = 0.0f;
        repaint();
    };
    addAndMakeVisible(clearButton);

    statusLabel.setText("Audio passes through unchanged.", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    statusLabel.setFont(juce::FontOptions(15.0f));
    addAndMakeVisible(statusLabel);

    modelSetupHeadingLabel.setJustificationType(juce::Justification::centredLeft);
    modelSetupHeadingLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    modelSetupHeadingLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    addChildComponent(modelSetupHeadingLabel);

    modelDestinationLabel.setJustificationType(juce::Justification::centredLeft);
    modelDestinationLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    modelDestinationLabel.setFont(juce::FontOptions(13.0f));
    addChildComponent(modelDestinationLabel);

    modelDownloadSizeLabel.setJustificationType(juce::Justification::centredLeft);
    modelDownloadSizeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    modelDownloadSizeLabel.setFont(juce::FontOptions(13.0f));
    addChildComponent(modelDownloadSizeLabel);

    modelSetupButton.setButtonText("Set Up Models");
    addChildComponent(modelSetupButton);

    refreshModelSetupPanel();
    presetBrowserPanel.setOnSave([this] {
        if (const auto& preset = presetBrowserModel.getCurrentPreset())
        {
            const auto result = presetBrowserModel.savePreset(*preset);
            statusLabel.setText(result.success ? "Preset saved." : result.errorMessage,
                juce::dontSendNotification);
            refreshPresetBrowser();
        }
    });
    presetBrowserPanel.setOnLoad([this] { loadSelectedPreset(); });
    presetBrowserPanel.setOnRename([this] {
        const auto presetId = presetBrowserPanel.getSelectedPresetId();
        const auto newName = presetBrowserPanel.getRenameText();
        if (presetId.isEmpty() || newName.isEmpty())
        {
            statusLabel.setText("Select a preset and enter a new name.",
                juce::dontSendNotification);
            return;
        }

        const auto result = presetBrowserModel.renamePreset(presetId, newName);
        statusLabel.setText(result.success ? "Preset renamed." : result.errorMessage,
            juce::dontSendNotification);
        refreshPresetBrowser();
    });
    presetBrowserPanel.setOnDelete([this] {
        const auto presetId = presetBrowserPanel.getSelectedPresetId();
        if (presetId.isNotEmpty())
        {
            const auto result = presetBrowserModel.deletePreset(presetId);
            statusLabel.setText(result.success ? "Preset deleted." : result.errorMessage,
                juce::dontSendNotification);
            refreshPresetBrowser();
        }
    });
    addAndMakeVisible(presetBrowserPanel);
    refreshPresetBrowser();

    startTimerHz(30);
    setSize(kEditorWidth, showModelSetup ? kEditorHeightWithSetup : kEditorHeightBase);
}

void AceStepAudioProcessorEditor::refreshModelSetupPanel()
{
    const auto paths = acestep_plugin::AceStepCApi::getDefaultBackendPaths();
    const auto missingModels = acestep_plugin::ModelDiscovery::getMissingModelFilenames();

    showModelSetup = !missingModels.empty();

    if (showModelSetup)
    {
        const auto statusText = juce::String("Model setup required: ")
            + juce::String(static_cast<int>(missingModels.size()))
            + " required model file(s) missing.";

        statusLabel.setText(statusText, juce::dontSendNotification);
        modelSetupHeadingLabel.setText(statusText, juce::dontSendNotification);
        modelDestinationLabel.setText("Destination: " + paths.modelsDirectory.getFullPathName(),
            juce::dontSendNotification);
        modelDownloadSizeLabel.setText(
            "Approximate download: "
                + formatDownloadSize(acestep_plugin::ModelDiscovery::getMissingTotalBytes()),
            juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText("Models ready. Audio passes through unchanged.",
            juce::dontSendNotification);
    }

    modelSetupHeadingLabel.setVisible(showModelSetup);
    modelDestinationLabel.setVisible(showModelSetup);
    modelDownloadSizeLabel.setVisible(showModelSetup);
    modelSetupButton.setVisible(showModelSetup);
}

void AceStepAudioProcessorEditor::refreshPresetBrowser()
{
    const auto result = presetBrowserModel.refresh();
    if (result.success)
    {
        presetBrowserPanel.setPresets(presetBrowserModel.getPresets());

        if (result.errorMessage.isNotEmpty())
            statusLabel.setText(result.errorMessage, juce::dontSendNotification);
    }
    else
    {
        presetBrowserPanel.setPresets({});
        statusLabel.setText(result.errorMessage, juce::dontSendNotification);
    }
}

void AceStepAudioProcessorEditor::loadSelectedPreset()
{
    const auto presetId = presetBrowserPanel.getSelectedPresetId();
    if (presetId.isEmpty())
        return;

    const auto result = presetBrowserModel.loadPreset(presetId);
    if (result.success)
    {
        presetBrowserPanel.setRenameText(result.preset.name);
        presetBrowserPanel.setSaveEnabled(true);
    }

    statusLabel.setText(
        result.success ? "Preset loaded: " + result.preset.name
                + " (" + result.preset.request.prompt + ")"
            : result.errorMessage,
        juce::dontSendNotification);
}

void AceStepAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colour(0xff171a1f));
    graphics.setColour(juce::Colour(0xff2f6f73));
    graphics.fillRect(getLocalBounds().removeFromTop(4));

    auto meterBounds = getLocalBounds().reduced(24).removeFromBottom(48);
    graphics.setColour(juce::Colour(0xff252b31));
    graphics.fillRoundedRectangle(meterBounds.toFloat(), 4.0f);

    auto filled = meterBounds.withWidth(
        juce::roundToInt(static_cast<float>(meterBounds.getWidth()) * meterLevel));
    graphics.setColour(juce::Colour(0xff47b3a9));
    graphics.fillRoundedRectangle(filled.toFloat(), 4.0f);
}

void AceStepAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(24);
    titleLabel.setBounds(bounds.removeFromTop(40));
    captureSourceLabel.setBounds(bounds.removeFromTop(28));

    auto controls = bounds.removeFromTop(36);
    armButton.setBounds(controls.removeFromLeft(96));
    clearButton.setBounds(controls.removeFromLeft(96).reduced(8, 0));

    statusLabel.setBounds(bounds.removeFromTop(28));
    presetBrowserPanel.setBounds(bounds.removeFromTop(102));

    if (showModelSetup)
    {
        bounds.removeFromTop(8);
        modelSetupHeadingLabel.setBounds(bounds.removeFromTop(24));
        modelDestinationLabel.setBounds(bounds.removeFromTop(22));
        modelDownloadSizeLabel.setBounds(bounds.removeFromTop(22));
        bounds.removeFromTop(8);
        modelSetupButton.setBounds(bounds.removeFromTop(36).removeFromLeft(160));
    }
}

void AceStepAudioProcessorEditor::timerCallback()
{
    const auto nextPeak = juce::jlimit(0.0f, 1.0f, audioProcessor.consumeReferencePeak());
    meterLevel = std::max(nextPeak, meterLevel * 0.82f);
    repaint();
}
