

# Struct sstep\_config



[**ClassList**](annotated.md) **>** [**sstep\_config**](structsstep__config.md)



_Complete configuration given to_ [_**sstep\_init()**_](group__sstep__ctrl.md#function-sstep_init) _._[More...](#detailed-description)

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  struct [**sstep\_ctrl\_cfg**](structsstep__ctrl__cfg.md) | [**ctrl**](#variable-ctrl)  <br>_Control and motion._  |
|  struct [**sstep\_callbacks**](structsstep__callbacks.md) | [**hal**](#variable-hal)  <br>_Hardware callbacks._  |
|  struct [**sstep\_motor\_cfg**](structsstep__motor__cfg.md) | [**motor**](#variable-motor)  <br>_Motor._  |
|  struct [**sstep\_stage\_cfg**](structsstep__stage__cfg.md) | [**stage**](#variable-stage)  <br>_Power stage and measurement._  |












































## Detailed Description


Copied by [**sstep\_init()**](group__sstep__ctrl.md#function-sstep_init); the structure may be discarded afterwards. 


    
## Public Attributes Documentation




### variable ctrl 

_Control and motion._ 
```C++
struct sstep_ctrl_cfg sstep_config::ctrl;
```




<hr>



### variable hal 

_Hardware callbacks._ 
```C++
struct sstep_callbacks sstep_config::hal;
```




<hr>



### variable motor 

_Motor._ 
```C++
struct sstep_motor_cfg sstep_config::motor;
```




<hr>



### variable stage 

_Power stage and measurement._ 
```C++
struct sstep_stage_cfg sstep_config::stage;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

