#include "crsf_self_test.h"

#include <stddef.h>
#include <stdint.h>

#include "crsf_decoder.h"
#include "crsf_protocol.h"
#include "raw_channels.h"

namespace
{
    constexpr uint8_t FRAME_ADDRESS =
        Crsf::ADDRESS_FLIGHT_CONTROLLER;

    constexpr size_t FRAME_SIZE =
        2 +                         // Address + Length
        1 +                         // Type
        Crsf::RC_CHANNEL_PAYLOAD_SIZE +
        1;                          // CRC

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


    void packChannels(
        const uint16_t* channels,
        uint8_t* payload
    )
    {
        for (
            uint8_t i = 0;
            i < Crsf::RC_CHANNEL_PAYLOAD_SIZE;
            ++i
        )
        {
            payload[i] = 0;
        }

        uint32_t bitBuffer = 0;
        uint8_t bitsInBuffer = 0;
        uint8_t payloadIndex = 0;

        for (
            uint8_t channel = 0;
            channel < Crsf::RC_CHANNEL_COUNT;
            ++channel
        )
        {
            bitBuffer |=
                static_cast<uint32_t>(
                    channels[channel] &
                    Crsf::RC_CHANNEL_MASK
                )
                << bitsInBuffer;

            bitsInBuffer += Crsf::RC_CHANNEL_BITS;

            while (bitsInBuffer >= 8)
            {
                payload[payloadIndex++] =
                    static_cast<uint8_t>(
                        bitBuffer & 0xFF
                    );

                bitBuffer >>= 8;
                bitsInBuffer -= 8;
            }
        }

        if (
            bitsInBuffer > 0 &&
            payloadIndex <
                Crsf::RC_CHANNEL_PAYLOAD_SIZE
        )
        {
            payload[payloadIndex] =
                static_cast<uint8_t>(
                    bitBuffer & 0xFF
                );
        }
    }


    void buildFrame(
        const uint16_t* channels,
        uint8_t* frame
    )
    {
        frame[0] = FRAME_ADDRESS;

        // Length includes:
        //
        // Type + Payload + CRC
        frame[1] =
            1 +
            Crsf::RC_CHANNEL_PAYLOAD_SIZE +
            1;

        frame[2] =
            Crsf::FRAME_RC_CHANNELS;

        packChannels(
            channels,
            &frame[3]
        );

        const size_t crcIndex =
            FRAME_SIZE - 1;

        // CRC covers Type + Payload.
        frame[crcIndex] =
            crc8DvbS2(
                &frame[2],
                crcIndex - 2
            );
    }


    bool channelsMatch(
        const RawChannels& decoded,
        const uint16_t* expected
    )
    {
        for (
            uint8_t i = 0;
            i < RawChannels::CHANNEL_COUNT;
            ++i
        )
        {
            if (decoded.channel[i] != expected[i])
            {
                return false;
            }
        }

        return true;
    }


    bool runValidFrameTest()
    {
        // Deliberately varied values.
        //
        // Includes:
        // - CRSF low/mid/high values
        // - zero
        // - maximum 11-bit value
        // - several arbitrary values
        //
        // This helps expose packing errors that a frame containing
        // sixteen identical values could hide.

        constexpr uint16_t expectedChannels[16] =
        {
            172,
            992,
            1811,
            500,

            1000,
            1200,
            1500,
            700,

            0,
            2047,
            321,
            1777,

            50,
            1024,
            1600,
            888
        };

        uint8_t frame[FRAME_SIZE] = {};

        buildFrame(
            expectedChannels,
            frame
        );

        CrsfDecoder decoder;

        for (
            size_t i = 0;
            i < FRAME_SIZE;
            ++i
        )
        {
            decoder.pushByte(frame[i]);
        }

        if (!decoder.hasNewChannels())
        {
            return false;
        }

        return channelsMatch(
            decoder.getChannels(),
            expectedChannels
        );
    }


    bool runBadCrcTest()
    {
        constexpr uint16_t expectedChannels[16] =
        {
            172, 992, 1811, 500,
            1000, 1200, 1500, 700,
            0, 2047, 321, 1777,
            50, 1024, 1600, 888
        };

        uint8_t frame[FRAME_SIZE] = {};

        buildFrame(
            expectedChannels,
            frame
        );

        // Deliberately corrupt the CRC.
        frame[FRAME_SIZE - 1] ^= 0xFF;

        CrsfDecoder decoder;

        for (
            size_t i = 0;
            i < FRAME_SIZE;
            ++i
        )
        {
            decoder.pushByte(frame[i]);
        }

        // A corrupted frame must never publish channels.
        return !decoder.hasNewChannels();
    }
}


bool CrsfSelfTest::run()
{
    if (!runValidFrameTest())
    {
        return false;
    }

    if (!runBadCrcTest())
    {
        return false;
    }

    return true;
}