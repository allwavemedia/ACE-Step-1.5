#include "ReferenceAudioResampler.h"

namespace acestep_plugin
{

juce::AudioBuffer<float> ReferenceAudioResampler::resample(
    const juce::AudioBuffer<float>& input,
    int inputRate,
    int targetRate)
{
    const int channels = input.getNumChannels();
    const int inputSamples = input.getNumSamples();

    if (inputSamples == 0 || channels == 0)
        return juce::AudioBuffer<float>(channels, 0);

    if (inputRate == targetRate)
    {
        juce::AudioBuffer<float> copy(channels, inputSamples);
        for (int ch = 0; ch < channels; ++ch)
            copy.copyFrom(ch, 0, input, ch, 0, inputSamples);
        return copy;
    }

    const double ratio = static_cast<double>(targetRate) / static_cast<double>(inputRate);
    const int outputSamples = static_cast<int>(std::ceil(static_cast<double>(inputSamples) * ratio));

    juce::AudioBuffer<float> output(channels, outputSamples);

    for (int ch = 0; ch < channels; ++ch)
    {
        juce::LagrangeInterpolator interpolator;
        interpolator.reset();

        const float* src = input.getReadPointer(ch);
        float* dst = output.getWritePointer(ch);

        interpolator.process(1.0 / ratio, src, dst, outputSamples, inputSamples, 0);
    }

    return output;
}

} // namespace acestep_plugin
