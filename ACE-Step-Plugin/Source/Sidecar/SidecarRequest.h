/** Versioned request/result types for the ACE-Step sidecar IPC protocol.
 *
 *  All heavyweight work (generation, MIDI transcription, stem separation) is
 *  dispatched through these envelopes so that request IDs, session IDs, and
 *  artifact paths can be correlated across the plugin/helper boundary.
 */
#pragma once

#include <juce_core/juce_core.h>

#include <vector>

namespace acestep_plugin
{

/** Wire protocol version carried in every sidecar envelope. */
constexpr const char* kSidecarProtocolVersion = "1.0";

/** Identifies the kind of work a job envelope carries. */
enum class SidecarJobKind
{
    generation, /**< ACE-Step audio generation. */
    midi,       /**< Basic-Pitch / MT3 MIDI transcription. */
    stems,      /**< Demucs / SCNet stem separation. */
};

/** One artifact file produced by a completed job. */
struct SidecarArtifact
{
    /** Absolute path to the file on disk. */
    juce::File path;

    /** Byte size of the file at completion time. */
    juce::int64 byteSize = 0;

    /** Lowercase hex SHA-256 digest of the file content. */
    juce::String sha256;
};

/** Versioned request envelope sent to the sidecar helper over the named pipe. */
struct SidecarJobRequest
{
    /** Protocol version; must equal kSidecarProtocolVersion. */
    juce::String protocolVersion { kSidecarProtocolVersion };

    /** UUIDv4 assigned by the caller before submission. */
    juce::String requestId;

    /** Plugin instance / editor session UUID for correlation. */
    juce::String sessionId;

    /** Kind of work being requested. */
    SidecarJobKind jobKind = SidecarJobKind::generation;

    /** Absolute paths to any input files (reference audio, stems, etc.). */
    std::vector<juce::File> inputPaths;

    /** Absolute directory where all output artifacts will be written. */
    juce::File outputDirectory;

    /** Opaque JSON blob forwarded verbatim to the helper for job parameters. */
    juce::String parametersJson;

    /** Set to true by the plugin to request cooperative cancellation. */
    bool cancellationRequested = false;
};

/** Outcome delivered by the sidecar helper when a job finishes or fails. */
struct SidecarJobResult
{
    /** Must match the requestId from the originating SidecarJobRequest. */
    juce::String requestId;

    /** True iff the job completed successfully and all artifacts are valid. */
    bool success = false;

    /** Artifacts produced; empty on failure. */
    std::vector<SidecarArtifact> artifacts;

    /** Non-fatal warnings the helper accumulated during the job. */
    juce::StringArray warnings;

    /** Path to the helper-written diagnostics / log file; may be empty. */
    juce::File diagnosticsPath;

    /** Version string of the backend (e.g. ACE-Step model version). */
    juce::String backendVersion;

    /** Version string of the tool (e.g. Python package version). */
    juce::String toolVersion;

    /** Non-zero numeric error code on failure; zero on success. */
    int errorCode = 0;

    /** Human-readable error description on failure; empty on success. */
    juce::String errorMessage;
};

} // namespace acestep_plugin
