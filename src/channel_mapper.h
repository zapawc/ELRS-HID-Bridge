#pragma once

#include <stdint.h>

#include "bridge_configuration.h"
#include "channel_state.h"
#include "normalized_channels.h"


class ChannelMapper
{
public:
    explicit ChannelMapper(
        const BridgeConfiguration& configuration
    );


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


    const BridgeConfiguration& configuration;


    uint16_t mappedValue(
        const NormalizedChannels& channels,
        const AxisMapping& mapping
    ) const;


    SwitchPosition decodeThreePosition(
        uint16_t value
    ) const;


    bool decodeTwoPosition(
        uint16_t value
    ) const;


    static void setButton(
        ChannelState& state,
        uint8_t buttonNumber
    );
};