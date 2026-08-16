#include "bridge_parameters.h"

#include "crsf_protocol.h"


BridgeParameters::BridgeParameters(
    BridgeConfiguration& configuration
)
    :
    configuration(
        configuration
    )
{
}


bool BridgeParameters::buildInversionParameterResponse(
    const CrsfParameterRead& request,
    uint8_t localAddress,
    uint8_t parameterNumber,
    const char* name,
    const AxisMapping& mapping,
    bool defaultInverted,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    CrsfDevice responseBuilder;


    return
        responseBuilder
            .buildTextSelectionParameterResponse(
                request,
                localAddress,
                parameterNumber,
                ROOT_PARAMETER,
                name,
                "Normal;Inverted",
                mapping.inverted
                    ? AXIS_INVERTED
                    : AXIS_NORMAL,
                AXIS_NORMAL,
                AXIS_INVERTED,
                defaultInverted
                    ? AXIS_INVERTED
                    : AXIS_NORMAL,
                "",
                output,
                outputCapacity,
                outputLength
            );
}


bool BridgeParameters::handleInversionWrite(
    const CrsfParameterWrite& request,
    uint8_t localAddress,
    uint8_t parameterNumber,
    AxisMapping& mapping,
    BridgeParameterChange change,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength,
    BridgeParameterWriteResult& result
)
{
    if (
        request.dataLength != 1
    )
    {
        return false;
    }


    const uint8_t selection =
        request.data[0];


    if (
        selection != AXIS_NORMAL &&
        selection != AXIS_INVERTED
    )
    {
        return false;
    }


    CrsfDevice responseBuilder;


    if (
        !responseBuilder
            .buildTextSelectionWriteResponse(
                request,
                localAddress,
                parameterNumber,
                selection,
                output,
                outputCapacity,
                outputLength
            )
    )
    {
        return false;
    }


    mapping.inverted =
        selection ==
        AXIS_INVERTED;


    result.change =
        change;

    result.pitchInverted =
        configuration.pitch.inverted;

    result.requiresPersistence =
        true;


    return true;
}


bool BridgeParameters::buildRestoreDefaultsResponse(
    uint8_t destination,
    uint8_t origin,
    uint8_t localAddress,
    uint8_t status,
    const char* info,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    CrsfDevice responseBuilder;


    return
        responseBuilder
            .buildCommandParameterResponse(
                destination,
                origin,
                localAddress,
                RESTORE_DEFAULTS_PARAMETER,
                ROOT_PARAMETER,
                "Restore Defaults",
                status,
                RESTORE_DEFAULTS_TIMEOUT,
                info,
                output,
                outputCapacity,
                outputLength
            );
}


