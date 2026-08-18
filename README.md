# ELRS HID Bridge

Convert an ExpressLRS/CRSF receiver into a standard USB HID joystick using an Adafruit QT Py RP2040.

ELRS-HID-Bridge is an open-source CRSF-to-USB HID bridge and development foundation. The reference application is a wireless FPV simulator joystick, but the firmware is structured so the CRSF, state, mapping, HID, diagnostics, configuration, and hardware layers can be reused by other projects.

The reference build uses only two functional components:

- Adafruit QT Py RP2040
- RadioMaster RP2 ExpressLRS receiver

No display, external pushbutton, custom PCB, custom USB driver, or mandatory companion application is required.

---

## Project Status

**Stable release:** `v1.0.0`  
**Development status:** post-v1.0 feature freeze / next feature-release validation

The stable v1.0.0 release remains the immutable baseline. The current development branch adds transmitter-side CRSF configuration, persistent bridge settings, receiver Bind maintenance, and additional diagnostics while preserving the validated RC-to-HID control path.

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
- Device Ping (`0x28`) -> Device Info (`0x29`) discovery
- CRSF parameter read/write configuration through EdgeTX
- persistent bridge configuration with safe defaults/migration behavior
- configurable inversion for all eight proportional HID axes
- Restore Defaults command
- Diagnostics folder with Failsafe Count
- receiver-side CRSF Bind from the BOOT maintenance UI

### Reference compatibility

The current reference receiver is a RadioMaster RP2.

Receiver-side BOOT-button Bind requires **ExpressLRS receiver firmware 3.4.0 or newer**. It has been hardware validated with ExpressLRS 3.4.3.

The reference ExpressLRS configuration is:

```text
333 Hz Full
16ch Rate/2
```

With the RP2 upgraded to ExpressLRS 3.4.3, CH13-CH16 have all been hardware validated as proportional inputs, including the transmitter's left and right shoulder sliders on CH15 and CH16.

---

## Design Goals

- Open source
- Plug-and-play standard USB HID
- Low-cost two-component reference hardware
- Deterministic receiver-loss behavior
- Robust CRSF handling
- Clear architectural boundaries
- Transmitter-side configuration through standard CRSF mechanisms
- Useful diagnostics without additional hardware
- Easy to build, understand, repair, fork, and extend
- Avoid features that do not solve a real user or maintenance problem

Optional functionality must not interfere with the RC-to-HID critical path.

---

## Current HID Profile

| HID control | Reference source |
|---|---|
| X | Roll / CH1 |
| Y | Pitch / CH2 |
| Slider 1 | Throttle / CH3 |
| Slider 2 | Yaw / CH4 |
| Z | AUX Analog 1 / CH13 |
| Rx | AUX Analog 2 / CH14 |
| Ry | AUX Analog 3 / CH15 |
| Rz | AUX Analog 4 / CH16 |
| Buttons | CH5-CH12 switch mapping |

Validated default orientation:

```text
Roll     normal
Pitch    inverted
Throttle normal
Yaw      normal
```

All eight proportional axes can be inverted through the EdgeTX CRSF device menu. The HID descriptor supports 32 buttons; the reference EdgeTX switch layout uses Buttons 1-14.

---

## Reference EdgeTX Channel Layout

```text
CH1  Roll
CH2  Pitch
CH3  Throttle
CH4  Yaw
CH5  SF
CH6  SA
CH7  SB
CH8  SC
CH9  SD
CH10 SE
CH11 SG
CH12 SH
CH13 S1
CH14 S2
CH15 left/right shoulder slider
CH16 left/right shoulder slider
```

Reference switch behavior:

```text
CH5 / SF:
  Button 1

CH6 / SA:
  Middle -> Button 2
  Down   -> Button 3

CH7 / SB:
  Middle -> Button 4
  Down   -> Button 5

CH8 / SC:
  Middle -> Button 6
  Down   -> Button 7

CH9 / SD:
  Middle -> Button 8
  Down   -> Button 9

CH10 / SE:
  Middle -> Button 10
  Down   -> Button 11

CH11 / SG:
  Middle -> Button 12
  Down   -> Button 13

CH12 / SH:
  Pressed -> Button 14
```

---

## EdgeTX CRSF Device Configuration

EdgeTX discovers `ELRS-HID-Bridge` under **Other Devices**.

Current root menu:

```text
ELRS-HID-Bridge
├─ LED Brightness
├─ Pitch Inversion
├─ Throttle Invert
├─ Roll Inversion
├─ Yaw Inversion
├─ Aux 1 Inversion
├─ Aux 2 Inversion
├─ Aux 3 Inversion
├─ Aux 4 Inversion
├─ Diagnostics
│  └─ Failsafe Count
└─ Restore Defaults
```

Settings are backed by the canonical `BridgeConfiguration` model and persisted across bridge power cycles. Restore Defaults returns the configuration to the known-good reference defaults.

### CRSF parameter-name compatibility rule

On the validated EdgeTX / ExpressLRS path, user-visible CRSF parameter names have an empirical compatibility boundary:

```text
16 characters or fewer  -> works
17 characters or more   -> menu enumeration stalls
```

The production throttle label is therefore intentionally `Throttle Invert` (15 characters).

Project rule: **keep CRSF/EdgeTX parameter names at 16 characters or fewer unless a longer name is explicitly revalidated against the target upstream stack.**

---

