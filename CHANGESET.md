# M1 — Repeating BOOT Maintenance Selection State Machine

## Intent

Implement only the physical BOOT-button maintenance-selection UI described in the 2026-08-17 coding handoff. No new CRSF receiver command is transmitted in this checkpoint.

## Replace these files

```text
src/maintenance_controller.h
src/maintenance_controller.cpp
src/status_display.h
src/status_display.cpp
src/status_led.h
src/status_led.cpp
```

`src/main.cpp` is intentionally unchanged in M1.

## New interaction

```text
release < 2 s  -> existing short diagnostic
2–4 s          -> Bind               / blue
4–6 s          -> Wi-Fi              / white
6–8 s          -> Receiver Reset     / blinking red
8–10 s         -> No Action / Cancel / normal status
10–12 s        -> Bind
12–14 s        -> Wi-Fi
14–16 s        -> Receiver Reset
16–18 s        -> No Action / Cancel
...repeat
```

Every maintenance interval is exactly two seconds. The four-slot menu repeats indefinitely while BOOT remains held.

## Safety behavior

- Selection changes while held.
- Actions are evaluated only on release.
- Release during No Action produces no action.
- The Receiver Reset/Recovery slot is **presentation-only in M1** and produces no action on release.
- No receiver-reset command is invented or transmitted.
- Bind and Wi-Fi retain their existing reserved action events; the existing `main.cpp` still sends no receiver command for either.

This means M1 cannot alter RP2 receiver configuration.

## LED behavior

- Bind: blue
- Wi-Fi: white
- Receiver Reset/Recovery: blinking red
- No Action: current normal operational/status indication

The red blink is non-blocking. `StatusDisplay::update()` advances the blink using `nowMs`; no multi-second `delay()` logic is introduced.

## Compatibility note

To keep M1 isolated from the CRSF/application command path, `main.cpp` is unchanged. The existing `MaintenanceSelection::Cancel` and `showMaintenanceCancel()` names are retained as compatibility aliases/wrappers for the new Receiver Reset/Recovery presentation slot. The actual No Action interval is represented by `MaintenanceSelection::None`, which causes the existing main-loop display dispatch to call `clearMaintenance()` and restore normal status.

This compatibility shim should be removed when a later checkpoint deliberately modifies `main.cpp` to add verified receiver command dispatch.

## Build gate

1. Overlay the ZIP contents at the repository root.
2. In VS Code / PlatformIO select the normal `pico` environment.
3. Build normally.
4. Confirm no build errors.
5. Confirm VS Code Problems contains no new issues.
6. Upload normally.

Avoid `pico_debug` and routine direct `pio` CLI use.

## Hardware test — maintenance UI

With normal RC/HID operation established:

1. Short press and release before 2 seconds.
   - Existing Link Quality diagnostic still appears.
   - It returns automatically to normal status.
2. Hold through 2 seconds.
   - LED becomes blue.
3. Continue through 4 seconds.
   - LED becomes white.
4. Continue through 6 seconds.
   - LED begins blinking red.
   - Confirm the bridge continues normal HID operation while it blinks.
5. Continue through 8 seconds.
   - Maintenance indication clears and normal operational/status LED returns.
6. Continue through 10 seconds.
   - Blue Bind selection returns.
7. Continue through at least one additional complete cycle.
   - Verify Blue -> White -> blinking Red -> normal/status repeats consistently.
8. Release during No Action.
   - Nothing executes.
9. Release during the red slot.
   - Nothing executes in M1; the RP2 must not reset or change configuration.

## Regression gate

After the maintenance UI test, verify:

- normal USB enumeration
- `joy.cpl`
- primary axes
- auxiliary axes
- buttons
- current EdgeTX device menu
- persisted configuration
- transmitter-off failsafe
- safe analog states
- released buttons during failsafe
- automatic reconnect
- Failsafe Count behavior

A Liftoff smoke test is optional because M1 does not modify the control path, but it remains useful if anything unexpected is observed.

## Intentionally unchanged

- RC timeout semantics
- failsafe policy
- HID mappings
- axis orientations
- switch mappings
- BridgeConfiguration persistence
- CRSF parameter menu
- Diagnostics folder
- Throttle Inversion state
- CRSF frame IDs/routing
- receiver Bind command implementation
- receiver Wi-Fi command implementation
- receiver Reset/Recovery command implementation

## Commit gate

Do not commit until the M1 hardware test and regression checks pass.

Suggested commit after validation:

```text
feat: add repeating BOOT maintenance selection cycle
```
