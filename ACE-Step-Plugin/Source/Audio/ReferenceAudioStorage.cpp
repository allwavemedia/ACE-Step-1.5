#include "ReferenceAudioBuffer.h"
#include "ReferenceAudioStorage.h"

namespace acestep_plugin
{

ReferenceAudioStorage::ReferenceAudioStorage(
    int channelCount,
    int capacity)
    : channels(channelCount),
      capacitySamples(capacity),
      samples(makeSamples(channelCount, capacity)),
      slotSequences(makeSlotSequences(capacity))
{
    const auto storageSize = static_cast<std::size_t>(channels)
        * static_cast<std::size_t>(capacitySamples);

    for (auto index = std::size_t { 0 }; index < storageSize; ++index)
        samples[index].store(0.0f, std::memory_order_relaxed);

    for (auto sample = 0; sample < capacitySamples; ++sample)
        slotSequences[static_cast<std::size_t>(sample)].store(
            invalidReferenceSequence,
            std::memory_order_relaxed);
}

std::unique_ptr<std::atomic<float>[]>
ReferenceAudioStorage::makeSamples(int channels, int capacitySamples)
{
    const auto storageSize = static_cast<std::size_t>(channels)
        * static_cast<std::size_t>(capacitySamples);

    return storageSize > 0
        ? std::make_unique<std::atomic<float>[]>(storageSize)
        : nullptr;
}

std::unique_ptr<std::atomic<std::uint64_t>[]>
ReferenceAudioStorage::makeSlotSequences(int capacitySamples)
{
    return capacitySamples > 0
        ? std::make_unique<std::atomic<std::uint64_t>[]>(
              static_cast<std::size_t>(capacitySamples))
        : nullptr;
}

} // namespace acestep_plugin
