# ELRS-HID-Bridge — Checkpoint 8 Isolation: No Throttle Inversion

## Purpose

This is an isolation build to test whether the long ExpressLRS parameter-loading
pause is specifically associated with the Throttle Inversion parameter.

Throttle Inversion is removed from the CRSF device menu and parameter policy.
The underlying throttle mapping remains at its normal BridgeConfiguration
default and is otherwise unchanged.

## Parameter order

1. LED Brightness
2. Pitch Inversion
3. Roll Inversion
4. Yaw Inversion
5. Aux 1 Inversion
6. Aux 2 Inversion
7. Aux 3 Inversion
8. Aux 4 Inversion
9. Restore Defaults

Parameter count: 9.

## Persistence

No EEPROM schema change.

Schema v2 remains unchanged. The stored throttle-inversion bit is simply not
user-configurable in this build. Restore Defaults still resets the complete
BridgeConfiguration object, including throttle inversion, to its normal default.

## Files changed

- `src/bridge_parameters.h`
- `src/bridge_parameters.cpp`
- `src/crsf_parameter_self_test.cpp`

The remaining files are inherited from the clean Checkpoint 8 menu-order
baseline.

## Primary test

Open:

ExpressLRS → Other Devices → ELRS-HID-Bridge

Watch the loading bar closely.

The key question is whether the previous long pause after Roll Inversion is
gone now that Throttle Inversion has been removed.

Also confirm:

- all 9 fields eventually appear,
- Restore Defaults is last,
- Roll and Yaw inversion still work,
- at least one Aux inversion still works,
- persistence remains functional,
- Restore Defaults still works,
- HID/failsafe/reconnect behavior is unchanged.

## Interpretation

If the long pause disappears, Throttle Inversion is strongly isolated as the
trigger and should remain removed until investigated separately.

If the pause still occurs, the problem is more general than that parameter and
we should investigate parameter-response timing/retries instead.

## Commit guidance

Treat this as diagnostic first.

If it loads cleanly and you want to retain the removal:

`fix: remove throttle inversion parameter`

Otherwise do not commit it.
