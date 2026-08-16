#pragma once
#include <stdint.h>


namespace Crsf
{
    // -------------------------------------------------------------------------
    // CRSF framing limits
    // -------------------------------------------------------------------------

    constexpr uint8_t MAX_FRAME_LENGTH = 62;

    constexpr uint8_t MAX_FRAME_SIZE = 64;


    // -------------------------------------------------------------------------
    // CRSF serial synchronization
    // -------------------------------------------------------------------------

    // Traditional CRSF serial synchronization value.
    //
    // Numerically identical to the Flight Controller device address.
    constexpr uint8_t SYNC_BYTE = 0xC8;


    // -------------------------------------------------------------------------
    // Device addresses
    // -------------------------------------------------------------------------

    constexpr uint8_t ADDRESS_BROADCAST = 0x00;

    constexpr uint8_t ADDRESS_CLOUD = 0x0E;
    constexpr uint8_t ADDRESS_USB = 0x10;
    constexpr uint8_t ADDRESS_BLUETOOTH_WIFI = 0x12;
    constexpr uint8_t ADDRESS_WIFI_RECEIVER = 0x13;
    constexpr uint8_t ADDRESS_VIDEO_RECEIVER = 0x14;
    constexpr uint8_t ADDRESS_DYNAMIC_MIN = 0x20;
    constexpr uint8_t ADDRESS_DYNAMIC_MAX = 0x7F;

    constexpr uint8_t ADDRESS_OSD = 0x80;
    constexpr uint8_t ADDRESS_RESERVED_8A = 0x8A;

    constexpr uint8_t ADDRESS_ESC_MIN = 0x90;
    constexpr uint8_t ADDRESS_ESC_MAX = 0x97;

    constexpr uint8_t ADDRESS_CROSSFIRE_RESERVED_B0 = 0xB0;
    constexpr uint8_t ADDRESS_CROSSFIRE_RESERVED_B2 = 0xB2;

    constexpr uint8_t ADDRESS_CURRENT_SENSOR = 0xC0;
    constexpr uint8_t ADDRESS_GPS = 0xC2;
    constexpr uint8_t ADDRESS_BLACKBOX = 0xC4;
    constexpr uint8_t ADDRESS_FLIGHT_CONTROLLER = 0xC8;
    constexpr uint8_t ADDRESS_RESERVED_CA = 0xCA;
    constexpr uint8_t ADDRESS_RACE_TAG = 0xCC;
    constexpr uint8_t ADDRESS_VTX = 0xCE;

    constexpr uint8_t ADDRESS_REMOTE_CONTROL = 0xEA;
    constexpr uint8_t ADDRESS_REPEATER_RECEIVER = 0xEB;
    constexpr uint8_t ADDRESS_RECEIVER = 0xEC;
    constexpr uint8_t ADDRESS_REPEATER_TRANSMITTER = 0xED;
    constexpr uint8_t ADDRESS_TRANSMITTER = 0xEE;

    constexpr uint8_t ADDRESS_RESERVED_F0 = 0xF0;
    constexpr uint8_t ADDRESS_RESERVED_F2 = 0xF2;


    // -------------------------------------------------------------------------
    // Frame-start validation
    // -------------------------------------------------------------------------

    inline bool isValidSyncByte(
        uint8_t value
    )
    {
        if (value == SYNC_BYTE)
        {
            return true;
        }


        if (value == ADDRESS_BROADCAST)
        {
            return true;
        }


        if (
            value >= ADDRESS_DYNAMIC_MIN &&
            value <= ADDRESS_DYNAMIC_MAX
        )
        {
            return true;
        }


        if (
            value >= ADDRESS_ESC_MIN &&
            value <= ADDRESS_ESC_MAX
        )
        {
            return true;
        }


        switch (value)
        {
            case ADDRESS_CLOUD:
            case ADDRESS_USB:
            case ADDRESS_BLUETOOTH_WIFI:
            case ADDRESS_WIFI_RECEIVER:
            case ADDRESS_VIDEO_RECEIVER:

            case ADDRESS_OSD:
            case ADDRESS_RESERVED_8A:

            case ADDRESS_CROSSFIRE_RESERVED_B0:
            case ADDRESS_CROSSFIRE_RESERVED_B2:
            case ADDRESS_CURRENT_SENSOR:
            case ADDRESS_GPS:
            case ADDRESS_BLACKBOX:
            case ADDRESS_RESERVED_CA:
            case ADDRESS_RACE_TAG:
            case ADDRESS_VTX:

            case ADDRESS_REMOTE_CONTROL:
            case ADDRESS_REPEATER_RECEIVER:
            case ADDRESS_RECEIVER:
            case ADDRESS_REPEATER_TRANSMITTER:
            case ADDRESS_TRANSMITTER:
            case ADDRESS_RESERVED_F0:
            case ADDRESS_RESERVED_F2:
            {
                return true;
            }


            default:
            {
                return false;
            }
        }
    }


    // -------------------------------------------------------------------------
    // Broadcast frame types
    // -------------------------------------------------------------------------

    constexpr uint8_t FRAME_LINK_STATISTICS = 0x14;
    constexpr uint8_t FRAME_RC_CHANNELS = 0x16;


    // -------------------------------------------------------------------------
    // Extended frame types
    // -------------------------------------------------------------------------

    constexpr uint8_t FRAME_DEVICE_PING = 0x28;

    constexpr uint8_t FRAME_DEVICE_INFO = 0x29;

    constexpr uint8_t FRAME_PARAMETER_SETTINGS_ENTRY = 0x2B;

    constexpr uint8_t FRAME_PARAMETER_READ = 0x2C;

    constexpr uint8_t FRAME_PARAMETER_WRITE = 0x2D;

    constexpr uint8_t FRAME_COMMAND = 0x32;


    // -------------------------------------------------------------------------
    // CRSF parameter data types
    //
    // Integer parameter types 0x00-0x05 exist for legacy compatibility but
    // are deliberately not introduced here. New numeric settings use FLOAT.
    // -------------------------------------------------------------------------

    constexpr uint8_t PARAMETER_TYPE_FLOAT = 0x08;
    constexpr uint8_t PARAMETER_TYPE_TEXT_SELECTION = 0x09;
    constexpr uint8_t PARAMETER_TYPE_STRING = 0x0A;
    constexpr uint8_t PARAMETER_TYPE_FOLDER = 0x0B;
    constexpr uint8_t PARAMETER_TYPE_INFO = 0x0C;
    constexpr uint8_t PARAMETER_TYPE_COMMAND = 0x0D;
    constexpr uint8_t PARAMETER_TYPE_OUT_OF_RANGE = 0x7F;


    // -------------------------------------------------------------------------
    // CRC
    // -------------------------------------------------------------------------

    constexpr uint8_t CRC_POLYNOMIAL = 0xD5;


    // -------------------------------------------------------------------------
    // Standard packed RC channel frame (0x16)
    //
    // 16 channels * 11 bits = 176 bits = 22 bytes
    // -------------------------------------------------------------------------

    constexpr uint8_t RC_CHANNEL_COUNT = 16;

    constexpr uint8_t RC_CHANNEL_BITS = 11;

    constexpr uint8_t RC_CHANNEL_PAYLOAD_SIZE = 22;

    constexpr uint16_t RC_CHANNEL_MASK = 0x07FF;
}
