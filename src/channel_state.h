#pragma once

#include <stdint.h>

struct ChannelState
{
    // Primary flight controls
    uint16_t roll = 32768;
    uint16_t pitch = 32768;
    uint16_t throttle = 0;
    uint16_t yaw = 32768;

    // Additional proportional controls
    uint16_t auxAnalog1 = 32768;
    uint16_t auxAnalog2 = 32768;
    uint16_t auxAnalog3 = 32768;
    uint16_t auxAnalog4 = 32768;

    // HID buttons 1-32
    uint32_t buttons = 0;
};