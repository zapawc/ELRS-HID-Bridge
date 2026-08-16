# ELRS HID Bridge

Convert an ExpressLRS/CRSF receiver into a standard USB HID joystick using an Adafruit QT Py RP2040.

ELRS-HID-Bridge is an open-source CRSF-to-USB HID bridge and development foundation. The reference application is a wireless FPV simulator joystick, but the firmware is intentionally structured so the CRSF, state, mapping, HID, diagnostics, and hardware layers can be reused by other projects.

The reference build uses only two functional components:

- Adafruit QT Py RP2040
- RadioMaster RP2 ExpressLRS receiver

No display, external pushbutton, custom PCB, custom USB driver, or mandatory companion application is required.

---

## Project Status

**Current Version:** 0.3  
**Status:** Active pre-v1.0 development

The core wireless joystick path is functional and has been validated in Liftoff.

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
- deterministic 500 ms RC timeout/failsafe
- automatic recovery after transmitter reconnect
- RGB status indication
- onboard BOOT-button diagnostics/maintenance UI
- short-press Link Quality diagnostic
- bidirectional/TX-capable CRSF UART abstraction
- generic CRSF parser/dispatcher boundary
- Device Ping (`0x28`) recognition

### Current protocol checkpoint

The first live bidirectional CRSF discovery experiment is now enabled. A valid Device Ping (`0x28`) can produce the already self-tested Parameter Device Information (`0x29`) response and send it through `CrsfUart::write()`.

Current behavior:

- Device Info encoding remains covered by deterministic startup self-tests.
- Broadcast and directly addressed pings are eligible for a response.
- Pings addressed to another CRSF device are ignored.
- Each consumed ping can produce at most one Device Info response attempt.
- The experimental local CRSF node address is `0xC8` (Flight Controller), matching the bridge's FC-side position on the RP2 UART.
- The discovery identity is `ELRS-HID-Bridge` with zero CRSF parameters.
- Proof-of-concept Serial/Hardware/Firmware ID fields are deterministic placeholders and are not yet release identity policy.

The next protocol task is **hardware validation, not more protocol implementation**: determine whether the RP2/Ranger/EdgeTX path discovers `ELRS-HID-Bridge`, capture the observed routing behavior, and verify that 333 Hz Full RC-to-HID operation, failsafe, reconnect, and HID reporting remain unaffected.

---

## Design Goals

- Open source
- Plug-and-play standard USB HID
- Low-cost two-component reference hardware
- Deterministic receiver-loss behavior
- Robust CRSF handling
- Clear architectural boundaries
- Useful diagnostics without additional hardware
- Easy to build, understand, repair, fork, and extend
- Prefer standard CRSF mechanisms over proprietary configuration protocols

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

Validated primary-axis orientation:

```text
Roll     normal
Pitch    inverted
Throttle normal
Yaw      normal
```

The HID descriptor supports 32 buttons. The current reference EdgeTX switch layout uses Buttons 1-14.

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
CH15 side slider
CH16 side slider
```

Three-position reference behavior:

```text
Up     -> no button
Middle -> first assigned button
Down   -> second assigned button
```

Current button assignment:

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

## ExpressLRS Packet/Channel Mode

Initial testing at 250 Hz Wide was adequate for sticks and switches but did not provide the intended proportional behavior for all auxiliary analog channels.

The current reference configuration is:

```text
333 Hz Full
16ch Rate/2
```

With the tested ExpressLRS 3.3.1 receiver firmware:

- CH13 behaved proportionally.
- CH14 behaved proportionally.
- CH15 and CH16 remained high.

Diagnostic remapping showed that the CH15/CH16 behavior follows the CRSF channels rather than the HID destinations. The bridge therefore retains the intended CH15->Ry and CH16->Rz mapping and does not implement an HID-side workaround for upstream channel behavior.

---

## Failsafe

RC health is based on valid RC Channels frames, not on generic CRSF traffic or Link Statistics.

Current trigger:

```text
500 ms without a valid RC channel frame
```

Current failsafe:

```text
Roll     center
Pitch    center
Yaw      center
Throttle minimum
Buttons  released
```

The current auxiliary-analog failsafe behavior still needs an explicit pre-v1.0 decision and regression test.

Important observed behavior:

```text
RF link lost
    ->
RC Channels frames stop
    ->
Link Statistics may continue
```

Therefore Link Statistics must never independently restore healthy RC state.

When valid RC frames return, live HID control resumes automatically without rebooting the bridge.

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

Pure green has a reserved meaning:

> Everything required for normal HID operation is healthy.

The short-press Link Quality diagnostic uses a visibly different green/lime presentation so a successful button action is distinguishable from the normal healthy state.

---

## BOOT Button

The QT Py onboard BOOT button is used as a local diagnostic/maintenance interface so the reference hardware remains a two-component design.

Current interaction model:

```text
short click
  -> acknowledge input
  -> show current diagnostic state
  -> return to normal state

~2 s hold
  -> Bind candidate
  -> release to request action

~5 s hold
  -> Wi-Fi candidate
  -> release to request action

continue holding
  -> Cancel candidate
  -> release with no maintenance action
