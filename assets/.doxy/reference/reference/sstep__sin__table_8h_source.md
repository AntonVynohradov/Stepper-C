

# File sstep\_sin\_table.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_sin\_table.h**](sstep__sin__table_8h.md)

[Go to the documentation of this file](sstep__sin__table_8h.md)


```C++
/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Private declaration of the quarter-wave sine table (src/sstep_sin_table.c).
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


#ifndef SSTEP_SIN_TABLE_H
#define SSTEP_SIN_TABLE_H

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include "sstep_math.h"

/* ==============================================================================
 * DEFINES & MACROS
 * =============================================================================*/

#define SSTEP_SIN_TABLE_N ((uint32_t)1U << SSTEP_CFG_SINE_TABLE_BITS)

#define SSTEP_SIN_TABLE_SIZE (SSTEP_SIN_TABLE_N + 1U)

/* ==============================================================================
 * GLOBAL VARIABLES (extern declarations)
 * =============================================================================*/

extern const sstep_real_t sstep_sin_table[SSTEP_SIN_TABLE_SIZE];

#endif /* SSTEP_SIN_TABLE_H */
```


