#pragma once

#include <stdint.h>

class DebugLog
{
public:
    static void begin();

    static void info(const char* message);
    static void value(const char* label, uint32_t value);
};