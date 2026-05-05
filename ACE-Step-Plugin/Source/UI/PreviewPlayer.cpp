#include "PreviewPlayer.h"

namespace acestep_plugin
{

PreviewPlayer::PreviewPlayer(OutputMode mode)
    : outputMode(mode)
{
    formatManager.registerBasicFormats();
}

PreviewPlayer::~PreviewPlayer()
{
    stop();

    if (deviceManager != nullptr)
        deviceManager->removeAudioCallback(&sourcePlayer);
}

bool PreviewPlayer::play(const juce::File& wavFile)
{
    stop();

    if (!wavFile.existsAsFile())
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(wavFile));

    if (reader == nullptr)
        return false;

    if (!ensureOutputReady())
        return false;

    const auto sampleRate = reader->sampleRate;
    auto nextSource = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
    transportSource.setSource(nextSource.get(), 0, nullptr, sampleRate);
    readerSource = std::move(nextSource);
    transportSource.setPosition(0.0);
    transportSource.start();

    return true;
}

void PreviewPlayer::stop() noexcept
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();
}

bool PreviewPlayer::isPlaying() const noexcept
{
    return transportSource.isPlaying();
}

bool PreviewPlayer::ensureOutputReady()
{
    if (outputReady)
        return true;

    if (outputMode == OutputMode::disabledForTests)
    {
        outputReady = true;
        return true;
    }

    deviceManager = std::make_unique<juce::AudioDeviceManager>();
    const auto error = deviceManager->initialiseWithDefaultDevices(0, 2);

    if (error.isNotEmpty())
    {
        deviceManager.reset();
        return false;
    }

    sourcePlayer.setSource(&transportSource);
    deviceManager->addAudioCallback(&sourcePlayer);
    outputReady = true;
    return true;
}

} // namespace acestep_plugin
