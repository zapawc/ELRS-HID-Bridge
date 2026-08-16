#include "crsf_parameter_self_test.h"

#include <stddef.h>
#include <stdint.h>

#include "bridge_configuration.h"
#include "bridge_parameters.h"
#include "crsf_protocol.h"


namespace
{
    constexpr uint8_t COMMAND_READY = 0;
    constexpr uint8_t COMMAND_START = 1;
    constexpr uint8_t COMMAND_CONFIRMATION_NEEDED = 3;
    constexpr uint8_t COMMAND_CONFIRM = 4;
    constexpr uint8_t COMMAND_CANCEL = 5;
    constexpr uint8_t COMMAND_POLL = 6;


    CrsfParameterRead makeRead(
        uint8_t parameterNumber
    )
    {
        CrsfParameterRead request;

        request.destination =
            Crsf::ADDRESS_FLIGHT_CONTROLLER;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber =
            parameterNumber;

        request.chunkNumber = 0;

        return request;
    }


    CrsfParameterWrite makeWrite(
        uint8_t parameterNumber,
        uint8_t value
    )
    {
        CrsfParameterWrite request;

        request.destination =
            Crsf::ADDRESS_FLIGHT_CONTROLLER;

        request.origin =
            Crsf::ADDRESS_REMOTE_CONTROL;

        request.parameterNumber =
            parameterNumber;

        request.dataLength = 1;
        request.data[0] = value;

        return request;
    }


    bool findCommandStatus(
        const uint8_t* frame,
        size_t frameLength,
        uint8_t& status
    )
    {
        if (
            frame == nullptr ||
            frameLength < 12 ||
            frame[2] !=
                Crsf::FRAME_PARAMETER_SETTINGS_ENTRY ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_COMMAND
        )
        {
            return false;
        }


        // Command name begins at frame[9] and is null terminated.
        size_t index = 9;


        while (
            index < frameLength &&
            frame[index] != 0
        )
        {
            ++index;
        }


        if (
            index + 2 >=
            frameLength
        )
        {
            return false;
        }


        status =
            frame[index + 1];


        return true;
    }


    bool runRootFolderResponseTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        const CrsfParameterRead request =
            makeRead(
                BridgeParameters::
                    ROOT_PARAMETER
            );


        uint8_t frame[
            64
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


        constexpr uint8_t expectedTail[] =
        {
            'R', 'O', 'O', 'T', 0,
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER,
            BridgeParameters::
                PITCH_INVERSION_PARAMETER,
            BridgeParameters::
                RESTORE_DEFAULTS_PARAMETER,
            0xFF
        };


        if (
            frame[5] !=
                BridgeParameters::
                    ROOT_PARAMETER ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_FOLDER
        )
        {
            return false;
        }


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


    bool runBrightnessAndPitchRegressionTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        uint8_t frame[64] = {};
        size_t frameLength = 0;


        if (
            !parameters.buildReadResponse(
                makeRead(
                    BridgeParameters::
                        LED_BRIGHTNESS_PARAMETER
                ),
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                frame,
                sizeof(frame),
                frameLength
            ) ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_FLOAT
        )
        {
            return false;
        }


        frameLength = 0;


        if (
            !parameters.buildReadResponse(
                makeRead(
                    BridgeParameters::
                        PITCH_INVERSION_PARAMETER
                ),
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                frame,
                sizeof(frame),
                frameLength
            ) ||
            frame[8] !=
                Crsf::PARAMETER_TYPE_TEXT_SELECTION
        )
        {
            return false;
        }


        return true;
    }


    bool runRestoreReadyReadTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        uint8_t frame[64] = {};
        size_t frameLength = 0;


        if (
            !parameters.buildReadResponse(
                makeRead(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER
                ),
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                frame,
                sizeof(frame),
                frameLength
            )
        )
        {
            return false;
        }


        uint8_t status = 0xFF;


