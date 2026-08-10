#pragma once

#include <stdint.h>

#include "channel_state.h"
#include "normalized_channels.h"

class ChannelMapper
{
public:
    void update(
        const NormalizedChannels& channels,
        ChannelState& state
    ) const;

private:
    static bool channelIsHigh(uint16_t value);
};