#include <Arduino.h>

#include "boot_button.h"
#include "bridge_configuration.h"
#include "bridge_state.h"
#include "channel_mapper.h"
#include "channel_normalizer.h"
#include "channel_state.h"
#include "crsf_decoder.h"
#include "crsf_device_self_test.h"
#include "crsf_frame_encoder_self_test.h"
#include "crsf_self_test.h"
#include "crsf_uart.h"
#include "failsafe_policy.h"
#include "maintenance_controller.h"
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


MaintenanceController maintenanceController;


CrsfUart crsfUart;


CrsfDecoder crsfDecoder;


BridgeState bridgeState;


bool startupSelfTestsPassed = false;


void setup()
{
    statusLed.begin();


    statusDisplay.reset();


    bootButton.begin();


    maintenanceController.reset();


    bridgeState.reset();


    usbHid.begin();


    // -------------------------------------------------------------------------
    // Startup CRSF tests
    //
    // Validate:
    //
    // - receive parsing/decoding
    // - outbound extended-frame construction
    // - Device Ping recognition
    //
    // None of these startup tests transmit on the live CRSF UART.
    // -------------------------------------------------------------------------

    startupSelfTestsPassed =
        CrsfSelfTest::run() &&
        CrsfFrameEncoderSelfTest::run() &&
        CrsfDeviceSelfTest::run();


    if (!startupSelfTestsPassed)
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


    statusDisplay.update(
        millis(),
        bridgeState
    );
}


void loop()
{
    if (!startupSelfTestsPassed)
    {
        delay(10);

        return;
    }


    // -------------------------------------------------------------------------
    // BOOT button / maintenance UI
    // -------------------------------------------------------------------------

    const BootButtonState buttonState =
        bootButton.update();


    const MaintenanceUpdate maintenanceUpdate =
        maintenanceController.update(
            buttonState
        );


    // -------------------------------------------------------------------------
    // Maintenance selection display
    // -------------------------------------------------------------------------

    if (
        maintenanceUpdate.selectionChanged
    )
    {
        switch (
            maintenanceUpdate.selection
        )
        {
            case MaintenanceSelection::Bind:
            {
                statusDisplay.showMaintenanceBind();

                break;
            }


            case MaintenanceSelection::Wifi:
            {
                statusDisplay.showMaintenanceWifi();

                break;
            }


            case MaintenanceSelection::Cancel:
            {
                statusDisplay.showMaintenanceCancel();

                break;
            }


            case MaintenanceSelection::None:
            {
                statusDisplay.clearMaintenance();

                break;
            }
        }
    }


    // -------------------------------------------------------------------------
    // Maintenance action on release
    // -------------------------------------------------------------------------

    switch (
        maintenanceUpdate.action
    )
    {
        case MaintenanceAction::Diagnostic:
        {
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


            break;
        }


        case MaintenanceAction::BindRequested:
        {
            // Reserved.
            //
            // No receiver command is sent yet.

            break;
        }


        case MaintenanceAction::WifiRequested:
        {
            // Reserved.
            //
            // No receiver command is sent yet.

            break;
        }


        case MaintenanceAction::None:
        {
            break;
        }
    }


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

    if (
        crsfDecoder.hasNewChannels()
    )
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
    // Device Ping
    //
    // Recognition is implemented and covered by startup tests.
    //
    // Live requests are deliberately consumed without generating
    // a response during this checkpoint.
    // -------------------------------------------------------------------------

    if (
        crsfDecoder.hasDevicePing()
    )
    {
        crsfDecoder.clearDevicePing();
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
    // Status display arbitration
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