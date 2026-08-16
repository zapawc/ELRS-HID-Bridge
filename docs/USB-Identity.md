# USB Identity Policy

**Status:** v1.0 policy fixed  
**Updated:** August 2026

## 1. Purpose

ELRS-HID-Bridge should identify as the project, not as the RP2040 development platform used to implement it.

The user-visible strings are therefore standardized as:

```text
USB product       ELRS-HID-Bridge
HID interface     ELRS-HID-Bridge
USB manufacturer  zapawc
CRSF device       ELRS-HID-Bridge
```

The internal PlatformIO environment name `pico` remains unchanged because it is only a build-target name.

## 2. Windows Validation Result

The firmware-side USB string change is validated.

On the reference Windows development system, PowerShell queried `DEVPKEY_Device_BusReportedDeviceDesc` for the active USB/HID instances and reported:

```text
Class         HIDClass
FriendlyName  USB Input Device
BusReported   ELRS-HID-Bridge

Class         USB
FriendlyName  USB Composite Device
BusReported   ELRS-HID-Bridge
```

The enumerated hardware ID remained:

```text
VID_2E8A&PID_000A
```

At the same time, `joy.cpl` continued to display the controller as:

```text
Pico
```

This proves that the project-controlled USB product descriptor is correct. The remaining `Pico` label is associated with the Windows game-controller identity path and the inherited VID/PID, not with an incorrect project product string.

Do not add registry-cleanup steps to the normal project installation workflow merely to change the label on one development machine.

## 3. Current VID/PID Policy

The current reference build inherits the RP2040/Arduino-Pico VID/PID:

```text
VID  0x2E8A
PID  0x000A
```

That pair is functional, but it does not give ELRS-HID-Bridge a distinct USB device identity. It may also cause host software to associate the device with a generic or previously cached Pico controller name.

The project must not invent a random VID/PID or claim a PID under a VID it does not control.

## 4. Optional Post-v1.0 Direction

If future distribution or production needs justify a distinct USB identity, the preferred open-source path is to request a dedicated PID for ELRS-HID-Bridge from **pid.codes** under open-source VID `0x1209`. This is not required for v1.0.

Reasons:

- ELRS-HID-Bridge has a public source repository.
- The project is licensed GPL-3.0-only.
- The firmware implements a distinct USB HID device identity.
- A unique VID/PID should prevent new host installations from conflating the bridge with the generic Pico identity.
- The allocation can be documented publicly with the source project.

Important qualification:

> pid.codes is an open-source community allocation service and explicitly states that it is not supported, endorsed, or associated with USB-IF.

The project should document that fact rather than presenting a pid.codes allocation as an official USB-IF assignment.

## 5. Future pid.codes Request Requirements

A future pid.codes request should be made only when the repository is ready to support the requested identity and there is a clear operational reason to change the v1.0 policy.

Their published prerequisites include:

- publicly available source,
- source code and/or modifiable hardware design for the USB device,
- a recognized open-source license in the repository.

The current project satisfies the basic source/license prerequisites. Because the reference design uses an off-the-shelf QT Py RP2040 rather than a custom PCB, pid.codes may request justification for why the software-defined USB device needs its own PID.

That justification is straightforward:

> ELRS-HID-Bridge is distributable GPL-3.0 firmware that turns a QT Py RP2040 plus an ExpressLRS receiver into a distinct USB HID joystick. The device exposes its own HID report descriptor, product identity, CRSF bridge behavior, failsafe policy, and release firmware. A unique PID prevents host operating systems from identifying distributed ELRS-HID-Bridge firmware as a generic Raspberry Pi Pico device.

Do not select or commit a candidate PID until immediately before submitting the request; pid.codes assignments are first-come and the requested number must still be unallocated when the pull request is submitted.

## 6. When a PID Is Allocated

After an allocation is accepted, update `platformio.ini` using Arduino-Pico's supported USB customization settings:

```ini
board_build.arduino.earlephilhower.usb_vid = 0x1209
board_build.arduino.earlephilhower.usb_pid = 0xXXXX
board_build.arduino.earlephilhower.usb_product = ELRS-HID-Bridge
board_build.arduino.earlephilhower.usb_manufacturer = zapawc
```

Replace `0xXXXX` only with the assigned PID.

Then perform a dedicated USB identity regression:

1. Build and flash the normal `pico` environment.
2. Disconnect/reconnect USB.
3. Confirm Windows enumerates the assigned VID/PID.
4. Confirm `BusReported` is `ELRS-HID-Bridge` on the USB parent and HID interface.
5. Confirm `joy.cpl` identifies the controller appropriately on a fresh enumeration.
6. Verify all eight analog axes and button mappings.
7. Verify deterministic failsafe and reconnect.
8. Verify Liftoff operation.
9. Verify EdgeTX **Other Devices** discovery remains unaffected.

Changing VID/PID is an identity change, not a CRSF/HID feature change, but any future VID/PID change still requires a full release regression.

## 7. Current Reference Identity

The inherited `0x2E8A:0x000A` identity is the deliberate v1.0 reference policy, not a temporary unmade decision. Documentation should consistently state that:

- USB descriptors report `ELRS-HID-Bridge`,
- some Windows game-controller views may still show `Pico`,
- this is a consequence of the inherited USB VID/PID rather than an incorrect HID product descriptor,
- no registry cleanup or custom driver is required for normal operation.


## v1.0 Decision

For v1.0, ELRS-HID-Bridge will retain the inherited RP2040/Pico USB VID/PID `0x2E8A:0x000A`.

Windows has been verified to report `ELRS-HID-Bridge` as the bus-reported product description for both the USB composite parent and HID interface. `joy.cpl` may still display `Pico` because of the inherited VID/PID and Windows game-controller naming behavior. This is a documented cosmetic limitation, not a functional defect.

A dedicated VID/PID allocation is therefore **not a v1.0 release blocker**. It can be reconsidered later if the project evolves into distributed production hardware where a unique USB identity has clear operational value.

Do not invent or appropriate an unallocated VID/PID merely to change the `joy.cpl` display name.
