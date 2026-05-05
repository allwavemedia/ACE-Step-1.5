#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace acestep_plugin
{

/** Starts copy-style external file drags for generated assets. */
class ExternalFileDrag final
{
public:
    using Performer = std::function<bool(const juce::StringArray&, bool canMoveFiles)>;

    /** Start a copy-style drag for an existing file using JUCE's native API. */
    static bool startCopyDrag(const juce::File& file);

    /** Start a copy-style drag with an injected performer for focused tests. */
    static bool startCopyDrag(const juce::File& file, const Performer& performer);
};

} // namespace acestep_plugin
