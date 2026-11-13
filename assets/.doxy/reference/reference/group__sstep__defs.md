

# Group sstep\_defs



[**Modules**](modules.md) **>** [**sstep\_defs**](group__sstep__defs.md)



_Constants shared by the public API._ [More...](#detailed-description)


















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**sstep\_selftest\_result**](#enum-sstep_selftest_result)  <br>_Result of_ [_**sstep\_selftest()**_](group__sstep__api.md#function-sstep_selftest) _, in_[_**sstep\_status::selftest**_](structsstep__status.md#variable-selftest) _._ |
| enum  | [**sstep\_state**](#enum-sstep_state)  <br>_Driver state._  |
| enum  | [**sstep\_topology**](#enum-sstep_topology)  <br>_Power stage topology._  |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SSTEP\_ERR\_CONFIG**](group__sstep__defs.md#define-sstep_err_config)  `(-2)`<br>_Configuration is consistent field by field but not as a whole (for example rated current above the power stage limit)._  |
| define  | [**SSTEP\_ERR\_PARAM**](group__sstep__defs.md#define-sstep_err_param)  `(-1)`<br>_NULL pointer or a value outside its documented range._  |
| define  | [**SSTEP\_ERR\_STATE**](group__sstep__defs.md#define-sstep_err_state)  `(-3)`<br>_Call not allowed in the current state (for example a move while faulted)._  |
| define  | [**SSTEP\_ERR\_UNSUPPORTED**](group__sstep__defs.md#define-sstep_err_unsupported)  `(-4)`<br>_Topology, self-test or stall detection not compiled in (see_ [_**SSTEP\_CFG\_TOPOLOGY\_FULL\_BRIDGE**_](group__sstep__config.md#define-sstep_cfg_topology_full_bridge) _,_[_**SSTEP\_CFG\_SELFTEST**_](group__sstep__config.md#define-sstep_cfg_selftest) _,_[_**SSTEP\_CFG\_STALL**_](group__sstep__config.md#define-sstep_cfg_stall) _)._ |
| define  | [**SSTEP\_ERR\_VBUS**](group__sstep__defs.md#define-sstep_err_vbus)  `(-5)`<br>_The last bus voltage sample is outside vbus\_min\_mv ._  |
| define  | [**SSTEP\_FAULT\_EXTERNAL**](group__sstep__defs.md#define-sstep_fault_external)  `0x0010U`<br>_Reported by the application through_ [_**sstep\_report\_fault()**_](group__sstep__api.md#function-sstep_report_fault) _(driver fault pin, over-temperature...)._ |
| define  | [**SSTEP\_FAULT\_HAL**](group__sstep__defs.md#define-sstep_fault_hal)  `0x0008U`<br>_A hardware callback returned an error._  |
| define  | [**SSTEP\_FAULT\_NONE**](group__sstep__defs.md#define-sstep_fault_none)  `0x0000U`<br>_Fault flags reported by_ [_**sstep\_get\_status()**_](group__sstep__api.md#function-sstep_get_status) _and the fault callback._ |
| define  | [**SSTEP\_FAULT\_OVERCURRENT**](group__sstep__defs.md#define-sstep_fault_overcurrent)  `0x0001U`<br>_Measured phase current above the configured trip level._  |
| define  | [**SSTEP\_FAULT\_OVERVOLTAGE**](group__sstep__defs.md#define-sstep_fault_overvoltage)  `0x0002U`<br>_Bus voltage above the configured maximum._  |
| define  | [**SSTEP\_FAULT\_SELFTEST**](group__sstep__defs.md#define-sstep_fault_selftest)  `0x0040U`<br>[_**sstep\_selftest()**_](group__sstep__api.md#function-sstep_selftest) _found the power stage or the motor wiring faulty; the result is in_[_**sstep\_status::selftest**_](structsstep__status.md#variable-selftest) _._ |
| define  | [**SSTEP\_FAULT\_STALL**](group__sstep__defs.md#define-sstep_fault_stall)  `0x0020U`<br>_The rotor does not follow the commanded electrical angle: lost synchronism in motion, or rotor motion while the command is at rest (sensorless back-EMF observer, enabled with_ [_**sstep\_ctrl\_cfg::stall\_time\_ms**_](structsstep__ctrl__cfg.md#variable-stall_time_ms) _)._ |
| define  | [**SSTEP\_FAULT\_UNDERVOLTAGE**](group__sstep__defs.md#define-sstep_fault_undervoltage)  `0x0004U`<br>_Bus voltage below the configured minimum._  |
| define  | [**SSTEP\_LEGS\_MAX**](group__sstep__defs.md#define-sstep_legs_max)  `4U`<br>_Number of half-bridge legs of the largest supported topology._  |
| define  | [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok)  `0`<br>_Success._  |

## Detailed Description


**Units.** Configuration values are integers in engineering units (milliohm, microhenry, milliampere, millivolt, hertz, nanosecond) so that [**sstep\_init()**](group__sstep__api.md#function-sstep_init) needs no floating point with the fixed point backend. The unit is part of every field name (`r_mohm`, `l_uh`...). Positions are in microsteps, velocities in microsteps per second.


**Sign convention.** A positive phase voltage drives a positive phase current; positive velocity increases the position. Phase A leads phase B by 90 electrical degrees for positive motion. 


    
## Public Types Documentation




### enum sstep\_selftest\_result 

_Result of_ [_**sstep\_selftest()**_](group__sstep__api.md#function-sstep_selftest) _, in_[_**sstep\_status::selftest**_](structsstep__status.md#variable-selftest) _._
```
enum sstep_selftest_result {
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
```




<hr>



### enum sstep\_state 

_Driver state._ 
```
enum sstep_state {
    SSTEP_STATE_DISABLED = 0,
    SSTEP_STATE_HOLDING = 1,
    SSTEP_STATE_MOVING = 2,
    SSTEP_STATE_FAULT = 3
};
```




<hr>



### enum sstep\_topology 

_Power stage topology._ 
```
enum sstep_topology {
    SSTEP_TOPOLOGY_FULL_BRIDGE = 0,
    SSTEP_TOPOLOGY_SHARED_LEG = 1
};
```




<hr>
## Macro Definition Documentation





### define SSTEP\_ERR\_CONFIG 

_Configuration is consistent field by field but not as a whole (for example rated current above the power stage limit)._ 
```
#define SSTEP_ERR_CONFIG `(-2)`
```




<hr>



### define SSTEP\_ERR\_PARAM 

_NULL pointer or a value outside its documented range._ 
```
#define SSTEP_ERR_PARAM `(-1)`
```




<hr>



### define SSTEP\_ERR\_STATE 

_Call not allowed in the current state (for example a move while faulted)._ 
```
#define SSTEP_ERR_STATE `(-3)`
```




<hr>



### define SSTEP\_ERR\_UNSUPPORTED 

_Topology, self-test or stall detection not compiled in (see_ [_**SSTEP\_CFG\_TOPOLOGY\_FULL\_BRIDGE**_](group__sstep__config.md#define-sstep_cfg_topology_full_bridge) _,_[_**SSTEP\_CFG\_SELFTEST**_](group__sstep__config.md#define-sstep_cfg_selftest) _,_[_**SSTEP\_CFG\_STALL**_](group__sstep__config.md#define-sstep_cfg_stall) _)._
```
#define SSTEP_ERR_UNSUPPORTED `(-4)`
```




<hr>



### define SSTEP\_ERR\_VBUS 

_The last bus voltage sample is outside vbus\_min\_mv ._ 
```
#define SSTEP_ERR_VBUS `(-5)`
```



. vbus\_max\_mv: the outputs stay off, no fault is latched ([**sstep\_enable()**](group__sstep__api.md#function-sstep_enable), [**sstep\_selftest()**](group__sstep__api.md#function-sstep_selftest)). 


        

<hr>



### define SSTEP\_FAULT\_EXTERNAL 

_Reported by the application through_ [_**sstep\_report\_fault()**_](group__sstep__api.md#function-sstep_report_fault) _(driver fault pin, over-temperature...)._
```
#define SSTEP_FAULT_EXTERNAL `0x0010U`
```




<hr>



### define SSTEP\_FAULT\_HAL 

_A hardware callback returned an error._ 
```
#define SSTEP_FAULT_HAL `0x0008U`
```




<hr>



### define SSTEP\_FAULT\_NONE 

_Fault flags reported by_ [_**sstep\_get\_status()**_](group__sstep__api.md#function-sstep_get_status) _and the fault callback._
```
#define SSTEP_FAULT_NONE `0x0000U`
```



A fault switches the outputs off; [**sstep\_clear\_faults()**](group__sstep__api.md#function-sstep_clear_faults) re-arms the driver. 


        

<hr>



### define SSTEP\_FAULT\_OVERCURRENT 

_Measured phase current above the configured trip level._ 
```
#define SSTEP_FAULT_OVERCURRENT `0x0001U`
```




<hr>



### define SSTEP\_FAULT\_OVERVOLTAGE 

_Bus voltage above the configured maximum._ 
```
#define SSTEP_FAULT_OVERVOLTAGE `0x0002U`
```




<hr>



### define SSTEP\_FAULT\_SELFTEST 

[_**sstep\_selftest()**_](group__sstep__api.md#function-sstep_selftest) _found the power stage or the motor wiring faulty; the result is in_[_**sstep\_status::selftest**_](structsstep__status.md#variable-selftest) _._
```
#define SSTEP_FAULT_SELFTEST `0x0040U`
```




<hr>



### define SSTEP\_FAULT\_STALL 

_The rotor does not follow the commanded electrical angle: lost synchronism in motion, or rotor motion while the command is at rest (sensorless back-EMF observer, enabled with_ [_**sstep\_ctrl\_cfg::stall\_time\_ms**_](structsstep__ctrl__cfg.md#variable-stall_time_ms) _)._
```
#define SSTEP_FAULT_STALL `0x0020U`
```




<hr>



### define SSTEP\_FAULT\_UNDERVOLTAGE 

_Bus voltage below the configured minimum._ 
```
#define SSTEP_FAULT_UNDERVOLTAGE `0x0004U`
```




<hr>



### define SSTEP\_LEGS\_MAX 

_Number of half-bridge legs of the largest supported topology._ 
```
#define SSTEP_LEGS_MAX `4U`
```




<hr>



### define SSTEP\_OK 

_Success._ 
```
#define SSTEP_OK `0`
```




<hr>

------------------------------


