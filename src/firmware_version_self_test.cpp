#include "firmware_version_self_test.h"

#include "firmware_version.h"

#include <cstring>

bool FirmwareVersionSelfTest::run()
{
    // Deliberate release assertion for v1.1.0.
    // Keep these expected values independent so an incomplete version bump
    // fails startup rather than silently publishing inconsistent identity.

    if (FirmwareVersion::MAJOR != 1)
        return false;

    if (FirmwareVersion::MINOR != 1)
        return false;

    if (FirmwareVersion::PATCH != 0)
        return false;

    if (std::strcmp(FirmwareVersion::STRING, "1.1.0") != 0)
        return false;

    if (FirmwareVersion::CRSF_ID != 0x01010000u)
        return false;

    return true;
}
