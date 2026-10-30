

# File sstep\_internal.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_internal.h**](sstep__internal_8h.md)

[Go to the documentation of this file](sstep__internal_8h.md)


```C++
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

#define SSTEP_MODE_HOLD 0U
#define SSTEP_MODE_MOVE 1U
#define SSTEP_MODE_RUN 2U

#define SSTEP_MOTION_DONE 0x01U

#define SSTEP_POS_ONE ((int64_t)65536)

#define SSTEP_POS_SNAP ((int64_t)64)

/* ==============================================================================
 * TYPE DEFINITIONS
 * =============================================================================*/

struct sstep_bridge_out
{
    uint16_t compare[SSTEP_LEGS_MAX]; 
    sstep_acc_t v[2];                 
    bool limited;                     
};

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS (library internal)
 * =============================================================================*/

void sstep_bridge_map(enum sstep_topology topology, sstep_acc_t va, sstep_acc_t vb,
                      const struct sstep_gain* inv_vbus, sstep_acc_t vbus, uint16_t period,
                      struct sstep_bridge_out* out);

uint8_t sstep_motion_step(struct sstep_motion* mo);

uint32_t sstep_isqrt64(uint64_t x);

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_INTERNAL_H */
```


