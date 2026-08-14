# Project Roadmap

**Version:** 0.3  
**Status:** Active Development

---

# Current Status

ELRS-HID-Bridge has reached functional hardware integration.

The complete input path has been validated on physical hardware:

    EdgeTX
        ↓
    ExpressLRS TX
        ↓
    ELRS RF Link
        ↓
    ExpressLRS Receiver
        ↓
    CRSF UART
        ↓
    RP2040
        ↓
    CRSF Decoder
        ↓
    Channel Normalization
        ↓
    Channel Mapping
        ↓
    USB HID
        ↓
    Host PC

Current development is focused on diagnostics, configuration, documentation,
and preparing the project for broader testing.

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
- [x] CRSF stream resynchronization
- [x] Frame dispatcher
- [x] Raw channel abstraction
- [x] Channel normalization
- [x] Channel mapping
- [x] Deterministic protocol self-tests
- [x] RGB status LED framework
- [x] Project documentation

---

# Version 0.3 - Hardware Integration

## Completed

- [x] RP2040 hardware UART driver
- [x] ExpressLRS receiver integration
- [x] CRSF UART at 420000 baud
- [x] Live CRSF frame decoding
- [x] Live RC channel decoding
- [x] ELRS-to-USB HID control
- [x] Primary axis mapping
- [x] AUX switch mapping
- [x] ELRS AUX1 / arming-channel handling
- [x] Receiver link-loss detection
- [x] HID failsafe behavior
- [x] Automatic recovery after receiver reconnect
- [x] Hardware validation with RadioMaster RP2 receiver

## Current Recommended EdgeTX Layout

    CH1  Roll
    CH2  Pitch
    CH3  Throttle
    CH4  Yaw
    CH5  SF  - 2-position / ELRS AUX1
    CH6  SA  - 3-position
    CH7  SB  - 3-position
    CH8  SC  - 3-position
    CH9  SD  - 3-position
    CH10 SE  - 3-position
    CH11 SG  - 3-position
    CH12 SH  - momentary

---

# Version 0.4 - Diagnostics

## In Progress

- [x] Basic RGB status indication
- [x] Receiver activity indication
- [x] Valid CRSF frame indication
- [x] Receiver timeout indication
- [x] Startup protocol self-test

## Planned

- [ ] CRSF link statistics (0x14)
- [ ] Link quality monitoring
- [ ] RSSI monitoring
- [ ] CRC error tracking
- [ ] Frame error counters
- [ ] Rich RGB LED status patterns
- [ ] BOOT button input support
- [ ] Diagnostic / maintenance mode
- [ ] Runtime diagnostic interface
- [ ] Startup diagnostics

---

# Version 0.5 - Configuration

## Planned

- [ ] Configurable channel mapping
- [ ] Configurable axis inversion
- [ ] Configurable AUX switch types
- [ ] Persistent configuration
- [ ] Calibration
- [ ] BOOT-button configuration interface
- [ ] Factory reset
- [ ] USB configuration interface

---

# Version 0.6 - Platform Expansion

## Planned

- [ ] Additional ExpressLRS receiver validation
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
- [ ] Recommended EdgeTX configuration guide
- [ ] Wiring guide
- [ ] Wiring diagram
- [ ] Enclosure files
- [ ] Build guide
- [ ] Release binaries
- [ ] Versioned releases
- [ ] Open-source license finalized

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
- PC automation using transmitter switches
- DVR / OBS control
- Race-station controls
- Timestamped pilot-input logging
- Link-quality logging
- Live stick-position display