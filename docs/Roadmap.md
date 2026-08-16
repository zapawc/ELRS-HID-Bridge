# ELRS HID Bridge Roadmap

**Status:** Active pre-v1.0 development  
**Updated:** August 2026

## 1. Project Direction

ELRS-HID-Bridge is a two-component, open-source CRSF-to-USB HID bridge.

The reference application is a wireless USB joystick for FPV simulators, but the codebase should also serve as a clean starting point for other ELRS/CRSF-to-PC projects.

The project should remain easy to build:

```text
ExpressLRS receiver
        +
USB-capable RP2040
```

The reference hardware is currently:

```text
RadioMaster RP2
        +
Adafruit QT Py RP2040
```

No additional display, pushbutton, companion application, custom USB driver, or custom PCB is required for the reference build.

---

## 2. Release Philosophy

Version 1.0 does not need to implement every plausible CRSF or HID capability.

The v1.0 release should mean:

- the reference hardware can be reproduced by another builder,
- the joystick path is stable and deterministic,
- receiver loss is handled safely,
- documentation describes the real implementation,
- firmware is structured well enough for contributors to extend,
- release artifacts and licensing are in place.

Future features should not delay v1.0 unless they materially affect the stability, maintainability, or recoverability of the reference design.

---

## 3. Proven Functional Baseline

The following behavior has been validated on physical hardware:

- ExpressLRS receiver connected to QT Py RP2040 over 420000-baud CRSF UART
- live CRSF frame reception
- CRC validation and stream recovery
- explicit decoding of all sixteen packed 11-bit RC channels
- Link Statistics (`0x14`) decoding
- USB HID joystick enumeration under Windows
- simulator operation in Liftoff with no USB connection to the transmitter
- eight conventional DirectInput analog axes
- 32-button HID capacity
- mapped 2-position, 3-position, and momentary transmitter switches
- deterministic RC timeout/failsafe
- automatic recovery after transmitter reconnect
- RGB status indication
- BOOT-button input
- short-press Link Quality diagnostic display

Current primary mapping:

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

Current HID analog mapping:

```text
X        Roll
Y        Pitch
Slider 1 Throttle
Slider 2 Yaw
Z        AUX Analog 1
Rx       AUX Analog 2
Ry       AUX Analog 3
Rz       AUX Analog 4
```

Validated orientation:

```text
Roll     normal
Pitch    inverted
Throttle normal
Yaw      normal
```

Current failsafe trigger:

```text
500 ms without a valid RC channel frame
```

Current failsafe behavior:

```text
Roll     center
Pitch    center
Yaw      center
Throttle minimum
Buttons  released
```

---

## 4. Immediate Pre-v1.0 Architecture Work

These are the highest-value structural changes because they become harder to make after bidirectional CRSF and persistent configuration are added.

They should be implemented one at a time, with regression testing after each meaningful change.

### 4.1 Separate CRSF parsing and dispatch

Refactor the current decoder so that:

```text
byte stream
   |
CrsfParser
   |
validated CrsfFrame
   |
CrsfDispatcher
```

Frame assembly/CRC logic should not know the meaning of individual frame types.

This creates a clean extension point for future CRSF device/configuration frames.

### 4.2 Generalize CRSF frame synchronization

Do not permanently assume every valid incoming CRSF frame must begin with `0xC8`.

Finalize parser behavior with extended/device-addressed CRSF traffic in mind.

Existing RC and Link Statistics behavior must remain unchanged.

### 4.3 Make CRSF transport bidirectional

Decouple UART transport from the current decoder consumer.

Provide a clean receive path and transmit path.

No new end-user feature is required immediately; the goal is to avoid another transport refactor when outbound CRSF is introduced.

### 4.4 Introduce BridgeState

Move operational facts and link/failsafe bookkeeping out of `main.cpp`.

Candidate state includes:

- UART activity
- RC seen/active
- last valid RC timestamp
- Link Statistics values/validity
- failsafe count
- parser/frame error counters
- USB/HID readiness where practical

Protocol modules should report facts. Application policy should interpret them.

### 4.5 Introduce BridgeConfiguration

Create one canonical configuration object.

Initially, values can be compiled defaults.

Expected future configuration includes:

- channel sources
- axis inversion
- switch mapping/type
- failsafe timeout
- LED preferences

Do not require persistence or a configuration UI merely to introduce the model.

