#include "maintenance_controller.h"


void MaintenanceController::reset()
{
    currentSelection =
        MaintenanceSelection::None;
}


MaintenanceUpdate MaintenanceController::update(
    const BootButtonState& button
)
{
    MaintenanceUpdate result;


    // -------------------------------------------------------------------------
    // Button currently held
    //
    // Continuously evaluate elapsed duration so selection changes occur
    // while the user is holding the button rather than only after release.
    // -------------------------------------------------------------------------

    if (button.pressed)
    {
        const MaintenanceSelection desiredSelection =
            selectionForDuration(
                button.durationMs
            );


        if (
            desiredSelection !=
            currentSelection
        )
        {
            currentSelection =
                desiredSelection;


            result.selectionChanged =
                true;
        }


        result.selection =
            currentSelection;


        return result;
    }


    // -------------------------------------------------------------------------
    // Button released
    // -------------------------------------------------------------------------

    if (button.releasedEvent)
    {
        result.action =
            actionForDuration(
                button.durationMs
            );


        if (
            currentSelection !=
            MaintenanceSelection::None
        )
        {
            result.selectionChanged =
                true;
        }


        currentSelection =
            MaintenanceSelection::None;


        result.selection =
            MaintenanceSelection::None;


        return result;
    }


    // -------------------------------------------------------------------------
    // Idle
    // -------------------------------------------------------------------------

    result.selection =
        currentSelection;


    return result;
}


MaintenanceSelection
MaintenanceController::selectionForDuration(
    uint32_t durationMs
) const
{
    if (
        durationMs >=
        CANCEL_THRESHOLD_MS
    )
    {
        return
            MaintenanceSelection::Cancel;
    }


    if (
        durationMs >=
        WIFI_THRESHOLD_MS
    )
    {
        return
            MaintenanceSelection::Wifi;
    }


    if (
        durationMs >=
        BIND_THRESHOLD_MS
    )
    {
        return
            MaintenanceSelection::Bind;
    }


    return
        MaintenanceSelection::None;
}


MaintenanceAction
MaintenanceController::actionForDuration(
    uint32_t durationMs
) const
{
    // Cancel explicitly consumes the release without producing
    // a maintenance action.

    if (
        durationMs >=
        CANCEL_THRESHOLD_MS
    )
    {
        return
            MaintenanceAction::None;
    }


    if (
        durationMs >=
        WIFI_THRESHOLD_MS
    )
    {
        return
            MaintenanceAction::WifiRequested;
    }


    if (
        durationMs >=
        BIND_THRESHOLD_MS
    )
    {
        return
            MaintenanceAction::BindRequested;
    }


    return
        MaintenanceAction::Diagnostic;
}