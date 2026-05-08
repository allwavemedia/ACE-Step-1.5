#include "SidecarPaths.h"

namespace acestep_plugin
{

juce::File SidecarPaths::getJobRoot()
{
    auto root = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("AceStepPlugin")
                    .getChildFile("jobs");
    root.createDirectory();
    return root;
}

juce::File SidecarPaths::createJobDirectory(const juce::String& prefix)
{
    auto root = getJobRoot();
    const auto name = prefix
                      + "-" + juce::String(juce::Time::getCurrentTime().toMilliseconds())
                      + "-" + juce::String(juce::Random::getSystemRandom().nextInt64());
    auto dir = root.getChildFile(name);
    dir.createDirectory();
    return dir;
}

} // namespace acestep_plugin
