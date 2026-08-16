#pragma once

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


private:
    bool devicePingAvailable = false;


    CrsfDevicePing latestDevicePing = {};
};