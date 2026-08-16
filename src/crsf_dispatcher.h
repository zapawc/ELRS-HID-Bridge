#pragma once
#include "crsf_device.h"
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
    // frame-type consumer.
    //
    // Unsupported but otherwise valid frame types are intentionally
    // ignored.
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


    // -------------------------------------------------------------------------
    // CRSF device traffic
    // -------------------------------------------------------------------------

    bool hasDevicePing() const;


    const CrsfDevicePing&
    getDevicePing() const;


    void clearDevicePing();


    bool hasParameterRead() const;


    const CrsfParameterRead&
    getParameterRead() const;


    void clearParameterRead();


    bool hasParameterWrite() const;


    const CrsfParameterWrite&
    getParameterWrite() const;


    void clearParameterWrite();


private:
    bool newChannels = false;
    bool newLinkStatistics = false;


    RawChannels channels;

    LinkStatistics linkStatistics;


    RcChannelDecoder rcChannelDecoder;

    LinkStatisticsDecoder linkStatisticsDecoder;

    CrsfDevice crsfDevice;
};
