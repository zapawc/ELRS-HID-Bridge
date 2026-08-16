# ELRS HID Bridge Release Guide

**Status:** Pre-v1.0 release hardening  
**Current development version:** `0.3.0-dev`  
**Updated:** August 2026

## 1. Purpose

This document is the release-policy companion to `Roadmap.md`.

The runtime CRSF-to-HID feature set is now intentionally frozen for the v1.0 cycle. Remaining work should make the proven reference build reproducible, identifiable, licensed, documented, and easy to release without expanding protocol scope.

---

## 2. Canonical Version Source

Firmware version information lives in:

```text
src/firmware_version.h
```

Current version:

```text
0.3.0-dev
```

Do not maintain an independent firmware version constant in `bridge_identity.h`, `main.cpp`, or protocol code.

### CRSF Firmware ID encoding

CRSF Device Info defines `Firmware_ID` as a 32-bit field. ELRS-HID-Bridge packs the semantic version tuple as:

```text
bits 31..24  major
bits 23..16  minor
bits 15..8   patch
bits 7..0    reserved (0)
```

Current value:

```text
0.3.0 -> 0x00030000
```

The human-readable prerelease suffix (`-dev`) is not encoded in this numeric field.

`FirmwareVersionSelfTest` verifies both the canonical version metadata and that `BridgeIdentity::CRSF_FIRMWARE_ID` consumes the same source.

---

## 3. Reference CRSF Identity

The v1.0 reference implementation uses:

```text
Address        0xC8        Flight Controller
Name           ELRS-HID-Bridge
Serial Number  0x45484231  "EHB1"
Hardware ID    0x51545059  "QTPY"
Parameters     0
```

`0xC8` has been hardware validated through the RP2/Ranger/EdgeTX path and is retained for the reference build.

`EHB1` and `QTPY` are stable project-defined identifiers. `EHB1` is a project-family identifier, not a unique serial number assigned to each physical bridge.

---

## 4. Build Environments

`platformio.ini` defines two environments:

```text
pico        normal HID-only release build
pico_debug  HID + USB CDC debug logging
```

Routine release validation should use the normal VS Code/PlatformIO workflow and the `pico` environment.

Use the CLI only when troubleshooting specifically requires it; the documented reference workflow should not depend on users manually invoking `pio` commands.

---

## 5. Remaining v1.0 Release Gates

### Release blocking

- [ ] Select and add an open-source license.
- [ ] Validate the wiring instructions against the final reference hardware.
- [ ] Validate the EdgeTX/ExpressLRS setup instructions from a clean setup perspective.
- [ ] Perform a clean `pico` build using the documented environment.
- [ ] Flash the release candidate and run the full hardware regression checklist.
- [ ] Produce and retain the tested release firmware binary.
- [ ] Define the Git tag / GitHub Release procedure.
- [ ] Perform final README/Architecture/Protocol/Roadmap/Release synchronization.

### Already complete

- [x] Stable CRSF-to-HID control path.
- [x] Eight-axis HID profile.
- [x] Switch/button mapping.
- [x] Deterministic complete failsafe policy.
- [x] Automatic reconnect.
- [x] Startup protocol and failsafe self-tests.
- [x] Bidirectional CRSF transport foundation.
- [x] Identity-only Device Ping -> Device Info discovery.
- [x] EdgeTX discovery under Other Devices.
- [x] Canonical firmware version source.
- [x] Deterministic CRSF Firmware ID encoding.

---

## 6. Version Transition Rules

During active development, use a semantic version with a prerelease suffix, for example:

```text
0.3.0-dev
```

For a release candidate, update `firmware_version.h` first, then synchronize release documentation in the same commit.

Do not make the Git tag the only version source. The firmware must carry a deterministic version identity even when built outside GitHub.

For v1.0, the intended final version string is:

```text
1.0.0
```

The corresponding packed CRSF Firmware ID would be:

```text
0x01000000
```

Do not change to `1.0.0` until the release-blocking checklist above is complete and the candidate has passed the full hardware regression.

---

## 7. Release Regression Checklist

Before tagging a release candidate, verify at minimum:

- USB HID enumerates normally.
- Roll direction/range correct.
- Pitch direction/range correct.
- Throttle direction/range correct.
- Yaw direction/range correct.
- AUX proportional axes behave as documented.
- Switch/button mapping correct.
- TX-off triggers failsafe at approximately 500 ms.
- Throttle moves to minimum on failsafe.
- All other analog axes center on failsafe.
- All buttons release on failsafe.
- Purple receiver-loss state displays correctly.
- TX reconnect automatically restores live HID control.
- Link Statistics do not override RC timeout state.
- BOOT short-press diagnostic works.
- Maintenance hold selections remain non-destructive.
- `ELRS-HID-Bridge` appears under EdgeTX **Other Devices**.
- Live Device Info TX does not degrade 333 Hz Full/Liftoff operation.
- Liftoff control/performance remains normal.

---

## 8. Scope Freeze

Do not delay v1.0 for:

- CRSF parameter configuration,
- persistent configuration,
- Bind/Wi-Fi receiver commands,
- desktop support software,
- additional receiver protocols,
- custom PCB work,
- application-specific integrations.

Any exception should be justified by a demonstrated release-blocking defect in the existing reference design.
