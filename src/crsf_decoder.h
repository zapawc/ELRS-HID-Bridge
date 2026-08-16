#pragma once

#include <stdint.h>
#include "crsf_device.h"
#include "crsf_dispatcher.h"
#include "crsf_parser.h"
#include "link_statistics.h"
#include "raw_channels.h"


class CrsfDecoder
{
public:
    void reset();


    // Compatibility facade for the CRSF receive path.
    //
    // Bytes are passed to CrsfParser. Complete validated frames
    // are synchronously routed through CrsfDispatcher.
    void pushByte(
        uint8_t byte
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
    CrsfParser parser;

    CrsfDispatcher dispatcher;
};
