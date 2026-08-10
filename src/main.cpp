#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

uint8_t const desc_hid_report[] =
{
    TUD_HID_REPORT_DESC_GAMEPAD()
};

Adafruit_USBD_HID usb_hid;

hid_gamepad_report_t report;

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

    usb_hid.begin();

    // Force Windows to re-enumerate after adding the HID class.
    if (TinyUSBDevice.mounted())
    {
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }

    memset(&report, 0, sizeof(report));
}

void loop()
{
    if (TinyUSBDevice.mounted() && usb_hid.ready())
    {
        usb_hid.sendReport(0, &report, sizeof(report));
    }

    delay(10);
}