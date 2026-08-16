#pragma once

#include "channel_state.h"


class FailsafePolicy
{
public:
    // Apply the reference bridge failsafe output.
    //
    // Current validated behavior:
    //
    // Roll      -> center
    // Pitch     -> center
    // Throttle  -> minimum
    // Yaw       -> center
    // Buttons   -> released
    //
    // Auxiliary proportional outputs are intentionally left
    // unchanged in this refactor to preserve existing behavior.
    void apply(
        ChannelState& state
    ) const;
};