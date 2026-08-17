#include "crsf_receiver_command.h"

#include "crsf_frame_encoder.h"
#include "crsf_protocol.h"

namespace
{
    // CRSF command namespace for receiver operations.
    constexpr uint8_t COMMAND_SUBCMD_RX = 0x10;
    constexpr uint8_t COMMAND_SUBCMD_RX_BIND = 0x01;

    // CRSF command frames carry an additional CRC8 using polynomial 0xBA.
    // This CRC covers Type + Destination + Origin + command payload.
    constexpr uint8_t COMMAND_CRC_POLYNOMIAL = 0xBA;
}


uint8_t CrsfReceiverCommand::crc8Command(
    const uint8_t* data,
    size_t length
)
{
    uint8_t crc = 0;

    for (
        size_t index = 0;
        index < length;
        ++index
    )
    {
        crc ^= data[index];

        for (
            uint8_t bit = 0;
            bit < 8;
            ++bit
        )
        {
            if (crc & 0x80)
            {
                crc = static_cast<uint8_t>(
                    (crc << 1) ^ COMMAND_CRC_POLYNOMIAL
                );
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}


bool CrsfReceiverCommand::buildBind(
    uint8_t* output,
    size_t outputCapacity,
    size_t& outputLength
) const
{
    outputLength = 0;

    // Command CRC input is exactly:
    //   Type 0x32
    //   Destination 0xEC (CRSF receiver)
    //   Origin 0xC8 (flight-controller side of the receiver UART)
    //   Receiver command group 0x10
    //   Bind subcommand 0x01
    constexpr uint8_t commandCrcInput[] =
    {
        Crsf::FRAME_COMMAND,
        Crsf::ADDRESS_RECEIVER,
        Crsf::ADDRESS_FLIGHT_CONTROLLER,
        COMMAND_SUBCMD_RX,
        COMMAND_SUBCMD_RX_BIND
    };

    uint8_t payload[] =
    {
        COMMAND_SUBCMD_RX,
        COMMAND_SUBCMD_RX_BIND,
        0x00
    };

    payload[2] = crc8Command(
        commandCrcInput,
        sizeof(commandCrcInput)
    );

    CrsfFrameEncoder encoder;

    return encoder.encodeExtended(
        Crsf::SYNC_BYTE,
        Crsf::FRAME_COMMAND,
        Crsf::ADDRESS_RECEIVER,
        Crsf::ADDRESS_FLIGHT_CONTROLLER,
        payload,
        sizeof(payload),
        output,
        outputCapacity,
        outputLength
    );
}
