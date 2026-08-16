# ELRS-HID-Bridge — Post-v1.0 Checkpoint 6

## Intent

Complete the first configuration lifecycle with a controlled transmitter-side
factory reset:

**Restore Defaults**

This checkpoint adds one CRSF `COMMAND` parameter and does not add any new
mapping/value setting.

## Existing configuration lifecycle

Before this checkpoint the bridge already supports:

- LED Brightness — CRSF FLOAT
- Pitch Inversion — CRSF TEXT_SELECTION
- persistent storage in Arduino-Pico EEPROM emulation
- versioned/CRC-protected configuration record
- safe boot fallback

Checkpoint 6 adds the recovery path:

configure → persist → restore known-good defaults

## Parameter list

- Parameter 0 — ROOT
- Parameter 1 — LED Brightness
- Parameter 2 — Pitch Inversion
- Parameter 3 — Restore Defaults

Device Info parameter count becomes 3 automatically because
`BridgeIdentity::CRSF_PARAMETER_COUNT` derives from
`BridgeParameters::PARAMETER_COUNT`.

## CRSF COMMAND behavior

Restore Defaults uses standard CRSF `COMMAND` (`0x0D`) state semantics.

States used:

- `0` READY
- `1` START
- `3` CONFIRMATION_NEEDED
- `4` CONFIRM
- `5` CANCEL
- `6` POLL

The command definition is returned as a Parameter Settings Entry (`0x2B`).

A COMMAND write is also answered with an updated `0x2B` command entry rather
than a simple value-only `0x2D` acknowledgement.

## EdgeTX / ExpressLRS Lua r18 flow

The uploaded Lua r18 script:

1. sends START when Restore Defaults is selected,
2. opens a confirmation popup when status 3 is returned,
3. sends CONFIRM when the user approves,
4. sends CANCEL when the user cancels,
5. can send POLL to refresh command state.

The firmware follows this exact lifecycle.

## Safety behavior

Selecting Restore Defaults does **not** immediately modify configuration.

Flow:

START
  → bridge marks confirmation pending
  → bridge returns CONFIRMATION_NEEDED
  → user sees confirmation dialog

If user cancels:

CANCEL
  → no configuration change
  → no flash write
  → READY / Cancelled

If user confirms:

CONFIRM
  → BridgeConfiguration::defaults() is staged
  → default record is written to EEPROM
  → only if persistence succeeds:
       - LED brightness default is applied immediately
       - pitch default is already visible through BridgeConfiguration
       - command becomes READY
       - READY / Defaults restored response is sent

If persistence fails:

- pre-command configuration is restored,
- no success response is transmitted,
- confirmation remains pending for host retry/poll.

## Restored defaults

Current known-good defaults remain:

- LED Brightness = 10
- Pitch Inversion = Inverted

No other configuration fields are changed away from
`BridgeConfiguration::defaults()`.

## Files changed

- `src/crsf_device.h`
- `src/crsf_device.cpp`
- `src/bridge_parameters.h`
- `src/bridge_parameters.cpp`
- `src/crsf_parameter_self_test.cpp`
- `src/main.cpp`

## Protocol abstraction

`CrsfDevice` gains one generic protocol helper for CRSF COMMAND parameter
entries.

Bridge-specific command policy remains in `BridgeParameters`.

No factory-reset knowledge is added to the CRSF parser, dispatcher, UART, or
frame encoder.

## Self-test coverage

The CRSF parameter startup self-test now covers:

- parameter count = 3,
- root folder contains all three parameters,
- existing FLOAT and TEXT_SELECTION entries still build,
- Restore Defaults initially reports READY,
- START returns CONFIRMATION_NEEDED,
- START does not modify configuration,
- POLL preserves pending confirmation,
- CONFIRM stages known-good defaults,
- CONFIRM requires persistence,
- simulated persistence failure leaves confirmation pending,
- simulated persistence success returns the command to READY,
- CANCEL leaves configuration unchanged,
- CONFIRM without a prior START cannot reset configuration.

The test does not write live EEPROM.

## Intentionally unchanged

- LED Brightness representation/range
- Lua r18 blank-unit workaround
- Pitch Inversion representation/default
- persistent record schema
- EEPROM storage layout
- HID descriptor
- channel assignments
- switch/button mappings
- receiver timeout
- failsafe policy
- Link Statistics behavior
- BOOT-button behavior
- Bind/Wi-Fi command execution
- board portability
- `pico_debug`
- firmware semantic version

## Build procedure

Use the established VS Code / PlatformIO workflow.

1. Overlay this ZIP at repository root.
2. Build the normal `pico` environment.
3. Confirm VS Code Problems is clear.
4. Flash the normal build.
5. Do not use `pico_debug`.

## Hardware test procedure

### Establish non-default persisted state

Before testing Restore Defaults:

1. Open ExpressLRS → Other Devices → ELRS-HID-Bridge.
2. Set LED Brightness to 50.
3. Set Pitch Inversion to Normal.
4. Power-cycle the bridge.
5. Confirm both values survived the reboot.

This proves the reset has real persisted state to replace.

### Command discovery

1. Reopen ELRS-HID-Bridge.
2. Confirm these entries appear:
   - LED Brightness
   - Pitch Inversion
   - Restore Defaults
3. Confirm the device page opens without a Lua error.

### Cancellation test

1. Select Restore Defaults.
2. A confirmation dialog should appear.
3. Cancel/exit the confirmation.
4. Confirm:
   - LED Brightness remains 50,
   - Pitch Inversion remains Normal,
   - HID pitch direction remains Normal.
5. Power-cycle and verify the same values remain persisted.

No reset and no EEPROM update should occur from cancellation.

### Confirmed Restore Defaults test

1. Select Restore Defaults again.
2. Confirm the action in the transmitter popup.
3. Expected immediate result:
   - LED brightness returns to 10,
   - Pitch Inversion returns to Inverted,
   - pitch direction changes accordingly in `joy.cpl`.
4. Close/reopen the device page.
5. Confirm both parameters show their defaults.

### Persistence test

1. Fully remove USB power.
2. Reconnect the bridge.
3. Re-establish ELRS.
4. Reopen ELRS-HID-Bridge.
5. Confirm:
   - LED Brightness = 10,
   - Pitch Inversion = Inverted.
6. Confirm `joy.cpl` reflects the validated inverted pitch orientation.

### General regression

1. Change LED Brightness and confirm it still persists normally.
2. Change Pitch Inversion and confirm it still persists normally.
3. Verify all eight analog controls.
4. Verify existing buttons.
5. Turn the transmitter off.
6. Confirm deterministic failsafe.
7. Turn the transmitter back on.
8. Confirm automatic recovery.

## Success criteria

Checkpoint 6 succeeds when:

- normal `pico` build is clean,
- startup self-tests pass,
- Restore Defaults appears as a command,
- selecting it requires explicit confirmation,
- cancellation leaves runtime and persistent settings untouched,
- confirmation restores both known-good defaults,
- restored defaults survive power cycle,
- ordinary parameter persistence still works,
- HID/failsafe behavior remains correct.

## Suggested commit

`feat: add confirmed restore defaults command`

## Next checkpoint

Once validated, the configuration lifecycle is complete enough to resume
feature expansion.

The next planning decision should compare:

- a high-value mapping/configuration parameter, versus
- the first small bridge-health telemetry checkpoint.

Do not add another recovery mechanism merely for symmetry.
