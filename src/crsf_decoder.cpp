#include "crsf_decoder.h"

#include "crsf_protocol.h"

namespace
{
    constexpr size_t CRSF_HEADER_SIZE = 2;
    constexpr size_t CRSF_LENGTH_OFFSET = 1;
    constexpr size_t CRSF_TYPE_OFFSET = 2;

    constexpr uint8_t CRSF_SYNC_BYTE = 0xC8;

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


void CrsfDecoder::reset()
{
    frameIndex = 0;
    expectedFrameSize = 0;

    newChannels = false;
    newLinkStatistics = false;
}


void CrsfDecoder::pushByte(uint8_t byte)
{
    // -------------------------------------------------------------------------
    // Synchronization
    //
    // Receiver -> host CRSF frames normally begin with the flight-controller
    // address/sync byte 0xC8.
    //
    // Ignore bytes until a plausible frame start is found. This allows the
    // decoder to recover if firmware begins listening in the middle of a frame.
    // -------------------------------------------------------------------------

    if (frameIndex == 0)
    {
        if (byte != CRSF_SYNC_BYTE)
        {
            return;
        }

        frameBuffer[frameIndex++] = byte;
        return;
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
            // If this byte itself is another sync byte, preserve it as the
            // beginning of the next candidate frame.
            frameIndex = 0;
            expectedFrameSize = 0;

            if (byte == CRSF_SYNC_BYTE)
            {
                frameBuffer[frameIndex++] = byte;
            }

            return;
        }

        frameBuffer[frameIndex++] = byte;

        expectedFrameSize =
            static_cast<size_t>(byte) +
            CRSF_HEADER_SIZE;

        return;
    }

    // -------------------------------------------------------------------------
    // Remaining frame bytes
    // -------------------------------------------------------------------------

    if (frameIndex >= MAX_FRAME_SIZE)
    {
        frameIndex = 0;
        expectedFrameSize = 0;
        return;
    }

    frameBuffer[frameIndex++] = byte;

    if (
        expectedFrameSize != 0 &&
        frameIndex == expectedFrameSize
    )
    {
        processFrame();

        // Always return to synchronization-search mode after processing a
        // complete candidate frame.
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


bool CrsfDecoder::hasNewLinkStatistics() const
{
    return newLinkStatistics;
}


const LinkStatistics&
CrsfDecoder::getLinkStatistics() const
{
    return linkStatistics;
}


void CrsfDecoder::clearNewLinkStatistics()
{
    newLinkStatistics = false;
}


void CrsfDecoder::processFrame()
{
    if (frameIndex < 4)
    {
        return;
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
        return;
    }

    // -------------------------------------------------------------------------
    // Construct a validated frame view
    // -------------------------------------------------------------------------

    CrsfFrame frame;

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

    dispatchFrame(frame);
}


void CrsfDecoder::dispatchFrame(
    const CrsfFrame& frame
)
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
        {
            if (
                linkStatisticsDecoder.decode(
                    frame,
                    linkStatistics
                )
            )
            {
                newLinkStatistics = true;
            }

            break;
        }

        default:
            // Unsupported frame types are intentionally ignored.
            break;
    }
}