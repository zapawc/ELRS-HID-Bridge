#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

// Standard USB HID joystick descriptor:
//
// 2 x 16-bit axes:
//   X
//   Y
//
// 2 x 16-bit sliders
//
// 32 buttons
//
// Total report size:
//   2 + 2 + 2 + 2 + 4 = 12 bytes

uint8_t const desc_hid_report[] =
{
    // Usage Page (Generic Desktop)
    0x05, 0x01,

    // Usage (Joystick)
    0x09, 0x04,

    // Collection (Application)
    0xA1, 0x01,

        // -------------------------
        // X and Y
        // -------------------------

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


        // -------------------------
        // Two sliders
        // -------------------------

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


        // -------------------------
        // 32 buttons
        // -------------------------

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

struct __attribute__((packed)) JoystickReport
{
    uint16_t x;
    uint16_t y;

    uint16_t slider1;
    uint16_t slider2;

    uint32_t buttons;
};

static_assert(sizeof(JoystickReport) == 12,
              "JoystickReport must be exactly 12 bytes");

Adafruit_USBD_HID usb_hid;

JoystickReport report = {};

void setup()
{
    if (!TinyUSBDevice.isInitialized())
    {
        TinyUSBDevice.begin(0);
    }

    usb_hid.setPollInterval(2);

    usb_hid.setReportDescriptor(
        desc_hid_report,
        sizeof(desc_hid_report)
    );

    usb_hid.setStringDescriptor("ELRS HID Bridge");

    usb_hid.begin();

    if (TinyUSBDevice.mounted())
    {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }

    // Start everything centered.
    report.x = 32768;
    report.y = 32768;

    report.slider1 = 32768;
    report.slider2 = 32768;

    report.buttons = 0;
}

void loop()
{
#ifdef TINYUSB_NEED_POLLING_TASK
    TinyUSBDevice.task();
#endif

    if (!TinyUSBDevice.mounted())
    {
        return;
    }

    if (usb_hid.ready())
    {
        usb_hid.sendReport(
            0,
            &report,
            sizeof(report)
        );
    }

    delay(10);
}