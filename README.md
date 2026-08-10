# ELRS HID Bridge

Convert an ExpressLRS/CRSF receiver into a USB HID joystick using an Adafruit QT Py RP2040.

## Status

🚧 Early development

Currently implemented:

- USB HID joystick
- Modular architecture
- CRSF frame parser
- CRC validation
- RC channel decoder framework

## Hardware

- Adafruit QT Py RP2040
- ExpressLRS CRSF Receiver

## Goals

- Zero driver installation
- Plug-and-play for Liftoff, VelociDrone, DRL, etc.
- Open source