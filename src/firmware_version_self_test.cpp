#include "firmware_version_self_test.h"

#include <string.h>
#include "bridge_identity.h"
#include "firmware_version.h"


namespace
{
    bool runSemanticVersionTest()
    {
        if (FirmwareVersion::MAJOR != 1)
        {
            return false;
        }

        if (FirmwareVersion::MINOR != 0)
        {
            return false;
        }

        if (FirmwareVersion::PATCH != 0)
        {
            return false;
        }

        if (
            strcmp(
                FirmwareVersion::PRERELEASE,
                "rc1"
            ) != 0
        )
        {
            return false;
        }

        if (
            strcmp(
                FirmwareVersion::STRING,
                "1.0.0-rc1"
            ) != 0
        )
        {
            return false;
        }

        return true;
    }


    bool runPackedIdTest()
    {
        if (
            FirmwareVersion::CRSF_ID !=
            0x01000000u
        )
        {
            return false;
        }

        // BridgeIdentity must consume the canonical version source rather than
        // maintaining an independent firmware identifier.
        if (
            BridgeIdentity::CRSF_FIRMWARE_ID !=
            FirmwareVersion::CRSF_ID
        )
        {
            return false;
        }

        return true;
    }
}


bool FirmwareVersionSelfTest::run()
{
    return
        runSemanticVersionTest() &&
        runPackedIdTest();
}
