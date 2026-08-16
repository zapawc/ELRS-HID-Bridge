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


    bool runBroadcastDeviceInfoResponseTest()
    {
        // A broadcast ping should produce a Device Info response back
        // to the ping origin.
        //
        // Identity values are test vectors only. Production identity
        // remains a caller-supplied policy decision at this checkpoint.

        CrsfDevicePing ping;

        ping.frameAddress =
            Crsf::ADDRESS_TRANSMITTER;

        ping.destination =
            Crsf::ADDRESS_BROADCAST;

        ping.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;


        CrsfDeviceIdentity identity;

        identity.name =
            "ELRS-HID-Bridge";

        identity.serialNumber =
            0x01020304;

        identity.hardwareId =
            0x11223344;

        identity.firmwareId =
            0x55667788;

        identity.parameterCount = 0;
        identity.parameterVersion = 1;


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 0;


        CrsfDevice device;


        if (
            !device.buildDeviceInfoResponse(
                ping,
                Crsf::ADDRESS_USB,
                identity,
                frame,
                sizeof(frame),
                frameLength
            )
        )
        {
            return false;
        }


        constexpr char expectedName[] =
            "ELRS-HID-Bridge";

        constexpr size_t expectedNameLength =
            sizeof(expectedName) - 1;

        constexpr size_t fixedFieldsLength = 14;

        constexpr size_t expectedPayloadLength =
            expectedNameLength +
            1 +
            fixedFieldsLength;

        constexpr size_t expectedFrameLength =
            expectedPayloadLength +
            6;


        if (
            frameLength !=
            expectedFrameLength
        )
        {
            return false;
        }


        if (
            frame[0] !=
                Crsf::SYNC_BYTE ||
            frame[1] !=
                static_cast<uint8_t>(
                    expectedPayloadLength + 4
                ) ||
            frame[2] !=
                Crsf::FRAME_DEVICE_INFO ||
            frame[3] !=
                Crsf::ADDRESS_REMOTE_CONTROL ||
            frame[4] !=
                Crsf::ADDRESS_USB
        )
        {
            return false;
        }


        for (
            size_t index = 0;
            index < expectedNameLength;
            ++index
        )
        {
            if (
                frame[5 + index] !=
                static_cast<uint8_t>(
                    expectedName[index]
                )
            )
            {
                return false;
            }
        }


        // Device_name must be null-terminated before the numeric fields.
        if (
            frame[
                5 +
                expectedNameLength
            ] != 0
        )
        {
            return false;
        }


        const size_t identityOffset =
            5 +
            expectedNameLength +
            1;


        constexpr uint8_t expectedIdentityBytes[] =
        {
            0x01, 0x02, 0x03, 0x04,
            0x11, 0x22, 0x33, 0x44,
            0x55, 0x66, 0x77, 0x88,
            0x00,
            0x01
        };


        for (
            size_t index = 0;
            index < sizeof(expectedIdentityBytes);
            ++index
        )
        {
            if (
                frame[
                    identityOffset + index
                ] !=
                expectedIdentityBytes[index]
            )
            {
                return false;
            }
        }


        const size_t crcIndex =
            frameLength - 1;


        const uint8_t expectedCrc =
            crc8DvbS2(
                &frame[2],
                frameLength - 3
            );


        if (
            frame[crcIndex] !=
            expectedCrc
        )
        {
            return false;
        }


        return true;
    }


    bool runAddressedDeviceInfoResponseTest()
    {
        CrsfDevicePing ping;

        ping.frameAddress =
            Crsf::SYNC_BYTE;

        ping.destination =
            Crsf::ADDRESS_USB;

        ping.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;


        CrsfDeviceIdentity identity;

        identity.name =
            "Bridge";


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 0;


        CrsfDevice device;


        if (
            !device.buildDeviceInfoResponse(
                ping,
                Crsf::ADDRESS_USB,
                identity,
                frame,
                sizeof(frame),
                frameLength
            )
        )
        {
            return false;
        }


        if (
            frameLength == 0 ||
            frame[3] !=
                Crsf::ADDRESS_REMOTE_CONTROL ||
            frame[4] !=
                Crsf::ADDRESS_USB
        )
        {
            return false;
        }


        return true;
    }


    bool runUnrelatedDestinationTest()
    {
        CrsfDevicePing ping;

        ping.destination =
            Crsf::ADDRESS_FLIGHT_CONTROLLER;

        ping.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;


        CrsfDeviceIdentity identity;

        identity.name =
            "ELRS-HID-Bridge";


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 123;


        CrsfDevice device;


        if (
            device.buildDeviceInfoResponse(
                ping,
                Crsf::ADDRESS_USB,
                identity,
                frame,
                sizeof(frame),
                frameLength
            )
        )
        {
            return false;
        }


        if (frameLength != 0)
        {
            return false;
        }


        return true;
    }


    bool runInvalidIdentityTest()
    {
        CrsfDevicePing ping;

        ping.destination =
            Crsf::ADDRESS_BROADCAST;

        ping.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;


        CrsfDeviceIdentity identity;

        identity.name = nullptr;


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 123;


        CrsfDevice device;


        if (
            device.buildDeviceInfoResponse(
                ping,
                Crsf::ADDRESS_USB,
                identity,
                frame,
                sizeof(frame),
                frameLength
            )
        )
        {
            return false;
        }


        if (frameLength != 0)
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


    if (!runBroadcastDeviceInfoResponseTest())
    {
        return false;
    }


    if (!runAddressedDeviceInfoResponseTest())
    {
        return false;
    }


    if (!runUnrelatedDestinationTest())
    {
        return false;
    }


    if (!runInvalidIdentityTest())
    {
        return false;
    }


    return true;
}
