# Configure + build (+ test) the runtime. Run from anywhere:   pwsh scripts\build.ps1 [-Test] [-Config Release] [-Clean] [-Filter <regex>]
# Works relative to the checkout the script lives in, so git worktrees build on their own. Output: <checkout>\build\<config>
param([ValidateSet('Release', 'Debug', 'RelWithDebInfo')][string]$Config = "Release", [switch]$Test,
      [switch]$Clean, [string]$Filter = "", [switch]$Verbose, [switch]$CoreOnly,
      [ValidateRange(1, 64)][int]$Jobs = 8)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$bd = [IO.Path]::GetFullPath((Join-Path $root ("build\" + $Config.ToLower() + $(if ($CoreOnly) { '-core' } else { '' }))))
if ($Clean -and (Test-Path -LiteralPath $bd)) {
    $buildRoot = [IO.Path]::GetFullPath((Join-Path $root 'build')) + [IO.Path]::DirectorySeparatorChar
    if (-not $bd.StartsWith($buildRoot, [StringComparison]::OrdinalIgnoreCase)) { throw 'Build path is outside runtime/build' }
    Remove-Item -LiteralPath $bd -Recurse -Force
}
$env_cmd = Join-Path $PSScriptRoot "env.cmd"
$cfg = "cmake -S `"$root`" -B `"$bd`" -G Ninja -DCMAKE_BUILD_TYPE=$Config -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DNXRT_GAMES=pkp -DNXRT_TEST_GAME=pkp"
if ($CoreOnly) { $cfg += ' -DNXRT_BUILD_GFX=OFF' } else { $cfg += ' -DNXRT_BUILD_GFX=ON' }
$bld = "cmake --build `"$bd`" --parallel $Jobs"
if ($Verbose) { $bld += " --verbose" }
$cmd = "call `"$env_cmd`" && $cfg && $bld"
if ($Test) {
    $cmd += " && ctest --test-dir `"$bd`" --output-on-failure"
    if ($Filter) {
        if ($Filter -match '["%\r\n]') { throw 'Unsupported characters in test filter' }
        $cmd += " -R `"$Filter`""
    }
}
cmd /c $cmd
exit $LASTEXITCODE
