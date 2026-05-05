#include "ExternalFileDrag.h"

namespace acestep_plugin
{

bool ExternalFileDrag::startCopyDrag(const juce::File& file)
{
    return startCopyDrag(file, [](const juce::StringArray& files, bool canMoveFiles) {
        return juce::DragAndDropContainer::performExternalDragDropOfFiles(files, canMoveFiles);
    });
}

bool ExternalFileDrag::startCopyDrag(const juce::File& file, const Performer& performer)
{
    if (!file.existsAsFile() || performer == nullptr)
        return false;

    juce::StringArray files;
    files.add(file.getFullPathName());

    return performer(files, false);
}

} // namespace acestep_plugin
