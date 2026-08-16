# ELRS-HID-Bridge Failsafe Hardening Checkpoint

## Purpose

This checkpoint moves the project into v1.0 hardening by making receiver-loss behavior deterministic across the complete HID report.

It does **not** expand CRSF device functionality. The validated Device Ping -> Device Info discovery path remains unchanged.

## Replace

Replace these files with the supplied complete versions:

```text
src/main.cpp
src/failsafe_policy.h
src/failsafe_policy.cpp
README.md
docs/Architecture.md
docs/Roadmap.md
docs/Protocol.md
```

## Add

Add these new files:

```text
src/failsafe_policy_self_test.h
src/failsafe_policy_self_test.cpp
```

## Behavioral change

After 500 ms without a valid RC Channels frame, `FailsafePolicy` now explicitly produces:

```text
Roll         center
Pitch        center
Yaw          center
Throttle     minimum
AUX Analog 1 center
AUX Analog 2 center
AUX Analog 3 center
AUX Analog 4 center
Buttons      released
```

No HID control intentionally retains its previous value during failsafe.

When valid RC frames resume, normal mapped values resume automatically as before.

## Startup self-test change

A new `FailsafePolicySelfTest` runs with the existing startup tests.

It verifies:

- Roll centers.
- Pitch centers.
- Yaw centers.
- Throttle goes to minimum.
- AUX Analog 1 centers.
- AUX Analog 2 centers.
- AUX Analog 3 centers.
- AUX Analog 4 centers.
- All 32 button bits clear.
- Reapplying failsafe is idempotent.

A failure follows the existing startup-test failure path and presents the fatal/red state.

## Documentation synchronization

This checkpoint also records the successfully completed live CRSF discovery experiment:

- `0xC8` Device Info routing works on the reference RP2/Ranger/EdgeTX path.
- EdgeTX discovers `ELRS-HID-Bridge` under **Other Devices**.
- 333 Hz Full RC-to-HID operation remains normal.
- Liftoff performance remains normal.
- Further CRSF feature expansion is frozen for the v1.0 cycle.

The prior Architecture text that still described live Device Info TX as not enabled has been corrected.

## Validation sequence

Use the normal VS Code / PlatformIO workflow.

### 1. Build

- Select the normal `pico` environment.
- Build successfully.
- Confirm VS Code reports no Problems.
- Do not use direct `pio` CLI commands unless troubleshooting requires them.

### 2. Flash / startup

- Flash the QT Py RP2040 normally.
- Confirm the startup self-tests do not produce the red fatal state.
- Confirm the normal LED state sequence remains expected.

### 3. Normal-control regression

With the transmitter connected:

- Roll correct.
- Pitch correct.
- Throttle correct.
- Yaw correct.
- AUX controls behave as they did before this checkpoint.
- Switch/button mapping remains correct.
- Normal state remains green.

### 4. Failsafe validation in `joy.cpl`

Before turning the transmitter off, deliberately place available analog controls away from center and activate one or more switches.

Turn the transmitter off.

After approximately 500 ms, verify:

```text
X / Roll       center
Y / Pitch      center
Slider 1       minimum
Slider 2 / Yaw center
Z / CH13       center
Rx / CH14      center
Ry / CH15      center
Rz / CH16      center
Buttons        all released
LED            purple
```

The CH15/CH16 live-channel limitation is useful here: if those axes are high during normal operation, they should visibly move to center when bridge-side failsafe is applied.

### 5. Reconnect

Turn the transmitter back on and verify:

- live controls resume automatically,
- AUX axes resume their live CRSF values,
- buttons respond normally,
- LED returns to green,
- no bridge reboot is required.

### 6. Simulator regression

Run Liftoff and confirm normal control/performance remains unchanged.

### 7. Bidirectional CRSF regression

Open ExpressLRS Lua -> **Other Devices** and confirm `ELRS-HID-Bridge` is still discoverable.

## Expected result

If all checks pass, the AUX analog failsafe item in the v1.0 Roadmap is complete and the next work should remain release hardening rather than feature expansion.

## Suggested commit message

```text
Make HID failsafe deterministic across all axes
```
