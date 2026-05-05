#pragma once

#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>

namespace acestep_plugin
{

constexpr auto invalidReferenceSequence = std::numeric_limits<std::uint64_t>::max();

struct ReferenceAudioStorage final
{
    ReferenceAudioStorage(int channelCount, int capacity);

    static std::unique_ptr<std::atomic<float>[]> makeSamples(
        int channels,
        int capacitySamples);

    static std::unique_ptr<std::atomic<std::uint64_t>[]> makeSlotSequences(
        int capacitySamples);

    const int channels = 0;
    const int capacitySamples = 0;
    std::unique_ptr<std::atomic<float>[]> samples;
    std::unique_ptr<std::atomic<std::uint64_t>[]> slotSequences;
    std::atomic<float> peak { 0.0f };
    std::atomic<std::uint64_t> readPosition { 0 };
    std::atomic<std::uint64_t> writePosition { 0 };
};

} // namespace acestep_plugin
