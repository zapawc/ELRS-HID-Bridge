#include "crsf_device.h"
#include "crsf_frame_encoder.h"
#include "crsf_protocol.h"


namespace
{
    constexpr size_t EXTENDED_FRAME_OVERHEAD = 6;

    constexpr size_t MAX_EXTENDED_PAYLOAD_SIZE =
        CrsfFrameEncoder::MAX_FRAME_SIZE -
        EXTENDED_FRAME_OVERHEAD;


    // Device Info fields following the null-terminated name.
    constexpr size_t DEVICE_INFO_FIXED_FIELD_SIZE = 14;


    constexpr size_t MAX_DEVICE_NAME_LENGTH =
        MAX_EXTENDED_PAYLOAD_SIZE -
        1 -
        DEVICE_INFO_FIXED_FIELD_SIZE;


    bool appendString(
        const char* value,
        uint8_t* payload,
        size_t payloadCapacity,
        size_t& payloadIndex
    )
    {
        if (
            value == nullptr ||
            payload == nullptr
        )
        {
            return false;
        }


        size_t index = 0;

        while (true)
        {
            if (
                payloadIndex >=
                payloadCapacity
            )
            {
                return false;
            }


            const uint8_t byte =
                static_cast<uint8_t>(
                    value[index]
                );

            payload[payloadIndex++] =
                byte;


            if (byte == 0)
            {
                return true;
            }


            ++index;
        }
    }
}


