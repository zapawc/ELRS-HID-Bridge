# ELRS-HID-Bridge — Post-v1.0 Checkpoint 2 (Lua r18 Compatibility Revision)

## Intent

Prove one standard CRSF/EdgeTX configuration parameter end-to-end:

**LED Brightness — 0 to 100**

This revision preserves the CRSF `FLOAT` implementation from the original
Checkpoint 2 but removes the literal `%` unit string because the uploaded
ExpressLRS `elrs.lua` r18 script constructs a Lua `string.format()` pattern by
concatenating the unit directly after the numeric conversion. A literal `%`
therefore produces the observed script error:

`bad argument #3 to 'format' (no value)`

This is a Lua r18 compatibility workaround, not a CRSF protocol rollback.

## Protocol behavior

- `0x29` Device Info reports one normal parameter.
- Parameter `0` is the standard `ROOT` folder.
- Parameter `1` is `LED Brightness`.
- `LED Brightness` remains CRSF `FLOAT` (`0x08`).
- Range is `0` through `100`.
- Decimal point is `0`.
- Step size is `1`.
- Unit string is intentionally empty.
- Parameter reads use `0x2C`.
- Parameter entries are returned in `0x2B`.
- Accepted FLOAT writes are acknowledged with `0x2D`.
- All entries fit in one CRSF frame, so only chunk `0` is required.

## Why the unit is blank

The uploaded Lua r18 script constructs FLOAT display formatting in the form:

`"%." .. precision .. "f" .. unit`

With `%` as the unit, the resulting format string is effectively:

`%.0f%`

The trailing `%` is interpreted as the beginning of another Lua formatting
directive and causes the script exception.

Leaving the unit empty retains the standards-based FLOAT parameter while
avoiding the Lua r18 display bug. Expected presentation is approximately:

`LED Brightness    10`

rather than:

`LED Brightness    10%`

## Runtime behavior

Default LED brightness is 10.

The previous NeoPixel brightness was 24/255, approximately 9.4%, so the new
default intentionally remains visually close to the v1.0.0 behavior.

A valid EdgeTX write:

1. is decoded from the standard four-byte CRSF FLOAT representation,
2. is range checked,
3. updates `BridgeConfiguration::ledBrightnessPercent`,
4. is applied immediately to the QT Py NeoPixel,
5. is acknowledged to EdgeTX.

The value is **not persisted**. Rebooting restores the default of 10.

## Files changed

- `src/bridge_configuration.h`
- `src/bridge_configuration.cpp`
- `src/status_led.h`
- `src/status_led.cpp`
- `src/crsf_protocol.h`
- `src/crsf_device.h`
- `src/crsf_device.cpp`
- `src/crsf_dispatcher.h`
- `src/crsf_dispatcher.cpp`
- `src/crsf_decoder.h`
- `src/crsf_decoder.cpp`
- `src/bridge_identity.h`
- `src/main.cpp`

## Files added

- `src/crsf_parameter_self_test.h`
- `src/crsf_parameter_self_test.cpp`

## Compatibility revision versus the first Checkpoint 2 ZIP

Only the FLOAT unit presentation is intentionally changed:

- Previous unit: `%`
- Revised unit: empty string

The FLOAT type, numeric range, step, default, read/write behavior, and bridge
runtime behavior remain unchanged.

## Intentionally unchanged

- HID mapping
- HID axis orientation
- receiver timeout
- failsafe policy
- Link Statistics behavior
- BOOT-button maintenance behavior
- USB descriptors
- persistent storage
- board support
- `pico_debug`
- firmware semantic version

## Build procedure

Use the normal VS Code / PlatformIO workflow.

1. Overlay this ZIP at the repository root.
2. Build the normal `pico` environment.
3. Check VS Code **Problems**.
4. Do not use `pico_debug` for this checkpoint.

## Hardware test procedure

### Parameter discovery

1. Flash the normal `pico` build.
2. Link the transmitter/receiver normally.
3. Open the ExpressLRS script on EdgeTX.
4. Open `Other Devices`.
5. Open `ELRS-HID-Bridge`.
6. Confirm the previous Lua syntax error no longer appears.
7. Confirm `LED Brightness` appears with a value near 10.

### Parameter write

Test these values:

- 0
- 10
- 25
- 50
- 100

For each value:

1. confirm EdgeTX accepts the change,
2. confirm the physical LED changes immediately,
3. confirm the CRSF device page remains responsive,
4. confirm RC/HID behavior continues normally.

At 0 the LED being dark is expected. HID must remain fully operational.

### Volatile configuration

1. Set a non-default value such as 50.
2. Power-cycle the bridge.
3. Confirm LED brightness returns to 10.

### HID / failsafe regression

1. Confirm all eight analog controls and existing buttons in `joy.cpl`.
2. Turn the transmitter off.
3. Confirm all eight analog controls neutralize and all buttons release.
4. Turn the transmitter back on.
5. Confirm automatic recovery.

## Expected checkpoint result

This checkpoint succeeds when:

- opening `ELRS-HID-Bridge` no longer throws the Lua r18 formatting error,
- `LED Brightness` is visible,
- EdgeTX can change it from 0–100,
- LED brightness changes immediately,
- reboot restores 10,
- HID behavior remains unchanged,
- failsafe behavior remains unchanged,
- startup self-tests pass.

## Troubleshooting boundary

If the Lua error disappears but the field is missing or cannot be edited, stop
and capture the exact behavior before making more changes:

- Does `ELRS-HID-Bridge` open normally?
- Does `LED Brightness` appear?
- What value is shown?
- Can editing mode be entered?
- Does changing the value affect the physical LED?
- Does the displayed value refresh after editing?

## Suggested commit

If validation passes:

`feat: add runtime CRSF LED brightness parameter`

## Next checkpoint

If hardware validation succeeds, do not immediately add persistence. First
review the working parameter path and decide whether a small reusable parameter
definition layer is justified before adding more settings.
