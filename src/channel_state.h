#pragma once

#include <stdint.h>

struct ChannelState
{
    // Primary flight controls
    uint16_t roll = 32768;
    uint16_t pitch = 32768;

    // Throttle starts at minimum
    uint16_t throttle = 0;

    // Rudder starts centered
    uint16_t yaw = 32768;

    // 32 HID buttons
    uint32_t buttons = 0;
};