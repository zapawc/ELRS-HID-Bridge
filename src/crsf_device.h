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
    uint32_t serialNumber = 0;
    uint32_t hardwareId = 0;

    uint32_t firmwareId = 0;


    // Number/version of CRSF parameters exposed by this device.
    uint8_t parameterCount = 0;

    uint8_t parameterVersion = 0;
};


struct CrsfParameterRead
{
    uint8_t frameAddress = 0;
    uint8_t destination = 0;
    uint8_t origin = 0;

    uint8_t parameterNumber = 0;
    uint8_t chunkNumber = 0;
};


struct CrsfParameterWrite
{
    static constexpr uint8_t MAX_DATA_LENGTH = 8;

    uint8_t frameAddress = 0;
    uint8_t destination = 0;
    uint8_t origin = 0;

    uint8_t parameterNumber = 0;

    uint8_t data[MAX_DATA_LENGTH] = {};
    uint8_t dataLength = 0;
};


class CrsfDevice
{
public:
    void reset();


    void handleDevicePing(
        const CrsfFrame& frame
    );

    bool hasDevicePing() const;

    const CrsfDevicePing&
    devicePing() const;

    void clearDevicePing();


    // Capture validated CRSF Parameter Read (0x2C) and Parameter Write
    // (0x2D) frames. Routing bytes are retained so application policy can
    // decide whether a request is addressed to this bridge.
    void handleParameterRead(
        const CrsfFrame& frame
    );

    bool hasParameterRead() const;

    const CrsfParameterRead&
    parameterRead() const;

    void clearParameterRead();


    void handleParameterWrite(
        const CrsfFrame& frame
    );

    bool hasParameterWrite() const;

    const CrsfParameterWrite&
    parameterWrite() const;

    void clearParameterWrite();


    // Construct a CRSF Parameter Device Information (0x29) response
    // for a previously recognized Device Ping (0x28).
    bool buildDeviceInfoResponse(
        const CrsfDevicePing& ping,
        uint8_t localAddress,
        const CrsfDeviceIdentity& identity,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    // Build parameter-entry responses for the small runtime parameter set.
    //
    // These methods encode protocol mechanics only. Parameter numbering and
    // bridge-specific values remain caller-owned policy.
    bool buildFolderParameterResponse(
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
    ) const;


    bool buildFloatParameterResponse(
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
    ) const;


    bool buildTextSelectionParameterResponse(
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
    ) const;


    bool buildInfoParameterResponse(
        const CrsfParameterRead& request,
        uint8_t localAddress,
        uint8_t parameterNumber,
        uint8_t parentFolder,
        const char* name,
        const char* info,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    // Build a stateful COMMAND Parameter Settings Entry (0x2B).
    //
    // This helper is used for both normal parameter reads and COMMAND write
    // responses. COMMAND writes are acknowledged with a full 0x2B entry rather
    // than a value-only 0x2D frame.
    bool buildCommandParameterResponse(
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
    ) const;


    // A successful FLOAT write is confirmed with a 0x2D frame containing
    // Parameter_number followed by the accepted 4-byte big-endian value.
    bool buildFloatWriteResponse(
        const CrsfParameterWrite& request,
        uint8_t localAddress,
        uint8_t parameterNumber,
        int32_t acceptedValue,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    // A successful TEXT_SELECTION write is confirmed with a 0x2D frame
    // containing Parameter_number followed by the accepted one-byte index.
    bool buildTextSelectionWriteResponse(
        const CrsfParameterWrite& request,
        uint8_t localAddress,
        uint8_t parameterNumber,
        uint8_t acceptedValue,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;


    static bool readInt32BigEndian(
        const uint8_t* data,
        size_t length,
        int32_t& value
    );


private:
    static void writeUint32BigEndian(
        uint32_t value,
        uint8_t* output
    );


    static bool requestIsForLocalDevice(
        uint8_t destination,
        uint8_t origin,
        uint8_t localAddress
    );


    bool devicePingAvailable = false;
    CrsfDevicePing latestDevicePing = {};


    bool parameterReadAvailable = false;
    CrsfParameterRead latestParameterRead = {};


    bool parameterWriteAvailable = false;
    CrsfParameterWrite latestParameterWrite = {};
};
