# Receiver Maintenance Protocol Research

**Date:** 2026-08-17  
**Reference receiver:** RadioMaster RP2 2.4 GHz  
**Validated receiver firmware:** ExpressLRS 3.4.3

## Purpose

Record the protocol basis for BOOT-button receiver maintenance and prevent unsupported CRSF/ExpressLRS commands from being invented.

## Current User Interface

The supported maintenance rotation is intentionally minimal:

```text
release <2 s -> Link Quality diagnostic
2-4 s        -> Bind / blue
4-6 s        -> No Action / Cancel
continue     -> repeat Bind -> No Action
```

Actions execute only when BOOT is released.

## Bind

**Status: implemented and hardware validated.**

ExpressLRS 3.4 added receiver-side handling for the standard CRSF receiver Bind command.

FC-side command:

```text
Frame type   0x32  COMMAND
Destination  0xEC  Receiver
Origin       0xC8  Flight Controller
Realm        0x10  Receiver command
Subcommand   0x01  Bind
```

The bridge generates the frame through `CrsfReceiverCommand`.

Hardware validation succeeded on RadioMaster RP2 running ExpressLRS 3.4.3.

### Compatibility requirement

Receiver-side BOOT-button Bind requires **ExpressLRS receiver firmware 3.4.0 or newer**.

## Wi-Fi

**Status: intentionally not implemented.**

ExpressLRS 3.4.3 defines `MSP_ELRS_SET_RX_WIFI_MODE = 0x0E` in its transmitter-to-receiver MSP/RF control path. That path is distinct from the receiver's FC-facing CRSF UART parser.

No equivalent supported FC-side CRSF UART Wi-Fi command was identified.

Therefore:

- the bridge does not synthesize a guessed `0x32` Wi-Fi command;
- the former white Wi-Fi selection has been removed from the visible BOOT rotation;
- normal receiver automatic Wi-Fi behavior remains available;
- Wi-Fi shortcut support may be reconsidered if upstream ExpressLRS later exposes a legitimate FC-UART mechanism.

## Receiver Factory Reset

**Status: retired / not implemented.**

No supported FC-facing CRSF UART command for clearing receiver configuration was identified.

The bridge does not fake this function. The former blinking-red factory-reset selection has been removed.

## Future Receiver Firmware Update / Serial Passthrough

A receiver bootloader transition is a legitimate basis for a future maintenance feature, but bootloader entry alone is not considered sufficient.

The desired future user story is:

```text
future blinking-red Firmware Update selection
    -> receiver enters bootloader
    -> QT Py enters dedicated USB serial/passthrough mode
    -> PC flashing tool communicates through QT Py UART
    -> receiver firmware is updated
```

Open work before implementation:

- confirm ExpressLRS Configurator direct-UART expectations
- confirm USB CDC/serial compatibility
- determine baud-rate transitions
- determine DTR/RTS requirements
- validate receiver bootloader behavior
- design buffering/flow control
- design interrupted-flash recovery
- design USB re-enumeration
- define safe update-mode exit/recovery

The future red indication should mean **Receiver Firmware Update**, not factory reset.

This feature is explicitly deferred until after the current release cycle.
