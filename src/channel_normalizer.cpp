#include "channel_normalizer.h"

namespace
{
    constexpr uint16_t INPUT_MIN = 172;
    constexpr uint16_t INPUT_MAX = 1811;
}

uint16_t ChannelNormalizer::normalize(uint16_t rawValue)
{
    if (rawValue <= INPUT_MIN)
    {
        return NormalizedChannels::MIN;
    }

    if (rawValue >= INPUT_MAX)
    {
        return NormalizedChannels::MAX;
    }

    const uint32_t inputRange =
        INPUT_MAX - INPUT_MIN;

    const uint32_t position =
        rawValue - INPUT_MIN;

    return static_cast<uint16_t>(
        (position * 65535UL) / inputRange
    );
}

void ChannelNormalizer::update(
    const RawChannels& raw,
    NormalizedChannels& normalized
) const
{
    for (
        uint8_t i = 0;
        i < RawChannels::CHANNEL_COUNT;
        ++i
    )
    {
        normalized.channel[i] =
            normalize(raw.channel[i]);
    }
}