#pragma once

#include <stdint.h>


namespace FirmwareVersion
{
    // -------------------------------------------------------------------------
    // Canonical firmware version
    // -------------------------------------------------------------------------
    //
    // Keep project firmware version information here rather than duplicating it
    // across CRSF identity, documentation, and future release metadata.
    //
    // The current tree remains pre-v1.0, so the existing documented 0.3 version
    // is normalized to semantic version form as 0.3.0-dev rather than being
    // arbitrarily incremented by this refactor.
    // -------------------------------------------------------------------------

    constexpr uint8_t MAJOR = 0;
    constexpr uint8_t MINOR = 3;
    constexpr uint8_t PATCH = 0;

    constexpr const char* PRERELEASE = "dev";
    constexpr const char* STRING = "0.3.0-dev";


    // CRSF Device Info defines Firmware_ID as an opaque uint32_t. The project
    // uses a deterministic packed semantic-version representation:
    //
    // bits 31..24 = major
    // bits 23..16 = minor
    // bits 15..8  = patch
    // bits 7..0   = reserved (currently zero)
    //
    // 0.3.0 therefore encodes as 0x00030000.
    //
    // The prerelease label remains human-readable metadata and is deliberately
    // not encoded into the CRSF numeric field.

    constexpr uint32_t CRSF_ID =
        (static_cast<uint32_t>(MAJOR) << 24) |
        (static_cast<uint32_t>(MINOR) << 16) |
        (static_cast<uint32_t>(PATCH) << 8);
}
