#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "Engine/AceStepCApi.h"
#include "Models/ModelDiscovery.h"
#include "Models/ModelSetupState.h"

namespace
{

constexpr auto kEditorWidth = 520;
constexpr auto kEditorHeight = 640;
constexpr auto kContentPadding = 24;
constexpr auto kSectionGap = 14;
constexpr auto kHeadingHeight = 24;
constexpr auto kStatusHeight = 44;
constexpr auto kGenerationPanelHeight = 346;
constexpr auto kPresetPanelHeight = 112;
constexpr auto kAssetPanelHeight = 142;
constexpr auto kDiagnosticsHeight = 82;

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
    scrollViewport.setViewedComponent(&scrollContent, false);
    scrollViewport.setScrollBarsShown(true, false);
    scrollViewport.setName("Single-scroll editor viewport");
    scrollContent.setName("Single-scroll editor content");
    addAndMakeVisible(scrollViewport);

    auto configureHeading = [](juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    };

    configureHeading(setupHeadingLabel, "Setup");
    configureHeading(generationHeadingLabel, "Generation");
    configureHeading(captureHeadingLabel, "Capture");
    configureHeading(generatedAssetsHeadingLabel, "Generated Assets");
    configureHeading(diagnosticsHeadingLabel, "Diagnostics");
    setupHeadingLabel.setName("Setup section");
    generationHeadingLabel.setName("Generation section");
    captureHeadingLabel.setName("Capture section");
    generatedAssetsHeadingLabel.setName("Generated assets section");
    diagnosticsHeadingLabel.setName("Diagnostics section");

    scrollContent.addAndMakeVisible(titleLabel);
    scrollContent.addAndMakeVisible(setupHeadingLabel);
    scrollContent.addAndMakeVisible(generationHeadingLabel);
    scrollContent.addAndMakeVisible(captureHeadingLabel);
    scrollContent.addAndMakeVisible(generatedAssetsHeadingLabel);
    scrollContent.addAndMakeVisible(diagnosticsHeadingLabel);

    captureSourceLabel.setText("Capture Source: Host input (current routing)",
        juce::dontSendNotification);
    captureSourceLabel.setJustificationType(juce::Justification::centredLeft);
    captureSourceLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    captureSourceLabel.setFont(juce::FontOptions(15.0f));
    scrollContent.addAndMakeVisible(captureSourceLabel);

    armButton.setButtonText("Arm");
    armButton.setName("Arm reference capture");
    armButton.onClick = [this] {
        audioProcessor.setReferenceCaptureEnabled(armButton.getToggleState());
    };
    scrollContent.addAndMakeVisible(armButton);

    clearButton.setButtonText("Clear");
    clearButton.setName("Clear reference capture");
    clearButton.onClick = [this] {
        audioProcessor.requestReferenceClear();
        meterLevel = 0.0f;
        repaint();
    };
    scrollContent.addAndMakeVisible(clearButton);

    captureMeterLabel.setText("Meter: idle", juce::dontSendNotification);
    captureMeterLabel.setJustificationType(juce::Justification::centredLeft);
    captureMeterLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    captureMeterLabel.setFont(juce::FontOptions(13.0f));
    scrollContent.addAndMakeVisible(captureMeterLabel);

