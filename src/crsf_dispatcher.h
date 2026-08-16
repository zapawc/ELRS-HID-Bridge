#pragma once

#include "crsf_frame.h"
#include "link_statistics.h"
#include "link_statistics_decoder.h"
#include "raw_channels.h"
#include "rc_channel_decoder.h"


class CrsfDispatcher
{
public:
    void reset();


    // Route a previously validated CRSF frame to the appropriate
    // frame-type decoder.
    //
    // CrsfDispatcher does not perform framing, synchronization,
    // length validation, or CRC validation. Those responsibilities
    // belong to CrsfParser.
    //
    // Unsupported but otherwise valid CRSF frame types are
    // intentionally ignored.
    void dispatch(
        const CrsfFrame& frame
    );


    // -------------------------------------------------------------------------
    // RC channels
    // -------------------------------------------------------------------------

    bool hasNewChannels() const;


    const RawChannels&
    getChannels() const;


    void clearNewChannels();


    // -------------------------------------------------------------------------
    // Link Statistics
    // -------------------------------------------------------------------------

    bool hasNewLinkStatistics() const;


    const LinkStatistics&
    getLinkStatistics() const;


    void clearNewLinkStatistics();


private:
    bool newChannels = false;

    bool newLinkStatistics = false;


    RawChannels channels;

    LinkStatistics linkStatistics;


    RcChannelDecoder rcChannelDecoder;

    LinkStatisticsDecoder linkStatisticsDecoder;
};