#include "crsf_parameter_self_test.h"
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


    bool runParameterReadCaptureTest()
    {
        CrsfFrameEncoder encoder;

        constexpr uint8_t payload[] =
        {
            0x01,
            0x00
        };


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 0;


        if (
            !encoder.encodeExtended(
                Crsf::SYNC_BYTE,
                Crsf::FRAME_PARAMETER_READ,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                Crsf::ADDRESS_REMOTE_CONTROL,
                payload,
                sizeof(payload),
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


        if (!decoder.hasParameterRead())
        {
            return false;
        }


        const CrsfParameterRead& request =
            decoder.getParameterRead();


        if (
            request.destination !=
                Crsf::ADDRESS_FLIGHT_CONTROLLER ||
            request.origin !=
                Crsf::ADDRESS_REMOTE_CONTROL ||
            request.parameterNumber != 1 ||
            request.chunkNumber != 0
        )
        {
            return false;
        }


        decoder.clearParameterRead();


        return
            !decoder.hasParameterRead();
    }


    bool runRootFolderResponseTest()
    {
        CrsfParameterRead request;

        request.destination =
            Crsf::ADDRESS_FLIGHT_CONTROLLER;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber = 0;
        request.chunkNumber = 0;


        constexpr uint8_t children[] =
        {
            1
        };


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 0;


        CrsfDevice device;


        if (
            !device.buildFolderParameterResponse(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                0,
                0,
                "ROOT",
                children,
                sizeof(children),
                frame,
                sizeof(frame),
                frameLength
            )
        )
        {
            return false;
        }


        if (
            frameLength < 13 ||
            frame[2] !=
                Crsf::FRAME_PARAMETER_SETTINGS_ENTRY ||
            frame[3] !=
                Crsf::ADDRESS_REMOTE_CONTROL ||
            frame[4] !=
                Crsf::ADDRESS_FLIGHT_CONTROLLER ||
            frame[5] != 0 ||
            frame[6] != 0 ||
            frame[7] != 0 ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_FOLDER
        )
        {
            return false;
        }


        constexpr uint8_t expectedTail[] =
        {
            'R', 'O', 'O', 'T', 0,
            1,
            0xFF
        };


        for (
            size_t index = 0;
            index < sizeof(expectedTail);
            ++index
        )
        {
            if (
                frame[9 + index] !=
                expectedTail[index]
            )
            {
                return false;
            }
        }


        return true;
    }


    bool runBrightnessEntryResponseTest()
    {
        CrsfParameterRead request;

        request.destination =
            Crsf::ADDRESS_FLIGHT_CONTROLLER;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber = 1;
        request.chunkNumber = 0;


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 0;


        CrsfDevice device;


        if (
            !device.buildFloatParameterResponse(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                1,
                0,
                "LED Brightness",
                10,
                0,
                100,
                10,
                0,
                1,
                "",
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
            frame[2] !=
                Crsf::FRAME_PARAMETER_SETTINGS_ENTRY ||
            frame[3] !=
                Crsf::ADDRESS_REMOTE_CONTROL ||
            frame[4] !=
                Crsf::ADDRESS_FLIGHT_CONTROLLER ||
            frame[5] != 1 ||
            frame[6] != 0 ||
            frame[7] != 0 ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_FLOAT
        )
        {
            return false;
        }


        constexpr char expectedName[] =
            "LED Brightness";

        constexpr size_t expectedNameLength =
            sizeof(expectedName);


        for (
            size_t index = 0;
            index < expectedNameLength;
            ++index
        )
        {
            if (
                frame[9 + index] !=
                static_cast<uint8_t>(
                    expectedName[index]
                )
            )
            {
                return false;
            }
        }


        const size_t valueOffset =
            9 +
            expectedNameLength;


        constexpr uint8_t expectedNumeric[] =
        {
            // Value = 10
            0x00, 0x00, 0x00, 0x0A,

            // Min = 0
            0x00, 0x00, 0x00, 0x00,

            // Max = 100
            0x00, 0x00, 0x00, 0x64,

            // Default = 10
            0x00, 0x00, 0x00, 0x0A,

            // Decimal point = 0
            0x00,

            // Step = 1
            0x00, 0x00, 0x00, 0x01,

            // Unit intentionally blank for ExpressLRS Lua r18 compatibility.
            0x00
        };


        for (
            size_t index = 0;
            index < sizeof(expectedNumeric);
            ++index
        )
        {
            if (
                frame[
                    valueOffset + index
                ] !=
                expectedNumeric[index]
            )
            {
                return false;
            }
        }


        return true;
    }


    bool runFloatWriteCaptureAndAckTest()
    {
        CrsfFrameEncoder encoder;

        constexpr uint8_t writePayload[] =
        {
            // Parameter 1
            0x01,

            // Value = 75
            0x00, 0x00, 0x00, 0x4B
        };


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 0;


        if (
            !encoder.encodeExtended(
                Crsf::SYNC_BYTE,
                Crsf::FRAME_PARAMETER_WRITE,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                Crsf::ADDRESS_REMOTE_CONTROL,
                writePayload,
                sizeof(writePayload),
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


        if (!decoder.hasParameterWrite())
        {
            return false;
        }


        const CrsfParameterWrite request =
            decoder.getParameterWrite();


        int32_t value = -1;


        if (
            request.parameterNumber != 1 ||
            !CrsfDevice::readInt32BigEndian(
                request.data,
                request.dataLength,
                value
            ) ||
            value != 75
        )
        {
            return false;
        }


        uint8_t ack[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t ackLength = 0;


        CrsfDevice device;


        if (
            !device.buildFloatWriteResponse(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                1,
                value,
                ack,
                sizeof(ack),
                ackLength
            )
        )
        {
            return false;
        }


        constexpr uint8_t expectedPayload[] =
        {
            0x01,
            0x00, 0x00, 0x00, 0x4B
        };


        if (
            ackLength != 11 ||
            ack[2] !=
                Crsf::FRAME_PARAMETER_WRITE ||
            ack[3] !=
                Crsf::ADDRESS_REMOTE_CONTROL ||
            ack[4] !=
                Crsf::ADDRESS_FLIGHT_CONTROLLER
        )
        {
            return false;
        }


        for (
            size_t index = 0;
            index < sizeof(expectedPayload);
            ++index
        )
        {
            if (
                ack[5 + index] !=
                expectedPayload[index]
            )
            {
                return false;
            }
        }


        return true;
    }


    bool runWrongAddressRejectedTest()
    {
        CrsfParameterRead request;

        request.destination =
            Crsf::ADDRESS_USB;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber = 1;


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 123;


        CrsfDevice device;


        if (
            device.buildFloatParameterResponse(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                1,
                0,
                "LED Brightness",
                10,
                0,
                100,
                10,
                0,
                1,
                "",
                frame,
                sizeof(frame),
                frameLength
            )
        )
        {
            return false;
        }


        return
            frameLength == 0;
    }
}


bool CrsfParameterSelfTest::run()
{
    return
        runParameterReadCaptureTest() &&
        runRootFolderResponseTest() &&
        runBrightnessEntryResponseTest() &&
        runFloatWriteCaptureAndAckTest() &&
        runWrongAddressRejectedTest();
}
