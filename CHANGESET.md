# ELRS-HID-Bridge — Post-v1.0 Checkpoint 7

## Intent

Complete inversion configuration for every analog HID control before starting
the bridge-health telemetry workstream.

This checkpoint adds runtime + persistent inversion controls for:

- Roll
- Throttle
- Yaw
- Aux Analog 1
- Aux Analog 2
- Aux Analog 3
- Aux Analog 4

Pitch Inversion already exists and remains unchanged.

Buttons/switches are intentionally out of scope.

## Resulting analog controls

All eight analog HID controls can now be independently configured:

- Roll → HID X
- Pitch → HID Y
- Throttle → HID Slider 1
- Yaw → HID Slider 2
- Aux Analog 1 → HID Z
- Aux Analog 2 → HID Rx
- Aux Analog 3 → HID Ry
- Aux Analog 4 → HID Rz

Every inversion setting uses CRSF `TEXT_SELECTION`:

- `Normal`
- `Inverted`

## Parameter IDs

Existing IDs are preserved:

- 1 — LED Brightness
- 2 — Pitch Inversion
- 3 — Restore Defaults

New settings are appended:

- 4 — Roll Inversion
- 5 — Throttle Inversion
- 6 — Yaw Inversion
- 7 — Aux 1 Inversion
- 8 — Aux 2 Inversion
- 9 — Aux 3 Inversion
- 10 — Aux 4 Inversion

The parameter count is now 10.

The existing Restore Defaults ID is deliberately **not** moved merely to make
the menu order prettier.

## Defaults

The known-good v1.0 orientation remains authoritative:

- Roll — Normal
- Pitch — Inverted
- Throttle — Normal
- Yaw — Normal
- Aux 1 — Normal
- Aux 2 — Normal
- Aux 3 — Normal
- Aux 4 — Normal

Flashing this checkpoint should therefore not change any HID direction by
itself.

## Persistence schema v2

This checkpoint is the first legitimate reason to advance the stored
configuration schema.

Record size remains 16 bytes.

Schema v2 uses one previously reserved byte for an inversion bitmask:

- bit 0 — Roll
- bit 1 — Throttle
- bit 2 — Yaw
- bit 3 — Aux 1
- bit 4 — Aux 2
- bit 5 — Aux 3
- bit 6 — Aux 4
- bit 7 — reserved

Pitch remains in its existing byte so schema-v1 migration stays simple.

## Schema-v1 compatibility

Existing Checkpoint 5/6 records are schema v1.

A valid v1 record is still accepted.

On migration:

- saved LED Brightness is retained,
- saved Pitch Inversion is retained,
- newly introduced inversion fields use current known-good defaults.

The record is not rewritten merely because the firmware booted.

The next successful persisted setting change writes a schema-v2 record.

This avoids unnecessary flash writes while preserving prior user settings.

## Restore Defaults

Restore Defaults now resets **all eight** inversion settings to the known-good
defaults and persists them in schema v2.

The existing confirmation workflow is unchanged.

## Files changed

- `src/bridge_parameters.h`
- `src/bridge_parameters.cpp`
- `src/crsf_parameter_self_test.cpp`
- `src/bridge_configuration_record.h`
- `src/bridge_configuration_record.cpp`
- `src/bridge_configuration_record_self_test.cpp`
- `src/main.cpp`

## Self-test coverage

CRSF parameter tests now verify:

- parameter count = 10,
- root folder advertises all settings,
- all eight inversion controls build as TEXT_SELECTION,
- each new inversion write updates the correct BridgeConfiguration mapping,
- invalid inversion selection is rejected,
- Restore Defaults resets the expanded inversion configuration.

Persistent-record tests now verify:

- schema-v2 round trip for all inversion fields,
- a real valid schema-v1 record migrates successfully,
- v1 LED/Pitch values are preserved,
- new fields take defaults during v1 migration,
- unsupported schema rejection,
- corruption rejection,
- invalid value rejection,
- wrong-size rejection.

## Intentionally unchanged

