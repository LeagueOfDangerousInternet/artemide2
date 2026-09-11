$ErrorActionPreference = 'Stop'
$toolDir = Join-Path $PSScriptRoot '.tools'
New-Item -ItemType Directory -Force $toolDir | Out-Null
$cli = Join-Path $toolDir 'arduino-cli.exe'
if (!(Test-Path -LiteralPath $cli)) {
  $archive = Join-Path $toolDir 'arduino-cli.zip'
  Invoke-WebRequest 'https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_64bit.zip' -OutFile $archive
  Expand-Archive -LiteralPath $archive -DestinationPath $toolDir -Force
}
$env:ARDUINO_DIRECTORIES_DATA = Join-Path $toolDir 'data'
$env:ARDUINO_DIRECTORIES_DOWNLOADS = Join-Path $toolDir 'downloads'
$env:ARDUINO_DIRECTORIES_USER = Join-Path $toolDir 'user'
& $cli core update-index
if ($LASTEXITCODE -ne 0) { throw 'Index update failed.' }
& $cli core install arduino:avr@1.8.8
if ($LASTEXITCODE -ne 0) { throw 'AVR core installation failed.' }
& $cli lib install Arduboy2
if ($LASTEXITCODE -ne 0) { throw 'Arduboy2 installation failed.' }
& "$PSScriptRoot/build.ps1"
