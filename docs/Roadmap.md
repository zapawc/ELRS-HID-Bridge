# ELRS HID Bridge Roadmap

**Status:** Active pre-v1.0 development  
**Updated:** August 2026

## 1. Project Direction

ELRS-HID-Bridge is a two-component, open-source CRSF-to-USB HID bridge.

The reference application is a wireless USB joystick for FPV simulators, but the codebase is intended to remain a clean starting point for other ELRS/CRSF-to-PC projects.

Reference hardware:

```text
RadioMaster RP2
        +
Adafruit QT Py RP2040
```

No additional display, pushbutton, companion application, custom USB driver, or custom PCB is required for the reference build.

---

## 2. Release Philosophy

Version 1.0 does not need every plausible CRSF or HID capability.

A v1.0 release should mean:

- another builder can reproduce the reference hardware,
- the joystick path is stable and deterministic,
- receiver loss is handled safely,
- automatic recovery works,
- documentation matches the implementation,
- the firmware has clean extension boundaries,
- release artifacts and licensing are in place.

Future features should not delay v1.0 unless they materially affect stability, maintainability, or recoverability.

---

## 3. Proven Functional Baseline

Validated on physical hardware:

- 420000-baud CRSF UART to RadioMaster RP2
- live CRSF frame reception
- CRC validation and stream recovery
- explicit decoding of all sixteen packed 11-bit RC channels
- Link Statistics (`0x14`) decoding
- USB HID joystick enumeration under Windows
- Liftoff operation with no USB tether to the transmitter
- eight conventional DirectInput analog axes
- 32-button HID capacity
- mapped 2-position, 3-position, and momentary transmitter switches
- deterministic RC timeout/failsafe
- automatic recovery after transmitter reconnect
- RGB status indication
- BOOT-button input
- short-press Link Quality diagnostic display

Reference EdgeTX channels:

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

HID analog mapping:

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

Current documented failsafe:

```text
Roll     center
Pitch    center
Yaw      center
Throttle minimum
Buttons  released
```

Auxiliary analog failsafe behavior still requires an explicit pre-v1.0 decision/test.

---

## 4. Completed Pre-v1.0 Structural Work

The high-value structural refactor identified earlier is complete.

Completed incrementally with regression checkpoints:

- [x] split CRSF parser and dispatcher
- [x] generalize frame synchronization/address handling beyond fixed `0xC8`
- [x] decouple `CrsfUart` from one decoder consumer
- [x] provide CRSF UART TX primitive
- [x] introduce `BridgeState`
- [x] introduce `BridgeConfiguration`
- [x] make `ChannelMapper` consume configuration
- [x] separate `BootButton` hardware from maintenance semantics
- [x] add `MaintenanceController`
- [x] add `StatusDisplay` arbitration above `StatusLed`
- [x] extract `FailsafePolicy`
- [x] add outbound extended-frame encoder
- [x] add `CrsfDevice` Device Ping recognition

Do not restart or broaden this architecture work unless a demonstrated requirement exposes a specific boundary problem.

---

## 5. Current CRSF Device Discovery Proof-of-Concept

The first bidirectional feature is deliberately limited to identity discovery.

### Completed

- [x] define Device Ping (`0x28`) and Device Info (`0x29`) frame types
- [x] recognize Device Ping through the normal parser/dispatcher path
- [x] retain Device Ping destination/origin routing
- [x] construct CRSF extended frames with destination/origin/CRC
- [x] construct Device Info response payloads
- [x] encode Device Info numeric fields big-endian
- [x] reject response construction for pings addressed to another device
- [x] deterministic self-test for broadcast ping response
- [x] deterministic self-test for directly addressed ping response
- [x] deterministic self-test for destination/origin reversal
- [x] deterministic self-test for null-terminated device name
- [x] deterministic self-test for frame length and CRC

### Intentionally unresolved at this checkpoint

- [ ] production bridge CRSF device address
- [ ] production Serial Number value/source
- [ ] production Hardware ID value
- [ ] production Firmware ID/version encoding
- [ ] live Device Info transmission
- [ ] EdgeTX device discovery

The Device Info builder therefore takes local address and identity as caller-supplied data rather than embedding a premature project policy.

