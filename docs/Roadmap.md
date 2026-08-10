# Project Roadmap

**Version:** 0.3  
**Status:** Active Development

---

# Current Status

The project has completed the core firmware architecture and is entering hardware integration.

Current focus:

- UART receiver integration
- Live CRSF decoding
- Hardware validation

---

# Version 0.1 - Foundation

## Completed

- [x] PlatformIO project
- [x] GitHub repository
- [x] USB HID joystick
- [x] Synthetic HID test generator

---

# Version 0.2 - Architecture

## Completed

- [x] Modular firmware architecture
- [x] CRSF frame assembler
- [x] CRC validation
- [x] Frame dispatcher
- [x] Raw channel abstraction
- [x] Channel normalization
- [x] Channel mapping
- [x] Deterministic protocol self-tests
- [x] Optional USB debug logging
- [x] RGB status LED framework
- [x] Project documentation

---

# Version 0.3 - Hardware Integration

## In Progress

- [ ] UART driver
- [ ] ELRS receiver integration
- [ ] Live CRSF frame decoding
- [ ] Receiver state machine
- [ ] Link detection
- [ ] Hardware validation

---

# Version 0.4 - Diagnostics

## Planned

- [ ] Link statistics (CRSF 0x14)
- [ ] CRC error indication
- [ ] Receiver failsafe indication
- [ ] Rich RGB LED status patterns
- [ ] Runtime debug messages
- [ ] Startup diagnostics

---

# Version 0.5 - Configuration

## Planned

- [ ] Configurable channel mapping
- [ ] Persistent configuration
- [ ] Calibration
- [ ] Maintenance pushbutton
- [ ] Factory reset
- [ ] USB configuration interface

---

# Version 0.6 - Platform Expansion

## Planned

- [ ] SBUS support
- [ ] iBUS support
- [ ] Additional receiver protocols
- [ ] Multiple HID layouts
- [ ] Multiple joystick profiles

---

# Version 1.0 - Initial Release

## Planned

- [ ] Stable firmware
- [ ] Complete documentation
- [ ] Wiring guide
- [ ] Enclosure files
- [ ] Build guide
- [ ] Release binaries
- [ ] Versioned releases

---

# Future Ideas

Potential future enhancements:

- Bluetooth HID
- XInput support
- Companion desktop utility
- Firmware update utility
- OLED status display
- CRSF telemetry decoding
- Multiple receiver profiles
- Automatic protocol detection