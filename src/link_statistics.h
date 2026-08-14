#pragma once

#include <stdint.h>

struct LinkStatistics
{
    uint8_t uplinkRssiAntenna1 = 0;
    uint8_t uplinkRssiAntenna2 = 0;
    uint8_t uplinkLinkQuality = 0;
    int8_t uplinkSnr = 0;

    uint8_t activeAntenna = 0;
    uint8_t rfProfile = 0;
    uint8_t uplinkRfPower = 0;

    uint8_t downlinkRssi = 0;
    uint8_t downlinkLinkQuality = 0;
    int8_t downlinkSnr = 0;
};