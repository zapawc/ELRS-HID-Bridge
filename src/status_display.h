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
    void reset();

    // Persistent fatal startup/self-test failure.
    void showFatalError();

    // Temporary diagnostic indications.
    void showLinkQuality(
        uint8_t linkQuality,
        uint32_t nowMs
    );

    void showDiagnosticUnavailable(
        uint32_t nowMs
    );

    // Maintenance-selection indications. These remain active until
    // clearMaintenance() is called.
    void showMaintenanceBind();
    void showMaintenanceWifi();
    void showMaintenanceReset();

    // Compatibility wrapper for the current main.cpp dispatch. During M1 the
    // former Cancel display hook selects Receiver Reset/Recovery.
    void showMaintenanceCancel();

    void clearMaintenance();

    // Reconcile physical LED output with current bridge state.
    void update(
        uint32_t nowMs,
        const BridgeState& state
    );

private:
    enum class DisplayMode
    {
        Normal,
        Diagnostic,
        Maintenance,
        FatalError
    };

    enum class MaintenancePresentation
    {
        None,
        Bind,
        Wifi,
        ReceiverReset
    };

    static constexpr uint32_t
        DIAGNOSTIC_DISPLAY_MS = 3000;

    static constexpr uint32_t
        MAINTENANCE_RESET_BLINK_MS = 300;

    StatusLed& led;

    DisplayMode mode =
        DisplayMode::Normal;

    MaintenancePresentation maintenancePresentation =
        MaintenancePresentation::None;

    uint32_t diagnosticStartMs = 0;
    uint32_t maintenanceBlinkLastToggleMs = 0;

    bool maintenanceBlinkInitialized = false;
    bool maintenanceBlinkOn = false;
    bool normalStatusRendered = false;

    SystemStatus lastNormalStatus =
        SystemStatus::Startup;

    SystemStatus normalStatusFor(
        const BridgeState& state
    ) const;

    void renderNormal(
        const BridgeState& state
    );

    void updateMaintenanceBlink(
        uint32_t nowMs
    );
};
