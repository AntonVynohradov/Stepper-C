

# File sstep.h

[**File List**](files.md) **>** [**sstep.h**](sstep_8h.md)

[Go to the documentation of this file](sstep_8h.md)


```C++
/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public API: library version, public types and all public functions.
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

#ifndef SSTEP_H
#define SSTEP_H

/* ==============================================================================
 * VERSION
 * =============================================================================*/

/* Set by cmake/version.cmake from the latest vX.Y.Z Git tag and stamped into this
 * file in the release package. 0.0.0 means unknown: a header taken from the
 * repository and built without CMake. Do not change these values for a release. */

#ifndef SSTEP_VERSION_MAJOR
#define SSTEP_VERSION_MAJOR 0
#endif

#ifndef SSTEP_VERSION_MINOR
#define SSTEP_VERSION_MINOR 0
#endif

#ifndef SSTEP_VERSION_PATCH
#define SSTEP_VERSION_PATCH 0
#endif

#ifndef SSTEP_VERSION
#define SSTEP_VERSION "0.0.0"
#endif

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include "sstep_config.h"
#include "sstep_defs.h"
#include "sstep_ctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS
 * =============================================================================*/

int sstep_init(struct sstep* drv, const struct sstep_config* cfg);

int sstep_enable(struct sstep* drv);

int sstep_disable(struct sstep* drv);

int sstep_fast_loop(struct sstep* drv, const struct sstep_sample* sample);

int sstep_slow_loop(struct sstep* drv);

int sstep_move_to(struct sstep* drv, int32_t position);

int sstep_move_by(struct sstep* drv, int32_t delta);

int sstep_run(struct sstep* drv, int32_t velocity);

int sstep_stop(struct sstep* drv);

int sstep_set_position(struct sstep* drv, int32_t position);

int sstep_set_current(struct sstep* drv, uint32_t run_ma, uint32_t hold_ma);

int sstep_set_motion_limits(struct sstep* drv, uint32_t v_max, uint32_t accel, uint32_t decel);

int sstep_report_fault(struct sstep* drv, uint16_t faults);

int sstep_clear_faults(struct sstep* drv);

int sstep_get_status(const struct sstep* drv, struct sstep_status* status);

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_H */
```


