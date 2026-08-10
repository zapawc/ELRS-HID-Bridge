#include <Arduino.h>

#include "raw_channel_test.h"

namespace
{
    constexpr uint16_t CRSF_MIN = 172;
    constexpr uint16_t CRSF_MAX = 1811;

    uint16_t triangleWave(
        uint32_t elapsedMs,
        uint32_t periodMs
    )
    {
        const uint32_t phase =
            elapsedMs % periodMs;

        const uint32_t halfPeriod =
            periodMs / 2;

        const uint32_t range =
            CRSF_MAX - CRSF_MIN;

        uint32_t scaled;

        if (phase < halfPeriod)
        {
            scaled =
                (phase * range) / halfPeriod;
        }
        else
        {
            scaled =
                ((periodMs - phase) * range) /
                halfPeriod;
        }

        return static_cast<uint16_t>(
            CRSF_MIN + scaled
        );
    }
}

void RawChannelTest::begin()
{
    startTimeMs = millis();
}

void RawChannelTest::update(RawChannels& channels)
{
    const uint32_t elapsedMs =
        millis() - startTimeMs;

    // Primary controls
    channels.set(
        ChannelIndex::CH1,
        triangleWave(elapsedMs, 4000)
    );

    channels.set(
        ChannelIndex::CH2,
        triangleWave(elapsedMs, 6000)
    );

    channels.set(
        ChannelIndex::CH3,
        triangleWave(elapsedMs, 8000)
    );

    channels.set(
        ChannelIndex::CH4,
        triangleWave(elapsedMs, 10000)
    );

    // AUX channels CH5-CH16.
    //
    // One channel is high at a time.
    constexpr uint32_t AUX_HOLD_MS = 500;
    constexpr uint8_t AUX_COUNT = 12;

    const uint8_t activeAux =
        (elapsedMs / AUX_HOLD_MS) % AUX_COUNT;

    for (uint8_t i = 0; i < AUX_COUNT; ++i)
    {
        const ChannelIndex index =
            static_cast<ChannelIndex>(4 + i);

        channels.set(
            index,
            i == activeAux
                ? CRSF_MAX
                : CRSF_MIN
        );
    }
}