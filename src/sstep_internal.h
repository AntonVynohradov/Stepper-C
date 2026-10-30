/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Private interface between the library modules (power stage, motion, control).
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


#ifndef SSTEP_INTERNAL_H
#define SSTEP_INTERNAL_H

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdbool.h>
#include <stdint.h>

#include "sstep_ctrl.h"
#include "sstep_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

/** @brief Motion generator modes. */
#define SSTEP_MODE_HOLD 0U
#define SSTEP_MODE_MOVE 1U
#define SSTEP_MODE_RUN 2U

/** @brief Result flags of sstep_motion_step(). */
#define SSTEP_MOTION_DONE 0x01U

/** @brief Fixed point position: 1 microstep = 2^16. */
#define SSTEP_POS_ONE ((int64_t)65536)

/** @brief A move ends when it is closer than this to the target and slow enough
 *         (1/1024 microstep). */
#define SSTEP_POS_SNAP ((int64_t)64)

/* ==============================================================================
 * TYPE DEFINITIONS
 * =============================================================================*/

/** @brief Result of the power stage mapping. */
struct sstep_bridge_out
{
    uint16_t compare[SSTEP_LEGS_MAX]; /**< Compare values per leg. */
    sstep_acc_t v[2];                 /**< Applied phase voltages [pu of V_base]. */
    bool limited;                     /**< The request was scaled down. */
};

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS (library internal)
 * =============================================================================*/

/**
 * @brief  Map requested phase voltages to leg compare values (averaged power stage).
 *         The voltage vector is scaled down with its direction kept when the stage cannot
 *         produce it: full bridge |a|, |b| <= 1, shared leg max(a,b,0) - min(a,b,0) <= 1,
 *         a and b relative to the measured bus voltage. Shared leg: common leg centred.
 * @param  topology  Power stage topology (compiled in).
 * @param  va, vb    Requested phase voltages [pu of V_base].
 * @param  inv_vbus  V_base / bus voltage.
 * @param  vbus      Bus voltage [pu of V_base], 0 .. 2.
 * @param  period    Compare value for 100 % duty cycle.
 * @param  out       Result.
 */
void sstep_bridge_map(enum sstep_topology topology, sstep_acc_t va, sstep_acc_t vb,
                      const struct sstep_gain* inv_vbus, sstep_acc_t vbus, uint16_t period,
                      struct sstep_bridge_out* out);

/**
 * @brief  One slow loop period of the motion generator: trapezoidal velocity profile with
 *         one-period look-ahead braking for moves, exact stop on the target.
 * @param  mo  Motion state; pos and vel are updated.
 * @return ::SSTEP_MOTION_DONE when a move reached its target in this period, else 0.
 */
uint8_t sstep_motion_step(struct sstep_motion* mo);

/**
 * @brief  Integer square root: floor(sqrt(x)).
 * @param  x  Value.
 * @return floor(sqrt(x)).
 */
uint32_t sstep_isqrt64(uint64_t x);

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_INTERNAL_H */
