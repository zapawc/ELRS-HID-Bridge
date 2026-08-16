# CRSF Protocol Notes

**Status:** Active pre-v1.0 implementation notes  
**Updated:** August 2026

## 1. Supported Protocol

ELRS-HID-Bridge targets the Team BlackSheep Crossfire (CRSF) serial protocol as implemented by ExpressLRS receivers.

Official specification:

https://github.com/tbs-fpv/tbs-crsf-spec

Reference UART configuration:

```text
420000 baud
```

---

## 2. Current Frame Support

| Frame | Description | Current status |
|---|---|---|
| `0x14` | Link Statistics | Decoded and used for diagnostics only |
| `0x16` | RC Channels Packed | Decoded; primary HID control source |
| `0x28` | Parameter Ping Devices / Device Ping | Recognized and routed to `CrsfDevice` |
| `0x29` | Parameter Device Information / Device Info | Construction self-tested; live response TX enabled for discovery POC; hardware routing validation pending |
| `0x2B` | Parameter Settings Entry | Defined for future work; not implemented |
| `0x2C` | Parameter Read | Defined for future work; not implemented |
| `0x2D` | Parameter Write | Defined for future work; not implemented |
| `0x32` | Command | Defined for future work; not implemented |

Unsupported but structurally valid frame types are intentionally ignored by the dispatcher.

---

## 3. Frame Structure

### Broadcast frame

```text
+---------+--------+------+---------+------+
| Sync    | Length | Type | Payload | CRC  |
+---------+--------+------+---------+------+
```

### Extended-header frame

```text
+---------+--------+------+-------------+--------+---------+------+
| Sync    | Length | Type | Destination | Origin | Payload | CRC  |
+---------+--------+------+-------------+--------+---------+------+
```

CRSF frames are at most 64 bytes including Sync and CRC.

The Length byte represents the number of bytes after Sync and Length:

```text
Broadcast:
Type + Payload + CRC

Extended:
Type + Destination + Origin + Payload + CRC
```

Valid CRSF length range:

```text
2 .. 62
```

---

## 4. Synchronization / Address Handling

The implementation no longer assumes that every valid frame begins only with `0xC8`.

`CrsfParser` validates frame-start bytes against known CRSF serial/device address values defined in `crsf_protocol.h`.

Important CRSF addresses currently represented include:

```text
0x00 Broadcast
0x10 USB Device
0xC8 Flight Controller / traditional serial sync
0xEA Remote Control
0xEC Receiver
0xEE Transmitter
```

The official CRSF specification permits the serial sync byte, broadcast address, or a device address to appear as the first byte of a received frame.

---

## 5. CRC

Polynomial:

```text
0xD5
```

The normal CRSF frame CRC covers bytes beginning at Type and ending at the final payload byte.

Therefore:

```text
Broadcast CRC input:
Type + Payload

Extended-frame CRC input:
Type + Destination + Origin + Payload
```

Sync/Address and Length are excluded from the CRC calculation.

---

## 6. RC Channels (`0x16`)

Payload size:

```text
22 bytes
```

Encoding:

- 16 channels
- 11 bits per channel
- packed consecutively
- explicit unpacking in firmware

Reference CRSF values commonly used by the project:

| Value | Meaning |
|---:|---|
| 172 | Minimum |
| 992 | Center |
| 1811 | Maximum |

The firmware does not use compiler-dependent packed C/C++ bitfields for this payload.

### Processing path

```text
CrsfParser
    |
CrsfDispatcher
    |
RcChannelDecoder
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

Receipt of a valid `0x16` frame is the primary fact used to maintain RC-active state.

---

## 7. Link Statistics (`0x14`)

Link Statistics are decoded for diagnostics such as Link Quality, RSSI, and SNR.

Architecture rule:

```text
Link Statistics received != RC control healthy
```

The RP2 has been observed continuing to emit `0x14` after valid RC Channels frames stopped during transmitter loss.

Therefore Link Statistics cannot reset or override the RC timeout/failsafe state.

---

## 8. Device Ping (`0x28`)

`0x28` is an extended-header frame.

Routing fields:

```text
Destination
Origin
```

The official CRSF Device Ping has no defined application payload.

The current `CrsfDevice` handler requires at least Destination and Origin and tolerates additional trailing bytes for compatibility/protocol investigation.

A ping may target:

- `0x00` Broadcast, or
- one specific CRSF device address.

Current production behavior:

```text
Device Ping recognized
    ->
routing retained
    ->
builder accepts only broadcast or local-address traffic
    ->
Device Info constructed
    ->
CrsfUart::write() sends one response attempt
    ->
