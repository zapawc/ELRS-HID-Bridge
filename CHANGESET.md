# ELRS-HID-Bridge — Post-v1.0 Checkpoint 3

## Intent

Refactor the proven CRSF parameter proof of concept into a small reusable
parameter seam **without changing user-visible behavior**.

Checkpoint 2 proved that EdgeTX can discover, read, edit, and write the
runtime LED Brightness CRSF FLOAT parameter on real hardware.

This checkpoint moves parameter-specific policy out of `main.cpp` before a
second setting is introduced.

## New component

`BridgeParameters` now owns:

- parameter IDs,
- parameter count,
- root-folder membership,
- parameter names,
- CRSF parameter types,
- ranges/defaults/steps,
- the ExpressLRS Lua r18 blank-unit compatibility behavior,
- parameter read-response construction,
- parameter write validation,
- updates to `BridgeConfiguration`,
- write acknowledgement construction.

`main.cpp` remains responsible for application-side effects and transport:

- receive the captured request from `CrsfDecoder`,
- ask `BridgeParameters` to process it,
- apply the returned LED-brightness side effect,
- transmit the already-built CRSF response.

This deliberately avoids a generic callback/template/persistence framework.

## User-visible behavior

There should be **no intentional behavior change** from validated Checkpoint 2.

EdgeTX should still show approximately:

`LED Brightness    10`

The parameter remains:

- CRSF `FLOAT`,
- range 0–100,
- precision 0,
- step 1,
- runtime-only,
- blank unit string for ExpressLRS Lua r18 compatibility.

The physical LED should still update immediately when the value changes.

Reboot should still restore the default value of 10.

## Device Info consistency

`BridgeIdentity::CRSF_PARAMETER_COUNT` now derives from:

`BridgeParameters::PARAMETER_COUNT`

This prevents Device Info metadata from drifting away from the actual parameter
component as additional parameters are introduced later.

## Files added

- `src/bridge_parameters.h`
- `src/bridge_parameters.cpp`

## Files changed

- `src/main.cpp`
- `src/bridge_identity.h`
- `src/crsf_parameter_self_test.cpp`

`src/crsf_parameter_self_test.h` is included as a complete replacement file for
checkpoint consistency but has no behavioral change.

## Self-test coverage

The CRSF parameter startup self-test now exercises the new abstraction directly:

- CRSF Parameter Read capture through decoder/dispatcher,
- root-folder response,
- LED Brightness FLOAT response,
- current runtime value reflected in reads,
- valid write updates `BridgeConfiguration`,
- valid write produces the expected acknowledgement,
- out-of-range write is rejected without modifying configuration,
- wrong-address read is rejected,
- parameter count remains one.

## Intentionally unchanged

- CRSF wire representation of the validated LED parameter
- blank unit workaround for ExpressLRS Lua r18
- HID mapping
- HID axis orientation
- receiver timeout
- failsafe policy
- Link Statistics behavior
- BOOT-button maintenance behavior
- USB descriptors
- persistence
- board support
- `pico_debug`
- firmware semantic version

## Build procedure

Use the established VS Code / PlatformIO workflow.

1. Overlay this ZIP at the repository root.
2. Build the normal `pico` environment.
3. Confirm VS Code **Problems** is clear.
4. Flash the normal build.
5. Do not use `pico_debug`.

## Hardware regression

### CRSF parameter

1. Open the ExpressLRS Lua script.
2. Open `Other Devices`.
3. Open `ELRS-HID-Bridge`.
4. Confirm it opens without a Lua error.
5. Confirm `LED Brightness` appears.
6. Confirm the initial value is 10.
7. Change it to several values such as 25, 50, and 100.
8. Confirm the physical LED changes immediately.
9. Set it to 0 and confirm only the LED goes dark; HID remains functional.
10. Return to a visible value.
11. Power-cycle the bridge and confirm brightness returns to 10.

### HID/failsafe smoke regression

1. Open `joy.cpl`.
2. Confirm all eight analog controls operate normally.
3. Confirm the existing button mappings remain correct.
4. Turn the transmitter off.
5. Confirm all analog controls neutralize and all buttons release.
6. Turn the transmitter back on.
7. Confirm automatic recovery.

## Success criteria

Checkpoint 3 succeeds if:

- the normal `pico` build is clean,
- startup self-tests pass,
- the EdgeTX parameter behaves exactly as in Checkpoint 2,
- the Lua r18 compatibility workaround remains effective,
- HID behavior is unchanged,
- failsafe behavior is unchanged.

## Suggested commit

`refactor: isolate CRSF bridge parameter policy`

## Next checkpoint

After validation and commit, add **one second real parameter** using
`BridgeParameters`.

The second parameter should be selected for low operational risk and clear
hardware/behavioral verification. Its implementation will test whether this
abstraction is genuinely useful before persistence is introduced.
