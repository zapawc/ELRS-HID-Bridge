#pragma once

#include <stdint.h>

struct RawChannels
{
    static constexpr uint8_t CHANNEL_COUNT = 16;

    uint16_t channel[CHANNEL_COUNT] = {};
};