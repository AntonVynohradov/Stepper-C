

# Group sstep\_config



[**Modules**](modules.md) **>** [**sstep\_config**](group__sstep__config.md)



_Macros that select the code compiled into the library._ [More...](#detailed-description)

































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SSTEP\_CFG\_ARG\_CHECKS**](group__sstep__config.md#define-sstep_cfg_arg_checks)  `1`<br>_1: every public function validates its pointer arguments (MISRA Dir 4.11)._  |
| define  | [**SSTEP\_CFG\_MATH\_BACKEND**](group__sstep__config.md#define-sstep_cfg_math_backend)  `[**SSTEP\_MATH\_BACKEND\_FIXED**](group__sstep__config.md#define-sstep_math_backend_fixed)`<br>_Selected numeric backend:_ [_**SSTEP\_MATH\_BACKEND\_FIXED**_](group__sstep__config.md#define-sstep_math_backend_fixed) _or_[_**SSTEP\_MATH\_BACKEND\_FLOAT**_](group__sstep__config.md#define-sstep_math_backend_float) _._ |
| define  | [**SSTEP\_CFG\_MICROSTEPS\_MAX**](group__sstep__config.md#define-sstep_cfg_microsteps_max)  `256U`<br>_Largest microstep division accepted by_ [_**sstep\_init()**_](group__sstep__api.md#function-sstep_init) _(power of two, 1..256)._ |
| define  | [**SSTEP\_CFG\_SINE\_TABLE\_BITS**](group__sstep__config.md#define-sstep_cfg_sine_table_bits)  `8U`<br>_log2 of the number of entries of the quarter-wave sine table (6..10)._  |
| define  | [**SSTEP\_CFG\_TOPOLOGY\_FULL\_BRIDGE**](group__sstep__config.md#define-sstep_cfg_topology_full_bridge)  `1`<br>_1: compile support for two full H-bridges (4 half-bridge legs)._  |
| define  | [**SSTEP\_CFG\_TOPOLOGY\_SHARED\_LEG**](group__sstep__config.md#define-sstep_cfg_topology_shared_leg)  `1`<br>_1: compile support for three half-bridges with a common leg (3 legs), for example STMicroelectronics B-G431B-ESC1._  |
| define  | [**SSTEP\_MATH\_BACKEND\_FIXED**](group__sstep__config.md#define-sstep_math_backend_fixed)  `0`<br>_Fixed point backend: Q15 signals, Q31 accumulators, integer-only arithmetic._  |
| define  | [**SSTEP\_MATH\_BACKEND\_FLOAT**](group__sstep__config.md#define-sstep_math_backend_float)  `1`<br>_Single-precision float backend._  |

## Detailed Description


Every macro has a default and may be overridden in one of two ways:
* on the compiler command line (`-DSSTEP_CFG_MATH_BACKEND=SSTEP_MATH_BACKEND_FLOAT`), or
* in a user header `sstep_user_config.h`, included when the macro `SSTEP_USER_CONFIG` is defined (CMake: set `SSTEP_CONFIG_DIR` to its folder).




Values that cannot change at run time (code size, table sizes, which code paths exist) live here. Motor and board parameters are run-time values passed to [**sstep\_init()**](group__sstep__api.md#function-sstep_init). 


    
## Macro Definition Documentation





### define SSTEP\_CFG\_ARG\_CHECKS 

_1: every public function validates its pointer arguments (MISRA Dir 4.11)._ 
```
#define SSTEP_CFG_ARG_CHECKS `1`
```



0 removes the checks from [**sstep\_fast\_loop()**](group__sstep__api.md#function-sstep_fast_loop) only, the function called from the PWM interrupt; all other functions always check. 


        

<hr>



### define SSTEP\_CFG\_MATH\_BACKEND 

_Selected numeric backend:_ [_**SSTEP\_MATH\_BACKEND\_FIXED**_](group__sstep__config.md#define-sstep_math_backend_fixed) _or_[_**SSTEP\_MATH\_BACKEND\_FLOAT**_](group__sstep__config.md#define-sstep_math_backend_float) _._
```
#define SSTEP_CFG_MATH_BACKEND `SSTEP_MATH_BACKEND_FIXED`
```




<hr>



### define SSTEP\_CFG\_MICROSTEPS\_MAX 

_Largest microstep division accepted by_ [_**sstep\_init()**_](group__sstep__api.md#function-sstep_init) _(power of two, 1..256)._
```
#define SSTEP_CFG_MICROSTEPS_MAX `256U`
```




<hr>



### define SSTEP\_CFG\_SINE\_TABLE\_BITS 

_log2 of the number of entries of the quarter-wave sine table (6..10)._ 
```
#define SSTEP_CFG_SINE_TABLE_BITS `8U`
```



The table is `static const` (flash): 2^bits + 1 entries of 2 bytes (fixed) or 4 bytes (float). Values between entries are linearly interpolated. 


        

<hr>



### define SSTEP\_CFG\_TOPOLOGY\_FULL\_BRIDGE 

_1: compile support for two full H-bridges (4 half-bridge legs)._ 
```
#define SSTEP_CFG_TOPOLOGY_FULL_BRIDGE `1`
```




<hr>



### define SSTEP\_CFG\_TOPOLOGY\_SHARED\_LEG 

_1: compile support for three half-bridges with a common leg (3 legs), for example STMicroelectronics B-G431B-ESC1._ 
```
#define SSTEP_CFG_TOPOLOGY_SHARED_LEG `1`
```




<hr>



### define SSTEP\_MATH\_BACKEND\_FIXED 

_Fixed point backend: Q15 signals, Q31 accumulators, integer-only arithmetic._ 
```
#define SSTEP_MATH_BACKEND_FIXED `0`
```



For MCUs without an FPU (Cortex-M0/M0+/M3, RISC-V RV32IMC, 8/16-bit cores). 


        

<hr>



### define SSTEP\_MATH\_BACKEND\_FLOAT 

_Single-precision float backend._ 
```
#define SSTEP_MATH_BACKEND_FLOAT `1`
```



For MCUs with a hardware FPU (Cortex-M4F/M7/M33 with FPU, STM32G4, ESP32-S3...). 


        

<hr>

------------------------------