- channel assignments
- HID descriptor
- button/switch behavior
- LED Brightness behavior
- ExpressLRS Lua r18 blank-unit workaround
- Restore Defaults confirmation behavior
- EEPROM transport
- receiver timeout
- failsafe policy
- Link Statistics behavior
- BOOT-button behavior
- telemetry
- board portability
- `pico_debug`
- firmware semantic version

## Build procedure

Use the normal VS Code / PlatformIO workflow.

1. Overlay this ZIP at repository root.
2. Build the normal `pico` environment.
3. Confirm VS Code Problems is clear.
4. Flash the normal build.
5. Do not use `pico_debug`.

## Hardware test procedure

### 1. Upgrade / schema-v1 migration

Before flashing, note your currently persisted values for:

- LED Brightness
- Pitch Inversion

After flashing Checkpoint 7:

1. Open ELRS-HID-Bridge.
2. Confirm the prior LED value is retained.
3. Confirm the prior Pitch Inversion value is retained.
4. Confirm all newly introduced inversion controls initially show their
   known-good defaults.

This validates real schema-v1 migration.

### 2. Parameter discovery

Confirm these inversion fields appear:

- Pitch Inversion
- Roll Inversion
- Throttle Inversion
- Yaw Inversion
- Aux 1 Inversion
- Aux 2 Inversion
- Aux 3 Inversion
- Aux 4 Inversion

### 3. Primary controls

With `joy.cpl` visible, change one setting at a time.

#### Roll
1. Change Roll Inversion to Inverted.
2. Confirm only X-axis direction reverses.
3. Return it to Normal.

#### Pitch
1. Confirm the existing Pitch Inversion control still works.
2. Return it to the desired test state.

#### Throttle
1. Change Throttle Inversion to Inverted.
2. Confirm only Slider 1 direction reverses.
3. Return it to Normal.

#### Yaw
1. Change Yaw Inversion to Inverted.
2. Confirm only Slider 2 direction reverses.
3. Return it to Normal.

### 4. Auxiliary analog controls

For each Aux 1–4 inversion parameter:

1. identify its corresponding HID axis in `joy.cpl`,
2. change Normal → Inverted,
3. confirm only that axis reverses,
4. restore it to Normal before moving to the next control.

Expected HID mapping:

- Aux 1 → Z
- Aux 2 → Rx
- Aux 3 → Ry
- Aux 4 → Rz

### 5. Persistence

Set a distinctive mixed configuration, for example:

- LED Brightness = 40
- Pitch = Normal
- Roll = Inverted
- Throttle = Inverted
- Yaw = Normal
- Aux 1 = Inverted
- Aux 2 = Normal
- Aux 3 = Inverted
- Aux 4 = Normal

Fully remove USB power, reconnect, and confirm every value survives.

This also causes the prior schema-v1 record to be rewritten as schema v2.

### 6. Restore Defaults regression

With several inversions still non-default:

1. run Restore Defaults,
2. confirm the transmitter asks for confirmation,
3. confirm the action,
4. verify:
   - LED Brightness = 10,
   - Roll = Normal,
   - Pitch = Inverted,
   - Throttle = Normal,
   - Yaw = Normal,
   - Aux 1–4 = Normal.
5. power-cycle and verify the restored defaults remain persisted.

### 7. HID/failsafe regression

1. Verify all eight analog controls.
2. Verify existing buttons remain unchanged.
3. Turn the transmitter off.
4. Confirm all eight analog controls neutralize and all buttons release.
5. Turn the transmitter back on.
6. Confirm automatic recovery.
7. Confirm the persisted inversion choices are still honored.

## Success criteria

Checkpoint 7 succeeds when:

- `pico` builds cleanly,
- startup self-tests pass,
- existing schema-v1 LED/Pitch values survive the upgrade,
- all eight analog inversion settings are visible,
- each setting reverses only its intended HID control,
- mixed inversion settings persist across power cycle,
- Restore Defaults resets and persists all analog directions,
- HID/failsafe behavior otherwise remains unchanged.

## Suggested commit

`feat: add persistent analog inversion controls`

## Next checkpoint

After this passes, analog mapping inversion is complete.

The recommended next workstream is the previously deferred:

**Minimal Bridge Health Telemetry**

Buttons/switch inversion is intentionally not part of the roadmap unless a
specific switch-polarity use case later justifies it.
