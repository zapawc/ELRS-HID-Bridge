#include <Arduino.h>

#include "channel_state.h"
#include "test_generator.h"
#include "usb_hid.h"

ChannelState channelState;
TestGenerator testGenerator;
UsbHid usbHid;

void setup()
{
    usbHid.begin();
    testGenerator.begin();
}

void loop()
{
    testGenerator.update(channelState);
    usbHid.update(channelState);

    delay(10);
}