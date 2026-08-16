# Build Pinning Checkpoint

## Purpose

Freeze the release-critical packages that produced the known-good hardware-tested firmware, while closing the v1.0 USB identity decision without changing runtime behavior.

## Replace

- `platformio.ini`
- `README.md`
- `docs/Architecture.md`
- `docs/Roadmap.md`
- `docs/Release.md`
- `docs/USB-Identity.md`

## Known-good capture

- Project Git HEAD: `75aa4ff7fbdb05b3251452f5f14a736a22174744`
- Raspberry Pi platform manifest: `1.20.0`
- Arduino-Pico framework: `1.60000.0`
- RP2040 toolchain: `5.160100.260719`
- Adafruit NeoPixel: `1.15.5`

## Expected behavior

No runtime behavior should change. USB product identity remains `ELRS-HID-Bridge`; inherited VID/PID remains `2E8A:000A`; `joy.cpl` may continue to show `Pico`.

## Test gate

1. Build the normal `pico` environment in VS Code/PlatformIO.
2. Confirm no new Problems/build errors.
3. Flash normally.
4. Verify axes/buttons and LED states.
5. Verify TX-off failsafe and automatic reconnect.
6. Verify Liftoff behavior/performance.
7. Verify `ELRS-HID-Bridge` still appears under EdgeTX **Other Devices**.
8. Optional: rerun `tools/Capture-BuildEnvironment.ps1`; framework/toolchain/NeoPixel values should match the pinned versions.

## Suggested commit

`Pin known-good release build dependencies`
