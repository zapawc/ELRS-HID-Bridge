# ELRS HID Bridge Architecture

**Version:** 0.3  
**Status:** Draft

---

# Overview

ELRS-HID-Bridge converts an ExpressLRS (CRSF) receiver into a standard USB HID joystick using an Adafruit QT Py RP2040.

The firmware is intentionally organized as a pipeline of small, loosely-coupled modules. Each module performs a single responsibility and communicates with the next stage through simple data structures.

This separation allows:

- Independent unit testing
- Easy protocol expansion
- Simple maintenance
- Reusable protocol code
- Clear separation between hardware, protocol, application logic and presentation

---

# Firmware Architecture

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
                      │         │
                      │         │
                      ▼         ▼
                  StatusLed   UsbHid
                                  │
                                  ▼
                          Windows HID Device
```

---

# Module Responsibilities

## CrsfDecoder

Consumes a continuous stream of bytes from the UART.

Responsibilities:

- Assemble CRSF frames
- Validate CRC
- Dispatch valid frames
- Ignore invalid frames

The decoder intentionally has no knowledge of RC channels.

---

## RcChannelDecoder

Decodes CRSF RC Channel (0x16) frames.

Responsibilities:

- Decode packed 11-bit channels
- Populate RawChannels

This module contains protocol-specific knowledge.

---

## RawChannels

Represents channel values exactly as transmitted by the receiver.

Typical CRSF values:

| Input | Meaning |
|-------:|---------|
| 172 | Minimum |
| 992 | Center |
| 1811 | Maximum |

No scaling occurs in this structure.

---

## ChannelNormalizer

Converts protocol-specific values into a common representation.

Responsibilities:

- Convert receiver values
- Produce normalized 16-bit values
- Isolate protocol-specific ranges

Output Range:

```
0
↓

65535
```

This allows future receiver protocols to share the same application layer.

---

## NormalizedChannels

Protocol-independent channel representation.

All future receiver implementations should produce identical normalized values regardless of the incoming protocol.

---

## ChannelMapper

Maps normalized channels into application controls.

Current Mapping

| Channel | Function |
|---------|----------|
| CH1 | Roll |
| CH2 | Pitch |
| CH3 | Throttle |
| CH4 | Yaw |
| CH5-CH16 | Buttons 1-12 |

Future versions will support configurable mappings.

---

## ChannelState

Represents the semantic joystick state.

Contains:

- Roll
- Pitch
- Throttle
- Yaw
- Buttons

This module has no knowledge of receiver protocols.

---

## UsbHid

Converts ChannelState into USB HID reports.

Responsibilities:

- USB enumeration
- HID report generation
- Communication with the host computer

No receiver-specific knowledge exists in this module.

---

## StatusLed

Controls the onboard QT Py RGB NeoPixel.

Current States

| State | Color |
|--------|-------|
| Startup | White |
| Ready | Blue |
| Error | Red |

Future firmware versions will expand this into a full diagnostic interface.

---

## CrsfSelfTest

Runs deterministic protocol validation during startup.

Current Tests

- Valid RC frame
- CRC validation
- 16-channel unpacking
- Invalid CRC rejection

A failed self-test:

- Sets Button 32
- Illuminates the status LED red

---

# Design Principles

The project follows several architectural principles.

## Single Responsibility

Every module performs one well-defined task.

---

## Layered Architecture

Information flows in one direction.

```
Hardware

↓

Protocol

↓

Application

↓

Presentation
```

No downstream layer should depend upon implementation details of an upstream layer.

---

## Protocol Independence

Only the protocol layer understands CRSF.

Everything above RawChannels is protocol independent.

This enables future support for:

- SBUS
- iBUS
- PPM
- Additional serial receiver protocols

without modifying the application layer.

---

## Deterministic Startup

Firmware validates itself during every boot before interacting with the receiver.

The onboard RGB LED and optional USB debug logging provide immediate diagnostic feedback.

---

## Testability

Where practical, modules are designed as pure transformations.

```
Input

↓

Output
```

This enables deterministic testing without requiring receiver hardware.

---

# Hardware

Current Target

- Adafruit QT Py RP2040

Current Interfaces

- USB-C
- CRSF UART
- RGB NeoPixel

---

## QT Py Notes

Because the firmware currently builds using the generic Raspberry Pi Pico PlatformIO target, the onboard NeoPixel requires explicit initialization.

| Function | GPIO |
|----------|------|
| NeoPixel Data | GPIO12 |
| NeoPixel Power Enable | GPIO11 |

The StatusLed module performs this initialization automatically.

---

# Future Expansion

Planned enhancements include:

- SBUS receiver support
- Link Statistics (CRSF 0x14)
- Receiver failsafe indication
- Optional USB debug interface
- Configurable channel mappings
- Persistent configuration storage
- Calibration
- Maintenance pushbutton
- Configuration utility

The architecture is intentionally designed so that these additions require minimal changes to existing modules.

---

# Current Project Status

Version: **0.3**

Completed

- USB HID implementation
- Modular firmware architecture
- CRSF frame assembly
- CRC validation
- Frame dispatcher
- RC channel decoder
- Raw channel abstraction
- Channel normalization
- Channel mapping
- Deterministic protocol self-tests
- RGB status LED

In Progress

- UART receiver integration

Planned

- Receiver diagnostics
- Configuration system
- Release documentation