@echo off
rem Sets up the build environment: MSVC x64 (vcvars64), LLVM clang-cl 23, CMake, Ninja, sccache.
rem Usage from cmd:  scripts\nx_env.cmd cmake --build build\x
rem                  scripts\nx_env.cmd            (no arguments: just prints the tool versions)
call "%~dp0..\..\runtime\scripts\env.cmd"
if errorlevel 1 exit /b 1
set "PATH=%~dp0..\..\tools\.venv\Scripts;%PATH%"
if "%~1"=="" (
  clang-cl --version & cmake --version & ninja --version
  where sccache >nul 2>&1 && sccache --version
  exit /b 0
)
%*
