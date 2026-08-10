#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "status_led.h"

namespace
{
    constexpr uint8_t NEOPIXEL_DATA_PIN = 12;
    constexpr uint8_t NEOPIXEL_POWER_PIN = 11;
    constexpr uint8_t PIXEL_COUNT = 1;

    // Keep the onboard pixel intentionally dim.
    constexpr uint8_t LED_BRIGHTNESS = 24;

    Adafruit_NeoPixel pixel(
        PIXEL_COUNT,
        NEOPIXEL_DATA_PIN,
        NEO_GRB + NEO_KHZ800
    );
}

void StatusLed::begin()
{
    // The QT Py RP2040 switches power to the onboard NeoPixel
    // through GPIO11. The generic Pico board definition does not
    // automatically enable this for us.
    pinMode(NEOPIXEL_POWER_PIN, OUTPUT);
    digitalWrite(NEOPIXEL_POWER_PIN, HIGH);

    // Give power a moment to stabilize before talking to the pixel.
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
            // Blue
            setColor(0, 0, 255);
            break;

        case SystemStatus::Error:
            // Red
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