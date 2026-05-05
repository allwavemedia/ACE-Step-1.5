#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Audio/ReferenceAudioBuffer.h"
#include "Engine/AceStepEngine.h"

class AceStepAudioProcessor final : public juce::AudioProcessor
{
public:
    AceStepAudioProcessor();
    ~AceStepAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void setReferenceCaptureEnabled(bool shouldCapture) noexcept;
    bool isReferenceCaptureEnabled() const noexcept;
    void requestReferenceClear() noexcept;
    float consumeReferencePeak() noexcept;
    std::shared_ptr<juce::AudioBuffer<float>> snapshotReference();

    /** Returns the engine instance owned by this processor. */
    acestep_plugin::AceStepEngine& getEngine() noexcept { return engine; }

private:
    acestep_plugin::ReferenceAudioBuffer referenceAudioBuffer;
    acestep_plugin::AceStepEngine engine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AceStepAudioProcessor)
};
