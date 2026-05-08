/** Manifest validation for the ACE-Step sidecar result protocol.
 *
 *  Validates a JSON manifest written by the sidecar helper against the
 *  expected request ID and artifact integrity data.  Logic is pure-functional;
 *  nothing here mutates process state.
 *
 *  Must not be called from the audio process-block thread.
 */
#pragma once

#include "SidecarProcess.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

/** Parse and validate a result manifest written by the sidecar helper.
 *
 *  Checks performed in order:
 *    1. Manifest file exists and is readable.
 *    2. JSON parses without error.
 *    3. protocolVersion field is present (non-empty).
 *    4. requestId matches expectedRequestId exactly.
 *    5. success field is true.
 *    6. artifacts array is present and non-empty.
 *    7. For each artifact: path exists on disk, file is non-empty, byteSize
 *       matches, and sha256 is present and matches case-insensitively.
 *
 *  @param manifestFile       JSON manifest file written by the sidecar helper.
 *  @param expectedRequestId  Request ID the manifest must carry.
 *
 *  @return SidecarProcessError::none on full validation success.
 *  @return SidecarProcessError::manifestInvalid on any check failure.
 */
SidecarProcessError validateResultManifest(const juce::File& manifestFile,
                                           const juce::String& expectedRequestId);

/** Validate a result manifest and return the paths of the validated artifacts.
 *
 *  Performs the same checks as validateResultManifest.  On success the returned
 *  StringArray contains the path of each validated artifact in manifest order.
 *  Returns an empty StringArray on any validation failure.
 *
 *  @param manifestFile       JSON manifest file written by the sidecar helper.
 *  @param expectedRequestId  Request ID the manifest must carry.
 *
 *  @return Non-empty list of artifact file paths on success; empty on failure.
 */
juce::StringArray getValidatedArtifactPaths(const juce::File& manifestFile,
                                            const juce::String& expectedRequestId);

} // namespace acestep_plugin
