#include "ReferenceAudioBuffer.h"
#include "ReferenceAudioStorage.h"

#include <algorithm>
#include <cmath>

namespace acestep_plugin
{

ReferenceAudioBuffer::StorageReadScope::StorageReadScope(
    const ReferenceAudioBuffer& buffer) noexcept
    : owner(buffer)
{
    owner.activeStorageReaders.fetch_add(1, std::memory_order_acquire);
}

ReferenceAudioBuffer::StorageReadScope::~StorageReadScope()
{
    owner.activeStorageReaders.fetch_sub(1, std::memory_order_release);
}

ReferenceAudioBuffer::ReferenceAudioBuffer()
{
}

ReferenceAudioBuffer::~ReferenceAudioBuffer() = default;

void ReferenceAudioBuffer::prepare(double sampleRate, int numChannels, double capacitySeconds)
{
    const auto channels = std::max(0, numChannels);
    const auto capacitySamples = std::max(
        0,
        static_cast<int>(std::ceil(sampleRate * capacitySeconds)));
    auto storage = std::make_unique<ReferenceAudioStorage>(channels, capacitySamples);
    auto* storagePointer = storage.get();

    {
        std::lock_guard<std::mutex> lock(storageMutex);

        if (activeStorageOwner != nullptr)
            retiredStorage.push_back(std::move(activeStorageOwner));

        activeStorageOwner = std::move(storage);
        activeStorage.store(storagePointer, std::memory_order_release);
    }

    clearRequested.store(false, std::memory_order_release);
    pruneRetiredStorageIfUnused();
}

void ReferenceAudioBuffer::setCapturing(bool shouldCapture) noexcept
{
    capturing.store(shouldCapture, std::memory_order_release);
}

bool ReferenceAudioBuffer::isCapturing() const noexcept
{
    return capturing.load(std::memory_order_acquire);
}

void ReferenceAudioBuffer::requestClear() noexcept
{
    StorageReadScope guard(*this);

    if (auto* storage = getStorage())
    {
        storage->readPosition.store(
            storage->writePosition.load(std::memory_order_acquire),
            std::memory_order_release);
        storage->peak.store(0.0f, std::memory_order_release);
    }

    clearRequested.store(true, std::memory_order_release);
}

void ReferenceAudioBuffer::handlePendingAudioThreadCommands() noexcept
{
    if (!clearRequested.exchange(false, std::memory_order_acq_rel))
        return;

    StorageReadScope guard(*this);

    if (auto* storage = getStorage())
    {
        storage->readPosition.store(
            storage->writePosition.load(std::memory_order_acquire),
            std::memory_order_release);
        storage->peak.store(0.0f, std::memory_order_release);
    }
}

void ReferenceAudioBuffer::push(const juce::AudioBuffer<float>& input) noexcept
{
    handlePendingAudioThreadCommands();

    StorageReadScope guard(*this);
    auto* storage = getStorage();

    if (storage == nullptr || !isCapturing()
        || storage->capacitySamples <= 0 || storage->channels <= 0)
        return;

    const auto samplesToCopy = std::min(input.getNumSamples(), storage->capacitySamples);
    const auto inputStart = input.getNumSamples() - samplesToCopy;
    writeBlock(*storage, input, inputStart, samplesToCopy);

    auto blockPeak = 0.0f;
    const auto channelsToCopy = std::min(storage->channels, input.getNumChannels());

    for (auto channel = 0; channel < channelsToCopy; ++channel)
        blockPeak = std::max(blockPeak,
            input.getMagnitude(channel, inputStart, samplesToCopy));

    storage->peak.store(std::max(storage->peak.load(std::memory_order_relaxed), blockPeak),
        std::memory_order_release);
}

std::shared_ptr<juce::AudioBuffer<float>> ReferenceAudioBuffer::snapshot()
{
    std::shared_ptr<juce::AudioBuffer<float>> output;

    {
        StorageReadScope guard(*this);

        if (auto* storage = getStorage())
            output = copySnapshotFromStorage(*storage);
        else
            output = std::make_shared<juce::AudioBuffer<float>>();
    }

    pruneRetiredStorageIfUnused();
    return output;
}

float ReferenceAudioBuffer::consumePeak() noexcept
{
    StorageReadScope guard(*this);

    if (auto* storage = getStorage())
        return storage->peak.exchange(0.0f, std::memory_order_acq_rel);

    return 0.0f;
}

int ReferenceAudioBuffer::getNumReady() const noexcept
{
    StorageReadScope guard(*this);

    auto* storage = getStorage();
    if (storage == nullptr)
        return 0;

    const auto writeEnd = storage->writePosition.load(std::memory_order_acquire);
    const auto capacity = static_cast<std::uint64_t>(std::max(0, storage->capacitySamples));
    const auto oldestReadable = writeEnd > capacity ? writeEnd - capacity : 0;
    const auto requestedReadStart = std::max(
        storage->readPosition.load(std::memory_order_acquire),
        oldestReadable);
    const auto readStart = std::min(requestedReadStart, writeEnd);

    return static_cast<int>(std::min(writeEnd - readStart, capacity));
}

int ReferenceAudioBuffer::getCapacitySamples() const noexcept
{
    StorageReadScope guard(*this);

    if (auto* storage = getStorage())
        return storage->capacitySamples;

    return 0;
}

ReferenceAudioStorage* ReferenceAudioBuffer::getStorage() const noexcept
{
    return activeStorage.load(std::memory_order_acquire);
}

void ReferenceAudioBuffer::pruneRetiredStorageIfUnused()
{
    if (activeStorageReaders.load(std::memory_order_acquire) != 0)
        return;

    std::lock_guard<std::mutex> lock(storageMutex);

    if (activeStorageReaders.load(std::memory_order_acquire) == 0)
        retiredStorage.clear();
}

} // namespace acestep_plugin
