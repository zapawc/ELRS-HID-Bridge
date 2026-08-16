# v1.0 Release-Candidate Checklist

**Candidate:** `1.0.0-rc1`  
**Reference build environment:** `pico`  
**Purpose:** Final regression and packaging gate before publishing a v1.0 release candidate.

Use this checklist against the exact source commit that will be tagged. If runtime code, dependency pins, HID descriptors, CRSF protocol behavior, or release identity changes after testing begins, restart the checklist for the new candidate.

---

## 1. Source / Documentation Preflight

- [ ] Working tree contains the intended release-candidate source and documentation.
- [ ] `src/firmware_version.h` reports `1.0.0-rc1`.
- [ ] `FirmwareVersionSelfTest` expects `1.0.0-rc1` and CRSF ID `0x01000000`.
- [ ] `LICENSE`, `AUTHORS.md`, and `THIRD_PARTY_NOTICES.md` are present.
- [ ] `README.md`, `docs/Architecture.md`, `docs/Protocol.md`, `docs/Roadmap.md`, and `docs/Release.md` agree on current version/status.
- [ ] `platformio.ini` still contains the known-good pinned framework/toolchain/NeoPixel versions.
- [ ] No new v1.0 runtime feature was added outside the frozen scope.

---

## 2. Clean Build

Use the normal VS Code PlatformIO controls and select the `pico` environment.

- [ ] Normal `pico` build completes successfully.
- [ ] VS Code Problems contains no project build errors.
- [ ] Build uses the HID-only `pico` environment, not `pico_debug`.
- [ ] `.pio/build/pico/firmware.uf2` exists after the build.

Do not use the `pico_debug` artifact as the release binary.

---

## 3. Flash / Startup

- [ ] Flash the candidate to the reference Adafruit QT Py RP2040.
- [ ] Device boots normally.
- [ ] Startup self-tests pass (no fatal red state).
- [ ] With transmitter unavailable, normal waiting/link-loss LED behavior is as documented.
- [ ] With valid RC control restored, LED reaches solid normal green.

---

## 4. USB / Windows HID

- [ ] Windows enumerates the HID device normally.
- [ ] Bus-reported USB product identity remains `ELRS-HID-Bridge`.
- [ ] `joy.cpl` opens the controller successfully.
- [ ] If `joy.cpl` displays `Pico`, confirm this remains only the documented cosmetic naming limitation.
- [ ] No custom driver is required.

Reference v1.0 VID/PID remains:

```text
0x2E8A:0x000A
```

---

## 5. Primary Axes

In `joy.cpl` or equivalent HID test view:

- [ ] Roll / X responds through the expected range and direction.
- [ ] Pitch / Y responds through the expected range and inverted direction.
- [ ] Throttle / Slider 1 responds through the expected range and direction.
- [ ] Yaw / Slider 2 responds through the expected range and direction.

Expected primary orientation:

```text
Roll      normal
Pitch     inverted
Throttle  normal
Yaw       normal
```

---

## 6. AUX Analog Axes

Reference mapping:

```text
CH13 -> Z
CH14 -> Rx
CH15 -> Ry
CH16 -> Rz
```

- [ ] CH13/Z responds as expected for the configured ExpressLRS mode.
- [ ] CH14/Rx responds as expected for the configured ExpressLRS mode.
- [ ] CH15/Ry behavior matches the documented upstream limitation.
- [ ] CH16/Rz behavior matches the documented upstream limitation.
- [ ] No HID-side workaround has accidentally been introduced for CH15/CH16.

Reference ExpressLRS configuration:

```text
333 Hz Full
16ch Rate/2
```

With the tested ExpressLRS 3.3.1 receiver firmware, CH15/CH16 may remain high.

---

## 7. Switch / Button Mapping

Validate the documented reference EdgeTX layout:

- [ ] CH5 / SF -> Button 1.
- [ ] CH6 / SA middle/down -> Buttons 2/3.
- [ ] CH7 / SB middle/down -> Buttons 4/5.
- [ ] CH8 / SC middle/down -> Buttons 6/7.
- [ ] CH9 / SD middle/down -> Buttons 8/9.
- [ ] CH10 / SE middle/down -> Buttons 10/11.
- [ ] CH11 / SG middle/down -> Buttons 12/13.
- [ ] CH12 / SH -> Button 14.
- [ ] No unexpected/stuck HID buttons are present.

---

## 8. Failsafe / Recovery

Before transmitter-off testing, move at least one primary axis, CH13/CH14, and one switch away from their safe states.

Power off the transmitter.

