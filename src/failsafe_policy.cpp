#include "failsafe_policy.h"

#include "normalized_channels.h"


void FailsafePolicy::apply(
    ChannelState& state
) const
{
    state.roll =
        NormalizedChannels::MID;

    state.pitch =
        NormalizedChannels::MID;

    state.throttle =
        NormalizedChannels::MIN;

    state.yaw =
        NormalizedChannels::MID;


    state.auxAnalog1 =
        NormalizedChannels::MID;

    state.auxAnalog2 =
        NormalizedChannels::MID;

    state.auxAnalog3 =
        NormalizedChannels::MID;

    state.auxAnalog4 =
        NormalizedChannels::MID;


    state.buttons = 0;
}
