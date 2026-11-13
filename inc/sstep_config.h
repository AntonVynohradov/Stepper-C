/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Build-time configuration: numeric backend, power stage topologies, limits.
 *
 * Author:      Anton Vynohradov
 * Email:       avynohradov@systemfromscratch.com
 *
 * License:     Apache License 2.0
 *
 * Copyright (c) 2026 Anton Vynohradov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/


#ifndef SSTEP_CONFIG_H
#define SSTEP_CONFIG_H

/**
 * @defgroup sstep_config Build-time configuration
 * @brief    Macros that select the code compiled into the library.
 *
 * Every macro has a default and may be overridden in one of two ways:
 * - on the compiler command line (`-DSSTEP_CFG_MATH_BACKEND=SSTEP_MATH_BACKEND_FLOAT`), or
 * - in a user header `sstep_user_config.h`, included when the macro
 *   `SSTEP_USER_CONFIG` is defined (CMake: set `SSTEP_CONFIG_DIR` to its folder).
 *
 * Values that cannot change at run time (code size, table sizes, which code paths exist)
 * live here. Motor and board parameters are run-time values passed to sstep_init().
 * @{
 */

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#ifdef SSTEP_USER_CONFIG
#include "sstep_user_config.h"
#endif

/* ==============================================================================
 * NUMERIC BACKEND
 * =============================================================================*/

/** @brief Fixed point backend: Q15 signals, Q31 accumulators, integer-only arithmetic.
 *         For MCUs without an FPU (Cortex-M0/M0+/M3, RISC-V RV32IMC, 8/16-bit cores). */
#define SSTEP_MATH_BACKEND_FIXED 0

/** @brief Single-precision float backend. For MCUs with a hardware FPU
 *         (Cortex-M4F/M7/M33 with FPU, STM32G4, ESP32-S3...). */
#define SSTEP_MATH_BACKEND_FLOAT 1

#ifndef SSTEP_CFG_MATH_BACKEND
/** @brief Selected numeric backend: ::SSTEP_MATH_BACKEND_FIXED or ::SSTEP_MATH_BACKEND_FLOAT. */
#define SSTEP_CFG_MATH_BACKEND SSTEP_MATH_BACKEND_FIXED
#endif

#if (SSTEP_CFG_MATH_BACKEND != SSTEP_MATH_BACKEND_FIXED) && \
    (SSTEP_CFG_MATH_BACKEND != SSTEP_MATH_BACKEND_FLOAT)
#error "SSTEP_CFG_MATH_BACKEND must be SSTEP_MATH_BACKEND_FIXED or SSTEP_MATH_BACKEND_FLOAT"
#endif

/* ==============================================================================
 * POWER STAGE TOPOLOGIES
 * =============================================================================*/

#ifndef SSTEP_CFG_TOPOLOGY_FULL_BRIDGE
/** @brief 1: compile support for two full H-bridges (4 half-bridge legs). */
#define SSTEP_CFG_TOPOLOGY_FULL_BRIDGE 1
#endif

#ifndef SSTEP_CFG_TOPOLOGY_SHARED_LEG
/** @brief 1: compile support for three half-bridges with a common leg (3 legs),
 *         for example STMicroelectronics B-G431B-ESC1. */
#define SSTEP_CFG_TOPOLOGY_SHARED_LEG 1
#endif

#if (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE != 0) && (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE != 1)
#error "SSTEP_CFG_TOPOLOGY_FULL_BRIDGE must be 0 or 1"
#endif

#if (SSTEP_CFG_TOPOLOGY_SHARED_LEG != 0) && (SSTEP_CFG_TOPOLOGY_SHARED_LEG != 1)
#error "SSTEP_CFG_TOPOLOGY_SHARED_LEG must be 0 or 1"
#endif

#if (SSTEP_CFG_TOPOLOGY_FULL_BRIDGE == 0) && (SSTEP_CFG_TOPOLOGY_SHARED_LEG == 0)
#error "At least one power stage topology must be enabled"
#endif

/* ==============================================================================
 * SELF-TEST
 * =============================================================================*/

#ifndef SSTEP_CFG_SELFTEST
/** @brief 1: compile the power stage and wiring self-test, sstep_selftest(). 0 removes its
 *         code and state; sstep_selftest() then returns ::SSTEP_ERR_UNSUPPORTED. */
#define SSTEP_CFG_SELFTEST 1
#endif

#if (SSTEP_CFG_SELFTEST != 0) && (SSTEP_CFG_SELFTEST != 1)
#error "SSTEP_CFG_SELFTEST must be 0 or 1"
#endif

/* ==============================================================================
 * STALL DETECTION
 * =============================================================================*/

#ifndef SSTEP_CFG_STALL
/** @brief 1: compile the sensorless stall detection (back-EMF observer, enabled at run time
 *         with sstep_ctrl_cfg::stall_time_ms). 0 removes its code and state; sstep_init()
 *         then returns ::SSTEP_ERR_UNSUPPORTED for stall_time_ms > 0. */
#define SSTEP_CFG_STALL 1
#endif

#if (SSTEP_CFG_STALL != 0) && (SSTEP_CFG_STALL != 1)
#error "SSTEP_CFG_STALL must be 0 or 1"
#endif

/* ==============================================================================
 * MICROSTEPPING
 * =============================================================================*/

#ifndef SSTEP_CFG_MICROSTEPS_MAX
/** @brief Largest microstep division accepted by sstep_init() (power of two, 1..256). */
#define SSTEP_CFG_MICROSTEPS_MAX 256U
#endif

#if (SSTEP_CFG_MICROSTEPS_MAX < 1U) || (SSTEP_CFG_MICROSTEPS_MAX > 256U) || \
    ((SSTEP_CFG_MICROSTEPS_MAX & (SSTEP_CFG_MICROSTEPS_MAX - 1U)) != 0U)
#error "SSTEP_CFG_MICROSTEPS_MAX must be a power of two in 1..256"
#endif

#ifndef SSTEP_CFG_SINE_TABLE_BITS
/** @brief log2 of the number of entries of the quarter-wave sine table (6..10).
 *         The table is `static const` (flash): 2^bits + 1 entries of 2 bytes (fixed)
 *         or 4 bytes (float). Values between entries are linearly interpolated. */
#define SSTEP_CFG_SINE_TABLE_BITS 8U
#endif

#if (SSTEP_CFG_SINE_TABLE_BITS < 6U) || (SSTEP_CFG_SINE_TABLE_BITS > 10U)
#error "SSTEP_CFG_SINE_TABLE_BITS must be in 6..10"
#endif

/* ==============================================================================
 * RUN-TIME CHECKS
 * =============================================================================*/

#ifndef SSTEP_CFG_ARG_CHECKS
/** @brief 1: every public function validates its pointer arguments (MISRA Dir 4.11).
 *         0 removes the checks from sstep_fast_loop() only, the function called from
 *         the PWM interrupt; all other functions always check. */
#define SSTEP_CFG_ARG_CHECKS 1
#endif

#if (SSTEP_CFG_ARG_CHECKS != 0) && (SSTEP_CFG_ARG_CHECKS != 1)
#error "SSTEP_CFG_ARG_CHECKS must be 0 or 1"
#endif

/** @} */

#endif /* SSTEP_CONFIG_H */
