#pragma once

#include <stdint.h>

#include "crsf_decoder.h"

class CrsfUart
{
public:
    void begin();

    // Reads all currently available UART bytes and feeds them
    // into the supplied CRSF decoder.
    void update(CrsfDecoder& decoder);

    bool hasReceivedData() const;
    uint32_t totalBytesReceived() const;

private:
    uint32_t byteCount = 0;
};