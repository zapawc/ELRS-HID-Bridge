#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bridge_configuration.h"


class BridgeConfigurationRecord
{
public:
    static constexpr size_t RECORD_SIZE = 16;

    static constexpr uint8_t SCHEMA_VERSION = 1;


    // Serialize only the currently persistent user settings.
    //
    // Record layout:
    //
    //  0..3   Magic "EHB1"
    //  4      Schema version
    //  5      Payload length
    //  6      LED brightness percent
    //  7      Pitch inversion (0 = normal, 1 = inverted)
    //  8..11  Reserved, zero
    // 12..15  CRC-32 of bytes 0..11, big endian
    static bool encode(
        const BridgeConfiguration& configuration,
        uint8_t* output,
        size_t outputCapacity
    );


    // Decode and validate a complete record.
    //
    // On failure, configuration is left unchanged.
    //
    // On success, a known-good default configuration is created and the
    // persistent fields from the record are overlaid onto it. This prevents
    // future non-persistent fields from inheriting undefined storage data.
    static bool decode(
        const uint8_t* data,
        size_t length,
        BridgeConfiguration& configuration
    );


private:
    static uint32_t crc32(
        const uint8_t* data,
        size_t length
    );


    static void writeUint32BigEndian(
        uint32_t value,
        uint8_t* output
    );


    static uint32_t readUint32BigEndian(
        const uint8_t* input
    );
};
