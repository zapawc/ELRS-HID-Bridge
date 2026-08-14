#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "status_led.h"

namespace
{
    constexpr uint8_t NEOPIXEL_DATA_PIN = 12;
    constexpr uint8_t NEOPIXEL_POWER_PIN = 11;
    constexpr uint8_t PIXEL_COUNT = 1;

    constexpr uint8_t LED_BRIGHTNESS = 24;

    Adafruit_NeoPixel pixel(
        PIXEL_COUNT,
        NEOPIXEL_DATA_PIN,
        NEO_GRB + NEO_KHZ800
    );
}


void StatusLed::begin()
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
        LED_BRIGHTNESS
    );

    pixel.clear();
    pixel.show();

    setStatus(
        SystemStatus::Startup
    );
}


void StatusLed::setStatus(
    SystemStatus status
)
{
    switch (status)
    {
        case SystemStatus::Startup:
            // White
            setColor(255, 255, 255);
            break;

        case SystemStatus::Ready:
            // Blue:
            // Firmware healthy, waiting for RC activity.
            setColor(0, 0, 255);
            break;

        case SystemStatus::ReceiverBytes:
            // Yellow:
            // UART bytes exist, but no valid RC frame yet.
            setColor(255, 180, 0);
            break;

        case SystemStatus::ReceiverFrames:
            // Normal-operation green.
            setColor(0, 255, 0);
            break;

        case SystemStatus::ReceiverLost:
            // Purple:
            // RC frame timeout / failsafe.
            setColor(180, 0, 255);
            break;

        case SystemStatus::Error:
            // Red:
            // Startup/self-test failure.
            setColor(255, 0, 0);
            break;
    }
}


void StatusLed::showLinkQuality(
    uint8_t linkQuality
)
{
    if (linkQuality >= 90)
    {
        // Lime green.
        //
        // Deliberately different from the normal-operation
        // pure green so a button press is immediately visible.
        setColor(120, 255, 0);
        return;
    }

    if (linkQuality >= 70)
    {
        // Yellow.
        setColor(255, 180, 0);
        return;
    }

    // Orange/red:
    // poor link quality.
    setColor(255, 45, 0);
}


void StatusLed::showDiagnosticUnavailable()
{
    // White indicates that diagnostic mode was entered,
    // but no current link-quality value was available.
    setColor(255, 255, 255);
}


void StatusLed::setColor(
    unsigned char red,
    unsigned char green,
    unsigned char blue
)
{
    pixel.setPixelColor(
        0,
        pixel.Color(
            red,
            green,
            blue
        )
    );

    pixel.show();
}