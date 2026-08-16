# ELRS-HID-Bridge Post-v1.0 Checkpoint 1

## Intent

Harden CRSF receive-path regression coverage before adding the first CRSF/EdgeTX configurable parameter.

This checkpoint intentionally changes **self-test coverage only**. It does not change production parser, decoder, HID, failsafe, LED, configuration, or CRSF device behavior.

## Baseline

- Stable release: `v1.0.0`
- Release commit: `f4403e2db7c2649e6560fafd45ad2d8cba3acacc`
- Authoritative build environment: PlatformIO `pico`

## Files changed

- `src/crsf_self_test.cpp`
- `src/crsf_self_test.h`

## Regression coverage added / strengthened

1. Frozen CRSF `0x16` RC-channel golden frame with fixed expected values for all 16 channels.
   - The primary positive fixture is no longer generated at runtime by the self-test.
   - This reduces the risk that test-frame generation and production decoding share the same packing mistake.

2. Frozen CRSF `0x14` Link Statistics frame.
   - Exercises the real parser -> dispatcher -> LinkStatisticsDecoder path.
   - Verifies all ten currently consumed Link Statistics fields, including signed SNR values.

3. Corrupt-CRC rejection using the frozen RC fixture.

4. Valid CRSF sync/address acceptance using the same frozen frame.

5. Invalid sync-byte recovery.

6. Good frame -> garbage -> good frame recovery.

7. Bad-CRC frame -> good frame recovery.

8. Repeated invalid-length resynchronization followed by a valid frame.

## Intentionally unchanged

- USB HID descriptor and mapping
- Eight-axis HID behavior
- Receiver timeout / failsafe behavior
- Link-state precedence rules
- CRSF frame parser implementation
- CRSF dispatcher implementation
- CRSF device discovery behavior
- EdgeTX interaction
- LED behavior
- BOOT-button behavior
- Persistent configuration
- `pico_debug`

## Build / validation

Use the established VS Code / PlatformIO workflow.

1. Overlay the ZIP contents into the repository root.
2. Open the project in VS Code.
3. Confirm there are no new entries in **Problems**.
4. Build the normal `pico` environment.
5. Flash the normal `pico` build to the QT Py RP2040.
6. Confirm startup completes normally. A CRSF self-test failure should prevent the normal startup state, so normal operation is the first hardware indication that all added fixtures passed.

## Hardware smoke regression

With the established RP2 / Ranger / EdgeTX setup:

1. TX off at bridge startup: confirm expected disconnected/failsafe indication.
2. Power TX and establish the ELRS link: confirm normal green operational state.
3. Open Windows `joy.cpl`:
   - verify Roll, Pitch, Throttle, and Yaw directions remain correct;
   - verify auxiliary analog axes still respond;
   - verify representative buttons still respond.
4. Power the TX off:
   - verify all eight analog HID controls neutralize;
   - verify all buttons release;
   - verify failsafe indication remains correct.
5. Power the TX back on:
   - verify normal control recovers without rebooting the bridge.
6. Optional but recommended: launch Liftoff and confirm normal simulator control.

## Commit boundary

If build and hardware regression pass, commit this checkpoint independently before beginning CRSF parameter work.

Suggested commit message:

`test: harden CRSF receive-path regression coverage`

## Next checkpoint

Implement the first CRSF/EdgeTX parameter proof of concept:

**LED brightness, represented as a standard CRSF FLOAT parameter from 0-100%.**

Do not add persistence in that checkpoint. The first objective is to prove discovery, read, write, validation, runtime configuration update, and visible LED behavior while preserving the v1.0.0 HID/failsafe baseline.
