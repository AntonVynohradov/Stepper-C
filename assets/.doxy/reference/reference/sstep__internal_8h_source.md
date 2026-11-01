

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

#define SSTEP_READY 0x53545043U

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
 * DRIVER INSTANCE
 * =============================================================================*/

struct sstep_setpoint
{
    sstep_angle_t angle;        
    int32_t dangle;             
    sstep_real_t i_amp;         
    sstep_real_t ff;            
    struct sstep_gain inv_vbus; 
};

struct sstep_motion
{
    int64_t pos;        
    int64_t target;     
    int64_t brake_vmax; 
    int32_t vel;        
    int32_t v_run;      
    int32_t v_max;      
    int32_t acc;        
    int32_t dec;        
    uint8_t mode;       
};

struct sstep_ctx
{
    struct sstep_config cfg; 
    /* Constants derived by sstep_init() */
    struct sstep_gain kp;      
    struct sstep_gain ki;      
    struct sstep_gain kb;      
    struct sstep_gain r_pu;    
    struct sstep_gain adc_i;   
    struct sstep_gain adc_v;   
    struct sstep_gain ff_gain; 
    sstep_real_t i_run;        
    sstep_real_t i_hold;       
    uint16_t i_trip_counts;    
    uint16_t vbus_min_raw;     
    uint16_t vbus_max_raw;     
    uint16_t ratio;            
    uint32_t hold_ticks;       
    uint8_t angle_shift;       
    /* Stall detection constants, sstep_stall_*() */
    struct sstep_gain emf_a;     
    struct sstep_gain emf_b;     
    struct sstep_gain emf_alpha; 
    uint32_t stall_ticks;        
    int32_t stall_emf_mv;        
    /* Slow loop -> fast loop hand-off (double buffer) */
    struct sstep_setpoint sp[2]; 
    volatile uint8_t sp_active;  
    volatile uint8_t sp_pending; 
    /* Fast loop state */
    struct sstep_setpoint cur;        
    uint16_t adv;                     
    sstep_integ_t integ[2];           
    sstep_real_t i_meas[2];           
    volatile uint16_t vbus_raw;       
    uint16_t compare[SSTEP_LEGS_MAX]; 
    sstep_real_t i_prev[2];           
    sstep_acc_t v_hist[2][2];         
    sstep_integ_t emf[2];             
    /* Slow loop state */
    struct sstep_motion mo;     
    sstep_angle_t angle_offset; 
    uint32_t still;             
    bool hold_active;           
    struct sstep_gain inv_vbus; 
    int32_t vel_cmd;            
    sstep_real_t ff_cmd;        
    int64_t emf_off[2];         
    uint32_t stall_count;       
    volatile uint16_t faults;        
    volatile enum sstep_state state; 
    uint32_t ready;                  
};

static inline struct sstep_ctx* sstep_ctx_of(struct sstep* drv)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (struct sstep_ctx*)drv;
}

static inline const struct sstep_ctx* sstep_ctx_of_const(const struct sstep* drv)
{
    /* cppcheck-suppress misra-c2012-11.3 ; DEV-01 opaque instance */
    return (const struct sstep_ctx*)drv;
}

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS (library internal)
 * =============================================================================*/

void sstep_bridge_map(enum sstep_topology topology, sstep_acc_t va, sstep_acc_t vb,
                      const struct sstep_gain* inv_vbus, sstep_acc_t vbus, uint16_t period,
                      struct sstep_bridge_out* out);

uint8_t sstep_motion_step(struct sstep_motion* mo);

uint32_t sstep_isqrt64(uint64_t x);

void sstep_stall_reset(struct sstep_ctx* m);

void sstep_stall_observe(struct sstep_ctx* m, const struct sstep_sincos* sc,
                         const sstep_acc_t v_applied[2]);

bool sstep_stall_check(struct sstep_ctx* m);

void sstep_stall_status(const struct sstep_ctx* m, struct sstep_status* status);

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_INTERNAL_H */
```


