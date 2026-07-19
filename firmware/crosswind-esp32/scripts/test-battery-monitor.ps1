$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$source = Get-Content -Raw (Join-Path $projectRoot 'src/battery_monitor.cpp')
$header = Get-Content -Raw (Join-Path $projectRoot 'src/battery_monitor.h')
$config = Get-Content -Raw (Join-Path $projectRoot 'src/config.h')
$main = Get-Content -Raw (Join-Path $projectRoot 'src/main.cpp')
$diagnostics = Get-Content -Raw (Join-Path $projectRoot 'src/diagnostics.cpp')
$platformio = Get-Content -Raw (Join-Path $projectRoot 'platformio.ini')

function Assert-Source([bool]$condition, [string]$message) {
    if (-not $condition) { throw "Battery source check failed: $message" }
}

Assert-Source (-not ($source -match '\bdelay\s*\(')) 'sampling must be non-blocking'
Assert-Source (-not ($source -match 'driveMotor|stopMotor|requestThrowerTrigger|cancelThrowerTrigger')) 'battery module must not control motor or trigger hardware'
Assert-Source (-not ($source -match 'Preferences|putFloat|putBool|putUChar')) 'battery sampling module must not write flash'
Assert-Source ($source -match 'analogReadMilliVolts') 'calibrated ESP32 millivolt API must be used'
Assert-Source ($source -match 'BATTERY_FILTER_ALPHA') 'EMA filtering must remain configured'
Assert-Source ($source -match 'BATTERY_STARTUP_SETTLE_MS' -and $source -match 'BATTERY_FILTER_MIN_SAMPLES') 'startup settling and sample warmup must be enforced'
Assert-Source ($source -match 'BATTERY_CRITICAL_CONFIRM_SAMPLES' -and $source -match 'BATTERY_RECOVERY_CONFIRM_SAMPLES') 'critical debounce and recovery hysteresis must be enforced'
Assert-Source ($header -match 'BATTERY_SENSOR_ERROR' -and $header -match 'BATTERY_RECOVERING') 'battery state model must retain error and recovery states'
Assert-Source ($config -match 'BATTERY_DIVIDER_UPPER_OHMS' -and $config -match 'BATTERY_DIVIDER_LOWER_OHMS') 'divider values must remain centralized'
Assert-Source ($config -match 'BATTERY_MONITOR_DEFAULT_ENABLED = false') 'uninstalled monitor must default disabled'
Assert-Source ($main -match 'FAULT_BATTERY_CRITICAL' -and $main -match 'batteryAllowsOperation') 'application must own critical fault and operation gating'
Assert-Source ($diagnostics -match 'batteryV=' -and $diagnostics -match 'batteryStatus=' -and $diagnostics -match 'batteryCalibrated=') 'BLE/Serial status must expose battery and calibration state'
Assert-Source ($platformio -match '\[env:esp32dev_battery_sim\]' -and $platformio -match 'CROSSWIND_BATTERY_SIMULATION=1') 'battery simulation environment must remain configured'

Write-Host 'Battery monitor source checks passed.'
