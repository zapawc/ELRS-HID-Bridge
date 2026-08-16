#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "usb_hid.h"

namespace
{
    uint8_t const descHidReport[] =
    {
        // Usage Page (Generic Desktop)
        0x05, 0x01,

        // Usage (Joystick)
        0x09, 0x04,

        // Collection (Application)
        0xA1, 0x01,

            // -----------------------------------------------------------------
            // X and Y
            // -----------------------------------------------------------------

            0x09, 0x30,       // X
            0x09, 0x31,       // Y
            0x15, 0x00,       // Logical Minimum 0
            0x27, 0xFF, 0xFF, 0x00, 0x00, // Logical Maximum 65535

            0x75, 0x10,       // Report Size 16
            0x95, 0x02,       // Report Count 2
            0x81, 0x02,       // Input


            // -----------------------------------------------------------------
            // Slider 1 and Slider 2
            // -----------------------------------------------------------------
            0x09, 0x36,       // Slider
            0x09, 0x36,       // Slider

            0x75, 0x10,
            0x95, 0x02,
            0x81, 0x02,


            // -----------------------------------------------------------------
            // Z, Rx, Ry, Rz
            // -----------------------------------------------------------------

            0x09, 0x32,       // Z
            0x09, 0x33,       // Rx
            0x09, 0x34,       // Ry
            0x09, 0x35,       // Rz
            0x75, 0x10,
            0x95, 0x04,
            0x81, 0x02,


            // -----------------------------------------------------------------
            // 32 buttons
            // -----------------------------------------------------------------

            0x05, 0x09,       // Usage Page (Button)

            0x19, 0x01,       // Usage Minimum 1
            0x29, 0x20,       // Usage Maximum 32
            0x15, 0x00,       // Logical Minimum 0
            0x25, 0x01,       // Logical Maximum 1

            0x75, 0x01,       // Report Size 1 bit
            0x95, 0x20,       // Report Count 32
            0x81, 0x02,       // Input

        0xC0
    };


    struct __attribute__((packed)) HidReport
    {
        uint16_t x;
        uint16_t y;

        uint16_t slider1;
        uint16_t slider2;

        uint16_t z;
        uint16_t rx;
        uint16_t ry;
        uint16_t rz;
        uint32_t buttons;
    };


    static_assert(
        sizeof(HidReport) == 20,
        "HidReport must be exactly 20 bytes"
    );


    Adafruit_USBD_HID usbHid;
}


void UsbHid::begin()
{
    if (!TinyUSBDevice.isInitialized())
    {
        TinyUSBDevice.begin(0);
    }

    usbHid.setPollInterval(2);

    usbHid.setReportDescriptor(
        descHidReport,
        sizeof(descHidReport)
    );

    // User-visible HID interface identity. Keep this synchronized with the
    // USB product string configured in platformio.ini.
    usbHid.setStringDescriptor(
        "ELRS-HID-Bridge"
    );

    usbHid.begin();
    if (TinyUSBDevice.mounted())
    {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
}


void UsbHid::update(
    const ChannelState& state
)
{
#ifdef TINYUSB_NEED_POLLING_TASK
    TinyUSBDevice.task();
#endif

    if (!TinyUSBDevice.mounted())
    {
        return;
    }

    if (!usbHid.ready())
    {
        return;
    }

    HidReport report;

    report.x =
        state.roll;

    report.y =
        state.pitch;
    report.slider1 =
        state.throttle;

    report.slider2 =
        state.yaw;

    report.z =
        state.auxAnalog1;

    report.rx =
        state.auxAnalog2;
    report.ry =
        state.auxAnalog3;

    report.rz =
        state.auxAnalog4;

    report.buttons =
        state.buttons;

    usbHid.sendReport(
        0,
        &report,
        sizeof(report)
    );
}
