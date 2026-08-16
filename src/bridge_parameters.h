#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bridge_configuration.h"
#include "crsf_device.h"


enum class BridgeParameterChange
{
    None,
    LedBrightness,
    PitchInversion,
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
    // Parameter 0 is the standard root folder.
    //
    // Normal numbered parameters begin at 1.
    static constexpr uint8_t ROOT_PARAMETER = 0;

    static constexpr uint8_t LED_BRIGHTNESS_PARAMETER = 1;

    static constexpr uint8_t PITCH_INVERSION_PARAMETER = 2;

    static constexpr uint8_t RESTORE_DEFAULTS_PARAMETER = 3;

    static constexpr uint8_t PARAMETER_COUNT = 3;


    explicit BridgeParameters(
        BridgeConfiguration& configuration
    );


    // Build the appropriate CRSF Parameter Settings Entry (0x2B)
    // response for a validated Parameter Read (0x2C).
    bool buildReadResponse(
        const CrsfParameterRead& request,
        uint8_t localAddress,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    // Validate and process a CRSF Parameter Write (0x2D).
    //
    // For FLOAT/TEXT_SELECTION:
    // - BridgeConfiguration is updated,
    // - result.requiresPersistence is true,
    // - output contains the normal 0x2D acknowledgement.
    //
    // For Restore Defaults COMMAND:
    // - START returns CONFIRMATION_NEEDED without changing configuration,
    // - CONFIRM stages BridgeConfiguration::defaults() and requires persistence,
    // - CANCEL/POLL return the appropriate command-state 0x2B entry.
    bool handleWrite(
        const CrsfParameterWrite& request,
        uint8_t localAddress,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength,
        BridgeParameterWriteResult& result
    );


    // Complete transactional state after main.cpp attempts persistence.
    //
    // For Restore Defaults, successful persistence clears the pending
    // confirmation state. Failed persistence intentionally leaves it pending so
    // the host can poll/retry rather than seeing a false READY state.
    void finalizePersistence(
        const BridgeParameterWriteResult& result,
        bool succeeded
    );


private:
    static constexpr int32_t LED_BRIGHTNESS_MIN = 0;
    static constexpr int32_t LED_BRIGHTNESS_MAX = 100;
    static constexpr int32_t LED_BRIGHTNESS_DEFAULT = 10;
    static constexpr int32_t LED_BRIGHTNESS_STEP = 1;


    static constexpr uint8_t PITCH_NORMAL = 0;
    static constexpr uint8_t PITCH_INVERTED = 1;

    static constexpr uint8_t PITCH_INVERSION_DEFAULT =
        PITCH_INVERTED;


    // CRSF COMMAND lifecycle states.
    static constexpr uint8_t COMMAND_READY = 0;
    static constexpr uint8_t COMMAND_START = 1;
    static constexpr uint8_t COMMAND_PROGRESS = 2;
    static constexpr uint8_t COMMAND_CONFIRMATION_NEEDED = 3;
    static constexpr uint8_t COMMAND_CONFIRM = 4;
    static constexpr uint8_t COMMAND_CANCEL = 5;
    static constexpr uint8_t COMMAND_POLL = 6;

    // ExpressLRS Lua r18 uses this value as the interval before a follow-up
    // command query. Responses in this checkpoint are immediate; 50 provides
    // comfortable margin without creating a long stale popup.
    static constexpr uint8_t RESTORE_DEFAULTS_TIMEOUT = 50;


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

    bool restoreDefaultsConfirmationPending = false;
};
