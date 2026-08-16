# ELRS HID Bridge Architecture

**Status:** Pre-v1.0 architecture baseline  
**Updated:** August 2026

## 1. Purpose

ELRS-HID-Bridge converts the CRSF output of an ExpressLRS receiver into a standard USB HID joystick.

The reference implementation is intentionally minimal:

- Adafruit QT Py RP2040
- ExpressLRS CRSF receiver, currently validated with RadioMaster RP2

No display, external pushbutton, companion application, custom USB driver, or custom carrier PCB is required for normal operation.

The reference use case is a wireless FPV simulator controller:

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

The architecture is intentionally broader than a simulator dongle. The project is intended to remain a clean, forkable CRSF-to-USB HID foundation.

---

## 2. Core Design Principles

### 2.1 Two-component reference build

The upstream reference design requires only:

1. a USB-capable RP2040 board, and
2. an ExpressLRS/CRSF receiver.

No mandatory feature should add hardware unless there is a compelling reason to change this principle.

### 2.2 Plug-and-play normal operation

Normal operation should remain:

```text
connect USB
    ->
power transmitter / establish ELRS link
    ->
use standard HID joystick
```

No proprietary PC driver or support application should be required for normal use.

### 2.3 CRSF-focused foundation

CRSF is the communication platform between the receiver and bridge, not merely an input encoding.

The upstream project should prioritize robust CRSF support rather than becoming a generic RC-protocol adapter. SBUS/iBUS/PPM support is not a current core goal.

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
- future PC gateway functionality must not break HID.

Loss of valid RC input must always produce deterministic HID failsafe behavior.

### 2.5 Facts are separate from policy

Protocol modules report facts. Higher-level application code decides what those facts mean.

For example:

```text
Link Statistics received != RC control healthy
```

The RP2 has been observed continuing to emit Link Statistics after RC Channels frames stopped, so telemetry presence must never override RC timeout state.

### 2.6 Hardware abstraction is separate from UI policy

Low-level modules expose physical behavior:

- UART bytes,
- button state/duration,
- NeoPixel output,
- USB HID reports.

Higher-level modules decide:

- maintenance selections,
- LED ownership priority,
- receiver-loss state,
- failsafe presentation.

### 2.7 Configuration is data

Runtime mapping, inversion, timeout, and future settings live in `BridgeConfiguration` rather than being distributed through protocol and UI code.

Future CRSF configuration and persistent storage should modify the same canonical configuration model.

### 2.8 Prefer extension points over feature accumulation

The core should make specialized applications easy without owning every specialized application.

OBS integration, race-station control, pilot-input logging, displays, and other application-specific features should generally remain optional modules/examples/forks.

---

## 3. Hardware Architecture

### 3.1 Reference MCU

Adafruit QT Py RP2040.

Current resources used:

- native USB: HID transport
- hardware UART: CRSF RX/TX
- onboard NeoPixel: status/diagnostics
- onboard BOOT button: diagnostic/maintenance input
- flash: firmware; future persistent configuration

Unused RP2040 capability is considered design headroom, not wasted capacity.

### 3.2 Reference receiver

RadioMaster RP2 2.4 GHz ExpressLRS receiver.

Reference wiring:

```text
RP2 5V  -> QT Py 5V
RP2 GND -> QT Py GND
RP2 TX  -> QT Py RX
RP2 RX  -> QT Py TX
```

CRSF UART rate:

```text
420000 baud
```

The physical interface is intentionally bidirectional.

---

## 4. Current Firmware Data Flow

The major pre-v1.0 structural refactor is complete.

Current receive/control architecture:

```text
                         +----------------------+
                         | BridgeConfiguration  |
                         +----------+-----------+
                                    |
ExpressLRS receiver                 |
        |                           |
        v                           |
     CrsfUart                       |
        |                           |
        v                           |
    CrsfParser                      |
        |                           |
 validated CrsfFrame                |
        |                           |
        v                           |
  CrsfDispatcher                    |
   |      |      |                  |
   |      |      +--> CrsfDevice    |
   |      |                         |
   |      +--> LinkStatisticsDecoder|
   |                |               |
   |                +--> BridgeState|
   |                                |
   +--> RcChannelDecoder            |
            |                       |
            v                       |
       RawChannels                  |
            |                       |
            v                       |
    ChannelNormalizer               |
            |                       |
            v                       |
   NormalizedChannels               |
            |                       |
            v                       |
      ChannelMapper <---------------+
            |
            v
      ChannelState
            |
            +------------------> UsbHid
```

