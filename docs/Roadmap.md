# ELRS HID Bridge Roadmap

**Status:** post-v1.0 feature freeze / next feature release preparation  
**Updated:** August 2026

## 1. Project Direction

ELRS-HID-Bridge is a minimal, open-source CRSF-to-USB HID bridge and reusable development foundation.

Reference hardware:

```text
RadioMaster RP2 + Adafruit QT Py RP2040
```

The project favors standard HID, standard CRSF mechanisms, transmitter-side configuration, deterministic failsafe behavior, and minimal hardware.

## 2. Stable Baseline — v1.0.0

`v1.0.0` is frozen and released.

Completed baseline:

- [x] robust CRSF parsing/CRC/stream recovery
- [x] 16-channel RC decode
- [x] Link Statistics decode
- [x] eight-axis USB HID
- [x] 32-button HID capacity
- [x] reference switch mapping
- [x] deterministic 500 ms failsafe
- [x] automatic reconnect
- [x] RGB status
- [x] BOOT diagnostic UI
- [x] parser/dispatcher architecture
- [x] bidirectional CRSF UART
- [x] Device Ping / Device Info discovery
- [x] canonical firmware identity/version
- [x] GPL-3.0-only release
- [x] release/checksum/manifest workflow
- [x] Liftoff hardware validation

## 3. Post-v1.0 Feature Cycle — Implemented

### CRSF / EdgeTX configuration

- [x] parameter registry/service
- [x] parameter read/write
- [x] LED Brightness
- [x] Pitch Inversion
- [x] Throttle Invert
- [x] Roll Inversion
- [x] Yaw Inversion
- [x] Aux 1-4 Inversion
- [x] Diagnostics folder
- [x] Failsafe Count
- [x] Restore Defaults

### Persistent configuration

- [x] persistent `BridgeConfiguration`
- [x] schema/version handling
- [x] validation
- [x] safe defaults
- [x] incompatible/corrupt fallback
- [x] migration behavior
- [x] persistent inversion settings
- [x] persistent LED brightness

### Receiver maintenance

- [x] repeating 2-second BOOT state machine
- [x] execute-on-release safety
- [x] short-press Link Quality diagnostic
- [x] receiver Bind
- [x] No Action / Cancel
- [x] ExpressLRS 3.4+ compatibility requirement identified
- [x] Bind hardware validated on RP2 / ExpressLRS 3.4.3

Wi-Fi and receiver factory reset were investigated and intentionally removed because no supported FC-facing CRSF UART command was identified for those operations.

### Throttle investigation

- [x] prove runtime throttle inversion path independently
- [x] isolate Lua enumeration failure from throttle mapping
- [x] identify parameter-name length as root cause
- [x] validate 16-character names
- [x] reproduce failure at 17+ characters
- [x] adopt production label `Throttle Invert`
- [x] add startup regression coverage
- [x] hardware validate inversion, persistence, defaults, failsafe, reconnect

## 4. Current Compatibility Rule

User-visible CRSF parameter names must remain **16 characters or fewer** unless longer names are explicitly revalidated against the target EdgeTX/ExpressLRS stack.

This rule is based on hardware testing in which 17- and 18-character parameter names stalled device-menu enumeration.

## 5. Current Hardware Validation

Reference configuration:

```text
333 Hz Full
16ch Rate/2
```

RadioMaster RP2 with ExpressLRS 3.4.3:

- [x] CH13 proportional
- [x] CH14 proportional
- [x] CH15 proportional
- [x] CH16 proportional
- [x] receiver Bind command

Receiver-side BOOT Bind requires ExpressLRS 3.4.0+.

## 6. Immediate Release Plan

Feature coding for this cycle is complete.

### Release blocking

