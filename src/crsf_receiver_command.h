#pragma once

#include <stddef.h>
#include <stdint.h>

class CrsfReceiverCommand
{
public:
    // Build the standard CRSF receiver Bind command used by ExpressLRS 3.4+
    // and Betaflight 4.5+. The completed frame can be written directly to the
    // receiver-facing CRSF UART.
    bool buildBind(
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength
    ) const;

private:
    static uint8_t crc8Command(
        const uint8_t* data,
        size_t length
    );
};
