#include "debug_log.h"

#if ELRS_DEBUG

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

void DebugLog::begin()
{
    Serial.begin(115200);

    // Do not block startup waiting for a terminal.
    delay(100);

    Serial.println();
    Serial.println("[ELRS-HID-Bridge] Debug logging enabled");
}

void DebugLog::info(const char* message)
{
    Serial.println(message);
}

void DebugLog::value(
    const char* label,
    uint32_t value
)
{
    Serial.print(label);
    Serial.println(value);
}

#else

void DebugLog::begin()
{
}

void DebugLog::info(const char*)
{
}

void DebugLog::value(
    const char*,
    uint32_t
)
{
}

#endif