#include "SidecarProcess.h"

#if JUCE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <vector>
#endif

namespace acestep_plugin
{

SidecarProcess::SidecarProcess() = default;

SidecarProcess::~SidecarProcess()
{
    cancel();

#if JUCE_WINDOWS
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

SidecarProcessError SidecarProcess::launch()
{
    if (!_pathSet)
        return SidecarProcessError::helperPathNotAbsolute;

    if (!_helperPath.existsAsFile())
        return SidecarProcessError::helperNotFound;

#if JUCE_WINDOWS
    // Create a job object with kill-on-close so the helper process tree is
    // terminated when this SidecarProcess is destroyed.
    auto* job = CreateJobObjectW(nullptr, nullptr);
    if (job != nullptr)
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(job, JobObjectExtendedLimitInformation,
                                &limits, sizeof(limits));
    }

    // Build a writable wide command-line buffer; lpCommandLine must be mutable.
    const juce::String cmdLine = "\"" + _helperPath.getFullPathName() + "\"";
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
    if (job != nullptr)
        AssignProcessToJobObject(job, pi.hProcess);

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
    if (_processHandle != nullptr)
        TerminateProcess(static_cast<HANDLE>(_processHandle), 1u);
#endif
}

SidecarProcessError SidecarProcess::waitForExit(int timeoutMs)
{
#if JUCE_WINDOWS
    if (_processHandle == nullptr)
        return SidecarProcessError::none;

    const DWORD ms = (timeoutMs < 0) ? INFINITE : static_cast<DWORD>(timeoutMs);
    const DWORD result = WaitForSingleObject(static_cast<HANDLE>(_processHandle), ms);

    return (result == WAIT_TIMEOUT) ? SidecarProcessError::timedOut
                                    : SidecarProcessError::none;
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
