#include <Arduino.h>

#include "channel_mapper.h"
#include "channel_normalizer.h"
#include "channel_state.h"
#include "normalized_channels.h"
#include "raw_channel_test.h"
#include "raw_channels.h"
#include "usb_hid.h"

RawChannels rawChannels;
NormalizedChannels normalizedChannels;
ChannelState channelState;

RawChannelTest rawChannelTest;
ChannelNormalizer channelNormalizer;
ChannelMapper channelMapper;
UsbHid usbHid;

void setup()
{
    usbHid.begin();
    rawChannelTest.begin();
}

void loop()
{
    // Synthetic CRSF-style values: approximately 172-1811.
    rawChannelTest.update(rawChannels);

    // Protocol/raw representation -> common 0-65535 representation.
    channelNormalizer.update(
        rawChannels,
        normalizedChannels
    );

    // Common channels -> semantic joystick state.
    channelMapper.update(
        normalizedChannels,
        channelState
    );

    // Semantic joystick state -> USB.
    usbHid.update(channelState);

    delay(10);
}