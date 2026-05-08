/** JSON message-protocol client for the ACE-Step sidecar named pipe.
 *
 *  SidecarNamedPipeClient manages the request/response protocol layered on top
 *  of a SidecarTransport (injected for testability).  Production code injects a
 *  real Windows named-pipe transport; tests inject a FakeTransport.
 *
 *  All methods must be called from a background or message thread.
 *  Never call from processBlock() or any real-time audio callback.
 */
#pragma once

#include "SidecarRequest.h"

#include <juce_core/juce_core.h>

#include <memory>

namespace acestep_plugin
{

/** Error codes for named-pipe protocol operations. */
enum class SidecarClientError
{
    none,
    protocolVersionMismatch, /**< Helper reported a different protocol version. */
    missingCapability,       /**< Helper lacks a required capability. */
    requestIdMismatch,       /**< Response carried an unexpected request ID. */
    staleCompletion,         /**< Completion arrived for a cancelled/superseded request. */
    helperDisconnected,      /**< Pipe broke or helper exited mid-flight. */
};

/** Outcome of a performHandshake() call. */
struct HandshakeResult
{
    SidecarClientError error = SidecarClientError::none;
    juce::String helperProtocolVersion;
    juce::StringArray capabilities;
};

/** Outcome of a submitJob() call. */
struct SubmitResult
{
    SidecarClientError error = SidecarClientError::none;
    juce::String acknowledgedRequestId;
    juce::File artifactRoot;
};

/** One progress notification received from the helper. */
struct ProgressEvent
{
    juce::String requestId;
    float progressFraction = 0.0f;
    juce::String statusMessage;
    bool isComplete = false; /**< True when this event carries a "completion" message. */
};

// ---------------------------------------------------------------------------

/** Minimal transport abstraction; injected into SidecarNamedPipeClient.
 *
 *  Production: backed by a Windows named pipe.
 *  Tests: backed by FakeTransport with a pre-loaded response queue.
 */
class SidecarTransport
{
public:
    virtual ~SidecarTransport() = default;

    /** Send a JSON message object.  Returns false if the transport has failed. */
    virtual bool send(const juce::var& message) = 0;

    /** Block for up to timeoutMs and return the next JSON message, or a void
     *  var if no message arrived within the timeout or the transport is broken.
     */
    virtual juce::var receive(int timeoutMs) = 0;

    /** Return true while the transport channel is open and usable. */
    virtual bool isConnected() const = 0;
};

// ---------------------------------------------------------------------------

/** Manages the JSON message protocol between the plugin and the sidecar helper.
 *
 *  Inject a SidecarTransport to enable unit-testing without a real named pipe.
 *  A single client instance is shared by all ACE-Step plugin instances in the
 *  host process (one lazy helper per host process).
 */
class SidecarNamedPipeClient final
{
public:
    explicit SidecarNamedPipeClient(std::unique_ptr<SidecarTransport> transport);
    ~SidecarNamedPipeClient() = default;

    SidecarNamedPipeClient(const SidecarNamedPipeClient&) = delete;
    SidecarNamedPipeClient& operator=(const SidecarNamedPipeClient&) = delete;

    /** Perform the initial protocol handshake.
     *
     *  Sends a "handshake" message and waits for the helper's acknowledgement.
     *  Returns protocolVersionMismatch if the helper's version != expectedVersion.
     *  Returns missingCapability if any entry in requiredCapabilities is absent.
     *  Returns helperDisconnected if the transport is unavailable or times out.
     */
    HandshakeResult performHandshake(const juce::String& expectedVersion,
                                     const juce::StringArray& requiredCapabilities);

    /** Submit a job request and wait for the helper's acknowledgement.
     *
     *  Returns requestIdMismatch if the ack carries a different request ID.
     *  Returns helperDisconnected if the transport fails.
     */
    SubmitResult submitJob(const SidecarJobRequest& request);

    /** Poll for one progress/completion event.
     *
     *  Writes the event into outEvent (requestId is empty if no message arrived).
     *  Returns staleCompletion if a message arrives for a request ID other than
     *  activeRequestId (i.e. for a cancelled or superseded job).
     *  Returns helperDisconnected if the transport fails.
     */
    SidecarClientError pollProgress(const juce::String& activeRequestId,
                                    ProgressEvent& outEvent,
                                    int timeoutMs);

    /** Send a cancellation notification for the given request. */
    SidecarClientError sendCancellation(const juce::String& requestId);

    /** Return true if the underlying transport is connected. */
    bool isConnected() const;

private:
    std::unique_ptr<SidecarTransport> _transport;
};

} // namespace acestep_plugin
