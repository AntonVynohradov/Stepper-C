

# Group sstep\_defs



[**Modules**](modules.md) **>** [**sstep\_defs**](group__sstep__defs.md)



_Constants shared by the public API._ [More...](#detailed-description)


















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**sstep\_state**](#enum-sstep_state)  <br>_Driver state._  |
| enum  | [**sstep\_topology**](#enum-sstep_topology)  <br>_Power stage topology._  |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SSTEP\_ERR\_CONFIG**](group__sstep__defs.md#define-sstep_err_config)  `(-2)`<br>_Configuration is consistent field by field but not as a whole (for example rated current above the power stage limit)._  |
| define  | [**SSTEP\_ERR\_PARAM**](group__sstep__defs.md#define-sstep_err_param)  `(-1)`<br>_NULL pointer or a value outside its documented range._  |
| define  | [**SSTEP\_ERR\_STATE**](group__sstep__defs.md#define-sstep_err_state)  `(-3)`<br>_Call not allowed in the current state (for example a move while faulted)._  |
| define  | [**SSTEP\_ERR\_UNSUPPORTED**](group__sstep__defs.md#define-sstep_err_unsupported)  `(-4)`<br>_Topology not compiled in (see_ [_**SSTEP\_CFG\_TOPOLOGY\_FULL\_BRIDGE**_](group__sstep__config.md#define-sstep_cfg_topology_full_bridge) _)._ |
| define  | [**SSTEP\_FAULT\_EXTERNAL**](group__sstep__defs.md#define-sstep_fault_external)  `0x0010U`<br>_Reported by the application through_ [_**sstep\_report\_fault()**_](group__sstep__ctrl.md#function-sstep_report_fault) _(driver fault pin, over-temperature...)._ |
| define  | [**SSTEP\_FAULT\_HAL**](group__sstep__defs.md#define-sstep_fault_hal)  `0x0008U`<br>_A hardware callback returned an error._  |
| define  | [**SSTEP\_FAULT\_NONE**](group__sstep__defs.md#define-sstep_fault_none)  `0x0000U`<br>_Fault flags reported by_ [_**sstep\_get\_status()**_](group__sstep__ctrl.md#function-sstep_get_status) _and the fault callback._ |
| define  | [**SSTEP\_FAULT\_OVERCURRENT**](group__sstep__defs.md#define-sstep_fault_overcurrent)  `0x0001U`<br>_Measured phase current above the configured trip level._  |
| define  | [**SSTEP\_FAULT\_OVERVOLTAGE**](group__sstep__defs.md#define-sstep_fault_overvoltage)  `0x0002U`<br>_Bus voltage above the configured maximum._  |
| define  | [**SSTEP\_FAULT\_UNDERVOLTAGE**](group__sstep__defs.md#define-sstep_fault_undervoltage)  `0x0004U`<br>_Bus voltage below the configured minimum._  |
| define  | [**SSTEP\_LEGS\_MAX**](group__sstep__defs.md#define-sstep_legs_max)  `4U`<br>_Number of half-bridge legs of the largest supported topology._  |
| define  | [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok)  `0`<br>_Success._  |

## Detailed Description


**Units.** Configuration values are integers in engineering units (milliohm, microhenry, milliampere, millivolt, hertz, nanosecond) so that [**sstep\_init()**](group__sstep__ctrl.md#function-sstep_init) needs no floating point with the fixed point backend. The unit is part of every field name (`r_mohm`, `l_uh`...). Positions are in microsteps, velocities in microsteps per second.


**Sign convention.** A positive phase voltage drives a positive phase current; positive velocity increases the position. Phase A leads phase B by 90 electrical degrees for positive motion. 


    
## Public Types Documentation




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

_Topology not compiled in (see_ [_**SSTEP\_CFG\_TOPOLOGY\_FULL\_BRIDGE**_](group__sstep__config.md#define-sstep_cfg_topology_full_bridge) _)._
```
#define SSTEP_ERR_UNSUPPORTED `(-4)`
```




<hr>



### define SSTEP\_FAULT\_EXTERNAL 

_Reported by the application through_ [_**sstep\_report\_fault()**_](group__sstep__ctrl.md#function-sstep_report_fault) _(driver fault pin, over-temperature...)._
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

_Fault flags reported by_ [_**sstep\_get\_status()**_](group__sstep__ctrl.md#function-sstep_get_status) _and the fault callback._
```
#define SSTEP_FAULT_NONE `0x0000U`
```



A fault switches the outputs off; [**sstep\_clear\_faults()**](group__sstep__ctrl.md#function-sstep_clear_faults) re-arms the driver. 


        

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