bool BridgeParameters::buildReadResponse(
    const CrsfParameterRead& request,
    uint8_t localAddress,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    CrsfDevice responseBuilder;

    const BridgeConfiguration defaults =
        BridgeConfiguration::defaults();


    switch (
        request.parameterNumber
    )
    {
        case ROOT_PARAMETER:
        {
            constexpr uint8_t children[] =
            {
                LED_BRIGHTNESS_PARAMETER,
                PITCH_INVERSION_PARAMETER,
                RESTORE_DEFAULTS_PARAMETER,
                ROLL_INVERSION_PARAMETER,
                THROTTLE_INVERSION_PARAMETER,
                YAW_INVERSION_PARAMETER,
                AUX1_INVERSION_PARAMETER,
                AUX2_INVERSION_PARAMETER,
                AUX3_INVERSION_PARAMETER,
                AUX4_INVERSION_PARAMETER
            };


            return
                responseBuilder
                    .buildFolderParameterResponse(
                        request,
                        localAddress,
                        ROOT_PARAMETER,
                        ROOT_PARAMETER,
                        "ROOT",
                        children,
                        sizeof(children),
                        output,
                        outputCapacity,
                        outputLength
                    );
        }


        case LED_BRIGHTNESS_PARAMETER:
        {
            return
                responseBuilder
                    .buildFloatParameterResponse(
                        request,
                        localAddress,
                        LED_BRIGHTNESS_PARAMETER,
                        ROOT_PARAMETER,
                        "LED Brightness",
                        configuration
                            .ledBrightnessPercent,
                        LED_BRIGHTNESS_MIN,
                        LED_BRIGHTNESS_MAX,
                        LED_BRIGHTNESS_DEFAULT,
                        0,
                        LED_BRIGHTNESS_STEP,
                        "",
                        output,
                        outputCapacity,
                        outputLength
                    );
        }


        case PITCH_INVERSION_PARAMETER:
        {
            return
                buildInversionParameterResponse(
                    request,
                    localAddress,
                    PITCH_INVERSION_PARAMETER,
                    "Pitch Inversion",
                    configuration.pitch,
                    defaults.pitch.inverted,
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        case RESTORE_DEFAULTS_PARAMETER:
        {
            if (
                request.chunkNumber != 0
            )
            {
                return false;
            }


            return
                buildRestoreDefaultsResponse(
                    request.destination,
                    request.origin,
                    localAddress,
                    restoreDefaultsConfirmationPending
                        ? COMMAND_CONFIRMATION_NEEDED
                        : COMMAND_READY,
                    restoreDefaultsConfirmationPending
                        ? "Restore defaults?"
                        : "",
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        case ROLL_INVERSION_PARAMETER:
        {
            return
                buildInversionParameterResponse(
                    request,
                    localAddress,
                    ROLL_INVERSION_PARAMETER,
                    "Roll Inversion",
                    configuration.roll,
                    defaults.roll.inverted,
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        case THROTTLE_INVERSION_PARAMETER:
        {
            return
                buildInversionParameterResponse(
                    request,
                    localAddress,
                    THROTTLE_INVERSION_PARAMETER,
                    "Throttle Inversion",
                    configuration.throttle,
                    defaults.throttle.inverted,
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        case YAW_INVERSION_PARAMETER:
        {
            return
                buildInversionParameterResponse(
                    request,
                    localAddress,
                    YAW_INVERSION_PARAMETER,
                    "Yaw Inversion",
                    configuration.yaw,
                    defaults.yaw.inverted,
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        case AUX1_INVERSION_PARAMETER:
        {
            return
                buildInversionParameterResponse(
                    request,
                    localAddress,
                    AUX1_INVERSION_PARAMETER,
                    "Aux 1 Inversion",
                    configuration.auxAnalog1,
                    defaults.auxAnalog1.inverted,
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        case AUX2_INVERSION_PARAMETER:
        {
            return
                buildInversionParameterResponse(
                    request,
                    localAddress,
                    AUX2_INVERSION_PARAMETER,
                    "Aux 2 Inversion",
                    configuration.auxAnalog2,
                    defaults.auxAnalog2.inverted,
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        case AUX3_INVERSION_PARAMETER:
        {
            return
                buildInversionParameterResponse(
                    request,
                    localAddress,
                    AUX3_INVERSION_PARAMETER,
                    "Aux 3 Inversion",
                    configuration.auxAnalog3,
                    defaults.auxAnalog3.inverted,
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        case AUX4_INVERSION_PARAMETER:
        {
            return
                buildInversionParameterResponse(
                    request,
                    localAddress,
                    AUX4_INVERSION_PARAMETER,
                    "Aux 4 Inversion",
                    configuration.auxAnalog4,
                    defaults.auxAnalog4.inverted,
                    output,
                    outputCapacity,
                    outputLength
                );
        }


        default:
        {
            return false;
        }
    }
}


bool BridgeParameters::handleWrite(
    const CrsfParameterWrite& request,
    uint8_t localAddress,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength,
    BridgeParameterWriteResult& result
)
{
    outputLength = 0;

    result =
        BridgeParameterWriteResult{};


    if (
        request.destination !=
            localAddress
    )
    {
        return false;
    }


    switch (
        request.parameterNumber
    )
    {
        case LED_BRIGHTNESS_PARAMETER:
        {
            int32_t brightness = 0;


            if (
                !CrsfDevice::readInt32BigEndian(
                    request.data,
                    request.dataLength,
                    brightness
                )
            )
            {
                return false;
            }


            if (
                brightness <
                    LED_BRIGHTNESS_MIN ||
                brightness >
                    LED_BRIGHTNESS_MAX
            )
            {
                return false;
            }


            CrsfDevice responseBuilder;


            if (
                !responseBuilder
                    .buildFloatWriteResponse(
                        request,
                        localAddress,
                        LED_BRIGHTNESS_PARAMETER,
                        brightness,
                        output,
                        outputCapacity,
                        outputLength
                    )
            )
            {
                return false;
            }


            configuration
                .ledBrightnessPercent =
                static_cast<uint8_t>(
                    brightness
                );


            result.change =
                BridgeParameterChange::
                    LedBrightness;

            result.ledBrightnessPercent =
                configuration
                    .ledBrightnessPercent;

            result.requiresPersistence =
                true;


            return true;
        }


        case PITCH_INVERSION_PARAMETER:
        {
            return
                handleInversionWrite(
                    request,
                    localAddress,
                    PITCH_INVERSION_PARAMETER,
                    configuration.pitch,
                    BridgeParameterChange::
                        PitchInversion,
                    output,
                    outputCapacity,
                    outputLength,
                    result
                );
        }


        case RESTORE_DEFAULTS_PARAMETER:
        {
            if (
                request.dataLength != 1
            )
            {
                return false;
            }


            const uint8_t command =
                request.data[0];


            switch (command)
            {
                case COMMAND_START:
                {
                    restoreDefaultsConfirmationPending =
                        true;


                    return
                        buildRestoreDefaultsResponse(
                            request.destination,
                            request.origin,
                            localAddress,
                            COMMAND_CONFIRMATION_NEEDED,
                            "Restore defaults?",
                            output,
                            outputCapacity,
                            outputLength
                        );
                }


                case COMMAND_CONFIRM:
                {
                    if (
                        !restoreDefaultsConfirmationPending
                    )
                    {
                        return
                            buildRestoreDefaultsResponse(
                                request.destination,
                                request.origin,
                                localAddress,
                                COMMAND_READY,
                                "No pending request",
                                output,
                                outputCapacity,
                                outputLength
                            );
                    }


                    configuration =
                        BridgeConfiguration::defaults();


                    result.change =
                        BridgeParameterChange::
                            RestoreDefaults;

                    result.ledBrightnessPercent =
                        configuration
                            .ledBrightnessPercent;

                    result.pitchInverted =
                        configuration.pitch.inverted;

                    result.requiresPersistence =
                        true;


                    return
                        buildRestoreDefaultsResponse(
                            request.destination,
                            request.origin,
                            localAddress,
                            COMMAND_READY,
                            "Defaults restored",
                            output,
                            outputCapacity,
                            outputLength
                        );
                }


                case COMMAND_CANCEL:
                {
                    restoreDefaultsConfirmationPending =
                        false;


                    return
                        buildRestoreDefaultsResponse(
                            request.destination,
                            request.origin,
                            localAddress,
                            COMMAND_READY,
                            "Cancelled",
                            output,
                            outputCapacity,
                            outputLength
                        );
                }


                case COMMAND_POLL:
                {
                    return
                        buildRestoreDefaultsResponse(
                            request.destination,
                            request.origin,
                            localAddress,
                            restoreDefaultsConfirmationPending
                                ? COMMAND_CONFIRMATION_NEEDED
                                : COMMAND_READY,
                            restoreDefaultsConfirmationPending
                                ? "Restore defaults?"
                                : "",
                            output,
                            outputCapacity,
                            outputLength
                        );
                }


                default:
                {
                    return false;
                }
            }
        }


        case ROLL_INVERSION_PARAMETER:
        {
            return
                handleInversionWrite(
                    request,
                    localAddress,
                    ROLL_INVERSION_PARAMETER,
                    configuration.roll,
                    BridgeParameterChange::
                        AxisInversion,
                    output,
                    outputCapacity,
                    outputLength,
                    result
                );
        }


        case THROTTLE_INVERSION_PARAMETER:
        {
            return
                handleInversionWrite(
                    request,
                    localAddress,
                    THROTTLE_INVERSION_PARAMETER,
                    configuration.throttle,
                    BridgeParameterChange::
                        AxisInversion,
                    output,
                    outputCapacity,
                    outputLength,
                    result
                );
        }


        case YAW_INVERSION_PARAMETER:
        {
            return
                handleInversionWrite(
                    request,
                    localAddress,
                    YAW_INVERSION_PARAMETER,
                    configuration.yaw,
                    BridgeParameterChange::
                        AxisInversion,
                    output,
                    outputCapacity,
                    outputLength,
                    result
                );
        }


        case AUX1_INVERSION_PARAMETER:
        {
            return
                handleInversionWrite(
                    request,
                    localAddress,
                    AUX1_INVERSION_PARAMETER,
                    configuration.auxAnalog1,
                    BridgeParameterChange::
                        AxisInversion,
                    output,
                    outputCapacity,
                    outputLength,
                    result
                );
        }


        case AUX2_INVERSION_PARAMETER:
        {
            return
                handleInversionWrite(
                    request,
                    localAddress,
                    AUX2_INVERSION_PARAMETER,
                    configuration.auxAnalog2,
                    BridgeParameterChange::
                        AxisInversion,
                    output,
                    outputCapacity,
                    outputLength,
                    result
                );
        }


        case AUX3_INVERSION_PARAMETER:
        {
            return
                handleInversionWrite(
                    request,
                    localAddress,
                    AUX3_INVERSION_PARAMETER,
                    configuration.auxAnalog3,
                    BridgeParameterChange::
                        AxisInversion,
                    output,
                    outputCapacity,
                    outputLength,
                    result
                );
        }


        case AUX4_INVERSION_PARAMETER:
        {
            return
                handleInversionWrite(
                    request,
                    localAddress,
                    AUX4_INVERSION_PARAMETER,
                    configuration.auxAnalog4,
                    BridgeParameterChange::
                        AxisInversion,
                    output,
                    outputCapacity,
                    outputLength,
                    result
                );
        }


        default:
        {
            return false;
        }
    }
}


void BridgeParameters::finalizePersistence(
    const BridgeParameterWriteResult& result,
    bool succeeded
)
{
    if (
        result.change ==
            BridgeParameterChange::
                RestoreDefaults &&
        succeeded
    )
    {
        restoreDefaultsConfirmationPending =
            false;
    }
}
