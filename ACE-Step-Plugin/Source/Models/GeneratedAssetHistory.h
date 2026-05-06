#pragma once

#include "../Stems/StemTypes.h"

#include <juce_core/juce_core.h>

#include <mutex>
#include <vector>

namespace acestep_plugin
{

/** Whether a generated asset can provide reliable MIDI note/event data. */
enum class MidiExportAvailability
{
    unavailable,
    available
};

/** Metadata for a single successfully generated WAV asset. */
struct GeneratedAsset
{
    /** Unique identifier (UUID string) assigned at creation time. */
    juce::String id;

    /** Absolute path to the generated WAV file. */
    juce::String outputPath;

    /** Duration of the audio in seconds. */
    float durationSeconds = 0.0f;

    /** Wall-clock time when this asset was generated. */
    juce::Time timestamp;

    /** MIDI export is unavailable until note/event data or analysis exists. */
    MidiExportAvailability midiAvailability = MidiExportAvailability::unavailable;

    /** Absolute path to the generated MIDI file when MIDI export is available. */
    juce::String midiPath;

    /** Stem files associated with this generation result. Empty when unavailable. */
    std::vector<StemAsset> stems;
};

/** Thread-safe ring buffer of the most recent generated assets.
 *
 *  Stores up to maxEntries assets, dropping the oldest when the cap is
 *  reached.  The asset list is ordered newest-first.
 */
class GeneratedAssetHistory final
{
public:
    /** Maximum number of assets retained in the history. */
    static constexpr int maxEntries = 8;

    GeneratedAssetHistory() = default;

    /** Add a new asset.  If size() == maxEntries the oldest is discarded. */
    void add(const GeneratedAsset& asset);

    /** Return a snapshot of all current assets (newest first). */
    std::vector<GeneratedAsset> getAssets() const;

    /** Return the number of assets currently tracked. */
    int size() const;

    /** Remove all tracked assets. */
    void clear();

private:
    mutable std::mutex mutex;
    std::vector<GeneratedAsset> assets; // newest at front

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratedAssetHistory)
};

} // namespace acestep_plugin
