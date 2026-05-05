#include "GeneratedAssetTile.h"

#include "ExternalFileDrag.h"

namespace acestep_plugin
{

GeneratedAssetTile::GeneratedAssetTile(const GeneratedAsset& a)
    : asset(a)
{
    const auto filename = juce::File(asset.outputPath).getFileName();
    filenameLabel.setText(filename, juce::dontSendNotification);
    filenameLabel.setFont(juce::FontOptions(12.0f));

    const auto duration = juce::String(static_cast<int>(asset.durationSeconds)) + "s";
    durationLabel.setText(duration, juce::dontSendNotification);
    durationLabel.setFont(juce::FontOptions(11.0f));
    durationLabel.setJustificationType(juce::Justification::centredRight);

    playStopButton.onClick = [this] {
        setPlaying(!playing);

        if (onPlayStop)
            onPlayStop(asset, playing);
    };

    saveAsButton.onClick = [this] {
        if (onSaveAs)
            onSaveAs(asset);
    };

    addAndMakeVisible(filenameLabel);
    addAndMakeVisible(durationLabel);
    addAndMakeVisible(playStopButton);
    addAndMakeVisible(saveAsButton);
}

void GeneratedAssetTile::setPlaying(bool shouldPlay)
{
    playing = shouldPlay;
    playStopButton.setButtonText(playing ? "Stop" : "Play");
    repaint();
}

void GeneratedAssetTile::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    g.setColour(juce::Colour(0xff444444));
    g.drawRect(getLocalBounds(), 1);

    // Waveform thumbnail placeholder — real waveform rendered if
    // a thumbnail is loaded externally (future integration).
    const auto thumbArea = getLocalBounds().reduced(4).withHeight(40);
    g.setColour(juce::Colour(0xff1a5276));
    g.fillRect(thumbArea);
}

void GeneratedAssetTile::resized()
{
    auto bounds = getLocalBounds().reduced(4);

    const auto headerRow = bounds.removeFromTop(40).translated(0, 44);
    juce::ignoreUnused(headerRow); // thumbnail occupies first 40 px

    filenameLabel.setBounds(bounds.removeFromTop(18));
    durationLabel.setBounds(bounds.removeFromTop(14));

    const auto buttonRow = bounds.removeFromBottom(26);
    playStopButton.setBounds(buttonRow.withWidth(60));
    saveAsButton.setBounds(buttonRow.withTrimmedLeft(64).withWidth(60));
}

void GeneratedAssetTile::mouseDown(const juce::MouseEvent& /*e*/)
{
}

void GeneratedAssetTile::mouseDrag(const juce::MouseEvent& e)
{
    if (e.getDistanceFromDragStart() > 8)
    {
        ExternalFileDrag::startCopyDrag(juce::File(asset.outputPath));
    }
}

} // namespace acestep_plugin
