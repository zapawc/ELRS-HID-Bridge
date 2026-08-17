#pragma once

#include <stdint.h>
#include "boot_button.h"


enum class MaintenanceSelection
{
    None,
    Bind,
    Wifi,
    ReceiverReset,

    // Compatibility alias for the current main.cpp display dispatch.
    // M1 keeps main.cpp unchanged; its legacy Cancel case now presents the
    // reserved Receiver Reset/Recovery slot. This alias can be removed when
    // command dispatch is added in a later checkpoint.
    Cancel = ReceiverReset
};


enum class MaintenanceAction
{
    None,
    Diagnostic,
    BindRequested,
    WifiRequested
};


struct MaintenanceUpdate
{
    // Current selection while the button is held.
    MaintenanceSelection selection =
        MaintenanceSelection::None;

    // True when selection changed during this update.
    // This includes returning to None on release and entering the No Action
    // interval while the button remains held.
    bool selectionChanged = false;

    // Action produced only by releasing the button.
    MaintenanceAction action =
        MaintenanceAction::None;
};


class MaintenanceController
{
public:
    void reset();

    MaintenanceUpdate update(
        const BootButtonState& button
    );

private:
    // Short presses below this threshold retain the existing diagnostic action.
    static constexpr uint32_t
        MAINTENANCE_START_MS = 2000;

    // Each maintenance selection occupies one two-second interval.
    static constexpr uint32_t
        MAINTENANCE_STEP_MS = 2000;

    static constexpr uint32_t
        MAINTENANCE_SLOT_COUNT = 4;

    MaintenanceSelection currentSelection =
        MaintenanceSelection::None;

    MaintenanceSelection selectionForDuration(
        uint32_t durationMs
    ) const;

    MaintenanceAction actionForDuration(
        uint32_t durationMs
    ) const;
};
