#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bridge_configuration.h"


class BridgeConfigurationRecord
{
public:
    static constexpr size_t RECORD_SIZE = 16;

    static constexpr uint8_t SCHEMA_VERSION = 2;

    static constexpr uint8_t LEGACY_SCHEMA_VERSION = 1;


    // Schema v2 layout:
    //
    //  0..3   Magic "EHB1"
    //  4      Schema version = 2
    //  5      Payload length = 3
    //  6      LED brightness percent
    //  7      Pitch inversion (0 = normal, 1 = inverted)
    //  8      Additional inversion bitmask:
    //           bit 0 Roll
    //           bit 1 Throttle
    //           bit 2 Yaw
    //           bit 3 Aux 1
    //           bit 4 Aux 2
    //           bit 5 Aux 3
    //           bit 6 Aux 4
    //           bit 7 reserved, must be zero
    //  9..11  Reserved, zero
    // 12..15  CRC-32 of bytes 0..11, big endian
    //
    // Schema v1 records remain readable. Their saved LED/Pitch settings are
    // retained and all newly introduced inversion fields use current defaults.
    static bool encode(
        const BridgeConfiguration& configuration,
        uint8_t* output,
        size_t outputCapacity
    );


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
