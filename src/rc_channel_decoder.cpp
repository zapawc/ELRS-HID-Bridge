#include "rc_channel_decoder.h"

#include "crsf_protocol.h"

bool RcChannelDecoder::decode(
    const CrsfFrame& frame,
    RawChannels& channels
) const
{
    // This decoder only handles standard packed RC channel frames.
    if (frame.type != Crsf::FRAME_RC_CHANNELS)
    {
        return false;
    }

    // The standard 0x16 payload contains 16 * 11 bits = 22 bytes.
    //
    // CRSF permits frames to grow in future protocol revisions, so accept
    // payloads larger than the currently defined size and ignore trailing
    // bytes.
    if (
        frame.payload == nullptr ||
        frame.payloadLength < Crsf::RC_CHANNEL_PAYLOAD_SIZE
    )
    {
        return false;
    }

    // CRSF packs the sixteen 11-bit values consecutively into the payload.
    //
    // Rather than relying on compiler-specific C/C++ bitfields, use an
    // explicit bit accumulator. This keeps the implementation portable
    // between RP2040 and other architectures.

    uint32_t bitBuffer = 0;
    uint8_t bitsInBuffer = 0;
    uint8_t payloadIndex = 0;

    for (
        uint8_t channelIndex = 0;
        channelIndex < Crsf::RC_CHANNEL_COUNT;
        ++channelIndex
    )
    {
        // Ensure at least 11 bits are available.
        while (bitsInBuffer < Crsf::RC_CHANNEL_BITS)
        {
            bitBuffer |=
                static_cast<uint32_t>(
                    frame.payload[payloadIndex++]
                )
                << bitsInBuffer;

            bitsInBuffer += 8;
        }

        // Lowest 11 bits are the next channel.
        const uint16_t value =
            static_cast<uint16_t>(
                bitBuffer & Crsf::RC_CHANNEL_MASK
            );

        channels.set(
            static_cast<ChannelIndex>(channelIndex),
            value
        );

        // Consume those 11 bits.
        bitBuffer >>= Crsf::RC_CHANNEL_BITS;
        bitsInBuffer -= Crsf::RC_CHANNEL_BITS;
    }

    return true;
}