### Immediate next development step

Wire the already-tested Device Info response to the live TX path with the smallest possible production change.

Success criteria:

1. only a valid Device Ping can trigger the response,
2. the response is sent with the tested frame builder,
3. RP2/Ranger/EdgeTX routing is observed on real hardware,
4. EdgeTX discovers `ELRS-HID-Bridge` or the captured behavior clearly identifies the remaining routing issue,
5. 333 Hz Full RC-to-HID behavior remains unaffected,
6. receiver-loss failsafe and reconnect remain unchanged.

If discovery fails, inspect addressing/routing before changing architecture or inventing a proprietary address convention.

### Scope stop

If identity-only discovery succeeds, stop CRSF feature expansion for the v1.0 cycle.

Do **not** immediately add:

- parameter tree,
- persistent configuration,
- Bind command,
- Wi-Fi command,
- telemetry sensors,
- desktop configuration software.

The discovery proof-of-concept is valuable because it validates the bidirectional foundation; a complete configuration system is not required for v1.0.

---

## 6. v1.0 Functional Completion

### Required / release blocking

- [x] stable CRSF-to-HID control path on reference hardware
- [x] deterministic receiver-loss timeout behavior
- [x] automatic reconnection
- [x] 8-axis HID profile implemented
- [x] switch/button mapping implemented
- [x] startup protocol self-tests
- [x] RGB state behavior implemented
- [ ] explicitly define/test AUX analog failsafe behavior
- [ ] final wiring guide validation
- [ ] final EdgeTX setup guide validation
- [ ] clean build from source on documented environment
- [ ] release binary
- [ ] open-source license selected
- [ ] version/tag/release process
- [ ] final README/Architecture/Protocol/Roadmap synchronization

### Strongly preferred

- [x] parser/dispatcher architecture
- [x] generalized CRSF synchronization
- [x] TX-capable CRSF transport
- [x] `BridgeState`
- [x] `BridgeConfiguration`
- [x] maintenance UI/controller separation
- [x] LED display arbitration
- [x] separated failsafe policy
- [x] documented Link Statistics diagnostics
- [ ] basic diagnostic/error counters where useful
- [ ] enclosure/reference enclosure documentation

### Not required for v1.0

- full CRSF parameter configuration
- persistent configuration
- desktop configuration utility
- receiver Bind/Wi-Fi command execution
- keyboard/media HID
- generic PC gateway
- additional RC receiver protocols
- OLED/display hardware
- SD-card logging
- Bluetooth
- RP2040-side Wi-Fi
- custom PCB

---

## 7. BOOT Button and Local Recovery

Current UI behavior:

```text
short click
    -> acknowledge press
    -> display current diagnostic state
    -> return to normal status

~2 s hold
    -> Bind candidate
    -> release requests Bind action

~5 s hold
    -> Wi-Fi candidate
    -> release requests Wi-Fi action

continue holding
    -> Cancel candidate
    -> release with no maintenance action
```

Current status:

- physical button handling: implemented
- maintenance selection semantics: implemented
- LED arbitration: implemented
- diagnostic short press: implemented
- Bind request: reserved/no receiver command sent
- Wi-Fi request: reserved/no receiver command sent

Before Bind/Wi-Fi can be advertised as supported:

- verify the receiver-side command mechanism on the RP2 UART path,
- verify across intended ExpressLRS firmware versions,
- confirm maintenance actions cannot interfere with HID,
- confirm accidental short presses cannot enter maintenance actions.

---

## 8. Post-v1.0: CRSF Device Configuration

