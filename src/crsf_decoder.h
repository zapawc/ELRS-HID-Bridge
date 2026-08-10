#pragma once

#include <stddef.h>
#include <stdint.h>

#include "crsf_frame.h"
#include "raw_channels.h"
#include "rc_channel_decoder.h"

class CrsfDecoder
{
public:
    void reset();

    // Feed one byte from the CRSF serial stream.
    void pushByte(uint8_t byte);

    // True after a valid RC channel frame has been decoded.
    bool hasNewChannels() const;

    const RawChannels& getChannels() const;

    // Clear the new-data indication after consuming it.
    void clearNewChannels();

private:
    static constexpr size_t MAX_FRAME_SIZE = 64;

    uint8_t frameBuffer[MAX_FRAME_SIZE] = {};

    size_t frameIndex = 0;
    size_t expectedFrameSize = 0;

    bool newChannels = false;

    RawChannels channels;
    RcChannelDecoder rcChannelDecoder;

    void processFrame();
    void dispatchFrame(const CrsfFrame& frame);
};