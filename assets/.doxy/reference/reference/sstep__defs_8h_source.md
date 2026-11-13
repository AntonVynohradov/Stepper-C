

# File sstep\_defs.h

[**File List**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**sstep\_defs.h**](sstep__defs_8h.md)

[Go to the documentation of this file](sstep__defs_8h.md)


```C++
/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public constants: status codes, topologies, power stage legs, faults, states.
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


#ifndef SSTEP_DEFS_H
#define SSTEP_DEFS_H


/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdint.h>

/* ==============================================================================
 * STATUS CODES
 * =============================================================================*/

#define SSTEP_OK 0
#define SSTEP_ERR_PARAM (-1)
#define SSTEP_ERR_CONFIG (-2)
#define SSTEP_ERR_STATE (-3)
#define SSTEP_ERR_UNSUPPORTED (-4)
#define SSTEP_ERR_VBUS (-5)

/* ==============================================================================
 * POWER STAGE
 * =============================================================================*/

#define SSTEP_LEGS_MAX 4U

enum sstep_topology
{
    SSTEP_TOPOLOGY_FULL_BRIDGE = 0,
    SSTEP_TOPOLOGY_SHARED_LEG = 1
};

/* ==============================================================================
 * FAULTS
 * =============================================================================*/

#define SSTEP_FAULT_NONE 0x0000U
#define SSTEP_FAULT_OVERCURRENT 0x0001U
#define SSTEP_FAULT_OVERVOLTAGE 0x0002U
#define SSTEP_FAULT_UNDERVOLTAGE 0x0004U
#define SSTEP_FAULT_HAL 0x0008U
#define SSTEP_FAULT_EXTERNAL 0x0010U
#define SSTEP_FAULT_STALL 0x0020U
#define SSTEP_FAULT_SELFTEST 0x0040U

/* ==============================================================================
 * STATES
 * =============================================================================*/

enum sstep_state
{
    SSTEP_STATE_DISABLED = 0, 
    SSTEP_STATE_HOLDING = 1,  
    SSTEP_STATE_MOVING = 2,   
    SSTEP_STATE_FAULT = 3     
};

/* ==============================================================================
 * SELF-TEST
 * =============================================================================*/

enum sstep_selftest_result
{
    SSTEP_SELFTEST_NONE = 0,     
    SSTEP_SELFTEST_RUNNING = 1,  
    SSTEP_SELFTEST_PASS = 2,     
    SSTEP_SELFTEST_LEAK = 3,     
    SSTEP_SELFTEST_OPEN = 4,     
    SSTEP_SELFTEST_SHORT = 5,    
    SSTEP_SELFTEST_POLARITY = 6, 
    SSTEP_SELFTEST_WRONG_PHASE = 7, 
    SSTEP_SELFTEST_ABORTED = 8      
};


#endif /* SSTEP_DEFS_H */
```


