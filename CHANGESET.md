# Throttle T9 — Permanent Throttle Inversion Candidate

## Scope

Adds a real configurable Throttle inversion parameter after the root-cause
investigation established that the original ExpressLRS Lua menu hang was caused
by parameter-name length, not throttle mapping.

Hardware diagnostics established:

- 16-character parameter name: menu enumerates normally
- 17-character parameter name: menu enumeration hangs
- 18-character parameter names: menu enumeration hangs
- Forced internal `configuration.throttle.inverted = true`: HID Slider 1
  reverses correctly

The permanent throttle label is therefore:

`Throttle Invert`

15 characters.

## Parameter order

1. LED Brightness
2. Pitch Inversion
3. Throttle Invert
4. Roll Inversion
5. Yaw Inversion
6. Aux 1 Inversion
7. Aux 2 Inversion
8. Aux 3 Inversion
9. Aux 4 Inversion
10. Diagnostics
11. Failsafe Count
12. Restore Defaults

`BridgeIdentity` already derives the CRSF Device Info count from
`BridgeParameters::PARAMETER_COUNT`, so no identity-file edit is required.

## Modified files

- `src/bridge_parameters.h`
- `src/bridge_parameters.cpp`
- `src/crsf_parameter_self_test.cpp`

## Behavior

Throttle uses the existing generic inversion path:

- Read: `configuration.throttle`
- Default: `BridgeConfiguration::defaults().throttle`
- Write: `configuration.throttle.inverted`
- Change class: `BridgeParameterChange::AxisInversion`
- Persistence: existing `requiresPersistence` flow and existing throttle bit in
  `BridgeConfigurationRecord`
- Runtime mapping: unchanged; already proven by T2
- Failsafe policy: unchanged; throttle must still go to safe minimum on RC loss

## Self-test changes

Startup parameter tests now verify:

- registry count is 12
- root child order includes Throttle
- Throttle is a valid text-selection entry
- the exact safe label `Throttle Invert` is serialized
- a throttle inversion write changes `configuration.throttle.inverted`
- an invalid throttle selection is rejected
- Restore Defaults restores throttle to its canonical default
- existing Diagnostics / Failsafe Count behavior remains covered

## Hardware validation checklist

1. Start from clean committed T-Maint state.
2. Overlay this ZIP.
3. Build normal `pico`.
4. Confirm VS Code Problems is clear.
5. Upload.
6. Confirm startup reaches normal LED state (no fatal red).
7. Open ExpressLRS -> Other Devices -> ELRS-HID-Bridge.
8. Confirm menu loads normally and order is:
   LED, Pitch, Throttle, Roll, Yaw, Aux 1-4, Diagnostics, Restore Defaults.
9. Verify Throttle Invert defaults to Normal.
10. In Windows `joy.cpl`, move throttle through full range.
11. Set Throttle Invert -> Inverted.
12. Verify Slider 1 reverses immediately.
13. Close/reopen the ELRS menu and confirm the setting reads Inverted.
14. Power-cycle the bridge and confirm the setting persists.
15. Set Throttle Invert -> Normal and confirm Slider 1 returns to canonical
    direction.
16. Set Throttle Invert -> Inverted, then use Restore Defaults.
17. Confirm Throttle returns to Normal.
18. Turn the transmitter off and confirm failsafe still sends safe minimum
    throttle, regardless of the configured live-input inversion.
19. Turn the transmitter back on and confirm live configured behavior resumes.
20. Smoke-test the other inversion parameters and BOOT diagnostic/Bind behavior.

## Commit status

This package is intended to be commit-worthy **only after** the hardware
validation above passes.
