#pragma once

#include <stdint.h>

#include "boot_button.h"


enum class MaintenanceSelection
{
    None,
    Bind,
    Wifi,
    Cancel
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
    //
    // This includes returning to None on release.
    bool selectionChanged = false;


    // Action produced by releasing the button.
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
    // -------------------------------------------------------------------------
    // Maintenance UI thresholds
    //
    // These belong here rather than in BootButton because they describe
    // maintenance-interface semantics, not physical button behavior.
    // -------------------------------------------------------------------------

    static constexpr uint32_t
        BIND_THRESHOLD_MS = 2000;


    static constexpr uint32_t
        WIFI_THRESHOLD_MS = 5000;


    static constexpr uint32_t
        CANCEL_THRESHOLD_MS = 10000;


    MaintenanceSelection currentSelection =
        MaintenanceSelection::None;


    MaintenanceSelection selectionForDuration(
        uint32_t durationMs
    ) const;


    MaintenanceAction actionForDuration(
        uint32_t durationMs
    ) const;
};