Application/UI support:

```text
BootButton
    |
    v
MaintenanceController
    |
    v
StatusDisplay <---- BridgeState
    |
    v
StatusLed
```

Failsafe behavior is generated by `FailsafePolicy`, not directly in `main.cpp`.

---

## 5. Module Boundaries

### 5.1 CrsfUart

Owns physical UART transport only.

Responsibilities:

- initialize the CRSF UART,
- receive raw bytes,
- transmit raw bytes,
- expose transport facts/statistics where useful.

It does not interpret CRSF frame types.

The TX primitive is now used by one deliberately narrow production path: a valid Device Ping may generate a Device Info response during the discovery proof-of-concept. No other live outbound CRSF behavior is enabled.

### 5.2 CrsfParser

Consumes raw bytes and produces validated `CrsfFrame` objects.

Responsibilities:

- frame synchronization,
- length validation,
- frame assembly,
- CRC validation,
- malformed/partial stream recovery.

It does not interpret payload semantics.

Frame synchronization is generalized beyond a permanent `0xC8`-only assumption and accepts valid CRSF serial/device address values.

### 5.3 CrsfFrame

Represents one validated CRSF frame.

Current fields include:

- frame address/sync byte,
- length,
- frame type,
- payload pointer,
- payload length.

The payload view is transient and is consumed synchronously by the dispatcher.

### 5.4 CrsfDispatcher

Routes validated frames by type.

Current routes include:

```text
0x16 RC Channels      -> RcChannelDecoder
0x14 Link Statistics  -> LinkStatisticsDecoder
0x28 Device Ping      -> CrsfDevice
```

Unsupported but otherwise valid frames are intentionally ignored.

### 5.5 RcChannelDecoder

Explicitly unpacks all sixteen packed 11-bit RC channels into `RawChannels`.

Compiler-dependent packed bitfields are intentionally avoided.

### 5.6 LinkStatisticsDecoder

Decodes CRSF Link Statistics (`0x14`).

These values are diagnostic facts only and cannot independently establish healthy RC control state.

### 5.7 CrsfFrameEncoder

Constructs outbound CRSF extended-header frames.

Responsibilities:

- extended-frame sizing,
- destination/origin placement,
- payload copy,
- CRC generation,
- output-capacity validation.

It does not own UART transmission.

### 5.8 CrsfDevice

Owns CRSF device-level protocol behavior.

Current responsibilities:

- recognize validated Device Ping (`0x28`) traffic,
- retain the latest ping routing information,
- construct a Device Info (`0x29`) response for a supplied local device address and identity.

The Device Info builder currently:

- responds only to broadcast or directly addressed pings,
- routes the response to the ping origin,
- uses the supplied local address as response origin,
- encodes the null-terminated device name,
- encodes Serial Number, Hardware ID, and Firmware ID as big-endian `uint32_t`,
- encodes parameter count/version,
- delegates final extended-frame/CRC construction to `CrsfFrameEncoder`.

The builder does **not** transmit.

The Device Info builder continues to receive address and identity as caller-supplied data. The current production proof-of-concept supplies those values through `bridge_identity.h`, keeping experimental discovery policy outside the protocol encoder and easy to revise after RP2/Ranger/EdgeTX validation.


### 5.9 BridgeIdentity

Owns the current project-level CRSF discovery identity used by the live proof-of-concept.

Current experimental values:

```text
Local CRSF address  0xC8 (Flight Controller)
Device name         ELRS-HID-Bridge
Parameters          0
```

The `0xC8` selection reflects the bridge's FC-side position on the RP2 UART. Serial/Hardware/Firmware ID values are deterministic proof-of-concept placeholders, not globally assigned identifiers or final release identity policy.

This separation is intentional: `CrsfDevice` owns protocol mechanics, while project identity/routing policy remains outside the reusable protocol layer.

### 5.10 BridgeState

Central application-state model for operational facts.

Current responsibilities include RC/UART/link facts and receiver timeout state.

It must remain an application-state model rather than a dumping ground for hardware implementation details.

### 5.11 ChannelNormalizer