## Failsafe

RC health is based on valid RC Channels frames, not generic CRSF traffic or Link Statistics.

Trigger:

```text
500 ms without a valid RC channel frame
```

Failsafe output:

```text
Roll         center
Pitch        center
Yaw          center
Throttle     minimum
AUX Analog 1 center
AUX Analog 2 center
AUX Analog 3 center
AUX Analog 4 center
Buttons      released
```

Link Statistics may continue after RC Channels frames stop, so telemetry reception never independently restores healthy RC state.

---

## Status LED

Conceptual normal states:

```text
White  startup
Blue   firmware healthy / waiting for RC
Yellow UART activity without valid RC control
Green  valid RC control / normal operation
Purple RC link lost / HID failsafe
Red    startup/self-test error
```

Pure green is reserved for fully healthy normal HID operation. Temporary diagnostic/maintenance indications are arbitrated separately from normal state.

---

## BOOT Button

The onboard QT Py BOOT button provides local diagnostics and receiver maintenance without adding hardware.

Current interaction:

```text
release before 2 s
  -> Link Quality diagnostic

2-4 s
  -> Bind / blue
  -> release sends receiver Bind command

4-6 s
  -> No Action / Cancel
  -> release does nothing

continue holding
  -> repeat Bind -> No Action in 2-second steps
```

Actions execute only on release. Crossing a timing threshold never executes an action.

Receiver Bind requires ExpressLRS receiver firmware 3.4.0+.

Wi-Fi and receiver factory-reset actions were investigated and intentionally removed because no supported FC-facing CRSF UART command was identified for those functions on the validated ExpressLRS 3.4.3 path.

A future **Receiver Firmware Update / USB Serial Passthrough** mode is retained as a roadmap concept. It is not implemented in the current release cycle.

---

## Firmware Architecture

The receive/control path remains layered:

```text
ExpressLRS receiver
        |
        v
     CrsfUart
        |
        v
    CrsfParser
        |
 validated CrsfFrame
        |
        v
  CrsfDispatcher
   |      |      |
   |      |      +--> CrsfDevice / BridgeParameters
   |      |
   |      +---------> LinkStatisticsDecoder
   |
   +----------------> RcChannelDecoder
                            |
                            v
                       RawChannels
                            |
                            v
                    ChannelNormalizer
                            |
                            v
                   NormalizedChannels
                            |
                            v
                     ChannelMapper
                            |
                            v
                     ChannelState
                            |
                            v
                        UsbHid
```

Supporting boundaries include `BridgeState`, `BridgeConfiguration`, persistent configuration storage, `FailsafePolicy`, `BootButton`, `MaintenanceController`, `StatusDisplay`, `StatusLed`, `CrsfFrameEncoder`, `CrsfDevice`, and the CRSF parameter service/registry.

See `docs/Architecture.md`.

---

## Hardware

### Reference MCU

- Adafruit QT Py RP2040

### Reference receiver

- RadioMaster RP2 2.4 GHz ExpressLRS receiver
- ExpressLRS 3.4.0+ required for BOOT-button receiver Bind
- ExpressLRS 3.4.3 hardware validated

### Wiring

```text
RP2 5V  -> QT Py 5V
RP2 GND -> QT Py GND
RP2 TX  -> QT Py RX
RP2 RX  -> QT Py TX
```

CRSF UART:

```text
420000 baud
```

The physical RX/TX connection is intentionally bidirectional.

A compact two-board enclosure has been designed separately with an integrated flexible arm to actuate the QT Py BOOT button, preserving the minimal-hardware design.

---

## Building and Flashing

The project uses Visual Studio Code, PlatformIO, the pinned Arduino-Pico framework/toolchain in `platformio.ini`, and TinyUSB/HID support from that framework.

Open the project in VS Code and use the normal PlatformIO Build / Upload controls.

Use the normal `pico` environment for release builds. `pico_debug` is not part of the release path.

After a successful `pico` build, the UF2 is produced under:

```text
.pio/build/pico/firmware.uf2
```

See `docs/Release.md` and `docs/Release-Checklist.md` for release procedure and regression requirements.

---

## Validation

Before release, validate:

- clean normal `pico` build
- VS Code Problems clear
- USB HID enumeration
- all eight proportional axes
- buttons
- all inversion controls
- persistent settings
- Restore Defaults
- transmitter-off failsafe
- automatic reconnect
- Diagnostics / Failsafe Count
- BOOT short diagnostic
- BOOT Bind
- BOOT No Action
- EdgeTX CRSF menu enumeration
- Liftoff smoke test

---

## Why Another Simulator Dongle?

The receiver -> microcontroller -> USB joystick concept has prior art. ELRS-HID-Bridge does not claim invention of that concept.

The project differentiates through deterministic failsafe behavior, explicit CRSF/state/configuration boundaries, transmitter-side configuration, persistence, onboard maintenance/diagnostics, and a minimal two-component reference design.

Features are not added solely for differentiation.

---

## Project Boundary

> An open-source, two-component CRSF-to-USB HID bridge and development foundation, with a wireless FPV simulator joystick as its reference application.

Application-specific integrations should generally remain optional extensions or forks unless they strengthen the reusable core.

---

## License

ELRS-HID-Bridge is licensed under GNU GPL-3.0-only.

See `LICENSE`, `AUTHORS.md`, and `THIRD_PARTY_NOTICES.md`.
