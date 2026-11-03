# Starts dkb.exe from build/dkb-probe with the settings this checkout needs (local paths; nothing here is game data).
#   pwsh game-build/dkb/run_dkb.ps1            window, play mode, keyboard input
#   pwsh game-build/dkb/run_dkb.ps1 -Headless  no window, 60 s, writes dkb_shot_*.png
param([switch]$Headless, [string]$Dump = $env:DKB_DUMP)   # folder holding exefs/ and romfs/ of the update, e.g. $env:DKB_DUMP = '<path>/updates/458752'
if (-not $Dump) { throw 'set DKB_DUMP (or pass -Dump) to the folder that holds exefs and romfs' }
$root = Resolve-Path (Join-Path $PSScriptRoot '../..')
Set-Location (Join-Path $root 'build/dkb-probe')
Get-Process dkb -ErrorAction SilentlyContinue | Stop-Process -Force
$env:NXRT_FS_ROMFS = "$Dump/romfs"                                   # read in place, never copied
$env:NXRT_NVN_SHADER_CACHE = Join-Path $root 'work/shaders/dkb_spirv_full'   # translated shaders (tools/shaders/scripts/dkb_extract_bnsh.py + spirv_harness)
if (-not (Test-Path "$env:NXRT_NVN_SHADER_CACHE/results.tsv")) { throw "shader cache missing: $env:NXRT_NVN_SHADER_CACHE" }
Remove-Item controls.ini -ErrorAction SilentlyContinue                      # rewritten from dkb_defaults.cpp
$exe = @('--exefs', "$Dump/exefs", '--no-pads')
if ($Headless) { $exe += @('--max-seconds', '60', '--screenshot-at', '20,45') } else { $exe += '--window' }
& .\dkb.exe @exe
