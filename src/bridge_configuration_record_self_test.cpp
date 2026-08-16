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
            left.roll.inverted ==
                right.roll.inverted &&
            left.pitch.inverted ==
                right.pitch.inverted &&
            left.throttle.inverted ==
                right.throttle.inverted &&
            left.yaw.inverted ==
                right.yaw.inverted &&
            left.auxAnalog1.inverted ==
                right.auxAnalog1.inverted &&
            left.auxAnalog2.inverted ==
                right.auxAnalog2.inverted &&
            left.auxAnalog3.inverted ==
                right.auxAnalog3.inverted &&
            left.auxAnalog4.inverted ==
                right.auxAnalog4.inverted;
    }


    bool runRoundTripTest()
    {
        BridgeConfiguration source =
            BridgeConfiguration::defaults();


        source.ledBrightnessPercent = 73;
        source.roll.inverted = true;
        source.pitch.inverted = false;
        source.throttle.inverted = true;
        source.yaw.inverted = true;
        source.auxAnalog1.inverted = true;
        source.auxAnalog2.inverted = false;
        source.auxAnalog3.inverted = true;
        source.auxAnalog4.inverted = false;


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


        if (
            record[4] !=
                BridgeConfigurationRecord::
                    SCHEMA_VERSION ||
            record[5] != 3
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


    bool runLegacySchemaMigrationTest()
    {
        // Real schema-v1 record:
        //
        // LED = 73
        // Pitch = Normal
        // CRC-32 = 0x6FF9B73A
        //
        // New inversion fields did not exist in schema v1 and must therefore
        // retain BridgeConfiguration::defaults().
        constexpr uint8_t legacyRecord[
            BridgeConfigurationRecord::
                RECORD_SIZE
        ] =
        {
            'E', 'H', 'B', '1',
            0x01,
            0x02,
            0x49,
            0x00,
            0x00, 0x00, 0x00, 0x00,
            0x6F, 0xF9, 0xB7, 0x3A
        };


        BridgeConfiguration decoded =
            BridgeConfiguration::defaults();


        if (
            !BridgeConfigurationRecord::decode(
                legacyRecord,
                sizeof(legacyRecord),
                decoded
            )
        )
        {
            return false;
        }


        const BridgeConfiguration defaults =
            BridgeConfiguration::defaults();


        return
            decoded.ledBrightnessPercent == 73 &&
            !decoded.pitch.inverted &&
            decoded.roll.inverted ==
                defaults.roll.inverted &&
            decoded.throttle.inverted ==
                defaults.throttle.inverted &&
            decoded.yaw.inverted ==
                defaults.yaw.inverted &&
            decoded.auxAnalog1.inverted ==
                defaults.auxAnalog1.inverted &&
            decoded.auxAnalog2.inverted ==
                defaults.auxAnalog2.inverted &&
            decoded.auxAnalog3.inverted ==
                defaults.auxAnalog3.inverted &&
            decoded.auxAnalog4.inverted ==
                defaults.auxAnalog4.inverted;
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

        target.ledBrightnessPercent = 67;
        target.roll.inverted = true;
        target.pitch.inverted = false;


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


    bool runUnsupportedSchemaRejectedTest()
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

        source.roll.inverted =
            true;


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


        record[8] ^= 0x01;


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
        runLegacySchemaMigrationTest() &&
        runBadMagicRejectedTest() &&
        runUnsupportedSchemaRejectedTest() &&
        runCorruptionRejectedTest() &&
        runInvalidValueRejectedTest() &&
        runWrongSizeRejectedTest();
}
