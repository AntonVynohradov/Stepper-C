

# File sstep\_ctrl.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_ctrl.c**](sstep__ctrl_8c.md)

[Go to the source code of this file](sstep__ctrl_8c_source.md)



* `#include "sstep.h"`
* `#include <stddef.h>`
* `#include "sstep_internal.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**align\_ctx**](structalign__ctx.md) <br> |
| struct | [**align\_pub**](structalign__pub.md) <br> |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int | [**check\_config**](#function-check_config) (const struct [**sstep\_config**](structsstep__config.md) \* cfg) <br>_Validate a configuration field by field and as a whole._  |
|  int | [**check\_stall**](#function-check_stall) (const struct [**sstep\_config**](structsstep__config.md) \* cfg) <br>_Validate the stall detection fields (ignored when stall\_time\_ms is 0)._  |
|  void | [**control**](#function-control) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m) <br>_Current control, power stage and PWM write of one fast loop period._  |
|  int | [**derive**](#function-derive) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m) <br>_Derive gains, scaling and limits from the (validated) configuration._  |
|  int | [**derive\_stall**](#function-derive_stall) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m) <br>_Derive the stall detection constants (after the other gains)._  |
|  int64\_t | [**div\_round**](#function-div_round) (int64\_t num, int64\_t den) <br>_round(num / den) for den &gt; 0, half away from zero._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**gain\_apply\_i32**](#function-gain_apply_i32) (const struct [**sstep\_gain**](structsstep__gain.md) \* g, int32\_t v) <br>_g \* v for a 32-bit integer v (slow loop, 64-bit product), as a saturated signal._  |
|  bool | [**is\_active**](#function-is_active) (enum [**sstep\_state**](group__sstep__defs.md#enum-sstep_state) s) <br>_Outputs are on._  |
|  bool | [**is\_moving**](#function-is_moving) (const struct [**sstep\_ctx**](structsstep__ctx.md) \* m) <br>_A command is executing._  |
|  bool | [**is\_pow2**](#function-is_pow2) (uint32\_t v) <br>_v is a power of two (v &gt; 0)._  |
|  uint8\_t | [**log2\_u32**](#function-log2_u32) (uint32\_t v) <br>_log2 of a power of two._  |
|  bool | [**mul\_ok**](#function-mul_ok) (uint64\_t a, uint64\_t b, uint64\_t \* out) <br>_out = a \* b; false when the product reaches 2^62._  |
|  int64\_t | [**pow2\_i64**](#function-pow2_i64) (uint32\_t n) <br>_2^n for n = 0 ._  |
|  void | [**publish**](#function-publish) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m, const struct [**sstep\_setpoint**](structsstep__setpoint.md) \* sp) <br>_Hand a setpoint to the fast loop (slow loop context)._  |
|  int | [**ratio**](#function-ratio) (uint64\_t n1, uint64\_t n2, uint64\_t d1, uint64\_t d2, struct [**sstep\_gain**](structsstep__gain.md) \* g) <br>_g = (n1 n2) / (d1 d2) with overflow checks (configuration only)._  |
|  [**sstep\_angle\_t**](group__sstep__math.md#typedef-sstep_angle_t) | [**raw\_angle**](#function-raw_angle) (const struct [**sstep\_ctx**](structsstep__ctx.md) \* m, int64\_t pos) <br>_Electrical angle of a position [Q16 microsteps]: microstep = 2^30 / M._  |
|  int | [**set\_currents**](#function-set_currents) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m, uint32\_t run\_ma, uint32\_t hold\_ma) <br>_Validate and store the run and hold currents._  |
|  int | [**set\_limits**](#function-set_limits) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m, uint32\_t v\_max, uint32\_t accel, uint32\_t decel) <br>_Validate and store the motion limits (engineering units -&gt; Q16 per period)._  |
|  void | [**shutdown**](#function-shutdown) (struct [**sstep\_ctx**](structsstep__ctx.md) \* m, uint16\_t faults) <br>_Switch the outputs off and enter the fault state (any context)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**Q16\_LIMIT**](sstep__ctrl_8c.md#define-q16_limit)  `((int64\_t)0x40000000)`<br>_Velocities and accelerations in Q16 per period stay below 2^30._  |
| define  | [**RATIO\_LIMIT**](sstep__ctrl_8c.md#define-ratio_limit)  `((uint64\_t)1U &lt;&lt; 62U)`<br>_Operand limit of the gain ratios (sstep\_gain\_from\_ratio64)._  |
| define  | [**TWO\_PI\_DEN**](sstep__ctrl_8c.md#define-two_pi_den)  `113U`<br> |
| define  | [**TWO\_PI\_NUM**](sstep__ctrl_8c.md#define-two_pi_num)  `710U`<br>_2 pi = 710 / 113 (error 8.5e-8)._  |
| define  | [**VALUE\_MAX**](sstep__ctrl_8c.md#define-value_max)  `10000000U`<br>_Largest engineering value accepted in the motor and stage configuration._  |
| define  | [**VBUS\_PU\_MAX**](sstep__ctrl_8c.md#define-vbus_pu_max)  `[**SSTEP\_ACC\_SCALE\_MAX**](group__sstep__math.md#define-sstep_acc_scale_max)`<br>_Bus voltage limit of the fast loop: 2 pu - 2^-15._  |

## Public Static Functions Documentation




### function check\_config 

_Validate a configuration field by field and as a whole._ 
```C++
static int check_config (
    const struct sstep_config * cfg
) 
```




<hr>



### function check\_stall 

_Validate the stall detection fields (ignored when stall\_time\_ms is 0)._ 
```C++
static int check_stall (
    const struct sstep_config * cfg
) 
```




<hr>



### function control 

_Current control, power stage and PWM write of one fast loop period._ 
```C++
static void control (
    struct sstep_ctx * m
) 
```




<hr>



### function derive 

_Derive gains, scaling and limits from the (validated) configuration._ 
```C++
static int derive (
    struct sstep_ctx * m
) 
```




<hr>



### function derive\_stall 

_Derive the stall detection constants (after the other gains)._ 
```C++
static int derive_stall (
    struct sstep_ctx * m
) 
```




<hr>



### function div\_round 

_round(num / den) for den &gt; 0, half away from zero._ 
```C++
static int64_t div_round (
    int64_t num,
    int64_t den
) 
```




<hr>



### function gain\_apply\_i32 

_g \* v for a 32-bit integer v (slow loop, 64-bit product), as a saturated signal._ 
```C++
static sstep_real_t gain_apply_i32 (
    const struct sstep_gain * g,
    int32_t v
) 
```




<hr>



### function is\_active 

_Outputs are on._ 
```C++
static bool is_active (
    enum sstep_state s
) 
```




<hr>



### function is\_moving 

_A command is executing._ 
```C++
static bool is_moving (
    const struct sstep_ctx * m
) 
```



In hold mode the velocity is always zero: a move ends with v = 0 on the target and a stop becomes hold only at v = 0. 


        

<hr>



### function is\_pow2 

_v is a power of two (v &gt; 0)._ 
```C++
static bool is_pow2 (
    uint32_t v
) 
```




<hr>



### function log2\_u32 

_log2 of a power of two._ 
```C++
static uint8_t log2_u32 (
    uint32_t v
) 
```




<hr>



### function mul\_ok 

_out = a \* b; false when the product reaches 2^62._ 
```C++
static bool mul_ok (
    uint64_t a,
    uint64_t b,
    uint64_t * out
) 
```




<hr>



### function pow2\_i64 

_2^n for n = 0 ._ 
```C++
static int64_t pow2_i64 (
    uint32_t n
) 
```



. 62, built with an unsigned shift (MISRA C:2012 Rule 10.1). 


        

<hr>



### function publish 

_Hand a setpoint to the fast loop (slow loop context)._ 
```C++
static void publish (
    struct sstep_ctx * m,
    const struct sstep_setpoint * sp
) 
```



The fast loop never reads the buffer being written: sp\_pending is cleared first (single core). 


        

<hr>



### function ratio 

_g = (n1 n2) / (d1 d2) with overflow checks (configuration only)._ 
```C++
static int ratio (
    uint64_t n1,
    uint64_t n2,
    uint64_t d1,
    uint64_t d2,
    struct sstep_gain * g
) 
```



Every factor is a product of at most two validated values (&lt; 10^7 each), so it fits. 


        

<hr>



### function raw\_angle 

_Electrical angle of a position [Q16 microsteps]: microstep = 2^30 / M._ 
```C++
static sstep_angle_t raw_angle (
    const struct sstep_ctx * m,
    int64_t pos
) 
```




<hr>



### function set\_currents 

_Validate and store the run and hold currents._ 
```C++
static int set_currents (
    struct sstep_ctx * m,
    uint32_t run_ma,
    uint32_t hold_ma
) 
```




<hr>



### function set\_limits 

_Validate and store the motion limits (engineering units -&gt; Q16 per period)._ 
```C++
static int set_limits (
    struct sstep_ctx * m,
    uint32_t v_max,
    uint32_t accel,
    uint32_t decel
) 
```




<hr>



### function shutdown 

_Switch the outputs off and enter the fault state (any context)._ 
```C++
static void shutdown (
    struct sstep_ctx * m,
    uint16_t faults
) 
```




<hr>
## Macro Definition Documentation





### define Q16\_LIMIT 

_Velocities and accelerations in Q16 per period stay below 2^30._ 
```C++
#define Q16_LIMIT `((int64_t)0x40000000)`
```




<hr>



### define RATIO\_LIMIT 

_Operand limit of the gain ratios (sstep\_gain\_from\_ratio64)._ 
```C++
#define RATIO_LIMIT `((uint64_t)1U << 62U)`
```




<hr>



### define TWO\_PI\_DEN 

```C++
#define TWO_PI_DEN `113U`
```




<hr>



### define TWO\_PI\_NUM 

_2 pi = 710 / 113 (error 8.5e-8)._ 
```C++
#define TWO_PI_NUM `710U`
```




<hr>



### define VALUE\_MAX 

_Largest engineering value accepted in the motor and stage configuration._ 
```C++
#define VALUE_MAX `10000000U`
```




<hr>



### define VBUS\_PU\_MAX 

_Bus voltage limit of the fast loop: 2 pu - 2^-15._ 
```C++
#define VBUS_PU_MAX `SSTEP_ACC_SCALE_MAX`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_ctrl.c`

