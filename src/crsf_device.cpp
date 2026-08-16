#include "crsf_device.h"
#include "crsf_frame_encoder.h"
#include "crsf_protocol.h"


namespace
{
    constexpr size_t EXTENDED_FRAME_OVERHEAD = 6;


    constexpr size_t MAX_EXTENDED_PAYLOAD_SIZE =
        CrsfFrameEncoder::MAX_FRAME_SIZE -
        EXTENDED_FRAME_OVERHEAD;


    // Device Info fields following the null-terminated name:
    //
    // Serial_number              4 bytes
    // Hardware_ID                4 bytes
    // Firmware_ID                4 bytes
    // Parameters_total           1 byte
    // Parameter_version_number   1 byte
    constexpr size_t DEVICE_INFO_FIXED_FIELD_SIZE = 14;


    constexpr size_t MAX_DEVICE_NAME_LENGTH =
        MAX_EXTENDED_PAYLOAD_SIZE -
        1 -
        DEVICE_INFO_FIXED_FIELD_SIZE;
}


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


void CrsfDevice::writeUint32BigEndian(
    uint32_t value,
    uint8_t* output
)
{
    output[0] =
        static_cast<uint8_t>(
            (value >> 24) & 0xFF
        );


    output[1] =
        static_cast<uint8_t>(
            (value >> 16) & 0xFF
        );


    output[2] =
        static_cast<uint8_t>(
            (value >> 8) & 0xFF
        );


    output[3] =
        static_cast<uint8_t>(
            value & 0xFF
        );
}


bool CrsfDevice::buildDeviceInfoResponse(
    const CrsfDevicePing& ping,
    uint8_t localAddress,
    const CrsfDeviceIdentity& identity,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    if (
        output == nullptr ||
        identity.name == nullptr
    )
    {
        return false;
    }


    // A device must have a concrete origin address.
    if (
        localAddress ==
            Crsf::ADDRESS_BROADCAST ||
        !Crsf::isValidSyncByte(
            localAddress
        )
    )
    {
        return false;
    }


    // The response destination is the ping origin, so the origin must
    // also identify a concrete CRSF device.
    if (
        ping.origin ==
            Crsf::ADDRESS_BROADCAST ||
        !Crsf::isValidSyncByte(
            ping.origin
        )
    )
    {
        return false;
    }


    // Device Ping can target every device or one specific device.
    // Do not answer traffic addressed to some other CRSF node.
    if (
        ping.destination !=
            Crsf::ADDRESS_BROADCAST &&
        ping.destination !=
            localAddress
    )
    {
        return false;
    }


    // Determine the name length while enforcing the CRSF maximum
    // frame size. A valid Device Info payload must leave room for the
    // terminating NUL and the fixed identity fields.
    size_t nameLength = 0;


    while (
        identity.name[nameLength] !=
        '\0'
    )
    {
        if (
            nameLength >=
            MAX_DEVICE_NAME_LENGTH
        )
        {
            return false;
        }


        ++nameLength;
    }


    uint8_t payload[
        MAX_EXTENDED_PAYLOAD_SIZE
    ] = {};


    size_t payloadIndex = 0;


    for (
        size_t index = 0;
        index < nameLength;
        ++index
    )
    {
        payload[payloadIndex++] =
            static_cast<uint8_t>(
                identity.name[index]
            );
    }


    // CRSF Device Info requires a null-terminated device name.
    payload[payloadIndex++] = 0;


    writeUint32BigEndian(
        identity.serialNumber,
        &payload[payloadIndex]
    );

    payloadIndex += 4;


    writeUint32BigEndian(
        identity.hardwareId,
        &payload[payloadIndex]
    );

    payloadIndex += 4;


    writeUint32BigEndian(
        identity.firmwareId,
        &payload[payloadIndex]
    );

    payloadIndex += 4;


    payload[payloadIndex++] =
        identity.parameterCount;


    payload[payloadIndex++] =
        identity.parameterVersion;


    CrsfFrameEncoder encoder;


    return
        encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_DEVICE_INFO,
            ping.origin,
            localAddress,
            payload,
            payloadIndex,
            output,
            outputCapacity,
            outputLength
        );
}
