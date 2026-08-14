#include "link_statistics_decoder.h"

#include "crsf_protocol.h"

namespace
{
    constexpr uint8_t LINK_STATISTICS_PAYLOAD_SIZE = 10;
}

bool LinkStatisticsDecoder::decode(
    const CrsfFrame& frame,
    LinkStatistics& statistics
) const
{
    if (frame.type != Crsf::FRAME_LINK_STATISTICS)
    {
        return false;
    }

    if (
        frame.payload == nullptr ||
        frame.payloadLength < LINK_STATISTICS_PAYLOAD_SIZE
    )
    {
        return false;
    }

    statistics.uplinkRssiAntenna1 =
        frame.payload[0];

    statistics.uplinkRssiAntenna2 =
        frame.payload[1];

    statistics.uplinkLinkQuality =
        frame.payload[2];

    statistics.uplinkSnr =
        static_cast<int8_t>(
            frame.payload[3]
        );

    statistics.activeAntenna =
        frame.payload[4];

    statistics.rfProfile =
        frame.payload[5];

    statistics.uplinkRfPower =
        frame.payload[6];

    statistics.downlinkRssi =
        frame.payload[7];

    statistics.downlinkLinkQuality =
        frame.payload[8];

    statistics.downlinkSnr =
        static_cast<int8_t>(
            frame.payload[9]
        );

    return true;
}