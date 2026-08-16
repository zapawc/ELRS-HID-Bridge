# Third-Party Software and License Notes

ELRS-HID-Bridge is licensed under **GPL-3.0-only**. The project is built with and/or depends on third-party open-source components that retain their own licenses and copyright notices.

This file is a practical dependency-license inventory for the reference PlatformIO build. It is not a replacement for the license text distributed by each upstream project.

## Runtime / firmware dependencies

| Component | Role | Upstream license | GPL-3.0 project compatibility |
|---|---|---|---|
| Arduino-Pico (Earle Philhower RP2040 core) | Arduino framework/core for RP2040 | LGPL-2.1-or-later | Compatible |
| Raspberry Pi Pico SDK | Low-level RP2040 SDK used by Arduino-Pico | BSD-3-Clause | Compatible |
| Adafruit TinyUSB Arduino | Arduino-facing TinyUSB integration | MIT | Compatible |
| TinyUSB | USB device stack | MIT | Compatible |
| Adafruit NeoPixel | QT Py RGB status LED | LGPL-3.0-or-later | Compatible |

## Build tooling

| Component | Role | Upstream license |
|---|---|---|
| maxgerhardt/platform-raspberrypi | PlatformIO integration used to select Arduino-Pico | Apache-2.0 |
| PlatformIO | Build system / VS Code integration | See upstream PlatformIO distribution |
| GCC / RP2040 toolchain | Compiler/toolchain | See upstream toolchain distribution |

Build tools are not relicensed as part of ELRS-HID-Bridge merely because they are used to build the firmware.

## Audit conclusion

No license conflict was identified in the dependencies explicitly used by the current reference build that would prevent licensing ELRS-HID-Bridge project code as GPL-3.0-only.

This is a project-maintenance audit, not legal advice. If the dependency set changes, especially if source is copied directly into the repository or a new statically linked library is added, update this inventory before a release.
