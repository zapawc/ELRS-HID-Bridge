#pragma once

#include <stdint.h>


enum class SystemStatus
{
    Startup,
    Ready,
    ReceiverBytes,
    ReceiverFrames,
    ReceiverLost,
    Error
};


class StatusLed
{
public:
    void begin();


    void setStatus(
        SystemStatus status
    );


    // Temporarily display Link Quality.
    void showLinkQuality(
        uint8_t linkQuality
    );


    // Diagnostic request when valid statistics
    // are not currently available.
    void showDiagnosticUnavailable();


    // Maintenance-selection indications.
    void showMaintenanceBind();
    void showMaintenanceWifi();
    void showMaintenanceCancel();


private:
    void setColor(
        unsigned char red,
        unsigned char green,
        unsigned char blue
    );
};