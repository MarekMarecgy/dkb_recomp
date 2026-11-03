@echo off
rem Sets up the MSVC + LLVM + CMake + Ninja environment for the runtime build (call it, then run cmake/ninja).
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    set "NX_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    call :find_vs
    if errorlevel 1 exit /b 1
)
if not defined LLVM_BIN if exist "%~dp0..\..\build\toolchain\clang+llvm-23.1.4-x86_64-pc-windows-msvc\bin\clang-cl.exe" set "LLVM_BIN=%~dp0..\..\build\toolchain\clang+llvm-23.1.4-x86_64-pc-windows-msvc\bin"
if not defined LLVM_BIN (
    if exist "C:\Program Files\LLVM\bin\clang-cl.exe" (
        set "LLVM_BIN=C:\Program Files\LLVM\bin"
    ) else (
        set "LLVM_BIN=%VCINSTALLDIR%Tools\Llvm\x64\bin"
    )
)
if not exist "%LLVM_BIN%\clang-cl.exe" (
    echo LLVM clang-cl not found. Set LLVM_BIN to its bin directory.
    exit /b 1
)
set "PATH=%LLVM_BIN%;C:\Program Files\CMake\bin;%LOCALAPPDATA%\Microsoft\WinGet\Links;%PATH%"
if exist "%VSINSTALLDIR%Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" set "PATH=%VSINSTALLDIR%Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
exit /b 0

:find_vs
if not exist "%NX_VSWHERE%" (
    echo Visual Studio Installer vswhere.exe not found.
    exit /b 1
)
for /f "usebackq delims=" %%I in (`"%NX_VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "NX_VS=%%I"
if not defined NX_VS (
    echo Visual Studio C++ Build Tools not found.
    exit /b 1
)
call "%NX_VS%\VC\Auxiliary\Build\vcvars64.bat" >nul
exit /b %errorlevel%
