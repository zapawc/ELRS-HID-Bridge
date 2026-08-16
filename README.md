# ELRS HID Bridge

Convert an ExpressLRS/CRSF receiver into a standard USB HID joystick using an Adafruit QT Py RP2040.

ELRS-HID-Bridge is an open-source CRSF-to-USB HID bridge and development foundation. The reference application is a wireless FPV simulator joystick, but the firmware is intentionally structured so the CRSF, state, mapping, HID, diagnostics, and hardware layers can be reused by other projects.

The reference build uses only two functional components:

- Adafruit QT Py RP2040
- RadioMaster RP2 ExpressLRS receiver

No display, external pushbutton, custom PCB, custom USB driver, or mandatory companion application is required.

---

## Project Status

**Current Version:** 1.0.0-rc1  
**Status:** v1.0 release candidate validation

The core wireless joystick path is functional and has been validated in Liftoff. Runtime behavior is now frozen for the `1.0.0-rc1` validation cycle; remaining work is release regression, clean-reader documentation validation, and packaging of the tested UF2 artifact.

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
- generic CRSF parser/dispatcher boundary
- Device Ping (`0x28`) recognition
- live Device Ping -> Device Info (`0x29`) discovery through RP2/Ranger/EdgeTX

### Current protocol checkpoint

The identity-only bidirectional CRSF discovery path is hardware validated. A valid Device Ping (`0x28`) can produce the self-tested Parameter Device Information (`0x29`) response and send it through `CrsfUart::write()`.

Validated behavior:

- Device Info encoding is covered by deterministic startup self-tests.
- Broadcast and directly addressed pings are eligible for a response.
- Pings addressed to another CRSF device are ignored.
- Each consumed ping can produce at most one Device Info response attempt.
- The reference local CRSF node address `0xC8` (Flight Controller) successfully routes through RP2 -> ELRS RF -> Ranger -> EdgeTX.
- EdgeTX discovers `ELRS-HID-Bridge` under **Other Devices**.
- 333 Hz Full RC-to-HID operation and Liftoff performance remain normal with live CRSF TX enabled.
- The discovery identity exposes zero CRSF parameters.
- CRSF Serial/Hardware IDs are stable project-defined identifiers for the reference build.
- CRSF Firmware ID is derived from the canonical firmware version in `firmware_version.h`.
- `1.0.0-rc1` encodes as CRSF Firmware ID `0x01000000`. The `-rc1` prerelease label is intentionally not encoded in the numeric CRSF field.

CRSF feature expansion is now frozen for the v1.0 cycle. The project is moving through release hardening rather than adding a parameter tree or other outbound protocol features.

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

`FailsafePolicy` explicitly assigns every HID control so no proportional or button state is retained accidentally after RC timeout. This behavior is covered by a deterministic startup self-test.

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
- the pinned Arduino-Pico framework/toolchain declared in `platformio.ini`
- TinyUSB/HID support provided by the selected framework

Clone the repository:

```bash
git clone https://github.com/zapawc/ELRS-HID-Bridge.git
```

Open the project in Visual Studio Code with the PlatformIO extension installed, then use the normal PlatformIO **Build** / **Upload** controls.

Use:

```text
pico        normal HID-only release build
pico_debug  HID + USB CDC debug logging
```

The project intentionally prefers the normal VS Code/PlatformIO workflow for routine builds and uploads. Direct `pio` CLI commands are mainly a troubleshooting tool and are not required by the reference workflow.

### Release UF2

After a successful normal `pico` build, PlatformIO produces the release-candidate UF2 at:

```text
.pio/build/pico/firmware.uf2
```

For a published release, use the versioned UF2 attached to the corresponding GitHub Release, for example:

```text
ELRS-HID-Bridge-v1.0.0-rc1.uf2
```

For manual flashing, use the QT Py RP2040's normal BOOTSEL/UF2 procedure. When the RP2040 mass-storage boot device appears, copy the UF2 onto it.

