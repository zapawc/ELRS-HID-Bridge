# ELRS HID Bridge Architecture

**Version:** 0.2  
**Status:** Draft

---

## Overview

ELRS-HID-Bridge converts an ExpressLRS / CRSF receiver into a standard USB HID joystick using an Adafruit QT Py RP2040.

The project is intentionally designed as a series of loosely-coupled processing stages. Each stage has a single responsibility and communicates with the next stage through simple data structures.

This architecture allows each module to be developed and tested independently while making it easier to support additional receiver protocols and USB device types in the future.

---

## Data Flow

```
            Hardware UART
                  │
                  ▼
           CRSF Byte Stream
                  │
                  ▼
            CrsfDecoder
        (Frame Assembly)
                  │
                  ▼
          CRC Validation
                  │
                  ▼
         Frame Dispatcher
                  │
                  ▼
        RcChannelDecoder
                  │
                  ▼
            RawChannels
                  │
                  ▼
        ChannelNormalizer
                  │
                  ▼
       NormalizedChannels
                  │
                  ▼
          ChannelMapper
                  │
                  ▼
           ChannelState
                  │
                  ▼
              UsbHid
                  │
                  ▼
          Windows USB HID
```

---

## Module Responsibilities

### CrsfDecoder

Receives a continuous stream of bytes from the UART.

Responsibilities:

- Assemble CRSF frames
- Validate CRC
- Dispatch validated frames to protocol-specific decoders

The decoder intentionally does not understand RC channel data.

---

### RcChannelDecoder

Decodes CRSF RC Channel (0x16) frames into RawChannels.

Responsibilities:

- Decode packed 11-bit channel values
- Populate RawChannels

This module contains protocol-specific knowledge.

---

### RawChannels

Represents channel values exactly as transmitted by the receiver.

These values remain protocol-specific.

---

### ChannelNormalizer

Converts protocol-specific channel values into a normalized 16-bit representation.

Responsibilities:

- Convert protocol ranges to 0-65535
- Provide a common representation for the remainder of the application

---

### NormalizedChannels

Protocol-independent representation of all channels.

Future receiver protocols should produce identical normalized values.

---

### ChannelMapper

Maps normalized input channels to application controls.

Current mapping:

- CH1 → Roll
- CH2 → Pitch
- CH3 → Throttle
- CH4 → Yaw
- CH5-CH16 → Buttons

Future releases will allow configurable mappings.

---

### ChannelState

Represents the current HID state.

This structure contains only semantic controls and has no knowledge of receiver protocols.

---

### UsbHid

Converts ChannelState into USB HID reports.

This module has no knowledge of CRSF, RC receivers, or channel normalization.

---

## Design Principles

- Single Responsibility Principle
- Testable modules
- Protocol-independent application layer
- Reusable protocol implementation
- Clean separation between transport, protocol, application, and presentation