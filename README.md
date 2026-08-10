# ELRS HID Bridge

Convert an ExpressLRS (CRSF) receiver into a standard USB HID joystick using an Adafruit QT Py RP2040.

The goal of this project is to create an inexpensive, open-source USB simulator dongle for FPV flight simulators such as Liftoff, VelociDrone, DRL, TRYP FPV, and others.

Unlike many existing simulator dongles, ELRS-HID-Bridge communicates directly with modern ExpressLRS receivers over the CRSF protocol while presenting itself to the host computer as a standard USB HID joystick. No drivers or proprietary software are required.

---

## Project Status

**Current Version:** 0.3

🚧 **Active Development**

Current development is focused on hardware integration with a real ExpressLRS receiver.

Completed:

- ✅ USB HID joystick implementation
- ✅ Modular firmware architecture
- ✅ CRSF frame parser
- ✅ CRC validation
- ✅ RC channel decoder
- ✅ Deterministic protocol self-tests
- ✅ RGB status LED
- ✅ Optional debug framework

Current work:

- 🔄 UART receiver integration
- 🔄 Live CRSF decoding

---

# Design Goals

- Open Source
- Plug-and-play USB HID joystick
- Low cost hardware
- Easy to build
- Well documented
- Modular firmware architecture
- Support for future receiver protocols

---

# Features

## Current

- USB HID joystick
- X/Y axis
- Two sliders
- 32 buttons
- CRSF protocol support
- Deterministic startup self-tests
- RGB status LED
- Optional USB debug logging

## Planned

- Link statistics
- Failsafe detection
- Configurable channel mapping
- Persistent configuration
- Calibration
- Maintenance mode
- SBUS support
- iBUS support

---

# Firmware Architecture

```
                 UART
                   │
                   ▼
            CRSF Decoder
                   │
                   ▼
          RC Channel Decoder
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
             │          │
             ▼          ▼
        Status LED   USB HID
```

The architecture is intentionally layered so that transport, protocol, application logic and USB presentation remain independent.

This makes the project easier to maintain and simplifies future expansion.

---

# Hardware

Current target platform:

- Adafruit QT Py RP2040

Current receiver:

- ExpressLRS CRSF UART receiver

---

# Building

The project uses:

- PlatformIO
- Visual Studio Code
- Raspberry Pi Pico Arduino Core
- TinyUSB

Clone the repository:

```bash
git clone https://github.com/zapawc/ELRS-HID-Bridge.git
```

Open the project in Visual Studio Code with the PlatformIO extension installed.

---

# Repository Layout

```
docs/
    Architecture.md
    Protocol.md
    Roadmap.md

src/
    ...
```

Additional documentation is available in the `docs` directory.

---

# Why another simulator dongle?

Most existing simulator dongles:

- Depend on older radio protocols
- Require proprietary firmware
- Are difficult to modify
- Have limited documentation

ELRS-HID-Bridge is intended to be:

- Modern
- Open
- Well documented
- Easy to extend
- Easy to repair

---

# Contributing

The project is under active development.

Ideas, bug reports and pull requests are welcome.

As additional receiver protocols are implemented, the firmware architecture is intended to make adding support straightforward.

---

# Roadmap

See:

```
docs/Roadmap.md
```

---

# License

License selection is pending.

The project will be released under an open-source license prior to Version 1.0.

---

# Acknowledgements

This project builds upon the excellent work of:

- ExpressLRS
- Team BlackSheep CRSF protocol
- Adafruit
- Earle Philhower's RP2040 Arduino Core
- TinyUSB