        return
            frame[5] ==
                BridgeParameters::
                    RESTORE_DEFAULTS_PARAMETER &&
            findCommandStatus(
                frame,
                frameLength,
                status
            ) &&
            status ==
                COMMAND_READY;
    }


    bool runRestoreConfirmationLifecycleTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        configuration.ledBrightnessPercent =
            61;

        configuration.pitch.inverted =
            false;


        BridgeParameters parameters(
            configuration
        );


        uint8_t response[64] = {};
        size_t responseLength = 0;

        BridgeParameterWriteResult result;


        // START must request confirmation without changing configuration.
        if (
            !parameters.handleWrite(
                makeWrite(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER,
                    COMMAND_START
                ),
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


        uint8_t status = 0xFF;


        if (
            !findCommandStatus(
                response,
                responseLength,
                status
            ) ||
            status !=
                COMMAND_CONFIRMATION_NEEDED ||
            result.requiresPersistence ||
            configuration
                .ledBrightnessPercent != 61 ||
            configuration.pitch.inverted
        )
        {
            return false;
        }


        // POLL while pending must preserve confirmation-needed state.
        responseLength = 0;


        if (
            !parameters.handleWrite(
                makeWrite(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER,
                    COMMAND_POLL
                ),
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                response,
                sizeof(response),
                responseLength,
                result
            ) ||
            !findCommandStatus(
                response,
                responseLength,
                status
            ) ||
            status !=
                COMMAND_CONFIRMATION_NEEDED
        )
        {
            return false;
        }


        // CONFIRM stages defaults and marks the result as persistence-required.
        responseLength = 0;


        if (
            !parameters.handleWrite(
                makeWrite(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER,
                    COMMAND_CONFIRM
                ),
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
            result.change !=
                BridgeParameterChange::
                    RestoreDefaults ||
            !result.requiresPersistence ||
            configuration
                .ledBrightnessPercent != 10 ||
            !configuration.pitch.inverted ||
            !findCommandStatus(
                response,
                responseLength,
                status
            ) ||
            status !=
                COMMAND_READY
        )
        {
            return false;
        }


        // A failed persistence attempt must leave the confirmation pending.
        parameters.finalizePersistence(
            result,
            false
        );


        responseLength = 0;


        if (
            !parameters.buildReadResponse(
                makeRead(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER
                ),
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                response,
                sizeof(response),
                responseLength
            ) ||
            !findCommandStatus(
                response,
                responseLength,
                status
            ) ||
            status !=
                COMMAND_CONFIRMATION_NEEDED
        )
        {
            return false;
        }


        // Successful persistence completes the command lifecycle.
        parameters.finalizePersistence(
            result,
            true
        );


        responseLength = 0;


        return
            parameters.buildReadResponse(
                makeRead(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER
                ),
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                response,
                sizeof(response),
                responseLength
            ) &&
            findCommandStatus(
                response,
                responseLength,
                status
            ) &&
            status ==
                COMMAND_READY;
    }


    bool runRestoreCancelTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        configuration.ledBrightnessPercent =
            44;

        configuration.pitch.inverted =
            false;


        BridgeParameters parameters(
            configuration
        );


        uint8_t response[64] = {};
        size_t responseLength = 0;

        BridgeParameterWriteResult result;


        if (
            !parameters.handleWrite(
                makeWrite(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER,
                    COMMAND_START
                ),
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


        responseLength = 0;


        if (
            !parameters.handleWrite(
                makeWrite(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER,
                    COMMAND_CANCEL
                ),
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


        uint8_t status = 0xFF;


        return
            !result.requiresPersistence &&
            configuration
                .ledBrightnessPercent == 44 &&
            !configuration.pitch.inverted &&
            findCommandStatus(
                response,
                responseLength,
                status
            ) &&
            status ==
                COMMAND_READY;
    }


    bool runConfirmWithoutStartDoesNotResetTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        configuration.ledBrightnessPercent =
            70;

        configuration.pitch.inverted =
            false;


        BridgeParameters parameters(
            configuration
        );


        uint8_t response[64] = {};
        size_t responseLength = 0;

        BridgeParameterWriteResult result;


        if (
            !parameters.handleWrite(
                makeWrite(
                    BridgeParameters::
                        RESTORE_DEFAULTS_PARAMETER,
                    COMMAND_CONFIRM
                ),
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
            result.change ==
                BridgeParameterChange::None &&
            !result.requiresPersistence &&
            configuration
                .ledBrightnessPercent == 70 &&
            !configuration.pitch.inverted;
    }
}


bool CrsfParameterSelfTest::run()
{
    return
        BridgeParameters::PARAMETER_COUNT == 3 &&
        runRootFolderResponseTest() &&
        runBrightnessAndPitchRegressionTest() &&
        runRestoreReadyReadTest() &&
        runRestoreConfirmationLifecycleTest() &&
        runRestoreCancelTest() &&
        runConfirmWithoutStartDoesNotResetTest();
}
