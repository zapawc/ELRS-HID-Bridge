#include <Arduino.h>

#include "channel_mapper.h"
#include "channel_normalizer.h"
#include "channel_state.h"
#include "crsf_decoder.h"
#include "crsf_self_test.h"
#include "crsf_uart.h"
#include "normalized_channels.h"
#include "raw_channel_test.h"
#include "raw_channels.h"
#include "status_led.h"
#include "usb_hid.h"

RawChannels rawChannels;
NormalizedChannels normalizedChannels;
ChannelState channelState;

RawChannelTest rawChannelTest;
ChannelNormalizer channelNormalizer;
ChannelMapper channelMapper;
UsbHid usbHid;
StatusLed statusLed;

CrsfUart crsfUart;
CrsfDecoder crsfDecoder;

bool crsfSelfTestPassed = false;

bool receiverBytesSeen = false;
bool receiverFramesSeen = false;

void setup()
{
    statusLed.begin();

    usbHid.begin();

    // Validate our CRSF implementation before accepting
    // live receiver data.
    crsfSelfTestPassed =
        CrsfSelfTest::run();

    if (!crsfSelfTestPassed)
    {
        statusLed.setStatus(
            SystemStatus::Error
        );
    }
    else
    {
        statusLed.setStatus(
            SystemStatus::Ready
        );
    }

    crsfUart.begin();

    // Keep the known-good synthetic HID source active.
    rawChannelTest.begin();
}

void loop()
{
    // -------------------------------------------------------------------------
    // Live CRSF validation path
    // -------------------------------------------------------------------------

    crsfUart.update(crsfDecoder);

    if (
        crsfSelfTestPassed &&
        !receiverBytesSeen &&
        crsfUart.hasReceivedData()
    )
    {
        receiverBytesSeen = true;

        statusLed.setStatus(
            SystemStatus::ReceiverBytes
        );
    }

    if (
        crsfSelfTestPassed &&
        crsfDecoder.hasNewChannels()
    )
    {
        receiverFramesSeen = true;

        statusLed.setStatus(
            SystemStatus::ReceiverFrames
        );

        // We are only proving decode at this stage.
        // The synthetic joystick source remains active.
        crsfDecoder.clearNewChannels();
    }

    // -------------------------------------------------------------------------
    // Existing known-good synthetic joystick path
    // -------------------------------------------------------------------------

    rawChannelTest.update(rawChannels);

    channelNormalizer.update(
        rawChannels,
        normalizedChannels
    );

    channelMapper.update(
        normalizedChannels,
        channelState
    );

    // Button 32 remains the startup self-test fault indicator.
    if (!crsfSelfTestPassed)
    {
        channelState.buttons |=
            (1UL << 31);
    }

    usbHid.update(channelState);

    delay(10);
}