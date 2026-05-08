#include "SidecarProcess.h"

#if JUCE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <vector>
#endif

namespace acestep_plugin
{

SidecarProcess::SidecarProcess()
{
#if JUCE_WINDOWS
    _jobAssignFn = [](void* job, void* process) -> bool {
        return AssignProcessToJobObject(static_cast<HANDLE>(job),
                                       static_cast<HANDLE>(process)) != FALSE;
    };
    _jobConfigFn = [](void* job) -> bool {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        return SetInformationJobObject(static_cast<HANDLE>(job),
                                      JobObjectExtendedLimitInformation,
                                      &limits, sizeof(limits)) != FALSE;
    };
#endif
}

SidecarProcess::~SidecarProcess()
{
    cancel();

#if JUCE_WINDOWS
    // cancel() closes and nulls _processHandle; this is a safe no-op guard.
    if (_processHandle != nullptr)
    {
        CloseHandle(static_cast<HANDLE>(_processHandle));
        _processHandle = nullptr;
    }
    if (_jobHandle != nullptr)
    {
        CloseHandle(static_cast<HANDLE>(_jobHandle));
        _jobHandle = nullptr;
    }
#endif
}

SidecarProcessError SidecarProcess::setHelperPath(const juce::String& rawHelperPath)
{
    if (rawHelperPath.isEmpty() || !juce::File::isAbsolutePath(rawHelperPath))
        return SidecarProcessError::helperPathNotAbsolute;

    _helperPath = juce::File(rawHelperPath);
    _pathSet = true;
    return SidecarProcessError::none;
}

juce::File SidecarProcess::getHelperPath() const
{
    return _helperPath;
}

void SidecarProcess::setHelperArguments(const juce::String& args)
{
    _helperArgs = args;
}

SidecarProcessError SidecarProcess::launch()
{
    if (!_pathSet)
        return SidecarProcessError::helperPathNotAbsolute;

    if (!_helperPath.existsAsFile())
        return SidecarProcessError::helperNotFound;

#if JUCE_WINDOWS
    // Reject second launch while a process is already running.
    if (_processHandle != nullptr)
        return SidecarProcessError::alreadyRunning;

    // Create a job object with kill-on-close so the helper process tree is
    // terminated when this SidecarProcess is destroyed.
    auto* job = CreateJobObjectW(nullptr, nullptr);
    if (job != nullptr)
    {
        if (!_jobConfigFn(job))
        {
            CloseHandle(job);
            return SidecarProcessError::jobConfigurationFailed;
        }
    }

    // Build a writable wide command-line buffer; lpCommandLine must be mutable.
    juce::String cmdLine = "\"" + _helperPath.getFullPathName() + "\"";
    if (!_helperArgs.isEmpty())
        cmdLine += " " + _helperArgs;
    const auto* wCmd = cmdLine.toWideCharPointer();
    std::vector<wchar_t> cmdBuf(wCmd, wCmd + wcslen(wCmd) + 1);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    const BOOL ok = CreateProcessW(
        _helperPath.getFullPathName().toWideCharPointer(),
        cmdBuf.data(),
        nullptr, nullptr,
        FALSE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED,
        nullptr, nullptr,
        &si, &pi);

    if (!ok)
    {
        if (job != nullptr)
            CloseHandle(job);
        return SidecarProcessError::launchFailed;
    }

    // Assign to job object before resuming to avoid a race where the helper
    // exits before we can assign it.
    if (job != nullptr && !_jobAssignFn(job, pi.hProcess))
    {
        // Assignment failed: terminate the suspended process and clean up all
        // handles before returning the error.
        TerminateProcess(pi.hProcess, 1u);
        WaitForSingleObject(pi.hProcess, 1000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(job);
        return SidecarProcessError::jobAssignmentFailed;
    }

    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    _processHandle = pi.hProcess;
    _jobHandle = job;
    _pid = static_cast<juce::int64>(pi.dwProcessId);
    return SidecarProcessError::none;
#else
    return SidecarProcessError::launchFailed;
#endif
}

void SidecarProcess::cancel()
{
#if JUCE_WINDOWS
    if (_processHandle == nullptr)
        return;

    // Only invoke the cooperative-cancellation callback and force-terminate
    // if the process is still alive.  If it has already exited naturally,
    // simply clean up handles without invoking the callback.
    if (isRunning())
    {
        if (_cancellationCallback)
            _cancellationCallback();

        TerminateProcess(static_cast<HANDLE>(_processHandle), 1u);
    }

    // Close and null handles so a second cancel() (e.g. from the destructor)
    // is a safe no-op.
    CloseHandle(static_cast<HANDLE>(_processHandle));
    _processHandle = nullptr;

    if (_jobHandle != nullptr)
    {
        CloseHandle(static_cast<HANDLE>(_jobHandle));
        _jobHandle = nullptr;
    }
    _pid = 0;
#endif
}

void SidecarProcess::setCancellationCallback(CancellationCallback callback)
{
    _cancellationCallback = std::move(callback);
}

#if JUCE_WINDOWS
void SidecarProcess::setJobAssignmentFunction(JobAssignFn fn)
{
    _jobAssignFn = std::move(fn);
}

void SidecarProcess::setJobConfigureFunction(JobConfigureFn fn)
{
    _jobConfigFn = std::move(fn);
}
#endif

SidecarProcessError SidecarProcess::waitForExit(int timeoutMs)
{
#if JUCE_WINDOWS
    if (_processHandle == nullptr)
        return SidecarProcessError::none;

    const DWORD ms = (timeoutMs < 0) ? INFINITE : static_cast<DWORD>(timeoutMs);
    const DWORD result = WaitForSingleObject(static_cast<HANDLE>(_processHandle), ms);

    if (result != WAIT_TIMEOUT)
    {
        // Process exited naturally; close and null handles so that any
        // subsequent cancel() (e.g. from the destructor) is a safe no-op
        // and will not invoke the cancellation callback.
        CloseHandle(static_cast<HANDLE>(_processHandle));
        _processHandle = nullptr;
        if (_jobHandle != nullptr)
        {
            CloseHandle(static_cast<HANDLE>(_jobHandle));
            _jobHandle = nullptr;
        }
        _pid = 0;
        return SidecarProcessError::none;
    }

    return SidecarProcessError::timedOut;
#else
    return SidecarProcessError::none;
#endif
}

bool SidecarProcess::isRunning() const
{
#if JUCE_WINDOWS
    if (_processHandle == nullptr)
        return false;

    return WaitForSingleObject(static_cast<HANDLE>(_processHandle), 0) == WAIT_TIMEOUT;
#else
    return false;
#endif
}

juce::int64 SidecarProcess::getPid() const
{
#if JUCE_WINDOWS
    return _pid;
#else
    return 0;
#endif
}

} // namespace acestep_plugin
