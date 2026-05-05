/** Real-engine smoke test for ACE-Step Plugin.
 *
 *  Launches the real `ace-synth` binary with a minimal 5-second generation
 *  request and verifies the output WAV file is produced and non-trivially
 *  large (> 1000 bytes).
 *
 *  Skip policy:
 *    If any required model file is absent the test exits 0 with a clear
 *    "[SKIP]" message so CI machines without GPU/models stay green.
 *
 *  Build requirement:
 *    ACESTEP_BUILD_REAL_SMOKE_TEST=ON (requires ACESTEP_ENABLE_ACESTEP_CPP=ON)
 *
 *  Run:
 *    AceStepRealSmokeTest
 */

#include "AceSynthPath.h"

#include "../Source/Models/ModelDiscovery.h"

#include <juce_core/juce_core.h>

#include <iostream>

using namespace acestep_plugin;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/** Print a single-line status prefix followed by the message. */
static void print(const char* tag, const juce::String& msg)
{
    (juce::String(tag) == "FAIL" ? std::cerr : std::cout)
        << "[" << tag << "] " << msg.toStdString() << "\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    // -----------------------------------------------------------------------
    // 1. Check model availability; skip cleanly when models are absent.
    // -----------------------------------------------------------------------
    const juce::File modelsDir = ModelDiscovery::getModelsDirectory();

    if (!ModelDiscovery::areAllModelsPresent())
    {
        const auto missing = ModelDiscovery::getMissingModelFilenames();

        print("SKIP", "Model files not found in:");
        std::cout << "       " << modelsDir.getFullPathName().toStdString() << "\n";
        std::cout << "       Missing (" << (int) missing.size() << " file(s)):\n";

        for (const auto& f : missing)
            std::cout << "         - " << f.toStdString() << "\n";

        print("SKIP", "Install models via scripts/download-models.ps1 and re-run.");
        return 0;
    }

    print("INFO", "Models directory: " + modelsDir.getFullPathName());

    // -----------------------------------------------------------------------
    // 2. Write a minimal request JSON to the system temp directory.
    //    The LM phase is skipped (ace-synth takes a pre-built request JSON
    //    directly; no LM model is needed for a text2music request).
    // -----------------------------------------------------------------------
    const juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    const juce::File requestJson = tempDir.getChildFile("acestep_smoke_request.json");

    // Output file naming: ace-synth strips the .json suffix and appends the
    // synth index (0) and the format extension (.wav).
    // E.g. "acestep_smoke_request.json" -> "acestep_smoke_request0.wav".
    const juce::File expectedOutput =
        tempDir.getChildFile(requestJson.getFileNameWithoutExtension() + "0.wav");

    // Remove any stale artefacts from a previous interrupted run.
    expectedOutput.deleteFile();

    // Minimal text2music request: caption required, 5 s duration, 8 DiT steps.
    // inference_steps=8 is the turbo-model default and keeps the run short.
    const juce::String requestContent =
        "{\n"
        "  \"caption\": \"upbeat electronic pop\",\n"
        "  \"lyrics\": \"[verse]\\nA simple smoke test song\",\n"
        "  \"duration\": 5.0,\n"
        "  \"seed\": 42,\n"
        "  \"lm_seed\": 42,\n"
        "  \"output_format\": \"wav16\",\n"
        "  \"synth_batch_size\": 1,\n"
        "  \"inference_steps\": 8\n"
        "}\n";

    if (!requestJson.replaceWithText(requestContent))
    {
        print("FAIL", "Cannot write request JSON: " + requestJson.getFullPathName());
        return 1;
    }

    // -----------------------------------------------------------------------
    // 3. Build and print the command, then run ace-synth.
    // -----------------------------------------------------------------------
    // ACESTEP_ACE_SYNTH_PATH is a CMake-generated raw string literal
    // (R"path(...)path") so Windows backslashes are preserved verbatim.
    const juce::String aceSynthPath(ACESTEP_ACE_SYNTH_PATH);

    // Wrap each path argument in double-quotes to handle spaces on Windows.
    juce::String cmd;
    cmd << "\"" << aceSynthPath << "\""
        << " --models \"" << modelsDir.getFullPathName() << "\""
        << " --request \"" << requestJson.getFullPathName() << "\"";

    print("INFO", "Command: " + cmd);

    juce::ChildProcess proc;

    if (!proc.start(cmd, 0))
    {
        print("FAIL", "Failed to start ace-synth.");
        print("FAIL", "Executable path: " + aceSynthPath);
        requestJson.deleteFile();
        return 1;
    }

    // Allow up to 5 minutes for GPU inference.
    const bool finished = proc.waitForProcessToFinish(300'000);

    if (!finished)
    {
        print("FAIL", "ace-synth timed out after 300 s.");
        proc.kill();
        requestJson.deleteFile();
        return 1;
    }

    const int exitCode = proc.getExitCode();
    if (exitCode != 0)
    {
        print("FAIL", "ace-synth exited with code " + juce::String(exitCode) + ".");
        requestJson.deleteFile();
        return 1;
    }

    // -----------------------------------------------------------------------
    // 4. Verify the output WAV.
    // -----------------------------------------------------------------------
    if (!expectedOutput.existsAsFile())
    {
        print("FAIL", "Expected output WAV not found:");
        std::cerr << "       " << expectedOutput.getFullPathName().toStdString() << "\n";
        requestJson.deleteFile();
        return 1;
    }

    const juce::int64 wavSize = expectedOutput.getSize();

    if (wavSize <= 1000)
    {
        print("FAIL", "Output WAV too small: " + juce::String(wavSize) + " bytes.");
        print("FAIL", "Path: " + expectedOutput.getFullPathName());
        expectedOutput.deleteFile();
        requestJson.deleteFile();
        return 1;
    }

    print("PASS", "Output WAV: " + expectedOutput.getFullPathName());
    print("PASS", "Size: " + juce::String(wavSize) + " bytes.");

    // Clean up temp artefacts.
    expectedOutput.deleteFile();
    requestJson.deleteFile();

    return 0;
}
