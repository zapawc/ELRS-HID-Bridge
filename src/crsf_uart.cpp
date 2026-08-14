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
    Serial2.setTX(CRSF_TX_PIN);
    Serial2.setRX(CRSF_RX_PIN);

    Serial2.setFIFOSize(UART_FIFO_SIZE);
    Serial2.begin(CRSF_BAUD);
}

void CrsfUart::update(CrsfDecoder& decoder)
{
    while (Serial2.available() > 0)
    {
        const int received = Serial2.read();

        if (received < 0)
        {
            continue;
        }

        ++byteCount;

        decoder.pushByte(
            static_cast<uint8_t>(received)
        );
    }
}

bool CrsfUart::hasReceivedData() const
{
    return byteCount > 0;
}

uint32_t CrsfUart::totalBytesReceived() const
{
    return byteCount;
}