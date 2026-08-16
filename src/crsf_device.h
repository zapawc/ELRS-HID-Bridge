#pragma once

#include <stddef.h>
#include <stdint.h>
#include "crsf_frame.h"


struct CrsfDevicePing
{
    // First byte of the received CRSF frame.
    //
    // Retained for diagnostics and protocol investigation.
    uint8_t frameAddress = 0;


    // Extended-header routing fields.
    uint8_t destination = 0;

    uint8_t origin = 0;
};


struct CrsfDeviceIdentity
{
    // Null-terminated CRSF device name.
    //
    // The caller owns the storage. The string is consumed immediately
    // while a Device Info response is being constructed.
    const char* name = nullptr;


    // CRSF Device Info identifiers.
    //
    // Their final project values are intentionally supplied by the
    // caller so this protocol checkpoint does not prematurely lock in
    // production identity/address policy.
    uint32_t serialNumber = 0;

    uint32_t hardwareId = 0;

    uint32_t firmwareId = 0;


    // Number/version of CRSF parameters exposed by this device.
    //
    // The current proof-of-concept uses zero parameters.
    uint8_t parameterCount = 0;

    uint8_t parameterVersion = 0;
};


class CrsfDevice
{
public:
    void reset();


    // Process a validated CRSF Device Ping frame.
    //
    // The current CrsfFrame abstraction exposes all bytes after
    // Frame Type through payload.
    //
    // For an extended-header frame:
    //
    // payload[0] = Destination
    // payload[1] = Origin
    // payload[2...] = application payload / optional newer fields
    //
    // Device Ping has no defined application payload, but additional
    // trailing bytes are intentionally tolerated.
    void handleDevicePing(
        const CrsfFrame& frame
    );


    bool hasDevicePing() const;


    const CrsfDevicePing&
    devicePing() const;


    void clearDevicePing();


    // Construct a CRSF Parameter Device Information (0x29) response
    // for a previously recognized Device Ping (0x28).
    //
    // This method only constructs the frame. It does not transmit it.
    //
    // A response is generated only when the ping is either:
    //
    // - broadcast, or
    // - addressed directly to localAddress.
    //
    // The response is routed back to the ping origin and uses
    // localAddress as the response Origin field.
    //
    // localAddress and identity are deliberately caller-supplied at
    // this checkpoint. The live RP2/EdgeTX path still needs to validate
    // the final device address and production identity values before
    // they are committed as project policy.
    //
    // outputLength is set to zero on failure.
    bool buildDeviceInfoResponse(
        const CrsfDevicePing& ping,
        uint8_t localAddress,
        const CrsfDeviceIdentity& identity,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


private:
    static void writeUint32BigEndian(
        uint32_t value,
        uint8_t* output
    );


    bool devicePingAvailable = false;


    CrsfDevicePing latestDevicePing = {};
};
