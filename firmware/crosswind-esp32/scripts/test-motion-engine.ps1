$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$motion = Get-Content -Raw (Join-Path $projectRoot 'src/motion.cpp')
$modes = Get-Content -Raw (Join-Path $projectRoot 'src/modes.cpp')
$config = Get-Content -Raw (Join-Path $projectRoot 'src/config.h')
$platformio = Get-Content -Raw (Join-Path $projectRoot 'platformio.ini')

function Assert-Source([bool]$condition, [string]$message) {
    if (-not $condition) {
        throw "Motion source check failed: $message"
    }
}

Assert-Source (-not (($motion + $modes) -match '\bdelay\s*\(')) 'motion sequencing must not use delay()'
Assert-Source (-not ($motion -match 'requestThrowerTrigger|writeTriggerRelay')) 'motion engine must not bypass the trigger event interface'
Assert-Source ($motion -match 'MOTION_EVENT_TRIGGER_REQUESTED') 'FLUSH must use a trigger-request event'
Assert-Source ($motion -match 'FAULT_BOTH_LIMITS') 'both-limits fault must be enforced'
Assert-Source ($motion -match 'FAULT_LIMIT_STUCK') 'stuck-limit fault must be enforced'
Assert-Source ($motion -match 'FAULT_UNEXPECTED_LIMIT') 'unexpected-limit fault must be enforced'
Assert-Source ($motion -match 'FAULT_TRAVEL_TIMEOUT') 'travel timeout must be enforced'
Assert-Source ($motion -match 'MIN_VALID_CALIBRATION_TIME_MS' -and $motion -match 'MAX_VALID_CALIBRATION_TIME_MS') 'calibration must validate its measurement'
Assert-Source ($motion -match 'MOTION_FIXED_RANDOM_SEED') 'fixed random seed support must remain available'
Assert-Source ($config -match 'RANDOM_SPEED_MIN_PWM' -and $config -match 'RANDOM_SPEED_MAX_PWM') 'random speed bounds must remain centralized'
Assert-Source ($config -match 'RANDOM_MOVE_MIN_MS' -and $config -match 'RANDOM_MOVE_MAX_MS') 'random movement bounds must remain centralized'
Assert-Source ($platformio -match '\[env:esp32dev_motion_sim\]') 'simulation build environment must remain configured'
Assert-Source ($platformio -match 'CROSSWIND_MOTION_SIMULATION=1') 'simulation build must explicitly enable safe simulation'

Write-Host 'Motion engine source checks passed.'
