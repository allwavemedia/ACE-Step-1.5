#include "../Source/Engine/ReferenceAudioResampler.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ReferenceAudioResamplerTests final : public juce::UnitTest
{
public:
    ReferenceAudioResamplerTests() : juce::UnitTest("ReferenceAudioResampler") {}

    void runTest() override
    {
        beginTest("empty input returns empty output");
        {
            juce::AudioBuffer<float> empty(1, 0);
            const auto out = ReferenceAudioResampler::resample(empty, 44100, 24000);
            expectEquals(out.getNumSamples(), 0);
        }

        beginTest("downsampling reduces sample count");
        {
            juce::AudioBuffer<float> input(1, 4410);
            input.clear();
            const auto out = ReferenceAudioResampler::resample(input, 44100, 22050);
            // Expect approximately half the samples (allow 1% tolerance).
            expect(out.getNumSamples() > 2000 && out.getNumSamples() < 2300,
                   "downsampled sample count in expected range");
        }

        beginTest("upsampling increases sample count");
        {
            juce::AudioBuffer<float> input(1, 2205);
            input.clear();
            const auto out = ReferenceAudioResampler::resample(input, 22050, 44100);
            expect(out.getNumSamples() > 4000, "upsampled sample count in expected range");
        }

        beginTest("same rate returns same sample count");
        {
            juce::AudioBuffer<float> input(1, 1000);
            input.clear();
            const auto out = ReferenceAudioResampler::resample(input, 44100, 44100);
            expectEquals(out.getNumSamples(), 1000);
        }
    }
};

static ReferenceAudioResamplerTests sReferenceAudioResamplerTests;

} // namespace acestep_plugin
