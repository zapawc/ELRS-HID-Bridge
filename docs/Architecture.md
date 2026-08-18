# ELRS HID Bridge Architecture

**Status:** post-v1.0 feature-freeze architecture baseline  
**Updated:** August 2026

## 1. Purpose

ELRS-HID-Bridge converts CRSF receiver data into a standard USB HID joystick while using the bidirectional CRSF link for transmitter-side configuration and supported receiver maintenance.

Reference hardware remains intentionally minimal:

1. Adafruit QT Py RP2040
2. RadioMaster RP2 ExpressLRS receiver

No display, external pushbutton, mandatory companion application, custom USB driver, or custom carrier PCB is required for normal operation.

A compact two-board enclosure has also been designed with an integrated flexible arm to actuate the QT Py BOOT button, preserving the minimal-hardware boundary.

## 2. Core Design Principles

### 2.1 Critical path isolation

The critical path remains:

```text
RC frame -> decode -> normalize -> map -> HID report
```

Configuration, telemetry, diagnostics, maintenance, LED/UI behavior, and future PC gateway functionality must not break or starve this path.

### 2.2 Facts are separate from policy

Protocol modules report facts. Application/policy modules decide what those facts mean.

`Link Statistics received != RC control healthy`.

Valid RC Channels frames remain authoritative for RC health.

### 2.3 Configuration is canonical data

Runtime configuration lives in `BridgeConfiguration`. CRSF parameter writes and persistent storage modify/read that same model rather than creating parallel configuration state.

### 2.4 Transmitter-first configuration

Normal bridge configuration should use standard CRSF/EdgeTX mechanisms where practical. A proprietary desktop configuration application is not required for the current product.

### 2.5 Maintenance uses existing hardware

The onboard BOOT button and NeoPixel provide local diagnostics/maintenance. Mandatory external controls should not be added without a compelling requirement.

### 2.6 Features must earn their place

Do not add diagnostics, telemetry, or maintenance actions merely because implementation is possible. Each should solve a concrete user, troubleshooting, or recovery problem.

## 3. Hardware Architecture

### 3.1 QT Py RP2040 resources

- native USB: HID transport
- hardware UART: CRSF RX/TX
- onboard NeoPixel: status/diagnostics/maintenance indication
- onboard BOOT button: diagnostic/maintenance input
- flash: firmware and persistent bridge configuration

### 3.2 Reference receiver

RadioMaster RP2 2.4 GHz ExpressLRS receiver.

```text
RP2 5V  -> QT Py 5V
RP2 GND -> QT Py GND
RP2 TX  -> QT Py RX
RP2 RX  -> QT Py TX
```

CRSF UART: `420000 baud`.

Receiver-side BOOT Bind requires ExpressLRS 3.4.0+ and is validated on 3.4.3.

## 4. Current Data Flow

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
   |      |      +--> CrsfDevice ---+--> BridgeParameters
   |      |                           \-> receiver commands
   |      |
   |      +--> LinkStatisticsDecoder ----> BridgeState
   |
   +--> RcChannelDecoder
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
      ChannelMapper <---------------+
            |
            v
      ChannelState
            |
            v
         UsbHid
```

Supporting UI/application path:

```text
BootButton -> MaintenanceController -> StatusDisplay -> StatusLed
                                      ^
                                      |
                                  BridgeState
