# ELRS-HID-Bridge v1.0.0-rc1 Preparation Checkpoint

This checkpoint begins release-candidate validation. It intentionally freezes runtime behavior.

## Add / replace these files

```text
.gitignore
CHANGELOG.md
README.md
CHANGESET.md

src/
    firmware_version.h
    firmware_version_self_test.cpp

docs/
    Architecture.md
    Protocol.md
    Release.md
    Release-Checklist.md
    Release-Notes-v1.0.0-rc1.md
    Roadmap.md
    USB-Identity.md

tools/
    Stage-Release.ps1
```

No `main.cpp`, CRSF parser/dispatcher, UART, HID mapping, failsafe, LED, or maintenance-controller behavior changes are included.

## Behavioral change

Only firmware/release identity changes:

```text
0.3.0-dev -> 1.0.0-rc1
CRSF Firmware ID -> 0x01000000
```

The CRSF prerelease suffix is not encoded, so final `1.0.0` will intentionally use the same numeric CRSF Firmware ID.

## Validation order

1. Extract this changeset over the repository root.
2. Use the normal VS Code PlatformIO workflow to build the `pico` environment.
3. Confirm no build errors / new VS Code Problems.
4. Flash the candidate.
5. Run `docs/Release-Checklist.md` against the exact candidate commit.
6. Confirm Liftoff performance and EdgeTX **Other Devices** discovery remain unchanged.
7. After the candidate has passed hardware testing, run:

   ```powershell
   .\tools\Stage-Release.ps1
   ```

8. Confirm the staged files exist under `dist/`:

   ```text
   ELRS-HID-Bridge-v1.0.0-rc1.uf2
   ELRS-HID-Bridge-v1.0.0-rc1.sha256.txt
   RELEASE-MANIFEST.txt
   ```

9. Do not tag/publish the release candidate until the checklist passes and the manifest refers to the exact tested commit.

## Release artifact rule

`Stage-Release.ps1` does not invoke PlatformIO. It copies the already-built `.pio/build/pico/firmware.uf2`, reads the canonical version from `src/firmware_version.h`, and generates release naming/hash metadata.

`dist/` is now ignored by Git so staged binaries are not accidentally committed.

## Suggested commit

```text
Prepare v1.0.0-rc1 release candidate
```
