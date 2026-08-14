#include "channel_mapper.h"

bool ChannelMapper::channelIsHigh(uint16_t value)
{
    return value > NormalizedChannels::MID;
}

void ChannelMapper::update(
    const NormalizedChannels& channels,
    ChannelState& state
) const
{
    // Primary control mapping
    //
    // CH1 -> Roll
    // CH2 -> Pitch
    // CH3 -> Throttle
    // CH4 -> Yaw
    //
    // HID direction corrections:
    // Roll     = normal
    // Pitch    = inverted
    // Throttle = normal
    // Yaw      = normal

    state.roll =
        channels.get(ChannelIndex::CH1);

    state.pitch =
        NormalizedChannels::MAX -
        channels.get(ChannelIndex::CH2);

    state.throttle =
        channels.get(ChannelIndex::CH3);

    state.yaw =
        channels.get(ChannelIndex::CH4);

    // Rebuild button state on every update.
    state.buttons = 0;

    // Current AUX behavior:
    //
    // CH5-CH16 -> HID Buttons 1-12
    //
    // A channel activates its button whenever its normalized value
    // is above the midpoint. This is intentionally simple for now.

    for (
        uint8_t channel = 4;
        channel < NormalizedChannels::CHANNEL_COUNT;
        ++channel
    )
    {
        if (channelIsHigh(channels.channel[channel]))
        {
            const uint8_t buttonIndex =
                channel - 4;

            state.buttons |=
                (1UL << buttonIndex);
        }
    }
}