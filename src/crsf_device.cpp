#include "crsf_device.h"

#include "crsf_protocol.h"


void CrsfDevice::reset()
{
    devicePingAvailable = false;


    latestDevicePing =
        CrsfDevicePing{};
}


void CrsfDevice::handleDevicePing(
    const CrsfFrame& frame
)
{
    if (
        frame.type !=
        Crsf::FRAME_DEVICE_PING
    )
    {
        return;
    }


    // Extended-header frames require at least:
    //
    // Destination
    // Origin
    //
    // The current CrsfFrame abstraction counts these routing bytes
    // inside payloadLength.

    if (
        frame.payload == nullptr ||
        frame.payloadLength < 2
    )
    {
        return;
    }


    latestDevicePing.frameAddress =
        frame.address;


    latestDevicePing.destination =
        frame.payload[0];


    latestDevicePing.origin =
        frame.payload[1];


    devicePingAvailable = true;
}


bool CrsfDevice::hasDevicePing() const
{
    return
        devicePingAvailable;
}


const CrsfDevicePing&
CrsfDevice::devicePing() const
{
    return
        latestDevicePing;
}


void CrsfDevice::clearDevicePing()
{
    devicePingAvailable = false;
}