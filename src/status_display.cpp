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

    diagnosticStartMs = 0;

    normalStatusRendered = false;

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


void StatusDisplay::update(
    uint32_t nowMs,
    const BridgeState& state
)
{
    // -------------------------------------------------------------------------
    // Fatal error
    //
    // Nothing may overwrite a fatal startup/self-test failure.
    // -------------------------------------------------------------------------

    if (
        mode ==
        DisplayMode::FatalError
    )
    {
        return;
    }


    // -------------------------------------------------------------------------
    // Receiver loss
    //
    // Preserve existing behavior:
    //
    // RC failsafe/loss immediately cancels any temporary diagnostic
    // indication so that the purple receiver-loss state is visible.
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


        // Force restoration of the current normal color even if it
        // matches the normal status that existed before diagnostics.
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


    // Avoid rewriting the NeoPixel on every pass through loop().
    //
    // Only update the physical LED when the desired normal state
    // actually changes, or when returning from another display mode.
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