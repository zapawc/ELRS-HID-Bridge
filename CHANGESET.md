# ELRS-HID-Bridge — Post-v1.0 Checkpoint 4

## Intent

Add one second real CRSF/EdgeTX configuration parameter using the parameter
architecture validated in Checkpoint 3.

New parameter:

**Pitch Inversion**

Options:

- `Normal`
- `Inverted`

The current validated v1.0 behavior remains the default:

**Inverted**

This checkpoint remains runtime-only. Persistence is intentionally deferred.

## Why this parameter

LED Brightness proved:

CRSF FLOAT → BridgeConfiguration → hardware presentation

Pitch Inversion now proves a different path:

CRSF TEXT_SELECTION → BridgeConfiguration → HID mapping

That exercises the parameter abstraction with a second CRSF data type and a
different application subsystem without broadening scope.

## CRSF representation

Parameter list:

- Parameter `0` — `ROOT`
- Parameter `1` — `LED Brightness`
- Parameter `2` — `Pitch Inversion`

`Pitch Inversion` uses standard CRSF `TEXT_SELECTION` (`0x09`).

Wire definition:

- Name: `Pitch Inversion`
- Options: `Normal;Inverted`
- Value:
  - `0` = Normal
  - `1` = Inverted
- Min: `0`
- Max: `1`
- Default: `1`
- Unit: empty string

TEXT_SELECTION writes use the standard one-byte selection index.

Accepted writes are acknowledged with `0x2D` containing:

- parameter number
- accepted one-byte selection index

## Runtime behavior

`BridgeConfiguration::pitch.inverted` is updated immediately.

`ChannelMapper` already references `BridgeConfiguration`, so no mapper rebuild
or additional state synchronization is required. The changed inversion setting
is used on the next decoded RC channel frame.

No other axis mapping changes.

## Default behavior

The bridge still boots with:

`Pitch Inversion = Inverted`

This preserves the previously hardware-validated HID orientation:

- Roll — normal
- Pitch — inverted
- Throttle — normal
- Yaw — normal

Merely flashing this checkpoint should therefore not change normal joystick
behavior.

## Files changed

- `src/crsf_device.h`
- `src/crsf_device.cpp`
- `src/bridge_parameters.h`
- `src/bridge_parameters.cpp`
- `src/main.cpp`
- `src/crsf_parameter_self_test.cpp`

## CRSF device additions

`CrsfDevice` gains protocol-level helpers for:

- TEXT_SELECTION parameter-entry construction
- one-byte TEXT_SELECTION write acknowledgement

Bridge-specific names/options/defaults remain in `BridgeParameters`.

## Parameter architecture proof

`BridgeParameters::PARAMETER_COUNT` increases from 1 to 2.

Because Device Info already derives its count from `BridgeParameters`, no
separate Device Info count edit is required.

The root folder now advertises both parameters.

## Self-test coverage

Startup CRSF parameter tests now cover:

- parameter count = 2
- root folder contains parameters 1 and 2
- LED Brightness still produces a FLOAT entry
- Pitch Inversion produces a TEXT_SELECTION entry
- option text is `Normal;Inverted`
- current/default selection is Inverted
- valid write to Normal updates configuration and produces acknowledgement
- valid write back to Inverted updates configuration and produces acknowledgement
- invalid selection index is rejected without modifying configuration
- wrong-address requests remain rejected

## Intentionally unchanged

- LED Brightness behavior
- ExpressLRS Lua r18 blank-unit workaround for LED Brightness
- channel assignments
- all non-pitch axis orientation
- switch/button mappings
- receiver timeout
- failsafe policy
- Link Statistics behavior
- BOOT-button behavior
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

## Hardware test procedure

### Baseline after flash

1. Power the bridge normally.
2. Link the transmitter.
3. Confirm the normal green operational indication.
4. Open `joy.cpl`.
5. Confirm the existing validated pitch direction is unchanged after flash.
6. Confirm all other axes and buttons still behave normally.

### Parameter discovery

1. Open the ExpressLRS Lua script.
2. Open `Other Devices`.
3. Open `ELRS-HID-Bridge`.
4. Confirm both parameters appear:
   - `LED Brightness`
   - `Pitch Inversion`
5. Confirm `Pitch Inversion` initially displays `Inverted`.

### Pitch Inversion — Normal

1. Leave `joy.cpl` visible.
2. Change `Pitch Inversion` from `Inverted` to `Normal`.
3. Move the pitch stick.
4. Confirm the HID Y-axis direction reverses immediately.
5. Confirm Roll, Throttle, Yaw, auxiliary axes, and buttons are unchanged.

### Pitch Inversion — Inverted

1. Change `Pitch Inversion` back to `Inverted`.
2. Move the pitch stick.
3. Confirm the previously validated Y-axis direction is restored.

### Existing LED parameter regression

1. Change LED Brightness to a visibly different value.
2. Confirm the LED changes immediately.
3. Confirm no Lua formatting error returns.

### Volatile configuration

1. Set `Pitch Inversion` to `Normal`.
2. Power-cycle the bridge.
3. Reopen the bridge device page.
4. Confirm `Pitch Inversion` returns to `Inverted`.
5. Confirm `joy.cpl` again shows the validated inverted pitch orientation.

This confirms persistence has not been introduced.

### Failsafe regression

1. Leave `joy.cpl` open with the transmitter linked.
2. Turn the transmitter off.
3. Confirm all eight analog controls neutralize and all buttons release.
4. Turn the transmitter back on.
5. Confirm automatic recovery.
6. Confirm the currently selected runtime pitch inversion is honored after
   recovery.

## Success criteria

Checkpoint 4 succeeds when:

- the normal `pico` build is clean,
- startup self-tests pass,
- both EdgeTX parameters are visible,
- Pitch Inversion defaults to Inverted,
- selecting Normal reverses only pitch,
- selecting Inverted restores the validated orientation,
- LED Brightness still works,
- reboot restores Pitch Inversion to Inverted,
- HID/failsafe behavior remains otherwise unchanged.

## Suggested commit

`feat: add runtime pitch inversion parameter`

## Next checkpoint

If this passes, the parameter abstraction has been validated with:

- two real settings,
- two CRSF parameter types,
- two different application subsystems.

At that point, persistent configuration becomes justified. The next design
checkpoint should define the persistence schema, validation, corruption
fallback, versioning, and reset-to-default behavior before adding more user
settings.
