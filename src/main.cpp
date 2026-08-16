#include <Arduino.h>
#include "boot_button.h"
#include "bridge_configuration.h"
#include "bridge_identity.h"
#include "bridge_state.h"
#include "channel_mapper.h"
#include "channel_normalizer.h"
#include "channel_state.h"
#include "crsf_decoder.h"
#include "crsf_device.h"
#include "crsf_device_self_test.h"
#include "crsf_frame_encoder.h"
#include "crsf_frame_encoder_self_test.h"
#include "crsf_self_test.h"
#include "crsf_uart.h"
#include "failsafe_policy.h"
#include "failsafe_policy_self_test.h"
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
    // Startup self-tests
    //
    // Validate:
    //
    // - receive parsing/decoding
    // - outbound extended-frame construction
    // - Device Ping recognition
    // - Device Info response construction
    // - complete failsafe output policy
    //
    // None of these startup tests transmit on the live CRSF UART.
    // -------------------------------------------------------------------------
    startupSelfTestsPassed =
        CrsfSelfTest::run() &&
        CrsfFrameEncoderSelfTest::run() &&
        CrsfDeviceSelfTest::run() &&
        FailsafePolicySelfTest::run();


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
    // Device Ping -> Device Info
    //
    // This is the first live bidirectional CRSF proof-of-concept.
    //
    // CrsfDevice::buildDeviceInfoResponse() remains the single source of
    // routing/filtering/encoding policy. It only creates a response when the
    // ping is broadcast or is directly addressed to our experimental local
    // CRSF node address.
    //
    // The experimental address/identity live in bridge_identity.h so this
    // hardware test does not bury temporary protocol policy in main.cpp.
    // -------------------------------------------------------------------------

    if (
        crsfDecoder.hasDevicePing()
    )
    {
        const CrsfDevicePing ping =
            crsfDecoder.getDevicePing();


        uint8_t response[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t responseLength = 0;


        const CrsfDeviceIdentity identity =
            BridgeIdentity::crsfDeviceIdentity();


        CrsfDevice responseBuilder;


        if (
            responseBuilder.buildDeviceInfoResponse(
                ping,
                BridgeIdentity::CRSF_DEVICE_ADDRESS,
                identity,
                response,
                sizeof(response),
                responseLength
            )
        )
        {
            crsfUart.write(
                response,
                responseLength
            );
        }


        // One incoming ping can produce at most one response attempt.
        // Clear it whether it was addressed to us or intentionally ignored.
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
