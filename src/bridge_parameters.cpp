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
                RESTORE_DEFAULTS_PARAMETER
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

                        // ExpressLRS elrs.lua r18 directly appends the
                        // unit to a Lua string.format pattern. A literal
                        // "%" therefore causes a formatter exception.
                        "",
                        output,
                        outputCapacity,
                        outputLength
                    );
        }


        case PITCH_INVERSION_PARAMETER:
        {
            const uint8_t currentValue =
                configuration.pitch.inverted
                    ? PITCH_INVERTED
                    : PITCH_NORMAL;


            return
                responseBuilder
                    .buildTextSelectionParameterResponse(
                        request,
                        localAddress,
                        PITCH_INVERSION_PARAMETER,
                        ROOT_PARAMETER,
                        "Pitch Inversion",
                        "Normal;Inverted",
                        currentValue,
                        PITCH_NORMAL,
                        PITCH_INVERTED,
                        PITCH_INVERSION_DEFAULT,
                        "",
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
            if (
                request.dataLength != 1
            )
            {
                return false;
            }


            const uint8_t selection =
                request.data[0];


            if (
                selection != PITCH_NORMAL &&
                selection != PITCH_INVERTED
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
                        PITCH_INVERSION_PARAMETER,
                        selection,
                        output,
                        outputCapacity,
                        outputLength
                    )
            )
            {
                return false;
            }


            configuration.pitch.inverted =
                selection ==
                PITCH_INVERTED;


            result.change =
                BridgeParameterChange::
                    PitchInversion;

            result.pitchInverted =
                configuration.pitch.inverted;

            result.requiresPersistence =
                true;


            return true;
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


                    // The response is constructed now but main.cpp does not
                    // transmit it until the default configuration has been
                    // committed successfully.
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
