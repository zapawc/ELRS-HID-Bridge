# ELRS-HID-Bridge — Checkpoint 10: Diagnostics Folder

## Intent

Finish the current development session with a cleaner separation between:

- user configuration,
- bridge diagnostics,
- future CRSF telemetry transport.

This checkpoint removes the redundant `RC Link` INFO field and moves
`Failsafe Count` into a dedicated `Diagnostics` folder.

No CRSF telemetry frames are added.

## Root menu

Expected top-level device page:

1. LED Brightness
2. Pitch Inversion
3. Roll Inversion
4. Yaw Inversion
5. Aux 1 Inversion
6. Aux 2 Inversion
7. Aux 3 Inversion
8. Aux 4 Inversion
9. Diagnostics
10. Restore Defaults

`Restore Defaults` remains last.

## Diagnostics folder

Opening `Diagnostics` should show:

- Failsafe Count

That is the only diagnostic item in this checkpoint.

## Failsafe Count

Unchanged semantically.

It is a read-only CRSF `INFO` value derived from:

`BridgeState::failsafeCount()`

It increments when the existing RC timeout transitions the bridge into
receiver-lost/failsafe state.

It does not increment continuously while the link remains lost.

A recovered RC frame does not erase the count.

The count is runtime-only and resets on bridge reboot.

## Why RC Link was removed

The transmitter/ExpressLRS environment already presents RF/control-link status
directly.

Duplicating `RC Link` in the bridge device menu adds little diagnostic value,
especially because the Lua device menu itself is unavailable while the radio
link is down.

Failsafe Count is different: after reconnection it provides a useful
post-incident artifact showing that the bridge actually experienced one or more
RC-channel timeout transitions.

## Why diagnostics stay out of telemetry

This project will not invent custom telemetry meanings or consume legitimate
CRSF sensor channels for bridge-internal bookkeeping.

The telemetry path remains architecturally available for future legitimate
upstream telemetry, for example:

Simulator/game
    → USB or virtual serial
    → ELRS-HID-Bridge
    → standard CRSF telemetry
    → ELRS
    → EdgeTX

That allows a simulator to provide genuine values such as battery, GPS,
temperature, RPM, or other data when those values naturally match standard CRSF
telemetry semantics.

## Parameter IDs

- 1 — LED Brightness
- 2 — Pitch Inversion
- 3 — Roll Inversion
- 4 — Yaw Inversion
- 5 — Aux 1 Inversion
- 6 — Aux 2 Inversion
- 7 — Aux 3 Inversion
- 8 — Aux 4 Inversion
- 9 — Diagnostics folder
- 10 — Failsafe Count
- 11 — Restore Defaults

Parameter count remains 11.

The former RC Link ID 9 is reused as the Diagnostics folder because this work is
still in the post-v1 development series and has not been released as a new
compatibility contract.

## Persistence

Unchanged.

- EEPROM schema remains v2.
- Diagnostics are not persisted.
- No configuration values are added or removed.
- Restore Defaults behavior is unchanged.

## Files changed

- `src/bridge_parameters.h`
- `src/bridge_parameters.cpp`
- `src/crsf_parameter_self_test.cpp`

All other files are carried forward complete from the validated Checkpoint 9
revision.

## Self-test coverage

Startup parameter tests now verify:

- parameter count remains 11,
- root contains Diagnostics but not Failsafe Count directly,
- Restore Defaults is last,
- Diagnostics is a CRSF FOLDER,
- Diagnostics contains Failsafe Count,
- Failsafe Count parent is Diagnostics,
- Failsafe Count remains INFO,
- initial count is 0,
- one receiver-loss transition reports 1,
- recovery preserves the count,
- diagnostics remain read-only,
- existing Restore Defaults behavior remains valid.

## Hardware test

### Menu structure

1. Build normal `pico`.
2. Flash.
3. Open ExpressLRS → Other Devices → ELRS-HID-Bridge.
4. Confirm the root menu loads promptly.
5. Confirm:
   - `Diagnostics` appears near the bottom,
   - `Failsafe Count` is no longer shown at root,
   - `Restore Defaults` is last.
6. Open Diagnostics.
7. Confirm `Failsafe Count` appears inside.

### Counter behavior

1. Note Failsafe Count.
2. Exit the device page if necessary.
3. Turn the transmitter off long enough to trigger normal bridge failsafe.
4. Turn the transmitter back on.
5. Reconnect.
6. Open Diagnostics.
7. Confirm Failsafe Count increased by exactly one.

### Regression

Confirm:

- LED Brightness still works and persists,
- one inversion setting still works and persists,
- Restore Defaults still confirms and persists defaults,
- HID neutralizes during transmitter loss,
- HID recovers after reconnection,
- no long Lua parameter-loading pause returns.

## Success criteria

Checkpoint 10 passes when:

- normal `pico` build is clean,
- root menu loads promptly,
- Diagnostics folder works,
- only Failsafe Count appears inside it,
- Restore Defaults remains last,
- counter behavior remains correct,
- existing configuration/persistence/HID/failsafe behavior remains unchanged.

## Suggested commit

`refactor: group bridge diagnostics`

## Pause / handoff state

This is intended as a clean pause point.

After validation, the next development session should begin from the principle:

- configuration belongs in the CRSF device menu,
- bridge-specific troubleshooting data belongs under Diagnostics,
- standard CRSF telemetry remains reserved for genuine telemetry semantics,
- simulator-to-transmitter telemetry transport is a future architectural
  opportunity rather than something to consume for internal bridge status.
