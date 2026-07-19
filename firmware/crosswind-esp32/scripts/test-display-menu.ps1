$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$menu = Get-Content -Raw (Join-Path $root 'src/menu.cpp')
$display = Get-Content -Raw (Join-Path $root 'src/display.cpp')
$inputs = Get-Content -Raw (Join-Path $root 'src/inputs.cpp')
$main = Get-Content -Raw (Join-Path $root 'src/main.cpp')
$storage = Get-Content -Raw (Join-Path $root 'src/storage.cpp')
$platformio = Get-Content -Raw (Join-Path $root 'platformio.ini')

function Assert-Source([bool]$condition, [string]$message) {
    if (-not $condition) { throw "Display/menu source check failed: $message" }
}

Assert-Source (-not (($menu + $display) -match '\bdelay\s*\(')) 'UI behavior must be non-blocking'
Assert-Source (-not ($menu -match 'driveMotor|stopMotor|requestThrowerTrigger|digitalWrite|Preferences')) 'menu must emit intents rather than control hardware or storage'
Assert-Source ($inputs -match 'UI_LONG_PRESS_MS' -and $inputs -match 'consumeMenuLongPressed') 'encoder long press must be supported'
Assert-Source ($menu -match 'UI_EDIT_VALUE' -and $menu -match 'editOriginal') 'working-copy edits and cancellation must exist'
Assert-Source ($menu -match 'UI_CONFIRM' -and $menu -match 'CONFIRM_MOTION_CALIBRATION') 'movement calibration must require confirmation'
Assert-Source ($menu -match 'CONFIRM_RESTORE_DEFAULTS') 'restore defaults must require confirmation'
Assert-Source ($menu -match 'controller.faultActive' -and $menu -match 'UI_FAULT') 'fault state must override normal pages'
Assert-Source ($menu -match 'UI_OFF_TIMEOUT_MS' -and $menu -match 'UI_DIM_TIMEOUT_MS') 'dim/off timeout behavior must remain configured'
Assert-Source ($main -match 'processUiIntent' -and $main -match 'movementSafeToStart') 'application must retain intent and motion safety authority'
Assert-Source ($main -match 'consumeArmPressed' -and $main -match 'consumeFirePressed') 'dedicated ARM and FIRE paths must remain present'
Assert-Source ($storage -match 'saveUiSettings' -and $storage -notmatch 'put.*ui.*update') 'UI persistence must remain explicit'
Assert-Source ($platformio -match '\[env:esp32dev_ui_sim\]' -and $platformio -match 'CROSSWIND_UI_SIMULATION=1') 'UI simulation environment must remain configured'
Assert-Source ($display -match 'renderBoot' -and $display -match 'renderStatus' -and $display -match 'renderFault') 'priority screens must remain implemented'
Assert-Source ($display -match 'bleClientConnected') 'BLE state must be visible locally'

Write-Host 'Display/menu source checks passed.'
