#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

#include "channel_state.h"
#include "test_generator.h"

// -----------------------------------------------------------------------------
// HID report descriptor
//
// Standard joystick exposing:
//
//   X axis
//   Y axis
//   Slider 1
//   Slider 2
//   32 buttons
//
// Report size:
//   2 + 2 + 2 + 2 + 4 = 12 bytes
// -----------------------------------------------------------------------------

uint8_t const desc_hid_report[] =
{
    // Usage Page (Generic Desktop)
    0x05, 0x01,

    // Usage (Joystick)
    0x09, 0x04,

    // Collection (Application)
    0xA1, 0x01,

        // ---------------------------------------------------------------------
        // X and Y axes
        // ---------------------------------------------------------------------

        // Usage (X)
        0x09, 0x30,

        // Usage (Y)
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


        // ---------------------------------------------------------------------
        // Two sliders
        // ---------------------------------------------------------------------

        // Usage (Slider)
        0x09, 0x36,

        // Usage (Slider)
        0x09, 0x36,

        // Report Size (16 bits)
        0x75, 0x10,

        // Report Count (2)
        0x95, 0x02,

        // Input (Data, Variable, Absolute)
        0x81, 0x02,


        // ---------------------------------------------------------------------
        // 32 buttons
        // ---------------------------------------------------------------------

        // Usage Page (Button)
        0x05, 0x09,

        // Usage Minimum (Button 1)
        0x19, 0x01,

        // Usage Maximum (Button 32)
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


// -----------------------------------------------------------------------------
// USB report format
// -----------------------------------------------------------------------------

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


// -----------------------------------------------------------------------------
// Global objects
// -----------------------------------------------------------------------------

Adafruit_USBD_HID usbHid;

ChannelState channelState;
TestGenerator testGenerator;


// -----------------------------------------------------------------------------
// setup()
// -----------------------------------------------------------------------------

void setup()
{
    if (!TinyUSBDevice.isInitialized())
    {
        TinyUSBDevice.begin(0);
    }

    usbHid.setPollInterval(2);

    usbHid.setReportDescriptor(
        desc_hid_report,
        sizeof(desc_hid_report)
    );

    usbHid.setStringDescriptor("ELRS HID Bridge");

    usbHid.begin();

    if (TinyUSBDevice.mounted())
    {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }

    testGenerator.begin();
}


// -----------------------------------------------------------------------------
// loop()
// -----------------------------------------------------------------------------

void loop()
{
#ifdef TINYUSB_NEED_POLLING_TASK
    TinyUSBDevice.task();
#endif

    // Update synthetic control values.
    testGenerator.update(channelState);

    if (!TinyUSBDevice.mounted())
    {
        return;
    }

    if (usbHid.ready())
    {
        HidReport report;

        report.x = channelState.roll;
        report.y = channelState.pitch;

        report.slider1 = channelState.throttle;
        report.slider2 = channelState.yaw;

        report.buttons = channelState.buttons;

        usbHid.sendReport(
            0,
            &report,
            sizeof(report)
        );
    }

    delay(10);
}