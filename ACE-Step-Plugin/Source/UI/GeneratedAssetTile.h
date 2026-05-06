#pragma once

#include "../Models/GeneratedAssetHistory.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

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
    using PlayStopCallback = std::function<void(const GeneratedAsset&, bool play)>;

    explicit GeneratedAssetTile(const GeneratedAsset& asset);
    ~GeneratedAssetTile() override = default;

    const GeneratedAsset& getAsset() const noexcept { return asset; }

    /** Update the playback state flag and repaint the play/stop button. */
    void setPlaying(bool shouldPlay);
    bool isPlaying() const noexcept { return playing; }
    bool canExportMidi() const noexcept;

    void setOnSaveAs(SaveAsCallback cb) { onSaveAs = std::move(cb); }
    void setOnMidiSaveAs(MidiSaveAsCallback cb) { onMidiSaveAs = std::move(cb); }
    void setOnPlayStop(PlayStopCallback cb) { onPlayStop = std::move(cb); }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    juce::File getExternalDragFile(const juce::Point<int>& mouseDownPosition) const;

    GeneratedAsset asset;
    bool playing = false;

    juce::TextButton playStopButton { "Play" };
    juce::TextButton saveAsButton { "Save As" };
    juce::TextButton midiExportButton { "MIDI N/A" };
    juce::Label filenameLabel;
    juce::Label durationLabel;

    SaveAsCallback onSaveAs;
    MidiSaveAsCallback onMidiSaveAs;
    PlayStopCallback onPlayStop;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratedAssetTile)
};

} // namespace acestep_plugin
