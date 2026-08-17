#pragma once

#include <stddef.h>
#include <stdint.h>
#include "bridge_configuration.h"
#include "bridge_state.h"
#include "crsf_device.h"


enum class BridgeParameterChange
{
    None,
    LedBrightness,
    PitchInversion,
    AxisInversion,
    RestoreDefaults
};


struct BridgeParameterWriteResult
{
    BridgeParameterChange change =
        BridgeParameterChange::None;

    uint8_t ledBrightnessPercent = 0;

    bool pitchInverted = true;

    // Simple value writes and confirmed Restore Defaults must be durably saved
    // before their CRSF response is transmitted.
    bool requiresPersistence = false;
};


class BridgeParameters
{
public:
    // ExpressLRS Lua r18 loads and displays top-level device parameters in
    // ascending numeric parameter-ID order.
    //
    // Hardware testing with the reference EdgeTX/ExpressLRS path established a
    // practical device-menu parameter-name ceiling of 16 characters:
    //
    //   16-character name -> enumerates normally
    //   17-character name -> menu enumeration stalls at that parameter
    //
    // Keep user-visible parameter names at 16 characters or fewer unless a
    // future upstream implementation is explicitly validated with longer names.
    static constexpr uint8_t ROOT_PARAMETER = 0;

    static constexpr uint8_t LED_BRIGHTNESS_PARAMETER = 1;
    static constexpr uint8_t PITCH_INVERSION_PARAMETER = 2;
    static constexpr uint8_t THROTTLE_INVERSION_PARAMETER = 3;

    static constexpr uint8_t ROLL_INVERSION_PARAMETER = 4;

    static constexpr uint8_t YAW_INVERSION_PARAMETER = 5;

    static constexpr uint8_t AUX1_INVERSION_PARAMETER = 6;

    static constexpr uint8_t AUX2_INVERSION_PARAMETER = 7;

    static constexpr uint8_t AUX3_INVERSION_PARAMETER = 8;

    static constexpr uint8_t AUX4_INVERSION_PARAMETER = 9;

    static constexpr uint8_t DIAGNOSTICS_FOLDER_PARAMETER = 10;
    static constexpr uint8_t FAILSAFE_COUNT_INFO_PARAMETER = 11;

    static constexpr uint8_t RESTORE_DEFAULTS_PARAMETER = 12;

    static constexpr uint8_t PARAMETER_COUNT = 12;


    explicit BridgeParameters(
        BridgeConfiguration& configuration
    );


    // Attach the existing live BridgeState after global construction.
    //
    // BridgeParameters never owns or mutates this state.
    void attachBridgeState(
        const BridgeState& state
    );


    bool buildReadResponse(
        const CrsfParameterRead& request,
        uint8_t localAddress,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    bool handleWrite(
        const CrsfParameterWrite& request,
        uint8_t localAddress,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength,
        BridgeParameterWriteResult& result
    );


    void finalizePersistence(
        const BridgeParameterWriteResult& result,
        bool succeeded
    );


private:
    static constexpr int32_t LED_BRIGHTNESS_MIN = 0;
    static constexpr int32_t LED_BRIGHTNESS_MAX = 100;
    static constexpr int32_t LED_BRIGHTNESS_DEFAULT = 10;
    static constexpr int32_t LED_BRIGHTNESS_STEP = 1;


    static constexpr uint8_t AXIS_NORMAL = 0;
    static constexpr uint8_t AXIS_INVERTED = 1;


    // CRSF COMMAND lifecycle states.
    static constexpr uint8_t COMMAND_READY = 0;
    static constexpr uint8_t COMMAND_START = 1;
    static constexpr uint8_t COMMAND_PROGRESS = 2;
    static constexpr uint8_t COMMAND_CONFIRMATION_NEEDED = 3;
    static constexpr uint8_t COMMAND_CONFIRM = 4;
    static constexpr uint8_t COMMAND_CANCEL = 5;
    static constexpr uint8_t COMMAND_POLL = 6;

    static constexpr uint8_t RESTORE_DEFAULTS_TIMEOUT = 50;


    bool buildInversionParameterResponse(
        const CrsfParameterRead& request,
        uint8_t localAddress,
        uint8_t parameterNumber,
        const char* name,
        const AxisMapping& mapping,
        bool defaultInverted,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    bool handleInversionWrite(
        const CrsfParameterWrite& request,
        uint8_t localAddress,
        uint8_t parameterNumber,
        AxisMapping& mapping,
        BridgeParameterChange change,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength,
        BridgeParameterWriteResult& result
    );


    bool buildRestoreDefaultsResponse(
        uint8_t destination,
        uint8_t origin,
        uint8_t localAddress,
        uint8_t status,
        const char* info,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    BridgeConfiguration& configuration;

    const BridgeState* bridgeState = nullptr;

    bool restoreDefaultsConfirmationPending = false;
};
