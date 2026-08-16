#include <Arduino.h>

#include "boot_button.h"
#include "channel_mapper.h"
#include "channel_normalizer.h"
#include "channel_state.h"
#include "crsf_decoder.h"
#include "crsf_self_test.h"
#include "crsf_uart.h"
#include "link_statistics.h"
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
BootButton bootButton;

CrsfUart crsfUart;
CrsfDecoder crsfDecoder;

bool crsfSelfTestPassed = false;

bool receiverBytesSeen = false;
bool receiverFramesSeen = false;
bool receiverLost = false;

bool linkStatisticsSeen = false;

bool diagnosticDisplayActive = false;

uint32_t lastValidRcFrameMs = 0;
uint32_t diagnosticDisplayStartMs = 0;


namespace
{
    constexpr uint32_t RECEIVER_TIMEOUT_MS = 500;

    constexpr uint32_t DIAGNOSTIC_DISPLAY_MS = 3000;


    void setFailsafeState(
        ChannelState& state
    )
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


    void restoreNormalLedState(
        StatusLed& led,
        bool receiverFramesSeen,
        bool receiverLost,
        bool receiverBytesSeen
    )
    {
        if (receiverLost)
        {
            led.setStatus(
                SystemStatus::ReceiverLost
            );

            return;
        }


        if (receiverFramesSeen)
        {
            led.setStatus(
                SystemStatus::ReceiverFrames
            );

            return;
        }


        if (receiverBytesSeen)
        {
            led.setStatus(
                SystemStatus::ReceiverBytes
            );

            return;
        }


        led.setStatus(
            SystemStatus::Ready
        );
    }
}


void setup()
{
    statusLed.begin();
    bootButton.begin();

    usbHid.begin();


    crsfSelfTestPassed =
        CrsfSelfTest::run();


    if (!crsfSelfTestPassed)
    {
        statusLed.setStatus(
            SystemStatus::Error
        );


        setFailsafeState(
            channelState
        );


        usbHid.update(
            channelState
        );


        return;
    }


    statusLed.setStatus(
        SystemStatus::Ready
    );


    crsfUart.begin();


    setFailsafeState(
        channelState
    );


    usbHid.update(
        channelState
    );
}


void loop()
{
    if (!crsfSelfTestPassed)
    {
        delay(10);

        return;
    }


    // -------------------------------------------------------------------------
    // BOOT button
    // -------------------------------------------------------------------------

    const BootButtonEvent buttonEvent =
        bootButton.update();


    if (
        buttonEvent ==
        BootButtonEvent::ShortPress
    )
    {
        diagnosticDisplayActive =
            true;


        diagnosticDisplayStartMs =
            millis();


        // Do not display stale RF health after the RC link has
        // already been declared lost.

        if (
            linkStatisticsSeen &&
            !receiverLost
        )
        {
            const LinkStatistics& statistics =
                crsfDecoder.getLinkStatistics();


            statusLed.showLinkQuality(
                statistics.uplinkLinkQuality
            );
        }
        else
        {
            statusLed.showDiagnosticUnavailable();
        }
    }


    // Long/very-long presses are reserved for future
    // maintenance and factory-reset functions.


    // -------------------------------------------------------------------------
    // Receive CRSF bytes
    // -------------------------------------------------------------------------

    uint8_t crsfByte = 0;


    while (
        crsfUart.readByte(
            crsfByte
        )
    )
    {
        crsfDecoder.pushByte(
            crsfByte
        );
    }


    if (
        !receiverBytesSeen &&
        crsfUart.hasReceivedData()
    )
    {
        receiverBytesSeen =
            true;


        if (!diagnosticDisplayActive)
        {
            statusLed.setStatus(
                SystemStatus::ReceiverBytes
            );
        }
    }


    // -------------------------------------------------------------------------
    // Process RC channel frames
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


        receiverFramesSeen =
            true;


        receiverLost =
            false;


        if (!diagnosticDisplayActive)
        {
            statusLed.setStatus(
                SystemStatus::ReceiverFrames
            );
        }


        crsfDecoder.clearNewChannels();
    }


    // -------------------------------------------------------------------------
    // Process Link Statistics
    // -------------------------------------------------------------------------

    if (
        crsfDecoder.hasNewLinkStatistics()
    )
    {
        linkStatisticsSeen =
            true;


        crsfDecoder.clearNewLinkStatistics();
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
            receiverLost =
                true;


            setFailsafeState(
                channelState
            );


            // Receiver loss has higher priority than a
            // temporary diagnostic display.

            diagnosticDisplayActive =
                false;


            statusLed.setStatus(
                SystemStatus::ReceiverLost
            );
        }
    }


    // -------------------------------------------------------------------------
    // End temporary diagnostic display
    // -------------------------------------------------------------------------

    if (diagnosticDisplayActive)
    {
        const uint32_t now =
            millis();


        if (
            (now - diagnosticDisplayStartMs) >=
                DIAGNOSTIC_DISPLAY_MS
        )
        {
            diagnosticDisplayActive =
                false;


            restoreNormalLedState(
                statusLed,
                receiverFramesSeen,
                receiverLost,
                receiverBytesSeen
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