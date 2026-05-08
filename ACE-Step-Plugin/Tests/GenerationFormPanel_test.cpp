#include "../Source/Generation/GenerationFormPanel.h"
#include "../Source/Generation/GenerationFormState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace acestep_plugin
{

namespace
{

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

} // namespace

class GenerationFormPanelTests final : public juce::UnitTest
{
public:
    GenerationFormPanelTests() : juce::UnitTest("GenerationFormPanel") {}

    void runTest() override
    {
        beginTest("empty prompt fails validation");
        {
            GenerationFormState state;
            state.prompt = "";
            state.durationSeconds = 30.0f;
            expect(state.validate().isNotEmpty());
        }

        beginTest("whitespace-only prompt fails validation");
        {
            GenerationFormState state;
            state.prompt = "   ";
            state.durationSeconds = 30.0f;
            expect(state.validate().isNotEmpty());
        }

        beginTest("duration below 1 second fails validation");
        {
            GenerationFormState state;
            state.prompt = "ambient";
            state.durationSeconds = 0.5f;
            expect(state.validate().isNotEmpty());
        }

        beginTest("valid state passes validation");
        {
            GenerationFormState state;
            state.prompt = "ambient piano";
            state.durationSeconds = 30.0f;
            expectEquals(state.validate(), juce::String());
        }

        beginTest("valid state converts to GenerationRequest");
        {
            GenerationFormState state;
            state.prompt = "ambient piano";
            state.lyrics = "Hello world";
            state.durationSeconds = 45.0f;
            state.seed = 42;
            state.cfgScale = 8.5f;
            state.lmSeed = 7;
            state.scheduler = "dpmpp_2m";

            const auto req = state.toRequest("/output/song.wav");
            expectEquals(req.prompt,          juce::String("ambient piano"));
            expectEquals(req.lyrics,          juce::String("Hello world"));
            expectEquals(req.durationSeconds, 45.0f);
            expectEquals(req.seed,            42);
            expectEquals(req.cfgScale,        8.5f);
            expectEquals(req.lmSeed,          7);
            expectEquals(req.scheduler,       juce::String("dpmpp_2m"));
            expectEquals(req.outputPath,      juce::String("/output/song.wav"));
        }

        beginTest("useCapturedReference is preserved by panel state roundtrip");
        {
            GenerationFormPanel panel;
            GenerationFormState state;
            state.prompt = "ambient";
            state.durationSeconds = 30.0f;
            state.useCapturedReference = true;

            panel.setState(state);
            const auto roundtripped = panel.getState();
            expect(roundtripped.useCapturedReference);
        }

        beginTest("form controls have accessibility titles and descriptions");
        {
            GenerationFormPanel panel;

            for (const auto title : {
                     "Generation form",
                     "Prompt",
                     "Lyrics",
                     "Duration seconds",
                     "Seed",
                     "CFG scale",
                     "LM seed",
                     "Scheduler",
                     "Use captured reference",
                     "Generate",
                     "Cancel generation",
                 })
            {
                auto* component = findComponentWithTitle(panel, title);
                expect(component != nullptr, "expected accessible title: " + juce::String(title));

                if (component != nullptr)
                {
                    expect(component->getDescription().isNotEmpty(),
                           "accessible control must include a description: " + juce::String(title));
                    expect(component->getHelpText().isNotEmpty(),
                           "accessible control must include help text: " + juce::String(title));
                }
            }
        }

        beginTest("panel setState and getState roundtrip");
        {
            GenerationFormPanel panel;
            GenerationFormState state;
            state.prompt = "jazz guitar";
            state.lyrics = "verse one";
            state.durationSeconds = 60.0f;
            state.seed = 123;
            state.cfgScale = 9.0f;
            state.lmSeed = 456;
            state.scheduler = "euler";
            state.useCapturedReference = false;

            panel.setState(state);
            const auto result = panel.getState();
            expectEquals(result.prompt,           juce::String("jazz guitar"));
            expectEquals(result.lyrics,           juce::String("verse one"));
            expectEquals(result.durationSeconds,  60.0f);
            expectEquals(result.seed,             123);
            expectEquals(result.cfgScale,         9.0f);
            expectEquals(result.lmSeed,           456);
            expectEquals(result.scheduler,        juce::String("euler"));
            expect(!result.useCapturedReference);
        }

        beginTest("setGenerating disables generate button");
        {
            GenerationFormPanel panel;
            expect(panel.isGenerateEnabled());

            panel.setGenerating(true);
            expect(!panel.isGenerateEnabled());

            panel.setGenerating(false);
            expect(panel.isGenerateEnabled());
        }

        beginTest("setOnGenerate fires callback with valid state");
        {
            GenerationFormPanel panel;
            GenerationFormState state;
            state.prompt = "valid prompt";
            state.durationSeconds = 30.0f;
            panel.setState(state);

            bool callbackFired = false;
            panel.setOnGenerate([&](const GenerationFormState& s) {
                callbackFired = true;
                expectEquals(s.prompt, juce::String("valid prompt"));
            });

            panel.triggerGenerate();
            expect(callbackFired);
        }

        beginTest("setOnGenerate does not fire with invalid (empty prompt) state");
        {
            GenerationFormPanel panel;
            GenerationFormState state;
            state.prompt = "";
            state.durationSeconds = 30.0f;
            panel.setState(state);

            bool callbackFired = false;
            panel.setOnGenerate([&](const GenerationFormState&) { callbackFired = true; });

            panel.triggerGenerate();
            expect(!callbackFired);
        }

        beginTest("setOnCancel fires when cancel is triggered");
        {
            GenerationFormPanel panel;
            panel.setGenerating(true);

            bool cancelFired = false;
            panel.setOnCancel([&] { cancelFired = true; });

            panel.triggerCancel();
            expect(cancelFired);
        }

        beginTest("parseSeedText: empty and whitespace return -1");
        {
            expectEquals(GenerationFormPanel::parseSeedText(""),    -1);
            expectEquals(GenerationFormPanel::parseSeedText("   "), -1);
        }

        beginTest("parseSeedText: valid integers parse correctly");
        {
            expectEquals(GenerationFormPanel::parseSeedText("-1"),        -1);
            expectEquals(GenerationFormPanel::parseSeedText("0"),          0);
            expectEquals(GenerationFormPanel::parseSeedText("42"),        42);
            expectEquals(GenerationFormPanel::parseSeedText("2147483647"), 2147483647);
        }

        beginTest("parseSeedText: overflow clamps to INT_MAX");
        {
            expectEquals(GenerationFormPanel::parseSeedText("2147483648"),  2147483647);
            expectEquals(GenerationFormPanel::parseSeedText("99999999999"), 2147483647);
        }

        beginTest("parseSeedText: below -1 clamps to -1");
        {
            expectEquals(GenerationFormPanel::parseSeedText("-2"),   -1);
            expectEquals(GenerationFormPanel::parseSeedText("-100"), -1);
        }

        beginTest("parseSeedText: malformed text returns -1");
        {
            expectEquals(GenerationFormPanel::parseSeedText("1-23"), -1);
            expectEquals(GenerationFormPanel::parseSeedText("--1"),  -1);
            expectEquals(GenerationFormPanel::parseSeedText("abc"),  -1);
            expectEquals(GenerationFormPanel::parseSeedText("-"),    -1);
        }

        beginTest("triggerGenerate does not fire while generating");
        {
            GenerationFormPanel panel;
            GenerationFormState state;
            state.prompt = "valid";
            state.durationSeconds = 30.0f;
            panel.setState(state);
            panel.setGenerating(true);

            bool fired = false;
            panel.setOnGenerate([&](const GenerationFormState&) { fired = true; });
            panel.triggerGenerate();
            expect(!fired);
        }

        beginTest("triggerCancel does not fire while not generating");
        {
            GenerationFormPanel panel;
            bool fired = false;
            panel.setOnCancel([&] { fired = true; });
            panel.triggerCancel();
            expect(!fired);
        }

        beginTest("triggerCancel fires while generating");
        {
            GenerationFormPanel panel;
            panel.setGenerating(true);

            bool fired = false;
            panel.setOnCancel([&] { fired = true; });
            panel.triggerCancel();
            expect(fired);
        }

        beginTest("setState with unknown scheduler selects euler");
        {
            GenerationFormPanel panel;
            GenerationFormState state;
            state.prompt = "ambient";
            state.durationSeconds = 30.0f;
            state.scheduler = "nonexistent_scheduler";
            panel.setState(state);
            expectEquals(panel.getState().scheduler, juce::String("euler"));
        }
    }
};

static GenerationFormPanelTests sGenerationFormPanelTests;

} // namespace acestep_plugin
