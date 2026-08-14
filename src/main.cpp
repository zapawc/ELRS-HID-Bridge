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

        return;
    }

    statusLed.setStatus(
        SystemStatus::Ready
    );

    // Start the physical CRSF UART.
    crsfUart.begin();
}

void loop()
{
    // If the startup protocol self-test failed, leave the
    // system in its error state and do not process receiver data.
    if (!crsfSelfTestPassed)
    {
        delay(10);
        return;
    }

    // -------------------------------------------------------------------------
    // Receive live CRSF data
    // -------------------------------------------------------------------------

    crsfUart.update(crsfDecoder);

    // UART activity proves that electrical/serial communication
    // exists between the RP2 and QT Py.
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
    // Process newly decoded RC channel frames
    // -------------------------------------------------------------------------

    if (crsfDecoder.hasNewChannels())
    {
        if (!receiverFramesSeen)
        {
            receiverFramesSeen = true;

            statusLed.setStatus(
                SystemStatus::ReceiverFrames
            );
        }

        // Copy the live CRSF channel values into our protocol/raw
        // channel representation.
        rawChannels =
            crsfDecoder.getChannels();

        // CRSF-specific raw values -> normalized 0-65535 values.
        channelNormalizer.update(
            rawChannels,
            normalizedChannels
        );

        // Normalized channels -> joystick axes/buttons.
        channelMapper.update(
            normalizedChannels,
            channelState
        );

        // Send the live transmitter state to Windows.
        usbHid.update(
            channelState
        );

        crsfDecoder.clearNewChannels();
    }

    delay(1);
}