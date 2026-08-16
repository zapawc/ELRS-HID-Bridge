# GPL + USB Identity Checkpoint

## Purpose

This checkpoint performs v1.0 project-identity and release-hygiene work without changing CRSF-to-HID control behavior.

It does four things:

1. licenses ELRS-HID-Bridge project code under GPL-3.0-only;
2. records original-project attribution;
3. documents the current third-party dependency/license audit;
4. changes the user-visible USB/HID identity from the generic Pico wording to `ELRS-HID-Bridge`.

## Add these files

```text
LICENSE
AUTHORS.md
THIRD_PARTY_NOTICES.md
```

## Replace these files

```text
platformio.ini
src/usb_hid.cpp
README.md
docs/Architecture.md
docs/Roadmap.md
docs/Protocol.md
docs/Release.md
```

No other source files should change for this checkpoint.

## Expected functional change

The only intended runtime-visible change is USB identity:

```text
USB product:       ELRS-HID-Bridge
HID interface:     ELRS-HID-Bridge
USB manufacturer:  zapawc
```

The internal PlatformIO environment remains named `pico`. That is a build-target label and is not intended to be user-facing.

## Test sequence

1. Build the normal `pico` environment in VS Code / PlatformIO.
2. Confirm there are no build errors and no new VS Code Problems.
3. Flash the QT Py RP2040.
4. Unplug and reconnect USB once after flashing.
5. Open `joy.cpl` and record the controller name shown by Windows.
6. Verify all eight analog HID axes and the expected buttons.
7. Verify TX-off failsafe after approximately 500 ms:
   - throttle -> minimum;
   - all other analog axes -> center;
   - all buttons -> released;
   - LED -> purple.
8. Power the transmitter back on and verify automatic recovery.
9. Run the normal Liftoff regression.
10. Open ExpressLRS Lua -> Other Devices and verify `ELRS-HID-Bridge` is still discoverable.

## Windows naming note

Windows can cache game-controller naming information. If `joy.cpl` still shows `Pico` after the firmware is flashed and USB has been unplugged/reconnected, **do not change VID/PID or alter the HID descriptor yet**. Report the observed name and continue the functional regression. The next troubleshooting step should distinguish a stale Windows name cache from an incorrect USB descriptor.

## License validation

Confirm GitHub recognizes the root `LICENSE` as GNU GPL v3 after the commit is pushed.

## Suggested commit message

```text
Add GPL-3.0 licensing and normalize USB identity
```
