/** Windows process lifecycle wrapper for the ACE-Step sidecar helper.
 *
 *  Launches the helper with CreateProcessW (CREATE_NO_WINDOW) and assigns it
 *  to a Windows job object (JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE) so that the
 *  entire helper process tree is cleaned up when this object is destroyed.
 *
 *  Must not be constructed or accessed from the audio process-block thread.
 */
#pragma once

#include <functional>

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

/** Error codes returned by SidecarProcess operations. */
enum class SidecarProcessError
{
    none,
    helperPathNotAbsolute, /**< The supplied path is relative or empty. */
    helperNotFound,        /**< No executable exists at the configured path. */
    launchFailed,          /**< CreateProcessW failed (check GetLastError). */
    timedOut,              /**< waitForExit() exceeded its timeout. */
    cancelled,             /**< The job was cancelled before or during execution. */
    helperDisconnected,    /**< The pipe broke or the process exited unexpectedly. */
    manifestInvalid,       /**< The output manifest is absent or malformed. */
};

/** Wraps one sidecar helper process and its associated Windows job object.
 *
 *  All public methods must be called from a background or message thread.
 *  Never call from processBlock() or any real-time audio callback.
 */
class SidecarProcess final
{
public:
    /** Callable invoked by cancel() before force-termination. */
    using CancellationCallback = std::function<void()>;

    SidecarProcess();
    ~SidecarProcess();

    SidecarProcess(const SidecarProcess&) = delete;
    SidecarProcess& operator=(const SidecarProcess&) = delete;

    /** Configure the absolute path to the sidecar helper executable.
     *
     *  The path must be absolute; relative paths and empty strings are rejected.
     *  Returns helperPathNotAbsolute on failure.  Does not touch the filesystem.
     *
     *  @param rawHelperPath  Raw path string (not yet normalised to juce::File).
     */
    SidecarProcessError setHelperPath(const juce::String& rawHelperPath);

    /** Return the helper path as a juce::File (empty if not yet set). */
    juce::File getHelperPath() const;

    /** Launch the helper process.
     *
     *  Uses CreateProcessW with CREATE_NO_WINDOW | CREATE_SUSPENDED, assigns
     *  the process to a job object with kill-on-close, then resumes the thread.
     *  Must be called from a background or message thread only.
     *
     *  Returns:
     *  - helperPathNotAbsolute if setHelperPath() has not been called successfully.
     *  - helperNotFound if the executable does not exist on disk.
     *  - launchFailed if CreateProcessW fails.
     */
    SidecarProcessError launch();

    /** Request process termination.
     *
     *  If a CancellationCallback has been set and the process is running,
     *  the callback is invoked first for cooperative cancellation before
     *  force-termination.  Safe to call even if the process has already
     *  exited or was never launched.
     */
    void cancel();

    /** Register a callback to be invoked before force-termination.
     *
     *  The callback runs synchronously inside cancel() only when the process
     *  has been successfully launched.  Pass an empty function to clear.
     */
    void setCancellationCallback(CancellationCallback callback);

    /** Block until the process exits or timeoutMs elapses.
     *
     *  @param timeoutMs  Maximum wait time in milliseconds; negative means infinite.
     *  Returns timedOut if the process is still running after the timeout.
     *  Returns none immediately if the process was never launched.
     */
    SidecarProcessError waitForExit(int timeoutMs);

    /** Return true if the helper process is currently running. */
    bool isRunning() const;

    /** Return the OS process ID, or 0 if the process was never launched. */
    juce::int64 getPid() const;

private:
    juce::File _helperPath;
    bool _pathSet = false;
    CancellationCallback _cancellationCallback;

#if JUCE_WINDOWS
    void* _processHandle = nullptr; /**< HANDLE from CreateProcessW. */
    void* _jobHandle = nullptr;     /**< Job object handle for kill-on-close. */
    juce::int64 _pid = 0;
#endif
};

} // namespace acestep_plugin
