#pragma once

#include <stdint.h>

namespace Crsf
{
    constexpr uint8_t ADDRESS_BROADCAST = 0x00;
    constexpr uint8_t ADDRESS_FLIGHT_CONTROLLER = 0xC8;
    constexpr uint8_t ADDRESS_RECEIVER = 0xEC;
    constexpr uint8_t ADDRESS_TRANSMITTER = 0xEE;

    constexpr uint8_t FRAME_LINK_STATISTICS = 0x14;
    constexpr uint8_t FRAME_RC_CHANNELS = 0x16;

    constexpr uint8_t CRC_POLYNOMIAL = 0xD5;
}