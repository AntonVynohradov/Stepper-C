

# Group sstep\_ctrl



[**Modules**](modules.md) **>** [**sstep\_ctrl**](group__sstep__ctrl.md)



_Initialise a driver, run its loops and command motion._ [More...](#detailed-description)
















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**sstep**](structsstep.md) <br>_Driver instance._  |
| struct | [**sstep\_callbacks**](structsstep__callbacks.md) <br>_Hardware callbacks._  |
| struct | [**sstep\_config**](structsstep__config.md) <br>_Complete configuration given to_ [_**sstep\_init()**_](group__sstep__ctrl.md#function-sstep_init) _._ |
| struct | [**sstep\_ctrl\_cfg**](structsstep__ctrl__cfg.md) <br>_Control and motion parameters._  |
| struct | [**sstep\_motion**](structsstep__motion.md) <br>_Motion generator state._  |
| struct | [**sstep\_motor\_cfg**](structsstep__motor__cfg.md) <br>_Motor parameters, from the datasheet or measured._  |
| struct | [**sstep\_sample**](structsstep__sample.md) <br>_Raw measurements of one fast loop period, as read from the ADC._  |
| struct | [**sstep\_setpoint**](structsstep__setpoint.md) <br>_Setpoint handed from the slow loop to the fast loop._  |
| struct | [**sstep\_stage\_cfg**](structsstep__stage__cfg.md) <br>_Power stage and measurement parameters._  |
| struct | [**sstep\_status**](structsstep__status.md) <br>_Snapshot returned by_ [_**sstep\_get\_status()**_](group__sstep__ctrl.md#function-sstep_get_status) _, in engineering units._ |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**sstep\_drive\_mode**](#enum-sstep_drive_mode)  <br>_How the phase voltages are obtained._  |
| enum  | [**sstep\_event**](#enum-sstep_event)  <br>_Events reported through_ [_**sstep\_event\_fn**_](group__sstep__ctrl.md#typedef-sstep_event_fn) _._ |
| typedef void(\* | [**sstep\_event\_fn**](#typedef-sstep_event_fn)  <br>_Notify the application of a motion event (optional)._  |
| typedef void(\* | [**sstep\_fault\_fn**](#typedef-sstep_fault_fn)  <br>_Notify the application of new faults (optional)._  |
| typedef int(\* | [**sstep\_output\_enable\_fn**](#typedef-sstep_output_enable_fn)  <br>_Enable or disable the power stage outputs (disabled = all legs high impedance)._  |
| typedef int(\* | [**sstep\_pwm\_write\_fn**](#typedef-sstep_pwm_write_fn)  <br>_Write the PWM compare values of all legs._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  int | [**sstep\_clear\_faults**](#function-sstep_clear_faults) (struct [**sstep**](structsstep.md) \* m) <br>_Clear all faults; the driver goes to_ SSTEP\_STATE\_DISABLED _._ |
|  int | [**sstep\_disable**](#function-sstep_disable) (struct [**sstep**](structsstep.md) \* m) <br>_Stop immediately and switch the outputs off (motor coasts, no holding torque)._  |
|  int | [**sstep\_enable**](#function-sstep_enable) (struct [**sstep**](structsstep.md) \* m) <br>_Enable the outputs and hold the current position with the hold current._  |
|  int | [**sstep\_fast\_loop**](#function-sstep_fast_loop) (struct [**sstep**](structsstep.md) \* m, const struct [**sstep\_sample**](structsstep__sample.md) \* sample) <br>_Fast loop: convert the measurements, run the current controllers, limit the voltage, map it to the legs of the topology and write the PWM._  |
|  int | [**sstep\_get\_status**](#function-sstep_get_status) (const struct [**sstep**](structsstep.md) \* m, struct [**sstep\_status**](structsstep__status.md) \* status) <br>_Read a status snapshot in engineering units._  |
|  int | [**sstep\_init**](#function-sstep_init) (struct [**sstep**](structsstep.md) \* m, const struct [**sstep\_config**](structsstep__config.md) \* cfg) <br>_Validate a configuration and initialise a driver._  |
|  int | [**sstep\_move\_by**](#function-sstep_move_by) (struct [**sstep**](structsstep.md) \* m, int32\_t delta) <br>_Start a move relative to the current target._  |
|  int | [**sstep\_move\_to**](#function-sstep_move_to) (struct [**sstep**](structsstep.md) \* m, int32\_t position) <br>_Start a move to an absolute position with the trapezoidal profile._  |
|  int | [**sstep\_report\_fault**](#function-sstep_report_fault) (struct [**sstep**](structsstep.md) \* m, uint16\_t faults) <br>_Report a fault detected by the application (driver fault pin, temperature)._  |
|  int | [**sstep\_run**](#function-sstep_run) (struct [**sstep**](structsstep.md) \* m, int32\_t velocity) <br>_Run at a constant velocity (accelerating with the configured limits) until another command._  |
|  int | [**sstep\_set\_current**](#function-sstep_set_current) (struct [**sstep**](structsstep.md) \* m, uint32\_t run\_ma, uint32\_t hold\_ma) <br>_Change the run and hold currents._  |
|  int | [**sstep\_set\_motion\_limits**](#function-sstep_set_motion_limits) (struct [**sstep**](structsstep.md) \* m, uint32\_t v\_max, uint32\_t accel, uint32\_t decel) <br>_Change the motion limits; applies to the next command._  |
|  int | [**sstep\_set\_position**](#function-sstep_set_position) (struct [**sstep**](structsstep.md) \* m, int32\_t position) <br>_Redefine the current position (homing)._  |
|  int | [**sstep\_slow\_loop**](#function-sstep_slow_loop) (struct [**sstep**](structsstep.md) \* m) <br>_Slow loop: advance the trajectory, update the angle and current references, handle the hold current and report events._  |
|  int | [**sstep\_stop**](#function-sstep_stop) (struct [**sstep**](structsstep.md) \* m) <br>_Decelerate to standstill with the configured deceleration and hold._  |




























## Detailed Description


**Execution model.** One driver instance (`struct sstep`) controls one motor. The application calls two functions periodically:



|Function   |Rate   |Context   |Work    |
|-----|-----|-----|-----|
|[**sstep\_fast\_loop()**](group__sstep__ctrl.md#function-sstep_fast_loop)   |`fast_loop_hz` (PWM rate or an integer fraction of it)   |PWM / ADC interrupt   |current control, voltage limit, topology mapping, PWM write    |
|[**sstep\_slow\_loop()**](group__sstep__ctrl.md#function-sstep_slow_loop)   |`slow_loop_hz` (typically 1 kHz)   |main loop or a timer task   |trajectory, microstep angle, hold current, fault handling   |






All other functions are called from the same context as [**sstep\_slow\_loop()**](group__sstep__ctrl.md#function-sstep_slow_loop). The library never reads a clock, never blocks, allocates no memory and has no global state: time is given by the configured loop rates.


**Hardware access** is done only through the callbacks in [**sstep\_callbacks**](structsstep__callbacks.md), given once to [**sstep\_init()**](group__sstep__ctrl.md#function-sstep_init). Measurements are passed to [**sstep\_fast\_loop()**](group__sstep__ctrl.md#function-sstep_fast_loop) as raw ADC counts. 


    
## Public Types Documentation




### enum sstep\_drive\_mode 

_How the phase voltages are obtained._ 
```
enum sstep_drive_mode {
    SSTEP_DRIVE_CURRENT = 0,
    SSTEP_DRIVE_VOLTAGE = 1
};
```




<hr>



### enum sstep\_event 

_Events reported through_ [_**sstep\_event\_fn**_](group__sstep__ctrl.md#typedef-sstep_event_fn) _._
```
enum sstep_event {
    SSTEP_EVENT_MOVE_DONE = 0,
    SSTEP_EVENT_HOLD_CURRENT = 1
};
```




<hr>



### typedef sstep\_event\_fn 

_Notify the application of a motion event (optional)._ 
```
typedef void(* sstep_event_fn) (void *context, enum sstep_event event);
```



Called from [**sstep\_slow\_loop()**](group__sstep__ctrl.md#function-sstep_slow_loop). 

**Parameters:**


* `context` [**sstep\_callbacks::context**](structsstep__callbacks.md#variable-context). 
* `event` The event. 




        

<hr>



### typedef sstep\_fault\_fn 

_Notify the application of new faults (optional)._ 
```
typedef void(* sstep_fault_fn) (void *context, uint16_t faults);
```



Called from the context that detected the fault, which may be [**sstep\_fast\_loop()**](group__sstep__ctrl.md#function-sstep_fast_loop) (interrupt). 

**Parameters:**


* `context` [**sstep\_callbacks::context**](structsstep__callbacks.md#variable-context). 
* `faults` Newly raised [**SSTEP\_FAULT\_OVERCURRENT**](group__sstep__defs.md#define-sstep_fault_overcurrent) ... flags. 




        

<hr>



### typedef sstep\_output\_enable\_fn 

_Enable or disable the power stage outputs (disabled = all legs high impedance)._ 
```
typedef int(* sstep_output_enable_fn) (void *context, bool enable);
```





**Parameters:**


* `context` [**sstep\_callbacks::context**](structsstep__callbacks.md#variable-context). 
* `enable` true to enable the gate drivers, false to switch every leg off. 



**Return value:**


* `0` Success. 
* `<0` Error, reported as [**SSTEP\_FAULT\_HAL**](group__sstep__defs.md#define-sstep_fault_hal). 




        

<hr>



### typedef sstep\_pwm\_write\_fn 

_Write the PWM compare values of all legs._ 
```
typedef int(* sstep_pwm_write_fn) (void *context, const uint16_t compare[SSTEP_LEGS_MAX]);
```



Called from [**sstep\_fast\_loop()**](group__sstep__ctrl.md#function-sstep_fast_loop). 

**Parameters:**


* `context` [**sstep\_callbacks::context**](structsstep__callbacks.md#variable-context). 
* `compare` One compare value per leg, 0 .. `pwm_period_ticks` (0 = low-side on, `pwm_period_ticks` = high-side on). Entries above the leg count of the topology are 0. Valid only during the call. 



**Return value:**


* `0` Success. 
* `<0` Error: the driver switches the outputs off with [**SSTEP\_FAULT\_HAL**](group__sstep__defs.md#define-sstep_fault_hal). 




        

<hr>
## Public Functions Documentation




### function sstep\_clear\_faults 

_Clear all faults; the driver goes to_ SSTEP\_STATE\_DISABLED _._
```
int sstep_clear_faults (
    struct sstep * m
) 
```





**Parameters:**


* `m` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_disable 

_Stop immediately and switch the outputs off (motor coasts, no holding torque)._ 
```
int sstep_disable (
    struct sstep * m
) 
```





**Parameters:**


* `m` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_enable 

_Enable the outputs and hold the current position with the hold current._ 
```
int sstep_enable (
    struct sstep * m
) 
```





**Parameters:**


* `m` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (faulted). 




        

<hr>



### function sstep\_fast\_loop 

_Fast loop: convert the measurements, run the current controllers, limit the voltage, map it to the legs of the topology and write the PWM._ 
```
int sstep_fast_loop (
    struct sstep * m,
    const struct sstep_sample * sample
) 
```



Call from the PWM/ADC interrupt at `fast_loop_hz`. Bounded execution time, no loops over data. 

**Parameters:**


* `m` Driver instance. 
* `sample` Measurements of this period. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (only when [**SSTEP\_CFG\_ARG\_CHECKS**](group__sstep__config.md#define-sstep_cfg_arg_checks) is 1). 




        

<hr>



### function sstep\_get\_status 

_Read a status snapshot in engineering units._ 
```
int sstep_get_status (
    const struct sstep * m,
    struct sstep_status * status
) 
```





**Parameters:**


* `m` Driver instance. 
* `status` Filled on success. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_init 

_Validate a configuration and initialise a driver._ 
```
int sstep_init (
    struct sstep * m,
    const struct sstep_config * cfg
) 
```



Outputs stay disabled. Derives the per-unit bases, the PI gains (Kp = L \* wc, Ki = R \* wc) and the trajectory constants; no floating point is used with the fixed point backend. 

**Parameters:**


* `m` Driver instance. 
* `cfg` Configuration; copied. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) Success, state SSTEP\_STATE\_DISABLED. 
* [**SSTEP\_ERR\_PARAM**](group__sstep__defs.md#define-sstep_err_param) NULL pointer, missing required callback or a field out of range. 
* [**SSTEP\_ERR\_CONFIG**](group__sstep__defs.md#define-sstep_err_config) Inconsistent fields (for example i\_run\_ma &gt; i\_trip\_ma). 
* [**SSTEP\_ERR\_UNSUPPORTED**](group__sstep__defs.md#define-sstep_err_unsupported) Topology not compiled in. 




        

<hr>



### function sstep\_move\_by 

_Start a move relative to the current target._ 
```
int sstep_move_by (
    struct sstep * m,
    int32_t delta
) 
```





**Parameters:**


* `m` Driver instance. 
* `delta` Distance [microsteps]. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (also when the target overflows), [**SSTEP\_ERR\_STATE**](group__sstep__defs.md#define-sstep_err_state). 




        

<hr>



### function sstep\_move\_to 

_Start a move to an absolute position with the trapezoidal profile._ 
```
int sstep_move_to (
    struct sstep * m,
    int32_t position
) 
```



Replaces the current move; the change is smooth (no velocity step). 

**Parameters:**


* `m` Driver instance. 
* `position` Target [microsteps]. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (disabled or faulted). 




        

<hr>



### function sstep\_report\_fault 

_Report a fault detected by the application (driver fault pin, temperature)._ 
```
int sstep_report_fault (
    struct sstep * m,
    uint16_t faults
) 
```



Switches the outputs off. May be called from an interrupt. 

**Parameters:**


* `m` Driver instance. 
* `faults` Flags to raise, typically [**SSTEP\_FAULT\_EXTERNAL**](group__sstep__defs.md#define-sstep_fault_external). 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_run 

_Run at a constant velocity (accelerating with the configured limits) until another command._ 
```
int sstep_run (
    struct sstep * m,
    int32_t velocity
) 
```





**Parameters:**


* `m` Driver instance. 
* `velocity` Signed velocity [microsteps/s], \|velocity\| &lt;= v\_max. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_set\_current 

_Change the run and hold currents._ 
```
int sstep_set_current (
    struct sstep * m,
    uint32_t run_ma,
    uint32_t hold_ma
) 
```





**Parameters:**


* `m` Driver instance. 
* `run_ma` Peak phase current while moving [mA]. 
* `hold_ma` Peak phase current at standstill [mA], &lt;= run\_ma. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (above the trip level). 




        

<hr>



### function sstep\_set\_motion\_limits 

_Change the motion limits; applies to the next command._ 
```
int sstep_set_motion_limits (
    struct sstep * m,
    uint32_t v_max,
    uint32_t accel,
    uint32_t decel
) 
```





**Parameters:**


* `m` Driver instance. 
* `v_max` Maximum velocity [microsteps/s]. 
* `accel` Acceleration [microsteps/s^2]. 
* `decel` Deceleration [microsteps/s^2]; 0 = same as accel. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_set\_position 

_Redefine the current position (homing)._ 
```
int sstep_set_position (
    struct sstep * m,
    int32_t position
) 
```



Only at standstill. 

**Parameters:**


* `m` Driver instance. 
* `position` New value of the current position [microsteps]. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (moving). 




        

<hr>



### function sstep\_slow\_loop 

_Slow loop: advance the trajectory, update the angle and current references, handle the hold current and report events._ 
```
int sstep_slow_loop (
    struct sstep * m
) 
```



Call at `slow_loop_hz`. 

**Parameters:**


* `m` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_stop 

_Decelerate to standstill with the configured deceleration and hold._ 
```
int sstep_stop (
    struct sstep * m
) 
```





**Parameters:**


* `m` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>

------------------------------


