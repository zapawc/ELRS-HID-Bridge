# ELRS HID Bridge Architecture

**Status:** Pre-v1.0 architecture baseline  
**Updated:** August 2026

## 1. Purpose

ELRS-HID-Bridge converts the CRSF output of an ExpressLRS receiver into a standard USB HID joystick.

The reference implementation is intentionally minimal:

- Adafruit QT Py RP2040
- ExpressLRS CRSF receiver, currently validated with RadioMaster RP2

No display, external pushbutton, companion application, custom USB driver, or custom carrier PCB is required for normal operation.

The primary reference use case is a wireless FPV simulator controller:

```text
EdgeTX transmitter
        |
ExpressLRS TX module
        |
   ELRS RF link
        |
ExpressLRS receiver
        |
     CRSF UART
        |
  QT Py RP2040
        |
      USB HID
        |
       PC
```

The architecture is intentionally broader than a simulator dongle. The project is intended to be a clean, forkable CRSF-to-USB HID foundation that other projects can extend without rewriting the protocol or HID core.

---

## 2. Core Design Principles

### 2.1 Two-component reference build

The upstream reference design should require only:

1. A USB-capable RP2040 board
2. An ExpressLRS/CRSF receiver

Optional hardware may be supported by forks or future extensions, but no mandatory upstream feature should require additional hardware unless there is a compelling reason to change this principle.

### 2.2 Plug-and-play normal operation

Normal use should require only:

- connect the bridge to USB,
- power the transmitter,
- establish the ELRS link,
- use the bridge as a standard HID joystick.

USB should remain standards-based and require no proprietary driver.

### 2.3 CRSF-focused foundation

CRSF is not merely an input encoding used by the project. It is the communication platform between the receiver and the bridge.

The upstream project should therefore prioritize robust CRSF support rather than becoming a generic multi-protocol RC receiver adapter.

Support for unrelated protocols such as SBUS or iBUS is not a current project goal. Forks remain free to use the higher-level abstractions for other protocols.

### 2.4 Critical path isolation

The critical path is:

```text
RC frame -> decode -> normalize -> map -> HID report
```

Optional functionality must not interfere with this path.

Examples:

- telemetry failure must not break HID,
- configuration failure must not break HID,
- LED/UI failure must not break HID,
- optional PC gateway functionality must not break HID.

Loss of valid RC input must always produce deterministic HID failsafe behavior.

### 2.5 Facts are separate from policy

Protocol decoders report facts.

Higher-level application logic decides what those facts mean.

For example:

- receiving Link Statistics means telemetry data is available,
- it does not mean valid RC control is available.

This distinction prevents optional telemetry from incorrectly overriding higher-priority operational state.

### 2.6 Hardware abstraction is separate from user interaction policy

Low-level modules should describe physical behavior:

- UART bytes,
- button press state,
- NeoPixel output,
- USB HID reports.

Higher-level modules should decide:

- what a button hold means,
- which LED indication has priority,
- what state constitutes receiver loss,
- what failsafe values should be presented.

### 2.7 Configuration is data

Channel mappings, inversion, timeouts, and other future settings should live in a configuration model rather than being distributed throughout protocol or mapping code.

This allows future CRSF-based configuration and persistent storage without rewriting application logic.

### 2.8 Prefer extension points over feature accumulation

The upstream project should provide clear architectural boundaries that forks can extend.

Application-specific features such as OBS control, race-station integration, displays, or dedicated logging hardware should not be added to the core merely because they are possible.

---

## 3. Current Hardware Architecture

### 3.1 Reference MCU

Adafruit QT Py RP2040

Current hardware resources used:

- native USB: HID transport
- hardware UART: CRSF
- onboard NeoPixel: status and diagnostics
- onboard BOOT button: local diagnostic and maintenance interface
- flash: firmware; future persistent configuration

Reserved hardware capability includes additional GPIO, I2C/STEMMA QT, ADC, PIO, a second RP2040 core, and additional USB interface possibilities. These are considered expansion headroom, not resources that need to be consumed.

### 3.2 Reference receiver

RadioMaster RP2 2.4 GHz ExpressLRS receiver.

Current wiring:

```text
RP2 5V  -> QT Py 5V
RP2 GND -> QT Py GND
RP2 TX  -> QT Py RX
RP2 RX  -> QT Py TX
```

The physical interface is intentionally bidirectional even though the current production path primarily consumes receiver output.

CRSF UART rate:

```text
420000 baud
```

---

## 4. Current Data Pipeline

The currently proven functional pipeline is:

