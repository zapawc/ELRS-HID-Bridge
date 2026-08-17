#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "status_led.h"


namespace
{
    constexpr uint8_t NEOPIXEL_DATA_PIN = 12;
    constexpr uint8_t NEOPIXEL_POWER_PIN = 11;
    constexpr uint8_t PIXEL_COUNT = 1;

    Adafruit_NeoPixel pixel(
        PIXEL_COUNT,
        NEOPIXEL_DATA_PIN,
        NEO_GRB + NEO_KHZ800
    );
}


uint8_t StatusLed::percentToNeoPixelBrightness(
    uint8_t brightnessPercent
)
{
    if (brightnessPercent > 100)
    {
        brightnessPercent = 100;
    }

    return
        static_cast<uint8_t>(
            (
                static_cast<uint16_t>(
                    brightnessPercent
                ) *
                255u +
                50u
            ) /
            100u
        );
}


void StatusLed::begin(
    uint8_t brightnessPercent
)
{
    pinMode(
        NEOPIXEL_POWER_PIN,
        OUTPUT
    );

    digitalWrite(
        NEOPIXEL_POWER_PIN,
        HIGH
    );

    delay(1);

    pixel.begin();
    pixel.setBrightness(
        percentToNeoPixelBrightness(
            brightnessPercent
        )
    );

    pixel.clear();
    pixel.show();

    setStatus(
        SystemStatus::Startup
    );
}


void StatusLed::setBrightnessPercent(
    uint8_t brightnessPercent
)
{
    pixel.setBrightness(
        percentToNeoPixelBrightness(
            brightnessPercent
        )
    );

    // Re-apply the unscaled logical color after every brightness change.
    // This avoids cumulative NeoPixel buffer rescaling and makes 0% -> nonzero
    // transitions deterministic.
    applyCurrentColor();
}


void StatusLed::setStatus(
    SystemStatus status
)
{
    switch (status)
    {
        case SystemStatus::Startup:
        {
            // White.
            setColor(255, 255, 255);
            break;
        }

        case SystemStatus::Ready:
        {
            // Blue: firmware healthy, waiting for RC activity.
            setColor(0, 0, 255);
            break;
        }

        case SystemStatus::ReceiverBytes:
        {
            // Yellow: UART bytes exist, but no valid RC frame yet.
            setColor(255, 180, 0);
            break;
        }

        case SystemStatus::ReceiverFrames:
        {
            // Pure green: normal CRSF -> HID operation.
            setColor(0, 255, 0);
            break;
        }

        case SystemStatus::ReceiverLost:
        {
            // Purple: RC frame timeout / failsafe.
            setColor(180, 0, 255);
            break;
        }

        case SystemStatus::Error:
        {
            // Red: startup/self-test failure.
            setColor(255, 0, 0);
            break;
        }
    }
}


void StatusLed::showLinkQuality(
    uint8_t linkQuality
)
{
    if (linkQuality >= 90)
    {
        // Lime green, deliberately distinct from normal-operation pure green.
        setColor(120, 255, 0);
        return;
    }

    if (linkQuality >= 70)
    {
        // Yellow.
        setColor(255, 180, 0);
        return;
    }

    // Orange/red: poor link quality.
    setColor(255, 45, 0);
}


void StatusLed::showDiagnosticUnavailable()
{
    // White indicates diagnostic mode was entered but no current Link Quality
    // was available.
    setColor(255, 255, 255);
}


void StatusLed::showMaintenanceBind()
{
    // Blue: Bind maintenance action currently selected.
    setColor(0, 0, 255);
}


void StatusLed::showMaintenanceWifi()
{
    // White: Wi-Fi maintenance action currently selected.
    setColor(255, 255, 255);
}


void StatusLed::showMaintenanceReset()
{
    // Red: Receiver Reset/Recovery maintenance action currently selected.
    setColor(255, 0, 0);
}


void StatusLed::showMaintenanceOff()
{
    // Used only as the off phase of a non-blocking maintenance blink.
    setColor(0, 0, 0);
}


void StatusLed::showMaintenanceCancel()
{
    // Compatibility wrapper for the current main.cpp selection switch.
    // M1 changes the former Cancel slot into Receiver Reset/Recovery.
    showMaintenanceReset();
}


void StatusLed::applyCurrentColor()
{
    pixel.setPixelColor(
        0,
        pixel.Color(
            currentRed,
            currentGreen,
            currentBlue
        )
    );

    pixel.show();
}


void StatusLed::setColor(
    unsigned char red,
    unsigned char green,
    unsigned char blue
)
{
    currentRed = red;
    currentGreen = green;
    currentBlue = blue;

    applyCurrentColor();
}
