# ELRS-HID-Bridge Release Guide

**Status:** final v1.0 validation  
**Current firmware version:** `1.0.0`  
**Updated:** August 2026

## 1. Purpose

This document defines the release procedure for the reference ELRS-HID-Bridge firmware.

The runtime CRSF-to-HID feature set is frozen for the v1.0 cycle. Final-release work should improve reproducibility, packaging, documentation, and validation. Do not add new CRSF/HID features unless a demonstrated release-blocking defect requires a runtime change.

The detailed validation sequence lives in `docs/Release-Checklist.md`.

---
## 2. Canonical Firmware Version

Firmware version information lives in:

```text
src/firmware_version.h
```

Current final-release identity:

```text
1.0.0
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

For v1.0.0:

```text
1.0.0 -> 0x01000000
```

Prerelease suffixes are deliberately not encoded in the numeric CRSF field. Therefore `1.0.0-rc1` and final `1.0.0` intentionally share CRSF Firmware ID `0x01000000`.

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

All published firmware binaries must come from the normal `pico` environment.

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

That commit is historical evidence for the pinning decision, not the final release commit itself.

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

After the final source has been built and flashed successfully, run from the repository root:

```powershell
.\tools\Stage-Release.ps1
```

The script does **not** invoke PlatformIO or rebuild firmware. It stages the already-built `pico` UF2 and reads the canonical version from `src/firmware_version.h`.

For `1.0.0`, it creates files under `dist/` similar to:

```text
ELRS-HID-Bridge-v1.0.0.uf2
ELRS-HID-Bridge-v1.0.0.sha256.txt
RELEASE-MANIFEST.txt
```

`RELEASE-MANIFEST.txt` records the firmware version, source Git commit when available, source artifact path, staged filename, and SHA-256 hash.

`dist/` is release staging output, not source. Do not commit staged binaries unless the repository policy changes explicitly.

---
## 7. Final v1.0 Validation

Use `docs/Release-Checklist.md` as the authoritative final regression checklist.

At minimum, final validation must cover:
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

The final `1.0.0` artifact must be rebuilt from the final source tree. Do not rename or republish the RC1 binary.

If a release-blocking runtime defect is discovered during final validation, do not publish `1.0.0`. Correct the defect, return to an appropriate release-candidate identity if needed, and repeat the full validation cycle before promoting final again.

---
## 8. Git Tag and GitHub Release Procedure

### Historical release candidate

The published `v1.0.0-rc1` release remains the validated prerelease baseline. Its tag and artifacts should remain unchanged as historical release evidence.

### Final v1.0.0

After the final `1.0.0` source passes the complete release checklist:

1. Ensure all intended source/documentation changes are committed and synchronized.
2. Confirm the tested working tree/commit contains `FirmwareVersion::STRING = "1.0.0"` and an empty prerelease label.
3. Stage the newly built UF2 using `tools/Stage-Release.ps1`.
4. Confirm the staged SHA-256 matches the staged UF2.
5. Confirm `RELEASE-MANIFEST.txt` records version `1.0.0` and the exact tested Git commit.
6. Create tag:

   ```text
   v1.0.0
   ```

   The tag must point to the exact source commit used to build/test the published UF2.
7. Create a GitHub Release titled:

   ```text
   ELRS-HID-Bridge v1.0.0
   ```

8. Publish it as a normal release, **not** a pre-release.
9. Attach:

   ```text
   ELRS-HID-Bridge-v1.0.0.uf2
   ELRS-HID-Bridge-v1.0.0.sha256.txt
   RELEASE-MANIFEST.txt
   ```

10. Use `docs/Release-Notes-v1.0.0.md` and the corresponding `CHANGELOG.md` entry as the release-description baseline.
11. Verify the GitHub-generated source archive includes `LICENSE`, `AUTHORS.md`, and `THIRD_PARTY_NOTICES.md` from the tagged source tree.

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
- [x] RC1 built, tested, staged, tagged, and published.
- [x] Canonical source identity advanced to final `1.0.0` for final validation.
- [ ] Validate final wiring instructions from a clean-reader perspective against the final source tree.
- [ ] Validate final EdgeTX/ExpressLRS setup instructions from a clean-reader perspective against the final source tree.
- [ ] Clean-build `1.0.0` using the documented `pico` environment.
- [ ] Flash and complete `docs/Release-Checklist.md` on reference hardware.
- [ ] Stage and verify the newly built final UF2/hash/manifest.
- [ ] Tag the exact tested final commit as `v1.0.0`.
- [ ] Publish the normal GitHub `v1.0.0` release.

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
