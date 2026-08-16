#include "bridge_configuration_record.h"


namespace
{
    constexpr uint8_t MAGIC[4] =
    {
        'E',
        'H',
        'B',
        '1'
    };


    constexpr uint8_t PAYLOAD_LENGTH = 2;

    constexpr size_t CRC_INPUT_LENGTH = 12;

    constexpr uint32_t CRC32_POLYNOMIAL =
        0xEDB88320u;
}


uint32_t BridgeConfigurationRecord::crc32(
    const uint8_t* data,
    size_t length
)
{
    uint32_t crc =
        0xFFFFFFFFu;


    for (
        size_t index = 0;
        index < length;
        ++index
    )
    {
        crc ^=
            static_cast<uint32_t>(
                data[index]
            );


        for (
            uint8_t bit = 0;
            bit < 8;
            ++bit
        )
        {
            if (
                (crc & 1u) != 0
            )
            {
                crc =
                    (crc >> 1) ^
                    CRC32_POLYNOMIAL;
            }
            else
            {
                crc >>= 1;
            }
        }
    }


    return
        crc ^
        0xFFFFFFFFu;
}


void BridgeConfigurationRecord::writeUint32BigEndian(
    uint32_t value,
    uint8_t* output
)
{
    output[0] =
        static_cast<uint8_t>(
            (value >> 24) & 0xFFu
        );

    output[1] =
        static_cast<uint8_t>(
            (value >> 16) & 0xFFu
        );

    output[2] =
        static_cast<uint8_t>(
            (value >> 8) & 0xFFu
        );

    output[3] =
        static_cast<uint8_t>(
            value & 0xFFu
        );
}


uint32_t BridgeConfigurationRecord::readUint32BigEndian(
    const uint8_t* input
)
{
    return
        (
            static_cast<uint32_t>(
                input[0]
            ) << 24
        ) |
        (
            static_cast<uint32_t>(
                input[1]
            ) << 16
        ) |
        (
            static_cast<uint32_t>(
                input[2]
            ) << 8
        ) |
        static_cast<uint32_t>(
            input[3]
        );
}


bool BridgeConfigurationRecord::encode(
    const BridgeConfiguration& configuration,
    uint8_t* output,
    size_t outputCapacity
)
{
    if (
        output == nullptr ||
        outputCapacity <
            RECORD_SIZE ||
        configuration
            .ledBrightnessPercent > 100
    )
    {
        return false;
    }


    for (
        size_t index = 0;
        index < RECORD_SIZE;
        ++index
    )
    {
        output[index] = 0;
    }


    output[0] = MAGIC[0];
    output[1] = MAGIC[1];
    output[2] = MAGIC[2];
    output[3] = MAGIC[3];

    output[4] =
        SCHEMA_VERSION;

    output[5] =
        PAYLOAD_LENGTH;

    output[6] =
        configuration
            .ledBrightnessPercent;

    output[7] =
        configuration.pitch.inverted
            ? 1
            : 0;


    const uint32_t recordCrc =
        crc32(
            output,
            CRC_INPUT_LENGTH
        );


    writeUint32BigEndian(
        recordCrc,
        &output[12]
    );


    return true;
}


bool BridgeConfigurationRecord::decode(
    const uint8_t* data,
    size_t length,
    BridgeConfiguration& configuration
)
{
    if (
        data == nullptr ||
        length != RECORD_SIZE
    )
    {
        return false;
    }


    if (
        data[0] != MAGIC[0] ||
        data[1] != MAGIC[1] ||
        data[2] != MAGIC[2] ||
        data[3] != MAGIC[3]
    )
    {
        return false;
    }


    if (
        data[4] !=
            SCHEMA_VERSION ||
        data[5] !=
            PAYLOAD_LENGTH
    )
    {
        return false;
    }


    if (
        data[6] > 100 ||
        data[7] > 1
    )
    {
        return false;
    }


    const uint32_t expectedCrc =
        crc32(
            data,
            CRC_INPUT_LENGTH
        );


    const uint32_t storedCrc =
        readUint32BigEndian(
            &data[12]
        );


    if (
        expectedCrc !=
        storedCrc
    )
    {
        return false;
    }


    BridgeConfiguration decoded =
        BridgeConfiguration::defaults();


    decoded.ledBrightnessPercent =
        data[6];

    decoded.pitch.inverted =
        data[7] != 0;


    configuration =
        decoded;


    return true;
}
