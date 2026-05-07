#pragma once

#include "../Models/GeneratedAssetHistory.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>
#include <vector>

namespace acestep_plugin
{

/** A single tile in the generated-asset history panel.
 *
 *  Displays: waveform thumbnail (AudioThumbnail), filename, duration label,
 *  play/stop button, and "Save As" button.  External drag-and-drop is
 *  triggered by pressing and dragging anywhere on the tile.
 *
 *  All callbacks are invoked on the message thread.
 */
class GeneratedAssetTile final : public juce::Component
{
public:
    using SaveAsCallback = std::function<void(const GeneratedAsset&)>;
    using MidiSaveAsCallback = std::function<void(const GeneratedAsset&)>;
    using StemPreviewCallback = std::function<void(const GeneratedAsset&, const StemAsset&, bool play)>;
    using StemSaveAsCallback = std::function<void(const GeneratedAsset&, const StemAsset&)>;
    using PlayStopCallback = std::function<void(const GeneratedAsset&, bool play)>;

    explicit GeneratedAssetTile(const GeneratedAsset& asset);
    ~GeneratedAssetTile() override = default;

    const GeneratedAsset& getAsset() const noexcept { return asset; }

    /** Update the playback state flag and repaint the play/stop button. */
    void setPlaying(bool shouldPlay);
    bool isPlaying() const noexcept { return playing; }
    bool canExportMidi() const noexcept;
    int getExportableStemCount() const;
    juce::File getMidiExportFile() const;
    juce::File getStemExportFileAt(int exportableStemIndex) const;
    bool toggleStemPreviewAt(int exportableStemIndex);
    bool exportStemAt(int exportableStemIndex);

    void setOnSaveAs(SaveAsCallback cb) { onSaveAs = std::move(cb); }
    void setOnMidiSaveAs(MidiSaveAsCallback cb) { onMidiSaveAs = std::move(cb); }
    void setOnStemPreview(StemPreviewCallback cb) { onStemPreview = std::move(cb); }
    void setOnStemSaveAs(StemSaveAsCallback cb) { onStemSaveAs = std::move(cb); }
    void setOnPlayStop(PlayStopCallback cb) { onPlayStop = std::move(cb); }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    const StemAsset* getExportableStem(int exportableStemIndex) const;
    juce::File getExternalDragFile(const juce::Point<int>& mouseDownPosition) const;

    GeneratedAsset asset;
    bool playing = false;

    juce::TextButton playStopButton { "Play" };
    juce::TextButton saveAsButton { "Save As" };
    juce::TextButton midiExportButton { "MIDI N/A" };
    std::vector<std::unique_ptr<juce::TextButton>> stemPreviewButtons;
    std::vector<std::unique_ptr<juce::TextButton>> stemExportButtons;
    std::vector<bool> stemPreviewStates;
    std::vector<size_t> exportableStemIndices;
    juce::Label filenameLabel;
    juce::Label durationLabel;

    SaveAsCallback onSaveAs;
    MidiSaveAsCallback onMidiSaveAs;
    StemPreviewCallback onStemPreview;
    StemSaveAsCallback onStemSaveAs;
    PlayStopCallback onPlayStop;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratedAssetTile)
};

} // namespace acestep_plugin
