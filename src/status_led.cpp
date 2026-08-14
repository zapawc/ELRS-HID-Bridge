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
    pinMode(NEOPIXEL_POWER_PIN, OUTPUT);
    digitalWrite(NEOPIXEL_POWER_PIN, HIGH);

    delay(1);

    pixel.begin();
    pixel.setBrightness(LED_BRIGHTNESS);
    pixel.clear();
    pixel.show();

    setStatus(SystemStatus::Startup);
}

void StatusLed::setStatus(SystemStatus status)
{
    switch (status)
    {
        case SystemStatus::Startup:
            // White
            setColor(255, 255, 255);
            break;

        case SystemStatus::Ready:
            // Blue:
            // Firmware healthy, no receiver bytes seen yet.
            setColor(0, 0, 255);
            break;

        case SystemStatus::ReceiverBytes:
            // Yellow:
            // UART activity exists, but no valid RC frame yet.
            setColor(255, 180, 0);
            break;

        case SystemStatus::ReceiverFrames:
            // Green:
            // Valid CRSF RC channel frames decoded.
            setColor(0, 255, 0);
            break;

        case SystemStatus::Error:
            // Red:
            // Startup/self-test failure.
            setColor(255, 0, 0);
            break;
    }
}

void StatusLed::setColor(
    unsigned char red,
    unsigned char green,
    unsigned char blue
)
{
    pixel.setPixelColor(
        0,
        pixel.Color(red, green, blue)
    );

    pixel.show();
}