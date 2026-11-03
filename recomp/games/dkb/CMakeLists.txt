# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.25)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(dkb_lift C CXX)
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_VERSION VERSION_LESS 23)
  message(FATAL_ERROR "DKB builds require LLVM 23 or newer; LLVM 19 crashes while compiling this module")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/NxRecomp.cmake")

set(DKB_EXEFS "" CACHE PATH "Existing DKB update 458752 ExeFS directory (read only)")
if(NOT IS_DIRECTORY "${DKB_EXEFS}")
  message(FATAL_ERROR "Set DKB_EXEFS to the existing extracted DKB update 458752 directory")
endif()
execute_process(COMMAND "${NX_PYTHON}" "${CMAKE_CURRENT_LIST_DIR}/verify_inputs.py" "${DKB_EXEFS}"
                RESULT_VARIABLE _dkb_check)
if(NOT _dkb_check EQUAL 0)
  message(FATAL_ERROR "DKB input verification failed")
endif()

set(DKB_SHARDS_MAIN 160 CACHE STRING "Translation units of main")
set(DKB_SHARDS_SDK 48 CACHE STRING "Translation units of sdk")
nxrecomp_add_module(dkb_main INPUT "${DKB_EXEFS}/main" NAME main INDEX 0 BASE 0x7100000000
                    GAME dkb SHARDS ${DKB_SHARDS_MAIN} JOBS 4)
nxrecomp_add_module(dkb_sdk INPUT "${DKB_EXEFS}/sdk" NAME sdk INDEX 1 BASE 0x7500000000
                    GAME dkb SHARDS ${DKB_SHARDS_SDK} JOBS 4)
nxrecomp_add_module(dkb_rtld INPUT "${DKB_EXEFS}/rtld" NAME rtld INDEX 2 BASE 0x7700000000
                    GAME dkb SHARDS 1 JOBS 1)
add_custom_target(dkb_modules DEPENDS dkb_main dkb_sdk dkb_rtld)
