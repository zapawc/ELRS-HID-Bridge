# Documentation Synchronization Changeset

## Intent

Bring public project documentation into alignment with the current post-v1.0 source and the hardware-validated findings from BOOT maintenance and Throttle Inversion troubleshooting.

## Files

- `README.md`
- `CHANGELOG.md`
- `docs/Architecture.md`
- `docs/Roadmap.md`
- `docs/Protocol.md`
- `docs/Receiver-Maintenance-Protocol.md`

## Captured current state

- CRSF/EdgeTX parameter configuration is implemented.
- Persistent bridge configuration is implemented.
- `Throttle Invert` is implemented and hardware validated.
- CRSF parameter names are limited to <=16 visible characters as a validated compatibility rule.
- Diagnostics currently contains only Failsafe Count.
- Restore Defaults is implemented.
- BOOT rotation is Diagnostic -> Bind -> No Action/Cancel -> repeat.
- Receiver Bind requires ExpressLRS 3.4.0+ and is validated on RP2 / 3.4.3.
- Wi-Fi and receiver factory reset are intentionally not implemented.
- CH15/CH16 proportional shoulder-slider behavior is validated after the RP2 firmware upgrade.
- Receiver Firmware Update / USB Serial Passthrough is retained as a future feature.
- Current runtime feature work is frozen pending full release regression.

## Intentionally not changed

- firmware version source
- release version number
- release checklist
- release packaging metadata
- source code
- platform/toolchain configuration

Version/release metadata should be changed only after the documentation overlay is committed and the full release regression has passed.
