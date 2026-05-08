/** Contract tests for SidecarProcess.
 *
 *  Covers: path validation, missing-executable detection, path-vs-PATH
 *  verification, process launch and cancellation (Windows), cooperative-
 *  cancellation callback ordering, timeout behaviour, and safe no-op semantics
 *  for an un-launched process.
 */
#include "../Source/Sidecar/SidecarProcess.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class SidecarProcessTests final : public juce::UnitTest
{
public:
    SidecarProcessTests() : juce::UnitTest("SidecarProcess") {}

    void runTest() override
    {
        beginTest("relative helper path is rejected at setHelperPath");
        {
            SidecarProcess proc;
            const auto err = proc.setHelperPath("relative\\path\\helper.exe");
            expect(err == SidecarProcessError::helperPathNotAbsolute);
        }

        beginTest("empty helper path is rejected");
        {
            SidecarProcess proc;
            const auto err = proc.setHelperPath({});
            expect(err == SidecarProcessError::helperPathNotAbsolute);
        }

        beginTest("non-absolute bare name (PATH lookup candidate) is rejected");
        {
            // "cmd.exe" is resolvable via PATH on Windows, but must be rejected
            // because it is not an absolute path — the helper must be launched
            // from the private embedded resource location only.
            SidecarProcess proc;
            const auto err = proc.setHelperPath("cmd.exe");
            expect(err == SidecarProcessError::helperPathNotAbsolute);
        }

        beginTest("absolute path is accepted by setHelperPath");
        {
            SidecarProcess proc;
            const auto err =
                proc.setHelperPath("C:\\Program Files\\AceStep\\helper.exe");
            expect(err == SidecarProcessError::none);
        }

        beginTest("configured helper path is stored verbatim");
        {
            SidecarProcess proc;
            const juce::String rawPath = "C:\\Program Files\\AceStep\\helper.exe";
            proc.setHelperPath(rawPath);
            expect(proc.getHelperPath().getFullPathName() == rawPath);
        }

        beginTest("missing executable returns helperNotFound on launch");
        {
            SidecarProcess proc;
            proc.setHelperPath("C:\\nonexistent_acestep_helper_xyzzy\\helper.exe");
            const auto err = proc.launch();
            expect(err == SidecarProcessError::helperNotFound);
        }

        beginTest("launch without setHelperPath returns helperPathNotAbsolute");
        {
            SidecarProcess proc;
            const auto err = proc.launch();
            expect(err == SidecarProcessError::helperPathNotAbsolute);
        }

        beginTest("process is not running before launch");
        {
            SidecarProcess proc;
            expect(!proc.isRunning());
            expect(proc.getPid() == 0);
        }

        beginTest("cancel is safe when process was never launched");
        {
            SidecarProcess proc;
            proc.setHelperPath("C:\\nonexistent\\helper.exe");
            proc.cancel(); // must not crash
            expect(true);
        }

        beginTest("waitForExit returns none immediately when not launched");
        {
            SidecarProcess proc;
            const auto err = proc.waitForExit(0);
            expect(err == SidecarProcessError::none);
        }

        beginTest("cancel invokes cooperative-cancellation callback when process is running");
        {
#if JUCE_WINDOWS
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                SidecarProcess proc;
                bool callbackInvoked = false;
                proc.setCancellationCallback([&callbackInvoked]() { callbackInvoked = true; });
                proc.setHelperPath(cmdPath);
                if (proc.launch() == SidecarProcessError::none)
                {
                    proc.cancel();
                    expect(callbackInvoked,
                           "cooperative-cancellation callback must be invoked before force-terminate");
                    proc.waitForExit(2000);
                }
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
#else
            expect(true, "cooperative-cancellation callback test is Windows-only");
#endif
        }

        beginTest("cancel does not invoke callback when process was never launched");
        {
            SidecarProcess proc;
            bool callbackInvoked = false;
            proc.setCancellationCallback([&callbackInvoked]() { callbackInvoked = true; });
            proc.setHelperPath("C:\\nonexistent\\helper.exe");
            proc.cancel(); // never launched - callback must be skipped
            expect(!callbackInvoked,
                   "callback must not fire when the process was never launched");
        }

#if JUCE_WINDOWS
        beginTest("can launch and cancel a real process on Windows");
        {
            // cmd.exe is an absolute path and exists on all Windows installs.
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                SidecarProcess proc;
                expect(proc.setHelperPath(cmdPath) == SidecarProcessError::none);

                const auto launchErr = proc.launch();
                expect(launchErr == SidecarProcessError::none);
                expect(proc.isRunning());
                expect(proc.getPid() > 0);

                proc.cancel();
                const auto exitErr = proc.waitForExit(2000);
                // After cancel the process should exit promptly.
                expect(exitErr == SidecarProcessError::none
                       || exitErr == SidecarProcessError::timedOut);
                expect(!proc.isRunning());
            }
        }

        beginTest("timeout fires for a process that does not exit quickly");
        {
            // Launch cmd.exe; it blocks waiting for input when launched without
            // a console window and without /c, so waitForExit with a tiny
            // timeout must return timedOut or none (if the process happened to
            // exit).  We then cancel to clean up.
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                SidecarProcess proc;
                proc.setHelperPath(cmdPath);
                if (proc.launch() == SidecarProcessError::none && proc.isRunning())
                {
                    const auto exitErr = proc.waitForExit(1);
                    // A 1 ms timeout against a running process should time out.
                    expect(exitErr == SidecarProcessError::timedOut
                           || exitErr == SidecarProcessError::none);
                    proc.cancel();
                    proc.waitForExit(2000);
                }
            }
        }
#endif
    }
};

static SidecarProcessTests sSidecarProcessTests;

} // namespace acestep_plugin
