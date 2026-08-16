#pragma once

#include <stdint.h>

#include "link_statistics.h"


class BridgeState
{
public:
    void reset();


    // -------------------------------------------------------------------------
    // Observed transport/protocol state
    // -------------------------------------------------------------------------

    void noteUartActivity();

    void noteRcFrame(
        uint32_t nowMs
    );

    void noteLinkStatistics(
        const LinkStatistics& statistics
    );


    // -------------------------------------------------------------------------
    // RC timeout state
    // -------------------------------------------------------------------------

    // Evaluate the RC-channel timeout.
    //
    // Returns true only when this call causes a transition from an
    // active RC state into the receiver-lost state.
    //
    // Link Statistics do not affect this timeout. Only receipt of a
    // valid RC channel frame refreshes lastValidRcFrameMs.
    bool updateRcTimeout(
        uint32_t nowMs,
        uint32_t timeoutMs
    );


    // -------------------------------------------------------------------------
    // State access
    // -------------------------------------------------------------------------

    bool hasReceiverBytes() const;

    bool hasRcFrames() const;

    bool isReceiverLost() const;

    bool hasLinkStatistics() const;


    uint32_t lastRcFrameTimeMs() const;

    uint32_t failsafeCount() const;


    const LinkStatistics&
    linkStatistics() const;


private:
    bool receiverBytesSeen = false;
    bool rcFramesSeen = false;
    bool receiverLost = false;

    bool linkStatisticsSeen = false;


    uint32_t lastValidRcFrameMs = 0;
    uint32_t receiverFailsafeCount = 0;


    LinkStatistics latestLinkStatistics = {};
};