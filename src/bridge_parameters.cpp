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
                LED_BRIGHTNESS_PARAMETER
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
                        //
                        // Keep the CRSF FLOAT parameter standards-based
                        // while leaving the presentation unit blank.
                        "",
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


            return true;
        }


        default:
        {
            return false;
        }
    }
}
