#pragma once

#include <stdint.h>

#include "bridge_state.h"
#include "status_led.h"


class StatusDisplay
{
public:
    explicit StatusDisplay(
        StatusLed& led
    );


    // Reset display arbitration state.
    //
    // StatusLed::begin() remains responsible for physically
    // initializing the onboard NeoPixel and showing Startup.
    void reset();


    // Enter a persistent fatal-error display.
    //
    // Once entered, normal runtime state and temporary diagnostics
    // cannot overwrite the error indication.
    void showFatalError();


    // Temporarily display current Link Quality.
    void showLinkQuality(
        uint8_t linkQuality,
        uint32_t nowMs
    );


    // Temporarily acknowledge a diagnostic request when no current
    // Link Statistics are available.
    void showDiagnosticUnavailable(
        uint32_t nowMs
    );


    // Reconcile the physical LED with current bridge state and any
    // active temporary display.
    //
    // Call frequently from loop().
    void update(
        uint32_t nowMs,
        const BridgeState& state
    );


private:
    enum class DisplayMode
    {
        Normal,
        Diagnostic,
        FatalError
    };


    static constexpr uint32_t
        DIAGNOSTIC_DISPLAY_MS = 3000;


    StatusLed& led;


    DisplayMode mode =
        DisplayMode::Normal;


    uint32_t diagnosticStartMs = 0;


    bool normalStatusRendered = false;

    SystemStatus lastNormalStatus =
        SystemStatus::Startup;


    SystemStatus normalStatusFor(
        const BridgeState& state
    ) const;


    void renderNormal(
        const BridgeState& state
    );
};