```

The maintenance UI and LED-selection behavior are implemented, but receiver Bind and Wi-Fi commands are still reserved and do not currently send receiver commands.

---

## Firmware Architecture

The current receive path is intentionally layered:

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
   |      |      +--> CrsfDevice
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

Supporting application boundaries include:

- `BridgeState` for operational facts and RC timeout state
- `BridgeConfiguration` for canonical runtime configuration/defaults
- `FailsafePolicy` for deterministic safe HID state
- `BootButton` for physical button input only
- `MaintenanceController` for button semantics
- `StatusLed` for physical NeoPixel output only
- `StatusDisplay` for display arbitration
- `CrsfFrameEncoder` for outbound extended-frame construction
- `CrsfDevice` for CRSF device-level traffic

`CrsfUart` provides both RX and TX primitives. The production loop now uses TX only for the controlled Device Ping -> Device Info discovery experiment.

See `docs/Architecture.md` for the detailed boundaries and design rules.

---

## Hardware

### Reference MCU

- Adafruit QT Py RP2040

### Reference receiver

- RadioMaster RP2 2.4 GHz ExpressLRS receiver

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

The RX/TX connection is intentionally bidirectional to support future CRSF device/configuration traffic without a hardware redesign.

---

## Building and Flashing

The project uses:

- Visual Studio Code
- PlatformIO
- RP2040 Arduino core/toolchain used by the project environment
- TinyUSB/HID support provided by the selected framework

Clone the repository:

```bash
git clone https://github.com/zapawc/ELRS-HID-Bridge.git
```

Open the project in Visual Studio Code with the PlatformIO extension installed, then use the normal PlatformIO build/upload controls for the selected environment.

The project intentionally prefers the normal VS Code/PlatformIO workflow for routine builds and uploads. Direct `pio` CLI commands are mainly useful when troubleshooting requires them.

---

## Validation

For control-path changes, validate at least:

- clean build with no VS Code Problems
- expected USB HID enumeration
- Roll direction/range
- Pitch direction/range
- Throttle direction/range
- Yaw direction/range
- AUX analog behavior
- switch/button mapping
- transmitter-off failsafe
- no stale buttons after failsafe
- automatic recovery after reconnect
- documented LED behavior
- Link Statistics cannot falsely restore healthy RC state
- Liftoff operation when the control path changed

For parser/CRSF changes, also validate:

- valid `0x16` RC frames
- valid `0x14` Link Statistics frames
- CRC rejection
- malformed/partial stream recovery
- startup self-tests
- no UART activity starvation of HID reporting

---

## Current CRSF Device Discovery Work

The first bidirectional CRSF proof-of-concept is intentionally small.

Implemented:

- Device Ping frame type (`0x28`) recognition
- Device Info frame type (`0x29`) definition
- extended-frame encoder
- deterministic Device Info response construction
- response routing/field/CRC self-tests
- live Device Info transmission for valid broadcast/direct pings
- isolated proof-of-concept identity/address definition in `bridge_identity.h`

Current hardware-validation items:

- confirm the experimental `0xC8` local address through the RP2/Ranger/EdgeTX path
- confirm EdgeTX discovery of `ELRS-HID-Bridge`
- verify live TX does not disturb the 333 Hz Full RC/HID path

Not implemented yet:

- final production bridge CRSF address/identity policy
- CRSF parameter tree
- persistent configuration

If the identity-only discovery experiment succeeds cleanly, full transmitter-side configuration remains post-v1.0 work unless there is a compelling reason to change scope.

---

## Repository Layout

```text
docs/
    Architecture.md
    Protocol.md
    Roadmap.md

src/
    ...
```

---

## Why Another Simulator Dongle?

The basic receiver->microcontroller->USB joystick concept has prior art and demonstrated community value. ELRS-HID-Bridge does not need to claim invention of that concept.

The project aims to differentiate through:

- deterministic receiver-loss handling
- robust and explicit CRSF processing
- clean transport/protocol/state/HID separation
- diagnostics and maintenance using existing board hardware
- a two-component reference build
- deliberate support for bidirectional CRSF extension
- documentation intended to make the project easy to understand and fork

Features are not added solely for differentiation.

---

## Project Boundary

The upstream project identity is:

> An open-source, two-component CRSF-to-USB HID bridge and development foundation, with a wireless FPV simulator joystick as its reference application.

Application-specific ideas such as OBS integration, race-station control, pilot-input logging, or specialized displays are better implemented as optional extensions or forks unless they strengthen the reusable core.

---

## Contributing

The project is under active development.

Ideas, bug reports, testing, documentation improvements, and pull requests are welcome.

Changes should preserve the critical RC-to-HID path and favor small regression-testable commits over broad rewrites.

---

## Roadmap

See `docs/Roadmap.md`.

---

## License

License selection is pending.

An open-source license should be selected before Version 1.0.

---

## Acknowledgements

This project builds on the work of:

- ExpressLRS
- Team BlackSheep CRSF protocol/specification
- Adafruit
- the RP2040 Arduino/toolchain ecosystem
- TinyUSB

Relevant prior-art projects should be acknowledged where appropriate as the public release documentation is finalized.
