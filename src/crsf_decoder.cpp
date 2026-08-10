#include "crsf_decoder.h"

namespace
{
    constexpr size_t CRSF_HEADER_SIZE = 2;
    constexpr size_t CRSF_LENGTH_OFFSET = 1;

    constexpr uint8_t CRSF_CRC_POLY = 0xD5;

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
                    crc =
                        static_cast<uint8_t>(
                            (crc << 1) ^ CRSF_CRC_POLY
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

void CrsfDecoder::reset()
{
    frameIndex = 0;
    expectedFrameSize = 0;
    newChannels = false;
}

void CrsfDecoder::pushByte(uint8_t byte)
{
    if (frameIndex >= MAX_FRAME_SIZE)
    {
        reset();
    }

    frameBuffer[frameIndex++] = byte;

    if (frameIndex == CRSF_HEADER_SIZE)
    {
        const uint8_t crsfLength =
            frameBuffer[CRSF_LENGTH_OFFSET];

        expectedFrameSize =
            static_cast<size_t>(crsfLength) + 2;

        // Smallest useful CRSF frame is:
        //
        // Address
        // Length
        // Type
        // CRC
        //
        if (
            expectedFrameSize < 4 ||
            expectedFrameSize > MAX_FRAME_SIZE
        )
        {
            reset();
            return;
        }
    }

    if (
        expectedFrameSize != 0 &&
        frameIndex == expectedFrameSize
    )
    {
        processFrame();

        frameIndex = 0;
        expectedFrameSize = 0;
    }
}

bool CrsfDecoder::hasNewChannels() const
{
    return newChannels;
}

const RawChannels& CrsfDecoder::getChannels() const
{
    return channels;
}

void CrsfDecoder::clearNewChannels()
{
    newChannels = false;
}

void CrsfDecoder::processFrame()
{
    // Frame layout:
    //
    // [0] Address
    // [1] Length
    // [2] Type
    // ...
    // [last] CRC

    if (frameIndex < 4)
    {
        return;
    }

    const size_t crcIndex = frameIndex - 1;

    const uint8_t receivedCrc =
        frameBuffer[crcIndex];

    // CRC covers Type + Payload.
    //
    // Type begins at index 2.
    const uint8_t calculatedCrc =
        crc8DvbS2(
            &frameBuffer[2],
            crcIndex - 2
        );

    if (receivedCrc != calculatedCrc)
    {
        return;
    }

    // Frame is structurally valid and passed CRC.
    //
    // RC channel decoding comes next.
}