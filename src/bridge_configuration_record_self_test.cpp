#include "bridge_configuration_record_self_test.h"

#include <stddef.h>
#include <stdint.h>

#include "bridge_configuration.h"
#include "bridge_configuration_record.h"


namespace
{
    bool configurationsMatchPersistentFields(
        const BridgeConfiguration& left,
        const BridgeConfiguration& right
    )
    {
        return
            left.ledBrightnessPercent ==
                right.ledBrightnessPercent &&
            left.pitch.inverted ==
                right.pitch.inverted;
    }


    bool runRoundTripTest()
    {
        BridgeConfiguration source =
            BridgeConfiguration::defaults();


        source.ledBrightnessPercent =
            73;

        source.pitch.inverted =
            false;


        uint8_t record[
            BridgeConfigurationRecord::
                RECORD_SIZE
        ] = {};


        if (
            !BridgeConfigurationRecord::encode(
                source,
                record,
                sizeof(record)
            )
        )
        {
            return false;
        }


        BridgeConfiguration decoded =
            BridgeConfiguration::defaults();


        if (
            !BridgeConfigurationRecord::decode(
                record,
                sizeof(record),
                decoded
            )
        )
        {
            return false;
        }


        return
            configurationsMatchPersistentFields(
                source,
                decoded
            );
    }


    bool runBadMagicRejectedTest()
    {
        BridgeConfiguration source =
            BridgeConfiguration::defaults();


        uint8_t record[
            BridgeConfigurationRecord::
                RECORD_SIZE
        ] = {};


        if (
            !BridgeConfigurationRecord::encode(
                source,
                record,
                sizeof(record)
            )
        )
        {
            return false;
        }


        record[0] ^= 0x01;


        BridgeConfiguration target =
            BridgeConfiguration::defaults();

        target.ledBrightnessPercent =
            67;

        target.pitch.inverted =
            false;


        const BridgeConfiguration before =
            target;


        if (
            BridgeConfigurationRecord::decode(
                record,
                sizeof(record),
                target
            )
        )
        {
            return false;
        }


        return
            configurationsMatchPersistentFields(
                before,
                target
            );
    }


    bool runWrongSchemaRejectedTest()
    {
        BridgeConfiguration source =
            BridgeConfiguration::defaults();


        uint8_t record[
            BridgeConfigurationRecord::
                RECORD_SIZE
        ] = {};


        if (
            !BridgeConfigurationRecord::encode(
                source,
                record,
                sizeof(record)
            )
        )
        {
            return false;
        }


        // Schema is protected by CRC. The test intentionally leaves the
        // original CRC in place: either the schema gate or CRC gate must reject
        // the record, and configuration must remain unchanged.
        record[4] =
            static_cast<uint8_t>(
                BridgeConfigurationRecord::
                    SCHEMA_VERSION + 1
            );


        BridgeConfiguration target =
            BridgeConfiguration::defaults();

        target.ledBrightnessPercent =
            42;


        const BridgeConfiguration before =
            target;


        if (
            BridgeConfigurationRecord::decode(
                record,
                sizeof(record),
                target
            )
        )
        {
            return false;
        }


        return
            configurationsMatchPersistentFields(
                before,
                target
            );
    }


    bool runCorruptionRejectedTest()
    {
        BridgeConfiguration source =
            BridgeConfiguration::defaults();

        source.ledBrightnessPercent =
            55;


        uint8_t record[
            BridgeConfigurationRecord::
                RECORD_SIZE
        ] = {};


        if (
            !BridgeConfigurationRecord::encode(
                source,
                record,
                sizeof(record)
            )
        )
        {
            return false;
        }


        // Corrupt a payload byte while retaining the original CRC.
        record[6] ^= 0x01;


        BridgeConfiguration target =
            BridgeConfiguration::defaults();

        target.ledBrightnessPercent =
            88;

        target.pitch.inverted =
            false;


        const BridgeConfiguration before =
            target;


        if (
            BridgeConfigurationRecord::decode(
                record,
                sizeof(record),
                target
            )
        )
        {
            return false;
        }


        return
            configurationsMatchPersistentFields(
                before,
                target
            );
    }


    bool runInvalidValueRejectedTest()
    {
        // A record with a valid CRC but an invalid LED value is easiest to
        // produce by first verifying that encode itself refuses it.
        BridgeConfiguration invalid =
            BridgeConfiguration::defaults();


        invalid.ledBrightnessPercent =
            101;


        uint8_t record[
            BridgeConfigurationRecord::
                RECORD_SIZE
        ] = {};


        return
            !BridgeConfigurationRecord::encode(
                invalid,
                record,
                sizeof(record)
            );
    }


    bool runWrongSizeRejectedTest()
    {
        BridgeConfiguration source =
            BridgeConfiguration::defaults();


        uint8_t record[
            BridgeConfigurationRecord::
                RECORD_SIZE
        ] = {};


        if (
            !BridgeConfigurationRecord::encode(
                source,
                record,
                sizeof(record)
            )
        )
        {
            return false;
        }


        BridgeConfiguration target =
            BridgeConfiguration::defaults();


        return
            !BridgeConfigurationRecord::decode(
                record,
                sizeof(record) - 1,
                target
            );
    }
}


bool BridgeConfigurationRecordSelfTest::run()
{
    return
        runRoundTripTest() &&
        runBadMagicRejectedTest() &&
        runWrongSchemaRejectedTest() &&
        runCorruptionRejectedTest() &&
        runInvalidValueRejectedTest() &&
        runWrongSizeRejectedTest();
}
