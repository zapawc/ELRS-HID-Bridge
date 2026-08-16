param(
    [string]$Environment = "pico"
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$PioProjectDir = Join-Path $RepoRoot ".pio"
$OutputText = Join-Path $PioProjectDir "build-environment.txt"
$OutputJson = Join-Path $PioProjectDir "build-environment.json"

if (-not (Test-Path $PioProjectDir)) {
    New-Item -ItemType Directory -Path $PioProjectDir | Out-Null
}

function Get-PackageJsonInfo {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        return $null
    }

    try {
        $json = Get-Content -Raw -Path $Path | ConvertFrom-Json
        return [pscustomobject]@{
            Name    = $json.name
            Version = $json.version
            Path    = (Resolve-Path (Split-Path -Parent $Path)).Path
        }
    }
    catch {
        return $null
    }
}

function Get-GitHead {
    param([string]$Path)

    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        return $null
    }

    if (-not (Test-Path (Join-Path $Path ".git"))) {
        return $null
    }

    $head = & git -C $Path rev-parse HEAD 2>$null
    if ($LASTEXITCODE -eq 0 -and $head) {
        return ($head | Select-Object -First 1).Trim()
    }

    return $null
}

function Get-LibraryPropertiesVersion {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        return $null
    }

    $line = Get-Content -Path $Path | Where-Object { $_ -match '^version\s*=' } | Select-Object -First 1
    if ($line -match '^version\s*=\s*(.+)$') {
        return $Matches[1].Trim()
    }

    return $null
}

$PlatformioCoreDir = if ($env:PLATFORMIO_CORE_DIR) {
    $env:PLATFORMIO_CORE_DIR
}
else {
    Join-Path $HOME ".platformio"
}

$ProjectGitHead = Get-GitHead -Path $RepoRoot

# Locate the installed Raspberry Pi PlatformIO platform. A git-based platform
# install normally lives below ~/.platformio/platforms and may retain its .git
# metadata, which lets us capture the exact tested commit without invoking pio.
$PlatformInfo = $null
$PlatformsRoot = Join-Path $PlatformioCoreDir "platforms"
if (Test-Path $PlatformsRoot) {
    foreach ($dir in Get-ChildItem -Path $PlatformsRoot -Directory -ErrorAction SilentlyContinue) {
        $platformJson = Join-Path $dir.FullName "platform.json"
        if (-not (Test-Path $platformJson)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Path $platformJson | ConvertFrom-Json
        }
        catch {
            continue
        }

        $text = Get-Content -Raw -Path $platformJson
        if ($json.name -eq "raspberrypi" -or $text -match 'arduinopico') {
            $piopmUrl = $null
            $piopmRequirements = $null
            $piopmPath = Join-Path $dir.FullName ".piopm"
            if (Test-Path $piopmPath) {
                try {
                    $piopm = Get-Content -Raw -Path $piopmPath | ConvertFrom-Json
                    $piopmUrl = $piopm.spec.url
                    $piopmRequirements = $piopm.spec.requirements
                }
                catch {
                    # Optional metadata only; continue with platform.json/.git data.
                }
            }

            $PlatformInfo = [pscustomobject]@{
                Name             = $json.name
                Version          = $json.version
                Path             = $dir.FullName
                GitHead          = Get-GitHead -Path $dir.FullName
                PackageSpecUrl   = $piopmUrl
                Requirements     = $piopmRequirements
            }
            break
        }
    }
}

# Capture the installed Arduino-Pico framework and RP2040 toolchain package
# versions. These are useful cross-checks even when the platform git commit is
# the value ultimately pinned in platformio.ini.
$PackagesRoot = Join-Path $PlatformioCoreDir "packages"
$FrameworkInfo = Get-PackageJsonInfo -Path (Join-Path $PackagesRoot "framework-arduinopico\package.json")
$ToolchainInfo = Get-PackageJsonInfo -Path (Join-Path $PackagesRoot "toolchain-rp2040-earlephilhower\package.json")

