#pragma once

#include "crsf_frame.h"
#include "raw_channels.h"

class RcChannelDecoder
{
public:
    // Decode a CRSF 0x16 packed RC-channel payload.
    //
    // Returns true when all 16 channels were decoded successfully.
    // Returns false if the frame is malformed or too short.
    bool decode(
        const CrsfFrame& frame,
        RawChannels& channels
    ) const;
};