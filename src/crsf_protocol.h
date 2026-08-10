#pragma once

#include <stdint.h>

namespace Crsf
{
    // -------------------------------------------------------------------------
    // Device addresses
    // -------------------------------------------------------------------------

    constexpr uint8_t ADDRESS_BROADCAST = 0x00;
    constexpr uint8_t ADDRESS_FLIGHT_CONTROLLER = 0xC8;
    constexpr uint8_t ADDRESS_RECEIVER = 0xEC;
    constexpr uint8_t ADDRESS_TRANSMITTER = 0xEE;

    // -------------------------------------------------------------------------
    // Frame types
    // -------------------------------------------------------------------------

    constexpr uint8_t FRAME_LINK_STATISTICS = 0x14;
    constexpr uint8_t FRAME_RC_CHANNELS = 0x16;

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