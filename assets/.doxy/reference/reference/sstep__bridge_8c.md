

# File sstep\_bridge.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_bridge.c**](sstep__bridge_8c.md)

[Go to the source code of this file](sstep__bridge_8c_source.md)



* `#include "sstep_internal.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**sstep\_bridge\_map**](#function-sstep_bridge_map) (enum [**sstep\_topology**](group__sstep__defs.md#enum-sstep_topology) topology, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) va, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) vb, const struct [**sstep\_gain**](structsstep__gain.md) \* inv\_vbus, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) vbus, uint16\_t period, struct [**sstep\_bridge\_out**](structsstep__bridge__out.md) \* out) <br>_Map requested phase voltages to leg compare values (averaged power stage)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**acc\_max**](#function-acc_max) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) a, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) b) <br>_Larger of a and b._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**scale\_to\_unit**](#function-scale_to_unit) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) x, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) m) <br>_x / m for \|x\| &lt;= m, keeping the sign of x (one division)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**ACC\_HALF**](sstep__bridge_8c.md#define-acc_half)  `[**SSTEP\_ACC\_HALF**](group__sstep__math.md#define-sstep_acc_half)`<br>_0.5 (50 % duty cycle)._  |

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
## Public Static Functions Documentation




### function acc\_max 

_Larger of a and b._ 
```C++
static sstep_acc_t acc_max (
    sstep_acc_t a,
    sstep_acc_t b
) 
```




<hr>



### function scale\_to\_unit 

_x / m for \|x\| &lt;= m, keeping the sign of x (one division)._ 
```C++
static sstep_acc_t scale_to_unit (
    sstep_acc_t x,
    sstep_acc_t m
) 
```




<hr>
## Macro Definition Documentation





### define ACC\_HALF 

_0.5 (50 % duty cycle)._ 
```C++
#define ACC_HALF `SSTEP_ACC_HALF`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_bridge.c`

