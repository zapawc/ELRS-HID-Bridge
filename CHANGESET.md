# ELRS-HID-Bridge M2 — Receiver Bind

## Intent

Implement the verified CRSF receiver Bind command behind the already validated M1 BOOT-button menu.

M1 interaction remains unchanged:

- short press: existing diagnostic
- 2–4 s: Bind / blue
- 4–6 s: Wi-Fi / white
- 6–8 s: reserved Reset/Recovery / blinking red
- 8–10 s: No Action / normal status
- repeat while held

Only **Bind** gains a live receiver command in M2.

## Files

Complete replacement / added files:

- `src/main.cpp`
- `src/crsf_receiver_command.h`
- `src/crsf_receiver_command.cpp`
- `src/crsf_receiver_command_self_test.h`
- `src/crsf_receiver_command_self_test.cpp`

No M1 state-machine/display files are changed.

## Verified CRSF Bind Frame

The implementation follows the receiver Bind command supported by ExpressLRS 3.4+ and the known-good Betaflight `crsfRxBind()` vector:

```text
C8 07 32 EC C8 10 01 9E E8
```

Meaning:

- Sync: `0xC8`
- Length: `0x07`
- Type: `0x32` COMMAND
- Destination: `0xEC` CRSF receiver
- Origin: `0xC8` flight-controller side
- Receiver command group: `0x10`
- Bind subcommand: `0x01`
- Command CRC8: `0x9E` (polynomial `0xBA`)
- Packet CRC8: `0xE8` (standard CRSF polynomial `0xD5`)

`CrsfReceiverCommandSelfTest` independently checks the generated frame against that exact golden vector during startup. The self-test does not transmit.

## ExpressLRS Version Requirement

Receiver-side support for this CRSF Bind command was added for **ExpressLRS 3.4**.

If the test RP2 is still on ExpressLRS 3.3.1, update the receiver to 3.4 or newer before expecting M2 Bind to work. The bridge build itself does not require changing the transmitter firmware merely to compile this checkpoint.

## Runtime Behavior

On release during the blue Bind interval:

1. `MaintenanceController` emits the existing `BindRequested` action.
2. `CrsfReceiverCommand` constructs the verified command frame.
3. `main.cpp` writes it once to the existing receiver-facing CRSF UART.
4. Normal loop processing continues.

No acknowledgement packet is invented or awaited. Hardware receiver behavior is the validation source.

## Intentionally Unchanged

- M1 2-second menu timing and repeat behavior
- execute-on-release semantics
- short-press diagnostic
- Wi-Fi remains a no-op (M3)
- red Reset/Recovery remains a no-op (M4 research)
- No Action remains a no-op
- RC timeout/failsafe semantics
- HID mapping/orientation
- configuration persistence
- EdgeTX device menu
- Diagnostics / Failsafe Count

## Build / Test

Use the normal `pico` environment in VS Code / PlatformIO.

### Precondition

Confirm the RP2 is running ExpressLRS 3.4+ before treating a no-response result as an M2 firmware failure.

### M2 Bind test

1. Power the bridge and establish normal ELRS control.
2. Confirm normal HID operation first.
3. Hold BOOT until the LED is blue (2–4 s).
4. Release while blue.
5. Verify the RP2 actually enters ExpressLRS binding mode.
6. Verify normal recovery/rebinding procedure.
7. Re-establish the normal link and confirm HID automatically resumes.

### Safety / menu regression

Verify:

- short press still performs diagnostic only,
- white Wi-Fi release does not trigger Bind,
- blinking-red release does not trigger Bind or reset anything,
- No Action release does nothing,
- a continued hold still repeats the M1 menu correctly.

### General regression

After rebinding:

- normal USB enumeration
- `joy.cpl`
- primary axes
- auxiliary axes
- buttons
- EdgeTX device menu
- persisted settings
- transmitter-off failsafe
- safe analog states / released buttons
- automatic reconnect
- Failsafe Count

Check VS Code Problems before committing.

## Commit Gate

Commit M2 only after the real RP2 enters bind mode from BOOT Blue → Release and the regression checks pass.
