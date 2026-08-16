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
    void begin(
        uint8_t brightnessPercent
    );


    void setBrightnessPercent(
        uint8_t brightnessPercent
    );


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
    static uint8_t percentToNeoPixelBrightness(
        uint8_t brightnessPercent
    );


    void applyCurrentColor();


    void setColor(
        unsigned char red,
        unsigned char green,
        unsigned char blue
    );


    // Keep the unscaled logical color locally. Adafruit_NeoPixel brightness
    // changes rescale its internal pixel buffer; retaining the raw color lets
    // brightness move cleanly through 0% without accumulating scale error.
    unsigned char currentRed = 0;
    unsigned char currentGreen = 0;
    unsigned char currentBlue = 0;
};
