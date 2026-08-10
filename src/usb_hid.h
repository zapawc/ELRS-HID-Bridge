#pragma once

#include "channel_state.h"

class UsbHid
{
public:
    void begin();
    void update(const ChannelState& state);
};