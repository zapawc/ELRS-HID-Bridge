# Changelog

All notable ELRS-HID-Bridge changes intended for public releases are summarized here.

The project uses semantic versioning for firmware identity. Prerelease labels are human-readable metadata and are not encoded in the CRSF numeric Firmware ID.

## [Unreleased]

Post-v1.0 feature work is hardware validated and currently feature-frozen pending documentation synchronization, full regression, version selection, and release packaging.

### Added

- EdgeTX/CRSF parameter configuration for bridge settings
- persistent `BridgeConfiguration` storage with validation, safe defaults, and migration/fallback behavior
- LED Brightness configuration
- configurable inversion for Pitch, Throttle, Roll, Yaw, and AUX 1-4
- Diagnostics folder with Failsafe Count
- Restore Defaults command
- BOOT-button receiver Bind action
- receiver command abstraction for supported CRSF maintenance commands
- startup regression coverage for parameter registry/order/serialization and throttle inversion writes

### Changed

- BOOT maintenance rotation simplified to Link Quality diagnostic, Bind, and No Action/Cancel
- maintenance selection remains a repeating 2-second state machine with execute-on-release safety
- `Throttle Invert` is the production throttle parameter label
- CRSF/EdgeTX parameter names are constrained to 16 visible characters or fewer unless longer names are explicitly revalidated
- reference receiver validation updated to RadioMaster RP2 / ExpressLRS 3.4.3
- CH15 and CH16 proportional shoulder-slider behavior validated after RP2 firmware upgrade

### Fixed

- configurable Throttle Inversion now correctly reverses HID Slider 1
- ExpressLRS device-menu enumeration stall caused by the previous 18-character `Throttle Inversion` label

### Compatibility

- BOOT-button receiver Bind requires ExpressLRS receiver firmware 3.4.0 or newer
- Bind hardware validated on ExpressLRS 3.4.3

### Removed / Deferred

- removed visible BOOT Wi-Fi selection because no supported FC-facing CRSF UART Wi-Fi command was identified
- retired receiver factory-reset BOOT action because no supported FC-facing CRSF UART reset command was identified
- Receiver Firmware Update / USB Serial Passthrough retained as future roadmap work

## [1.0.0] - 2026-08-16

Stable first release of the validated CRSF-to-USB HID joystick baseline.

### Added / Validated

- robust CRSF stream parsing with CRC validation and malformed/partial stream recovery
- explicit 16-channel RC decoding
- Link Statistics diagnostics
- standard USB HID joystick with eight analog axes and 32-button capacity
- deterministic 500 ms receiver-loss failsafe across every HID control
- automatic HID recovery after reconnect
- RGB status and BOOT diagnostic/maintenance UI
- clean parser/dispatcher, state, configuration, failsafe, HID, UART, and UI boundaries
- bidirectional CRSF UART support
- Device Ping / Device Info discovery through RP2/Ranger/EdgeTX
- canonical firmware version / CRSF Firmware ID
- GPL-3.0-only licensing
- pinned release dependencies and reproducible release staging/checksum workflow
- wireless Liftoff operation

### Known cosmetic limitation

Windows `joy.cpl` may display the controller as `Pico` because the reference build retains inherited VID/PID `0x2E8A:0x000A`; the bus-reported product identity is `ELRS-HID-Bridge`.

## [1.0.0-rc1] - 2026-08-16

First v1.0 release candidate. See Git history and release notes for the detailed RC validation record.
