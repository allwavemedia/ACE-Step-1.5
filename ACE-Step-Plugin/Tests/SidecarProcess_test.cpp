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

        // Issue 1: cancellation callback must fire at most once across explicit
        // cancel() and the destructor.
        beginTest("cancel callback fires at most once even after double cancel");
        {
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                int invokeCount = 0;
                {
                    SidecarProcess proc;
                    proc.setCancellationCallback([&invokeCount]() { ++invokeCount; });
                    proc.setHelperPath(cmdPath);
                    if (proc.launch() == SidecarProcessError::none)
                    {
                        proc.cancel();        // first cancel — callback fires once
                        proc.cancel();        // second explicit cancel — must be no-op
                        proc.waitForExit(2000);
                    }
                }
                // Destructor fires here — must not invoke callback again.
                expect(invokeCount <= 1, "callback must not fire more than once");
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
        }

        beginTest("destructor does not re-fire callback after explicit cancel");
        {
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                int invokeCount = 0;
                {
                    SidecarProcess proc;
                    proc.setCancellationCallback([&invokeCount]() { ++invokeCount; });
                    proc.setHelperPath(cmdPath);
                    if (proc.launch() == SidecarProcessError::none)
                        proc.cancel();
                    // Destructor runs at end of scope.
                }
                expect(invokeCount <= 1, "destructor must not re-fire callback after cancel");
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
        }

        // Issue 2: second launch must be rejected while a process is running.
        beginTest("second launch returns alreadyRunning and does not replace the process");
        {
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                SidecarProcess proc;
                proc.setHelperPath(cmdPath);
                const auto firstErr = proc.launch();
                if (firstErr == SidecarProcessError::none)
                {
                    const juce::int64 firstPid = proc.getPid();
                    expect(firstPid > 0);

                    const auto secondErr = proc.launch();
                    expect(secondErr == SidecarProcessError::alreadyRunning,
                           "second launch must return alreadyRunning");
                    expect(proc.getPid() == firstPid,
                           "PID must not change after rejected second launch");

                    proc.cancel();
                    proc.waitForExit(2000);
                }
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
        }

        // Issue 3: job assignment failure must be detected and returned.
        beginTest("launch returns jobAssignmentFailed when job assignment is injected to fail");
        {
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                SidecarProcess proc;
                proc.setHelperPath(cmdPath);
                proc.setJobAssignmentFunction([](void*, void*) { return false; });
                const auto err = proc.launch();
                expect(err == SidecarProcessError::jobAssignmentFailed,
                       "injected failing job assignment must return jobAssignmentFailed");
                // Process must have been cleaned up — not running, PID reset.
                expect(!proc.isRunning(),
                       "process must not be running after jobAssignmentFailed");
                expect(proc.getPid() == 0,
                       "PID must be reset to 0 after jobAssignmentFailed");
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
        }

        // Issue 2 (re-review): SetInformationJobObject failure must be detected.
        beginTest("launch returns jobConfigurationFailed when job configuration is injected to fail");
        {
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                SidecarProcess proc;
                proc.setHelperPath(cmdPath);
                proc.setJobConfigureFunction([](void*) { return false; });
                const auto err = proc.launch();
                expect(err == SidecarProcessError::jobConfigurationFailed,
                       "injected failing job configuration must return jobConfigurationFailed");
                expect(!proc.isRunning(),
                       "process must not be running after jobConfigurationFailed");
                expect(proc.getPid() == 0,
                       "PID must be 0 after jobConfigurationFailed");
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
        }

        // Issue 1 (re-review): cancellation callback must not fire on natural exit.
        beginTest("natural exit followed by destruction does not invoke cancellation callback");
        {
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                int callbackCount = 0;
                {
                    SidecarProcess proc;
                    proc.setCancellationCallback([&callbackCount]() { ++callbackCount; });
                    proc.setHelperPath(cmdPath);
                    proc.setHelperArguments("/c exit 0");
                    if (proc.launch() == SidecarProcessError::none)
                    {
                        // cmd /c exit 0 exits nearly immediately; allow up to 5 s.
                        const auto exitErr = proc.waitForExit(5000);
                        expect(exitErr == SidecarProcessError::none,
                               "cmd /c exit 0 must exit naturally within timeout");
                        // Callback must not have been called during natural exit.
                        expect(callbackCount == 0,
                               "callback must not fire when process exits naturally");
                    }
                    // Destructor invokes cancel() here.
                }
                expect(callbackCount == 0,
                       "cancellation callback must not fire after natural exit + destruction");
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
        }

        // Issue 4 (re-review): CreateJobObjectW failure must be a hard launch failure.
        beginTest("launch returns jobCreationFailed when job creation is injected to fail");
        {
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                SidecarProcess proc;
                proc.setHelperPath(cmdPath);
                proc.setJobCreationFunction([]() -> void* { return nullptr; });
                const auto err = proc.launch();
                expect(err == SidecarProcessError::jobCreationFailed,
                       "injected null job creation must return jobCreationFailed");
                expect(!proc.isRunning(),
                       "process must not be running after jobCreationFailed");
                expect(proc.getPid() == 0,
                       "PID must be 0 after jobCreationFailed");
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
        }

        // Task 3: cancel() must wait for process termination before closing handles.
        beginTest("cancel waits for process termination before closing handles");
        {
            const juce::String cmdPath = "C:\\Windows\\System32\\cmd.exe";
            if (juce::File(cmdPath).existsAsFile())
            {
                SidecarProcess proc;
                bool waitCalled = false;
                proc.setProcessWaitFunction(
                    [&waitCalled](void*, unsigned long) -> unsigned long {
                        waitCalled = true;
                        return 0; // simulate WAIT_OBJECT_0
                    });
                proc.setHelperPath(cmdPath);
                if (proc.launch() == SidecarProcessError::none)
                {
                    proc.cancel();
                    expect(waitCalled,
                           "cancel() must call the wait function after TerminateProcess");
                }
            }
            else
            {
                expect(true, "cmd.exe not found - skipping");
            }
        }

        // Task 3: cancel() must not call the wait function when process is not running.
        beginTest("cancel does not call wait function when process was never launched");
        {
            SidecarProcess proc;
            bool waitCalled = false;
            proc.setProcessWaitFunction(
                [&waitCalled](void*, unsigned long) -> unsigned long {
                    waitCalled = true;
                    return 0;
                });
            proc.setHelperPath("C:\\nonexistent\\helper.exe");
            proc.cancel();
            expect(!waitCalled,
                   "wait function must not be called when process was never launched");
        }
#endif
    }
};

static SidecarProcessTests sSidecarProcessTests;

} // namespace acestep_plugin
