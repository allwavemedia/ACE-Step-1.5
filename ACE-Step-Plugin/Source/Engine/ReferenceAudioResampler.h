#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace acestep_plugin
{

/** Resamples a captured reference audio buffer to a target sample rate.
 *
 *  Uses juce::LagrangeInterpolator for each channel independently.  The
 *  output buffer has the same channel count as the input.
 *
 *  This should be called from the generation worker thread, never from
 *  processBlock or the message thread.
 */
class ReferenceAudioResampler final
{
public:
    ReferenceAudioResampler() = delete;

    /** Resample @p input from @p inputRate to @p targetRate.
     *
     *  @returns A new AudioBuffer<float> at the target rate.
     *           Returns an empty buffer if the input is empty.
     */
    static juce::AudioBuffer<float> resample(
        const juce::AudioBuffer<float>& input,
        int inputRate,
        int targetRate);
};

} // namespace acestep_plugin
