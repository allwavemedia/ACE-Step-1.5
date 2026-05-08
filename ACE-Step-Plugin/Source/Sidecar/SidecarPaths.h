/** File-system path utilities for sidecar job management. */
#pragma once

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

/** Manages the plugin-owned file-system locations used for sidecar job I/O. */
class SidecarPaths final
{
public:
    /** Return (and lazily create) the plugin-owned job root directory.
     *
     *  Location: <userApplicationDataDirectory>/AceStepPlugin/jobs/
     *  The directory is created if it does not already exist.
     */
    static juce::File getJobRoot();

    /** Create and return a unique sub-directory of getJobRoot().
     *
     *  The directory name is: <prefix>-<milliseconds>-<random64>
     *  The directory is created before returning.
     *
     *  @param prefix  Short label embedded in the directory name (e.g. "generation").
     */
    static juce::File createJobDirectory(const juce::String& prefix);

private:
    SidecarPaths() = delete;
};

} // namespace acestep_plugin
