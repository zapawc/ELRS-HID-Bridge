# ELRS-HID-Bridge v1.0.0-rc1

This is the first v1.0 release candidate for ELRS-HID-Bridge, a two-component ExpressLRS/CRSF-to-USB HID bridge using an Adafruit QT Py RP2040 and a RadioMaster RP2 receiver.

## Highlights

- wireless standard USB HID joystick operation validated in Liftoff
- eight analog HID axes plus 32-button capacity
- deterministic 500 ms receiver-loss failsafe across every HID control
- automatic recovery when the transmitter returns
- robust CRSF parsing/CRC handling and Link Statistics diagnostics
- EdgeTX discovery as `ELRS-HID-Bridge` under **Other Devices**
- bidirectional CRSF transport foundation with identity-only Device Info response
- BOOT-button diagnostics/maintenance selection UI using the QT Py's onboard hardware
- GPL-3.0-only licensing
- pinned known-good release dependencies and versioned UF2/checksum workflow

## Reference hardware

```text
Adafruit QT Py RP2040
RadioMaster RP2 ExpressLRS receiver
```

Reference wiring:

```text
RP2 5V  -> QT Py 5V
RP2 GND -> QT Py GND
RP2 TX  -> QT Py RX
RP2 RX  -> QT Py TX
```

Reference ExpressLRS configuration:

```text
333 Hz Full
16ch Rate/2
```

## Installation

Use the attached:

```text
ELRS-HID-Bridge-v1.0.0-rc1.uf2
```

Flash it using the QT Py RP2040's normal BOOTSEL/UF2 procedure.

No custom Windows driver or companion application is required for normal HID operation.

## Known limitations

- With the tested ExpressLRS 3.3.1 receiver firmware, CH15/CH16 remained high. Diagnostic remapping showed this behavior follows the upstream CRSF channels rather than the HID axes.
- Windows `joy.cpl` may display the controller as `Pico` because this release candidate retains inherited VID/PID `0x2E8A:0x000A`. Windows bus-reported product identity is correctly `ELRS-HID-Bridge`; the label is cosmetic.
- BOOT-button Bind and Wi-Fi selections are currently non-destructive placeholders; receiver commands are not sent.
- CRSF parameter configuration and persistent settings remain post-v1.0 work.

## Validation request

For RC testing, please report:

- hardware used
- ExpressLRS/EdgeTX versions and packet/channel mode
- Windows/HID behavior
- simulator/application tested
- any axis/button/failsafe discrepancy
- whether `ELRS-HID-Bridge` appears under EdgeTX **Other Devices**

See `CHANGELOG.md` and `docs/Release-Checklist.md` for the detailed feature and validation baseline.
