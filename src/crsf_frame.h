#pragma once

#include <stdint.h>

struct CrsfFrame
{
    uint8_t address = 0;
    uint8_t length = 0;
    uint8_t type = 0;

    const uint8_t* payload = nullptr;
    uint8_t payloadLength = 0;
};