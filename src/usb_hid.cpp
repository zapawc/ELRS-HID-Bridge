#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

#include "usb_hid.h"

// -----------------------------------------------------------------------------
// HID report descriptor
// -----------------------------------------------------------------------------

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

            // X and Y axes
            0x09, 0x30,
            0x09, 0x31,

            // Logical Minimum (0)
            0x15, 0x00,

            // Logical Maximum (65535)
            0x27, 0xFF, 0xFF, 0x00, 0x00,

            // Report Size (16 bits)
            0x75, 0x10,

            // Report Count (2)
            0x95, 0x02,

            // Input (Data, Variable, Absolute)
            0x81, 0x02,

            // Two sliders
            0x09, 0x36,
            0x09, 0x36,

            // Report Size (16 bits)
            0x75, 0x10,

            // Report Count (2)
            0x95, 0x02,

            // Input (Data, Variable, Absolute)
            0x81, 0x02,

            // Button page
            0x05, 0x09,

            // Buttons 1-32
            0x19, 0x01,
            0x29, 0x20,

            // Logical Minimum (0)
            0x15, 0x00,

            // Logical Maximum (1)
            0x25, 0x01,

            // Report Size (1 bit)
            0x75, 0x01,

            // Report Count (32)
            0x95, 0x20,

            // Input (Data, Variable, Absolute)
            0x81, 0x02,

        // End Collection
        0xC0
    };

    struct __attribute__((packed)) HidReport
    {
        uint16_t x;
        uint16_t y;
        uint16_t slider1;
        uint16_t slider2;
        uint32_t buttons;
    };

    static_assert(
        sizeof(HidReport) == 12,
        "HidReport must be exactly 12 bytes"
    );

    Adafruit_USBD_HID usbHid;
}


// -----------------------------------------------------------------------------
// UsbHid::begin
// -----------------------------------------------------------------------------

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

    usbHid.setStringDescriptor("ELRS HID Bridge");

    usbHid.begin();

    if (TinyUSBDevice.mounted())
    {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
}


// -----------------------------------------------------------------------------
// UsbHid::update
// -----------------------------------------------------------------------------

void UsbHid::update(const ChannelState& state)
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

    report.x = state.roll;
    report.y = state.pitch;
    report.slider1 = state.throttle;
    report.slider2 = state.yaw;
    report.buttons = state.buttons;

    usbHid.sendReport(
        0,
        &report,
        sizeof(report)
    );
}