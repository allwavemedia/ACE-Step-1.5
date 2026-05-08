#include "GenerationFormPanel.h"

namespace acestep_plugin
{

GenerationFormPanel::GenerationFormPanel()
{
    promptEditor.setTextToShowWhenEmpty("Describe the music style and mood...", juce::Colours::grey);

    lyricsEditor.setTextToShowWhenEmpty("Optional lyrics...", juce::Colours::grey);
    lyricsEditor.setMultiLine(true, true);
    lyricsEditor.setReturnKeyStartsNewLine(true);

    durationSlider.setRange(1.0, 300.0, 1.0);
    durationSlider.setValue(30.0, juce::dontSendNotification);
    durationSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 22);

    seedEditor.setText("-1", false);
    seedEditor.setInputRestrictions(11, "-0123456789");

    cfgScaleSlider.setRange(1.0, 20.0, 0.5);
    cfgScaleSlider.setValue(7.0, juce::dontSendNotification);
    cfgScaleSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 22);

    lmSeedEditor.setText("-1", false);
    lmSeedEditor.setInputRestrictions(11, "-0123456789");

    schedulerCombo.addItem("euler",    1);
    schedulerCombo.addItem("dpmpp_2m", 2);
    schedulerCombo.addItem("dpmpp_3m", 3);
    schedulerCombo.setSelectedItemIndex(0, juce::dontSendNotification);

    cancelButton.setEnabled(false);

    generateButton.onClick = [this] { onGenerateClicked(); };
    cancelButton.onClick   = [this] { if (onCancel) onCancel(); };

    addAndMakeVisible(promptLabel);
    addAndMakeVisible(promptEditor);
    addAndMakeVisible(lyricsLabel);
    addAndMakeVisible(lyricsEditor);
    addAndMakeVisible(durationLabel);
    addAndMakeVisible(durationSlider);
    addAndMakeVisible(seedLabel);
    addAndMakeVisible(seedEditor);
    addAndMakeVisible(cfgScaleLabel);
    addAndMakeVisible(cfgScaleSlider);
    addAndMakeVisible(lmSeedLabel);
    addAndMakeVisible(lmSeedEditor);
    addAndMakeVisible(schedulerLabel);
    addAndMakeVisible(schedulerCombo);
    addAndMakeVisible(capturedReferenceToggle);
    addAndMakeVisible(generateButton);
    addAndMakeVisible(cancelButton);
}

GenerationFormState GenerationFormPanel::getState() const
{
    GenerationFormState state;
    state.prompt = promptEditor.getText().trim();
    state.lyrics = lyricsEditor.getText();
    state.durationSeconds = static_cast<float>(durationSlider.getValue());
    state.seed = seedEditor.getText().trim().getIntValue();
    state.cfgScale = static_cast<float>(cfgScaleSlider.getValue());
    state.lmSeed = lmSeedEditor.getText().trim().getIntValue();

    const int idx = schedulerCombo.getSelectedItemIndex();
    if (idx >= 0)
        state.scheduler = schedulerCombo.getItemText(idx);

    state.useCapturedReference = capturedReferenceToggle.getToggleState();
    return state;
}

void GenerationFormPanel::setState(const GenerationFormState& state)
{
    promptEditor.setText(state.prompt, false);
    lyricsEditor.setText(state.lyrics, false);
    durationSlider.setValue(static_cast<double>(state.durationSeconds), juce::dontSendNotification);
    seedEditor.setText(juce::String(state.seed), false);
    cfgScaleSlider.setValue(static_cast<double>(state.cfgScale), juce::dontSendNotification);
    lmSeedEditor.setText(juce::String(state.lmSeed), false);

    for (int i = 0; i < schedulerCombo.getNumItems(); ++i)
    {
        if (schedulerCombo.getItemText(i) == state.scheduler)
        {
            schedulerCombo.setSelectedItemIndex(i, juce::dontSendNotification);
            break;
        }
    }

    capturedReferenceToggle.setToggleState(state.useCapturedReference, juce::dontSendNotification);
}

void GenerationFormPanel::setGenerating(bool isGenerating)
{
    generateButton.setEnabled(!isGenerating);
    cancelButton.setEnabled(isGenerating);
}

bool GenerationFormPanel::isGenerateEnabled() const
{
    return generateButton.isEnabled();
}

void GenerationFormPanel::setOnGenerate(std::function<void(const GenerationFormState&)> callback)
{
    onGenerate = std::move(callback);
}

void GenerationFormPanel::setOnCancel(std::function<void()> callback)
{
    onCancel = std::move(callback);
}

void GenerationFormPanel::simulateGenerate()
{
    onGenerateClicked();
}

void GenerationFormPanel::simulateCancel()
{
    if (onCancel)
        onCancel();
}

void GenerationFormPanel::onGenerateClicked()
{
    const auto state = getState();
    if (state.validate().isNotEmpty())
        return;

    if (onGenerate)
        onGenerate(state);
}

void GenerationFormPanel::resized()
{
    auto bounds = getLocalBounds().reduced(8);

    const int labelH    = 18;
    const int controlH  = 24;
    const int multiLineH = 80;
    const int gap       = 4;

    auto addRow = [&](juce::Component& label, juce::Component& control, int h)
    {
        label.setBounds(bounds.removeFromTop(labelH));
        bounds.removeFromTop(2);
        control.setBounds(bounds.removeFromTop(h));
        bounds.removeFromTop(gap);
    };

    addRow(promptLabel,    promptEditor,    controlH);
    addRow(lyricsLabel,    lyricsEditor,    multiLineH);
    addRow(durationLabel,  durationSlider,  controlH);
    addRow(seedLabel,      seedEditor,      controlH);
    addRow(cfgScaleLabel,  cfgScaleSlider,  controlH);
    addRow(lmSeedLabel,    lmSeedEditor,    controlH);
    addRow(schedulerLabel, schedulerCombo,  controlH);

    capturedReferenceToggle.setBounds(bounds.removeFromTop(24));
    bounds.removeFromTop(gap);

    auto buttonRow = bounds.removeFromTop(30);
    generateButton.setBounds(buttonRow.removeFromLeft(100).reduced(2, 0));
    cancelButton.setBounds(buttonRow.removeFromLeft(80).reduced(2, 0));
}

} // namespace acestep_plugin
