# T-Maint Checkpoint — Maintenance Rotation Cleanup

## Scope

This checkpoint changes only the BOOT-button maintenance selection rotation.
It does not include any Throttle Inversion work or other feature changes.

## Modified files

- `src/maintenance_controller.h`
- `src/maintenance_controller.cpp`

## Behavior change

Previous hold rotation:

- release before 2 s: Diagnostic
- 2–4 s: Bind / Blue
- 4–6 s: Wi-Fi / White
- 6–8 s: Receiver Reset / Red
- 8–10 s: No Action
- repeat

New hold rotation:

- release before 2 s: Diagnostic
- 2–4 s: Bind / Blue
- 4–6 s: No Action / Cancel
- 6–8 s: Bind / Blue
- 8–10 s: No Action / Cancel
- repeat

## Preserved behavior

- Short-press Link Quality diagnostic
- Two-second maintenance selection intervals
- Repeating hold rotation
- Execute-on-release safety
- Existing CRSF receiver Bind action
- No Action as a safe escape
- Non-blocking button handling

## Implementation note

`MaintenanceSelection::Wifi`, `MaintenanceSelection::ReceiverReset`, and
`MaintenanceAction::WifiRequested` remain as compatibility enum values so this
checkpoint does not require unrelated changes to `main.cpp`. The controller no
longer emits those selections/actions, so the white Wi-Fi and red reset slots
are unreachable and no longer visible during the maintenance rotation.

They can be removed in a later cleanup when the surrounding display/action
dispatch is intentionally revised.

## Validation checklist

1. Overlay the two `src` files from this package.
2. Build the normal `pico` environment in VS Code / PlatformIO.
3. Confirm VS Code reports no Problems.
4. Flash the bridge.
5. With the transmitter linked, short-press BOOT and verify the existing Link
   Quality diagnostic still operates.
6. Hold BOOT and verify:
   - 2–4 s: blue Bind indication
   - 4–6 s: normal operational indication / No Action
   - 6–8 s: blue Bind indication
   - 8–10 s: normal operational indication / No Action
   - continued holding repeats this two-slot cycle
7. Release during a No Action interval and verify nothing executes.
8. Release during a Bind interval and verify the receiver Bind command still
   executes as previously validated on ExpressLRS 3.4+.
9. Confirm normal HID operation and reconnect/failsafe behavior remain unchanged.

## Out of scope

- Throttle Inversion
- CRSF parameter changes
- Wi-Fi commands
- Receiver factory reset
- Firmware update / serial passthrough
- Documentation synchronization beyond this checkpoint note
