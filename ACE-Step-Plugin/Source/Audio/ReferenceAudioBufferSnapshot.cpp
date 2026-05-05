#include "ReferenceAudioBuffer.h"
#include "ReferenceAudioStorage.h"

#include <algorithm>

namespace acestep_plugin
{

std::shared_ptr<juce::AudioBuffer<float>>
ReferenceAudioBuffer::copySnapshotFromStorage(ReferenceAudioStorage& storage)
{
    const auto writeEnd = storage.writePosition.load(std::memory_order_acquire);
    const auto capacity = static_cast<std::uint64_t>(std::max(0, storage.capacitySamples));
    const auto oldestReadable = writeEnd > capacity ? writeEnd - capacity : 0;
    const auto requestedReadStart = std::max(
        storage.readPosition.load(std::memory_order_acquire),
        oldestReadable);
    const auto readStart = std::min(requestedReadStart, writeEnd);
    const auto ready = static_cast<int>(std::min(writeEnd - readStart, capacity));
    auto output = std::make_shared<juce::AudioBuffer<float>>(storage.channels, ready);

    const auto copied = copyReadableSamples(storage, *output, readStart, ready, capacity);

    if (copied != ready)
        output->setSize(storage.channels, copied, true, true, true);

    storage.readPosition.store(writeEnd, std::memory_order_release);
    return output;
}

int ReferenceAudioBuffer::copyReadableSamples(
    ReferenceAudioStorage& storage,
    juce::AudioBuffer<float>& output,
    std::uint64_t readStart,
    int ready,
    std::uint64_t capacity)
{
    auto copied = 0;

    for (auto offset = 0; offset < ready; ++offset)
    {
        if (copySlotIfStable(storage, output, readStart, offset, copied, capacity))
            ++copied;
        else
            copied = 0;
    }

    return copied;
}

bool ReferenceAudioBuffer::copySlotIfStable(
    ReferenceAudioStorage& storage,
    juce::AudioBuffer<float>& output,
    std::uint64_t readStart,
    int offset,
    int outputSample,
    std::uint64_t capacity)
{
    const auto position = readStart + static_cast<std::uint64_t>(offset);
    const auto slot = static_cast<int>(position % capacity);
    const auto slotIndex = static_cast<std::size_t>(slot);

    if (storage.slotSequences[slotIndex].load(std::memory_order_acquire) != position)
        return false;

    for (auto channel = 0; channel < storage.channels; ++channel)
    {
        const auto sampleIndex = static_cast<std::size_t>(
            channel * storage.capacitySamples + slot);
        output.setSample(channel, outputSample,
            storage.samples[sampleIndex].load(std::memory_order_acquire));
    }

    return storage.slotSequences[slotIndex].load(std::memory_order_acquire) == position;
}

} // namespace acestep_plugin
