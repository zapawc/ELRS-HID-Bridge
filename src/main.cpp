#include <Arduino.h>

#include "channel_mapper.h"
#include "channel_normalizer.h"
#include "channel_state.h"
#include "crsf_decoder.h"
#include "crsf_self_test.h"
#include "crsf_uart.h"
#include "normalized_channels.h"
#include "raw_channels.h"
#include "status_led.h"
#include "usb_hid.h"

RawChannels rawChannels;
NormalizedChannels normalizedChannels;
ChannelState channelState;

ChannelNormalizer channelNormalizer;
ChannelMapper channelMapper;
UsbHid usbHid;
StatusLed statusLed;

CrsfUart crsfUart;
CrsfDecoder crsfDecoder;

bool crsfSelfTestPassed = false;
bool receiverBytesSeen = false;
bool receiverFramesSeen = false;
bool receiverLost = false;

uint32_t lastValidRcFrameMs = 0;

namespace
{
    constexpr uint32_t RECEIVER_TIMEOUT_MS = 500;

    void setFailsafeState(ChannelState& state)
    {
        state.roll =
            NormalizedChannels::MID;

        state.pitch =
            NormalizedChannels::MID;

        state.throttle =
            NormalizedChannels::MIN;

        state.yaw =
            NormalizedChannels::MID;

        state.buttons = 0;
    }
}

void setup()
{
    statusLed.begin();

    usbHid.begin();

    // Validate the CRSF implementation before accepting
    // live receiver data.
    crsfSelfTestPassed =
        CrsfSelfTest::run();

    if (!crsfSelfTestPassed)
    {
        statusLed.setStatus(
            SystemStatus::Error
        );

        setFailsafeState(channelState);
        usbHid.update(channelState);

        return;
    }

    statusLed.setStatus(
        SystemStatus::Ready
    );

    // Start the physical CRSF UART.
    crsfUart.begin();

    // Start in a safe neutral state until the first
    // valid live RC frame arrives.
    setFailsafeState(channelState);
    usbHid.update(channelState);
}

void loop()
{
    if (!crsfSelfTestPassed)
    {
        delay(10);
        return;
    }

    // -------------------------------------------------------------------------
    // Receive CRSF bytes
    // -------------------------------------------------------------------------

    crsfUart.update(crsfDecoder);

    if (
        !receiverBytesSeen &&
        crsfUart.hasReceivedData()
    )
    {
        receiverBytesSeen = true;

        statusLed.setStatus(
            SystemStatus::ReceiverBytes
        );
    }

    // -------------------------------------------------------------------------
    // Process valid RC channel frames
    // -------------------------------------------------------------------------

    if (crsfDecoder.hasNewChannels())
    {
        rawChannels =
            crsfDecoder.getChannels();

        channelNormalizer.update(
            rawChannels,
            normalizedChannels
        );

        channelMapper.update(
            normalizedChannels,
            channelState
        );

        lastValidRcFrameMs =
            millis();

        receiverFramesSeen = true;
        receiverLost = false;

        statusLed.setStatus(
            SystemStatus::ReceiverFrames
        );

        crsfDecoder.clearNewChannels();
    }

    // -------------------------------------------------------------------------
    // Receiver timeout / failsafe
    // -------------------------------------------------------------------------

    if (receiverFramesSeen)
    {
        const uint32_t now =
            millis();

        if (
            !receiverLost &&
            (now - lastValidRcFrameMs) >=
                RECEIVER_TIMEOUT_MS
        )
        {
            receiverLost = true;

            setFailsafeState(
                channelState
            );

            statusLed.setStatus(
                SystemStatus::ReceiverLost
            );
        }
    }

    // -------------------------------------------------------------------------
    // USB HID
    // -------------------------------------------------------------------------

    usbHid.update(
        channelState
    );

    delay(1);
}