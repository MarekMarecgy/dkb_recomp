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
if (-not $env:NXRT_NVN_SKIP_GS_HASHES) { $env:NXRT_NVN_SKIP_GS_HASHES = '498d2702' }   # this geometry shader loses the GPU (nvlddmkm 153); see docs/BUILD.md
# the translated NDSR upscaler compute shaders (HMMA emulation) still lose the GPU: keep the blit stand-in until they are fixed. Set NXRT_NVN_SKIP_CS_HASHES='' (empty is ignored: use 'none') to try them
if (-not $env:NXRT_NVN_SKIP_CS_HASHES) { $env:NXRT_NVN_SKIP_CS_HASHES = '02bca801 14bc7873 2f0be2c8 49e52831 4d8437fc 5a8d2482 632c8af8 635823b1 7105a13f 71d425ed 8569d67d 91d02b1b 983fc014 9d40136b 9e1b2285 a9863b38 ac772b6c c07db2eb c55b55ac ca23b96d cbe6ab44 d94a1a0b e2f8d329 e92f908e efc55988 f1ef901e fb05ae6b' }
$env:NXRT_DIAG_SOAK_CSV = 'dkb_soak.jsonl'                                 # gauge sampler: vma.device_usage_mb, vk.memory_bytes, ... one JSON line per interval
$env:NXRT_DIAG_SOAK_INTERVAL_S = '2'
Remove-Item dkb_soak.jsonl -ErrorAction SilentlyContinue
Remove-Item controls.ini -ErrorAction SilentlyContinue                      # rewritten from dkb_defaults.cpp
$exe = @('--exefs', "$Dump/exefs", '--no-pads')
if ($Headless) { $exe += @('--max-seconds', '60', '--screenshot-at', '20,45') } else { $exe += '--window' }
if ($env:DKB_SHOTS) { $exe += @('--screenshot-at', $env:DKB_SHOTS) }                      # e.g. '70,85' (seconds): screenshots of the window run
& .\dkb.exe @exe
