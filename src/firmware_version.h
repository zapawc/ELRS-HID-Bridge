#pragma once

#include <stdint.h>

namespace FirmwareVersion
{
    constexpr uint8_t MAJOR = 1;
    constexpr uint8_t MINOR = 1;
    constexpr uint8_t PATCH = 0;

    constexpr const char* PRERELEASE = "";
    constexpr const char* STRING = "1.1.0";

    // CRSF Device Info Firmware_ID:
    // bits 31..24 = major
    // bits 23..16 = minor
    // bits 15..8  = patch
    // bits 7..0   = reserved
    constexpr uint32_t CRSF_ID =
        (static_cast<uint32_t>(MAJOR) << 24) |
        (static_cast<uint32_t>(MINOR) << 16) |
        (static_cast<uint32_t>(PATCH) << 8);
}
