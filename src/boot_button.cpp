#include <Arduino.h>

#include "boot_button.h"

namespace
{
    // QT Py RP2040 onboard BOOT button.
    //
    // We currently build against the generic Pico target, so use
    // the actual RP2040 GPIO number rather than a board alias.
    constexpr uint8_t BOOT_BUTTON_PIN = 21;

    constexpr uint32_t DEBOUNCE_MS = 30;

    constexpr uint32_t LONG_PRESS_MS = 3000;
    constexpr uint32_t VERY_LONG_PRESS_MS = 10000;
}


void BootButton::begin()
{
    pinMode(
        BOOT_BUTTON_PIN,
        INPUT_PULLUP
    );

    lastRawPressed =
        digitalRead(BOOT_BUTTON_PIN) == LOW;

    stablePressed =
        lastRawPressed;

    lastChangeMs =
        millis();

    if (stablePressed)
    {
        pressStartMs =
            millis();
    }
}


BootButtonEvent BootButton::update()
{
    const uint32_t now =
        millis();

    const bool rawPressed =
        digitalRead(BOOT_BUTTON_PIN) == LOW;

    // Raw state changed. Start the debounce timer again.
    if (rawPressed != lastRawPressed)
    {
        lastRawPressed =
            rawPressed;

        lastChangeMs =
            now;
    }

    // Wait until the new state has remained stable long enough.
    if (
        rawPressed != stablePressed &&
        (now - lastChangeMs) >= DEBOUNCE_MS
    )
    {
        stablePressed =
            rawPressed;

        if (stablePressed)
        {
            // Button has just become pressed.
            pressStartMs =
                now;

            return BootButtonEvent::None;
        }

        // Button has just been released.
        const uint32_t pressDuration =
            now - pressStartMs;

        if (pressDuration >= VERY_LONG_PRESS_MS)
        {
            return BootButtonEvent::VeryLongPress;
        }

        if (pressDuration >= LONG_PRESS_MS)
        {
            return BootButtonEvent::LongPress;
        }

        return BootButtonEvent::ShortPress;
    }

    return BootButtonEvent::None;
}