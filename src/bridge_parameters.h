#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bridge_configuration.h"
#include "crsf_device.h"


enum class BridgeParameterChange
{
    None,
    LedBrightness
};


struct BridgeParameterWriteResult
{
    BridgeParameterChange change =
        BridgeParameterChange::None;

    uint8_t ledBrightnessPercent = 0;
};


class BridgeParameters
{
public:
    // Parameter 0 is the standard root folder.
    //
    // Normal numbered parameters begin at 1.
    static constexpr uint8_t ROOT_PARAMETER = 0;

    static constexpr uint8_t LED_BRIGHTNESS_PARAMETER = 1;

    static constexpr uint8_t PARAMETER_COUNT = 1;


    explicit BridgeParameters(
        BridgeConfiguration& configuration
    );


    // Build the appropriate CRSF Parameter Settings Entry (0x2B)
    // response for a validated Parameter Read (0x2C).
    //
    // Returns false for unknown parameters, unsupported chunks,
    // invalid routing, or insufficient output capacity.
    bool buildReadResponse(
        const CrsfParameterRead& request,
        uint8_t localAddress,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    // Validate and apply a CRSF Parameter Write (0x2D).
    //
    // On success:
    // - BridgeConfiguration is updated,
    // - result identifies the application-side effect to perform,
    // - output contains the 0x2D acknowledgement.
    //
    // Persistence is intentionally outside this component.
    bool handleWrite(
        const CrsfParameterWrite& request,
        uint8_t localAddress,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength,
        BridgeParameterWriteResult& result
    );


private:
    static constexpr int32_t LED_BRIGHTNESS_MIN = 0;
    static constexpr int32_t LED_BRIGHTNESS_MAX = 100;
    static constexpr int32_t LED_BRIGHTNESS_DEFAULT = 10;
    static constexpr int32_t LED_BRIGHTNESS_STEP = 1;


    BridgeConfiguration& configuration;
};