Target experience:

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
bridge validates/saves configuration
```

Potential parameters:

- axis source channel
- axis inversion
- switch type
- HID button assignments
- failsafe timeout/behavior
- LED brightness/behavior
- calibration
- HID profile
- diagnostic options
- restore defaults
- reboot

Architectural prerequisites already completed:

- generic parser
- frame dispatcher
- bidirectional `CrsfUart`
- outbound frame encoder
- CRSF device handler
- canonical `BridgeConfiguration`

Still required for a real parameter system:

- parameter model/service
- persistent configuration format
- schema versioning
- validation/default fallback
- parameter read/write handling

Do not build these until identity discovery is proven and v1.0 scope is secure.

---

## 9. Post-v1.0: Persistent Configuration

Persistent configuration should use `BridgeConfiguration` directly.

Requirements:

- schema/version identifier
- validation
- safe defaults
- corrupt/incompatible fallback
- deliberate flash writes
- migration strategy
- hardware recovery path

Factory defaults must always produce a usable reference joystick configuration.

---

## 10. Post-v1.0: Bridge Health Telemetry

Potential bridge-originated information:

- firmware version
- USB/HID ready state
- RC stream active
- Link Quality
- RSSI
- SNR
- parser/CRC errors
- receiver timeout/failsafe count
- uptime
- reset reason
- HID/report statistics

Telemetry is informational and must never override primary RC-derived state or failsafe logic.

---

## 11. Future USB Expansion

Potential optional profiles:

- composite HID + CDC serial
- alternate HID joystick profiles
- keyboard/media HID
- structured diagnostics interface

The standard joystick must remain the simplest/default user experience.

No proprietary driver should be required for ordinary use.

---

## 12. Future PC/ELRS Gateway

Possible optional applications:

- OBS/DVR state/control
- race timer state
- lap/event notification
- race-station controls
- PC automation driven by AUX switches
- timestamped pilot-input logging
- link-quality logging
- application-state feedback to EdgeTX

These belong above the stable CRSF/HID foundation and should not be mixed into core configuration semantics.

---

## 13. Fork/Community Expansion Ideas

Good fork/optional-module candidates:

- keyboard/media controller
- OBS-specific integration
- race-station controller
- pilot-input "black box"
- link-quality logger
- alternate RP2040 boards
- optional STEMMA QT display
- sensors
- custom enclosures/carrier boards
- specialized HID profiles

The repository should make these projects easy without requiring upstream to implement each one.

---

## 14. Explicit Non-Goals for the Current Core

Do not pursue these merely for feature completeness:

- generic SBUS/iBUS/PPM support
- mandatory OLED
- mandatory external button
- mandatory custom PCB
- mandatory desktop software
- RP2040-side Bluetooth/Wi-Fi
- RTOS conversion
- multicore use without demonstrated need
- dynamic plugin architecture
- application-specific integrations in the critical HID path

---

## 15. Testing and Release Workflow

Preferred workflow:

```text
known-good Git commit
      |
one structural/functional change
      |
compile
      |
deterministic/startup tests
      |
bench hardware test
      |
Windows joy.cpl validation
      |
Liftoff test when control path changed
      |
commit/tag checkpoint
```

### Control-path regression checklist

- device enumerates as expected
- Roll direction/range correct
- Pitch direction/range correct
- Throttle direction/range correct
- Yaw direction/range correct
- AUX analog axes behave as expected
- switch mappings remain correct
- transmitter-off causes failsafe within expected timeout
- no stale button state remains after failsafe
- reconnect restores live control without bridge reboot
- LED follows documented state
- Link Statistics cannot falsely restore healthy RC state

### Parser/CRSF regression checklist

- valid `0x16` frames decode
- valid `0x14` frames decode
- CRC failures are rejected
- partial/malformed stream recovers
- startup self-tests pass
- frame/transport counters remain sane where implemented
- UART traffic does not starve HID reporting

### Bidirectional CRSF regression checklist

When live TX is enabled:

- only intended requests generate responses
- destination/origin values match observed routing requirements
- response CRC/length remain correct
- response traffic does not corrupt inbound parsing
- 333 Hz Full control remains stable
- failsafe/reconnect remain stable
- EdgeTX behavior is documented from observation, not assumption

---

## 16. Documentation Before Public v1.0 Announcement

Keep these synchronized at every meaningful checkpoint:

- `README.md`
- `docs/Architecture.md`
- `docs/Protocol.md`
- `docs/Roadmap.md`

Before broader public announcement, also document:

- supported/reference hardware
- exact wiring
- tested ExpressLRS/EdgeTX configuration
- current CH15/CH16 limitation
- exact failsafe behavior including AUX axes
- build/flash workflow
- Windows validation
- simulator validation
- current Device Ping/Device Info status
- known limitations
- bug-report guidance
- contribution/fork guidance
- prior art/acknowledgements
- project license
