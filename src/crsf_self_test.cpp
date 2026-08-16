#include "crsf_self_test.h"

#include <stddef.h>
#include <stdint.h>

#include "crsf_decoder.h"
#include "crsf_frame.h"
#include "crsf_parser.h"
#include "crsf_protocol.h"
#include "raw_channels.h"


namespace
{
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

            bitsInBuffer +=
                Crsf::RC_CHANNEL_BITS;


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
        uint8_t address,
        const uint16_t* channels,
        uint8_t* frame
    )
    {
        frame[0] =
            address;


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
            if (
                decoded.channel[i] !=
                expected[i]
            )
            {
                return false;
            }
        }

        return true;
    }


    bool feedParser(
        CrsfParser& parser,
        const uint8_t* frameBytes,
        size_t frameSize,
        CrsfFrame& parsedFrame
    )
    {
        bool frameReceived = false;

        for (
            size_t i = 0;
            i < frameSize;
            ++i
        )
        {
            if (
                parser.pushByte(
                    frameBytes[i],
                    parsedFrame
                )
            )
            {
                frameReceived = true;
            }
        }

        return frameReceived;
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
            Crsf::ADDRESS_FLIGHT_CONTROLLER,
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
            decoder.pushByte(
                frame[i]
            );
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
            Crsf::ADDRESS_FLIGHT_CONTROLLER,
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
            decoder.pushByte(
                frame[i]
            );
        }


        // A corrupted frame must never publish channels.

        return !decoder.hasNewChannels();
    }


    bool runValidSyncAddressTest()
    {
        constexpr uint16_t expectedChannels[16] =
        {
            172, 992, 1811, 500,
            1000, 1200, 1500, 700,
            0, 2047, 321, 1777,
            50, 1024, 1600, 888
        };


        // Representative valid CRSF frame-start values.
        //
        // These cover:
        //
        // - traditional serial sync / Flight Controller
        // - broadcast
        // - Remote Control
        // - Receiver
        // - Transmitter Module
        // - dynamic/NAT address space

        constexpr uint8_t validAddresses[] =
        {
            Crsf::ADDRESS_FLIGHT_CONTROLLER,
            Crsf::ADDRESS_BROADCAST,
            Crsf::ADDRESS_REMOTE_CONTROL,
            Crsf::ADDRESS_RECEIVER,
            Crsf::ADDRESS_TRANSMITTER,
            Crsf::ADDRESS_DYNAMIC_MIN
        };


        for (
            size_t addressIndex = 0;
            addressIndex <
                sizeof(validAddresses) /
                sizeof(validAddresses[0]);
            ++addressIndex
        )
        {
            uint8_t frameBytes[FRAME_SIZE] = {};


            buildFrame(
                validAddresses[addressIndex],
                expectedChannels,
                frameBytes
            );


            CrsfParser parser;
            CrsfFrame parsedFrame;


            if (
                !feedParser(
                    parser,
                    frameBytes,
                    FRAME_SIZE,
                    parsedFrame
                )
            )
            {
                return false;
            }


            if (
                parsedFrame.address !=
                validAddresses[addressIndex]
            )
            {
                return false;
            }


            if (
                parsedFrame.type !=
                Crsf::FRAME_RC_CHANNELS
            )
            {
                return false;
            }


            if (
                parsedFrame.payloadLength !=
                Crsf::RC_CHANNEL_PAYLOAD_SIZE
            )
            {
                return false;
            }
        }


        return true;
    }


    bool runInvalidSyncAddressTest()
    {
        constexpr uint16_t expectedChannels[16] =
        {
            172, 992, 1811, 500,
            1000, 1200, 1500, 700,
            0, 2047, 321, 1777,
            50, 1024, 1600, 888
        };


        // 0x01 is not currently assigned as a CRSF device address
        // and is outside the defined dynamic-address range.

        constexpr uint8_t invalidAddress =
            0x01;


        uint8_t frameBytes[FRAME_SIZE] = {};


        buildFrame(
            invalidAddress,
            expectedChannels,
            frameBytes
        );


        CrsfParser parser;
        CrsfFrame parsedFrame;


        return !feedParser(
            parser,
            frameBytes,
            FRAME_SIZE,
            parsedFrame
        );
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


    if (!runValidSyncAddressTest())
    {
        return false;
    }


    if (!runInvalidSyncAddressTest())
    {
        return false;
    }


    return true;
}