#include <Arduino.h>

#include "channel_mapper.h"
#include "channel_normalizer.h"
#include "channel_state.h"
#include "crsf_self_test.h"
#include "debug_log.h"
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

bool crsfSelfTestPassed = false;

void setup()
{
    DebugLog::begin();

    usbHid.begin();

    // Run the CRSF protocol self-test once at startup.
    crsfSelfTestPassed =
        CrsfSelfTest::run();

    if (crsfSelfTestPassed)
    {
        DebugLog::info(
            "[SELFTEST] CRSF decoder: PASS"
        );
    }
    else
    {
        DebugLog::info(
            "[SELFTEST] CRSF decoder: FAIL"
        );
    }

    rawChannelTest.begin();
}

void loop()
{
    // Generate synthetic CRSF-style channel values.
    rawChannelTest.update(rawChannels);

    // Raw/protocol values -> normalized 0-65535 values.
    channelNormalizer.update(
        rawChannels,
        normalizedChannels
    );

    // Normalized channels -> semantic joystick state.
    channelMapper.update(
        normalizedChannels,
        channelState
    );

    // Development-time fault indicator.
    //
    // Button 32 remains OFF when the CRSF self-test passes.
    // Button 32 is forced ON if the self-test fails.
    if (!crsfSelfTestPassed)
    {
        channelState.buttons |=
            (1UL << 31);
    }

    usbHid.update(channelState);

    delay(10);
}