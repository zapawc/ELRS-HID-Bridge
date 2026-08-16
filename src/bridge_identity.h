#pragma once
#include <stdint.h>
#include "crsf_device.h"
#include "crsf_protocol.h"
#include "firmware_version.h"


namespace BridgeIdentity
{
    // -------------------------------------------------------------------------
    // Reference CRSF device identity
    // -------------------------------------------------------------------------
    //
    // The bridge occupies the flight-controller side of the RP2 CRSF UART.
    // Hardware validation confirmed that the standard CRSF Flight Controller
    // address (0xC8) routes Device Info successfully through:
    //
    // RP2 -> ELRS RF -> Ranger -> EdgeTX
    //
    // EdgeTX discovers ELRS-HID-Bridge under Other Devices with this address.
    // -------------------------------------------------------------------------

    constexpr uint8_t CRSF_DEVICE_ADDRESS =
        Crsf::ADDRESS_FLIGHT_CONTROLLER;


    constexpr const char* CRSF_DEVICE_NAME =
        "ELRS-HID-Bridge";


    // Project-defined deterministic identifiers.
    //
    // 0x45484231 = ASCII "EHB1"
    // 0x51545059 = ASCII "QTPY"
    constexpr uint32_t CRSF_SERIAL_NUMBER =
        0x45484231u;

    constexpr uint32_t CRSF_HARDWARE_ID =
        0x51545059u;


    constexpr uint32_t CRSF_FIRMWARE_ID =
        FirmwareVersion::CRSF_ID;


    // Parameter 0 is the standardized root folder. Parameter count reports
    // the normal numbered settings exposed after that root entry.
    //
    // Current set:
    //
    // 0 = ROOT folder
    // 1 = LED Brightness
    constexpr uint8_t CRSF_PARAMETER_COUNT = 1;

    constexpr uint8_t CRSF_PARAMETER_VERSION = 1;


    inline CrsfDeviceIdentity crsfDeviceIdentity()
    {
        CrsfDeviceIdentity identity;

        identity.name =
            CRSF_DEVICE_NAME;

        identity.serialNumber =
            CRSF_SERIAL_NUMBER;

        identity.hardwareId =
            CRSF_HARDWARE_ID;

        identity.firmwareId =
            CRSF_FIRMWARE_ID;

        identity.parameterCount =
            CRSF_PARAMETER_COUNT;

        identity.parameterVersion =
            CRSF_PARAMETER_VERSION;

        return identity;
    }
}
