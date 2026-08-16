#include "bridge_configuration_store.h"

#include <EEPROM.h>
#include <stddef.h>
#include <stdint.h>

#include "bridge_configuration_record.h"


namespace
{
    // Arduino-Pico emulates EEPROM in one 4 KiB flash sector.
    //
    // EEPROM.begin() accepts 256..4096 bytes and rounds the requested size to
    // a 256-byte flash-programming boundary. We only need 16 bytes, so use the
    // smallest supported working buffer.
    constexpr size_t EEPROM_WORKING_SIZE = 256;

    constexpr int CONFIGURATION_OFFSET = 0;
}


bool BridgeConfigurationStore::load(
    BridgeConfiguration& configuration
) const
{
    EEPROM.begin(
        EEPROM_WORKING_SIZE
    );


    uint8_t record[
        BridgeConfigurationRecord::
            RECORD_SIZE
    ] = {};


    for (
        size_t index = 0;
        index < sizeof(record);
        ++index
    )
    {
        record[index] =
            EEPROM.read(
                CONFIGURATION_OFFSET +
                static_cast<int>(
                    index
                )
            );
    }


    // end() commits only when the EEPROM buffer is dirty and then releases the
    // RAM copy. A read-only load therefore does not cause a flash erase/write.
    EEPROM.end();


    return
        BridgeConfigurationRecord::decode(
            record,
            sizeof(record),
            configuration
        );
}


bool BridgeConfigurationStore::save(
    const BridgeConfiguration& configuration
) const
{
    uint8_t record[
        BridgeConfigurationRecord::
            RECORD_SIZE
    ] = {};


    if (
        !BridgeConfigurationRecord::encode(
            configuration,
            record,
            sizeof(record)
        )
    )
    {
        return false;
    }


    EEPROM.begin(
        EEPROM_WORKING_SIZE
    );


    for (
        size_t index = 0;
        index < sizeof(record);
        ++index
    )
    {
        EEPROM.write(
            CONFIGURATION_OFFSET +
                static_cast<int>(
                    index
                ),
            record[index]
        );
    }


    // commit() returns true when:
    // - the buffer is unchanged (no flash write required), or
    // - a dirty buffer is successfully erased/programmed.
    const bool committed =
        EEPROM.commit();


    // commit() above clears the dirty flag on success. end() then releases the
    // RAM copy without causing another flash erase/program operation.
    const bool ended =
        EEPROM.end();


    return
        committed &&
        ended;
}
