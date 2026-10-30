

# File sstep.h

[**File List**](files.md) **>** [**sstep.h**](sstep_8h.md)

[Go to the documentation of this file](sstep_8h.md)


```C++
/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Umbrella header: includes the public API and defines the version.
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

/* Normally defined by cmake/version.cmake from the latest vX.Y.Z tag; these
 * fallbacks are used when the library is built without CMake. Keep them in sync
 * with SSTEP_DEFAULT_VERSION and CHANGELOG.md. */

#ifndef SSTEP_VERSION_MAJOR
#define SSTEP_VERSION_MAJOR 0
#endif

#ifndef SSTEP_VERSION_MINOR
#define SSTEP_VERSION_MINOR 1
#endif

#ifndef SSTEP_VERSION_PATCH
#define SSTEP_VERSION_PATCH 0
#endif

#ifndef SSTEP_VERSION
#define SSTEP_VERSION "0.1.0"
#endif

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include "sstep_config.h"
#include "sstep_defs.h"
#include "sstep_math.h"
#include "sstep_ctrl.h"

#endif /* SSTEP_H */
```


