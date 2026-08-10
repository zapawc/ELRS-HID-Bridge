#include <Arduino.h>

#include "test_generator.h"

namespace
{
    uint16_t triangleWave(uint32_t elapsedMs, uint32_t periodMs)
    {
        const uint32_t phase = elapsedMs % periodMs;
        const uint32_t halfPeriod = periodMs / 2;

        if (phase < halfPeriod)
        {
            return static_cast<uint16_t>(
                (phase * 65535UL) / halfPeriod
            );
        }

        return static_cast<uint16_t>(
            ((periodMs - phase) * 65535UL) / halfPeriod
        );
    }

    uint32_t sequentialButton(uint32_t elapsedMs)
    {
        constexpr uint32_t BUTTON_HOLD_MS = 250;
        constexpr uint32_t BUTTON_COUNT = 32;

        const uint32_t buttonIndex =
            (elapsedMs / BUTTON_HOLD_MS) % BUTTON_COUNT;

        return (1UL << buttonIndex);
    }
}

void TestGenerator::begin()
{
    startTimeMs = millis();
}

void TestGenerator::update(ChannelState& state)
{
    const uint32_t elapsedMs = millis() - startTimeMs;

    // Different periods make it easy to distinguish the four controls.

    // Roll: 4-second sweep.
    state.roll = triangleWave(elapsedMs, 4000);

    // Pitch: 6-second sweep.
    state.pitch = triangleWave(elapsedMs, 6000);

    // Throttle: 8-second sweep.
    state.throttle = triangleWave(elapsedMs, 8000);

    // Yaw: 10-second sweep.
    state.yaw = triangleWave(elapsedMs, 10000);

    // Walk through buttons 1-32.
    state.buttons = sequentialButton(elapsedMs);
}