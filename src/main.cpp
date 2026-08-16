#include <Arduino.h>
#include "boot_button.h"
#include "bridge_configuration.h"
#include "bridge_configuration_record_self_test.h"
#include "bridge_configuration_store.h"
#include "bridge_identity.h"
#include "bridge_parameters.h"
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


RawChannels rawChannels;

NormalizedChannels normalizedChannels;

ChannelState channelState;


// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

BridgeConfiguration bridgeConfiguration =
    BridgeConfiguration::defaults();


BridgeConfigurationStore
    bridgeConfigurationStore;


BridgeParameters bridgeParameters(
    bridgeConfiguration
);


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
    // -------------------------------------------------------------------------
    // Persistent configuration
    //
    // bridgeConfiguration already contains known-good defaults. A missing,
    // corrupt, incompatible, or otherwise invalid record is therefore a safe
    // no-op.
    // -------------------------------------------------------------------------

    bridgeConfigurationStore.load(
        bridgeConfiguration
    );


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
    // - persistent configuration record validation/corruption rejection
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
        BridgeConfigurationRecordSelfTest::run() &&
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
    // BridgeParameters owns parameter IDs, metadata, ranges, and CRSF entry
    // construction. main.cpp only transports the resulting response.
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


        if (
            bridgeParameters
                .buildReadResponse(
                    request,
                    BridgeIdentity::CRSF_DEVICE_ADDRESS,
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


        crsfDecoder.clearParameterRead();
    }


    // -------------------------------------------------------------------------
    // CRSF parameter writes
    //
    // BridgeParameters owns simple values plus the Restore Defaults command
    // lifecycle. Responses requiring durable state are transmitted only after
    // persistence succeeds.
    // -------------------------------------------------------------------------

    if (
        crsfDecoder.hasParameterWrite()
    )
    {
        const CrsfParameterWrite request =
            crsfDecoder.getParameterWrite();


        uint8_t response[
            CrsfFrameEncoder::MAX_FRAME_SIZE
        ] = {};

        size_t responseLength = 0;


        BridgeParameterWriteResult result;


        // Keep a complete pre-write snapshot so persistence failure can be
        // rolled back without exposing a runtime value that was never saved.
        const BridgeConfiguration
            previousConfiguration =
                bridgeConfiguration;


        if (
            bridgeParameters
                .handleWrite(
                    request,
                    BridgeIdentity::CRSF_DEVICE_ADDRESS,
                    response,
                    sizeof(response),
                    responseLength,
                    result
                )
        )
        {
            bool persistenceSucceeded =
                true;


            if (
                result.requiresPersistence
            )
            {
                persistenceSucceeded =
                    bridgeConfigurationStore
                        .save(
                            bridgeConfiguration
                        );
            }


            bridgeParameters
                .finalizePersistence(
                    result,
                    persistenceSucceeded
                );


            if (persistenceSucceeded)
            {
                switch (
                    result.change
                )
                {
                    case BridgeParameterChange::
                        LedBrightness:
                    {
                        statusLed
                            .setBrightnessPercent(
                                result
                                    .ledBrightnessPercent
                            );

                        break;
                    }


                    case BridgeParameterChange::
                        PitchInversion:
                    {
                        // ChannelMapper references BridgeConfiguration
                        // directly. The persisted inversion setting is
                        // therefore consumed on the next RC frame.

                        break;
                    }


                    case BridgeParameterChange::
                        AxisInversion:
                    {
                        // All analog mappings reference BridgeConfiguration
                        // directly. The persisted inversion setting is consumed
                        // on the next RC frame without a mapper rebuild.

                        break;
                    }


                    case BridgeParameterChange::
                        RestoreDefaults:
                    {
                        // Restore every application-side setting that has an
                        // immediate presentation effect. ChannelMapper already
                        // references BridgeConfiguration for every analog
                        // inversion setting.
                        statusLed
                            .setBrightnessPercent(
                                result
                                    .ledBrightnessPercent
                            );

                        break;
                    }


                    case BridgeParameterChange::None:
                    {
                        break;
                    }
                }


                // Simple parameter writes use 0x2D acknowledgements.
                // COMMAND writes use stateful 0x2B responses. In either case
                // the response prepared by BridgeParameters is transmitted
                // only when any required persistence has succeeded.
                crsfUart.write(
                    response,
                    responseLength
                );
            }
            else
            {
                // Restore the entire configuration snapshot if durable storage
                // failed. For Restore Defaults, BridgeParameters intentionally
                // leaves the confirmation pending so EdgeTX can poll/retry.
                bridgeConfiguration =
                    previousConfiguration;
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
