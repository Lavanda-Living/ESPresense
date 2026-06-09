# ESPresense wireless OTA upload to multiple devices.
# Requires Arduino OTA enabled on each ESP (web UI or HA Configuration switch).
#
# Usage:
#   .\ota-upload.ps1
#   .\ota-upload.ps1 -Rooms bedroom,office
#   .\ota-upload.ps1 -SkipBuild
#   .\ota-upload.ps1 -Targets 192.168.1.42,192.168.1.43
#
# Hostnames follow ESPresense convention: espresense-<room>.local
# (room name = value from each node's web UI / HA device name)

param(
    [string[]]$Rooms = @("bedroom", "office"),
    [string[]]$Targets,
    [string]$Environment = "esp32c3-cdc",
    [int]$OtaPort = 3232,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$RepoRoot = $PSScriptRoot
Set-Location $RepoRoot

function Get-EspOtaTool {
    $path = Join-Path $env:USERPROFILE ".platformio\packages\framework-arduinoespressif32\tools\espota.py"
    if (Test-Path $path) { return $path }

    $found = Get-ChildItem (Join-Path $env:USERPROFILE ".platformio\packages") -Recurse -Filter "espota.py" -ErrorAction SilentlyContinue |
        Select-Object -First 1 -ExpandProperty FullName
    if ($found) { return $found }

    throw "espota.py not found. Run a USB flash once so PlatformIO installs the ESP32 framework."
}

function Resolve-OtaHost {
    param([string]$Name)

    if ($Name -match '^\d+\.\d+\.\d+\.\d+$') { return $Name }

    try {
        $addrs = [System.Net.Dns]::GetHostAddresses($Name)
        $ipv4 = $addrs | Where-Object { $_.AddressFamily -eq 'InterNetwork' } | Select-Object -First 1
        if ($ipv4) { return $ipv4.IPAddressToString }
    } catch {
        # fall through
    }

    return $Name
}

if ($Targets) {
    $UploadHosts = $Targets
} else {
    $UploadHosts = $Rooms | ForEach-Object { "espresense-$_.local" }
}

$espota = Get-EspOtaTool
$firmware = Join-Path $RepoRoot ".pio\build\$Environment\firmware.bin"

Write-Host "ESPresense OTA upload"
Write-Host "  Environment: $Environment"
Write-Host "  Firmware:    $firmware"
Write-Host "  Targets:     $($UploadHosts -join ', ')"
Write-Host ""

if (-not $SkipBuild) {
    Write-Host "Building firmware..."
    python -m platformio run -e $Environment
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed."
        exit 1
    }
    Write-Host ""
}

if (-not (Test-Path $firmware)) {
    Write-Error "Firmware not found: $firmware (build first or drop -SkipBuild)"
    exit 1
}

$failed = @()
foreach ($uploadHost in $UploadHosts) {
    $ip = Resolve-OtaHost $uploadHost
    Write-Host "Uploading to $uploadHost ($ip) ..."

    python $espota -i $ip -p $OtaPort -f $firmware
    if ($LASTEXITCODE -ne 0) {
        $failed += $uploadHost
        Write-Warning "Upload failed: $uploadHost"
    } else {
        Write-Host "OK: $uploadHost"
    }
    Write-Host ""
}

if ($failed.Count -gt 0) {
    Write-Error "Failed uploads: $($failed -join ', ')"
    Write-Host "Tips:"
    Write-Host "  - Enable Arduino OTA on each ESP"
    Write-Host "  - PC must be on the same LAN as the devices"
    Write-Host "  - If .local fails on Windows, use IPs: .\ota-upload.ps1 -Targets 192.168.x.x,192.168.x.y"
    exit 1
}

Write-Host "All uploads completed."
