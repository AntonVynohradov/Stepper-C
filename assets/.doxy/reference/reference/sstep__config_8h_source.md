

# File sstep\_config.h

[**File List**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**sstep\_config.h**](sstep__config_8h.md)

[Go to the documentation of this file](sstep__config_8h.md)


```C++
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

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#ifdef SSTEP_USER_CONFIG
#include "sstep_user_config.h"
#endif

/* ==============================================================================
 * NUMERIC BACKEND
 * =============================================================================*/

#define SSTEP_MATH_BACKEND_FIXED 0

#define SSTEP_MATH_BACKEND_FLOAT 1

#ifndef SSTEP_CFG_MATH_BACKEND
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
#define SSTEP_CFG_TOPOLOGY_FULL_BRIDGE 1
#endif

#ifndef SSTEP_CFG_TOPOLOGY_SHARED_LEG
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
#define SSTEP_CFG_SELFTEST 1
#endif

#if (SSTEP_CFG_SELFTEST != 0) && (SSTEP_CFG_SELFTEST != 1)
#error "SSTEP_CFG_SELFTEST must be 0 or 1"
#endif

/* ==============================================================================
 * STALL DETECTION
 * =============================================================================*/

#ifndef SSTEP_CFG_STALL
#define SSTEP_CFG_STALL 1
#endif

#if (SSTEP_CFG_STALL != 0) && (SSTEP_CFG_STALL != 1)
#error "SSTEP_CFG_STALL must be 0 or 1"
#endif

/* ==============================================================================
 * MICROSTEPPING
 * =============================================================================*/

#ifndef SSTEP_CFG_MICROSTEPS_MAX
#define SSTEP_CFG_MICROSTEPS_MAX 256U
#endif

#if (SSTEP_CFG_MICROSTEPS_MAX < 1U) || (SSTEP_CFG_MICROSTEPS_MAX > 256U) || \
    ((SSTEP_CFG_MICROSTEPS_MAX & (SSTEP_CFG_MICROSTEPS_MAX - 1U)) != 0U)
#error "SSTEP_CFG_MICROSTEPS_MAX must be a power of two in 1..256"
#endif

#ifndef SSTEP_CFG_SINE_TABLE_BITS
#define SSTEP_CFG_SINE_TABLE_BITS 8U
#endif

#if (SSTEP_CFG_SINE_TABLE_BITS < 6U) || (SSTEP_CFG_SINE_TABLE_BITS > 10U)
#error "SSTEP_CFG_SINE_TABLE_BITS must be in 6..10"
#endif

/* ==============================================================================
 * RUN-TIME CHECKS
 * =============================================================================*/

#ifndef SSTEP_CFG_ARG_CHECKS
#define SSTEP_CFG_ARG_CHECKS 1
#endif

#if (SSTEP_CFG_ARG_CHECKS != 0) && (SSTEP_CFG_ARG_CHECKS != 1)
#error "SSTEP_CFG_ARG_CHECKS must be 0 or 1"
#endif

#endif /* SSTEP_CONFIG_H */
```