ping cleared
```

---

## 9. Device Info (`0x29`)

The official Device Info payload is:

```text
char[]   Device_name             // null-terminated
uint32_t Serial_number
uint32_t Hardware_ID
uint32_t Firmware_ID
uint8_t  Parameters_total
uint8_t  Parameter_version_number
```

CRSF uses big-endian byte ordering for multi-byte values.

### Current implementation checkpoint

`CrsfDevice::buildDeviceInfoResponse()` constructs the complete extended Device Info frame. The production loop now calls the builder for received Device Ping traffic and passes successful responses to `CrsfUart::write()`.

Inputs:

- recognized `CrsfDevicePing`
- caller-supplied local CRSF device address
- caller-supplied `CrsfDeviceIdentity`
- caller output buffer/capacity

Current response policy:

```text
if ping destination == Broadcast
    respond
else if ping destination == local device address
    respond
else
    do not construct a response
```

Response routing:

```text
Destination = ping Origin
Origin      = local device address
```

Frame construction:

```text
Sync = 0xC8 serial sync
Type = 0x29 Device Info
Destination = ping origin
Origin = caller-supplied local address
Payload = Device Info fields
CRC = DVB-S2 CRSF CRC over Type through payload
```

### Current live discovery identity

The first hardware proof-of-concept uses:

```text
Local CRSF address  0xC8  Flight Controller
Device name         ELRS-HID-Bridge
Serial Number       0x45484231  (POC: "EHB1")
Hardware ID         0x51545059  (POC: "QTPY")
Firmware ID         0x00000001  (POC)
Parameters total    0
Parameter version   0
```

`0xC8` was selected because the bridge occupies the flight-controller side of the RP2 UART. The three 32-bit identity values are deterministic proof-of-concept constants only; they are not claimed to be globally assigned identifiers and are not yet release identity policy.

These values are isolated in `bridge_identity.h`. The generic construction layer still accepts caller-supplied address/identity data so hardware findings can change project policy without changing CRSF encoding mechanics.

---

## 10. Device Info Self-Test Coverage

Startup tests now verify:

- known valid Device Ping recognition
- extra Device Ping trailing-byte tolerance
- rejection of malformed Device Ping lacking routing fields
- non-Ping frames are not surfaced as Device Ping
- broadcast ping -> Device Info construction
- directly addressed ping -> Device Info construction
- unrelated destination -> no response construction
- response Destination = ping Origin
- response Origin = local address
- null-terminated `Device_name`
- big-endian 32-bit identity fields
- parameter count/version placement
- CRSF Length field
- CRSF CRC

These tests are deterministic and do not transmit on the live UART.

---

## 11. Outbound Extended-Frame Encoder

`CrsfFrameEncoder` owns generic extended-header frame construction.

It receives:

- sync/address byte
- frame type
- destination
- origin
- payload pointer/length
- output buffer/capacity

It validates frame sizing/capacity, writes the extended header, copies the payload, computes CRC, and returns the complete frame length.

`CrsfDevice` uses this encoder rather than implementing a second frame/CRC path.

---

## 12. Current Bidirectional Boundary

The reference wiring includes both UART directions:

```text
RP2 TX -> QT Py RX
RP2 RX <- QT Py TX
```

`CrsfUart` provides both receive and transmit primitives.

The production firmware now uses the transmit primitive for exactly one purpose: reply to an eligible Device Ping with the already self-tested Device Info frame.

No parameter entries, writes, commands, telemetry sensors, Bind/Wi-Fi commands, or other outbound CRSF behavior are enabled by this checkpoint.

The next checkpoint is hardware observation: verify whether the RP2/Ranger/EdgeTX route carries the response and whether EdgeTX discovers `ELRS-HID-Bridge`.

---

## 13. ExpressLRS Channel-Resolution Observation

Reference testing moved from 250 Hz Wide to:

```text
333 Hz Full
16ch Rate/2
```

This allowed CH13 and CH14 to behave proportionally.

With the tested ExpressLRS 3.3.1 receiver firmware, CH15/CH16 remained high. Diagnostic remapping showed this follows the CRSF channels rather than HID axes.

No protocol/HID workaround is currently implemented.

---

## 14. Protocol State vs Application State

CRSF decoders provide protocol facts.

`BridgeState` and higher-level application code determine operational state.

Examples:

```text
valid 0x16 received
    -> RC frame fact
    -> refresh RC-active timeout

valid 0x14 received
    -> Link Statistics fact
    -> update diagnostics only

0x16 absent for 500 ms
    -> receiver timeout
    -> FailsafePolicy applied
```

This separation is a core safety/maintainability rule.

---

## 15. Planned Protocol Work

Immediate:

1. build and flash the live Device Info TX checkpoint,
2. verify RP2/Ranger/EdgeTX routing,
3. look for `ELRS-HID-Bridge` through the ExpressLRS Lua `Other Devices` path,
4. verify 333 Hz Full RC/HID/failsafe/reconnect behavior is unchanged,
5. document the observed routing behavior before changing address policy or adding protocol features.

Post-v1.0 candidates:

- CRSF parameter entries (`0x2B`)
- parameter reads (`0x2C`)
- parameter writes (`0x2D`)
- commands (`0x32`) where justified
- bridge health information
- persistent configuration integration

Do not build the full parameter system until identity-only discovery has been proven.