### 4.6 Separate button hardware from maintenance UI

Refactor `BootButton` so it reports physical button state/duration/events.

Add a higher-level maintenance UI that maps hold duration to actions.

Current intended UX:

```text
click        current-state diagnostic
~2 s hold    Bind candidate
~5 s hold    Wi-Fi candidate
longer hold  Cancel / exit
```

Action selection should be visible while held and execute on release.

### 4.7 Add LED display arbitration

Keep `StatusLed` as the hardware output abstraction.

Add a display/UI layer with explicit ownership priority:

```text
fatal/startup error
    >
button-selection UI
    >
temporary diagnostic
    >
normal system state
```

Pure green remains reserved for fully healthy normal HID operation.

### 4.8 Separate failsafe policy

Move failsafe state generation out of `main.cpp`.

Preserve the currently proven 500 ms timeout and deterministic HID behavior.

---

## 5. v1.0 Functional Completion

The following should be considered release-blocking or strong v1.0 candidates.

### Required

- stable CRSF-to-HID control path
- deterministic receiver-loss failsafe
- automatic reconnection
- current 8-axis HID profile
- switch/button mapping documented
- reliable startup/self-tests
- clear RGB state behavior
- reproducible QT Py + RP2 wiring/build instructions
- EdgeTX setup guide
- tested release binary
- clean build from source
- open-source license selected
- version/tag/release process
- documentation synchronized with actual firmware

### Strongly preferred

- revised parser/dispatcher architecture
- TX-capable CRSF transport boundary
- BridgeState separation
- BridgeConfiguration abstraction using compiled defaults
- finalized BOOT-button maintenance UI
- documented Link Statistics diagnostics
- basic diagnostic/error counters
- enclosure files or reference enclosure documentation

### Not required for v1.0

- CRSF parameter configuration
- persistent configuration
- desktop configuration utility
- keyboard/media HID
- generic PC gateway
- additional receiver protocols
- OLED/display hardware
- SD-card logging
- Bluetooth
- Wi-Fi on the RP2040 side
- custom PCB

---

## 6. BOOT Button and Local Recovery

The QT Py BOOT button should provide initial setup and recovery without requiring extra hardware or a support application.

Target interaction:

```text
short click
    -> acknowledge press
    -> display current diagnostic state
    -> return to normal status

hold to first threshold
    -> LED transition
    -> blue Bind candidate
    -> release to request receiver bind mode

hold to second threshold
    -> LED transition
    -> white Wi-Fi candidate
    -> release to request receiver Wi-Fi mode

continue holding
    -> LED transition
    -> Cancel/exit candidate
    -> release with no maintenance action
```

Exact timing and animation should be tuned through physical testing.

### Validation gates

Before advertising Bind/Wi-Fi control as supported:

- verify the required receiver-side command mechanism on the RP2 UART path,
- verify commands across the intended ELRS firmware versions,
- ensure maintenance actions do not interfere with USB HID behavior,
- confirm accidental short presses cannot enter maintenance states.

If receiver Wi-Fi can be triggered cleanly, it provides a powerful support path because receiver configuration/firmware access remains available without opening the enclosure.

---

## 7. Post-v1.0: CRSF Device Configuration

A major planned capability is for the bridge to identify as a configurable CRSF device and use EdgeTX as the primary configuration UI.

Target user experience:

```text
plug bridge into PC
      |
ELRS connects
      |
open CRSF device configuration on transmitter
      |
select ELRS-HID-Bridge
      |
change settings
      |
bridge validates and saves configuration
```

Potential parameters:

- axis source channel
- axis inversion
- switch type
- HID button assignments
- failsafe timeout
- failsafe behavior
- LED brightness/behavior
- calibration
- HID profile
- diagnostic options
- restore defaults
- reboot

### Architectural prerequisites

- bidirectional `CrsfUart`
- generic frame parser
- frame dispatcher
- CRSF device handler
- configuration service
- persistent configuration format
- configuration schema versioning
- validation/default fallback

### First proof-of-concept

Do not begin with a full parameter tree.

First prove that the bridge can respond through the RP2/Ranger path as a discoverable CRSF device and expose its identity to EdgeTX.

If that succeeds, proceed incrementally.

---

## 8. Post-v1.0: Persistent Configuration

Persistent configuration should use the same `BridgeConfiguration` model used by runtime code.

Requirements:

