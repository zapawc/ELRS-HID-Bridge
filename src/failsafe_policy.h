#pragma once

#include "channel_state.h"


class FailsafePolicy
{
public:
    // Apply the reference bridge failsafe output.
    //
    // v1.0 failsafe invariant:
    //
    // Roll        -> center
    // Pitch       -> center
    // Throttle    -> minimum
    // Yaw         -> center
    // AUX Analog 1 -> center
    // AUX Analog 2 -> center
    // AUX Analog 3 -> center
    // AUX Analog 4 -> center
    // Buttons     -> released
    //
    // This policy deliberately defines every HID control rather than
    // retaining stale proportional or button state after RC timeout.
    void apply(
        ChannelState& state
    ) const;
};
