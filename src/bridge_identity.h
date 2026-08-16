#pragma once

#include <stdint.h>
#include "crsf_device.h"
#include "crsf_protocol.h"


namespace BridgeIdentity
{
    // -------------------------------------------------------------------------
    // Experimental CRSF device identity
    // -------------------------------------------------------------------------
    //
    // The bridge sits on the flight-controller side of the RP2 CRSF UART, so
    // the first live Device Info discovery experiment uses the standard CRSF
    // Flight Controller address (0xC8).
    //
    // This is intentionally an experimental routing choice. Hardware testing
    // through RP2 -> Ranger -> EdgeTX must validate it before it is treated as
    // permanent project policy.
    // -------------------------------------------------------------------------

    constexpr uint8_t CRSF_DEVICE_ADDRESS =
        Crsf::ADDRESS_FLIGHT_CONTROLLER;


    constexpr const char* CRSF_DEVICE_NAME =
        "ELRS-HID-Bridge";


    // Deterministic proof-of-concept identity values.
    //
    // These values are not claimed to be globally assigned identifiers. They
    // simply give the discovery experiment stable, non-zero fields while the
    // project determines what production identity/version policy should be.
    //
    // 0x45484231 = ASCII "EHB1"
    // 0x51545059 = ASCII "QTPY"

    constexpr uint32_t CRSF_SERIAL_NUMBER =
        0x45484231;

    constexpr uint32_t CRSF_HARDWARE_ID =
        0x51545059;

    constexpr uint32_t CRSF_FIRMWARE_ID =
        0x00000001;


    // Identity-only discovery exposes no CRSF parameters yet.
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