- schema/version identifier
- validation
- safe defaults
- corrupt/incompatible configuration fallback
- deliberate write behavior to avoid unnecessary flash wear
- migration strategy if schema evolves

Factory defaults should always produce a usable reference joystick configuration.

A hardware recovery path should remain possible even if persistent settings become unusable.

---

## 9. Post-v1.0: Bridge Health Telemetry

Potential bridge-originated telemetry:

- firmware version
- USB connected/ready
- HID ready
- RC stream active
- Link Quality
- RSSI
- SNR
- CRC/frame errors
- receiver timeout count
- failsafe count
- uptime
- reset reason
- HID/report statistics

Telemetry is informational.

It must never override the primary RC-derived operational state or failsafe logic.

Information primarily useful during configuration may be exposed as read-only CRSF device information instead of permanent EdgeTX telemetry sensors.

---

## 10. Future USB Expansion

Native USB provides useful headroom.

Potential optional profiles include:

- composite USB HID + CDC serial
- alternate HID joystick profiles
- keyboard/media HID
- structured diagnostics interface

The standard joystick profile must remain the simplest and default user experience.

No proprietary driver should be required for ordinary use.

A companion desktop application is not a current project goal.

---

## 11. Future PC/ELRS Gateway

The hardware could eventually become a bidirectional wireless PC/transmitter bridge rather than only a joystick.

Potential applications:

- OBS/DVR state or control
- race timer state
- lap/event notification
- race-station controls
- PC automation driven by AUX switches
- timestamped pilot-input logging
- link-quality logging
- application-state feedback to EdgeTX

This should be treated as an optional application layer above the stable CRSF and USB foundations.

It should not be mixed into CRSF parameter configuration semantics.

---

## 12. Fork/Community Expansion Ideas

The following are good examples for community forks or optional modules rather than core requirements:

- keyboard/media controller
- OBS-specific integration
- race-station controller
- pilot-input "black box"
- link-quality logger
- alternate RP2040 boards
- optional STEMMA QT display
- sensors
- custom enclosure/carrier boards
- specialized HID profiles

The repository should make these projects easy without requiring the upstream firmware to own every application.

---

## 13. Explicit Non-Goals for the Current Core

Unless project requirements change, do not pursue the following merely for feature completeness:

- generic SBUS/iBUS/PPM support
- mandatory OLED
- mandatory external button
- mandatory custom PCB
- mandatory desktop software
- RP2040-side Bluetooth/Wi-Fi
- RTOS conversion
- multicore use without a demonstrated need
- dynamic plugin architecture
- application-specific integrations in the critical HID path

Unused RP2040 hardware capability is considered design headroom, not wasted capacity.

---

## 14. Testing and Release Workflow

Development should use small, reversible steps.

Preferred workflow:

```text
known-good Git commit
      |
one structural or functional change
      |
compile
      |
deterministic/self tests
      |
bench hardware test
      |
Windows joy.cpl validation
      |
Liftoff test when control path is affected
      |
commit/tag checkpoint
```

### Regression checklist for control-path changes

- device enumerates as expected
- Roll direction/range correct
- Pitch direction/range correct
- Throttle direction/range correct
- Yaw direction/range correct
- AUX analog axes behave as expected
- switch mappings remain correct
- transmitter power-off causes failsafe within expected timeout
- no stale button/axis state remains after failsafe
- reconnect restores live control without bridge reboot
- LED follows documented state
- Link Statistics cannot falsely restore healthy RC state

### Regression checklist for parser/CRSF changes

- valid `0x16` frames decode
- valid `0x14` frames decode
- CRC failures are rejected
- partial/malformed stream recovers
- frame counters remain sane
- UART traffic does not starve HID reporting

---

## 15. Documentation Work Before Public v1.0 Announcement

Before broader Reddit/forum release, synchronize:

- `README.md`
- `docs/Architecture.md`
- `docs/Protocol.md`
- `docs/Roadmap.md`

README positioning should clearly state:

> ELRS-HID-Bridge is an open-source CRSF-to-USB HID bridge and development foundation. The reference build uses only an ExpressLRS receiver and RP2040 board and functions as a wireless USB joystick for FPV simulators.

Also document:

- prior art and acknowledgements where appropriate
- exact supported/reference hardware
- exact wiring
- tested ELRS/EdgeTX configurations
- current limitations
- how to build/flash
- how to validate in Windows
- how to report bugs
- contribution/fork guidance
- project license