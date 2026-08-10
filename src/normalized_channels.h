#pragma once

#include <stdint.h>

#include "channel_index.h"

struct NormalizedChannels
{
    static constexpr uint8_t CHANNEL_COUNT = 16;

    static constexpr uint16_t MIN = 0;
    static constexpr uint16_t MID = 32768;
    static constexpr uint16_t MAX = 65535;

    uint16_t channel[CHANNEL_COUNT] = {};

    uint16_t get(ChannelIndex index) const
    {
        return channel[static_cast<uint8_t>(index)];
    }

    void set(ChannelIndex index, uint16_t value)
    {
        channel[static_cast<uint8_t>(index)] = value;
    }
};