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


bool crsfSelfTestPassed = false;


void setup()
{
    statusLed.begin();

    statusDisplay.reset();


    bootButton.begin();

    maintenanceController.reset();


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
            // Do not display stale RF health after the RC link
            // has already been declared lost.

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
            // The maintenance UI now recognizes and reports the
            // Bind selection, but no CRSF bind command is sent yet.
            //
            // Protocol/path validation will be implemented separately.

            break;
        }


        case MaintenanceAction::WifiRequested:
        {
            // Reserved.
            //
            // The maintenance UI now recognizes and reports the
            // Wi-Fi selection, but no CRSF Wi-Fi command is sent yet.
            //
            // Protocol/path validation will be implemented separately.

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