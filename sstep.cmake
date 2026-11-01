################################################################################
# Stepper-C: Stepper Motor Control Implementation
#
# Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
# Description: Library target definition. Can be included directly by an
#              integrating project: include(path/to/sstep.cmake)
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# References:
#   - MISRA C:2012 Guidelines for the use of the C language in critical systems
#   - CMake Documentation: https://cmake.org/documentation/
#
# Last Updated: October 2026
# SPDX-License-Identifier: Apache-2.0
#
################################################################################

# ==============================================================================
# CMAKE CONFIGURATION
# ==============================================================================

list(APPEND CMAKE_MODULE_PATH

    ${CMAKE_CURRENT_LIST_DIR}/cmake
)

# ==============================================================================
# VERSION EXTRACTION
# ==============================================================================

include(version) # Include the version extraction and parsing logic

# ==============================================================================
# INCLUDE DIRECTORIES
# ==============================================================================

list(APPEND SSTEP_HEADERS

    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# ==============================================================================
# SOURCE FILES
# ==============================================================================

list(APPEND SSTEP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/sstep_math.c
    ${CMAKE_CURRENT_LIST_DIR}/src/sstep_sin_table.c
    ${CMAKE_CURRENT_LIST_DIR}/src/sstep_bridge.c
    ${CMAKE_CURRENT_LIST_DIR}/src/sstep_motion.c
    ${CMAKE_CURRENT_LIST_DIR}/src/sstep_stall.c
    ${CMAKE_CURRENT_LIST_DIR}/src/sstep_ctrl.c
)

set_source_files_properties(${SSTEP_SOURCES} PROPERTIES LANGUAGE C)

# Library sources only (SSTEP_SOURCES also receives the test sources below)
set(SSTEP_LIBRARY_SOURCES ${SSTEP_SOURCES})

# ==============================================================================
# BUILD-TIME CONFIGURATION (see inc/sstep_config.h)
# ==============================================================================

# Numeric backend: FIXED (Q15, no FPU needed) or FLOAT (single precision, FPU)
set(SSTEP_MATH_BACKEND "FIXED" CACHE STRING "Stepper-C numeric backend: FIXED or FLOAT")
set_property(CACHE SSTEP_MATH_BACKEND PROPERTY STRINGS FIXED FLOAT)
if(NOT SSTEP_MATH_BACKEND MATCHES "^(FIXED|FLOAT)$")
    message(FATAL_ERROR "SSTEP_MATH_BACKEND must be FIXED or FLOAT, got '${SSTEP_MATH_BACKEND}'")
endif()
list(APPEND SSTEP_COMPILE_DEFINITIONS "SSTEP_CFG_MATH_BACKEND=SSTEP_MATH_BACKEND_${SSTEP_MATH_BACKEND}")

# Optional folder with sstep_user_config.h (overrides the SSTEP_CFG_* defaults)
set(SSTEP_CONFIG_DIR "" CACHE PATH "Folder containing sstep_user_config.h (optional)")
if(SSTEP_CONFIG_DIR)
    list(APPEND SSTEP_HEADERS ${SSTEP_CONFIG_DIR})
    list(APPEND SSTEP_COMPILE_DEFINITIONS "SSTEP_USER_CONFIG")
endif()

# ==============================================================================
# PROJECT CONFIGURATION
# ==============================================================================

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND SSTEP_COMPILE_DEFINITIONS "SSTEP_DEBUG")
endif()

set(SSTEP_LIBRARY_TYPE "STATIC")        # Default to static library, can be overridden by CMake presets
set(SSTEP_LIBRARY_INTERFACE "PUBLIC")   # Interface scope for consumers of the library

# ==============================================================================
# UNIT TESTS
# ==============================================================================

if(SSTEP_UNIT_TESTS)

    list(REMOVE_ITEM SSTEP_SOURCES
        # Add any source files that should be excluded from the main library when unit tests are enabled
    )

    list(APPEND SSTEP_UNIT_TEST_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_sstep_math.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_sstep_bridge.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_sstep_motion.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_sstep_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_sstep_loop.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit/gtest_sstep_stall.cpp
        # C compile check: sstep.h alone, no private header (#error otherwise)
        ${CMAKE_CURRENT_LIST_DIR}/tests/compile/sstep_api_check.c
    )

    list(APPEND SSTEP_UNIT_TEST_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/tests/unit
        ${CMAKE_CURRENT_LIST_DIR}/src
    )

    list(APPEND SSTEP_HEADERS ${SSTEP_UNIT_TEST_HEADERS})
    list(APPEND SSTEP_SOURCES ${SSTEP_UNIT_TEST_SOURCES})

    set(SSTEP_LIBRARY_TYPE "INTERFACE")         # Sources are compiled into the test executable (coverage)
    set(SSTEP_LIBRARY_INTERFACE "INTERFACE")    # Consumers (the test executable) link against this target

endif()

# ==============================================================================
# LIBRARY TARGET
# ==============================================================================

# Without sources the target would be header-only
if(NOT SSTEP_SOURCES)
    set(SSTEP_LIBRARY_TYPE "INTERFACE")
    set(SSTEP_LIBRARY_INTERFACE "INTERFACE")
endif()

set(SSTEP_C "sstep")
add_library(${SSTEP_C} ${SSTEP_LIBRARY_TYPE} ${SSTEP_SOURCES})
target_include_directories(${SSTEP_C} ${SSTEP_LIBRARY_INTERFACE} ${SSTEP_HEADERS})
target_compile_definitions(${SSTEP_C} ${SSTEP_LIBRARY_INTERFACE} ${SSTEP_COMPILE_DEFINITIONS})

if(SSTEP_COMPILE_OPTIONS) # Add any additional compile options if defined
    target_compile_options(${SSTEP_C} ${SSTEP_LIBRARY_INTERFACE} ${SSTEP_COMPILE_OPTIONS})
endif()
