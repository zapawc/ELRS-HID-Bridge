#pragma once

#include <stdint.h>
#include "channel_index.h"


struct AxisMapping
{
    ChannelIndex channel = ChannelIndex::CH1;
    bool inverted = false;
};


struct TwoPositionSwitchMapping
{
    ChannelIndex channel = ChannelIndex::CH1;
    uint8_t button = 0;
};


struct ThreePositionSwitchMapping
{
    ChannelIndex channel = ChannelIndex::CH1;

    uint8_t centerButton = 0;
    uint8_t highButton = 0;
};


class BridgeConfiguration
{
public:
    static constexpr uint8_t TWO_POSITION_SWITCH_COUNT = 2;
    static constexpr uint8_t THREE_POSITION_SWITCH_COUNT = 6;


    // Create the known-good reference configuration.
    //
    // These defaults intentionally reproduce the currently
    // validated EdgeTX -> CRSF -> USB HID mapping.
    static BridgeConfiguration defaults();

    // -------------------------------------------------------------------------
    // Primary controls
    // -------------------------------------------------------------------------

    AxisMapping roll;
    AxisMapping pitch;
    AxisMapping throttle;
    AxisMapping yaw;


    // -------------------------------------------------------------------------
    // Additional proportional controls
    // -------------------------------------------------------------------------

    AxisMapping auxAnalog1;
    AxisMapping auxAnalog2;
    AxisMapping auxAnalog3;
    AxisMapping auxAnalog4;


    // -------------------------------------------------------------------------
    // Switch controls
    // -------------------------------------------------------------------------

    TwoPositionSwitchMapping
        twoPositionSwitches[TWO_POSITION_SWITCH_COUNT] = {};

    ThreePositionSwitchMapping
        threePositionSwitches[THREE_POSITION_SWITCH_COUNT] = {};


    // -------------------------------------------------------------------------
    // Switch decoding
    //
    // Normalized channel range:
    //
    // MIN = 0
    // MID = 32768
    // MAX = 65535
    //
    // Existing known-good thresholds are preserved.
    // -------------------------------------------------------------------------

    uint16_t switchLowThreshold = 16384;
    uint16_t switchHighThreshold = 49151;


    // -------------------------------------------------------------------------
    // Receiver behavior
    // -------------------------------------------------------------------------

    uint32_t receiverTimeoutMs = 500;


    // -------------------------------------------------------------------------
    // Local status display
    //
    // Runtime-only in the first CRSF-parameter checkpoint.
    // Persistence is intentionally deferred.
    // -------------------------------------------------------------------------

    uint8_t ledBrightnessPercent = 10;
};
