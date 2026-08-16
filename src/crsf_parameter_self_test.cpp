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
                PITCH_INVERSION_PARAMETER,
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
                    PITCH_INVERSION_PARAMETER ||
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
            frameLength < 14 ||
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
            BridgeParameters::
                PITCH_INVERSION_PARAMETER,
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
            frame[5] !=
                BridgeParameters::
                    LED_BRIGHTNESS_PARAMETER ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_FLOAT
        )
        {
            return false;
        }


        return true;
    }


    bool runPitchSelectionEntryResponseTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        // Validated baseline is inverted.
        if (!configuration.pitch.inverted)
        {
            return false;
        }


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
                PITCH_INVERSION_PARAMETER;

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
                    PITCH_INVERSION_PARAMETER ||
            frame[6] != 0 ||
            frame[7] !=
                BridgeParameters::
                    ROOT_PARAMETER ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_TEXT_SELECTION
        )
        {
            return false;
        }


        constexpr uint8_t expectedBody[] =
        {
            'P','i','t','c','h',' ',
            'I','n','v','e','r','s','i','o','n',0,

            'N','o','r','m','a','l',';',
            'I','n','v','e','r','t','e','d',0,

            // Current = Inverted
            0x01,

            // Min = Normal
            0x00,

            // Max = Inverted
            0x01,

            // Default = Inverted
            0x01,

            // Unit = ""
            0x00
        };


        for (
            size_t index = 0;
            index < sizeof(expectedBody);
            ++index
        )
        {
            if (
                frame[9 + index] !=
                expectedBody[index]
            )
            {
                return false;
            }
        }


        return true;
    }


    bool runValidBrightnessWriteTest()
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

        // Value = 75
        request.data[0] = 0x00;
        request.data[1] = 0x00;
        request.data[2] = 0x00;
        request.data[3] = 0x4B;


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


        return
            configuration
                .ledBrightnessPercent == 75 &&
            result.change ==
                BridgeParameterChange::
                    LedBrightness &&
            result.ledBrightnessPercent == 75 &&
            responseLength == 11 &&
            response[2] ==
                Crsf::FRAME_PARAMETER_WRITE &&
            response[5] ==
                BridgeParameters::
                    LED_BRIGHTNESS_PARAMETER;
    }


    bool runPitchWriteNormalTest()
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
                PITCH_INVERSION_PARAMETER;

        request.dataLength = 1;

        // Normal.
        request.data[0] = 0;


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


        return
            !configuration.pitch.inverted &&
            result.change ==
                BridgeParameterChange::
                    PitchInversion &&
            !result.pitchInverted &&
            responseLength == 8 &&
            response[2] ==
                Crsf::FRAME_PARAMETER_WRITE &&
            response[3] ==
                Crsf::ADDRESS_REMOTE_CONTROL &&
            response[4] ==
                Crsf::ADDRESS_FLIGHT_CONTROLLER &&
            response[5] ==
                BridgeParameters::
                    PITCH_INVERSION_PARAMETER &&
            response[6] == 0;
    }


    bool runPitchWriteInvertedTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        configuration.pitch.inverted =
            false;

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
                PITCH_INVERSION_PARAMETER;

        request.dataLength = 1;

        // Inverted.
        request.data[0] = 1;


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


        return
            configuration.pitch.inverted &&
            result.change ==
                BridgeParameterChange::
                    PitchInversion &&
            result.pitchInverted &&
            responseLength == 8 &&
            response[6] == 1;
    }


    bool runInvalidPitchSelectionRejectedTest()
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
                PITCH_INVERSION_PARAMETER;

        request.dataLength = 1;

        // Only 0 and 1 are valid.
        request.data[0] = 2;


        uint8_t response[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t responseLength = 123;

        BridgeParameterWriteResult result;


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
            configuration.pitch.inverted &&
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
                PITCH_INVERSION_PARAMETER;


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
        BridgeParameters::PARAMETER_COUNT == 2 &&
        runParameterReadCaptureTest() &&
        runRootFolderResponseTest() &&
        runBrightnessEntryResponseTest() &&
        runPitchSelectionEntryResponseTest() &&
        runValidBrightnessWriteTest() &&
        runPitchWriteNormalTest() &&
        runPitchWriteInvertedTest() &&
        runInvalidPitchSelectionRejectedTest() &&
        runWrongAddressRejectedTest();
}
