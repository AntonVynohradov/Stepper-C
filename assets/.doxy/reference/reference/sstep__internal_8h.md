

# File sstep\_internal.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_internal.h**](sstep__internal_8h.md)

[Go to the source code of this file](sstep__internal_8h_source.md)



* `#include <stdbool.h>`
* `#include <stdint.h>`
* `#include "sstep_ctrl.h"`
* `#include "sstep_math.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**sstep\_bridge\_out**](structsstep__bridge__out.md) <br>_Result of the power stage mapping._  |
| struct | [**sstep\_ctx**](structsstep__ctx.md) <br>_Driver state behind the opaque_ [_**sstep**_](structsstep.md) _._ |
| struct | [**sstep\_motion**](structsstep__motion.md) <br>_Motion generator state._  |
| struct | [**sstep\_setpoint**](structsstep__setpoint.md) <br>_Setpoint handed from the slow loop to the fast loop._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**sstep\_bridge\_map**](#function-sstep_bridge_map) (enum [**sstep\_topology**](group__sstep__defs.md#enum-sstep_topology) topology, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) va, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) vb, const struct [**sstep\_gain**](structsstep__gain.md) \* inv\_vbus, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) vbus, uint16\_t period, struct [**sstep\_bridge\_out**](structsstep__bridge__out.md) \* out) <br>_Map requested phase voltages to leg compare values (averaged power stage)._  |
|  uint32\_t | [**sstep\_isqrt64**](#function-sstep_isqrt64) (uint64\_t x) <br>_Integer square root: floor(sqrt(x))._  |
|  uint8\_t | [**sstep\_motion\_step**](#function-sstep_motion_step) (struct [**sstep\_motion**](structsstep__motion.md) \* mo) <br>_One slow loop period of the motion generator: trapezoidal velocity profile with one-period look-ahead braking for moves, exact stop on the target._  |
|  bool | [**sstep\_stall\_check**](#function-sstep_stall_check) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m) <br>_Stall decision of one slow loop period, against the command of the period that just ended (vel\_cmd, ff\_cmd)._  |
|  void | [**sstep\_stall\_observe**](#function-sstep_stall_observe) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m, const struct [**sstep\_sincos**](structsstep__sincos.md) \* sc, const [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) v\_applied) <br>_Back-EMF observer of one fast loop period (fast loop, current drive)._  |
|  void | [**sstep\_stall\_reset**](#function-sstep_stall_reset) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m) <br>_Reset the stall detector when the outputs are enabled: empty voltage history, filters at zero, current history from the last measurement._  |
|  void | [**sstep\_stall\_status**](#function-sstep_stall_status) (const struct [**sstep\_ctx**](structsstep__ctx.md) \* m, struct [**sstep\_status**](structsstep__status.md) \* status) <br>_Fill the observer fields of a status (zero when stall detection is off)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  struct [**sstep\_ctx**](structsstep__ctx.md) \* | [**sstep\_ctx\_of**](#function-sstep_ctx_of) (struct [**sstep**](structsstep.md) \* drv) <br>_Driver state of an instance._  |
|  const struct [**sstep\_ctx**](structsstep__ctx.md) \* | [**sstep\_ctx\_of\_const**](#function-sstep_ctx_of_const) (const struct [**sstep**](structsstep.md) \* drv) <br>_Read-only variant of sstep\_ctx\_of()._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SSTEP\_MODE\_HOLD**](sstep__internal_8h.md#define-sstep_mode_hold)  `0U`<br>_Motion generator modes._  |
| define  | [**SSTEP\_MODE\_MOVE**](sstep__internal_8h.md#define-sstep_mode_move)  `1U`<br> |
| define  | [**SSTEP\_MODE\_RUN**](sstep__internal_8h.md#define-sstep_mode_run)  `2U`<br> |
| define  | [**SSTEP\_MOTION\_DONE**](sstep__internal_8h.md#define-sstep_motion_done)  `0x01U`<br>_Result flags of sstep\_motion\_step()._  |
| define  | [**SSTEP\_POS\_ONE**](sstep__internal_8h.md#define-sstep_pos_one)  `((int64\_t)65536)`<br>_Fixed point position: 1 microstep = 2^16._  |
| define  | [**SSTEP\_POS\_SNAP**](sstep__internal_8h.md#define-sstep_pos_snap)  `((int64\_t)64)`<br>_A move ends when it is closer than this to the target and slow enough (1/1024 microstep)._  |
| define  | [**SSTEP\_READY**](sstep__internal_8h.md#define-sstep_ready)  `0x53545043U`<br>_Value of_ [_**sstep\_ctx::ready**_](structsstep__ctx.md#variable-ready) _after a successful_[_**sstep\_init()**_](group__sstep__api.md#function-sstep_init) _("STPC")._ |

## Public Functions Documentation




### function sstep\_bridge\_map 

_Map requested phase voltages to leg compare values (averaged power stage)._ 
```C++
void sstep_bridge_map (
    enum sstep_topology topology,
    sstep_acc_t va,
    sstep_acc_t vb,
    const struct sstep_gain * inv_vbus,
    sstep_acc_t vbus,
    uint16_t period,
    struct sstep_bridge_out * out
) 
```



The voltage vector is scaled down with its direction kept when the stage cannot produce it: full bridge \|a\|, \|b\| &lt;= 1, shared leg max(a,b,0) - min(a,b,0) &lt;= 1, a and b relative to the measured bus voltage. Shared leg: common leg centred. 

**Parameters:**


* `topology` Power stage topology (compiled in). 
* `va` Requested phase voltages [pu of V\_base]. 
* `inv_vbus` V\_base / bus voltage. 
* `vbus` Bus voltage [pu of V\_base], 0 .. 2. 
* `period` Compare value for 100 % duty cycle. 
* `out` Result. 




        

<hr>



### function sstep\_isqrt64 

_Integer square root: floor(sqrt(x))._ 
```C++
uint32_t sstep_isqrt64 (
    uint64_t x
) 
```





**Parameters:**


* `x` Value. 



**Returns:**

floor(sqrt(x)). 





        

<hr>



### function sstep\_motion\_step 

_One slow loop period of the motion generator: trapezoidal velocity profile with one-period look-ahead braking for moves, exact stop on the target._ 
```C++
uint8_t sstep_motion_step (
    struct sstep_motion * mo
) 
```





**Parameters:**


* `mo` Motion state; pos and vel are updated. 



**Returns:**

SSTEP\_MOTION\_DONE when a move reached its target in this period, else 0. 





        

<hr>



### function sstep\_stall\_check 

_Stall decision of one slow loop period, against the command of the period that just ended (vel\_cmd, ff\_cmd)._ 
```C++
bool sstep_stall_check (
    struct sstep_ctx * m
) 
```





**Parameters:**


* `m` Driver state. 



**Returns:**

true when the stall condition has persisted for stall\_time\_ms. 





        

<hr>



### function sstep\_stall\_observe 

_Back-EMF observer of one fast loop period (fast loop, current drive)._ 
```C++
void sstep_stall_observe (
    struct sstep_ctx * m,
    const struct sstep_sincos * sc,
    const sstep_acc_t v_applied
) 
```



Uses the voltage computed two periods ago (applied during the period that just ended), the measured currents and the commanded angle, then stores v\_applied. 

**Parameters:**


* `m` Driver state; i\_meas holds the currents of this period. 
* `sc` Sine and cosine of the commanded angle of this period. 
* `v_applied` Phase voltages computed in this period [pu V]. 




        

<hr>



### function sstep\_stall\_reset 

_Reset the stall detector when the outputs are enabled: empty voltage history, filters at zero, current history from the last measurement._ 
```C++
void sstep_stall_reset (
    struct sstep_ctx * m
) 
```





**Parameters:**


* `m` Driver state. 




        

<hr>



### function sstep\_stall\_status 

_Fill the observer fields of a status (zero when stall detection is off)._ 
```C++
void sstep_stall_status (
    const struct sstep_ctx * m,
    struct sstep_status * status
) 
```





**Parameters:**


* `m` Driver state. 
* `status` Status to complete. 




        

<hr>
## Public Static Functions Documentation




### function sstep\_ctx\_of 

_Driver state of an instance._ 
```C++
static inline struct sstep_ctx * sstep_ctx_of (
    struct sstep * drv
) 
```





**Note:**

MISRA C:2012 Rule 11.3, deviation DEV-01 (docs/misra.md): [**sstep**](structsstep.md) is the opaque storage of [**sstep\_ctx**](structsstep__ctx.md); size and alignment are checked in sstep\_ctrl.c. 




**Parameters:**


* `drv` Driver instance, may be NULL. 



**Returns:**

Driver state, NULL if drv is NULL. 





        

<hr>



### function sstep\_ctx\_of\_const 

_Read-only variant of sstep\_ctx\_of()._ 
```C++
static inline const struct sstep_ctx * sstep_ctx_of_const (
    const struct sstep * drv
) 
```




<hr>
## Macro Definition Documentation





### define SSTEP\_MODE\_HOLD 

_Motion generator modes._ 
```C++
#define SSTEP_MODE_HOLD `0U`
```




<hr>



### define SSTEP\_MODE\_MOVE 

```C++
#define SSTEP_MODE_MOVE `1U`
```




<hr>



### define SSTEP\_MODE\_RUN 

```C++
#define SSTEP_MODE_RUN `2U`
```




<hr>



### define SSTEP\_MOTION\_DONE 

_Result flags of sstep\_motion\_step()._ 
```C++
#define SSTEP_MOTION_DONE `0x01U`
```




<hr>



### define SSTEP\_POS\_ONE 

_Fixed point position: 1 microstep = 2^16._ 
```C++
#define SSTEP_POS_ONE `((int64_t)65536)`
```




<hr>



### define SSTEP\_POS\_SNAP 

_A move ends when it is closer than this to the target and slow enough (1/1024 microstep)._ 
```C++
#define SSTEP_POS_SNAP `((int64_t)64)`
```




<hr>



### define SSTEP\_READY 

_Value of_ [_**sstep\_ctx::ready**_](structsstep__ctx.md#variable-ready) _after a successful_[_**sstep\_init()**_](group__sstep__api.md#function-sstep_init) _("STPC")._
```C++
#define SSTEP_READY `0x53545043U`
```



A zeroed or never initialized instance holds another value: every function but [**sstep\_init()**](group__sstep__api.md#function-sstep_init) then returns [**SSTEP\_ERR\_STATE**](group__sstep__defs.md#define-sstep_err_state) instead of using an empty configuration. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_internal.h`

