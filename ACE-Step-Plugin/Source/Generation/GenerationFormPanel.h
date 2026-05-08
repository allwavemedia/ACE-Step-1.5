#pragma once

#include "GenerationFormState.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace acestep_plugin
{

/** UI panel presenting all music-generation parameters.
 *
 *  Contains: prompt editor, lyrics editor, duration and CFG-scale sliders,
 *  seed and LM-seed text editors, scheduler combo, use-captured-reference
 *  toggle, Generate button, and Cancel button.
 *
 *  Use setState/getState to read and write the full form state.
 *  Use setOnGenerate/setOnCancel to register action callbacks.
 *  Use setGenerating to lock the form while a generation is in progress.
 */
class GenerationFormPanel final : public juce::Component
{
public:
    GenerationFormPanel();

    /** Read the current form field values as a GenerationFormState. */
    GenerationFormState getState() const;

    /** Populate all form fields from the supplied state. */
    void setState(const GenerationFormState& state);

    /** Disable Generate and enable Cancel while a generation is in progress;
     *  reverse when isGenerating is false.
     */
    void setGenerating(bool isGenerating);

    /** True when the Generate button is enabled. */
    bool isGenerateEnabled() const;

    /** Register a callback invoked with the current state when Generate is clicked
     *  and the state is valid.  Invalid forms silently suppress the callback.
     */
    void setOnGenerate(std::function<void(const GenerationFormState&)> callback);

    /** Register a callback invoked when Cancel is clicked. */
    void setOnCancel(std::function<void()> callback);

    /** Parse a seed field text string into a clamped integer.
     *
     *  Returns -1 for empty or whitespace input.
     *  Accepts an optional leading minus followed by digits only; any other
     *  pattern (e.g. "1-23", "--1", "abc") returns -1.
     *  Values below -1 are clamped to -1; values above INT_MAX are clamped to INT_MAX.
     */
    static int parseSeedText(const juce::String& text);

    /** Programmatically invoke the generate action.
     *
     *  Mirrors the real Generate button: does nothing when the button is
     *  disabled (i.e. while a generation is already in progress).
     */
    void triggerGenerate();

    /** Programmatically invoke the cancel action.
     *
     *  Mirrors the real Cancel button: does nothing when the button is
     *  disabled (i.e. when no generation is in progress).
     */
    void triggerCancel();

    void resized() override;

private:
    void onGenerateClicked();

    juce::Label promptLabel       { {}, "Prompt" };
    juce::TextEditor promptEditor;

    juce::Label lyricsLabel       { {}, "Lyrics" };
    juce::TextEditor lyricsEditor;

    juce::Label durationLabel     { {}, "Duration (s)" };
    juce::Slider durationSlider;

    juce::Label seedLabel         { {}, "Seed (-1 = random)" };
    juce::TextEditor seedEditor;

    juce::Label cfgScaleLabel     { {}, "CFG Scale" };
    juce::Slider cfgScaleSlider;

    juce::Label lmSeedLabel       { {}, "LM Seed (-1 = random)" };
    juce::TextEditor lmSeedEditor;

    juce::Label schedulerLabel    { {}, "Scheduler" };
    juce::ComboBox schedulerCombo;

    juce::ToggleButton capturedReferenceToggle { "Use captured reference" };

    juce::TextButton generateButton { "Generate" };
    juce::TextButton cancelButton   { "Cancel" };

    std::function<void(const GenerationFormState&)> onGenerate;
    std::function<void()> onCancel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerationFormPanel)
};

} // namespace acestep_plugin