```text
CrsfUart
    |
CrsfDecoder
    |
    +-- RcChannelDecoder
    |
    +-- LinkStatisticsDecoder
            |
        RawChannels
            |
    ChannelNormalizer
            |
    NormalizedChannels
            |
      ChannelMapper
            |
       ChannelState
            |
          UsbHid
```

The implementation already separates RC channel decoding and Link Statistics decoding.

For the next architecture pass, CRSF frame assembly and frame-type dispatch should become distinct responsibilities.

Target direction:

```text
                     +----------------------+
                     | BridgeConfiguration  |
                     +----------+-----------+
                                |
ExpressLRS RX                  |
     |                         |
     v                         |
  CrsfUart <-------------------+-------------------+
     |                                             |
     v                                             |
 CrsfParser                                        |
     |                                             |
     v                                             |
CrsfDispatcher                                     |
  |        |        \                               |
  |        |         \                              |
  v        v          v                             |
RC Decode Link Stats  Future CrsfDevice             |
  |        |                                         |
  v        v                                         |
RawChannels  ---------> BridgeState                  |
  |                                                  |
  v                                                  |
ChannelNormalizer                                    |
  |                                                  |
  v                                                  |
NormalizedChannels                                   |
  |                                                  |
  v                                                  |
ChannelMapper <------------- BridgeConfiguration ----+
  |
  v
ChannelState
  |
  +---------------------------> UsbHid
```

---

## 5. Recommended Module Boundaries

The following represent the desired architectural boundaries. Some are current modules; others are planned refactors.

### 5.1 CrsfUart

Owns only the physical UART transport.

Responsibilities:

- initialize CRSF UART,
- receive raw bytes,
- transmit raw bytes,
- expose transport statistics where useful.

It should not know which CRSF frame types are being processed.

The UART layer should support TX because bidirectional CRSF is a long-term architectural requirement.

### 5.2 CrsfParser

Consumes a byte stream and produces validated CRSF frames.

Responsibilities:

- frame synchronization,
- length validation,
- frame assembly,
- CRC validation,
- recovery after malformed or partial data.

The parser should not interpret frame type payloads.

Frame synchronization must not be permanently coupled to only the `0xC8` address. CRSF extended/device traffic should be considered when finalizing synchronization rules.

### 5.3 CrsfFrame

Represents one validated CRSF frame.

It should carry enough information for later routing and extended-frame support without requiring higher layers to re-parse the byte buffer.

### 5.4 CrsfDispatcher

Routes validated frames to the appropriate consumer.

Examples:

- RC Channels `0x16` -> `RcChannelDecoder`
- Link Statistics `0x14` -> `LinkStatisticsDecoder`
- future device/configuration frames -> `CrsfDevice`

The dispatcher should remain easy for forks to extend with additional frame consumers.

### 5.5 RcChannelDecoder

Decodes CRSF RC channel frames.

Responsibilities:

- explicitly unpack all sixteen 11-bit channels,
- populate `RawChannels`,
- report receipt of a valid RC control frame.

Compiler-dependent packed bitfields should not be used for the channel payload.

### 5.6 LinkStatisticsDecoder

Decodes CRSF Link Statistics frames.

Current information includes useful RF/link metrics such as LQ, RSSI, and SNR.

Link Statistics are diagnostic data only. Their presence must never independently establish healthy RC control state.

### 5.7 BridgeState

A planned central application-state model.

It should contain current operational facts such as:

- whether valid RC control has ever been received,
- whether the RC stream is currently active,
- time of last valid RC frame,
- Link Statistics validity/current values,
- UART activity,
- USB/HID readiness where available,
- diagnostic counters,
- failsafe transitions/count.

`BridgeState` should replace scattered system-state globals in `main.cpp`.

It is not intended to become a global dumping ground. It represents current bridge facts, not hardware implementation details.

### 5.8 ChannelNormalizer

Converts CRSF-specific channel values into the protocol-independent internal range.

Current normalized range:

```text
0 .. 65535
```

This module should remain a pure transformation where practical.

### 5.9 BridgeConfiguration

Planned canonical configuration model.

Initial fields may include:

- axis source channel,
- axis inversion,
- failsafe timeout,
- switch mapping policy,
- LED brightness or behavior.

Initially, this object can be populated entirely from compiled defaults.

Future CRSF configuration and persistent storage should modify the same configuration model rather than bypassing it.

Persistent configuration should have its own schema/version and validation mechanism.

### 5.10 ChannelMapper

Consumes normalized channel values plus `BridgeConfiguration` and produces semantic HID state.

It should not contain CRSF framing knowledge.

Current validated reference mapping:

