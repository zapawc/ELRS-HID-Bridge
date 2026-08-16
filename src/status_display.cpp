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


    diagnosticStartMs =
        0;


    normalStatusRendered =
        false;


    lastNormalStatus =
        SystemStatus::Startup;
}


void StatusDisplay::showFatalError()
{
    mode =
        DisplayMode::FatalError;


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


    led.showMaintenanceWifi();
}


void StatusDisplay::showMaintenanceCancel()
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


    led.showMaintenanceCancel();
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
    // Fatal error
    //
    // Highest priority.
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
    // While the user deliberately holds the BOOT button beyond a maintenance
    // threshold, the selected action remains visible.
    //
    // This intentionally has priority over normal runtime status.
    // -------------------------------------------------------------------------

    if (
        mode ==
        DisplayMode::Maintenance
    )
    {
        return;
    }


    // -------------------------------------------------------------------------
    // Receiver loss
    //
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