#include <Arduino.h>

#include "boot_button.h"
#include "bridge_configuration.h"
#include "bridge_state.h"
#include "channel_mapper.h"
#include "channel_normalizer.h"
#include "channel_state.h"
#include "crsf_decoder.h"
#include "crsf_self_test.h"
#include "crsf_uart.h"
#include "failsafe_policy.h"
#include "normalized_channels.h"
#include "raw_channels.h"
#include "status_led.h"
#include "usb_hid.h"


RawChannels rawChannels;
NormalizedChannels normalizedChannels;
ChannelState channelState;


// -----------------------------------------------------------------------------
// Configuration
//
// The reference firmware currently uses compiled defaults only.
//
// Future persistent storage and CRSF parameter configuration can modify this
// canonical model without requiring ChannelMapper or the rest of the runtime
// path to own configuration policy.
// -----------------------------------------------------------------------------

BridgeConfiguration bridgeConfiguration =
    BridgeConfiguration::defaults();


ChannelNormalizer channelNormalizer;

ChannelMapper channelMapper(
    bridgeConfiguration
);

FailsafePolicy failsafePolicy;

UsbHid usbHid;
StatusLed statusLed;
BootButton bootButton;

CrsfUart crsfUart;
CrsfDecoder crsfDecoder;

BridgeState bridgeState;

bool crsfSelfTestPassed = false;

bool diagnosticDisplayActive = false;

uint32_t diagnosticDisplayStartMs = 0;


namespace
{
    constexpr uint32_t DIAGNOSTIC_DISPLAY_MS = 3000;


    void restoreNormalLedState(
        StatusLed& led,
        const BridgeState& state
    )
    {
        if (state.isReceiverLost())
        {
            led.setStatus(
                SystemStatus::ReceiverLost
            );

            return;
        }


        if (state.hasRcFrames())
        {
            led.setStatus(
                SystemStatus::ReceiverFrames
            );

            return;
        }


        if (state.hasReceiverBytes())
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

    bridgeState.reset();

    usbHid.begin();


    crsfSelfTestPassed =
        CrsfSelfTest::run();


    if (!crsfSelfTestPassed)
    {
        statusLed.setStatus(
            SystemStatus::Error
        );


        failsafePolicy.apply(
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


    failsafePolicy.apply(
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
            bridgeState.hasLinkStatistics() &&
            !bridgeState.isReceiverLost()
        )
        {
            statusLed.showLinkQuality(
                bridgeState
                    .linkStatistics()
                    .uplinkLinkQuality
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
        !bridgeState.hasReceiverBytes() &&
        crsfUart.hasReceivedData()
    )
    {
        bridgeState.noteUartActivity();


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


        bridgeState.noteRcFrame(
            millis()
        );


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
        bridgeState.noteLinkStatistics(
            crsfDecoder.getLinkStatistics()
        );


        crsfDecoder.clearNewLinkStatistics();
    }


    // -------------------------------------------------------------------------
    // Receiver timeout / failsafe
    // -------------------------------------------------------------------------

    if (
        bridgeState.updateRcTimeout(
            millis(),
            bridgeConfiguration.receiverTimeoutMs
        )
    )
    {
        failsafePolicy.apply(
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
                bridgeState
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