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


        uint8_t frame[64] = {};
        size_t frameLength = 0;


        if (
            !parameters.buildReadResponse(
                makeRead(
                    BridgeParameters::
                        ROOT_PARAMETER
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


        constexpr uint8_t expectedTail[] =
        {
            'R', 'O', 'O', 'T', 0,
            BridgeParameters::
                LED_BRIGHTNESS_PARAMETER,
            BridgeParameters::
                PITCH_INVERSION_PARAMETER,
            BridgeParameters::
                ROLL_INVERSION_PARAMETER,
            BridgeParameters::
                YAW_INVERSION_PARAMETER,
            BridgeParameters::
                AUX1_INVERSION_PARAMETER,
            BridgeParameters::
                AUX2_INVERSION_PARAMETER,
            BridgeParameters::
                AUX3_INVERSION_PARAMETER,
            BridgeParameters::
                AUX4_INVERSION_PARAMETER,
            BridgeParameters::
                RESTORE_DEFAULTS_PARAMETER,
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


    bool runInversionEntryTest(
        uint8_t parameterNumber
    )
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        uint8_t frame[64] = {};
        size_t frameLength = 0;


        return
            parameters.buildReadResponse(
                makeRead(
                    parameterNumber
                ),
                Crsf::ADDRESS_FLIGHT_CONTROLLER,
                frame,
                sizeof(frame),
                frameLength
            ) &&
            frameLength > 10 &&
            frame[5] ==
                parameterNumber &&
            frame[8] ==
                Crsf::PARAMETER_TYPE_TEXT_SELECTION;
    }


    bool runAllInversionEntriesTest()
    {
        return
            runInversionEntryTest(
                BridgeParameters::
                    PITCH_INVERSION_PARAMETER
            ) &&
            runInversionEntryTest(
                BridgeParameters::
                    ROLL_INVERSION_PARAMETER
            ) &&
            runInversionEntryTest(
                BridgeParameters::
                    YAW_INVERSION_PARAMETER
            ) &&
            runInversionEntryTest(
                BridgeParameters::
                    AUX1_INVERSION_PARAMETER
            ) &&
            runInversionEntryTest(
                BridgeParameters::
                    AUX2_INVERSION_PARAMETER
            ) &&
            runInversionEntryTest(
                BridgeParameters::
                    AUX3_INVERSION_PARAMETER
            ) &&
            runInversionEntryTest(
                BridgeParameters::
                    AUX4_INVERSION_PARAMETER
            );
    }


    bool applyInversionWrite(
        BridgeParameters& parameters,
        uint8_t parameterNumber,
        uint8_t selection
    )
    {
        uint8_t response[64] = {};
        size_t responseLength = 0;

        BridgeParameterWriteResult result;


        if (
            !parameters.handleWrite(
                makeWrite(
                    parameterNumber,
                    selection
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
            result.requiresPersistence &&
            responseLength == 8 &&
            response[2] ==
                Crsf::FRAME_PARAMETER_WRITE &&
            response[5] ==
                parameterNumber &&
            response[6] ==
                selection;
    }


    bool runAllInversionWritesTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        BridgeParameters parameters(
            configuration
        );


        if (
            !applyInversionWrite(
                parameters,
                BridgeParameters::
                    PITCH_INVERSION_PARAMETER,
                0
            ) ||
            configuration.pitch.inverted
        )
        {
            return false;
        }


        if (
            !applyInversionWrite(
                parameters,
                BridgeParameters::
                    ROLL_INVERSION_PARAMETER,
                1
            ) ||
            !configuration.roll.inverted
        )
        {
            return false;
        }


        if (
            !applyInversionWrite(
                parameters,
                BridgeParameters::
                    YAW_INVERSION_PARAMETER,
                1
            ) ||
            !configuration.yaw.inverted
        )
        {
            return false;
        }


        if (
            !applyInversionWrite(
                parameters,
                BridgeParameters::
                    AUX1_INVERSION_PARAMETER,
                1
            ) ||
            !configuration.auxAnalog1.inverted
        )
        {
            return false;
        }


        if (
            !applyInversionWrite(
                parameters,
                BridgeParameters::
                    AUX2_INVERSION_PARAMETER,
                1
            ) ||
            !configuration.auxAnalog2.inverted
        )
        {
            return false;
        }


        if (
            !applyInversionWrite(
                parameters,
                BridgeParameters::
                    AUX3_INVERSION_PARAMETER,
                1
            ) ||
            !configuration.auxAnalog3.inverted
        )
        {
            return false;
        }


        if (
            !applyInversionWrite(
                parameters,
                BridgeParameters::
                    AUX4_INVERSION_PARAMETER,
                1
            ) ||
            !configuration.auxAnalog4.inverted
        )
        {
            return false;
        }


        // Invalid selection must not modify the current setting.
        uint8_t response[64] = {};
        size_t responseLength = 123;
        BridgeParameterWriteResult result;


        if (
            parameters.handleWrite(
                makeWrite(
                    BridgeParameters::
                        ROLL_INVERSION_PARAMETER,
                    2
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
            configuration.roll.inverted &&
            responseLength == 0 &&
            result.change ==
                BridgeParameterChange::None;
    }


    bool runRestoreDefaultsRegressionTest()
    {
        BridgeConfiguration configuration =
            BridgeConfiguration::defaults();

        configuration.ledBrightnessPercent =
            54;

        configuration.roll.inverted =
            true;

        configuration.pitch.inverted =
            false;

        configuration.throttle.inverted =
            true;

        configuration.auxAnalog4.inverted =
            true;


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


        uint8_t status = 0xFF;


        if (
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


        const BridgeConfiguration defaults =
            BridgeConfiguration::defaults();


        if (
            result.change !=
                BridgeParameterChange::
                    RestoreDefaults ||
            !result.requiresPersistence ||
            configuration
                .ledBrightnessPercent !=
                    defaults.ledBrightnessPercent ||
            configuration.roll.inverted !=
                defaults.roll.inverted ||
            configuration.pitch.inverted !=
                defaults.pitch.inverted ||
            configuration.throttle.inverted !=
                defaults.throttle.inverted ||
            configuration.yaw.inverted !=
                defaults.yaw.inverted ||
            configuration.auxAnalog1.inverted !=
                defaults.auxAnalog1.inverted ||
            configuration.auxAnalog2.inverted !=
                defaults.auxAnalog2.inverted ||
            configuration.auxAnalog3.inverted !=
                defaults.auxAnalog3.inverted ||
            configuration.auxAnalog4.inverted !=
                defaults.auxAnalog4.inverted
        )
        {
            return false;
        }


        parameters.finalizePersistence(
            result,
            true
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
                COMMAND_READY
        )
        {
            return false;
        }


        // Re-establish a non-default value and prove cancel does not reset it.
        configuration.roll.inverted =
            true;


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


        return
            configuration.roll.inverted;
    }
}


bool CrsfParameterSelfTest::run()
{
    return
        BridgeParameters::PARAMETER_COUNT == 9 &&
        runRootFolderResponseTest() &&
        runAllInversionEntriesTest() &&
        runAllInversionWritesTest() &&
        runRestoreDefaultsRegressionTest();
}
