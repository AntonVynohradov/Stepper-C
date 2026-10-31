

# File sstep\_ctrl.h



[**FileList**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**sstep\_ctrl.h**](sstep__ctrl_8h.md)

[Go to the source code of this file](sstep__ctrl_8h_source.md)



* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "sstep_config.h"`
* `#include "sstep_defs.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**sstep**](structsstep.md) <br>_Driver instance._  |
| struct | [**sstep\_callbacks**](structsstep__callbacks.md) <br>_Hardware callbacks._  |
| struct | [**sstep\_config**](structsstep__config.md) <br>_Complete configuration given to_ [_**sstep\_init()**_](group__sstep__api.md#function-sstep_init) _._ |
| struct | [**sstep\_ctrl\_cfg**](structsstep__ctrl__cfg.md) <br>_Control and motion parameters._  |
| struct | [**sstep\_motor\_cfg**](structsstep__motor__cfg.md) <br>_Motor parameters, from the datasheet or measured._  |
| struct | [**sstep\_sample**](structsstep__sample.md) <br>_Raw measurements of one fast loop period, as read from the ADC._  |
| struct | [**sstep\_stage\_cfg**](structsstep__stage__cfg.md) <br>_Power stage and measurement parameters._  |
| struct | [**sstep\_status**](structsstep__status.md) <br>_Snapshot returned by_ [_**sstep\_get\_status()**_](group__sstep__api.md#function-sstep_get_status) _, in engineering units._ |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**sstep\_drive\_mode**](#enum-sstep_drive_mode)  <br>_How the phase voltages are obtained._  |
| enum  | [**sstep\_event**](#enum-sstep_event)  <br>_Events reported through sstep\_event\_fn._  |
| typedef void(\* | [**sstep\_event\_fn**](#typedef-sstep_event_fn)  <br>_Notify the application of a motion event (optional)._  |
| typedef void(\* | [**sstep\_fault\_fn**](#typedef-sstep_fault_fn)  <br>_Notify the application of new faults (optional)._  |
| typedef int(\* | [**sstep\_output\_enable\_fn**](#typedef-sstep_output_enable_fn)  <br>_Enable or disable the power stage outputs (disabled = all legs high impedance)._  |
| typedef int(\* | [**sstep\_pwm\_write\_fn**](#typedef-sstep_pwm_write_fn)  <br>_Write the PWM compare values of all legs._  |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SSTEP\_CONFIG\_BYTES**](sstep__ctrl_8h.md#define-sstep_config_bytes)  `136U`<br>_Bytes of the configuration copy at the start of the instance: 136 with 32-bit pointers (Cortex-M), 160 with 64-bit pointers._  |
| define  | [**SSTEP\_INSTANCE\_WORDS**](sstep__ctrl_8h.md#define-sstep_instance_words)  `((SSTEP\_CONFIG\_BYTES + SSTEP\_STATE\_BYTES + 7U) / 8U)`<br>_Size of_ [_**sstep**_](structsstep.md) _in 64-bit words._ |
| define  | [**SSTEP\_STATE\_BYTES**](sstep__ctrl_8h.md#define-sstep_state_bytes)  `200U`<br>_Bytes of the private driver state that follow the configuration copy._  |

## Public Types Documentation




### enum sstep\_drive\_mode 

_How the phase voltages are obtained._ 
```C++
enum sstep_drive_mode {
    SSTEP_DRIVE_CURRENT = 0,
    SSTEP_DRIVE_VOLTAGE = 1
};
```




<hr>



### enum sstep\_event 

_Events reported through sstep\_event\_fn._ 
```C++
enum sstep_event {
    SSTEP_EVENT_MOVE_DONE = 0,
    SSTEP_EVENT_HOLD_CURRENT = 1
};
```




<hr>



### typedef sstep\_event\_fn 

_Notify the application of a motion event (optional)._ 
```C++
typedef void(* sstep_event_fn) (void *context, enum sstep_event event);
```



Called from [**sstep\_slow\_loop()**](group__sstep__api.md#function-sstep_slow_loop). 

**Parameters:**


* `context` [**sstep\_callbacks::context**](structsstep__callbacks.md#variable-context). 
* `event` The event. 




        

<hr>



### typedef sstep\_fault\_fn 

_Notify the application of new faults (optional)._ 
```C++
typedef void(* sstep_fault_fn) (void *context, uint16_t faults);
```



Called from the context that detected the fault, which may be [**sstep\_fast\_loop()**](group__sstep__api.md#function-sstep_fast_loop) (interrupt). 

**Parameters:**


* `context` [**sstep\_callbacks::context**](structsstep__callbacks.md#variable-context). 
* `faults` Newly raised [**SSTEP\_FAULT\_OVERCURRENT**](group__sstep__defs.md#define-sstep_fault_overcurrent) ... flags. 




        

<hr>



### typedef sstep\_output\_enable\_fn 

_Enable or disable the power stage outputs (disabled = all legs high impedance)._ 
```C++
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
```C++
typedef int(* sstep_pwm_write_fn) (void *context, const uint16_t compare[SSTEP_LEGS_MAX]);
```



Called from [**sstep\_fast\_loop()**](group__sstep__api.md#function-sstep_fast_loop). 

**Parameters:**


* `context` [**sstep\_callbacks::context**](structsstep__callbacks.md#variable-context). 
* `compare` One compare value per leg, 0 .. `pwm_period_ticks` (0 = low-side on, `pwm_period_ticks` = high-side on). Entries above the leg count of the topology are 0. Valid only during the call. 



**Return value:**


* `0` Success. 
* `<0` Error: the driver switches the outputs off with [**SSTEP\_FAULT\_HAL**](group__sstep__defs.md#define-sstep_fault_hal). 




        

<hr>
## Macro Definition Documentation





### define SSTEP\_CONFIG\_BYTES 

_Bytes of the configuration copy at the start of the instance: 136 with 32-bit pointers (Cortex-M), 160 with 64-bit pointers._ 
```C++
#define SSTEP_CONFIG_BYTES `136U`
```



A plain constant, so that the instance size needs no sizeof (static analysis); sstep\_ctrl.c checks it. 


        

<hr>



### define SSTEP\_INSTANCE\_WORDS 

_Size of_ [_**sstep**_](structsstep.md) _in 64-bit words._
```C++
#define SSTEP_INSTANCE_WORDS `((SSTEP_CONFIG_BYTES + SSTEP_STATE_BYTES + 7U) / 8U)`
```




<hr>



### define SSTEP\_STATE\_BYTES 

_Bytes of the private driver state that follow the configuration copy._ 
```C++
#define SSTEP_STATE_BYTES `200U`
```



Covers both numeric backends on 32-bit (Cortex-M) and 64-bit hosts; sstep\_ctrl.c checks it at compile time. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

