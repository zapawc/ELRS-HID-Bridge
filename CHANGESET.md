# ELRS-HID-Bridge — Post-v1.0 Checkpoint 5

## Intent

## Compatibility correction

The first Checkpoint 5 package incorrectly targeted Mbed KVStore. The actual
PlatformIO build uses `framework-arduinopico`, so this revision replaces only
the persistence transport with the Arduino-Pico `EEPROM` API. The record schema,
CRC validation, boot fallback, parameter behavior, and transactional
acknowledgement logic are unchanged.

Add persistent storage for the two CRSF-configurable settings already proven on
hardware:

- LED Brightness
- Pitch Inversion

This checkpoint does not add any new user-facing parameter.

## Storage architecture

Persistence is deliberately split into two layers.

### BridgeConfigurationRecord

Pure serialization/validation logic.

It owns:

- record magic,
- schema version,
- payload length,
- persisted-field encoding,
- CRC-32 generation/validation,
- range validation,
- safe decode behavior.

It has no flash or CRSF dependency.

### BridgeConfigurationStore

Storage transport only.

The project's actual PlatformIO toolchain uses Earle Philhower's Arduino-Pico
core (`framework-arduinopico`), not ArduinoCore-mbed.

Persistence therefore uses the Arduino-Pico `EEPROM` emulation API. The core
reserves one 4 KiB flash sector at the end of RP2040 flash for EEPROM emulation.
The bridge stores its 16-byte record beginning at EEPROM offset 0.

`EEPROM.begin(256)` uses the smallest supported working buffer. The core only
marks the buffer dirty when a byte actually changes, so committing an unchanged
record does not erase/program flash.

## Record schema v1

Fixed length: 16 bytes

Layout:

- bytes 0–3: magic `EHB1`
- byte 4: schema version = 1
- byte 5: payload length = 2
- byte 6: LED brightness, 0–100
- byte 7: pitch inversion
  - 0 = Normal
  - 1 = Inverted
- bytes 8–11: reserved, zero
- bytes 12–15: CRC-32 over bytes 0–11

Only the two real configurable settings are persisted.

All other BridgeConfiguration fields continue to come from
`BridgeConfiguration::defaults()`.

## Boot behavior

The global configuration object is still initialized from known-good defaults.

During `setup()`:

1. the bridge attempts to load the KVStore record,
2. the record must pass exact-size, magic, schema, payload, value-range, and
   CRC validation,
3. only then are the persistent fields accepted,
4. otherwise defaults remain untouched.

The load occurs before `StatusLed::begin()`, so a stored brightness is applied
from the first normal LED state.

## Write behavior

CRSF parameter writes are now transactional from the bridge's perspective.

1. Save a copy of the previous BridgeConfiguration.
2. `BridgeParameters` validates and stages the requested change.
3. The complete configuration record is written through Arduino-Pico EEPROM
   emulation and committed to flash.
4. Only if persistence succeeds:
   - the application-side effect is applied,
   - the CRSF `0x2D` acknowledgement is transmitted.
5. If persistence fails:
   - BridgeConfiguration is restored from the snapshot,
   - no acknowledgement is sent,
   - no new runtime value is exposed.

This makes an acknowledged parameter write mean both runtime acceptance and
durable storage.

## Flash-write frequency

The ExpressLRS Lua editor sends a parameter write when an edit is committed,
not on every cursor increment. Storage writes therefore occur on explicit
parameter changes, not continuously in the main loop.

## Self-test coverage

A new startup self-test exercises the record codec without touching live flash:

- encode/decode round trip,
- bad magic rejection,
- schema mismatch rejection,
- payload corruption / CRC rejection,
- invalid value rejection,
- wrong record-size rejection,
- failed decode leaves the current configuration unchanged.

The startup self-test does **not** erase, rewrite, or corrupt the real
EEPROM-emulation flash sector.

## Files added

- `src/bridge_configuration_record.h`
- `src/bridge_configuration_record.cpp`
- `src/bridge_configuration_store.h`
- `src/bridge_configuration_store.cpp`
- `src/bridge_configuration_record_self_test.h`
- `src/bridge_configuration_record_self_test.cpp`

## Files changed

- `src/main.cpp`

## Intentionally unchanged

- CRSF parameter definitions
- LED Brightness range/type
- ExpressLRS Lua r18 blank-unit workaround
- Pitch Inversion options/default
- channel assignments
- HID descriptors
- failsafe policy
- receiver timeout
- Link Statistics behavior
- BOOT-button behavior
- telemetry
- board portability
- `pico_debug`
- firmware semantic version

## Build procedure

Use the established VS Code / PlatformIO workflow.

1. Overlay this ZIP at the repository root.
2. Build the normal `pico` environment.
3. Confirm VS Code **Problems** is clear.
4. Flash the normal build.
5. Do not use `pico_debug`.

## Hardware test procedure

### First boot after Checkpoint 5 flash

Existing devices do not yet have a Checkpoint 5 configuration record in the
Arduino-Pico EEPROM-emulation sector.

Expected behavior:

1. bridge boots normally,
2. LED Brightness starts at 10,
3. Pitch Inversion starts at Inverted,
4. all HID behavior matches the prior validated checkpoint.

An erased/uninitialized EEPROM record fails magic/CRC validation and is treated
exactly like "no saved configuration yet."

### Persist both settings

1. Open ExpressLRS → Other Devices → ELRS-HID-Bridge.
2. Change LED Brightness to a distinctive value such as 50.
3. Change Pitch Inversion to Normal.
4. Confirm both changes take effect immediately.
5. Close/reopen the device page and confirm both current values are still shown.

### Power-cycle persistence

1. Disconnect USB power from the bridge.
2. Reconnect it.
3. Re-establish the ELRS link.
4. Reopen ELRS-HID-Bridge.
5. Confirm:
   - LED Brightness is still 50,
   - Pitch Inversion is still Normal.
6. Open `joy.cpl`.
7. Confirm the Y-axis still reflects Normal pitch orientation.

### Change persisted values again

1. Set LED Brightness to another value such as 25.
2. Set Pitch Inversion back to Inverted.
3. Power-cycle again.
4. Confirm both new values survive the reboot.

This verifies overwrite/update behavior, not just the first record creation.

### HID/failsafe regression

1. Confirm all eight analog controls operate normally.
2. Confirm all existing buttons remain correct.
3. Turn the transmitter off.
4. Confirm all analog controls neutralize and all buttons release.
5. Turn the transmitter back on.
6. Confirm automatic recovery.
7. Confirm the persisted pitch orientation is still honored.

## Corruption/fallback validation

Do not intentionally damage live flash for this checkpoint.

Corrupt-record and schema-mismatch behavior is exercised by
`BridgeConfigurationRecordSelfTest` using in-memory records during startup.

A real erased/uninitialized record is naturally exercised on the first
Checkpoint 5 boot and must fall back to defaults.

A future factory-reset/maintenance feature can provide a controlled way to
invalidate/erase the stored record; that is intentionally outside this
checkpoint.

## Success criteria

Checkpoint 5 succeeds when:

- normal `pico` build is clean,
- startup self-tests pass,
- first boot without a record uses defaults,
- both parameter values survive a full USB power cycle,
- subsequent changes overwrite the stored record successfully,
- LED and HID behavior remain correct,
- receiver-loss failsafe and reconnection remain unchanged.

## Suggested commit

`feat: persist bridge configuration`

## Next checkpoint

After validation, persistence is considered proven.

The next design choice should be whether to add:

- a controlled Restore Defaults CRSF command, or
- the next high-value configurable mapping parameter.

Do not add schema migration logic until a schema-v2 requirement actually exists.
