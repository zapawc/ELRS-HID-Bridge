[CmdletBinding()]
param(
    [string]$Environment = "pico",
    [string]$OutputDirectory = "dist"
)

$ErrorActionPreference = "Stop"

$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$VersionHeader = Join-Path $RepositoryRoot "src\firmware_version.h"
$FirmwareSource = Join-Path $RepositoryRoot ".pio\build\$Environment\firmware.uf2"
$OutputRoot = Join-Path $RepositoryRoot $OutputDirectory

if (-not (Test-Path $VersionHeader)) {
    throw "Canonical firmware version header not found: $VersionHeader"
}

$VersionMatch = Select-String `
    -Path $VersionHeader `
    -Pattern 'constexpr\s+const\s+char\*\s+STRING\s*=\s*"([^"]+)"' |
    Select-Object -First 1

if (-not $VersionMatch) {
    throw "Unable to read FirmwareVersion::STRING from $VersionHeader"
}

$Version = $VersionMatch.Matches[0].Groups[1].Value

if ([string]::IsNullOrWhiteSpace($Version)) {
    throw "Canonical firmware version is empty."
}

if (-not (Test-Path $FirmwareSource)) {
    throw @"
Release UF2 not found:
  $FirmwareSource

Build the normal '$Environment' environment using the VS Code PlatformIO Build control, then run this script again.
This script intentionally does not invoke PlatformIO or rebuild firmware.
"@
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$BaseName = "ELRS-HID-Bridge-v$Version"
$StagedFirmware = Join-Path $OutputRoot "$BaseName.uf2"
$HashFile = Join-Path $OutputRoot "$BaseName.sha256.txt"
$ManifestFile = Join-Path $OutputRoot "RELEASE-MANIFEST.txt"

Copy-Item -Path $FirmwareSource -Destination $StagedFirmware -Force

$Hash = (Get-FileHash -Path $StagedFirmware -Algorithm SHA256).Hash.ToLowerInvariant()
$StagedFileName = Split-Path $StagedFirmware -Leaf

"$Hash  $StagedFileName" | Set-Content -Path $HashFile -Encoding ascii

$GitHead = "unavailable"
$Git = Get-Command git -ErrorAction SilentlyContinue

if ($Git) {
    try {
        $GitResult = (& git -C $RepositoryRoot rev-parse HEAD 2>$null)

        if ($LASTEXITCODE -eq 0 -and $GitResult) {
            $GitHead = ($GitResult | Select-Object -First 1).Trim()
        }
    }
    catch {
        $GitHead = "unavailable"
    }
}

$SourceRelative = ".pio/build/$Environment/firmware.uf2"
$CapturedAt = (Get-Date).ToString("o")

@"
ELRS-HID-Bridge release manifest
Version: $Version
Environment: $Environment
Git commit: $GitHead
Source artifact: $SourceRelative
Staged artifact: $StagedFileName
SHA256: $Hash
Staged at: $CapturedAt
"@ | Set-Content -Path $ManifestFile -Encoding ascii

Write-Host "ELRS-HID-Bridge release assets staged successfully."
Write-Host ""
Write-Host "Version:  $Version"
Write-Host "Firmware: $StagedFirmware"
Write-Host "SHA256:   $HashFile"
Write-Host "Manifest: $ManifestFile"
Write-Host ""
Write-Host "This script copied the existing build artifact; it did not rebuild firmware."
