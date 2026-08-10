#pragma once

#include <stdint.h>

#include "channel_state.h"

class TestGenerator
{
public:
    void begin();
    void update(ChannelState& state);

private:
    uint32_t startTimeMs = 0;
};