#include "crsf_decoder.h"

namespace
{
    constexpr size_t CRSF_HEADER_SIZE = 2;

    // CRSF length byte includes:
    //
    //   Type
    //   Payload
    //   CRC
    //
    // It does NOT include:
    //
    //   Address
    //   Length
    //
    constexpr size_t CRSF_LENGTH_OFFSET = 1;
}

void CrsfDecoder::reset()
{
    frameIndex = 0;
    expectedFrameSize = 0;
    newChannels = false;
}

void CrsfDecoder::pushByte(uint8_t byte)
{
    // Prevent malformed data from overflowing the buffer.
    if (frameIndex >= MAX_FRAME_SIZE)
    {
        reset();
    }

    frameBuffer[frameIndex++] = byte;

    // Once we receive the length byte, we know how large
    // the complete frame should be.
    if (frameIndex == CRSF_HEADER_SIZE)
    {
        const uint8_t crsfLength =
            frameBuffer[CRSF_LENGTH_OFFSET];

        expectedFrameSize =
            static_cast<size_t>(crsfLength) + 2;

        if (expectedFrameSize > MAX_FRAME_SIZE)
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
    // Frame parsing will be implemented next.
}