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
 *    6. artifacts array is present.
 *    7. For each artifact: path exists on disk, byteSize matches, sha256
 *       matches (case-insensitive) when a non-empty digest is recorded.
 *
 *  @param manifestFile       JSON manifest file written by the sidecar helper.
 *  @param expectedRequestId  Request ID the manifest must carry.
 *
 *  @return SidecarProcessError::none on full validation success.
 *  @return SidecarProcessError::manifestInvalid on any check failure.
 */
SidecarProcessError validateResultManifest(const juce::File& manifestFile,
                                           const juce::String& expectedRequestId);

} // namespace acestep_plugin
