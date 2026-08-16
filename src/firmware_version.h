#pragma once
#include <stdint.h>

namespace FirmwareVersion
{
    // -------------------------------------------------------------------------
    // Canonical firmware version
    // -------------------------------------------------------------------------
    //
    // v1.0.0 is the first stable-release identity for ELRS-HID-Bridge.
    // Runtime behavior is intentionally unchanged from the validated
    // v1.0.0-rc1 baseline; this transition advances release identity and
    // release documentation only.
    // -------------------------------------------------------------------------
    constexpr uint8_t MAJOR = 1;
    constexpr uint8_t MINOR = 0;
    constexpr uint8_t PATCH = 0;

    constexpr const char* PRERELEASE = "";
    constexpr const char* STRING = "1.0.0";

    // CRSF Device Info defines Firmware_ID as an opaque uint32_t. The project
    // uses a deterministic packed semantic-version representation:
    //
    // bits 31..24 = major
    // bits 23..16 = minor
    // bits 15..8  = patch
    // bits 7..0   = reserved (currently zero)
    //
    // 1.0.0 therefore encodes as 0x01000000.
    //
    // Prerelease labels are human-readable metadata and are deliberately not
    // encoded into the CRSF numeric field. Therefore v1.0.0-rc1 and v1.0.0
    // intentionally share CRSF_ID 0x01000000.
    constexpr uint32_t CRSF_ID =
        (static_cast<uint32_t>(MAJOR) << 24) |
        (static_cast<uint32_t>(MINOR) << 16) |
        (static_cast<uint32_t>(PATCH) << 8);
}
