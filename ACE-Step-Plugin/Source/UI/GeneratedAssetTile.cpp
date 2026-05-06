#include "GeneratedAssetTile.h"

#include "ExternalFileDrag.h"
#include "../Stems/StemCapability.h"

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

    const auto midiAvailable = canExportMidi();
    midiExportButton.setButtonText(midiAvailable ? "MIDI" : "MIDI N/A");
    midiExportButton.setEnabled(midiAvailable);
    midiExportButton.setTooltip(
        midiAvailable ? "Export MIDI" : "MIDI export unavailable: no reliable note/event data");
    midiExportButton.onClick = [this] {
        if (canExportMidi() && onMidiSaveAs)
            onMidiSaveAs(asset);
    };
    midiExportButton.addMouseListener(this, true);

    for (int index = 0; index < getExportableStemCount(); ++index)
    {
        const auto* stem = getExportableStem(index);
        const auto stemName = StemCapability::getDisplayName(stem->group);
        auto previewButton = std::make_unique<juce::TextButton>("Play " + stemName);
        auto exportButton = std::make_unique<juce::TextButton>("Save " + stemName);

        previewButton->onClick = [this, index] { toggleStemPreviewAt(index); };
        exportButton->onClick = [this, index] { exportStemAt(index); };
        previewButton->addMouseListener(this, true);
        exportButton->addMouseListener(this, true);

        addAndMakeVisible(*previewButton);
        addAndMakeVisible(*exportButton);
        stemPreviewButtons.push_back(std::move(previewButton));
        stemExportButtons.push_back(std::move(exportButton));
        stemPreviewStates.push_back(false);
    }

    addAndMakeVisible(filenameLabel);
    addAndMakeVisible(durationLabel);
    addAndMakeVisible(playStopButton);
    addAndMakeVisible(saveAsButton);
    addAndMakeVisible(midiExportButton);
}

void GeneratedAssetTile::setPlaying(bool shouldPlay)
{
    playing = shouldPlay;
    playStopButton.setButtonText(playing ? "Stop" : "Play");
    repaint();
}

bool GeneratedAssetTile::canExportMidi() const noexcept
{
    return asset.midiAvailability == MidiExportAvailability::available;
}

int GeneratedAssetTile::getExportableStemCount() const
{
    int count = 0;

    for (const auto& stem : asset.stems)
        if (stem.success && stem.outputPath.isNotEmpty())
            ++count;

    return count;
}

juce::File GeneratedAssetTile::getStemExportFileAt(int exportableStemIndex) const
{
    if (const auto* stem = getExportableStem(exportableStemIndex))
        return juce::File(stem->outputPath);

    return {};
}

bool GeneratedAssetTile::toggleStemPreviewAt(int exportableStemIndex)
{
    const auto* stem = getExportableStem(exportableStemIndex);
    if (stem == nullptr)
        return false;

    const auto index = static_cast<size_t>(exportableStemIndex);
    if (index >= stemPreviewStates.size())
        return false;

    stemPreviewStates[index] = !stemPreviewStates[index];
    stemPreviewButtons[index]->setButtonText(
        (stemPreviewStates[index] ? "Stop " : "Play ")
        + StemCapability::getDisplayName(stem->group));

    if (onStemPreview)
        onStemPreview(asset, *stem, stemPreviewStates[index]);

    return true;
}

bool GeneratedAssetTile::exportStemAt(int exportableStemIndex)
{
    const auto* stem = getExportableStem(exportableStemIndex);
    if (stem == nullptr || onStemSaveAs == nullptr)
        return false;

    onStemSaveAs(asset, *stem);
    return true;
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

    if (!stemExportButtons.empty())
    {
        auto stemRow = bounds.removeFromBottom(24);
        const auto stemButtonCount = static_cast<int>(stemExportButtons.size() * 2);
        const int stemWidth = juce::jmax(1, stemRow.getWidth() / stemButtonCount);

        for (size_t index = 0; index < stemExportButtons.size(); ++index)
        {
            stemPreviewButtons[index]->setBounds(stemRow.removeFromLeft(stemWidth).reduced(1, 0));
            stemExportButtons[index]->setBounds(stemRow.removeFromLeft(stemWidth).reduced(1, 0));
        }
    }

    const auto buttonRow = bounds.removeFromBottom(26);
    playStopButton.setBounds(buttonRow.withWidth(60));
    saveAsButton.setBounds(buttonRow.withTrimmedLeft(64).withWidth(60));
    midiExportButton.setBounds(buttonRow.withTrimmedLeft(128).withWidth(76));
}

void GeneratedAssetTile::mouseDown(const juce::MouseEvent& /*e*/)
{
}

void GeneratedAssetTile::mouseDrag(const juce::MouseEvent& e)
{
    const auto eventInTile = e.getEventRelativeTo(this);

    if (eventInTile.getDistanceFromDragStart() > 8)
    {
        ExternalFileDrag::startCopyDrag(
            getExternalDragFile(eventInTile.getMouseDownPosition()));
    }
}

const StemAsset* GeneratedAssetTile::getExportableStem(int exportableStemIndex) const
{
    if (exportableStemIndex < 0)
        return nullptr;

    int currentIndex = 0;
    for (const auto& stem : asset.stems)
    {
        if (!stem.success || stem.outputPath.isEmpty())
            continue;

        if (currentIndex == exportableStemIndex)
            return &stem;

        ++currentIndex;
    }

    return nullptr;
}

juce::File GeneratedAssetTile::getExternalDragFile(
    const juce::Point<int>& mouseDownPosition) const
{
    for (int index = 0; index < static_cast<int>(stemExportButtons.size()); ++index)
    {
        const auto vectorIndex = static_cast<size_t>(index);
        if (stemPreviewButtons[vectorIndex]->getBounds().contains(mouseDownPosition)
            || stemExportButtons[vectorIndex]->getBounds().contains(mouseDownPosition))
            return getStemExportFileAt(index);
    }

    if (midiExportButton.getBounds().contains(mouseDownPosition)
        && canExportMidi()
        && asset.midiPath.isNotEmpty())
    {
        return juce::File(asset.midiPath);
    }

    return juce::File(asset.outputPath);
}

} // namespace acestep_plugin