void CrsfDevice::reset()
{
    devicePingAvailable = false;

    latestDevicePing =
        CrsfDevicePing{};


    parameterReadAvailable = false;

    latestParameterRead =
        CrsfParameterRead{};


    parameterWriteAvailable = false;

    latestParameterWrite =
        CrsfParameterWrite{};
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


void CrsfDevice::handleParameterRead(
    const CrsfFrame& frame
)
{
    if (
        frame.type !=
        Crsf::FRAME_PARAMETER_READ ||
        frame.payload == nullptr ||
        frame.payloadLength < 4
    )
    {
        return;
    }


    latestParameterRead.frameAddress =
        frame.address;

    latestParameterRead.destination =
        frame.payload[0];

    latestParameterRead.origin =
        frame.payload[1];

    latestParameterRead.parameterNumber =
        frame.payload[2];

    latestParameterRead.chunkNumber =
        frame.payload[3];

    parameterReadAvailable = true;
}


bool CrsfDevice::hasParameterRead() const
{
    return
        parameterReadAvailable;
}


const CrsfParameterRead&
CrsfDevice::parameterRead() const
{
    return
        latestParameterRead;
}


void CrsfDevice::clearParameterRead()
{
    parameterReadAvailable = false;
}


void CrsfDevice::handleParameterWrite(
    const CrsfFrame& frame
)
{
    if (
        frame.type !=
        Crsf::FRAME_PARAMETER_WRITE ||
        frame.payload == nullptr ||
        frame.payloadLength < 3
    )
    {
        return;
    }


    const size_t dataLength =
        frame.payloadLength - 3;


    if (
        dataLength >
        CrsfParameterWrite::MAX_DATA_LENGTH
    )
    {
        return;
    }


    latestParameterWrite =
        CrsfParameterWrite{};

    latestParameterWrite.frameAddress =
        frame.address;

    latestParameterWrite.destination =
        frame.payload[0];

    latestParameterWrite.origin =
        frame.payload[1];

    latestParameterWrite.parameterNumber =
        frame.payload[2];

    latestParameterWrite.dataLength =
        static_cast<uint8_t>(
            dataLength
        );


    for (
        size_t index = 0;
        index < dataLength;
        ++index
    )
    {
        latestParameterWrite.data[index] =
            frame.payload[3 + index];
    }


    parameterWriteAvailable = true;
}


bool CrsfDevice::hasParameterWrite() const
{
    return
        parameterWriteAvailable;
}


const CrsfParameterWrite&
CrsfDevice::parameterWrite() const
{
    return
        latestParameterWrite;
}


void CrsfDevice::clearParameterWrite()
{
    parameterWriteAvailable = false;
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


bool CrsfDevice::readInt32BigEndian(
    const uint8_t* data,
    size_t length,
    int32_t& value
)
{
    if (
        data == nullptr ||
        length != 4
    )
    {
        return false;
    }


    const uint32_t unsignedValue =
        (
            static_cast<uint32_t>(
                data[0]
            ) << 24
        ) |
        (
            static_cast<uint32_t>(
                data[1]
            ) << 16
        ) |
        (
            static_cast<uint32_t>(
                data[2]
            ) << 8
        ) |
        static_cast<uint32_t>(
            data[3]
        );


    value =
        static_cast<int32_t>(
            unsignedValue
        );


    return true;
}


bool CrsfDevice::requestIsForLocalDevice(
    uint8_t destination,
    uint8_t origin,
    uint8_t localAddress
)
{
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


    if (
        origin ==
            Crsf::ADDRESS_BROADCAST ||
        !Crsf::isValidSyncByte(
            origin
        )
    )
    {
        return false;
    }


    return
        destination ==
        localAddress;
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


    if (
        ping.destination !=
            Crsf::ADDRESS_BROADCAST &&
        ping.destination !=
            localAddress
    )
    {
        return false;
    }


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


bool CrsfDevice::buildFolderParameterResponse(
    const CrsfParameterRead& request,
    uint8_t localAddress,
    uint8_t parameterNumber,
    uint8_t parentFolder,
    const char* name,
    const uint8_t* children,
    size_t childCount,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    if (
        request.parameterNumber !=
            parameterNumber ||
        request.chunkNumber != 0 ||
        !requestIsForLocalDevice(
            request.destination,
            request.origin,
            localAddress
        )
    )
    {
        return false;
    }


    if (
        childCount > 0 &&
        children == nullptr
    )
    {
        return false;
    }


    uint8_t payload[
        MAX_EXTENDED_PAYLOAD_SIZE
    ] = {};

    size_t payloadIndex = 0;


    payload[payloadIndex++] =
        parameterNumber;

    // Entire entry fits into one CRSF frame.
    payload[payloadIndex++] = 0;

    payload[payloadIndex++] =
        parentFolder;

    payload[payloadIndex++] =
        Crsf::PARAMETER_TYPE_FOLDER;


    if (
        !appendString(
            name,
            payload,
            sizeof(payload),
            payloadIndex
        )
    )
    {
        return false;
    }


    for (
        size_t index = 0;
        index < childCount;
        ++index
    )
    {
        if (
            payloadIndex >=
            sizeof(payload)
        )
        {
            return false;
        }


        payload[payloadIndex++] =
            children[index];
    }


    if (
        payloadIndex >=
        sizeof(payload)
    )
    {
        return false;
    }


    // Folder child lists are terminated by 0xFF.
    payload[payloadIndex++] =
        0xFF;


    CrsfFrameEncoder encoder;


    return
        encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_PARAMETER_SETTINGS_ENTRY,
            request.origin,
            localAddress,
            payload,
            payloadIndex,
            output,
            outputCapacity,
            outputLength
        );
}


bool CrsfDevice::buildFloatParameterResponse(
    const CrsfParameterRead& request,
    uint8_t localAddress,
    uint8_t parameterNumber,
    uint8_t parentFolder,
    const char* name,
    int32_t value,
    int32_t minimum,
    int32_t maximum,
    int32_t defaultValue,
    uint8_t decimalPoint,
    int32_t stepSize,
    const char* unit,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    if (
        request.parameterNumber !=
            parameterNumber ||
        request.chunkNumber != 0 ||
        !requestIsForLocalDevice(
            request.destination,
            request.origin,
            localAddress
        )
    )
    {
        return false;
    }


    if (
        value < minimum ||
        value > maximum ||
        defaultValue < minimum ||
        defaultValue > maximum ||
        stepSize <= 0
    )
    {
        return false;
    }


    uint8_t payload[
        MAX_EXTENDED_PAYLOAD_SIZE
    ] = {};

    size_t payloadIndex = 0;


    payload[payloadIndex++] =
        parameterNumber;

    payload[payloadIndex++] = 0;

    payload[payloadIndex++] =
        parentFolder;

    payload[payloadIndex++] =
        Crsf::PARAMETER_TYPE_FLOAT;


    if (
        !appendString(
            name,
            payload,
            sizeof(payload),
            payloadIndex
        )
    )
    {
        return false;
    }


    constexpr size_t numericBytes =
        4 + 4 + 4 + 4 + 1 + 4;


    if (
        payloadIndex +
        numericBytes >
        sizeof(payload)
    )
    {
        return false;
    }


    writeUint32BigEndian(
        static_cast<uint32_t>(
            value
        ),
        &payload[payloadIndex]
    );
    payloadIndex += 4;


    writeUint32BigEndian(
        static_cast<uint32_t>(
            minimum
        ),
        &payload[payloadIndex]
    );
    payloadIndex += 4;


    writeUint32BigEndian(
        static_cast<uint32_t>(
            maximum
        ),
        &payload[payloadIndex]
    );
    payloadIndex += 4;


    writeUint32BigEndian(
        static_cast<uint32_t>(
            defaultValue
        ),
        &payload[payloadIndex]
    );
    payloadIndex += 4;


    payload[payloadIndex++] =
        decimalPoint;


    writeUint32BigEndian(
        static_cast<uint32_t>(
            stepSize
        ),
        &payload[payloadIndex]
    );
    payloadIndex += 4;


    if (
        !appendString(
            unit,
            payload,
            sizeof(payload),
            payloadIndex
        )
    )
    {
        return false;
    }


    CrsfFrameEncoder encoder;


    return
        encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_PARAMETER_SETTINGS_ENTRY,
            request.origin,
            localAddress,
            payload,
            payloadIndex,
            output,
            outputCapacity,
            outputLength
        );
}


bool CrsfDevice::buildTextSelectionParameterResponse(
    const CrsfParameterRead& request,
    uint8_t localAddress,
    uint8_t parameterNumber,
    uint8_t parentFolder,
    const char* name,
    const char* options,
    uint8_t value,
    uint8_t minimum,
    uint8_t maximum,
    uint8_t defaultValue,
    const char* unit,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    if (
        request.parameterNumber !=
            parameterNumber ||
        request.chunkNumber != 0 ||
        !requestIsForLocalDevice(
            request.destination,
            request.origin,
            localAddress
        )
    )
    {
        return false;
    }


    if (
        value < minimum ||
        value > maximum ||
        defaultValue < minimum ||
        defaultValue > maximum
    )
    {
        return false;
    }


    uint8_t payload[
        MAX_EXTENDED_PAYLOAD_SIZE
    ] = {};

    size_t payloadIndex = 0;


    payload[payloadIndex++] =
        parameterNumber;

    // Entire entry fits into one CRSF frame.
    payload[payloadIndex++] = 0;

    payload[payloadIndex++] =
        parentFolder;

    payload[payloadIndex++] =
        Crsf::PARAMETER_TYPE_TEXT_SELECTION;


    if (
        !appendString(
            name,
            payload,
            sizeof(payload),
            payloadIndex
        )
    )
    {
        return false;
    }


    if (
        !appendString(
            options,
            payload,
            sizeof(payload),
            payloadIndex
        )
    )
    {
        return false;
    }


    constexpr size_t valueBytes = 4;


    if (
        payloadIndex +
        valueBytes >
        sizeof(payload)
    )
    {
        return false;
    }


    payload[payloadIndex++] =
        value;

    payload[payloadIndex++] =
        minimum;

    payload[payloadIndex++] =
        maximum;

    payload[payloadIndex++] =
        defaultValue;


    if (
        !appendString(
            unit,
            payload,
            sizeof(payload),
            payloadIndex
        )
    )
    {
        return false;
    }


    CrsfFrameEncoder encoder;


    return
        encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_PARAMETER_SETTINGS_ENTRY,
            request.origin,
            localAddress,
            payload,
            payloadIndex,
            output,
            outputCapacity,
            outputLength
        );
}


