

# File sstep\_ctrl.h

[**File List**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**sstep\_ctrl.h**](sstep__ctrl_8h.md)

[Go to the documentation of this file](sstep__ctrl_8h.md)


```C++
/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public types: configuration, hardware callbacks, measurements, driver instance.
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


#ifndef SSTEP_CTRL_H
#define SSTEP_CTRL_H

/* ==============================================================================
 * INCLUDES
 * =============================================================================*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sstep_config.h"
#include "sstep_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * CALLBACK TYPES
 * =============================================================================*/

typedef int (*sstep_pwm_write_fn)(void* context, const uint16_t compare[SSTEP_LEGS_MAX]);

typedef int (*sstep_output_enable_fn)(void* context, bool enable);

typedef void (*sstep_fault_fn)(void* context, uint16_t faults);

enum sstep_event
{
    SSTEP_EVENT_MOVE_DONE = 0,    
    SSTEP_EVENT_HOLD_CURRENT = 1, 
    SSTEP_EVENT_SELFTEST_DONE = 2 
};

typedef void (*sstep_event_fn)(void* context, enum sstep_event event);

/* ==============================================================================
 * CONFIGURATION TYPES
 * =============================================================================*/

struct sstep_callbacks
{
    void* context;                        
    sstep_pwm_write_fn pwm_write;         
    sstep_output_enable_fn output_enable; 
    sstep_fault_fn on_fault;              
    sstep_event_fn on_event;              
};

struct sstep_motor_cfg
{
    uint32_t r_mohm;             
    uint32_t l_uh;               
    uint32_t i_rated_ma;         
    uint32_t ke_uv_s_per_rad;    
    uint16_t full_steps_per_rev; 
};

struct sstep_stage_cfg
{
    enum sstep_topology topology; 
    uint32_t vbus_nominal_mv;     
    uint32_t vbus_min_mv;         
    uint32_t vbus_max_mv;         
    uint32_t i_full_scale_ma;     
    uint32_t i_trip_ma;           
    uint16_t pwm_period_ticks;    
    uint32_t pwm_freq_hz;         
    uint32_t dead_time_ns;        
    uint16_t adc_i_zero;          
    int32_t adc_i_span;           
    uint32_t adc_vbus_mv_full;    
    uint16_t adc_full;            
    uint16_t sample_window_ticks; 
};

enum sstep_drive_mode
{
    SSTEP_DRIVE_CURRENT = 0,
    SSTEP_DRIVE_VOLTAGE = 1
};

struct sstep_ctrl_cfg
{
    enum sstep_drive_mode drive_mode; 
    uint32_t fast_loop_hz;            
    uint32_t slow_loop_hz;            
    uint16_t microsteps;              
    uint32_t current_bw_hz;           
    uint32_t i_run_ma;                
    uint32_t i_hold_ma;               
    uint32_t hold_delay_ms;           
    uint32_t v_max;                   
    uint32_t accel;                   
    uint32_t decel;                   
    uint32_t stall_time_ms;           
    uint32_t stall_filter_us;         
    uint32_t stall_speed;             
    uint16_t stall_sync_pct;          
    bool i_trip_voltage_drive;        
    uint32_t i_slew_ma_per_ms;        
};

struct sstep_config
{
    struct sstep_motor_cfg motor; 
    struct sstep_stage_cfg stage; 
    struct sstep_ctrl_cfg ctrl;   
    struct sstep_callbacks hal;   
};

/* ==============================================================================
 * MEASUREMENT AND STATUS TYPES
 * =============================================================================*/

struct sstep_sample
{
    uint16_t i_a;  
    uint16_t i_b;  
    uint16_t vbus; 
};

struct sstep_status
{
    enum sstep_state state;  
    uint16_t faults;         
    int32_t position;        
    int32_t target;          
    int32_t velocity;        
    int32_t i_a_ma;          
    int32_t i_b_ma;          
    uint32_t vbus_mv;        
    int32_t emf_d_mv;        
    int32_t emf_q_mv;        
    int32_t rotor_velocity;  
    int32_t load_angle_cdeg; 
    uint8_t stall_level_pct; 
    enum sstep_selftest_result selftest; 
    uint8_t selftest_phase;              
    int32_t selftest_i_ma;  
    uint16_t selftest_duty; 
};

/* ==============================================================================
 * DRIVER INSTANCE
 * =============================================================================*/

#if (SSTEP_CFG_SELFTEST == 1)
#define SSTEP_SELFTEST_BYTES 40U 
#else
#define SSTEP_SELFTEST_BYTES 0U
#endif

#if defined(UINTPTR_MAX) && (UINTPTR_MAX > 0xFFFFFFFFU)
#define SSTEP_CONFIG_BYTES 192U
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
#define SSTEP_STATE_BYTES (304U + SSTEP_SELFTEST_BYTES)
#else
#define SSTEP_STATE_BYTES (336U + SSTEP_SELFTEST_BYTES)
#endif
#else
#define SSTEP_CONFIG_BYTES 156U
#if (SSTEP_CFG_MATH_BACKEND == SSTEP_MATH_BACKEND_FIXED)
#define SSTEP_STATE_BYTES (292U + SSTEP_SELFTEST_BYTES)
#else
#define SSTEP_STATE_BYTES (324U + SSTEP_SELFTEST_BYTES)
#endif
#endif

#define SSTEP_INSTANCE_WORDS ((SSTEP_CONFIG_BYTES + SSTEP_STATE_BYTES + 7U) / 8U)

struct sstep
{
    uint64_t opaque[SSTEP_INSTANCE_WORDS]; 
};

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_CTRL_H */
```


