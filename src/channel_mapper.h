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
    enum class SwitchPosition
    {
        Low,
        Center,
        High
    };

    static SwitchPosition decodeThreePosition(
        uint16_t value
    );

    static bool decodeTwoPosition(
        uint16_t value
    );

    static void setButton(
        ChannelState& state,
        uint8_t buttonNumber
    );
};