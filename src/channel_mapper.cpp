#include "channel_mapper.h"

namespace
{
    constexpr uint16_t CRSF_CHANNEL_MIN = 172;
    constexpr uint16_t CRSF_CHANNEL_MAX = 1811;

    constexpr uint16_t CRSF_CHANNEL_MID =
        (CRSF_CHANNEL_MIN + CRSF_CHANNEL_MAX) / 2;
}


uint16_t ChannelMapper::scaleChannel(uint16_t rawValue)
{
    if (rawValue <= CRSF_CHANNEL_MIN)
    {
        return 0;
    }

    if (rawValue >= CRSF_CHANNEL_MAX)
    {
        return 65535;
    }

    const uint32_t inputRange =
        CRSF_CHANNEL_MAX - CRSF_CHANNEL_MIN;

    const uint32_t normalized =
        static_cast<uint32_t>(
            rawValue - CRSF_CHANNEL_MIN
        );

    return static_cast<uint16_t>(
        (normalized * 65535UL) / inputRange
    );
}


bool ChannelMapper::channelIsHigh(uint16_t rawValue)
{
    return rawValue > CRSF_CHANNEL_MID;
}


void ChannelMapper::update(
    const RawChannels& raw,
    ChannelState& state
) const
{
    // Default RC mapping:
    //
    // CH1 -> Roll
    // CH2 -> Pitch
    // CH3 -> Throttle
    // CH4 -> Yaw

    state.roll =
        scaleChannel(raw.get(ChannelIndex::CH1));

    state.pitch =
        scaleChannel(raw.get(ChannelIndex::CH2));

    state.throttle =
        scaleChannel(raw.get(ChannelIndex::CH3));

    state.yaw =
        scaleChannel(raw.get(ChannelIndex::CH4));

    // Clear all HID buttons before rebuilding their state.
    state.buttons = 0;

    // Initial/default AUX behavior:
    //
    // CH5-CH16 each control one HID button.
    //
    // This is deliberately simple for now. Later this logic
    // will become configurable for 2-position, 3-position,
    // momentary switches, etc.

    for (
        uint8_t channel = 4;
        channel < RawChannels::CHANNEL_COUNT;
        ++channel
    )
    {
        if (channelIsHigh(raw.channel[channel]))
        {
            const uint8_t buttonIndex = channel - 4;

            state.buttons |=
                (1UL << buttonIndex);
        }
    }
}