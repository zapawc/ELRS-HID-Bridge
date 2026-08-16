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
#include "status_display.h"
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

StatusDisplay statusDisplay(
    statusLed
);

BootButton bootButton;

CrsfUart crsfUart;
CrsfDecoder crsfDecoder;

BridgeState bridgeState;

bool crsfSelfTestPassed = false;


void setup()
{
    statusLed.begin();
    statusDisplay.reset();

    bootButton.begin();

    bridgeState.reset();

    usbHid.begin();


    crsfSelfTestPassed =
        CrsfSelfTest::run();


    if (!crsfSelfTestPassed)
    {
        statusDisplay.showFatalError();


        failsafePolicy.apply(
            channelState
        );


        usbHid.update(
            channelState
        );


        return;
    }


    crsfUart.begin();


    failsafePolicy.apply(
        channelState
    );


    usbHid.update(
        channelState
    );


    // Transition from the startup indication to the current
    // normal bridge state.
    statusDisplay.update(
        millis(),
        bridgeState
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
        // Do not display stale RF health after the RC link has
        // already been declared lost.

        if (
            bridgeState.hasLinkStatistics() &&
            !bridgeState.isReceiverLost()
        )
        {
            statusDisplay.showLinkQuality(
                bridgeState
                    .linkStatistics()
                    .uplinkLinkQuality,
                millis()
            );
        }
        else
        {
            statusDisplay.showDiagnosticUnavailable(
                millis()
            );
        }
    }


    // Long/very-long presses remain reserved for future
    // maintenance actions.


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
    }


    // -------------------------------------------------------------------------
    // Status display
    //
    // StatusDisplay owns LED arbitration. Runtime code reports facts and
    // requests temporary indications; it no longer decides which normal
    // color should currently be displayed.
    // -------------------------------------------------------------------------

    statusDisplay.update(
        millis(),
        bridgeState
    );


    // -------------------------------------------------------------------------
    // USB HID
    // -------------------------------------------------------------------------

    usbHid.update(
        channelState
    );


    delay(1);
}