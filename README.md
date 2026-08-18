# ELRS HID Bridge

Convert an ExpressLRS/CRSF receiver into a standard USB HID joystick using an Adafruit QT Py RP2040.

ELRS-HID-Bridge is an open-source CRSF-to-USB HID bridge and development foundation. The reference application is a wireless FPV simulator joystick, but the firmware is structured so the CRSF, state, mapping, HID, diagnostics, configuration, and hardware layers can be reused by other projects.

The reference build uses only two functional components:

- Adafruit QT Py RP2040
- RadioMaster RP2 ExpressLRS receiver

No display, external pushbutton, custom PCB, custom USB driver, or mandatory companion application is required.

---

## Project Status

**Current stable release:** `v1.1.0`

The v1.1.0 release adds transmitter-side CRSF configuration, persistent bridge settings, configurable inversion for all eight proportional HID axes, bridge diagnostics, Restore Defaults, and BOOT-button receiver Bind while preserving the validated v1.0.0 CRSF-to-HID control path.

### Proven on hardware

- 420000-baud CRSF UART with a RadioMaster RP2
- live CRSF frame reception
- frame synchronization, length validation, and CRC validation
- recovery after malformed/partial CRSF traffic
- explicit 16-channel 11-bit RC unpacking (`0x16`)
- Link Statistics decoding (`0x14`)
- USB HID joystick enumeration under Windows
- eight conventional DirectInput analog axes
- 32-button HID capacity
- deterministic 500 ms RC timeout/failsafe across all HID controls
- automatic recovery after transmitter reconnect
- RGB status indication
- onboard BOOT-button diagnostics/maintenance UI
- short-press Link Quality diagnostic
- bidirectional/TX-capable CRSF UART abstraction
- Device Ping (`0x28`) → Device Info (`0x29`) discovery
- CRSF parameter read/write configuration through EdgeTX
- persistent bridge configuration with safe defaults and migration behavior
- configurable inversion for all eight proportional HID axes
- Restore Defaults command
- Diagnostics folder with Failsafe Count
- receiver-side CRSF Bind from the BOOT maintenance UI
- successful wireless operation in Liftoff

### Reference compatibility

The current reference receiver is a RadioMaster RP2.

Receiver-side BOOT-button Bind requires **ExpressLRS receiver firmware 3.4.0 or newer**. It has been hardware validated with ExpressLRS 3.4.3.

The reference ExpressLRS configuration is:

```text
333 Hz Full
16ch Rate/2
