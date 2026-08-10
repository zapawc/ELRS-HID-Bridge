#pragma once

#include <stdint.h>

enum class ChannelIndex : uint8_t
{
    CH1 = 0,
    CH2,
    CH3,
    CH4,
    CH5,
    CH6,
    CH7,
    CH8,
    CH9,
    CH10,
    CH11,
    CH12,
    CH13,
    CH14,
    CH15,
    CH16
};

struct RawChannels
{
    static constexpr uint8_t CHANNEL_COUNT = 16;

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