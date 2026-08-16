#include <Arduino.h>

#include "crsf_uart.h"


namespace
{
    // Adafruit QT Py RP2040 physical UART pads:
    //
    // TX/D6 pad -> RP2040 GPIO20 -> UART1 TX
    // RX/D7 pad -> RP2040 GPIO5  -> UART1 RX
    //
    // The project currently builds against the generic Pico
    // board definition, so the actual RP2040 GPIO numbers are used.

    constexpr uint8_t CRSF_TX_PIN = 20;
    constexpr uint8_t CRSF_RX_PIN = 5;

    constexpr uint32_t CRSF_BAUD = 420000;

    constexpr size_t UART_FIFO_SIZE = 256;
}


void CrsfUart::begin()
{
    receiveByteCount = 0;
    transmitByteCount = 0;

    Serial2.setTX(CRSF_TX_PIN);
    Serial2.setRX(CRSF_RX_PIN);

    Serial2.setFIFOSize(UART_FIFO_SIZE);
    Serial2.begin(CRSF_BAUD);
}


bool CrsfUart::readByte(
    uint8_t& byte
)
{
    if (Serial2.available() <= 0)
    {
        return false;
    }


    const int received =
        Serial2.read();


    if (received < 0)
    {
        return false;
    }


    byte =
        static_cast<uint8_t>(
            received
        );


    ++receiveByteCount;


    return true;
}


size_t CrsfUart::write(
    const uint8_t* data,
    size_t length
)
{
    if (
        data == nullptr ||
        length == 0
    )
    {
        return 0;
    }


    const size_t written =
        Serial2.write(
            data,
            length
        );


    transmitByteCount +=
        static_cast<uint32_t>(
            written
        );


    return written;
}


bool CrsfUart::hasReceivedData() const
{
    return receiveByteCount > 0;
}


uint32_t CrsfUart::totalBytesReceived() const
{
    return receiveByteCount;
}


uint32_t CrsfUart::totalBytesTransmitted() const
{
    return transmitByteCount;
}