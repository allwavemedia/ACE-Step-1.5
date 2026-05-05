#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <memory>

namespace acestep_plugin
{

/** Plays generated WAV previews through a UI-owned output path.
 *
 *  Preview playback is independent of the plugin processor and does not feed
 *  or modify the host `processBlock` pass-through path.
 */
class PreviewPlayer final
{
public:
    enum class OutputMode
    {
        useDefaultDevice,
        disabledForTests
    };

    explicit PreviewPlayer(OutputMode outputMode = OutputMode::useDefaultDevice);
    ~PreviewPlayer();

    /** Start previewing a WAV file; returns false when the file cannot be read. */
    bool play(const juce::File& wavFile);

    /** Stop preview playback and release the current reader. */
    void stop() noexcept;

    bool isPlaying() const noexcept;

private:
    bool ensureOutputReady();

    OutputMode outputMode;
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    juce::AudioSourcePlayer sourcePlayer;

    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    bool outputReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewPlayer)
};

} // namespace acestep_plugin
