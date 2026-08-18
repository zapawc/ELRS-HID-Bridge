# CRSF Protocol Notes

**Status:** post-v1.0 configuration/maintenance protocol baseline  
**Updated:** August 2026

## 1. Supported Protocol

ELRS-HID-Bridge targets the Team BlackSheep Crossfire (CRSF) serial protocol as implemented by ExpressLRS receivers.

Reference UART:

```text
420000 baud
```

## 2. Current Frame Support

| Frame | Description | Current status |
|---|---|---|
| `0x14` | Link Statistics | Decoded for diagnostics only |
| `0x16` | RC Channels Packed | Decoded; primary HID control source |
| `0x28` | Device Ping | Recognized/routed |
| `0x29` | Device Info | Constructed/transmitted; hardware validated |
| `0x2B` | Parameter Settings Entry | Implemented for CRSF device parameters |
| `0x2C` | Parameter Read | Implemented |
| `0x2D` | Parameter Write | Implemented |
| `0x32` | Command | Implemented for supported bridge/receiver commands including receiver Bind |

Unsupported but structurally valid frames are intentionally ignored.

## 3. RC Health Rule

Valid `0x16` RC Channels frames are authoritative for RC-active state.

```text
Link Statistics received != RC control healthy
```

The RP2 may continue emitting `0x14` after RC frames stop. Link Statistics therefore cannot reset or override the RC timeout/failsafe state.

## 4. Device Identity

Reference CRSF identity:

```text
Local address   0xC8  Flight Controller
Device name     ELRS-HID-Bridge
Serial Number   0x45484231 ("EHB1")
Hardware ID     0x51545059 ("QTPY")
Firmware ID     derived from firmware_version.h
```

Device Ping / Device Info routing has been hardware validated through RP2 -> ELRS RF -> Ranger -> EdgeTX.

## 5. Parameter Service

The device now exposes a real parameter tree through standard CRSF device mechanisms.

Current root order:

```text
LED Brightness
Pitch Inversion
Throttle Invert
Roll Inversion
Yaw Inversion
Aux 1 Inversion
Aux 2 Inversion
Aux 3 Inversion
Aux 4 Inversion
Diagnostics
Restore Defaults
```

Diagnostics currently contains:

```text
Failsafe Count
```

Parameter writes update canonical `BridgeConfiguration` and persistent storage where appropriate.

Restore Defaults returns the bridge to canonical reference defaults.

## 6. Parameter Name Compatibility

Hardware troubleshooting established an empirical compatibility boundary on the validated EdgeTX/ExpressLRS path:

```text
15 characters -> works
16 characters -> works
17 characters -> enumeration stalls
18 characters -> enumeration stalls
```

The original `Throttle Inversion` label is 18 characters and reproduced the failure. The production label is `Throttle Invert` (15 characters).

**Project protocol rule:** keep user-visible CRSF parameter names at 16 characters or fewer unless longer names are explicitly revalidated against a future upstream stack.

This is treated as a compatibility constraint even though the upstream limitation has not been formally characterized by this project.

Do not test parameter enumeration by deliberately violating registry/count invariants; startup self-tests correctly reject malformed registries before normal boot.

## 7. Throttle Inversion Finding

A disposable diagnostic forced:

```cpp
configuration.throttle.inverted = true;
```

after configuration load.

Throttle then visibly inverted in Windows `joy.cpl`.

This proved the runtime path is healthy:

```text
CRSF CH3
 -> normalization
 -> ChannelMapper
 -> ChannelState
 -> HID Slider 1
```

The prior failure was therefore in CRSF parameter presentation/compatibility, not throttle normalization or HID mapping.

Failsafe semantics remain independent: throttle failsafe is always safe minimum.

## 8. Receiver Bind Command

Receiver Bind is implemented through CRSF `COMMAND` (`0x32`).

Validated ExpressLRS receiver command:

```text
Destination  0xEC  Receiver
Origin       0xC8  Flight Controller
Realm        0x10  Receiver command
Subcommand   0x01  Bind
```

The bridge constructs this through the receiver-command abstraction rather than hard-coding a complete frame in application logic.

Receiver-side handling requires ExpressLRS 3.4.0+ and is hardware validated on RadioMaster RP2 / ExpressLRS 3.4.3.

## 9. Unsupported Maintenance Commands

### Wi-Fi

ExpressLRS 3.4.3 contains an ELRS-specific MSP/RF Wi-Fi control path, but no equivalent supported FC-facing CRSF UART command was identified.

The bridge must not guess or spoof one.

### Receiver factory reset

No supported FC-facing CRSF UART receiver factory-reset command was identified.

The bridge does not implement one.

## 10. Future Bootloader / Passthrough Work

A legitimate receiver bootloader transition has been identified as a possible foundation for a future complete firmware-update feature.

That feature is intentionally deferred until the USB serial/passthrough and recovery workflow is designed and validated.

Bootloader entry by itself is not considered a complete user feature.

## 11. Protocol Safety Rules

- RC health derives from valid RC frames.
- Parameter/maintenance traffic cannot restore RC health.
- Parameter registry invariants are startup-tested.
- User-visible parameter names remain <=16 characters unless revalidated.
- Unsupported receiver commands are not invented.
- Optional outbound CRSF behavior must not starve HID reporting.
- Failsafe policy remains independent of live inversion settings.
