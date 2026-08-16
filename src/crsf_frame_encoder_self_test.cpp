#include "crsf_frame_encoder_self_test.h"

#include <stddef.h>
#include <stdint.h>

#include "crsf_frame_encoder.h"
#include "crsf_protocol.h"


namespace
{
    bool buffersMatch(
        const uint8_t* actual,
        const uint8_t* expected,
        size_t length
    )
    {
        for (
            size_t index = 0;
            index < length;
            ++index
        )
        {
            if (
                actual[index] !=
                expected[index]
            )
            {
                return false;
            }
        }


        return true;
    }


    bool runKnownDevicePingVectorTest()
    {
        CrsfFrameEncoder encoder;


        uint8_t encoded[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};


        size_t encodedLength = 0;


        // Known CRSF Device Ping:
        //
        // Sync        0xEE
        // Length      0x04
        // Type        0x28
        // Destination 0x00
        // Origin      0xEA
        // CRC         0x54

        constexpr uint8_t expected[] =
        {
            0xEE,
            0x04,
            0x28,
            0x00,
            0xEA,
            0x54
        };


        if (
            !encoder.encodeExtended(
                Crsf::ADDRESS_TRANSMITTER,
                Crsf::FRAME_DEVICE_PING,
                Crsf::ADDRESS_BROADCAST,
                Crsf::ADDRESS_REMOTE_CONTROL,
                nullptr,
                0,
                encoded,
                sizeof(encoded),
                encodedLength
            )
        )
        {
            return false;
        }


        if (
            encodedLength !=
            sizeof(expected)
        )
        {
            return false;
        }


        return buffersMatch(
            encoded,
            expected,
            sizeof(expected)
        );
    }


    bool runPayloadFrameTest()
    {
        CrsfFrameEncoder encoder;


        constexpr uint8_t payload[] =
        {
            0x11,
            0x22,
            0x33,
            0x44
        };


        uint8_t encoded[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};


        size_t encodedLength = 0;


        if (
            !encoder.encodeExtended(
                Crsf::SYNC_BYTE,
                Crsf::FRAME_DEVICE_INFO,
                Crsf::ADDRESS_REMOTE_CONTROL,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                payload,
                sizeof(payload),
                encoded,
                sizeof(encoded),
                encodedLength
            )
        )
        {
            return false;
        }


        // Payload length 4 gives:
        //
        // Length byte:
        //   Type + Destination + Origin + Payload + CRC
        //   1 + 1 + 1 + 4 + 1 = 8
        //
        // Total frame:
        //   Sync + Length + remaining 8 bytes = 10

        if (encodedLength != 10)
        {
            return false;
        }


        if (
            encoded[0] !=
            Crsf::SYNC_BYTE
        )
        {
            return false;
        }


        if (encoded[1] != 8)
        {
            return false;
        }


        if (
            encoded[2] !=
            Crsf::FRAME_DEVICE_INFO
        )
        {
            return false;
        }


        if (
            encoded[3] !=
            Crsf::ADDRESS_REMOTE_CONTROL
        )
        {
            return false;
        }


        if (
            encoded[4] !=
            Crsf::ADDRESS_FLIGHT_CONTROLLER
        )
        {
            return false;
        }


        for (
            size_t index = 0;
            index < sizeof(payload);
            ++index
        )
        {
            if (
                encoded[5 + index] !=
                payload[index]
            )
            {
                return false;
            }
        }


        return true;
    }


    bool runInsufficientBufferTest()
    {
        CrsfFrameEncoder encoder;


        constexpr uint8_t payload[] =
        {
            0x01,
            0x02,
            0x03
        };


        uint8_t encoded[4] = {};


        size_t encodedLength =
            1234;


        const bool result =
            encoder.encodeExtended(
                Crsf::SYNC_BYTE,
                Crsf::FRAME_DEVICE_INFO,
                Crsf::ADDRESS_REMOTE_CONTROL,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                payload,
                sizeof(payload),
                encoded,
                sizeof(encoded),
                encodedLength
            );


        if (result)
        {
            return false;
        }


        // Failure must leave outputLength in a deterministic state.

        return encodedLength == 0;
    }


    bool runNullPayloadTest()
    {
        CrsfFrameEncoder encoder;


        uint8_t encoded[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};


        size_t encodedLength = 0;


        // A non-zero payload length with a null payload pointer
        // must be rejected.

        return !encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_DEVICE_INFO,
            Crsf::ADDRESS_REMOTE_CONTROL,
            Crsf::ADDRESS_FLIGHT_CONTROLLER,
            nullptr,
            1,
            encoded,
            sizeof(encoded),
            encodedLength
        );
    }


    bool runMaximumPayloadTest()
    {
        CrsfFrameEncoder encoder;


        // Extended CRSF framing consumes four bytes from the CRSF
        // Length field:
        //
        // Type + Destination + Origin + CRC
        //
        // Maximum Length = 62
        //
        // Maximum extended payload = 58 bytes.

        constexpr size_t MAX_EXTENDED_PAYLOAD =
            Crsf::MAX_FRAME_LENGTH - 4;


        uint8_t payload[
            MAX_EXTENDED_PAYLOAD
        ] = {};


        for (
            size_t index = 0;
            index < sizeof(payload);
            ++index
        )
        {
            payload[index] =
                static_cast<uint8_t>(
                    index
                );
        }


        uint8_t encoded[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};


        size_t encodedLength = 0;


        if (
            !encoder.encodeExtended(
                Crsf::SYNC_BYTE,
                Crsf::FRAME_DEVICE_INFO,
                Crsf::ADDRESS_REMOTE_CONTROL,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                payload,
                sizeof(payload),
                encoded,
                sizeof(encoded),
                encodedLength
            )
        )
        {
            return false;
        }


        return
            encodedLength ==
            CrsfFrameEncoder::MAX_FRAME_SIZE;
    }


    bool runOversizedPayloadTest()
    {
        CrsfFrameEncoder encoder;


        constexpr size_t OVERSIZED_PAYLOAD =
            Crsf::MAX_FRAME_LENGTH - 3;


        uint8_t payload[
            OVERSIZED_PAYLOAD
        ] = {};


        uint8_t encoded[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};


        size_t encodedLength = 0;


        return !encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_DEVICE_INFO,
            Crsf::ADDRESS_REMOTE_CONTROL,
            Crsf::ADDRESS_FLIGHT_CONTROLLER,
            payload,
            sizeof(payload),
            encoded,
            sizeof(encoded),
            encodedLength
        );
    }
}


bool CrsfFrameEncoderSelfTest::run()
{
    if (!runKnownDevicePingVectorTest())
    {
        return false;
    }


    if (!runPayloadFrameTest())
    {
        return false;
    }


    if (!runInsufficientBufferTest())
    {
        return false;
    }


    if (!runNullPayloadTest())
    {
        return false;
    }


    if (!runMaximumPayloadTest())
    {
        return false;
    }


    if (!runOversizedPayloadTest())
    {
        return false;
    }


    return true;
}