```

Persistent storage reads/writes `BridgeConfiguration`. `FailsafePolicy` creates deterministic safe HID state independently of normal mapping.

## 5. Major Boundaries

### CrsfUart

Owns physical CRSF UART RX/TX only.

### CrsfParser

Owns frame synchronization, length validation, assembly, CRC validation, and stream recovery.

### CrsfDispatcher

Routes validated frame types to semantic handlers.

### RcChannelDecoder / ChannelNormalizer

Decode all 16 CRSF RC channels and convert them to the protocol-independent normalized range.

### ChannelMapper

Maps normalized channels to HID-facing `ChannelState` using `BridgeConfiguration`.

Validated defaults:

```text
Roll     normal
Pitch    inverted
Throttle normal
Yaw      normal
```

All eight proportional axes have configurable inversion. Throttle inversion uses the same generic configuration/mapping architecture; failsafe throttle remains safe minimum regardless of live inversion.

### BridgeState

Owns operational facts such as RC activity, link statistics, timeout/failsafe state, and diagnostic counters.

### BridgeConfiguration

Owns canonical runtime configuration/defaults, including LED brightness and axis inversion.

### Persistent configuration

Persists bridge configuration with schema/version handling, validation, safe defaults, and migration/fallback behavior. Restore Defaults writes canonical defaults.

### CrsfDevice / parameter service

Owns device discovery and CRSF device-level parameter/command behavior.

Current capabilities include:

- Device Ping / Device Info discovery
- parameter read/write
- text-selection inversion parameters
- LED brightness
- Diagnostics folder
- Failsafe Count
- Restore Defaults
- supported receiver Bind command

### CRSF parameter-name compatibility rule

Hardware investigation established an empirical EdgeTX/ExpressLRS compatibility boundary:

```text
<= 16 visible characters -> validated
>= 17 visible characters -> menu enumeration stalls
```

`Throttle Invert` is intentionally 15 characters.

**Architecture rule:** user-visible CRSF parameter names must remain at 16 characters or fewer unless longer names are explicitly revalidated on the target EdgeTX/ExpressLRS stack.

Startup self-tests should protect the production parameter registry, ordering, serialization, and write routing.

### FailsafePolicy

Owns deterministic receiver-loss output:

- Roll/Pitch/Yaw centered
- Throttle minimum
- AUX 1-4 centered
- all buttons released

### BootButton

Low-level physical input only.

### MaintenanceController

Owns BOOT interaction policy.

Current visible rotation:

```text
release <2 s -> Link Quality diagnostic
2-4 s        -> Bind / blue
4-6 s        -> No Action / Cancel
continue     -> repeat in 2-second steps
```

Actions execute on release only.

Wi-Fi and receiver factory reset are intentionally absent because no supported FC-facing CRSF UART command was identified for those operations on ExpressLRS 3.4.3.

### StatusDisplay / StatusLed

`StatusDisplay` arbitrates ownership. `StatusLed` only renders physical color/pattern requests.

## 6. RC Health and Failsafe

RC validity is based only on valid `0x16` RC Channels frames.

```text
valid RC
  -> RC frames stop
  -> 500 ms timeout
  -> FailsafePolicy
  -> purple status
  -> valid RC resumes
  -> live mapped state
  -> green status
```

Link Statistics may continue during RC loss and must not restore healthy state.

## 7. HID Architecture

Current proportional mapping:

```text
X        CH1  Roll
Y        CH2  Pitch
Slider 1 CH3  Throttle
Slider 2 CH4  Yaw
Z        CH13 AUX 1
Rx       CH14 AUX 2
Ry       CH15 AUX 3
Rz       CH16 AUX 4
```

With RP2 firmware 3.4.3, CH13-CH16 are hardware validated as proportional in the reference 333 Hz Full / 16ch Rate/2 configuration.

## 8. Startup Self-Test Policy

Deterministic startup tests protect protocol and safety invariants where practical.

Current coverage includes:

- CRSF parsing/decoding
- outbound extended-frame construction
- Device Ping / Device Info
- parameter registry count/order/serialization
- throttle parameter production label
- valid/invalid inversion writes
- Restore Defaults behavior
- Diagnostics / Failsafe Count
- complete failsafe output policy
- firmware version/identity consistency

A failed invariant may intentionally prevent normal startup rather than permit a malformed protocol registry to run.

## 9. Future Receiver Firmware Update Mode

A future feature is reserved conceptually for a complete **Receiver Firmware Update / USB Serial Passthrough** workflow.

Potential architecture:

```text
future red selection
    -> supported receiver bootloader transition
    -> QT Py enters dedicated USB serial personality
    -> PC flashing tool communicates through UART passthrough
    -> receiver firmware update
```

This is a feature addition, not current release cleanup.

Open questions include USB CDC/tool compatibility, baud transitions, DTR/RTS expectations, buffering, interrupted-flash recovery, re-enumeration, and exit behavior.

Do not expose a bootloader-only red action until the complete user recovery/update path is designed.

## 10. Telemetry and Diagnostics Boundary

Diagnostics currently contains only `Failsafe Count`.

Do not consume legitimate CRSF telemetry sensor semantics for bridge bookkeeping. Future real telemetry forwarding from a PC/simulator remains possible and should use semantically correct CRSF telemetry when implemented.

## 11. Compatibility and Portability

ExpressLRS is the hardware-validated CRSF transport.

TBS Crossfire and Tracer are expected CRSF compatibility targets but remain unvalidated. Future validation should be community-driven rather than blocking current releases.

Board portability should preserve clean board-specific boundaries, but additional RP2040 targets should be demand-driven.

## 12. Release Architecture Rule

`v1.0.0` is the immutable stable baseline.

The current post-v1.0 feature set is at feature freeze. No new runtime functionality should enter before documentation synchronization, complete regression, and the next feature release.

Future work should resume only after architecture/roadmap review.