bool CrsfDevice::buildCommandParameterResponse(
    uint8_t destination,
    uint8_t origin,
    uint8_t localAddress,
    uint8_t parameterNumber,
    uint8_t parentFolder,
    const char* name,
    uint8_t status,
    uint8_t timeout,
    const char* info,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    if (
        !requestIsForLocalDevice(
            destination,
            origin,
            localAddress
        )
    )
    {
        return false;
    }


    uint8_t payload[
        MAX_EXTENDED_PAYLOAD_SIZE
    ] = {};

    size_t payloadIndex = 0;


    payload[payloadIndex++] =
        parameterNumber;

    // Entire command definition fits into one CRSF frame.
    payload[payloadIndex++] = 0;

    payload[payloadIndex++] =
        parentFolder;

    payload[payloadIndex++] =
        Crsf::PARAMETER_TYPE_COMMAND;


    if (
        !appendString(
            name,
            payload,
            sizeof(payload),
            payloadIndex
        )
    )
    {
        return false;
    }


    if (
        payloadIndex + 2 >
        sizeof(payload)
    )
    {
        return false;
    }


    payload[payloadIndex++] =
        status;

    payload[payloadIndex++] =
        timeout;


    if (
        !appendString(
            info,
            payload,
            sizeof(payload),
            payloadIndex
        )
    )
    {
        return false;
    }


    CrsfFrameEncoder encoder;


    return
        encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_PARAMETER_SETTINGS_ENTRY,
            origin,
            localAddress,
            payload,
            payloadIndex,
            output,
            outputCapacity,
            outputLength
        );
}


bool CrsfDevice::buildFloatWriteResponse(
    const CrsfParameterWrite& request,
    uint8_t localAddress,
    uint8_t parameterNumber,
    int32_t acceptedValue,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    if (
        request.parameterNumber !=
            parameterNumber ||
        !requestIsForLocalDevice(
            request.destination,
            request.origin,
            localAddress
        )
    )
    {
        return false;
    }


    uint8_t payload[5] = {};

    payload[0] =
        parameterNumber;


    writeUint32BigEndian(
        static_cast<uint32_t>(
            acceptedValue
        ),
        &payload[1]
    );


    CrsfFrameEncoder encoder;


    return
        encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_PARAMETER_WRITE,
            request.origin,
            localAddress,
            payload,
            sizeof(payload),
            output,
            outputCapacity,
            outputLength
        );
}


bool CrsfDevice::buildTextSelectionWriteResponse(
    const CrsfParameterWrite& request,
    uint8_t localAddress,
    uint8_t parameterNumber,
    uint8_t acceptedValue,
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;


    if (
        request.parameterNumber !=
            parameterNumber ||
        !requestIsForLocalDevice(
            request.destination,
            request.origin,
            localAddress
        )
    )
    {
        return false;
    }


    const uint8_t payload[] =
    {
        parameterNumber,
        acceptedValue
    };


    CrsfFrameEncoder encoder;


    return
        encoder.encodeExtended(
            Crsf::SYNC_BYTE,
            Crsf::FRAME_PARAMETER_WRITE,
            request.origin,
            localAddress,
            payload,
            sizeof(payload),
            output,
            outputCapacity,
            outputLength
        );
}
