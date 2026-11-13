

# Group sstep\_api



[**Modules**](modules.md) **>** [**sstep\_api**](group__sstep__api.md)



_Initialise a driver, run its loops and command motion._ [More...](#detailed-description)






































## Public Functions

| Type | Name |
| ---: | :--- |
|  int | [**sstep\_clear\_faults**](#function-sstep_clear_faults) (struct [**sstep**](structsstep.md) \* drv) <br>_Clear all faults; the driver goes to_ SSTEP\_STATE\_DISABLED _._ |
|  int | [**sstep\_disable**](#function-sstep_disable) (struct [**sstep**](structsstep.md) \* drv) <br>_Stop immediately and switch the outputs off (motor coasts, no holding torque)._  |
|  int | [**sstep\_enable**](#function-sstep_enable) (struct [**sstep**](structsstep.md) \* drv) <br>_Enable the outputs and hold the current position with the hold current._  |
|  int | [**sstep\_fast\_loop**](#function-sstep_fast_loop) (struct [**sstep**](structsstep.md) \* drv, const struct [**sstep\_sample**](structsstep__sample.md) \* sample) <br>_Fast loop: convert the measurements, run the current controllers, limit the voltage, map it to the legs of the topology and write the PWM._  |
|  int | [**sstep\_get\_status**](#function-sstep_get_status) (const struct [**sstep**](structsstep.md) \* drv, struct [**sstep\_status**](structsstep__status.md) \* status) <br>_Read a status snapshot in engineering units._  |
|  int | [**sstep\_init**](#function-sstep_init) (struct [**sstep**](structsstep.md) \* drv, const struct [**sstep\_config**](structsstep__config.md) \* cfg) <br>_Validate a configuration and initialise a driver._  |
|  int | [**sstep\_move\_by**](#function-sstep_move_by) (struct [**sstep**](structsstep.md) \* drv, int32\_t delta) <br>_Start a move relative to the current target._  |
|  int | [**sstep\_move\_to**](#function-sstep_move_to) (struct [**sstep**](structsstep.md) \* drv, int32\_t position) <br>_Start a move to an absolute position with the trapezoidal profile._  |
|  int | [**sstep\_report\_fault**](#function-sstep_report_fault) (struct [**sstep**](structsstep.md) \* drv, uint16\_t faults) <br>_Report a fault detected by the application (driver fault pin, temperature)._  |
|  int | [**sstep\_run**](#function-sstep_run) (struct [**sstep**](structsstep.md) \* drv, int32\_t velocity) <br>_Run at a constant velocity (accelerating with the configured limits) until another command._  |
|  int | [**sstep\_selftest**](#function-sstep_selftest) (struct [**sstep**](structsstep.md) \* drv, uint32\_t i\_test\_ma) <br>_Test the power stage and the motor wiring before the first enable._  |
|  int | [**sstep\_set\_current**](#function-sstep_set_current) (struct [**sstep**](structsstep.md) \* drv, uint32\_t run\_ma, uint32\_t hold\_ma) <br>_Change the run and hold currents._  |
|  int | [**sstep\_set\_motion\_limits**](#function-sstep_set_motion_limits) (struct [**sstep**](structsstep.md) \* drv, uint32\_t v\_max, uint32\_t accel, uint32\_t decel) <br>_Change the motion limits; they apply from the next_ [_**sstep\_slow\_loop()**_](group__sstep__api.md#function-sstep_slow_loop) _, also to a running command._ |
|  int | [**sstep\_set\_position**](#function-sstep_set_position) (struct [**sstep**](structsstep.md) \* drv, int32\_t position) <br>_Redefine the current position (homing)._  |
|  int | [**sstep\_slow\_loop**](#function-sstep_slow_loop) (struct [**sstep**](structsstep.md) \* drv) <br>_Slow loop: advance the trajectory, update the angle and current references, handle the hold current and report events._  |
|  int | [**sstep\_stop**](#function-sstep_stop) (struct [**sstep**](structsstep.md) \* drv) <br>_Decelerate to standstill with the configured deceleration and hold._  |




























## Detailed Description


**Execution model.** One driver instance (`struct sstep`) controls one motor. The application calls two functions periodically:



|Function   |Rate   |Context   |Work    |
|-----|-----|-----|-----|
|[**sstep\_fast\_loop()**](group__sstep__api.md#function-sstep_fast_loop)   |`fast_loop_hz` (PWM rate or an integer fraction of it)   |PWM / ADC interrupt   |current control, voltage limit, topology mapping, PWM write    |
|[**sstep\_slow\_loop()**](group__sstep__api.md#function-sstep_slow_loop)   |`slow_loop_hz` (typically 1 kHz)   |main loop or a timer task   |trajectory, microstep angle, hold current, fault handling   |






All other functions are called from the same context as [**sstep\_slow\_loop()**](group__sstep__api.md#function-sstep_slow_loop). The library never reads a clock, never blocks, allocates no memory and has no global state: time is given by the configured loop rates.


**Hardware access** is done only through the callbacks in [**sstep\_callbacks**](structsstep__callbacks.md), given once to [**sstep\_init()**](group__sstep__api.md#function-sstep_init). Measurements are passed to [**sstep\_fast\_loop()**](group__sstep__api.md#function-sstep_fast_loop) as raw ADC counts.


**Initialization.** Every function except [**sstep\_init()**](group__sstep__api.md#function-sstep_init) returns [**SSTEP\_ERR\_STATE**](group__sstep__defs.md#define-sstep_err_state) for an instance without a successful [**sstep\_init()**](group__sstep__api.md#function-sstep_init) (a zeroed instance, or one whose last [**sstep\_init()**](group__sstep__api.md#function-sstep_init) failed in the derivation of the constants), and touches no hardware. [**sstep\_fast\_loop()**](group__sstep__api.md#function-sstep_fast_loop) checks this only when [**SSTEP\_CFG\_ARG\_CHECKS**](group__sstep__config.md#define-sstep_cfg_arg_checks) is 1. 


    
## Public Functions Documentation




### function sstep\_clear\_faults 

_Clear all faults; the driver goes to_ SSTEP\_STATE\_DISABLED _._
```
int sstep_clear_faults (
    struct sstep * drv
) 
```





**Parameters:**


* `drv` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_disable 

_Stop immediately and switch the outputs off (motor coasts, no holding torque)._ 
```
int sstep_disable (
    struct sstep * drv
) 
```





**Parameters:**


* `drv` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_enable 

_Enable the outputs and hold the current position with the hold current._ 
```
int sstep_enable (
    struct sstep * drv
) 
```



Refused without a fault when the last bus voltage sample is outside the trip limits (no sample yet: not checked, the fast loop trips). 

**Parameters:**


* `drv` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (faulted or self-test running), [**SSTEP\_ERR\_VBUS**](group__sstep__defs.md#define-sstep_err_vbus). 




        

<hr>



### function sstep\_fast\_loop 

_Fast loop: convert the measurements, run the current controllers, limit the voltage, map it to the legs of the topology and write the PWM._ 
```
int sstep_fast_loop (
    struct sstep * drv,
    const struct sstep_sample * sample
) 
```



Call from the PWM/ADC interrupt at `fast_loop_hz`. Bounded execution time, no loops over data. 

**Parameters:**


* `drv` Driver instance. 
* `sample` Measurements of this period. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (only when [**SSTEP\_CFG\_ARG\_CHECKS**](group__sstep__config.md#define-sstep_cfg_arg_checks) is 1). 




        

<hr>



### function sstep\_get\_status 

_Read a status snapshot in engineering units._ 
```
int sstep_get_status (
    const struct sstep * drv,
    struct sstep_status * status
) 
```





**Parameters:**


* `drv` Driver instance. 
* `status` Filled on success. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_init 

_Validate a configuration and initialise a driver._ 
```
int sstep_init (
    struct sstep * drv,
    const struct sstep_config * cfg
) 
```



Outputs stay disabled. Derives the per-unit bases, the PI gains (Kp = L \* wc, Ki = R \* wc) and the trajectory constants; no floating point is used with the fixed point backend. 

**Parameters:**


* `drv` Driver instance. 
* `cfg` Configuration; copied. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) Success, state SSTEP\_STATE\_DISABLED. 
* [**SSTEP\_ERR\_PARAM**](group__sstep__defs.md#define-sstep_err_param) NULL pointer, missing required callback or a field out of range. 
* [**SSTEP\_ERR\_CONFIG**](group__sstep__defs.md#define-sstep_err_config) Inconsistent fields (for example i\_run\_ma &gt; i\_trip\_ma). 
* [**SSTEP\_ERR\_UNSUPPORTED**](group__sstep__defs.md#define-sstep_err_unsupported) Topology not compiled in, or stall\_time\_ms &gt; 0 built with [**SSTEP\_CFG\_STALL**](group__sstep__config.md#define-sstep_cfg_stall) = 0. 




        

<hr>



### function sstep\_move\_by 

_Start a move relative to the current target._ 
```
int sstep_move_by (
    struct sstep * drv,
    int32_t delta
) 
```





**Parameters:**


* `drv` Driver instance. 
* `delta` Distance [microsteps]. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (also when the target overflows), [**SSTEP\_ERR\_STATE**](group__sstep__defs.md#define-sstep_err_state). 




        

<hr>



### function sstep\_move\_to 

_Start a move to an absolute position with the trapezoidal profile._ 
```
int sstep_move_to (
    struct sstep * drv,
    int32_t position
) 
```



Replaces the current move; the change is smooth (no velocity step). 

**Parameters:**


* `drv` Driver instance. 
* `position` Target [microsteps]. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (disabled or faulted). 




        

<hr>



### function sstep\_report\_fault 

_Report a fault detected by the application (driver fault pin, temperature)._ 
```
int sstep_report_fault (
    struct sstep * drv,
    uint16_t faults
) 
```



Switches the outputs off. May be called from an interrupt. 

**Parameters:**


* `drv` Driver instance. 
* `faults` Flags to raise, typically [**SSTEP\_FAULT\_EXTERNAL**](group__sstep__defs.md#define-sstep_fault_external). 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_run 

_Run at a constant velocity (accelerating with the configured limits) until another command._ 
```
int sstep_run (
    struct sstep * drv,
    int32_t velocity
) 
```





**Parameters:**


* `drv` Driver instance. 
* `velocity` Signed velocity [microsteps/s], \|velocity\| &lt;= v\_max. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_selftest 

_Test the power stage and the motor wiring before the first enable._ 
```
int sstep_selftest (
    struct sstep * drv,
    uint32_t i_test_ma
) 
```



The outputs are switched on; for each phase in turn the fast loop raises the voltage slowly until the phase current reaches i\_test\_ma, holds it and lets it decay. It checks that no current flows with every leg low, that each phase reaches the current with the expected sign at a plausible duty cycle, and that the other phase stays below half of it. A rotor still moving into a step induces currents that break these conditions for a while: the hold and the decay go on until they have held for 2 ms and 5 ms in a row, for at most 100 ms each. Takes about 40 to 55 ms of fast loop periods (at most about 440 ms), then the outputs go off, SSTEP\_EVENT\_SELFTEST\_DONE is reported by [**sstep\_slow\_loop()**](group__sstep__api.md#function-sstep_slow_loop) and the result is in the status. A failure raises [**SSTEP\_FAULT\_SELFTEST**](group__sstep__defs.md#define-sstep_fault_selftest). The rotor moves to the nearest full step of the tested phase (at most one full step) and must be free to: an external torque that turns it fails the test. 

**Parameters:**


* `drv` Driver instance. 
* `i_test_ma` Test current [mA], 1 .. i\_run\_ma; typically 30 % of the rated current. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) The test runs. 
* [**SSTEP\_ERR\_PARAM**](group__sstep__defs.md#define-sstep_err_param) NULL pointer or i\_test\_ma out of range. 
* [**SSTEP\_ERR\_STATE**](group__sstep__defs.md#define-sstep_err_state) Not disabled, faulted, or a test is running. 
* [**SSTEP\_ERR\_VBUS**](group__sstep__defs.md#define-sstep_err_vbus) No bus voltage sample yet, or outside the trip limits. 
* [**SSTEP\_ERR\_UNSUPPORTED**](group__sstep__defs.md#define-sstep_err_unsupported) Built with [**SSTEP\_CFG\_SELFTEST**](group__sstep__config.md#define-sstep_cfg_selftest) = 0. 




        

<hr>



### function sstep\_set\_current 

_Change the run and hold currents._ 
```
int sstep_set_current (
    struct sstep * drv,
    uint32_t run_ma,
    uint32_t hold_ma
) 
```





**Parameters:**


* `drv` Driver instance. 
* `run_ma` Peak phase current while moving [mA]. 
* `hold_ma` Peak phase current at standstill [mA], &lt;= run\_ma. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (above the trip level). 




        

<hr>



### function sstep\_set\_motion\_limits 

_Change the motion limits; they apply from the next_ [_**sstep\_slow\_loop()**_](group__sstep__api.md#function-sstep_slow_loop) _, also to a running command._
```
int sstep_set_motion_limits (
    struct sstep * drv,
    uint32_t v_max,
    uint32_t accel,
    uint32_t decel
) 
```





**Parameters:**


* `drv` Driver instance. 
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
    struct sstep * drv,
    int32_t position
) 
```



Only at standstill. 

**Parameters:**


* `drv` Driver instance. 
* `position` New value of the current position [microsteps]. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (moving). 




        

<hr>



### function sstep\_slow\_loop 

_Slow loop: advance the trajectory, update the angle and current references, handle the hold current and report events._ 
```
int sstep_slow_loop (
    struct sstep * drv
) 
```



Call at `slow_loop_hz`. 

**Parameters:**


* `drv` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>



### function sstep\_stop 

_Decelerate to standstill with the configured deceleration and hold._ 
```
int sstep_stop (
    struct sstep * drv
) 
```





**Parameters:**


* `drv` Driver instance. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) 




        

<hr>

------------------------------


