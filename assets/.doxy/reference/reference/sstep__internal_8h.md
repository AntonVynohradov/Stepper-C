

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






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**sstep\_bridge\_map**](#function-sstep_bridge_map) (enum [**sstep\_topology**](group__sstep__defs.md#enum-sstep_topology) topology, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) va, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) vb, const struct [**sstep\_gain**](structsstep__gain.md) \* inv\_vbus, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) vbus, uint16\_t period, struct [**sstep\_bridge\_out**](structsstep__bridge__out.md) \* out) <br>_Map requested phase voltages to leg compare values (averaged power stage)._  |
|  uint32\_t | [**sstep\_isqrt64**](#function-sstep_isqrt64) (uint64\_t x) <br>_Integer square root: floor(sqrt(x))._  |
|  uint8\_t | [**sstep\_motion\_step**](#function-sstep_motion_step) (struct [**sstep\_motion**](structsstep__motion.md) \* mo) <br>_One slow loop period of the motion generator: trapezoidal velocity profile with one-period look-ahead braking for moves, exact stop on the target._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SSTEP\_MODE\_HOLD**](sstep__internal_8h.md#define-sstep_mode_hold)  `0U`<br>_Motion generator modes._  |
| define  | [**SSTEP\_MODE\_MOVE**](sstep__internal_8h.md#define-sstep_mode_move)  `1U`<br> |
| define  | [**SSTEP\_MODE\_RUN**](sstep__internal_8h.md#define-sstep_mode_run)  `2U`<br> |
| define  | [**SSTEP\_MOTION\_DONE**](sstep__internal_8h.md#define-sstep_motion_done)  `0x01U`<br>_Result flags of sstep\_motion\_step()._  |
| define  | [**SSTEP\_POS\_ONE**](sstep__internal_8h.md#define-sstep_pos_one)  `((int64\_t)65536)`<br>_Fixed point position: 1 microstep = 2^16._  |
| define  | [**SSTEP\_POS\_SNAP**](sstep__internal_8h.md#define-sstep_pos_snap)  `((int64\_t)64)`<br>_A move ends when it is closer than this to the target and slow enough (1/1024 microstep)._  |

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

------------------------------
The documentation for this class was generated from the following file `src/sstep_internal.h`

