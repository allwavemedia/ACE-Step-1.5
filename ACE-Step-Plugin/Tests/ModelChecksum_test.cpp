#include "../Source/Models/ModelChecksum.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ModelChecksumTests final : public juce::UnitTest
{
public:
    ModelChecksumTests() : juce::UnitTest("ModelChecksum") {}

    void runTest() override
    {
        beginTest("computeForFile returns empty string for nonexistent file");
        {
            const auto hash =
                ModelChecksum::computeForFile(juce::File("/nonexistent/missing.bin"));
            expect(hash.isEmpty());
        }

        beginTest("computeForFile returns non-empty hash for existing file");
        {
            juce::TemporaryFile tmp(".bin");
            {
                juce::FileOutputStream out(tmp.getFile());
                out.write("hello", 5);
            }
            const auto hash = ModelChecksum::computeForFile(tmp.getFile());
            expect(hash.isNotEmpty());
            expectEquals(hash.length(), 64); // SHA-256 = 64 hex chars
        }

        beginTest("verifyFile returns true for matching hash");
        {
            juce::TemporaryFile tmp(".bin");
            {
                juce::FileOutputStream out(tmp.getFile());
                out.write("hello", 5);
            }
            const auto hash = ModelChecksum::computeForFile(tmp.getFile());
            expect(ModelChecksum::verifyFile(tmp.getFile(), hash));
        }

        beginTest("verifyFile returns false for wrong hash");
        {
            juce::TemporaryFile tmp(".bin");
            {
                juce::FileOutputStream out(tmp.getFile());
                out.write("hello", 5);
            }
            const auto badHash = juce::String::repeatedString("0", 64);
            expect(!ModelChecksum::verifyFile(tmp.getFile(), badHash));
        }

        beginTest("verifyFile returns false for empty expected hash");
        {
            juce::TemporaryFile tmp(".bin");
            tmp.getFile().create();
            expect(!ModelChecksum::verifyFile(tmp.getFile(), {}));
        }
    }
};

static ModelChecksumTests sModelChecksumTests;

} // namespace acestep_plugin
