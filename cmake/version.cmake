################################################################################
# Stepper-C: Stepper Motor Control Implementation
#
# Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
# Description: Extracts the library version from the latest Git tag (vX.Y.Z)
#              and exposes it as SSTEP_VERSION_* compile definitions.
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
# DEFAULT VERSION
# ==============================================================================

# Used when the source tree has no Git metadata or no vX.Y.Z tag yet; keep it in
# sync with the fallback values in sstep.h.
set(SSTEP_DEFAULT_VERSION "1.0.0")

# ==============================================================================
# VERSION EXTRACTION
# ==============================================================================

find_package(Git QUIET)

set(SSTEP_GIT_TAG "")
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
        RESULT_VARIABLE SSTEP_GIT_DESCRIBE_RESULT
        ERROR_VARIABLE SSTEP_GIT_DESCRIBE_ERROR
        OUTPUT_VARIABLE SSTEP_GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
endif()

# ==============================================================================
# VERSION PARSING
# ==============================================================================

string(REGEX MATCH "^[Vv]([0-9]+)\\.([0-9]+)\\.([0-9]+)$" SSTEP_VERSION_MATCH "${SSTEP_GIT_TAG}")

if(NOT SSTEP_VERSION_MATCH)
    message(WARNING "No vX.Y.Z Git tag found (${SSTEP_GIT_DESCRIBE_ERROR}); "
                    "using default version ${SSTEP_DEFAULT_VERSION}")
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" SSTEP_VERSION_MATCH "${SSTEP_DEFAULT_VERSION}")
endif()

# ==============================================================================
# VERSION COMPONENTS
# ==============================================================================

set(SSTEP_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(SSTEP_VERSION_MINOR "${CMAKE_MATCH_2}")
set(SSTEP_VERSION_PATCH "${CMAKE_MATCH_3}")

set(SSTEP_VERSION "${SSTEP_VERSION_MAJOR}.${SSTEP_VERSION_MINOR}.${SSTEP_VERSION_PATCH}")

message(STATUS "SSTEP library is version: ${SSTEP_VERSION}")

# ==============================================================================
# CONFIGURE VERSION DEFINITIONS
# ==============================================================================

list(APPEND SSTEP_COMPILE_DEFINITIONS
    SSTEP_VERSION_MAJOR=${SSTEP_VERSION_MAJOR}
    SSTEP_VERSION_MINOR=${SSTEP_VERSION_MINOR}
    SSTEP_VERSION_PATCH=${SSTEP_VERSION_PATCH}
    SSTEP_VERSION="${SSTEP_VERSION}"
)
