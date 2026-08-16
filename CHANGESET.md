# Release Identity / Version Hardening Checkpoint

This checkpoint makes firmware/version identity deterministic without adding new runtime features.

## Files to add

```text
src/firmware_version.h
src/firmware_version_self_test.h
src/firmware_version_self_test.cpp
docs/Release.md
```

## Files to replace

```text
src/bridge_identity.h
src/main.cpp
README.md
docs/Architecture.md
docs/Roadmap.md
docs/Protocol.md
```

## Behavioral scope

No RC mapping, HID behavior, CRSF routing, failsafe policy, BOOT behavior, or LED behavior is intentionally changed.

The only firmware-runtime difference is one additional deterministic startup self-test for version/identity consistency.

## Version policy introduced

```text
Human version:       0.3.0-dev
CRSF Firmware ID:    0x00030000
CRSF address:        0xC8
CRSF Serial Number:  0x45484231 (EHB1 project-family ID)
CRSF Hardware ID:    0x51545059 (QTPY reference hardware)
```

The Firmware ID is derived from `src/firmware_version.h`; it is no longer independently hard-coded in `bridge_identity.h`.

## Validation

1. Replace/add the files above.
2. Use the normal VS Code PlatformIO build for the `pico` environment.
3. Confirm no build errors and no VS Code Problems.
4. Flash normally.
5. Confirm startup reaches the expected healthy state; a version self-test failure would produce the existing fatal/self-test behavior.
6. Run the normal basic HID regression.
7. Verify transmitter-off deterministic failsafe and reconnect.
8. Verify Liftoff remains normal.
9. Verify `ELRS-HID-Bridge` still appears under EdgeTX **Other Devices**.

Because CRSF Device Info contains a numeric Firmware ID rather than a human-readable semantic version string, this checkpoint does not require the EdgeTX UI to visibly display `0.3.0-dev`.

## Suggested commit

```text
Centralize firmware version and CRSF release identity
```