```text
CH1  -> Roll
CH2  -> Pitch
CH3  -> Throttle
CH4  -> Yaw

CH5  -> SF
CH6  -> SA
CH7  -> SB
CH8  -> SC
CH9  -> SD
CH10 -> SE
CH11 -> SG
CH12 -> SH

CH13 -> Auxiliary Analog 1
CH14 -> Auxiliary Analog 2
CH15 -> Auxiliary Analog 3
CH16 -> Auxiliary Analog 4
```

Validated axis orientation:

```text
Roll     normal
Pitch    inverted
Throttle normal
Yaw      normal
```

Three-position switch reference behavior:

```text
Up     -> no button
Middle -> first assigned button
Down   -> second assigned button
```

### 5.11 ChannelState

Protocol-independent semantic HID state.

The current HID profile exposes eight conventional DirectInput analog axes:

```text
X        Roll
Y        Pitch
Slider 1 Throttle
Slider 2 Yaw
Z        Auxiliary Analog 1
Rx       Auxiliary Analog 2
Ry       Auxiliary Analog 3
Rz       Auxiliary Analog 4
```

and up to 32 HID buttons.

### 5.12 FailsafePolicy

Failsafe behavior should be separated from `main.cpp` application orchestration.

Current trigger:

```text
500 ms without a valid RC channel frame
```

Current reference failsafe:

```text
Roll       center
Pitch      center
Yaw        center
Throttle   minimum
Buttons    released
```

Auxiliary analog behavior should be explicitly documented and tested as the implementation is finalized.

When valid RC frames resume, live control resumes automatically without a bridge reset.

### 5.13 UsbHid

Owns USB HID presentation.

Responsibilities:

- USB enumeration,
- HID report descriptor,
- HID report generation,
- host communication.

It should remain unaware of CRSF and receiver specifics.

Future composite USB interfaces are permitted by the architecture but are not required for v1.0.

### 5.14 BootButton

Low-level BOOT button abstraction.

The physical button module should report button state and duration/events, not encode application semantics such as "bind" or "Wi-Fi."

The QT Py BOOT button is intentionally reused so the two-component hardware requirement is preserved.

### 5.15 MaintenanceUi / ButtonController

Planned higher-level button policy.

Candidate selection occurs while the button remains held; action occurs on release.

Current intended interaction:

```text
short click    -> current-state diagnostic
~2 s hold      -> Bind candidate
~5 s hold      -> Wi-Fi candidate
continued hold -> Cancel / exit candidate
```

The exact cancel threshold should be tuned through hardware use rather than treated as a protocol constant.

The LED provides visual "detents" as thresholds are crossed.

### 5.16 StatusLed

Low-level NeoPixel output abstraction.

It should provide deterministic color/pattern output but should not decide which subsystem owns the display.

Normal conceptual states:

```text
White  Startup
Blue   Firmware healthy / waiting for RC
Yellow UART activity without valid RC control
Green  Valid RC control / normal operation
Purple RC link lost / HID failsafe
Red    Startup/self-test error
```

Pure green has a strong reserved meaning:

> Everything required for normal HID operation is healthy.

### 5.17 StatusDisplay

Planned LED arbitration layer.

The display must support multiple competing presentation modes without allowing lower-priority state to overwrite higher-priority information.

Suggested priority:

```text
Fatal/startup error
    >
Button-selection UI
    >
Temporary diagnostic
    >
Normal system state
```

Button interaction temporarily owns the LED.

Current intended maintenance colors include:

```text
Blue  Bind candidate
White Wi-Fi candidate
```

A short button click should first visibly acknowledge input, then display current diagnostic state and return automatically to the normal system-state display.

### 5.18 CrsfSelfTest

Runs deterministic protocol validation during startup.

Self-tests should remain small, deterministic, and independent of receiver hardware where practical.

They complement but do not replace host-side/unit testing.

---

## 6. RC Link and Failsafe State

RC validity is based on receipt of valid RC channel frames, not generic CRSF traffic.

Important observed behavior:

```text
RF loss
  |
RC channel frames stop
  |
Link Statistics may continue
```

Therefore:

```text
Link Statistics received != RC control healthy
```

The state model must preserve this distinction.

A normal link-loss sequence is:

```text
valid RC control
      |
RC frames stop
      |
500 ms timeout
      |
failsafe ChannelState
      |
purple status
      |
valid RC resumes
      |
live ChannelState resumes
      |
green status
```

---

## 7. ELRS Channel Resolution

Initial testing used a 250 Hz Wide configuration.

Additional proportional AUX channels required a Full Resolution configuration.

Validated testing with:

```text
333 Hz Full
16ch Rate/2
```

allowed CH13 and CH14 to operate proportionally.