- [ ] synchronize README / Architecture / Roadmap / Protocol / maintenance docs
- [ ] commit documentation synchronization
- [ ] normal `pico` build
- [ ] VS Code Problems clear
- [ ] verify USB HID enumeration
- [ ] verify all eight analog axes
- [ ] verify buttons
- [ ] verify every inversion control
- [ ] verify persistence across reboot
- [ ] verify Restore Defaults
- [ ] verify transmitter-off failsafe
- [ ] verify reconnect
- [ ] verify Diagnostics / Failsafe Count
- [ ] verify BOOT short diagnostic
- [ ] verify BOOT Bind
- [ ] verify BOOT No Action
- [ ] verify EdgeTX menu enumeration
- [ ] Liftoff smoke test
- [ ] choose next feature-release version
- [ ] update version/release metadata
- [ ] stage UF2 / checksum / manifest
- [ ] publish and verify GitHub release

No new runtime feature should be added before these gates are complete.

## 7. Pocketed Future Feature — Receiver Firmware Update

A future **Receiver Firmware Update / USB Serial Passthrough** feature is considered useful and feasible enough to retain on the roadmap.

Potential user flow:

```text
future blinking-red maintenance selection
    -> receiver bootloader
    -> QT Py dedicated USB serial/passthrough mode
    -> ExpressLRS flashing tool
    -> receiver firmware update
```

Required architecture/research before implementation:

- [ ] confirm ExpressLRS Configurator direct-UART behavior
- [ ] determine USB CDC compatibility requirements
- [ ] determine baud-rate transitions
- [ ] determine DTR/RTS requirements
- [ ] validate receiver bootloader transition
- [ ] design buffering/flow control
- [ ] design interrupted-flash recovery
- [ ] design USB re-enumeration
- [ ] define safe update-mode exit behavior
- [ ] preserve normal HID release personality

Do not expose a red bootloader action until the complete update/recovery workflow is usable.

## 8. Future Telemetry

Telemetry remains deliberately unconsumed by bridge bookkeeping.

Potential future direction:

```text
PC / simulator telemetry
    -> bridge
    -> semantically correct CRSF telemetry
    -> ELRS
    -> EdgeTX
```

Only implement telemetry when a real data source/use case exists.

## 9. CRSFJoystick Parity / Compatibility Workstream

Maintain an explicit compatibility comparison without cloning features for their own sake.

Future work:

- [ ] community Crossfire hardware validation
- [ ] community Tracer hardware validation
- [ ] document tested/expected RF systems
- [ ] publish enclosure/build assets
- [ ] publish concise BOM/wiring/build guide
- [ ] consider additional RP2040 targets only when demand justifies them

Crossfire/Tracer hardware purchases are not required from the primary maintainer.

## 10. Physical Build

A compact `.step` enclosure design now exists for the QT Py + receiver reference build.

Key design characteristic:

- integrated flexible arm actuates the onboard BOOT button

Future publication work:

- [ ] review/finalize enclosure
- [ ] add printable/exported formats as appropriate
- [ ] document assembly
- [ ] document BOOT flex-arm operation
- [ ] include enclosure in a future release/build guide

A custom PCB remains optional and is not required for product completeness.

## 11. Explicit Non-Goals

Do not pursue merely for feature completeness:

- generic SBUS/iBUS/PPM support
- mandatory OLED/display
- mandatory external button
- mandatory custom PCB
- mandatory desktop configuration software
- arbitrary diagnostic counters
- fake Wi-Fi/reset CRSF commands
- RTOS/multicore conversion without demonstrated need
- application-specific integrations in the critical HID path

## 12. Development Workflow

Preferred sequence:

```text
inspect live repository
    -> one coherent change
    -> complete-file ZIP overlay
    -> build
    -> deterministic/self-tests
    -> hardware test
    -> joy.cpl / EdgeTX validation
    -> Liftoff when control path changes
    -> commit
    -> sync/push
    -> verify GitHub
```

The normal `pico` environment is authoritative for release builds. Hardware behavior is authoritative for release-critical validation.

## 13. Next Architecture Review

After the current release is published, generate a new handoff and return to architecture/roadmap review before starting the next feature version.

Likely discussion candidates:

1. Receiver Firmware Update / USB Serial Passthrough
2. enclosure/build publication
3. community Crossfire/Tracer validation
4. genuine simulator/PC telemetry forwarding

The next version should be scoped deliberately rather than accumulating features simply because the platform can support them.