    statusLabel.setText("Audio passes through unchanged.", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    statusLabel.setFont(juce::FontOptions(15.0f));
    scrollContent.addAndMakeVisible(statusLabel);

    modelSetupHeadingLabel.setJustificationType(juce::Justification::centredLeft);
    modelSetupHeadingLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    modelSetupHeadingLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    scrollContent.addChildComponent(modelSetupHeadingLabel);

    modelDestinationLabel.setJustificationType(juce::Justification::centredLeft);
    modelDestinationLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    modelDestinationLabel.setFont(juce::FontOptions(13.0f));
    scrollContent.addChildComponent(modelDestinationLabel);

    modelDownloadSizeLabel.setJustificationType(juce::Justification::centredLeft);
    modelDownloadSizeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    modelDownloadSizeLabel.setFont(juce::FontOptions(13.0f));
    scrollContent.addChildComponent(modelDownloadSizeLabel);

    modelSetupButton.setButtonText("Set Up Models");
    scrollContent.addChildComponent(modelSetupButton);

    generationFormPanel.setOnGenerate([this](const acestep_plugin::GenerationFormState&) {
        statusLabel.setText("Generation sidecar wiring is pending.", juce::dontSendNotification);
    });
    generationFormPanel.setOnCancel([this] {
        statusLabel.setText("No active generation to cancel.", juce::dontSendNotification);
    });
    generationFormPanel.setName("Generation form");
    scrollContent.addAndMakeVisible(generationFormPanel);

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
    scrollContent.addAndMakeVisible(presetBrowserPanel);
    refreshPresetBrowser();

    generatedAssetsBodyLabel.setText(
        "Generated asset history is empty. Completed full-mix WAV assets will appear here.",
        juce::dontSendNotification);
    generatedAssetsBodyLabel.setJustificationType(juce::Justification::centredLeft);
    generatedAssetsBodyLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    generatedAssetsBodyLabel.setFont(juce::FontOptions(13.0f));
    generatedAssetsBodyLabel.setName("Generated asset history");
    scrollContent.addAndMakeVisible(generatedAssetsBodyLabel);

    exportStatusLabel.setName("Export actions");

    exportStatusLabel.setText(
        "Exports: WAV Save As/drag, MIDI export, and stem export appear per generated asset.",
        juce::dontSendNotification);
    exportStatusLabel.setJustificationType(juce::Justification::centredLeft);
    exportStatusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    exportStatusLabel.setFont(juce::FontOptions(13.0f));
    scrollContent.addAndMakeVisible(exportStatusLabel);

    diagnosticsBodyLabel.setText(
        "Sidecar: idle\nLast request: none\nArtifacts: none\nValidation failures: none",
        juce::dontSendNotification);
    diagnosticsBodyLabel.setJustificationType(juce::Justification::topLeft);
    diagnosticsBodyLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    diagnosticsBodyLabel.setFont(juce::FontOptions(13.0f));
    scrollContent.addAndMakeVisible(diagnosticsBodyLabel);

    startTimerHz(30);
    setSize(kEditorWidth, kEditorHeight);
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
}

void AceStepAudioProcessorEditor::resized()
{
    scrollViewport.setBounds(getLocalBounds());

    const int contentWidth = scrollViewport.getMaximumVisibleWidth() > 0
        ? scrollViewport.getMaximumVisibleWidth()
        : getWidth();

    auto bounds = juce::Rectangle<int>(0, 0, contentWidth, 1).reduced(kContentPadding, 0);
    int y = kContentPadding;

    auto nextArea = [&](int height)
    {
        auto area = juce::Rectangle<int>(bounds.getX(), y, bounds.getWidth(), height);
        y += height;
        return area;
    };

    auto addGap = [&] { y += kSectionGap; };

    titleLabel.setBounds(nextArea(40));
    addGap();

    setupHeadingLabel.setBounds(nextArea(kHeadingHeight));
    statusLabel.setBounds(nextArea(kStatusHeight));
    if (showModelSetup)
    {
        modelSetupHeadingLabel.setBounds(nextArea(24));
        modelDestinationLabel.setBounds(nextArea(22));
        modelDownloadSizeLabel.setBounds(nextArea(22));
        y += 8;
        modelSetupButton.setBounds(nextArea(36).removeFromLeft(160));
    }
    addGap();

    generationHeadingLabel.setBounds(nextArea(kHeadingHeight));
    generationFormPanel.setBounds(nextArea(kGenerationPanelHeight));
    addGap();

    captureHeadingLabel.setBounds(nextArea(kHeadingHeight));
    captureSourceLabel.setBounds(nextArea(28));
    auto controls = nextArea(36);
    armButton.setBounds(controls.removeFromLeft(96));
    clearButton.setBounds(controls.removeFromLeft(96).reduced(8, 0));
    captureMeterLabel.setText("Meter: "
                                  + juce::String(juce::roundToInt(meterLevel * 100.0f))
                                  + "%",
        juce::dontSendNotification);
    captureMeterLabel.setBounds(nextArea(24));
    addGap();

    generatedAssetsHeadingLabel.setBounds(nextArea(kHeadingHeight));
    generatedAssetsBodyLabel.setBounds(nextArea(kAssetPanelHeight - 28));
    exportStatusLabel.setBounds(nextArea(28));
    addGap();

    presetBrowserPanel.setBounds(nextArea(kPresetPanelHeight));
    addGap();

    diagnosticsHeadingLabel.setBounds(nextArea(kHeadingHeight));
    diagnosticsBodyLabel.setBounds(nextArea(kDiagnosticsHeight));
    y += kContentPadding;

    scrollContent.setSize(contentWidth, y);
}

void AceStepAudioProcessorEditor::timerCallback()
{
    const auto nextPeak = juce::jlimit(0.0f, 1.0f, audioProcessor.consumeReferencePeak());
    meterLevel = std::max(nextPeak, meterLevel * 0.82f);
    captureMeterLabel.setText("Meter: " + juce::String(juce::roundToInt(meterLevel * 100.0f)) + "%",
        juce::dontSendNotification);
    repaint();
}
