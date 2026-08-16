# v1.0.0 Final Release Promotion Checkpoint

This package advances ELRS-HID-Bridge from the published `v1.0.0-rc1` source identity to final `1.0.0` validation.

## Replace these files

```text
src/firmware_version.h
src/firmware_version_self_test.cpp
README.md
CHANGELOG.md
docs/Release.md
docs/Release-Checklist.md
```

## Add this file

```text
docs/Release-Notes-v1.0.0.md
```

## Runtime scope

No CRSF parsing, channel decoding, HID mapping, failsafe behavior, USB descriptors, BOOT behavior, LED behavior, device-discovery behavior, dependency pins, or release-staging code is intentionally changed.

The release identity changes from:

```text
1.0.0-rc1
```

to:

```text
1.0.0
```

The CRSF Firmware ID intentionally remains:

```text
0x01000000
```

because prerelease labels are not encoded into the numeric CRSF firmware field.

## Apply

Copy the package contents over the repository root while preserving the included directory structure.

Do not copy `CHANGESET.md` into the repository unless you want to retain it as local release-working documentation.

## Build / validation gate

1. Use the normal VS Code PlatformIO workflow.
2. Select/build the `pico` environment only.
3. Confirm the build completes with no project errors/VS Code Problems.
4. Flash the resulting final-source firmware.
5. Complete `docs/Release-Checklist.md` from top to bottom.
6. Do not tag or publish `v1.0.0` until every release-blocking item passes.
7. After validation, run `tools/Stage-Release.ps1` to stage a newly built final UF2/hash/manifest.
8. Verify the staged manifest identifies version `1.0.0` and the exact tested Git commit.

Do **not** rename the RC1 UF2 as the final artifact.

## Suggested commit message

```text
Promote release identity to v1.0.0
```
