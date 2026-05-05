#include "PluginProcessor.h"

#include "PluginEditor.h"

AceStepAudioProcessor::AceStepAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void AceStepAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    referenceAudioBuffer.prepare(sampleRate, getTotalNumInputChannels(), 60.0);
}

void AceStepAudioProcessor::releaseResources()
{
}

bool AceStepAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainInput = layouts.getMainInputChannelSet();
    const auto& mainOutput = layouts.getMainOutputChannelSet();

    return mainInput == juce::AudioChannelSet::stereo()
        && mainOutput == juce::AudioChannelSet::stereo();
}

void AceStepAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    referenceAudioBuffer.push(buffer);

    for (auto channel = getTotalNumInputChannels();
         channel < getTotalNumOutputChannels();
         ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* AceStepAudioProcessor::createEditor()
{
    return new AceStepAudioProcessorEditor(*this);
}

bool AceStepAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String AceStepAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AceStepAudioProcessor::acceptsMidi() const
{
    return false;
}

bool AceStepAudioProcessor::producesMidi() const
{
    return false;
}

bool AceStepAudioProcessor::isMidiEffect() const
{
    return false;
}

double AceStepAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AceStepAudioProcessor::getNumPrograms()
{
    return 1;
}

int AceStepAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AceStepAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AceStepAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AceStepAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void AceStepAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    destData.reset();
}

void AceStepAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

void AceStepAudioProcessor::setReferenceCaptureEnabled(bool shouldCapture) noexcept
{
    referenceAudioBuffer.setCapturing(shouldCapture);
}

bool AceStepAudioProcessor::isReferenceCaptureEnabled() const noexcept
{
    return referenceAudioBuffer.isCapturing();
}

void AceStepAudioProcessor::requestReferenceClear() noexcept
{
    referenceAudioBuffer.requestClear();
}

float AceStepAudioProcessor::consumeReferencePeak() noexcept
{
    return referenceAudioBuffer.consumePeak();
}

std::shared_ptr<juce::AudioBuffer<float>> AceStepAudioProcessor::snapshotReference()
{
    return referenceAudioBuffer.snapshot();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AceStepAudioProcessor();
}
