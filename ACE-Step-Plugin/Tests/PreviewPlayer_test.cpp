#include "../Source/UI/PreviewPlayer.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class PreviewPlayerTests final : public juce::UnitTest
{
public:
    PreviewPlayerTests() : juce::UnitTest("PreviewPlayer") {}

    void runTest() override
    {
        beginTest("isPlaying returns false after construction");
        {
            PreviewPlayer player(PreviewPlayer::OutputMode::disabledForTests);
            expect(!player.isPlaying());
        }

        beginTest("play returns false for invalid file path");
        {
            PreviewPlayer player(PreviewPlayer::OutputMode::disabledForTests);
            const bool started = player.play(juce::File("/nonexistent/missing.wav"));
            expect(!started);
            expect(!player.isPlaying());
        }

        beginTest("stop is a no-op when not playing");
        {
            PreviewPlayer player(PreviewPlayer::OutputMode::disabledForTests);
            player.stop(); // Must not crash.
            expect(!player.isPlaying());
        }
    }
};

static PreviewPlayerTests sPreviewPlayerTests;

} // namespace acestep_plugin
