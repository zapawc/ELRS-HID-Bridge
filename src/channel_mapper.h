#pragma once

#include <stdint.h>

#include "channel_state.h"
#include "raw_channels.h"

class ChannelMapper
{
public:
    void update(
        const RawChannels& raw,
        ChannelState& state
    ) const;

private:
    static uint16_t scaleChannel(uint16_t rawValue);
    static bool channelIsHigh(uint16_t rawValue);
};