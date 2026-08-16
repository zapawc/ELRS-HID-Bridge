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
    // The reference implementation therefore retains 0xC8 for v1.0 hardening.
    // -------------------------------------------------------------------------

    constexpr uint8_t CRSF_DEVICE_ADDRESS =
        Crsf::ADDRESS_FLIGHT_CONTROLLER;


    constexpr const char* CRSF_DEVICE_NAME =
        "ELRS-HID-Bridge";


    // Project-defined deterministic identifiers.
    //
    // CRSF Device Info specifies 32-bit Serial Number and Hardware ID fields but
    // does not define a public assignment registry for this project. The
    // reference implementation uses readable, stable FourCC-style identifiers:
    //
    // 0x45484231 = ASCII "EHB1" -> ELRS-HID-Bridge reference identity
    // 0x51545059 = ASCII "QTPY" -> QT Py RP2040 reference hardware
    //
    // CRSF_SERIAL_NUMBER is a project-family identifier, not a per-unit unique
    // hardware serial number.

    constexpr uint32_t CRSF_SERIAL_NUMBER =
        0x45484231u;

    constexpr uint32_t CRSF_HARDWARE_ID =
        0x51545059u;


    // Firmware ID is derived from the canonical semantic version source.
    constexpr uint32_t CRSF_FIRMWARE_ID =
        FirmwareVersion::CRSF_ID;


    // Identity-only discovery exposes no CRSF parameters in the v1.0 cycle.
    constexpr uint8_t CRSF_PARAMETER_COUNT = 0;
    constexpr uint8_t CRSF_PARAMETER_VERSION = 0;


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
