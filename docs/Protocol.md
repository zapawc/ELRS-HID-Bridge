# CRSF Protocol Notes

**Version:** 0.2  
**Status:** Draft

---

## Supported Protocol

Current development targets the Team BlackSheep Crossfire (CRSF) serial protocol as implemented by ExpressLRS receivers.

Official specification:

https://github.com/tbs-fpv/tbs-crsf-spec

---

## Current Frame Support

| Frame | Description | Status |
|--------|-------------|--------|
| 0x16 | RC Channels | In Progress |
| 0x14 | Link Statistics | Planned |

Additional frame types will be added as needed.

---

## Frame Structure

```
+---------+--------+------+---------+------+
| Address | Length | Type | Payload | CRC  |
+---------+--------+------+---------+------+
```

The CRC is calculated over:

```
Type + Payload
```

The Address and Length bytes are excluded from the CRC calculation.

---

## CRC

Polynomial:

```
0xD5
```

Current implementation uses a software CRC calculator.

---

## RC Channel Frame

Frame Type:

```
0x16
```

Payload Size:

```
22 bytes
```

Encoding:

- 16 channels
- 11 bits per channel
- Packed consecutively
- Little-endian bit ordering

Typical channel values:

| Value | Meaning |
|------:|---------|
| 172 | Minimum |
| 992 | Center |
| 1811 | Maximum |

---

## Internal Processing

Incoming CRSF data flows through the following stages:

```
UART

↓

Frame Assembly

↓

CRC Validation

↓

Frame Dispatch

↓

RC Channel Decode

↓

RawChannels

↓

Channel Normalization

↓

Application Mapping

↓

USB HID
```

Only the protocol layer understands CRSF-specific details.

All higher layers remain protocol independent.