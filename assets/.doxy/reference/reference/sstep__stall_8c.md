

# File sstep\_stall.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_stall.c**](sstep__stall_8c.md)

[Go to the source code of this file](sstep__stall_8c_source.md)



* `#include "sstep_internal.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  bool | [**sstep\_stall\_check**](#function-sstep_stall_check) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m) <br>_Stall decision of one slow loop period, against the command of the period that just ended (vel\_cmd, ff\_cmd)._  |
|  void | [**sstep\_stall\_observe**](#function-sstep_stall_observe) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m, const struct [**sstep\_sincos**](structsstep__sincos.md) \* sc, const [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) v\_applied) <br>_Back-EMF observer of one fast loop period (fast loop, current drive)._  |
|  void | [**sstep\_stall\_reset**](#function-sstep_stall_reset) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m) <br>_Reset the stall detector when the outputs are enabled: empty voltage history, filters at zero, current history from the last measurement._  |
|  void | [**sstep\_stall\_status**](#function-sstep_stall_status) (const struct [**sstep\_ctx**](structsstep__ctx.md) \* m, struct [**sstep\_status**](structsstep__status.md) \* status) <br>_Fill the observer fields of a status (zero when stall detection is off)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int64\_t | [**abs\_i64**](#function-abs_i64) (int64\_t v) <br>_\|v\| for \|v\| &lt; 2^63._  |
|  [**sstep\_angle\_t**](group__sstep__math.md#typedef-sstep_angle_t) | [**atan2\_turn**](#function-atan2_turn) (int32\_t y, int32\_t x) <br>_Angle of the vector (x, y), atan2(y, x), in 2^-32 turn (CORDIC, vectoring mode)._  |
|  int32\_t | [**emf\_mv**](#function-emf_mv) (const struct [**sstep\_ctx**](structsstep__ctx.md) \* m, uint32\_t axis) <br>_Filtered back-EMF of one axis (0 = d, 1 = q) [mV]._  |
|  uint64\_t | [**muldiv\_u64**](#function-muldiv_u64) (uint64\_t a, uint64\_t b, uint64\_t d) <br>_a \* b / d (truncated) without overflow for a &lt; 2^42, b &lt; 2^25, 0 &lt; d &lt; 2^35._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**CDEG\_PER\_TURN**](sstep__stall_8c.md#define-cdeg_per_turn)  `36000`<br>_Centidegrees per turn._  |
| define  | [**CORDIC\_NORM**](sstep__stall_8c.md#define-cordic_norm)  `((int32\_t)0x08000000)`<br>_The CORDIC operands are scaled up to at least 2^27 for resolution; the CORDIC gain (1.65 \* sqrt(2)) keeps them below 2^30._  |
| define  | [**CORDIC\_STEPS**](sstep__stall_8c.md#define-cordic_steps)  `16U`<br>_CORDIC iterations of atan2\_turn() (last step atan(2^-15) = 0.0017 deg)._  |
| define  | [**HALF\_TURN**](sstep__stall_8c.md#define-half_turn)  `0x80000000U`<br>_Half a turn of_ [_**sstep\_angle\_t**_](group__sstep__math.md#typedef-sstep_angle_t) _._ |
| define  | [**OFF\_ONE**](sstep__stall_8c.md#define-off_one)  `((int64\_t)65536)`<br>_Fixed point one of_ [_**sstep\_ctx::emf\_off**_](structsstep__ctx.md#variable-emf_off) _(mV \* 2^16)._ |
| define  | [**TURN**](sstep__stall_8c.md#define-turn)  `((int64\_t)4294967296)`<br>_One turn of_ [_**sstep\_angle\_t**_](group__sstep__math.md#typedef-sstep_angle_t) _as a 64-bit value._ |
| define  | [**TWO\_PI\_DEN**](sstep__stall_8c.md#define-two_pi_den)  `113U`<br> |
| define  | [**TWO\_PI\_NUM**](sstep__stall_8c.md#define-two_pi_num)  `710U`<br>_2 pi = 710 / 113, as in the gain derivation._  |

## Public Functions Documentation




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




### function abs\_i64 

_\|v\| for \|v\| &lt; 2^63._ 
```C++
static int64_t abs_i64 (
    int64_t v
) 
```




<hr>



### function atan2\_turn 

_Angle of the vector (x, y), atan2(y, x), in 2^-32 turn (CORDIC, vectoring mode)._ 
```C++
static sstep_angle_t atan2_turn (
    int32_t y,
    int32_t x
) 
```



0 for the zero vector. 

**Parameters:**


* `y` Components, \|x\|, \|y\| &lt; 2^27. 




        

<hr>



### function emf\_mv 

_Filtered back-EMF of one axis (0 = d, 1 = q) [mV]._ 
```C++
static int32_t emf_mv (
    const struct sstep_ctx * m,
    uint32_t axis
) 
```




<hr>



### function muldiv\_u64 

_a \* b / d (truncated) without overflow for a &lt; 2^42, b &lt; 2^25, 0 &lt; d &lt; 2^35._ 
```C++
static uint64_t muldiv_u64 (
    uint64_t a,
    uint64_t b,
    uint64_t d
) 
```




<hr>
## Macro Definition Documentation





### define CDEG\_PER\_TURN 

_Centidegrees per turn._ 
```C++
#define CDEG_PER_TURN `36000`
```




<hr>



### define CORDIC\_NORM 

_The CORDIC operands are scaled up to at least 2^27 for resolution; the CORDIC gain (1.65 \* sqrt(2)) keeps them below 2^30._ 
```C++
#define CORDIC_NORM `((int32_t)0x08000000)`
```




<hr>



### define CORDIC\_STEPS 

_CORDIC iterations of atan2\_turn() (last step atan(2^-15) = 0.0017 deg)._ 
```C++
#define CORDIC_STEPS `16U`
```




<hr>



### define HALF\_TURN 

_Half a turn of_ [_**sstep\_angle\_t**_](group__sstep__math.md#typedef-sstep_angle_t) _._
```C++
#define HALF_TURN `0x80000000U`
```




<hr>



### define OFF\_ONE 

_Fixed point one of_ [_**sstep\_ctx::emf\_off**_](structsstep__ctx.md#variable-emf_off) _(mV \* 2^16)._
```C++
#define OFF_ONE `((int64_t)65536)`
```




<hr>



### define TURN 

_One turn of_ [_**sstep\_angle\_t**_](group__sstep__math.md#typedef-sstep_angle_t) _as a 64-bit value._
```C++
#define TURN `((int64_t)4294967296)`
```




<hr>



### define TWO\_PI\_DEN 

```C++
#define TWO_PI_DEN `113U`
```




<hr>



### define TWO\_PI\_NUM 

_2 pi = 710 / 113, as in the gain derivation._ 
```C++
#define TWO_PI_NUM `710U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_stall.c`

