#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace
{

juce::Viewport* findDirectViewport(juce::Component& component)
{
    for (int i = 0; i < component.getNumChildComponents(); ++i)
    {
        if (auto* viewport = dynamic_cast<juce::Viewport*>(component.getChildComponent(i)))
            return viewport;
    }

    return nullptr;
}

bool containsLabelText(juce::Component& component, const juce::String& text)
{
    if (auto* label = dynamic_cast<juce::Label*>(&component))
    {
        if (label->getText().containsIgnoreCase(text))
            return true;
    }

    if (auto* button = dynamic_cast<juce::Button*>(&component))
    {
        if (button->getButtonText().containsIgnoreCase(text))
            return true;
    }

    for (int i = 0; i < component.getNumChildComponents(); ++i)
    {
        if (containsLabelText(*component.getChildComponent(i), text))
            return true;
    }

    return false;
}

bool containsComponentName(juce::Component& component, const juce::String& name)
{
    if (component.getName().containsIgnoreCase(name))
        return true;

    for (int i = 0; i < component.getNumChildComponents(); ++i)
    {
        if (containsComponentName(*component.getChildComponent(i), name))
            return true;
    }

    return false;
}

int countViewports(juce::Component& component)
{
    int count = dynamic_cast<juce::Viewport*>(&component) == nullptr ? 0 : 1;

    for (int i = 0; i < component.getNumChildComponents(); ++i)
        count += countViewports(*component.getChildComponent(i));

    return count;
}

} // namespace

class PluginEditorTests final : public juce::UnitTest
{
public:
    PluginEditorTests() : juce::UnitTest("PluginEditor") {}

    void runTest() override
    {
        beginTest("editor uses one vertical viewport with no horizontal scrollbar");
        {
            AceStepAudioProcessor processor;
            AceStepAudioProcessorEditor editor(processor);
            editor.resized();

            auto* viewport = findDirectViewport(editor);
            expect(viewport != nullptr, "editor must expose one direct viewport");
            expect(viewport->isVerticalScrollBarShown(), "viewport must support vertical scrolling");
            expect(!viewport->isHorizontalScrollBarShown(), "viewport must not horizontally scroll");
            expect(viewport->getViewedComponent() != nullptr,
                   "viewport must own the single-scroll content component");
            expectEquals(countViewports(editor), 1, "editor must not add nested scroll views");
        }

        beginTest("single-scroll content contains generation, capture, presets, assets, diagnostics");
        {
            AceStepAudioProcessor processor;
            AceStepAudioProcessorEditor editor(processor);
            editor.resized();

            auto* viewport = findDirectViewport(editor);
            expect(viewport != nullptr, "precondition: viewport exists");
            auto* content = viewport == nullptr ? nullptr : viewport->getViewedComponent();
            expect(content != nullptr, "precondition: scroll content exists");

            if (content == nullptr)
                return;

            expect(containsLabelText(*content, "Prompt"), "generation prompt must be reachable");
            expect(containsLabelText(*content, "Generate"), "Generate control must be reachable");
            expect(containsLabelText(*content, "Cancel"), "Cancel control must be reachable");
            expect(containsLabelText(*content, "Arm"), "capture Arm control must be reachable");
            expect(containsLabelText(*content, "Clear"), "capture Clear control must be reachable");
            expect(containsLabelText(*content, "Preset"), "preset browser must be reachable");
            expect(containsLabelText(*content, "Generated Assets"),
                   "generated asset history must be reachable");
            expect(containsLabelText(*content, "WAV"), "WAV export placeholder must be reachable");
            expect(containsLabelText(*content, "MIDI"), "MIDI export placeholder must be reachable");
            expect(containsLabelText(*content, "stem"), "stem export placeholder must be reachable");
            expect(containsLabelText(*content, "Diagnostics"), "diagnostics must be reachable");
        }

        beginTest("single-scroll sections have accessibility names");
        {
            AceStepAudioProcessor processor;
            AceStepAudioProcessorEditor editor(processor);
            editor.resized();

            auto* viewport = findDirectViewport(editor);
            auto* content = viewport == nullptr ? nullptr : viewport->getViewedComponent();
            expect(content != nullptr, "precondition: scroll content exists");

            if (content == nullptr)
                return;

            expect(containsComponentName(*content, "Generation section"));
            expect(containsComponentName(*content, "Capture section"));
            expect(containsComponentName(*content, "Generated assets section"));
            expect(containsComponentName(*content, "Export actions"));
            expect(containsComponentName(*content, "Diagnostics section"));
        }
    }
};

static PluginEditorTests sPluginEditorTests;
