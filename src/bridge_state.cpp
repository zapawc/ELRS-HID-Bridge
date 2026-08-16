#include "bridge_state.h"


void BridgeState::reset()
{
    receiverBytesSeen = false;
    rcFramesSeen = false;
    receiverLost = false;

    linkStatisticsSeen = false;

    lastValidRcFrameMs = 0;
    receiverFailsafeCount = 0;

    latestLinkStatistics =
        LinkStatistics{};
}


void BridgeState::noteUartActivity()
{
    receiverBytesSeen = true;
}


void BridgeState::noteRcFrame(
    uint32_t nowMs
)
{
    // A valid decoded RC frame necessarily means that UART traffic
    // has been observed as well.
    receiverBytesSeen = true;

    rcFramesSeen = true;
    receiverLost = false;

    lastValidRcFrameMs =
        nowMs;
}


void BridgeState::noteLinkStatistics(
    const LinkStatistics& statistics
)
{
    // A valid decoded Link Statistics frame also necessarily means
    // that UART traffic has been observed.
    receiverBytesSeen = true;

    latestLinkStatistics =
        statistics;

    linkStatisticsSeen = true;
}


bool BridgeState::updateRcTimeout(
    uint32_t nowMs,
    uint32_t timeoutMs
)
{
    // A timeout cannot occur until at least one valid RC channel
    // frame has been received.
    if (!rcFramesSeen)
    {
        return false;
    }


    // Only report the transition into failsafe once.
    if (receiverLost)
    {
        return false;
    }


    // Unsigned subtraction intentionally preserves correct behavior
    // across the millis() counter rollover.
    if (
        (nowMs - lastValidRcFrameMs) <
        timeoutMs
    )
    {
        return false;
    }


    receiverLost = true;

    ++receiverFailsafeCount;


    return true;
}


bool BridgeState::hasReceiverBytes() const
{
    return receiverBytesSeen;
}


bool BridgeState::hasRcFrames() const
{
    return rcFramesSeen;
}


bool BridgeState::isReceiverLost() const
{
    return receiverLost;
}


bool BridgeState::hasLinkStatistics() const
{
    return linkStatisticsSeen;
}


uint32_t BridgeState::lastRcFrameTimeMs() const
{
    return lastValidRcFrameMs;
}


uint32_t BridgeState::failsafeCount() const
{
    return receiverFailsafeCount;
}


const LinkStatistics&
BridgeState::linkStatistics() const
{
    return latestLinkStatistics;
}