#include "ReferenceAudioBufferTestUtils.h"

#include "../Source/Audio/ReferenceAudioBuffer.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ReferenceAudioBufferBasicTests final : public juce::UnitTest
{
public:
    ReferenceAudioBufferBasicTests() : juce::UnitTest("ReferenceAudioBufferBasic") {}

    void runTest() override
    {
        beginTest("prepare sets capacity");
        {
            ReferenceAudioBuffer buf;
            buf.prepare(44100.0, 2, 5.0);
            expect(buf.getCapacitySamples() > 0);
        }

        beginTest("push and snapshot roundtrip");
        {
            ReferenceAudioBuffer buf;
            buf.prepare(44100.0, 1, 1.0);
            buf.setCapturing(true);

            juce::AudioBuffer<float> block(1, 512);
            block.setSample(0, 0, 0.5f);
            buf.push(block);

            expect(buf.getNumReady() > 0);
        }

        beginTest("not capturing discards push");
        {
            ReferenceAudioBuffer buf;
            buf.prepare(44100.0, 1, 1.0);
            buf.setCapturing(false);

            juce::AudioBuffer<float> block(1, 512);
            buf.push(block);

            expectEquals(buf.getNumReady(), 0);
        }
    }
};

static ReferenceAudioBufferBasicTests sReferenceAudioBufferBasicTests;

} // namespace acestep_plugin
