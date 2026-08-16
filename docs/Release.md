# ELRS-HID-Bridge Release Guide

**Status:** v1.0 release-candidate validation  
**Current firmware version:** `1.0.0-rc1`  
**Updated:** August 2026

## 1. Purpose

This document defines the release procedure for the reference ELRS-HID-Bridge firmware.

The runtime CRSF-to-HID feature set is frozen for the v1.0 cycle. Release-candidate work should improve reproducibility, packaging, documentation, and validation. Do not add new CRSF/HID features unless a demonstrated release-blocking defect requires a runtime change.

The detailed validation sequence lives in `docs/Release-Checklist.md`.

---

## 2. Canonical Firmware Version

Firmware version information lives in:

```text
src/firmware_version.h
```

Current release candidate:

```text
1.0.0-rc1
```

Do not maintain an independent firmware version constant in `bridge_identity.h`, `main.cpp`, release scripts, or protocol code.

### CRSF Firmware ID encoding

CRSF Device Info exposes a 32-bit `Firmware_ID`. ELRS-HID-Bridge packs the semantic version tuple as:

```text
bits 31..24  major
bits 23..16  minor
bits 15..8   patch
bits 7..0    reserved (0)
```

For the current candidate:

```text
1.0.0-rc1 -> 0x01000000
```

The prerelease suffix is deliberately not encoded in the numeric CRSF field. Therefore `1.0.0-rc1` and final `1.0.0` intentionally share CRSF Firmware ID `0x01000000`.

`FirmwareVersionSelfTest` verifies the semantic-version constants/string and confirms that `BridgeIdentity::CRSF_FIRMWARE_ID` consumes the same canonical source.

---

## 3. Reference Identity

CRSF identity:

```text
Address        0xC8        Flight Controller
Name           ELRS-HID-Bridge
Serial Number  0x45484231  "EHB1"
Hardware ID    0x51545059  "QTPY"
Parameters     0
```

USB identity:

```text
USB product       ELRS-HID-Bridge
HID interface     ELRS-HID-Bridge
USB manufacturer  zapawc
VID/PID           0x2E8A:0x000A
```

Windows validation confirmed that the USB composite parent and HID interface report `ELRS-HID-Bridge` through `DEVPKEY_Device_BusReportedDeviceDesc`. `joy.cpl` may continue to display `Pico` because the v1.0 reference build intentionally retains the inherited RP2040/Pico VID/PID. This is a documented cosmetic limitation and is not a v1.0 blocker.

See `docs/USB-Identity.md` for the full policy.

---

## 4. Release Build Environment

`platformio.ini` defines:

```text
pico        normal HID-only release build
pico_debug  HID + USB CDC debug logging
```

All release candidates and published firmware binaries must come from the normal `pico` environment.

The v1.0 cycle pins the hardware-tested dependencies:

```text
Platform manifest        raspberrypi 1.20.0
Arduino-Pico framework   1.60000.0
RP2040 toolchain         5.160100.260719
Adafruit NeoPixel        1.15.5
```

The captured known-good environment was associated with project Git HEAD:

```text
75aa4ff7fbdb05b3251452f5f14a736a22174744
```

That commit is historical evidence for the pinning decision, not the release-candidate commit itself.

Routine build/upload should use the normal VS Code PlatformIO controls. Direct `pio` CLI commands are not required by the reference release workflow.

---

## 5. Build Artifact

After a successful normal `pico` build, the expected UF2 artifact is:

```text
.pio/build/pico/firmware.uf2
```

Do not publish an artifact from `pico_debug` as the normal release firmware.

For manual UF2 flashing, use the board's standard RP2040 BOOTSEL/UF2 procedure. When the RP2040 mass-storage boot device appears, copy the release UF2 onto it.

---

## 6. Staging Release Assets

After the candidate has been built and flashed successfully, run from the repository root:

```powershell
.\tools\Stage-Release.ps1
```

The script does **not** invoke PlatformIO or rebuild firmware. It stages the already-built `pico` UF2 and reads the canonical version from `src/firmware_version.h`.

For `1.0.0-rc1`, it creates files under `dist/` similar to:

```text
ELRS-HID-Bridge-v1.0.0-rc1.uf2
ELRS-HID-Bridge-v1.0.0-rc1.sha256.txt
RELEASE-MANIFEST.txt
```

`RELEASE-MANIFEST.txt` records the firmware version, source Git commit when available, source artifact path, staged filename, and SHA-256 hash.

`dist/` is release staging output, not source. Do not commit staged binaries unless the repository policy changes explicitly.

---

## 7. Release-Candidate Validation

