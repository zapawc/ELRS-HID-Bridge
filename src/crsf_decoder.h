#pragma once

#include <stddef.h>
#include <stdint.h>

#include "crsf_frame.h"
#include "link_statistics.h"
#include "link_statistics_decoder.h"
#include "raw_channels.h"
#include "rc_channel_decoder.h"

class CrsfDecoder
{
public:
    void reset();

    void pushByte(uint8_t byte);

    bool hasNewChannels() const;
    const RawChannels& getChannels() const;
    void clearNewChannels();

    bool hasNewLinkStatistics() const;
    const LinkStatistics& getLinkStatistics() const;
    void clearNewLinkStatistics();

private:
    static constexpr size_t MAX_FRAME_SIZE = 64;

    uint8_t frameBuffer[MAX_FRAME_SIZE] = {};

    size_t frameIndex = 0;
    size_t expectedFrameSize = 0;

    bool newChannels = false;
    bool newLinkStatistics = false;

    RawChannels channels;
    LinkStatistics linkStatistics;

    RcChannelDecoder rcChannelDecoder;
    LinkStatisticsDecoder linkStatisticsDecoder;

    void processFrame();
    void dispatchFrame(const CrsfFrame& frame);
};