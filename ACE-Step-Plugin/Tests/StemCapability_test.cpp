#include "../Source/Engine/GenerationRequest.h"
#include "../Source/Stems/StemCapability.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class StemCapabilityTests final : public juce::UnitTest
{
public:
    StemCapabilityTests() : juce::UnitTest("StemCapability") {}

    void runTest() override
    {
        beginTest("stems unavailable by default");
        {
            expect(!StemCapability::isAvailable(StemCapabilityState::unavailable));
        }

        beginTest("known backend support makes stems available");
        {
            expect(StemCapability::isAvailable(StemCapabilityState::available));
        }

        beginTest("generation request does not request stems by default");
        {
            GenerationRequest request;
            expect(!request.stemsEnabled);
            expect(request.requestedStemGroups.empty());
        }

        beginTest("generation request carries selected stem groups");
        {
            GenerationRequest request;
            request.stemsEnabled = true;
            request.requestedStemGroups.push_back(StemGroup::vocals);
            request.requestedStemGroups.push_back(StemGroup::drums);

            expect(request.stemsEnabled);
            expectEquals(static_cast<int>(request.requestedStemGroups.size()), 2);
            expect(request.requestedStemGroups[0] == StemGroup::vocals);
            expect(request.requestedStemGroups[1] == StemGroup::drums);
        }
    }
};

static StemCapabilityTests sStemCapabilityTests;

} // namespace acestep_plugin
