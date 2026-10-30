################################################################################
# Stepper-C: Stepper Motor Control Implementation
#
# Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
# Description: CMake configuration for the pytest integration tests (Python
#              codec and the C/Python cross-check)
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# References:
#   - Stepper Motor Control specification: TODO-specification-url
#   - pytest Documentation: https://docs.pytest.org/
#   - CMake Documentation: https://cmake.org/documentation/
#
# Last Updated: October 2026
# SPDX-License-Identifier: Apache-2.0
#
################################################################################

# ==============================================================================
# CMAKE CONFIGURATION
# ==============================================================================

find_package(Python3 REQUIRED COMPONENTS Interpreter)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import pytest"
    RESULT_VARIABLE SSTEP_PYTEST_RESULT
    OUTPUT_QUIET
    ERROR_QUIET
)

if(NOT SSTEP_PYTEST_RESULT EQUAL 0)
    message(FATAL_ERROR "pytest not found for ${Python3_EXECUTABLE}. Install it with: python -m pip install pytest")
endif()

# ==============================================================================
# TESTING
# ==============================================================================

enable_testing()

add_test(NAME sstep_integration_tests
    COMMAND ${Python3_EXECUTABLE} -m pytest
            ${CMAKE_SOURCE_DIR}/tests/integration
            -v
            -p no:cacheprovider
            --junitxml=${CMAKE_BINARY_DIR}/integration-tests.xml
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set_tests_properties(sstep_integration_tests PROPERTIES LABELS "integration")

# Convenience target, mirrors 'coverage' for the unit tests
add_custom_target(integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L integration
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    USES_TERMINAL
)
