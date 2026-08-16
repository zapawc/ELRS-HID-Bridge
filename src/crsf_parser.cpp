#include "crsf_parser.h"

#include "crsf_protocol.h"


namespace
{
    constexpr size_t CRSF_HEADER_SIZE = 2;
    constexpr size_t CRSF_LENGTH_OFFSET = 1;
    constexpr size_t CRSF_TYPE_OFFSET = 2;

    constexpr uint8_t CRSF_MIN_LENGTH = 2;
    constexpr uint8_t CRSF_MAX_LENGTH = 62;


    uint8_t crc8DvbS2(
        const uint8_t* data,
        size_t length
    )
    {
        uint8_t crc = 0;

        for (size_t i = 0; i < length; ++i)
        {
            crc ^= data[i];

            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                if (crc & 0x80)
                {
                    crc = static_cast<uint8_t>(
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
}


void CrsfParser::reset()
{
    frameIndex = 0;
    expectedFrameSize = 0;
}


bool CrsfParser::pushByte(
    uint8_t byte,
    CrsfFrame& frame
)
{
    // -------------------------------------------------------------------------
    // Synchronization
    //
    // CRSF permits the first byte of a frame to be:
    //
    // - the traditional serial sync byte,
    // - the broadcast address, or
    // - a valid CRSF device address.
    //
    // Do not assume that all valid CRSF traffic begins with 0xC8.
    // -------------------------------------------------------------------------

    if (frameIndex == 0)
    {
        if (!Crsf::isValidSyncByte(byte))
        {
            return false;
        }

        frameBuffer[frameIndex++] = byte;

        return false;
    }


    // -------------------------------------------------------------------------
    // Length byte
    // -------------------------------------------------------------------------

    if (frameIndex == 1)
    {
        if (
            byte < CRSF_MIN_LENGTH ||
            byte > CRSF_MAX_LENGTH
        )
        {
            // Invalid candidate frame.
            //
            // If this byte can itself begin another CRSF frame,
            // preserve it as the first byte of the next candidate.
            frameIndex = 0;
            expectedFrameSize = 0;

            if (Crsf::isValidSyncByte(byte))
            {
                frameBuffer[frameIndex++] = byte;
            }

            return false;
        }

        frameBuffer[frameIndex++] = byte;

        expectedFrameSize =
            static_cast<size_t>(byte) +
            CRSF_HEADER_SIZE;

        return false;
    }


    // -------------------------------------------------------------------------
    // Remaining frame bytes
    // -------------------------------------------------------------------------

    if (frameIndex >= MAX_FRAME_SIZE)
    {
        frameIndex = 0;
        expectedFrameSize = 0;

        return false;
    }

    frameBuffer[frameIndex++] = byte;

    if (
        expectedFrameSize != 0 &&
        frameIndex == expectedFrameSize
    )
    {
        const bool validFrame =
            processFrame(frame);

        // Always return to synchronization-search mode after processing a
        // complete candidate frame.
        //
        // The contents of frameBuffer are deliberately left intact so that
        // the returned CrsfFrame payload view remains valid until the next
        // byte is supplied to this parser.
        frameIndex = 0;
        expectedFrameSize = 0;

        return validFrame;
    }

    return false;
}


bool CrsfParser::processFrame(
    CrsfFrame& frame
)
{
    if (frameIndex < 4)
    {
        return false;
    }

    const size_t crcIndex =
        frameIndex - 1;

    const uint8_t receivedCrc =
        frameBuffer[crcIndex];


    // CRSF CRC covers Type + Payload only.
    //
    // Address, Length, and the CRC byte itself are excluded.

    const uint8_t calculatedCrc =
        crc8DvbS2(
            &frameBuffer[CRSF_TYPE_OFFSET],
            crcIndex - CRSF_TYPE_OFFSET
        );

    if (receivedCrc != calculatedCrc)
    {
        return false;
    }


    // -------------------------------------------------------------------------
    // Construct validated frame view
    // -------------------------------------------------------------------------

    frame.address =
        frameBuffer[0];

    frame.length =
        frameBuffer[CRSF_LENGTH_OFFSET];

    frame.type =
        frameBuffer[CRSF_TYPE_OFFSET];


    // CRSF Length includes:
    //
    // Type + Payload + CRC
    //
    // Therefore:
    //
    // Payload Length = Length - Type - CRC

    frame.payloadLength =
        static_cast<uint8_t>(
            frame.length - 2
        );

    if (frame.payloadLength > 0)
    {
        frame.payload =
            &frameBuffer[3];
    }
    else
    {
        frame.payload =
            nullptr;
    }

    return true;
}