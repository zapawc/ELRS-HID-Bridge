#include "crsf_decoder.h"

#include "crsf_protocol.h"

namespace
{
    constexpr size_t CRSF_HEADER_SIZE = 2;
    constexpr size_t CRSF_LENGTH_OFFSET = 1;
    constexpr size_t CRSF_TYPE_OFFSET = 2;

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
                        (crc << 1) ^ Crsf::CRC_POLYNOMIAL
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
    // Protect against malformed input overflowing the frame buffer.
    if (frameIndex >= MAX_FRAME_SIZE)
    {
        reset();
    }

    frameBuffer[frameIndex++] = byte;

    // Once Address and Length have arrived, determine the complete
    // number of bytes expected for this frame.
    if (frameIndex == CRSF_HEADER_SIZE)
    {
        const uint8_t crsfLength =
            frameBuffer[CRSF_LENGTH_OFFSET];

        // CRSF Length includes:
        //
        //   Type
        //   Payload
        //   CRC
        //
        // It does not include:
        //
        //   Address
        //   Length
        //
        expectedFrameSize =
            static_cast<size_t>(crsfLength) + CRSF_HEADER_SIZE;

        // Minimum meaningful frame:
        //
        // Address
        // Length
        // Type
        // CRC
        if (
            expectedFrameSize < 4 ||
            expectedFrameSize > MAX_FRAME_SIZE
        )
        {
            reset();
            return;
        }
    }

    // A complete frame has been assembled.
    if (
        expectedFrameSize != 0 &&
        frameIndex == expectedFrameSize
    )
    {
        processFrame();

        // Prepare immediately for the next frame.
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
    // Layout:
    //
    // [0]      Address
    // [1]      Length
    // [2]      Type
    // [3..N-1] Payload
    // [N]      CRC

    if (frameIndex < 4)
    {
        return;
    }

    const size_t crcIndex = frameIndex - 1;

    const uint8_t receivedCrc =
        frameBuffer[crcIndex];

    // CRSF CRC covers Type + Payload.
    //
    // It does not cover Address, Length, or the CRC byte itself.
    const uint8_t calculatedCrc =
        crc8DvbS2(
            &frameBuffer[CRSF_TYPE_OFFSET],
            crcIndex - CRSF_TYPE_OFFSET
        );

    if (receivedCrc != calculatedCrc)
    {
        return;
    }

    // From this point onward the frame has passed CRC validation.
    CrsfFrame frame;

    frame.address = frameBuffer[0];
    frame.length = frameBuffer[1];
    frame.type = frameBuffer[CRSF_TYPE_OFFSET];

    // Length contains:
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
        frame.payload = &frameBuffer[3];
    }

    dispatchFrame(frame);
}


void CrsfDecoder::dispatchFrame(const CrsfFrame& frame)
{
    switch (frame.type)
    {
        case Crsf::FRAME_RC_CHANNELS:
        {
            if (
                rcChannelDecoder.decode(
                    frame,
                    channels
                )
            )
            {
                newChannels = true;
            }

            break;
        }

        case Crsf::FRAME_LINK_STATISTICS:
            // Reserved for future link-statistics support.
            break;

        default:
            // Unknown or currently unsupported frame types are
            // intentionally ignored.
            break;
    }
}