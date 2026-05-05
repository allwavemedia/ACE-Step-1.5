#include "ReferenceAudioBuffer.h"
#include "ReferenceAudioStorage.h"

#include <algorithm>

namespace acestep_plugin
{

void ReferenceAudioBuffer::writeBlock(
    ReferenceAudioStorage& storage,
    const juce::AudioBuffer<float>& input,
    int inputStart,
    int numSamples) noexcept
{
    const auto startPosition = storage.writePosition.load(std::memory_order_relaxed);
    const auto channelsToCopy = std::min(storage.channels, input.getNumChannels());

    for (auto offset = 0; offset < numSamples; ++offset)
    {
        const auto position = startPosition + static_cast<std::uint64_t>(offset);
        const auto slot = static_cast<int>(
            position % static_cast<std::uint64_t>(storage.capacitySamples));

        storage.slotSequences[static_cast<std::size_t>(slot)].store(
            invalidReferenceSequence,
            std::memory_order_release);

        for (auto channel = 0; channel < channelsToCopy; ++channel)
        {
            storage
                .samples[static_cast<std::size_t>(
                    channel * storage.capacitySamples + slot)]
                .store(
                    input.getSample(channel, inputStart + offset),
                    std::memory_order_release);
        }

        for (auto channel = channelsToCopy; channel < storage.channels; ++channel)
        {
            storage
                .samples[static_cast<std::size_t>(
                    channel * storage.capacitySamples + slot)]
                .store(0.0f, std::memory_order_release);
        }

        storage.slotSequences[static_cast<std::size_t>(slot)].store(
            position,
            std::memory_order_release);
    }

    storage.writePosition.store(
        startPosition + static_cast<std::uint64_t>(numSamples),
        std::memory_order_release);
}

} // namespace acestep_plugin
