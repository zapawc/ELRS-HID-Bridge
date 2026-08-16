#pragma once

#include <stddef.h>
#include <stdint.h>


class CrsfUart
{
public:
    // Initialize the physical CRSF UART.
    void begin();


    // Read one byte from the receive FIFO.
    //
    // Returns true when a byte was successfully read.
    // Returns false when no receive data is currently available.
    //
    // CrsfUart deliberately has no knowledge of the CRSF parser,
    // decoder, dispatcher, or any other protocol consumer.
    bool readByte(
        uint8_t& byte
    );


    // Transmit raw bytes over the CRSF UART.
    //
    // Returns the number of bytes accepted by the underlying
    // UART implementation.
    //
    // This provides the transport primitive required for future
    // outbound CRSF frames without introducing device/protocol
    // behavior into the UART layer.
    size_t write(
        const uint8_t* data,
        size_t length
    );


    bool hasReceivedData() const;

    uint32_t totalBytesReceived() const;

    uint32_t totalBytesTransmitted() const;


private:
    uint32_t receiveByteCount = 0;
    uint32_t transmitByteCount = 0;
};