#include <Arduino.h>

#include "boot_button.h"


namespace
{
    // QT Py RP2040 onboard BOOT button.
    //
    // The project currently builds against the generic Pico target,
    // so use the actual RP2040 GPIO number.
    constexpr uint8_t BOOT_BUTTON_PIN = 21;


    constexpr uint32_t DEBOUNCE_MS = 30;
}


void BootButton::begin()
{
    pinMode(
        BOOT_BUTTON_PIN,
        INPUT_PULLUP
    );


    lastRawPressed =
        digitalRead(
            BOOT_BUTTON_PIN
        ) == LOW;


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


BootButtonState BootButton::update()
{
    const uint32_t now =
        millis();


    BootButtonState state;


    const bool rawPressed =
        digitalRead(
            BOOT_BUTTON_PIN
        ) == LOW;


    // -------------------------------------------------------------------------
    // Raw-state change
    //
    // Restart the debounce interval whenever the electrical state changes.
    // -------------------------------------------------------------------------

    if (
        rawPressed !=
        lastRawPressed
    )
    {
        lastRawPressed =
            rawPressed;


        lastChangeMs =
            now;
    }


    // -------------------------------------------------------------------------
    // Debounced transition
    // -------------------------------------------------------------------------

    if (
        rawPressed !=
            stablePressed &&
        (now - lastChangeMs) >=
            DEBOUNCE_MS
    )
    {
        stablePressed =
            rawPressed;


        if (stablePressed)
        {
            // Button has just become stably pressed.

            pressStartMs =
                now;


            state.pressed =
                true;

            state.pressedEvent =
                true;

            state.durationMs =
                0;


            return state;
        }


        // Button has just become stably released.

        state.pressed =
            false;

        state.releasedEvent =
            true;

        state.durationMs =
            now - pressStartMs;


        return state;
    }


    // -------------------------------------------------------------------------
    // Stable current state
    // -------------------------------------------------------------------------

    state.pressed =
        stablePressed;


    if (stablePressed)
    {
        state.durationMs =
            now - pressStartMs;
    }


    return state;
}