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
        {
            // White.
            setColor(
                255,
                255,
                255
            );

            break;
        }


        case SystemStatus::Ready:
        {
            // Blue:
            // firmware healthy, waiting for RC activity.
            setColor(
                0,
                0,
                255
            );

            break;
        }


        case SystemStatus::ReceiverBytes:
        {
            // Yellow:
            // UART bytes exist, but no valid RC frame yet.
            setColor(
                255,
                180,
                0
            );

            break;
        }


        case SystemStatus::ReceiverFrames:
        {
            // Pure green:
            // normal CRSF -> HID operation.
            setColor(
                0,
                255,
                0
            );

            break;
        }


        case SystemStatus::ReceiverLost:
        {
            // Purple:
            // RC frame timeout / failsafe.
            setColor(
                180,
                0,
                255
            );

            break;
        }


        case SystemStatus::Error:
        {
            // Red:
            // startup/self-test failure.
            setColor(
                255,
                0,
                0
            );

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
        // Lime green.
        //
        // Deliberately different from normal-operation pure green.
        setColor(
            120,
            255,
            0
        );

        return;
    }


    if (linkQuality >= 70)
    {
        // Yellow.
        setColor(
            255,
            180,
            0
        );

        return;
    }


    // Orange/red:
    // poor link quality.
    setColor(
        255,
        45,
        0
    );
}


void StatusLed::showDiagnosticUnavailable()
{
    // White indicates that diagnostic mode was entered,
    // but no current Link Quality was available.
    setColor(
        255,
        255,
        255
    );
}


void StatusLed::showMaintenanceBind()
{
    // Blue:
    // Bind maintenance action currently selected.
    setColor(
        0,
        0,
        255
    );
}


void StatusLed::showMaintenanceWifi()
{
    // White:
    // Wi-Fi maintenance action currently selected.
    setColor(
        255,
        255,
        255
    );
}


void StatusLed::showMaintenanceCancel()
{
    // Cyan:
    // maintenance action cancelled if released now.
    //
    // Deliberately distinct from:
    //
    // pure green = normal operation
    // purple     = receiver lost
    // red        = fatal error
    setColor(
        0,
        255,
        255
    );
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