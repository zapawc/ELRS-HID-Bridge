#pragma once

#include <stdint.h>


struct BootButtonState
{
    // Current debounced physical state.
    bool pressed = false;


    // True for one update() call when the debounced button
    // transitions from released to pressed.
    bool pressedEvent = false;


    // True for one update() call when the debounced button
    // transitions from pressed to released.
    bool releasedEvent = false;


    // While pressed:
    //
    //     elapsed time since the debounced press began.
    //
    // On the update() call that reports releasedEvent:
    //
    //     duration of the completed press.
    //
    // Otherwise:
    //
    //     zero.
    uint32_t durationMs = 0;
};


class BootButton
{
public:
    void begin();


    // Read and debounce the physical QT Py BOOT button.
    //
    // This class deliberately assigns no semantic meaning to
    // press duration. It reports only physical button state,
    // edges, and elapsed press time.
    BootButtonState update();


private:
    bool lastRawPressed = false;
    bool stablePressed = false;

    uint32_t lastChangeMs = 0;
    uint32_t pressStartMs = 0;
};