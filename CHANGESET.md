# v1.1.0 Version Identity Changeset

Updates the canonical firmware identity and its independent startup release assertion from v1.0.0 to v1.1.0.

## Files

- `src/firmware_version.h`
- `src/firmware_version_self_test.cpp`

## Expected identity

- MAJOR: 1
- MINOR: 1
- PATCH: 0
- STRING: `1.1.0`
- CRSF_ID: `0x01010000`

The self-test remains deliberately independent of the canonical values so incomplete release-version changes are detected at startup.

## Validation

Build and upload only the normal `pico` environment. Do not commit until the board passes startup, returns to normal operation, HID is live, and the EdgeTX device menu enumerates normally.