Use `docs/Release-Checklist.md` as the authoritative release-candidate regression checklist.

At minimum, RC validation must cover:

- clean normal `pico` build,
- successful flash and USB enumeration,
- all primary axes and switch/button mappings,
- all four AUX HID axes,
- deterministic 500 ms failsafe across every HID control,
- automatic reconnect,
- LED/BOOT diagnostic behavior,
- Link Statistics not overriding RC timeout state,
- EdgeTX `Other Devices` discovery,
- Liftoff behavior/performance,
- wiring/setup documentation validation,
- staged UF2/hash/manifest verification.

If a release-blocking defect requires runtime code changes after RC validation begins, increment the prerelease identifier (`rc2`, `rc3`, and so on) and repeat the full checklist.

---

## 8. Git Tag and GitHub Release Procedure

### Release candidate

After `1.0.0-rc1` passes the release checklist:

1. Ensure all intended source/documentation changes are committed and synchronized.
2. Confirm the tested working tree/commit contains `FirmwareVersion::STRING = "1.0.0-rc1"`.
3. Stage the tested UF2 using `tools/Stage-Release.ps1`.
4. Confirm the staged SHA-256 matches the staged UF2.
5. Create tag:

   ```text
   v1.0.0-rc1
   ```

   The tag must point to the exact source commit used to build/test the published UF2.
6. Create a GitHub Release titled:

   ```text
   ELRS-HID-Bridge v1.0.0-rc1
   ```

7. Mark the GitHub Release as a **pre-release**.
8. Attach:

   ```text
   ELRS-HID-Bridge-v1.0.0-rc1.uf2
   ELRS-HID-Bridge-v1.0.0-rc1.sha256.txt
   RELEASE-MANIFEST.txt
   ```

9. Use `docs/Release-Notes-v1.0.0-rc1.md` (and the corresponding `CHANGELOG.md` entry) as the release description baseline.
10. Verify the GitHub-generated source archive includes `LICENSE`, `AUTHORS.md`, and `THIRD_PARTY_NOTICES.md` from the tagged source tree.

### Final v1.0.0

After the RC has proven stable and all release blockers are closed:

1. Change the canonical version to `1.0.0` with an empty prerelease label.
2. Update `FirmwareVersionSelfTest` and release documentation/changelog in the same commit.
3. Rebuild and repeat the full release checklist.
4. Stage a newly built `ELRS-HID-Bridge-v1.0.0.uf2` and checksum/manifest.
5. Tag the exact tested commit:

   ```text
   v1.0.0
   ```

6. Publish the GitHub Release as a normal release, not a pre-release.

Do not simply rename the RC binary to `v1.0.0`; final release artifacts must be rebuilt from the final tagged source tree.

---

## 9. Licensing and Attribution

Project code is licensed under **GPL-3.0-only**.

The tagged source tree must include:

```text
LICENSE
AUTHORS.md
THIRD_PARTY_NOTICES.md
```

The GitHub Release should identify the corresponding source tag/commit for every distributed firmware binary. If third-party dependencies change, update `THIRD_PARTY_NOTICES.md` before publishing the release.

Copyright (C) 2026 Tommy Mills.

---

## 10. Remaining v1.0 Gates

Release blocking:

- [x] GPL-3.0-only license and attribution files.
- [x] Canonical firmware version source and CRSF Firmware ID encoding.
- [x] Project-controlled USB descriptors validated on Windows.
- [x] v1.0 USB VID/PID policy fixed and `joy.cpl` limitation documented.
- [x] Release-critical framework/toolchain/library versions pinned.
- [x] Release artifact naming/staging procedure defined.
- [x] Git tag / GitHub Release procedure defined.
- [ ] Validate final wiring instructions from a clean-reader perspective.
- [ ] Validate final EdgeTX/ExpressLRS setup instructions from a clean-reader perspective.
- [ ] Clean-build `1.0.0-rc1` using the documented `pico` environment.
- [ ] Flash and complete `docs/Release-Checklist.md` on reference hardware.
- [ ] Stage and verify the tested RC UF2/hash/manifest.
- [ ] Publish/observe the RC as needed before final `1.0.0`.
- [ ] Transition to final `1.0.0`, rebuild, retest, tag, and publish.

---

## 11. Scope Freeze

Do not delay v1.0 for:

- full CRSF parameter configuration,
- persistent configuration,
- Bind/Wi-Fi receiver commands,
- desktop support software,
- additional receiver protocols,
- custom PCB work,
- application-specific integrations,
- a dedicated USB VID/PID solely to change the Windows `joy.cpl` label.

Any exception should be justified by a demonstrated release-blocking defect in the existing reference design.
