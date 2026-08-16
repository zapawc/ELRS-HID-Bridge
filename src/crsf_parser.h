#pragma once

#include <stddef.h>
#include <stdint.h>

#include "crsf_frame.h"


class CrsfParser
{
public:
    // Reset the parser to its initial synchronization state.
    void reset();


    // Consume one byte from the CRSF byte stream.
    //
    // Returns true only when a complete, CRC-valid CRSF frame has
    // been assembled.
    //
    // When true is returned, frame contains a transient view into
    // the parser's internal receive buffer. The frame must therefore
    // be consumed before another byte is supplied to this parser.
    bool pushByte(
        uint8_t byte,
        CrsfFrame& frame
    );


private:
    // Maximum complete CRSF frame size:
    //
    // Address        1 byte
    // Length         1 byte
    // Frame contents up to 62 bytes
    //
    // Total maximum = 64 bytes
    static constexpr size_t MAX_FRAME_SIZE = 64;


    uint8_t frameBuffer[MAX_FRAME_SIZE] = {};


    // Number of bytes currently stored in frameBuffer.
    size_t frameIndex = 0;


    // Expected total frame size once the CRSF length byte
    // has been received.
    size_t expectedFrameSize = 0;


    // Validate CRC and construct a CrsfFrame view from the
    // completed receive buffer.
    bool processFrame(
        CrsfFrame& frame
    );
};