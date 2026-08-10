#pragma once

#include <stdint.h>

#include "raw_channels.h"

class RawChannelTest
{
public:
    void begin();
    void update(RawChannels& channels);

private:
    uint32_t startTimeMs = 0;
};