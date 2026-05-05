/** Test runner entry point.
 *
 *  Runs all juce::UnitTest subclasses registered in the binary.
 *  Returns the number of failures (0 = pass).
 */

#include "ReferenceAudioBufferTestUtils.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <iostream>

int main()
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runAllTests();

    int failures = 0;

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* result = runner.getResult(i);
        failures += result->failures;

        if (result->failures > 0)
            std::cerr << result->unitTestName << ": " << result->failures << " failure(s)\n";
    }

    if (failures == 0)
        std::cout << "All tests passed.\n";

    return failures;
}
