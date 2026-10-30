

# File sstep\_ctrl.h

[**File List**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**sstep\_ctrl.h**](sstep__ctrl_8h.md)

[Go to the documentation of this file](sstep__ctrl_8h.md)


```C++
/*******************************************************************************
 * Stepper-C: Stepper Motor Control Implementation
 *
 * Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
 * Description: Public API: configuration, hardware callbacks, control loops, motion commands.
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
#include "sstep_math.h"

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
    SSTEP_EVENT_HOLD_CURRENT = 1 
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
};

/* ==============================================================================
 * DRIVER INSTANCE (PRIVATE)
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

struct sstep
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
    /* Slow loop state */
    struct sstep_motion mo;           
    sstep_angle_t angle_offset;       
    uint32_t still;                   
    bool hold_active;                 
    struct sstep_gain inv_vbus;       
    volatile uint16_t faults;         
    volatile enum sstep_state state;  
};

/* ==============================================================================
 * PUBLIC FUNCTION DECLARATIONS
 * =============================================================================*/

int sstep_init(struct sstep* m, const struct sstep_config* cfg);

int sstep_enable(struct sstep* m);

int sstep_disable(struct sstep* m);

int sstep_fast_loop(struct sstep* m, const struct sstep_sample* sample);

int sstep_slow_loop(struct sstep* m);

int sstep_move_to(struct sstep* m, int32_t position);

int sstep_move_by(struct sstep* m, int32_t delta);

int sstep_run(struct sstep* m, int32_t velocity);

int sstep_stop(struct sstep* m);

int sstep_set_position(struct sstep* m, int32_t position);

int sstep_set_current(struct sstep* m, uint32_t run_ma, uint32_t hold_ma);

int sstep_set_motion_limits(struct sstep* m, uint32_t v_max, uint32_t accel, uint32_t decel);

int sstep_report_fault(struct sstep* m, uint16_t faults);

int sstep_clear_faults(struct sstep* m);

int sstep_get_status(const struct sstep* m, struct sstep_status* status);

#ifdef __cplusplus
}
#endif

#endif /* SSTEP_CTRL_H */
```


