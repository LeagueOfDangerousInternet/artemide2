$ErrorActionPreference = 'Stop'
$env:ARDUINO_DIRECTORIES_DATA = Join-Path $PSScriptRoot '.tools/data'
$env:ARDUINO_DIRECTORIES_DOWNLOADS = Join-Path $PSScriptRoot '.tools/downloads'
$env:ARDUINO_DIRECTORIES_USER = Join-Path $PSScriptRoot '.tools/user'
$cli = Join-Path $PSScriptRoot '.tools/arduino-cli.exe'
& $cli compile --fqbn arduino:avr:leonardo --warnings all --build-path "$PSScriptRoot/.build" --output-dir "$PSScriptRoot/build" "$PSScriptRoot/Starter"
if ($LASTEXITCODE -ne 0) { throw 'Arduino compilation failed.' }
Write-Host "Ready for ProjectABE: $PSScriptRoot/build/Starter.ino.hex"
