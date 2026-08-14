#pragma once

#include <stdint.h>

enum class BootButtonEvent
{
    None,
    ShortPress,
    LongPress,
    VeryLongPress
};

class BootButton
{
public:
    void begin();

    // Call frequently from loop().
    // An event is returned after the button is released.
    BootButtonEvent update();

private:
    bool lastRawPressed = false;
    bool stablePressed = false;

    uint32_t lastChangeMs = 0;
    uint32_t pressStartMs = 0;
};