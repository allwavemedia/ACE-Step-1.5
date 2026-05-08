/** Behaviour tests for validateResultManifest.
 *
 *  Verifies that a trivial helper-written artifact and manifest pass
 *  validation, and that a mismatched request ID or a missing artifact
 *  produce manifestInvalid.
 */
#include "../Source/Sidecar/SidecarManifest.h"

#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>

namespace acestep_plugin
{

class SidecarManifestTests final : public juce::UnitTest
{
public:
    SidecarManifestTests() : juce::UnitTest("SidecarManifest") {}

    void runTest() override
    {
        beginTest("valid manifest with correct artifact passes validation");
        {
            const auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getChildFile("sidecar_manifest_test_ok");
            dir.createDirectory();

            // Write a trivial artifact
            const auto artFile = dir.getChildFile("output.wav");
            artFile.replaceWithText("FAKE_AUDIO_CONTENT");

            // Compute real SHA-256 and byte size
            const auto sha256Hex = juce::SHA256(artFile).toHexString();
            const auto byteSize = artFile.getSize();

            // Build artifact JSON object
            juce::DynamicObject::Ptr artObj = new juce::DynamicObject();
            artObj->setProperty("path", artFile.getFullPathName());
            artObj->setProperty("byteSize", static_cast<juce::int64>(byteSize));
            artObj->setProperty("sha256", sha256Hex);

            juce::Array<juce::var> artArray;
            artArray.add(juce::var(artObj.get()));

            // Build manifest JSON object
            juce::DynamicObject::Ptr manifest = new juce::DynamicObject();
            manifest->setProperty("protocolVersion", juce::String("1.0"));
            manifest->setProperty("requestId", juce::String("req-test-001"));
            manifest->setProperty("success", true);
            manifest->setProperty("artifacts", juce::var(artArray));

            const auto manifestFile = dir.getChildFile("manifest.json");
            manifestFile.replaceWithText(juce::JSON::toString(juce::var(manifest.get())));

            const auto err = validateResultManifest(manifestFile, "req-test-001");
            expect(err == SidecarProcessError::none,
                   "valid manifest with correct artifact must pass");

            artFile.deleteFile();
            manifestFile.deleteFile();
            dir.deleteRecursively();
        }

        beginTest("manifest with mismatched request ID fails validation");
        {
            const auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getChildFile("sidecar_manifest_test_badid");
            dir.createDirectory();

            const auto artFile = dir.getChildFile("output.wav");
            artFile.replaceWithText("FAKE_AUDIO_CONTENT");

            juce::DynamicObject::Ptr artObj = new juce::DynamicObject();
            artObj->setProperty("path", artFile.getFullPathName());
            artObj->setProperty("byteSize", static_cast<juce::int64>(artFile.getSize()));
            artObj->setProperty("sha256", juce::String(""));

            juce::Array<juce::var> artArray;
            artArray.add(juce::var(artObj.get()));

            juce::DynamicObject::Ptr manifest = new juce::DynamicObject();
            manifest->setProperty("protocolVersion", juce::String("1.0"));
            manifest->setProperty("requestId", juce::String("req-test-001"));
            manifest->setProperty("success", true);
            manifest->setProperty("artifacts", juce::var(artArray));

            const auto manifestFile = dir.getChildFile("manifest.json");
            manifestFile.replaceWithText(juce::JSON::toString(juce::var(manifest.get())));

            const auto err = validateResultManifest(manifestFile, "req-WRONG-ID");
            expect(err == SidecarProcessError::manifestInvalid,
                   "mismatched request ID must yield manifestInvalid");

            artFile.deleteFile();
            manifestFile.deleteFile();
            dir.deleteRecursively();
        }

        beginTest("manifest referencing a missing artifact file fails validation");
        {
            const auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getChildFile("sidecar_manifest_test_noart");
            dir.createDirectory();

            juce::DynamicObject::Ptr artObj = new juce::DynamicObject();
            artObj->setProperty("path",
                                dir.getChildFile("missing_artifact.wav").getFullPathName());
            artObj->setProperty("byteSize", static_cast<juce::int64>(42));
            artObj->setProperty("sha256", juce::String(""));

            juce::Array<juce::var> artArray;
            artArray.add(juce::var(artObj.get()));

            juce::DynamicObject::Ptr manifest = new juce::DynamicObject();
            manifest->setProperty("protocolVersion", juce::String("1.0"));
            manifest->setProperty("requestId", juce::String("req-test-002"));
            manifest->setProperty("success", true);
            manifest->setProperty("artifacts", juce::var(artArray));

            const auto manifestFile = dir.getChildFile("manifest.json");
            manifestFile.replaceWithText(juce::JSON::toString(juce::var(manifest.get())));

            const auto err = validateResultManifest(manifestFile, "req-test-002");
            expect(err == SidecarProcessError::manifestInvalid,
                   "absent artifact file must yield manifestInvalid");

            manifestFile.deleteFile();
            dir.deleteRecursively();
        }
    }
};

static SidecarManifestTests sSidecarManifestTests;

} // namespace acestep_plugin
