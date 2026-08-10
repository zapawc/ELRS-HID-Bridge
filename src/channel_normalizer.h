#pragma once

#include <stdint.h>

#include "normalized_channels.h"
#include "raw_channels.h"

class ChannelNormalizer
{
public:
    void update(
        const RawChannels& raw,
        NormalizedChannels& normalized
    ) const;

private:
    static uint16_t normalize(uint16_t rawValue);
};