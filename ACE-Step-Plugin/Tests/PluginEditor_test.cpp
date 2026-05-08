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

bool containsComponentTitle(juce::Component& component, const juce::String& title)
{
    if (component.getTitle().containsIgnoreCase(title))
        return true;

    for (int i = 0; i < component.getNumChildComponents(); ++i)
    {
        if (containsComponentTitle(*component.getChildComponent(i), title))
            return true;
    }

    return false;
}

juce::Component* findComponentWithTitle(juce::Component& component, const juce::String& title)
{
    if (component.getTitle().containsIgnoreCase(title))
        return &component;

    for (int i = 0; i < component.getNumChildComponents(); ++i)
    {
        if (auto* child = findComponentWithTitle(*component.getChildComponent(i), title))
            return child;
    }

    return nullptr;
}

juce::Label* findLabelContainingText(juce::Component& component, const juce::String& text)
{
    if (auto* label = dynamic_cast<juce::Label*>(&component))
        if (label->getText().containsIgnoreCase(text))
            return label;

    for (int i = 0; i < component.getNumChildComponents(); ++i)
        if (auto* label = findLabelContainingText(*component.getChildComponent(i), text))
            return label;

    return nullptr;
}

juce::Button* findButtonWithText(juce::Component& component, const juce::String& text)
{
    if (auto* button = dynamic_cast<juce::Button*>(&component))
        if (button->getButtonText().containsIgnoreCase(text))
            return button;

    for (int i = 0; i < component.getNumChildComponents(); ++i)
        if (auto* button = findButtonWithText(*component.getChildComponent(i), text))
            return button;

    return nullptr;
}

int countDirectViewports(juce::Component& component)
{
    int count = 0;

    for (int i = 0; i < component.getNumChildComponents(); ++i)
        if (dynamic_cast<juce::Viewport*>(component.getChildComponent(i)) != nullptr)
            ++count;

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
            expectEquals(countDirectViewports(editor), 1,
                         "editor must expose exactly one direct viewport");
            expectEquals(countDirectViewports(*viewport->getViewedComponent()), 0,
                         "scroll content must not add a nested app-level viewport");
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

        beginTest("single-scroll workflow has accessibility titles and descriptions");
        {
            AceStepAudioProcessor processor;
            AceStepAudioProcessorEditor editor(processor);
            editor.resized();

            auto* viewport = findDirectViewport(editor);
            auto* content = viewport == nullptr ? nullptr : viewport->getViewedComponent();
            expect(viewport != nullptr, "precondition: viewport exists");
            expect(content != nullptr, "precondition: scroll content exists");

            if (viewport == nullptr || content == nullptr)
                return;

            expect(viewport->getTitle().isNotEmpty(), "viewport must have an accessibility title");
            expect(viewport->getDescription().isNotEmpty(),
                   "viewport must describe the scroll workflow");
            expect(containsComponentTitle(*content, "Setup section"));
            expect(containsComponentTitle(*content, "Generation section"));
            expect(containsComponentTitle(*content, "Capture section"));
            expect(containsComponentTitle(*content, "Generated assets section"));
            expect(containsComponentTitle(*content, "Export actions"));
            expect(containsComponentTitle(*content, "Preset browser"));
            expect(containsComponentTitle(*content, "Diagnostics section"));

            for (const auto title : { "Generate", "Clear reference capture", "Capture meter" })
            {
                auto* component = findComponentWithTitle(*content, title);
                expect(component != nullptr,
                       "expected accessible component title: " + juce::String(title));
                if (component != nullptr)
                {
                    expect(component->getDescription().isNotEmpty(),
                           "accessible component must include a description: " + juce::String(title));
                    expect(component->getHelpText().isNotEmpty(),
                           "accessible component must include help text: " + juce::String(title));
                }
            }
        }

        beginTest("clear capture updates meter label immediately");
        {
            AceStepAudioProcessor processor;
            AceStepAudioProcessorEditor editor(processor);
            editor.resized();

            auto* viewport = findDirectViewport(editor);
            auto* content = viewport == nullptr ? nullptr : viewport->getViewedComponent();
            expect(content != nullptr, "precondition: scroll content exists");
            if (content == nullptr)
                return;

            auto* meterLabel = findLabelContainingText(*content, "Meter:");
            auto* clearButton = findButtonWithText(*content, "Clear");
            expect(meterLabel != nullptr, "precondition: meter label exists");
            expect(clearButton != nullptr, "precondition: clear button exists");
            if (meterLabel == nullptr || clearButton == nullptr)
                return;

            meterLabel->setText("Meter: stale", juce::dontSendNotification);
            expect(static_cast<bool>(clearButton->onClick),
                   "precondition: clear button must have a click handler");
            clearButton->onClick();

            expectEquals(meterLabel->getText(), juce::String("Meter: 0%"),
                         "clear must synchronously reset the displayed meter text");
        }

        beginTest("model setup button has user-visible action");
        {
            AceStepAudioProcessor processor;
            AceStepAudioProcessorEditor editor(processor);
            editor.resized();

            auto* viewport = findDirectViewport(editor);
            auto* content = viewport == nullptr ? nullptr : viewport->getViewedComponent();
            expect(content != nullptr, "precondition: scroll content exists");
            if (content == nullptr)
                return;

            auto* setupButton = findButtonWithText(*content, "Set Up Models");
            expect(setupButton != nullptr, "model setup button must exist");
            if (setupButton == nullptr)
                return;

            expect(static_cast<bool>(setupButton->onClick),
                   "model setup button must have a click handler");
            setupButton->onClick();

            expect(containsLabelText(*content, "Model setup action"),
                   "clicking Set Up Models must produce visible setup feedback");
        }
    }
};

static PluginEditorTests sPluginEditorTests;
