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
#include "crsf_parameter_self_test.h"
#include "crsf_self_test.h"
#include "crsf_uart.h"
#include "failsafe_policy.h"
#include "failsafe_policy_self_test.h"
#include "firmware_version_self_test.h"
#include "maintenance_controller.h"
#include "normalized_channels.h"
#include "raw_channels.h"
#include "status_display.h"
#include "status_led.h"
#include "usb_hid.h"


namespace
{
    constexpr uint8_t PARAMETER_ROOT = 0;
    constexpr uint8_t PARAMETER_LED_BRIGHTNESS = 1;

    constexpr int32_t LED_BRIGHTNESS_MIN = 0;
    constexpr int32_t LED_BRIGHTNESS_MAX = 100;
    constexpr int32_t LED_BRIGHTNESS_DEFAULT = 10;
    constexpr int32_t LED_BRIGHTNESS_STEP = 1;


    bool requestIsForBridge(
        uint8_t destination
    )
    {
        return
            destination ==
            BridgeIdentity::CRSF_DEVICE_ADDRESS;
    }
}


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
    statusLed.begin(
        bridgeConfiguration
            .ledBrightnessPercent
    );


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
    // - CRSF parameter read/write encoding
    // - complete failsafe output policy
    // - canonical firmware version / CRSF Firmware ID consistency
    //
    // None of these startup tests transmit on the live CRSF UART.
    // -------------------------------------------------------------------------

    startupSelfTestsPassed =
        CrsfSelfTest::run() &&
        CrsfFrameEncoderSelfTest::run() &&
        CrsfDeviceSelfTest::run() &&
        CrsfParameterSelfTest::run() &&
        FailsafePolicySelfTest::run() &&
        FirmwareVersionSelfTest::run();


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


        crsfDecoder.clearDevicePing();
    }


    // -------------------------------------------------------------------------
    // CRSF parameter reads
    //
    // Parameter 0 = standardized ROOT folder
    // Parameter 1 = LED Brightness, FLOAT 0-100%
    //
    // All entries fit in one CRSF frame, so only chunk 0 is supported.
    // -------------------------------------------------------------------------

    if (
        crsfDecoder.hasParameterRead()
    )
    {
        const CrsfParameterRead request =
            crsfDecoder.getParameterRead();


        uint8_t response[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t responseLength = 0;


        CrsfDevice responseBuilder;


        if (
            request.parameterNumber ==
                PARAMETER_ROOT
        )
        {
            constexpr uint8_t children[] =
            {
                PARAMETER_LED_BRIGHTNESS
            };


            if (
                responseBuilder
                    .buildFolderParameterResponse(
                        request,
                        BridgeIdentity::CRSF_DEVICE_ADDRESS,
                        PARAMETER_ROOT,
                        PARAMETER_ROOT,
                        "ROOT",
                        children,
                        sizeof(children),
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
        }
        else if (
            request.parameterNumber ==
                PARAMETER_LED_BRIGHTNESS
        )
        {
            if (
                responseBuilder
                    .buildFloatParameterResponse(
                        request,
                        BridgeIdentity::CRSF_DEVICE_ADDRESS,
                        PARAMETER_LED_BRIGHTNESS,
                        PARAMETER_ROOT,
                        "LED Brightness",
                        bridgeConfiguration
                            .ledBrightnessPercent,
                        LED_BRIGHTNESS_MIN,
                        LED_BRIGHTNESS_MAX,
                        LED_BRIGHTNESS_DEFAULT,
                        0,
                        LED_BRIGHTNESS_STEP,
                        "",
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
        }


        crsfDecoder.clearParameterRead();
    }


    // -------------------------------------------------------------------------
    // CRSF parameter writes
    //
    // Runtime-only checkpoint:
    //
    // - accept only parameter 1
    // - require a standard four-byte FLOAT value
    // - validate 0-100
    // - apply immediately
    // - acknowledge with 0x2D
    // - do not persist across reboot
    // -------------------------------------------------------------------------

    if (
        crsfDecoder.hasParameterWrite()
    )
    {
        const CrsfParameterWrite request =
            crsfDecoder.getParameterWrite();


        if (
            request.parameterNumber ==
                PARAMETER_LED_BRIGHTNESS &&
            requestIsForBridge(
                request.destination
            )
        )
        {
            int32_t brightness = 0;


            if (
                CrsfDevice::readInt32BigEndian(
                    request.data,
                    request.dataLength,
                    brightness
                ) &&
                brightness >=
                    LED_BRIGHTNESS_MIN &&
                brightness <=
                    LED_BRIGHTNESS_MAX
            )
            {
                bridgeConfiguration
                    .ledBrightnessPercent =
                    static_cast<uint8_t>(
                        brightness
                    );


                statusLed.setBrightnessPercent(
                    bridgeConfiguration
                        .ledBrightnessPercent
                );


                uint8_t response[
                    CrsfFrameEncoder::MAX_FRAME_SIZE
                ] = {};

                size_t responseLength = 0;


                CrsfDevice responseBuilder;


                if (
                    responseBuilder
                        .buildFloatWriteResponse(
                            request,
                            BridgeIdentity::CRSF_DEVICE_ADDRESS,
                            PARAMETER_LED_BRIGHTNESS,
                            brightness,
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
            }
        }


        crsfDecoder.clearParameterWrite();
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