During testing with ExpressLRS 3.3.1, CH15 and CH16 remained high regardless of HID destination. Diagnostic remapping demonstrated that the behavior followed CH15/CH16 and was upstream of the HID mapping.

The bridge therefore retains the intended mapping for CH15/CH16 rather than encoding an application-level workaround for receiver/ELRS behavior.

---

## 8. CRSF Bidirectional Roadmap

The receiver is physically connected bidirectionally:

```text
RP2 TX -> QT Py RX
RP2 RX <- QT Py TX
```

This is intended to support future outbound CRSF traffic without changing hardware.

### 8.1 CRSF device discovery and configuration

A future bridge may identify itself as a CRSF device and expose transmitter-side configuration through standard CRSF mechanisms.

Candidate capabilities:

- device identity and firmware version,
- channel and HID assignments,
- axis inversion,
- switch types,
- failsafe timeout and behavior,
- LED settings,
- diagnostics,
- restore defaults,
- reboot.

Normal configuration should preferably be performed from the transmitter rather than requiring a dedicated desktop application.

This is a post-foundation capability and should not delay a stable v1.0 joystick release unless implementation proves unusually low-risk.

### 8.2 Bridge health telemetry

Potential outbound bridge telemetry includes:

- firmware version,
- USB/HID readiness,
- RC state,
- current LQ/RSSI/SNR,
- CRC/frame errors,
- timeout/failsafe counters,
- uptime/reset information.

Telemetry describes bridge state; it must not control the critical HID state machine.

### 8.3 Future PC gateway

A future optional application may allow the PC to send low-bandwidth events or state back toward EdgeTX through the bridge and ELRS telemetry path.

Examples include race, recording, or automation status.

This is intentionally outside the v1.0 core.

---

## 9. Local Maintenance Philosophy

The BOOT button and NeoPixel form a complete zero-additional-hardware maintenance interface.

The physical button is intended for:

- immediate diagnostics,
- initial binding/recovery,
- receiver Wi-Fi access,
- recovery-oriented functions.

It should not become the primary configuration UI.

Routine configuration should preferentially use standard CRSF device configuration if that capability is implemented.

No dedicated support application should be required for basic setup or receiver recovery.

---

## 10. Forkability and Extension Philosophy

The project should be understandable by reading the source tree.

A contributor should be able to identify:

```text
where bytes enter,
where CRSF frames are validated,
where frame types are dispatched,
where channels are normalized,
where mappings are applied,
where system state is derived,
where HID reports leave.
```

Likely fork extension points include:

- new CRSF frame consumers,
- alternative HID profiles,
- keyboard/media HID,
- PC automation,
- logging,
- race-station integration,
- alternative RP2040 boards,
- optional STEMMA QT hardware.

The upstream project does not need to implement all of these.

---

## 11. Refactors Recommended Before Architectural Expansion

These are structural changes worth completing while the firmware is still small.

1. Split CRSF frame parsing from frame-type dispatch.
2. Generalize CRSF synchronization/address handling beyond an assumption that every valid frame starts with `0xC8`.
3. Decouple `CrsfUart` from `CrsfDecoder` and provide a TX-capable transport API.
4. Introduce `BridgeState` and move link/failsafe/state bookkeeping out of `main.cpp`.
5. Introduce `BridgeConfiguration`, initially populated from compiled defaults.
6. Make `ChannelMapper` consume configuration rather than own configuration policy.
7. Separate low-level `BootButton` behavior from maintenance-button semantics.
8. Add a `StatusDisplay`/UI arbitration layer above `StatusLed`.
9. Move failsafe generation into a distinct policy/function/module.

These changes should be performed incrementally, with hardware validation after each meaningful step.

---

## 12. Testing Philosophy

The project has moved from proof-of-concept to regression-sensitive development.

Every significant structural change should preserve a known-good checkpoint.

Preferred cycle:

```text
known-good commit
      |
small refactor
      |
build/self-test
      |
bench validation
      |
Windows HID validation
      |
Liftoff validation when control path changed
      |
commit
```

Protocol transformations should be deterministically testable without RF hardware where practical.

Hardware testing remains mandatory for behaviors involving:

- ELRS packet/channel modes,
- receiver loss/reconnect,
- CRSF timing,
- UART direction,
- BOOT button behavior,
- NeoPixel interaction,
- USB enumeration and HID behavior.

---

## 13. Project Boundary

The upstream project's reference identity is:

> An open-source, two-component CRSF-to-USB HID bridge and development foundation, with a wireless FPV simulator joystick as its reference application.

Features that make the foundation more robust or easier to extend belong upstream.

Features that merely implement a specific end application should generally remain examples, optional extensions, or forks.
