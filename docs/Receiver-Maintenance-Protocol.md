# Receiver Maintenance Protocol Research

**Date:** 2026-08-17  
**Reference receiver:** RadioMaster RP2 2.4 GHz  
**Validated receiver firmware:** ExpressLRS 3.4.3

## Purpose

Record the protocol basis for physical BOOT-button receiver maintenance actions and prevent unsupported CRSF/ExpressLRS commands from being invented.

## Bind

Status: **implemented and hardware validated**.

ExpressLRS 3.4 added receiver-side handling for the standard CRSF receiver Bind command. The FC-side UART command is a CRSF `COMMAND` (`0x32`) frame addressed to the CRSF receiver (`0xEC`) from the flight-controller address (`0xC8`), using receiver command realm `0x10` and Bind subcommand `0x01`.

The bridge implementation generates this frame through `CrsfReceiverCommand` rather than hard-coding the complete frame in `main.cpp`.

Hardware validation was completed successfully on a RadioMaster RP2 running ExpressLRS 3.4.3.

### Compatibility requirement

Receiver-side BOOT-button Bind requires **ExpressLRS 3.4.0 or newer**. Receivers on earlier firmware should not be expected to respond to the CRSF receiver Bind command.

## Wi-Fi

Status: **not implemented; no supported FC-side UART command identified for ExpressLRS 3.4.3**.

ExpressLRS 3.4.3 defines the ELRS-specific opcode:

```text
MSP_ELRS_SET_RX_WIFI_MODE = 0x0E
```

The receiver handles that opcode in the assembled ExpressLRS MSP data path and then defers a call to `setWifiUpdateMode()` so the MSP transfer can be acknowledged first.

That path is distinct from the receiver's FC-side CRSF UART parser. In ExpressLRS 3.4.3, the FC-side UART parser explicitly handles receiver Bind, bootloader, model-match, and Device Ping internal cases, but does not expose a corresponding Wi-Fi transition command.

Therefore the bridge must **not** construct a guessed `0x32` Wi-Fi command or attempt to reuse `MSP_ELRS_SET_RX_WIFI_MODE` as though it were an FC-side UART command.

### Current disposition

- Keep the 4-6 second white Wi-Fi selection reserved.
- Release during the Wi-Fi selection must remain non-transmitting.
- Revisit only if an authoritative ExpressLRS implementation adds or documents a receiver-side FC-UART mechanism that can legitimately enter Wi-Fi mode.

## Receiver Reset / Recovery

Status: **not yet researched to completion**.

No reset/recovery implementation should be added until an authoritative receiver-side mechanism is identified. The blinking-red slot remains reserved and execute-on-release safety remains mandatory.

## Authoritative implementation references

Research was based on ExpressLRS 3.4.3 source and the public CRSF receiver command definition, specifically:

- `ExpressLRS/src/src/rx-serial/SerialCRSF.cpp`
- `ExpressLRS/src/lib/Telemetry/telemetry.cpp`
- `ExpressLRS/src/src/rx_main.cpp`
- `ExpressLRS/src/lib/MSP/msptypes.h`
- CRSF receiver command realm (`0x10`) and Bind subcommand (`0x01`)

The implementation rule remains: supported receiver maintenance commands only; no guessed frame IDs, destinations, subcommands, or payloads.