- [ ] RC timeout occurs at approximately 500 ms after valid RC frames stop.
- [ ] Roll centers.
- [ ] Pitch centers.
- [ ] Yaw centers.
- [ ] Throttle moves to minimum.
- [ ] AUX Analog 1 centers.
- [ ] AUX Analog 2 centers.
- [ ] AUX Analog 3 centers.
- [ ] AUX Analog 4 centers.
- [ ] All HID buttons release.
- [ ] LED enters the documented purple receiver-loss state.
- [ ] Continued Link Statistics traffic does not restore healthy RC state.

Restore the transmitter.

- [ ] Valid live HID data resumes automatically without bridge reboot.
- [ ] LED returns to normal green.
- [ ] No stale button/axis state remains after recovery.

---

## 9. BOOT / Diagnostic UI

- [ ] Short BOOT press is acknowledged.
- [ ] Link Quality diagnostic appears when Link Statistics are available.
- [ ] Diagnostic-unavailable behavior is correct when link data is unavailable.
- [ ] ~2 s hold selects the Bind candidate display.
- [ ] ~5 s hold selects the Wi-Fi candidate display.
- [ ] Continued hold reaches Cancel behavior.
- [ ] Releasing Bind/Wi-Fi candidates does not execute unsupported receiver commands.
- [ ] Maintenance display never masks a fatal startup error.

---

## 10. Bidirectional CRSF Discovery

In the ExpressLRS Lua interface:

- [ ] Open **Other Devices**.
- [ ] `ELRS-HID-Bridge` appears.
- [ ] Device discovery does not disrupt normal RC/HID operation.
- [ ] Identity remains parameter-count zero.
- [ ] No unexpected CRSF parameter/configuration UI has appeared.

---

## 11. Simulator Regression

Using Liftoff with the transmitter fully wireless:

- [ ] Device remains recognized and usable.
- [ ] Roll/pitch/yaw/throttle feel unchanged from the known-good build.
- [ ] No noticeable latency, stutter, or control degradation is introduced.
- [ ] Arm/mode/reset controls mapped to HID buttons behave normally.
- [ ] Extended session does not reveal disconnect/reconnect instability.

---

## 12. Wiring / Setup Documentation Validation

Validate the README as though assembling/configuring the project from scratch.

Reference wiring:

```text
RP2 5V  -> QT Py 5V
RP2 GND -> QT Py GND
RP2 TX  -> QT Py RX
RP2 RX  -> QT Py TX
```

- [ ] Wiring instructions match the physical reference build.
- [ ] RX/TX direction labels are unambiguous.
- [ ] 420000-baud CRSF requirement is documented.
- [ ] EdgeTX channel layout matches the tested model.
- [ ] ExpressLRS `333 Hz Full / 16ch Rate/2` setup is documented.
- [ ] CH15/CH16 limitation is documented.
- [ ] BOOT maintenance limitations are documented.
- [ ] Windows `joy.cpl` `Pico` naming quirk is documented as cosmetic.

---

## 13. Release Asset Staging

After the exact candidate has passed the hardware regression:

```powershell
.\tools\Stage-Release.ps1
```

- [ ] `dist/ELRS-HID-Bridge-v1.0.0-rc1.uf2` is created.
- [ ] `dist/ELRS-HID-Bridge-v1.0.0-rc1.sha256.txt` is created.
- [ ] `dist/RELEASE-MANIFEST.txt` is created.
- [ ] Manifest version is `1.0.0-rc1`.
- [ ] Manifest Git commit matches the exact tested source commit (when Git is available).
- [ ] SHA-256 file matches the staged UF2.
- [ ] Staged UF2 is byte-for-byte copied from `.pio/build/pico/firmware.uf2`.

Optional but recommended final proof:

- [ ] Flash the staged/renamed UF2 itself using the normal BOOTSEL/UF2 process.
- [ ] Confirm startup and a short HID/CRSF smoke test.

---

## 14. RC Publication Gate

Only after every release-blocking item above is complete:

- [ ] Create tag `v1.0.0-rc1` pointing to the exact tested source commit.
- [ ] Create GitHub Release `ELRS-HID-Bridge v1.0.0-rc1`.
- [ ] Mark it as a pre-release.
- [ ] Attach the staged UF2, SHA-256 file, and release manifest.
- [ ] Use `CHANGELOG.md` / RC release notes for the release description.
- [ ] Verify source archive contains GPL license/attribution files.

---

## 15. Final v1.0.0 Gate

Do not convert RC1 directly into final by renaming files.

When RC validation is considered complete:

- [ ] Change canonical firmware version to `1.0.0`.
- [ ] Clear the prerelease label.
- [ ] Update the version self-test.
- [ ] Update `CHANGELOG.md` and release documentation.
- [ ] Rebuild from the final source tree.
- [ ] Repeat this full checklist.
- [ ] Stage a new `ELRS-HID-Bridge-v1.0.0.uf2`.
- [ ] Tag exact tested commit `v1.0.0`.
- [ ] Publish the normal GitHub release.