Release maintainers can stage the already-built UF2, SHA-256 checksum, and release manifest without invoking PlatformIO:

```powershell
.\tools\Stage-Release.ps1
```

Release/version policy is documented in `docs/Release.md`; the complete release-candidate hardware regression is in `docs/Release-Checklist.md`.

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
- AUX analog axes center on failsafe
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

The first bidirectional CRSF feature remains intentionally small.

Implemented:

- Device Ping frame type (`0x28`) recognition
- Device Info frame type (`0x29`) definition
- extended-frame encoder
- deterministic Device Info response construction
- response routing/field/CRC self-tests
- live Device Info transmission for valid broadcast/direct pings
- isolated project identity/address definition in `bridge_identity.h`
- canonical semantic firmware version in `firmware_version.h`
- startup self-test that verifies firmware version / CRSF Firmware ID consistency

Hardware validation completed:

- reference `0xC8` local address routes successfully through the RP2/Ranger/EdgeTX path
- EdgeTX discovers `ELRS-HID-Bridge` under **Other Devices**
- live TX does not disturb the 333 Hz Full RC/HID path or Liftoff behavior

Not implemented yet:

- CRSF parameter tree
- persistent configuration

Identity-only discovery has succeeded cleanly. Version/identity metadata is now centralized for release hardening. Full transmitter-side configuration remains post-v1.0 work. Runtime behavior is frozen for the release-candidate cycle; the remaining work is clean-build validation, documentation verification, release-binary staging, and final regression.

---

## Firmware Versioning

`src/firmware_version.h` is the canonical firmware version source.

Current release-candidate version:

```text
1.0.0-rc1
```

The CRSF Device Info `Firmware_ID` is derived from the same constants using:

```text
bits 31..24  major
bits 23..16  minor
bits 15..8   patch
bits 7..0    reserved (0)
```

For `1.0.0-rc1`, the numeric CRSF Firmware ID is `0x01000000`. The human-readable `-rc1` prerelease label is intentionally not encoded into that CRSF field, so the final `1.0.0` release will use the same numeric CRSF Firmware ID.

A startup self-test verifies that the version string/tuple and the CRSF identity remain synchronized.

Version tags and final release-number transition policy are tracked in `docs/Release.md`.

---

## Repository Layout

```text
CHANGELOG.md
LICENSE
AUTHORS.md
THIRD_PARTY_NOTICES.md

docs/
    Architecture.md
    Protocol.md
    Release.md
    Release-Checklist.md
    Release-Notes-v1.0.0-rc1.md
    Roadmap.md
    USB-Identity.md

tools/
    Capture-BuildEnvironment.ps1
    Stage-Release.ps1

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

ELRS-HID-Bridge is licensed under the **GNU General Public License v3.0 only (`GPL-3.0-only`)**.

Distributed derivative firmware remains covered by the GPL terms. See `LICENSE`, `AUTHORS.md`, and `THIRD_PARTY_NOTICES.md` for the project license, attribution, and third-party dependency notices.

---

## Acknowledgements

This project builds on the work of:

- ExpressLRS
- Team BlackSheep CRSF protocol/specification
- Adafruit
- the RP2040 Arduino/toolchain ecosystem
- TinyUSB

Relevant prior-art projects should be acknowledged where appropriate as the public release documentation is finalized.


## Reproducible v1.0 Build Baseline

The hardware-tested v1.0 hardening baseline pins:

```text
Arduino-Pico framework   1.60000.0
RP2040 toolchain         5.160100.260719
Adafruit NeoPixel        1.15.5
```

These versions are declared in `platformio.ini`. Dependency updates should be intentional and followed by the full hardware regression suite.

### Windows controller name

The firmware advertises `ELRS-HID-Bridge` as its USB product and HID interface name. Windows has been verified to expose that bus-reported identity. Because the reference build retains the inherited RP2040/Pico VID/PID (`2E8A:000A`), Windows `joy.cpl` may display the controller as `Pico`. This is a cosmetic naming limitation; no custom driver or registry modification is required.
