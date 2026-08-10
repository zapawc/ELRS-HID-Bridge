# ELRS HID Bridge Architecture

## Objective

Convert RC channel data received over CRSF into a standard USB HID joystick.

The USB device presented to the host exposes:

- X axis
- Y axis
- Slider 1
- Slider 2
- 32 buttons

The HID descriptor is intended to remain stable once validated against Windows and Liftoff.

---

## Design Goals

- Standard USB HID joystick
- No custom Windows driver
- No required desktop application
- CRSF receiver isolated from USB implementation
- Testable without an RF receiver attached
- Simple firmware structure
- Reproducible builds
- Suitable for open-source distribution
- Future protocol support without redesigning the HID layer

---

## Data Flow

```text
+--------------------+
|  Input Provider    |
|                    |
|  Test Generator    |
|       or           |
|  CRSF Receiver     |
+---------+----------+
          |
          v
+--------------------+
|   Channel State    |
|                    |
| roll      uint16   |
| pitch     uint16   |
| throttle  uint16   |
| yaw       uint16   |
| buttons   uint32   |
+---------+----------+
          |
          v
+--------------------+
|     USB HID        |
|                    |
| Roll      -> X     |
| Pitch     -> Y     |
| Throttle  -> Slider|
| Yaw       -> Slider|
| Buttons   -> 1-32  |
+---------+----------+
          |
          v
+--------------------+
|      Host PC       |
|                    |
| joy.cpl / Liftoff  |
+--------------------+