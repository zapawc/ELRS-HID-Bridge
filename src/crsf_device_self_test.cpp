#include "crsf_device_self_test.h"

#include <stddef.h>
#include <stdint.h>

#include "crsf_decoder.h"
#include "crsf_device.h"
#include "crsf_frame_encoder.h"
#include "crsf_protocol.h"


namespace
{
    void feedFrame(
        CrsfDecoder& decoder,
        const uint8_t* frame,
        size_t length
    )
    {
        for (
            size_t index = 0;
            index < length;
            ++index
        )
        {
            decoder.pushByte(
                frame[index]
            );
        }
    }


    uint8_t crc8DvbS2(
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


    bool runKnownPingVectorTest()
    {
        // Known valid CRSF Device Ping:
        //
        // EE 04 28 00 EA 54
        //
        // Frame address:
        //     CRSF Transmitter
        //
        // Destination:
        //     Broadcast
        //
        // Origin:
        //     Remote Control

        constexpr uint8_t frame[] =
        {
            0xEE,
            0x04,
            0x28,
            0x00,
            0xEA,
            0x54
        };


        CrsfDecoder decoder;


        feedFrame(
            decoder,
            frame,
            sizeof(frame)
        );


        if (!decoder.hasDevicePing())
        {
            return false;
        }


        const CrsfDevicePing& ping =
            decoder.getDevicePing();


        if (
            ping.frameAddress !=
            Crsf::ADDRESS_TRANSMITTER
        )
        {
            return false;
        }


        if (
            ping.destination !=
            Crsf::ADDRESS_BROADCAST
        )
        {
            return false;
        }


        if (
            ping.origin !=
            Crsf::ADDRESS_REMOTE_CONTROL
        )
        {
            return false;
        }


        decoder.clearDevicePing();


        if (decoder.hasDevicePing())
        {
            return false;
        }


        return true;
    }


    bool runExtraFieldCompatibilityTest()
    {
        // Build a Device Ping containing one additional trailing byte.
        //
        // The extended-header routing fields must still be recognized:
        //
        // Destination
        // Origin
        // Extra field

        CrsfFrameEncoder encoder;


        constexpr uint8_t extraPayload[] =
        {
            0xA5
        };


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};


        size_t frameLength = 0;


        if (
            !encoder.encodeExtended(
                Crsf::SYNC_BYTE,
                Crsf::FRAME_DEVICE_PING,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                Crsf::ADDRESS_REMOTE_CONTROL,
                extraPayload,
                sizeof(extraPayload),
                frame,
                sizeof(frame),
                frameLength
            )
        )
        {
            return false;
        }


        CrsfDecoder decoder;


        feedFrame(
            decoder,
            frame,
            frameLength
        );


        if (!decoder.hasDevicePing())
        {
            return false;
        }


        const CrsfDevicePing& ping =
            decoder.getDevicePing();


        if (
            ping.frameAddress !=
            Crsf::SYNC_BYTE
        )
        {
            return false;
        }


        if (
            ping.destination !=
            Crsf::ADDRESS_FLIGHT_CONTROLLER
        )
        {
            return false;
        }


        if (
            ping.origin !=
            Crsf::ADDRESS_REMOTE_CONTROL
        )
        {
            return false;
        }


        return true;
    }


    bool runMissingRoutingFieldsTest()
    {
        // Construct a structurally valid CRSF frame whose frame type
        // claims to be Device Ping but which does not contain the
        // Destination and Origin bytes required by an extended frame.
        //
        // CrsfParser should accept the basic frame and CRC.
        // CrsfDevice must reject it as an unusable Device Ping.

        uint8_t frame[] =
        {
            Crsf::SYNC_BYTE,

            // Length:
            //
            // Type + CRC
            0x02,

            Crsf::FRAME_DEVICE_PING,

            0x00
        };


        frame[3] =
            crc8DvbS2(
                &frame[2],
                1
            );


        CrsfDecoder decoder;


        feedFrame(
            decoder,
            frame,
            sizeof(frame)
        );


        if (decoder.hasDevicePing())
        {
            return false;
        }


        return true;
    }


    bool runNonPingFrameTest()
    {
        // Construct a valid extended-header Device Info frame.
        //
        // It is structurally valid CRSF traffic but must not be
        // surfaced as a Device Ping.

        uint8_t frame[] =
        {
            Crsf::SYNC_BYTE,

            // Length:
            //
            // Type
            // Destination
            // Origin
            // CRC
            0x04,

            Crsf::FRAME_DEVICE_INFO,

            Crsf::ADDRESS_REMOTE_CONTROL,

            Crsf::ADDRESS_FLIGHT_CONTROLLER,

            0x00
        };


        frame[5] =
            crc8DvbS2(
                &frame[2],
                3
            );


        CrsfDecoder decoder;


        feedFrame(
            decoder,
            frame,
            sizeof(frame)
        );


        if (decoder.hasDevicePing())
        {
            return false;
        }


        return true;
    }
}


bool CrsfDeviceSelfTest::run()
{
    if (!runKnownPingVectorTest())
    {
        return false;
    }


    if (!runExtraFieldCompatibilityTest())
    {
        return false;
    }


    if (!runMissingRoutingFieldsTest())
    {
        return false;
    }


    if (!runNonPingFrameTest())
    {
        return false;
    }


    return true;
}