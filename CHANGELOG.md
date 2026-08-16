# Changelog

All notable ELRS-HID-Bridge changes intended for public releases are summarized here.

The project uses semantic versioning for firmware identity. During the v1.0 release-candidate cycle, prerelease labels are human-readable metadata and are not encoded in the CRSF numeric Firmware ID.

## [1.0.0-rc1] - 2026-08-16

First v1.0 release candidate. Runtime behavior is feature-frozen pending final regression and documentation validation.

### Added

- robust CRSF stream parsing with CRC validation and malformed/partial stream recovery
- explicit 16-channel 11-bit RC channel decoding
- Link Statistics decoding for diagnostics without treating telemetry as RC-health state
- standard USB HID joystick with eight analog axes and 32-button capacity
- reference EdgeTX switch mapping using Buttons 1-14
- deterministic 500 ms receiver-loss failsafe across every HID control
- automatic HID recovery when valid RC frames return
- QT Py RGB status display and BOOT-button diagnostic/maintenance UI
- clean parser/dispatcher, state, configuration, failsafe, HID, UART, and UI boundaries
- bidirectional CRSF UART support
- Device Ping (`0x28`) recognition and self-tested Device Info (`0x29`) encoding
- live CRSF identity discovery through RP2/Ranger/EdgeTX under **Other Devices**
- canonical firmware version source and deterministic CRSF Firmware ID encoding
- GPL-3.0-only licensing and third-party notices
- pinned known-good Arduino-Pico/toolchain/NeoPixel release dependencies
- release-candidate regression and UF2 staging/checksum workflow

### Changed

- USB product and HID interface identity standardized as `ELRS-HID-Bridge`
- USB manufacturer standardized as `zapawc`
- canonical firmware version advanced from `0.3.0-dev` to `1.0.0-rc1`
- CRSF Firmware ID advanced from `0x00030000` to `0x01000000`

### Validated

- wireless Liftoff operation through ELRS -> RP2 -> QT Py RP2040 -> USB HID
- 333 Hz Full / 16ch Rate/2 control path with live CRSF Device Info TX enabled
- deterministic transmitter-off failsafe and automatic reconnect
- EdgeTX discovery of `ELRS-HID-Bridge` under **Other Devices**
- Windows bus-reported USB identity `ELRS-HID-Bridge`

### Known limitations

- With the tested ExpressLRS 3.3.1 receiver firmware, CH15 and CH16 remained high even in the tested Full-resolution configuration; diagnostic remapping showed this behavior follows the upstream CRSF channels rather than the HID axes.
- Windows `joy.cpl` may display the controller as `Pico` because v1.0 retains inherited VID/PID `0x2E8A:0x000A`; Windows bus-reported product identity is correctly `ELRS-HID-Bridge`.
- BOOT-button Bind and Wi-Fi selections are UI placeholders only; receiver commands are not executed.
- CRSF parameter configuration and persistent settings are intentionally post-v1.0 features.

## Pre-release development

Development before `1.0.0-rc1` established and validated the core architecture and reference hardware behavior. Detailed checkpoint history remains available in the Git repository and `docs/Roadmap.md`.
