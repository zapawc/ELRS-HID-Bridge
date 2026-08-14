#pragma once

#include "crsf_frame.h"
#include "link_statistics.h"

class LinkStatisticsDecoder
{
public:
    bool decode(
        const CrsfFrame& frame,
        LinkStatistics& statistics
    ) const;
};