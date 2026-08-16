#pragma once

#include <stddef.h>
#include <stdint.h>


class CrsfFrameEncoder
{
public:
    static constexpr size_t MAX_FRAME_SIZE = 64;


    // Encode a CRSF extended-header frame:
    //
    // Sync
    // Length
    // Type
    // Destination
    // Origin
    // Payload...
    // CRC
    //
    // Returns true on success.
    //
    // outputLength is set to zero on failure.
    bool encodeExtended(
        uint8_t syncByte,
        uint8_t frameType,
        uint8_t destination,
        uint8_t origin,
        const uint8_t* payload,
        size_t payloadLength,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


private:
    static uint8_t crc8DvbS2(
        const uint8_t* data,
        size_t length
    );
};