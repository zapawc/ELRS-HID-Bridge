#include "crsf_parameter_self_test.h"

#include <stddef.h>
#include <stdint.h>

#include "bridge_configuration.h"
#include "bridge_parameters.h"
#include "crsf_decoder.h"
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
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER,
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
            request.parameterNumber !=
                BridgeParameters::
                    LED_BRIGHTNESS_PARAMETER ||
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
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        CrsfParameterRead request;

        request.destination =
            Crsf::ADDRESS_FLIGHT_CONTROLLER;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber =
            BridgeParameters::
                ROOT_PARAMETER;

        request.chunkNumber = 0;


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 0;


        if (
            !parameters.buildReadResponse(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
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
            frame[5] !=
                BridgeParameters::
                    ROOT_PARAMETER ||
            frame[6] != 0 ||
            frame[7] !=
                BridgeParameters::
                    ROOT_PARAMETER ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_FOLDER
        )
        {
            return false;
        }


        constexpr uint8_t expectedTail[] =
        {
            'R', 'O', 'O', 'T', 0,
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER,
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
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        configuration.ledBrightnessPercent =
            37;


        BridgeParameters parameters(
            configuration
        );


        CrsfParameterRead request;

        request.destination =
            Crsf::ADDRESS_FLIGHT_CONTROLLER;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber =
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER;

        request.chunkNumber = 0;


        uint8_t frame[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t frameLength = 0;


        if (
            !parameters.buildReadResponse(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
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
            frame[5] !=
                BridgeParameters::
                    LED_BRIGHTNESS_PARAMETER ||
            frame[6] != 0 ||
            frame[7] !=
                BridgeParameters::
                    ROOT_PARAMETER ||
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
            // Current value = 37
            0x00, 0x00, 0x00, 0x25,

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


    bool runFloatWriteCaptureTest(
        CrsfParameterWrite& request
    )
    {
        CrsfFrameEncoder encoder;

        constexpr uint8_t writePayload[] =
        {
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER,

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


        request =
            decoder.getParameterWrite();


        decoder.clearParameterWrite();


        return
            request.destination ==
                Crsf::ADDRESS_FLIGHT_CONTROLLER &&
            request.origin ==
                Crsf::ADDRESS_REMOTE_CONTROL &&
            request.parameterNumber ==
                BridgeParameters::
                    LED_BRIGHTNESS_PARAMETER &&
            request.dataLength == 4;
    }


    bool runValidBrightnessWriteTest()
    {
        CrsfParameterWrite request;


        if (
            !runFloatWriteCaptureTest(
                request
            )
        )
        {
            return false;
        }


        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        uint8_t response[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t responseLength = 0;

        BridgeParameterWriteResult result;


        if (
            !parameters.handleWrite(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                response,
                sizeof(response),
                responseLength,
                result
            )
        )
        {
            return false;
        }


        if (
            configuration
                .ledBrightnessPercent != 75 ||
            result.change !=
                BridgeParameterChange::
                    LedBrightness ||
            result.ledBrightnessPercent != 75
        )
        {
            return false;
        }


        constexpr uint8_t expectedPayload[] =
        {
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER,
            0x00, 0x00, 0x00, 0x4B
        };


        if (
            responseLength != 11 ||
            response[2] !=
                Crsf::FRAME_PARAMETER_WRITE ||
            response[3] !=
                Crsf::ADDRESS_REMOTE_CONTROL ||
            response[4] !=
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
                response[5 + index] !=
                expectedPayload[index]
            )
            {
                return false;
            }
        }


        return true;
    }


    bool runOutOfRangeWriteRejectedTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        CrsfParameterWrite request;

        request.destination =
            Crsf::ADDRESS_FLIGHT_CONTROLLER;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber =
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER;

        request.dataLength = 4;

        // Value = 101
        request.data[0] = 0x00;
        request.data[1] = 0x00;
        request.data[2] = 0x00;
        request.data[3] = 0x65;


        uint8_t response[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t responseLength = 123;

        BridgeParameterWriteResult result;

        result.change =
            BridgeParameterChange::
                LedBrightness;

        result.ledBrightnessPercent =
            99;


        if (
            parameters.handleWrite(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                response,
                sizeof(response),
                responseLength,
                result
            )
        )
        {
            return false;
        }


        return
            configuration
                .ledBrightnessPercent == 10 &&
            responseLength == 0 &&
            result.change ==
                BridgeParameterChange::None;
    }


    bool runWrongAddressRejectedTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        CrsfParameterRead request;

        request.destination =
            Crsf::ADDRESS_USB;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber =
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER;


        uint8_t response[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t responseLength = 123;


        if (
            parameters.buildReadResponse(
                request,
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                response,
                sizeof(response),
                responseLength
            )
        )
        {
            return false;
        }


        return
            responseLength == 0;
    }
}


bool CrsfParameterSelfTest::run()
{
    return
        BridgeParameters::PARAMETER_COUNT == 1 &&
        runParameterReadCaptureTest() &&
        runRootFolderResponseTest() &&
        runBrightnessEntryResponseTest() &&
        runValidBrightnessWriteTest() &&
        runOutOfRangeWriteRejectedTest() &&
        runWrongAddressRejectedTest();
}
