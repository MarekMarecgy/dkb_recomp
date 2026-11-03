# SPDX-License-Identifier: GPL-3.0-or-later
# Toolchain file: LLVM clang-cl (C:\Program Files\LLVM\bin) with the MSVC x64 environment (run through scripts/nx_env.cmd,
# or from a shell where vcvars64.bat has run), Ninja, optional sccache.
set(CMAKE_SYSTEM_NAME Windows)
find_program(NX_CLANG_CL clang-cl HINTS "$ENV{LLVM_BIN}" "C:/Program Files/LLVM/bin" REQUIRED)
find_program(NX_LLD_LINK lld-link HINTS "$ENV{LLVM_BIN}" "C:/Program Files/LLVM/bin" REQUIRED)
set(CMAKE_C_COMPILER "${NX_CLANG_CL}")
set(CMAKE_CXX_COMPILER "${NX_CLANG_CL}")
set(CMAKE_LINKER "${NX_LLD_LINK}")
find_program(NX_LLVM_RC llvm-rc HINTS "$ENV{LLVM_BIN}" "C:/Program Files/LLVM/bin")
find_program(NX_LLVM_MT llvm-mt HINTS "$ENV{LLVM_BIN}" "C:/Program Files/LLVM/bin")
if(NX_LLVM_RC)
  set(CMAKE_RC_COMPILER "${NX_LLVM_RC}")
endif()
if(NX_LLVM_MT)
  set(CMAKE_MT "${NX_LLVM_MT}")
endif()
# static MSVC runtime (/MT): the lifted libraries and the runtime that links them must agree, and players then need no
# Visual C++ Redistributable (CMake needs policy CMP0091 NEW, i.e. cmake_minimum_required 3.15 or later, for this to apply)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# the debug info format of an exe built here is not useful to the oracles: keep objects small
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "")
if(NOT DEFINED ENV{NX_NO_SCCACHE})
  find_program(NX_SCCACHE sccache HINTS "$ENV{LOCALAPPDATA}/Microsoft/WinGet/Links")
  if(NX_SCCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER "${NX_SCCACHE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${NX_SCCACHE}")
  endif()
endif()
