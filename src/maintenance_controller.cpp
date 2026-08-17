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
    // Continuously evaluate elapsed duration so selection changes occur while
    // the user is holding the button. No action executes while held.
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
    //
    // Release is the only event that can produce a maintenance action.
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
        durationMs <
        MAINTENANCE_START_MS
    )
    {
        return
            MaintenanceSelection::None;
    }

    const uint32_t slot =
        (
            (
                durationMs -
                MAINTENANCE_START_MS
            ) /
            MAINTENANCE_STEP_MS
        ) %
        MAINTENANCE_SLOT_COUNT;

    switch (slot)
    {
        case 0:
        {
            return
                MaintenanceSelection::Bind;
        }

        case 1:
        {
            return
                MaintenanceSelection::Wifi;
        }

        case 2:
        {
            return
                MaintenanceSelection::ReceiverReset;
        }

        case 3:
        default:
        {
            // No Action / Cancel. Returning None causes StatusDisplay to resume
            // the current operational indication while the button remains held.
            return
                MaintenanceSelection::None;
        }
    }
}


MaintenanceAction
MaintenanceController::actionForDuration(
    uint32_t durationMs
) const
{
    if (
        durationMs <
        MAINTENANCE_START_MS
    )
    {
        return
            MaintenanceAction::Diagnostic;
    }

    switch (
        selectionForDuration(
            durationMs
        )
    )
    {
        case MaintenanceSelection::Bind:
        {
            return
                MaintenanceAction::BindRequested;
        }

        case MaintenanceSelection::Wifi:
        {
            return
                MaintenanceAction::WifiRequested;
        }

        case MaintenanceSelection::ReceiverReset:
        {
            // M1 reserves and displays this slot but intentionally does not
            // create or transmit a receiver reset command. A legitimate
            // ExpressLRS recovery mechanism must be verified first.
            return
                MaintenanceAction::None;
        }

        case MaintenanceSelection::None:
        default:
        {
            // No Action / Cancel explicitly consumes the release without
            // producing a maintenance action.
            return
                MaintenanceAction::None;
        }
    }
}
