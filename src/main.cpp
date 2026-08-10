#include <Arduino.h>

#include "channel_mapper.h"
#include "channel_state.h"
#include "raw_channel_test.h"
#include "raw_channels.h"
#include "usb_hid.h"

RawChannels rawChannels;
ChannelState channelState;

RawChannelTest rawChannelTest;
ChannelMapper channelMapper;
UsbHid usbHid;

void setup()
{
    usbHid.begin();
    rawChannelTest.begin();
}

void loop()
{
    rawChannelTest.update(rawChannels);

    channelMapper.update(
        rawChannels,
        channelState
    );

    usbHid.update(channelState);

    delay(10);
}