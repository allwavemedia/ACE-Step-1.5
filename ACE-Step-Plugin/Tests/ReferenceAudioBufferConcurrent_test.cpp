#include "../Source/Audio/ReferenceAudioBuffer.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ReferenceAudioBufferConcurrentTests final : public juce::UnitTest
{
public:
    ReferenceAudioBufferConcurrentTests() : juce::UnitTest("ReferenceAudioBufferConcurrent") {}

    void runTest() override
    {
        beginTest("concurrent push and snapshot do not crash");
        {
            ReferenceAudioBuffer buf;
            buf.prepare(44100.0, 1, 2.0);
            buf.setCapturing(true);

            std::atomic<bool> stop { false };

            juce::Thread::launch([&] {
                juce::AudioBuffer<float> block(1, 256);
                while (!stop.load())
                    buf.push(block);
            });

            for (int i = 0; i < 20; ++i)
            {
                buf.snapshot();
                juce::Thread::sleep(1);
            }

            stop.store(true);
            juce::Thread::sleep(20);
            expect(true, "no crash during concurrent access");
        }

        beginTest("clear request is honoured");
        {
            ReferenceAudioBuffer buf;
            buf.prepare(44100.0, 1, 1.0);
            buf.setCapturing(true);

            juce::AudioBuffer<float> block(1, 512);
            buf.push(block);

            buf.requestClear();
            buf.handlePendingAudioThreadCommands();

            expectEquals(buf.getNumReady(), 0);
        }
    }
};

static ReferenceAudioBufferConcurrentTests sReferenceAudioBufferConcurrentTests;

} // namespace acestep_plugin
