#pragma once

#include <stdint.h>
#include <stddef.h>

#include "raw_channels.h"

class CrsfDecoder
{
public:
    void reset();

    // Feed one byte from the CRSF serial stream.
    void pushByte(uint8_t byte);

    // True after a valid RC channel frame has been decoded.
    bool hasNewChannels() const;

    const RawChannels& getChannels() const;

    // Clear the "new data" indication after consuming it.
    void clearNewChannels();

private:
    static constexpr size_t MAX_FRAME_SIZE = 64;

    uint8_t frameBuffer[MAX_FRAME_SIZE] = {};

    size_t frameIndex = 0;
    size_t expectedFrameSize = 0;

    bool newChannels = false;

    RawChannels channels;

    void processFrame();
};