################################################################################
# Stepper-C: Stepper Motor Control Implementation
#
# Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
# Description: CMake configuration for GoogleTest unit tests and coverage
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

enable_language(CXX)

find_package(GTest REQUIRED)

if(NOT GTest_FOUND)
    message(FATAL_ERROR "Google Test not found. Please install Google Test to build unit tests.")
endif()

# ==============================================================================
# PROJECT CONFIGURATION
# ==============================================================================

include(GoogleTest)
include(CTest)

# ==============================================================================
# TESTING
# ==============================================================================

enable_testing()

set(SSTEP_GTEST ${CMAKE_PROJECT_NAME}_gtest)

add_executable(${SSTEP_GTEST} ${SSTEP_SOURCES})

set_target_properties(${SSTEP_GTEST} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    )

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${SSTEP_GTEST} PRIVATE -O0 -g --coverage)
    target_link_options(${SSTEP_GTEST} PRIVATE --coverage)
endif()

target_link_libraries(${SSTEP_GTEST}
                    GTest::gtest
                    GTest::gtest_main
                    sstep
                    )

gtest_discover_tests(${SSTEP_GTEST} PROPERTIES LABELS "unit" DISCOVERY_TIMEOUT 60)

# The library sources must stay warning-free: turn every warning into an error
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(${SSTEP_LIBRARY_SOURCES} PROPERTIES
        COMPILE_OPTIONS "-Wall;-Wextra;-Wpedantic;-Wconversion;-Werror")
endif()

# ==============================================================================
# COVERAGE
# ==============================================================================

set(SSTEP_COVERAGE_MIN 100 CACHE STRING "Minimum line and branch coverage (%) required by the 'coverage' target")

find_program(GCOVR_EXECUTABLE gcovr)
if(GCOVR_EXECUTABLE)
    set(COVERAGE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/Coverage)
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L unit
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_DIR}
        COMMAND ${GCOVR_EXECUTABLE}
            -r ${CMAKE_SOURCE_DIR}
            --object-directory ${CMAKE_BINARY_DIR}
            ${CMAKE_BINARY_DIR}         # search only this build (not another preset's)
            --filter ${CMAKE_SOURCE_DIR}/src
            --exclude ${CMAKE_SOURCE_DIR}/tests
            --lcov ${COVERAGE_OUTPUT_DIR}/coverage.info
            --html-details=${COVERAGE_OUTPUT_DIR}/coverage.html
            --print-summary
            --delete                    # fresh counters for the next run
            --fail-under-line ${SSTEP_COVERAGE_MIN}
            --fail-under-branch ${SSTEP_COVERAGE_MIN}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
        )
else()
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E echo "gcovr not found. Install gcovr to generate coverage reports."
        )
endif()
