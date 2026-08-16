#include "crsf_frame_encoder.h"

#include "crsf_protocol.h"


uint8_t CrsfFrameEncoder::crc8DvbS2(
    const uint8_t* data,
    size_t length
)
{
    uint8_t crc = 0;


    for (
        size_t index = 0;
        index < length;
        ++index
    )
    {
        crc ^=
            data[index];


        for (
            uint8_t bit = 0;
            bit < 8;
            ++bit
        )
        {
            if (crc & 0x80)
            {
                crc =
                    static_cast<uint8_t>(
                        (crc << 1) ^
                        Crsf::CRC_POLYNOMIAL
                    );
            }
            else
            {
                crc <<= 1;
            }
        }
    }


    return crc;
}


bool CrsfFrameEncoder::encodeExtended(
    uint8_t syncByte,
    uint8_t frameType,
    uint8_t destination,
    uint8_t origin,
    const uint8_t* payload,
    size_t payloadLength,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    // -------------------------------------------------------------------------
    // Validate caller-provided storage.
    // -------------------------------------------------------------------------

    if (output == nullptr)
    {
        return false;
    }


    if (
        payloadLength > 0 &&
        payload == nullptr
    )
    {
        return false;
    }


    // -------------------------------------------------------------------------
    // Extended CRSF frame sizing
    //
    // CRSF Length includes:
    //
    // Type
    // Destination
    // Origin
    // Payload
    // CRC
    //
    // Therefore:
    //
    // Length = Payload + 4
    //
    // Complete frame adds:
    //
    // Sync + Length byte
    //
    // Total = Payload + 6
    // -------------------------------------------------------------------------

    const size_t crsfLength =
        payloadLength + 4;


    const size_t totalFrameSize =
        payloadLength + 6;


    if (
        crsfLength >
        Crsf::MAX_FRAME_LENGTH
    )
    {
        return false;
    }


    if (
        totalFrameSize >
        MAX_FRAME_SIZE
    )
    {
        return false;
    }


    if (
        outputCapacity <
        totalFrameSize
    )
    {
        return false;
    }


    // -------------------------------------------------------------------------
    // Frame header
    // -------------------------------------------------------------------------

    output[0] =
        syncByte;


    output[1] =
        static_cast<uint8_t>(
            crsfLength
        );


    output[2] =
        frameType;


    output[3] =
        destination;


    output[4] =
        origin;


    // -------------------------------------------------------------------------
    // Payload
    // -------------------------------------------------------------------------

    for (
        size_t index = 0;
        index < payloadLength;
        ++index
    )
    {
        output[5 + index] =
            payload[index];
    }


    // -------------------------------------------------------------------------
    // CRC
    //
    // CRSF CRC covers everything beginning with Type and ending
    // with the final payload byte.
    //
    // For an extended frame that therefore includes:
    //
    // Type + Destination + Origin + Payload
    // -------------------------------------------------------------------------

    const size_t crcIndex =
        totalFrameSize - 1;


    output[crcIndex] =
        crc8DvbS2(
            &output[2],
            3 + payloadLength
        );


    outputLength =
        totalFrameSize;


    return true;
}