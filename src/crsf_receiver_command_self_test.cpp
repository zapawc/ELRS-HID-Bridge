#include "crsf_receiver_command_self_test.h"

#include <stddef.h>
#include <stdint.h>

#include "crsf_frame_encoder.h"
#include "crsf_receiver_command.h"

namespace
{
    bool buffersMatch(
        const uint8_t* actual,
        const uint8_t* expected,
        size_t length
    )
    {
        for (
            size_t index = 0;
            index < length;
            ++index
        )
        {
            if (actual[index] != expected[index])
            {
                return false;
            }
        }

        return true;
    }
}


bool CrsfReceiverCommandSelfTest::run()
{
    CrsfReceiverCommand commandBuilder;

    uint8_t encoded[
        CrsfFrameEncoder::MAX_FRAME_SIZE
    ] = {};

    size_t encodedLength = 0;

    // Independent golden vector from Betaflight's crsfRxBind():
    //
    // C8 07 32 EC C8 10 01 9E E8
    //
    // 0x9E = command CRC8 (poly 0xBA)
    // 0xE8 = CRSF packet CRC8 (poly 0xD5)
    constexpr uint8_t expected[] =
    {
        0xC8,
        0x07,
        0x32,
        0xEC,
        0xC8,
        0x10,
        0x01,
        0x9E,
        0xE8
    };

    if (
        !commandBuilder.buildBind(
            encoded,
            sizeof(encoded),
            encodedLength
        )
    )
    {
        return false;
    }

    if (encodedLength != sizeof(expected))
    {
        return false;
    }

    return buffersMatch(
        encoded,
        expected,
        sizeof(expected)
    );
}
