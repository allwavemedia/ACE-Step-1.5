#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "Generation/GenerationFormPanel.h"
#include "Presets/PresetBrowserModel.h"
#include "UI/PresetBrowserPanel.h"

class AceStepAudioProcessor;

class AceStepAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit AceStepAudioProcessorEditor(AceStepAudioProcessor& processor);
    ~AceStepAudioProcessorEditor() override = default;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshModelSetupPanel();
    void refreshPresetBrowser();
    void loadSelectedPreset();
    void updateCaptureMeterText();

    AceStepAudioProcessor& audioProcessor;
    acestep_plugin::PresetBrowserModel presetBrowserModel;
    juce::Component scrollContent;
    juce::Viewport scrollViewport;
    juce::Label titleLabel;
    juce::Label setupHeadingLabel;
    juce::Label generationHeadingLabel;
    juce::Label captureHeadingLabel;
    juce::Label generatedAssetsHeadingLabel;
    juce::Label diagnosticsHeadingLabel;
    juce::Label captureSourceLabel;
    juce::Label statusLabel;
    juce::ToggleButton armButton;
    juce::TextButton clearButton;
    juce::Label captureMeterLabel;
    juce::Label generatedAssetsBodyLabel;
    juce::Label exportStatusLabel;
    float meterLevel = 0.0f;

    // Model setup panel (visible only when required models are missing)
    juce::Label modelSetupHeadingLabel;
    juce::Label modelDestinationLabel;
    juce::Label modelDownloadSizeLabel;
    juce::TextButton modelSetupButton;
    bool showModelSetup = false;

    acestep_plugin::PresetBrowserPanel presetBrowserPanel;
    acestep_plugin::GenerationFormPanel generationFormPanel;
    juce::Label diagnosticsBodyLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AceStepAudioProcessorEditor)
};
