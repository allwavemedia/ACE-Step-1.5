#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace acestep_plugin
{

struct ReferenceAudioStorage;

class ReferenceAudioBuffer final
{
public:
    ReferenceAudioBuffer();
    ~ReferenceAudioBuffer();

    void prepare(double sampleRate, int numChannels, double capacitySeconds);
    void setCapturing(bool shouldCapture) noexcept;
    bool isCapturing() const noexcept;

    void requestClear() noexcept;
    void handlePendingAudioThreadCommands() noexcept;

    void push(const juce::AudioBuffer<float>& input) noexcept;
    std::shared_ptr<juce::AudioBuffer<float>> snapshot();

    float consumePeak() noexcept;
    int getNumReady() const noexcept;
    int getCapacitySamples() const noexcept;

private:
    class StorageReadScope final
    {
    public:
        explicit StorageReadScope(const ReferenceAudioBuffer& owner) noexcept;
        ~StorageReadScope();

    private:
        const ReferenceAudioBuffer& owner;
    };

    std::shared_ptr<juce::AudioBuffer<float>> copySnapshotFromStorage(
        ReferenceAudioStorage& storage);

    static int copyReadableSamples(
        ReferenceAudioStorage& storage,
        juce::AudioBuffer<float>& output,
        std::uint64_t readStart,
        int ready,
        std::uint64_t capacity);

    static bool copySlotIfStable(
        ReferenceAudioStorage& storage,
        juce::AudioBuffer<float>& output,
        std::uint64_t readStart,
        int offset,
        int outputSample,
        std::uint64_t capacity);

    void writeBlock(
        ReferenceAudioStorage& storage,
        const juce::AudioBuffer<float>& input,
        int inputStart,
        int numSamples) noexcept;

    ReferenceAudioStorage* getStorage() const noexcept;
    void pruneRetiredStorageIfUnused();

    std::unique_ptr<ReferenceAudioStorage> activeStorageOwner;
    std::vector<std::unique_ptr<ReferenceAudioStorage>> retiredStorage;
    std::mutex storageMutex;
    std::atomic<ReferenceAudioStorage*> activeStorage { nullptr };
    mutable std::atomic<int> activeStorageReaders { 0 };
    std::atomic<bool> capturing { false };
    std::atomic<bool> clearRequested { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReferenceAudioBuffer)
};

} // namespace acestep_plugin