Converts CRSF channel values into the protocol-independent internal range.

Current normalized range:

```text
0 .. 65535
```

### 5.12 BridgeConfiguration

Canonical runtime configuration model.

It is currently populated from compiled defaults and is consumed by mapping/state code.

Future CRSF parameters and persistent storage should modify this same model instead of creating parallel configuration paths.

### 5.13 ChannelMapper

Consumes `NormalizedChannels` plus `BridgeConfiguration` and produces `ChannelState`.

Current reference channel layout:

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

Validated primary-axis orientation:

```text
Roll     normal
Pitch    inverted
Throttle normal
Yaw      normal
```

### 5.14 ChannelState

Protocol-independent semantic HID state.

Current analog profile:

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

The descriptor supports 32 buttons.

### 5.15 FailsafePolicy

Owns construction of deterministic failsafe HID state.

Current trigger:

```text
500 ms without a valid RC channel frame
```

Current documented behavior:

```text
Roll       center
Pitch      center
Yaw        center
Throttle   minimum
Buttons    released
```

Auxiliary analog behavior still needs an explicit pre-v1.0 decision and test.

When valid RC frames resume, live mapped control resumes automatically.

### 5.16 UsbHid

Owns USB HID presentation:

- enumeration,
- report descriptor,
- HID report generation,
- host communication.

It remains unaware of CRSF details.

### 5.17 BootButton

Low-level onboard BOOT-button abstraction.

It exposes physical state/duration/events and does not encode Bind/Wi-Fi semantics.

### 5.18 MaintenanceController

Owns maintenance-button policy.

Current interaction model:

```text
short click    -> diagnostic action
~2 s hold      -> Bind candidate
~5 s hold      -> Wi-Fi candidate
continued hold -> Cancel candidate
```

Selection occurs while held; action occurs on release.

Bind/Wi-Fi actions are currently reserved and do not transmit receiver commands.

### 5.19 StatusLed

Low-level NeoPixel output abstraction.

It produces requested colors/patterns but does not decide display ownership.

### 5.20 StatusDisplay

Arbitrates competing LED presentation modes.

Priority:

```text
fatal/startup error
    >
button-selection UI
    >
temporary diagnostic
    >
normal system state
```

Conceptual normal states:

```text
White  startup
Blue   firmware healthy / waiting for RC
Yellow UART activity without valid RC control
Green  valid RC control / normal operation
Purple RC link lost / HID failsafe
Red    startup/self-test error
```

Pure green is reserved for fully healthy normal HID operation.

### 5.21 Startup self-tests

Protocol transformations are tested deterministically during startup where practical.

Current startup coverage includes:

- CRSF receive parsing/decoding,
- extended-frame construction,
- Device Ping recognition,
- Device Info response construction.

The Device Info tests validate:

- broadcast ping handling,
- directly addressed ping handling,
- unrelated destination rejection,
- destination/origin reversal,
- null-terminated name encoding,
- big-endian identity fields,
- CRSF length,
- CRC.

None of these startup tests transmit on the live CRSF UART.

---

## 6. RC Link and Failsafe State

RC validity is based on receipt of valid RC Channels frames.

Observed receiver-loss behavior:

```text
RF loss
  |
RC Channels frames stop
  |
Link Statistics may continue
```

Therefore:

```text
Link Statistics received != RC control healthy
```

Normal state sequence:

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

Initial testing used 250 Hz Wide.

Additional proportional AUX channels required Full Resolution behavior.

Current tested transmitter configuration:

```text
333 Hz Full
16ch Rate/2
```

With the tested ExpressLRS 3.3.1 receiver firmware:

- CH13 and CH14 operate proportionally.
- CH15 and CH16 remained high.

Diagnostic remapping demonstrated that CH15/CH16 behavior follows those CRSF channels and is upstream of HID mapping. The bridge therefore retains the intended mapping rather than adding an HID-side workaround.

---

## 8. Bidirectional CRSF Direction

The physical link is already bidirectional:

```text
RP2 TX -> QT Py RX
RP2 RX <- QT Py TX
```

The transport and frame-encoding boundaries are now TX-capable.

### 8.1 Current Device Discovery Proof-of-Concept

Current state:

```text
Device Ping received
      |
      v
CrsfDevice recognizes routing
      |
      v
Device Info response can be constructed and self-tested
      |
      X
live UART transmission not yet enabled
```

The immediate next experiment is to:

1. select a temporary local CRSF device address/identity for bench use,
2. expose the tested Device Info response to the production loop,
3. send it only for valid Device Ping traffic,
4. verify discovery through RP2 -> ELRS RF -> Ranger -> EdgeTX,
5. verify RC-to-HID behavior remains unaffected.

Do not build a parameter tree before the identity-only discovery path is proven.

### 8.2 Address/identity policy

The protocol builder intentionally does not hard-code the bridge address or identity yet.

The live discovery experiment must determine the routing semantics actually accepted by the reference RP2/Ranger/EdgeTX path before a permanent value is documented.

This avoids creating a proprietary or arbitrary address convention merely to make a local test pass.

### 8.3 Future CRSF configuration

If discovery succeeds, future transmitter-side configuration may expose:

- channel/HID assignments,
- axis inversion,
- switch types,
- failsafe timeout/behavior,
- LED settings,
- calibration,
- diagnostics,
- restore defaults,
- reboot.

This remains post-v1.0 unless it proves unusually low-risk and valuable.

### 8.4 Bridge health telemetry

Potential future outbound data includes:

- firmware version,
- USB/HID readiness,
- RC state,
- LQ/RSSI/SNR,
- parser/CRC counters,
- timeout/failsafe counters,
- uptime/reset information.

Telemetry is informational and must not control the critical HID state machine.

---

## 9. Local Maintenance Philosophy

The BOOT button and NeoPixel provide a zero-additional-hardware maintenance interface.

Intended uses:

- immediate diagnostics,
- initial binding/recovery,
- receiver Wi-Fi access,
- recovery-oriented functions.

The button should not become the primary configuration UI.

Routine configuration should preferentially use standard CRSF device configuration if that capability is implemented.

---

## 10. Forkability and Extension Philosophy

A contributor should be able to identify directly from the source tree:

```text
where bytes enter,
where CRSF frames are validated,
where frame types are dispatched,
where channels are decoded/normalized,
where system state is derived,
where mappings are applied,
where outbound frames are constructed,
where HID reports leave.
```

Likely extension points include:

- new CRSF frame consumers,
- alternative HID profiles,
- keyboard/media HID,
- PC automation,
- logging,
- race-station integration,
- alternative RP2040 boards,
- optional STEMMA QT hardware.

The upstream project does not need to implement all of them.

---

## 11. Completed Structural Refactor

The previously recommended pre-expansion refactors are now complete:

1. CRSF parsing separated from frame-type dispatch.
2. CRSF synchronization generalized beyond a fixed `0xC8` assumption.
3. `CrsfUart` decoupled from `CrsfDecoder` and given a TX-capable API.
4. `BridgeState` introduced for application state/facts.
5. `BridgeConfiguration` introduced and consumed by mapping.
6. BOOT-button hardware separated from maintenance semantics.
7. `StatusDisplay` added above `StatusLed` for arbitration.
8. failsafe generation extracted into `FailsafePolicy`.
9. outbound extended-frame encoding added through `CrsfFrameEncoder`.
10. `CrsfDevice` introduced for Device Ping/Device Info work.

The architecture should not be refactored again merely because additional abstraction is possible. New changes should be driven by demonstrated protocol or release needs.

---

## 12. Testing Philosophy

The project is regression-sensitive.

Preferred workflow:

```text
known-good commit
      |
one small change
      |
compile + startup self-tests
      |
bench validation
      |
Windows HID validation
      |
Liftoff validation when control path changed
      |
commit
```

Protocol transformations should be deterministic and hardware-independent where practical.

Hardware validation remains mandatory for:

- ELRS packet/channel modes,
- receiver loss/reconnect,
- CRSF timing,
- UART direction/transmission,
- EdgeTX discovery/routing,
- BOOT-button behavior,
- NeoPixel interaction,
- USB enumeration/HID behavior.

---

## 13. Project Boundary

The upstream reference identity is:

> An open-source, two-component CRSF-to-USB HID bridge and development foundation, with a wireless FPV simulator joystick as its reference application.

Features that make the reusable foundation more robust or easier to extend belong upstream.

Features that merely implement a specific end application should generally remain examples, optional extensions, or forks.
