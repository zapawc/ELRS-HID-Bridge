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
    // Default primary control mapping.
    state.roll =
        channels.get(ChannelIndex::CH1);

    state.pitch =
        channels.get(ChannelIndex::CH2);

    state.throttle =
        channels.get(ChannelIndex::CH3);

    state.yaw =
        channels.get(ChannelIndex::CH4);

    // Rebuild button state every update.
    state.buttons = 0;

    // CH5-CH16 currently map to buttons 1-12.
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