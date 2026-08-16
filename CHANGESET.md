# ELRS-HID-Bridge — Checkpoint 9 Revision: RC Link + Failsafe Count

## Why this revision

The first Checkpoint 9 build loaded the menu immediately, proving that the
single short CRSF INFO field did not reproduce the earlier parameter-loading
delay.

However, `RC Link = Lost` cannot be directly inspected while the transmitter is
off because the ExpressLRS Lua device connection itself depends on that link.

This revision adds one additional read-only field that remains useful after the
link recovers:

**Failsafe Count**

## Menu order

1. LED Brightness
2. Pitch Inversion
3. Roll Inversion
4. Yaw Inversion
5. Aux 1 Inversion
6. Aux 2 Inversion
7. Aux 3 Inversion
8. Aux 4 Inversion
9. RC Link
10. Failsafe Count
11. Restore Defaults

## RC Link

Unchanged:

- Waiting
- Active
- Lost

It is still useful for normal linked-state display and internal/self-test
coverage, even though Lost cannot practically be viewed through Lua while the
RF control link itself is absent.

## Failsafe Count

CRSF type:

`INFO`

Value:

decimal count from `BridgeState::failsafeCount()`.

The existing BridgeState increments this count only when the RC timeout
transitions into receiver-lost/failsafe state.

Recovery does not erase the count.

The counter is runtime diagnostic state and is not persisted across bridge
power cycles.

## Practical hardware test

1. Power bridge and transmitter normally.
2. Open ELRS-HID-Bridge.
3. Confirm:
   - RC Link = Active
   - note the current Failsafe Count.
4. Exit the Lua device page if necessary.
5. Turn the transmitter off.
6. Wait long enough for the bridge's normal failsafe timeout.
7. Turn the transmitter back on.
8. Wait for ELRS to reconnect.
9. Reopen ELRS-HID-Bridge.
10. Confirm:
    - RC Link = Active
    - Failsafe Count increased by exactly one.

Repeat once if desired. Each actual transition into receiver-lost should add
one; remaining disconnected should not continuously increment the counter.

## Menu loading test

The device page should still populate promptly.

If adding Failsafe Count reintroduces a long loading pause, stop there and do
not commit this revision.

## Persistence

Unchanged.

Neither RC Link nor Failsafe Count is stored in EEPROM.

Configuration schema v2 remains current.

## Files changed from first Checkpoint 9

- `src/bridge_parameters.h`
- `src/bridge_parameters.cpp`
- `src/crsf_parameter_self_test.cpp`

Other files are carried forward complete from the first Checkpoint 9 package.

## Self-test additions

Startup tests now verify:

- Failsafe Count initially reports 0,
- one receiver-lost transition reports 1,
- subsequent RC recovery leaves the count at 1,
- the INFO parameter is read-only.

## Regression test

Also confirm:

- Restore Defaults is still last,
- one analog inversion still works,
- LED Brightness still persists,
- HID neutralizes on transmitter loss,
- HID recovers after reconnection.

## Suggested commit

If the menu remains fast and the counter test succeeds:

`feat: add bridge link health info`
