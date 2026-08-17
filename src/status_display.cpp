#include "status_display.h"


StatusDisplay::StatusDisplay(
    StatusLed& led
)
    : led(led)
{
}


void StatusDisplay::reset()
{
    mode =
        DisplayMode::Normal;

    maintenancePresentation =
        MaintenancePresentation::None;

    diagnosticStartMs = 0;
    maintenanceBlinkLastToggleMs = 0;

    maintenanceBlinkInitialized = false;
    maintenanceBlinkOn = false;
    normalStatusRendered = false;

    lastNormalStatus =
        SystemStatus::Startup;
}


void StatusDisplay::showFatalError()
{
    mode =
        DisplayMode::FatalError;

    maintenancePresentation =
        MaintenancePresentation::None;

    led.setStatus(
        SystemStatus::Error
    );
}


void StatusDisplay::showLinkQuality(
    uint8_t linkQuality,
    uint32_t nowMs
)
{
    if (
        mode ==
        DisplayMode::FatalError
    )
    {
        return;
    }

    mode =
        DisplayMode::Diagnostic;

    maintenancePresentation =
        MaintenancePresentation::None;

    diagnosticStartMs =
        nowMs;

    led.showLinkQuality(
        linkQuality
    );
}


void StatusDisplay::showDiagnosticUnavailable(
    uint32_t nowMs
)
{
    if (
        mode ==
        DisplayMode::FatalError
    )
    {
        return;
    }

    mode =
        DisplayMode::Diagnostic;

    maintenancePresentation =
        MaintenancePresentation::None;

    diagnosticStartMs =
        nowMs;

    led.showDiagnosticUnavailable();
}


void StatusDisplay::showMaintenanceBind()
{
    if (
        mode ==
        DisplayMode::FatalError
    )
    {
        return;
    }

    mode =
        DisplayMode::Maintenance;

    maintenancePresentation =
        MaintenancePresentation::Bind;

    maintenanceBlinkInitialized = false;
    maintenanceBlinkOn = false;

    led.showMaintenanceBind();
}


void StatusDisplay::showMaintenanceWifi()
{
    if (
        mode ==
        DisplayMode::FatalError
    )
    {
        return;
    }

    mode =
        DisplayMode::Maintenance;

    maintenancePresentation =
        MaintenancePresentation::Wifi;

    maintenanceBlinkInitialized = false;
    maintenanceBlinkOn = false;

    led.showMaintenanceWifi();
}


void StatusDisplay::showMaintenanceReset()
{
    if (
        mode ==
        DisplayMode::FatalError
    )
    {
        return;
    }

    mode =
        DisplayMode::Maintenance;

    maintenancePresentation =
        MaintenancePresentation::ReceiverReset;

    // Start visibly red immediately. The main update loop establishes the
    // first toggle timestamp without blocking any CRSF/HID work.
    maintenanceBlinkInitialized = false;
    maintenanceBlinkOn = true;

    led.showMaintenanceReset();
}


void StatusDisplay::showMaintenanceCancel()
{
    // Compatibility wrapper for the unchanged M1 main.cpp dispatch.
    showMaintenanceReset();
}


void StatusDisplay::clearMaintenance()
{
    if (
        mode ==
        DisplayMode::FatalError
    )
    {
        return;
    }

    if (
        mode ==
        DisplayMode::Maintenance
    )
    {
        mode =
            DisplayMode::Normal;

        maintenancePresentation =
            MaintenancePresentation::None;

        maintenanceBlinkInitialized = false;
        maintenanceBlinkOn = false;

        normalStatusRendered =
            false;
    }
}


void StatusDisplay::update(
    uint32_t nowMs,
    const BridgeState& state
)
{
    // -------------------------------------------------------------------------
    // Fatal error -- highest priority.
    // -------------------------------------------------------------------------
    if (
        mode ==
        DisplayMode::FatalError
    )
    {
        return;
    }

    // -------------------------------------------------------------------------
    // Maintenance selection
    //
    // Maintenance presentation has priority over normal runtime status. The
    // reset/recovery warning blink is advanced here without delay().
    // -------------------------------------------------------------------------
    if (
        mode ==
        DisplayMode::Maintenance
    )
    {
        if (
            maintenancePresentation ==
            MaintenancePresentation::ReceiverReset
        )
        {
            updateMaintenanceBlink(
                nowMs
            );
        }

        return;
    }

    // -------------------------------------------------------------------------
    // Receiver loss interrupts a temporary diagnostic display.
    // -------------------------------------------------------------------------
    if (state.isReceiverLost())
    {
        if (
            mode ==
            DisplayMode::Diagnostic
        )
        {
            mode =
                DisplayMode::Normal;

            normalStatusRendered =
                false;
        }

        renderNormal(
            state
        );

        return;
    }

    // -------------------------------------------------------------------------
    // Temporary diagnostic
    // -------------------------------------------------------------------------
    if (
        mode ==
        DisplayMode::Diagnostic
    )
    {
        if (
            (nowMs - diagnosticStartMs) <
            DIAGNOSTIC_DISPLAY_MS
        )
        {
            return;
        }

        mode =
            DisplayMode::Normal;

        normalStatusRendered =
            false;
    }

    // -------------------------------------------------------------------------
    // Normal runtime state
    // -------------------------------------------------------------------------
    renderNormal(
        state
    );
}


SystemStatus
StatusDisplay::normalStatusFor(
    const BridgeState& state
) const
{
    if (state.isReceiverLost())
    {
        return
            SystemStatus::ReceiverLost;
    }

    if (state.hasRcFrames())
    {
        return
            SystemStatus::ReceiverFrames;
    }

    if (state.hasReceiverBytes())
    {
        return
            SystemStatus::ReceiverBytes;
    }

    return
        SystemStatus::Ready;
}


void StatusDisplay::renderNormal(
    const BridgeState& state
)
{
    const SystemStatus desiredStatus =
        normalStatusFor(
            state
        );

    if (
        normalStatusRendered &&
        desiredStatus ==
            lastNormalStatus
    )
    {
        return;
    }

    led.setStatus(
        desiredStatus
    );

    lastNormalStatus =
        desiredStatus;

    normalStatusRendered =
        true;
}


void StatusDisplay::updateMaintenanceBlink(
    uint32_t nowMs
)
{
    if (!maintenanceBlinkInitialized)
    {
        maintenanceBlinkLastToggleMs =
            nowMs;

        maintenanceBlinkInitialized =
            true;

        return;
    }

    if (
        (nowMs - maintenanceBlinkLastToggleMs) <
        MAINTENANCE_RESET_BLINK_MS
    )
    {
        return;
    }

    maintenanceBlinkLastToggleMs =
        nowMs;

    maintenanceBlinkOn =
        !maintenanceBlinkOn;

    if (maintenanceBlinkOn)
    {
        led.showMaintenanceReset();
    }
    else
    {
        led.showMaintenanceOff();
    }
}