# Find the NeoPixel library that was actually resolved for the requested project
# environment. PlatformIO registry packages normally expose library.properties.
$NeoPixelInfo = $null
$LibDepsRoot = Join-Path $PioProjectDir "libdeps\$Environment"
if (Test-Path $LibDepsRoot) {
    $properties = Get-ChildItem -Path $LibDepsRoot -Filter "library.properties" -File -Recurse -ErrorAction SilentlyContinue |
        Where-Object {
            (Get-Content -Raw -Path $_.FullName -ErrorAction SilentlyContinue) -match '(?im)^name\s*=\s*Adafruit NeoPixel\s*$'
        } |
        Select-Object -First 1

    if ($properties) {
        $libraryDir = Split-Path -Parent $properties.FullName
        $NeoPixelInfo = [pscustomobject]@{
            Name    = "Adafruit NeoPixel"
            Version = Get-LibraryPropertiesVersion -Path $properties.FullName
            Path    = $libraryDir
            GitHead = Get-GitHead -Path $libraryDir
        }
    }
}

$Capture = [ordered]@{
    CapturedAt          = (Get-Date).ToString("o")
    RepositoryRoot      = $RepoRoot
    ProjectGitHead      = $ProjectGitHead
    PlatformioCoreDir   = $PlatformioCoreDir
    Environment         = $Environment
    Platform            = $PlatformInfo
    ArduinoPicoFramework = $FrameworkInfo
    Rp2040Toolchain     = $ToolchainInfo
    AdafruitNeoPixel    = $NeoPixelInfo
}

$lines = @()
$lines += "ELRS-HID-Bridge known-good build environment capture"
$lines += "Captured: $($Capture.CapturedAt)"
$lines += "Environment: $Environment"
$lines += "Project Git HEAD: $(if ($ProjectGitHead) { $ProjectGitHead } else { '(not available)' })"
$lines += "PlatformIO core dir: $PlatformioCoreDir"
$lines += ""

if ($PlatformInfo) {
    $lines += "PlatformIO platform: $($PlatformInfo.Name)"
    $lines += "Platform version: $($PlatformInfo.Version)"
    $lines += "Platform path: $($PlatformInfo.Path)"
    $lines += "Platform git HEAD: $(if ($PlatformInfo.GitHead) { $PlatformInfo.GitHead } else { '(not available)' })"
    $lines += "Platform package spec URL: $(if ($PlatformInfo.PackageSpecUrl) { $PlatformInfo.PackageSpecUrl } else { '(not available)' })"
    $lines += "Platform package requirements: $(if ($PlatformInfo.Requirements) { $PlatformInfo.Requirements } else { '(not available)' })"
}
else {
    $lines += "PlatformIO platform: NOT FOUND"
}

$lines += ""
if ($FrameworkInfo) {
    $lines += "Arduino-Pico framework: $($FrameworkInfo.Version)"
    $lines += "Framework path: $($FrameworkInfo.Path)"
}
else {
    $lines += "Arduino-Pico framework: NOT FOUND"
}

$lines += ""
if ($ToolchainInfo) {
    $lines += "RP2040 toolchain: $($ToolchainInfo.Version)"
    $lines += "Toolchain path: $($ToolchainInfo.Path)"
}
else {
    $lines += "RP2040 toolchain: NOT FOUND"
}

$lines += ""
if ($NeoPixelInfo) {
    $lines += "Adafruit NeoPixel: $($NeoPixelInfo.Version)"
    $lines += "NeoPixel path: $($NeoPixelInfo.Path)"
    $lines += "NeoPixel git HEAD: $(if ($NeoPixelInfo.GitHead) { $NeoPixelInfo.GitHead } else { '(registry package / not available)' })"
}
else {
    $lines += "Adafruit NeoPixel: NOT FOUND for environment '$Environment'"
}

$lines | Set-Content -Path $OutputText -Encoding UTF8
$Capture | ConvertTo-Json -Depth 6 | Set-Content -Path $OutputJson -Encoding UTF8

$lines | ForEach-Object { Write-Host $_ }
Write-Host ""
Write-Host "Saved: $OutputText"
Write-Host "Saved: $OutputJson"
