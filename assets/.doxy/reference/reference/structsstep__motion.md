

# Struct sstep\_motion



[**ClassList**](annotated.md) **>** [**sstep\_motion**](structsstep__motion.md)



_Motion generator state._ [More...](#detailed-description)

* `#include <sstep_internal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**acc**](#variable-acc)  <br>_Acceleration limit [Q16 microsteps / period^2]._  |
|  int64\_t | [**brake\_vmax**](#variable-brake_vmax)  <br>_Braking distance from v\_max [Q16 microsteps]._  |
|  int32\_t | [**dec**](#variable-dec)  <br>_Deceleration limit._  |
|  uint8\_t | [**mode**](#variable-mode)  <br>_0 hold, 1 move, 2 run._  |
|  int64\_t | [**pos**](#variable-pos)  <br>_Position [Q16 microsteps]._  |
|  int64\_t | [**target**](#variable-target)  <br>_Move target [Q16 microsteps]._  |
|  int32\_t | [**v\_max**](#variable-v_max)  <br>_Velocity limit._  |
|  int32\_t | [**v\_run**](#variable-v_run)  <br>_Requested velocity of a RUN command._  |
|  int32\_t | [**vel**](#variable-vel)  <br>_Velocity [Q16 microsteps / period]._  |












































## Detailed Description


Integer arithmetic in both numeric backends: positions in 1/65536 microstep, velocities in 1/65536 microstep per slow loop period, accelerations in 1/65536 microstep per period squared. 


    
## Public Attributes Documentation




### variable acc 

_Acceleration limit [Q16 microsteps / period^2]._ 
```C++
int32_t sstep_motion::acc;
```




<hr>



### variable brake\_vmax 

_Braking distance from v\_max [Q16 microsteps]._ 
```C++
int64_t sstep_motion::brake_vmax;
```




<hr>



### variable dec 

_Deceleration limit._ 
```C++
int32_t sstep_motion::dec;
```




<hr>



### variable mode 

_0 hold, 1 move, 2 run._ 
```C++
uint8_t sstep_motion::mode;
```




<hr>



### variable pos 

_Position [Q16 microsteps]._ 
```C++
int64_t sstep_motion::pos;
```




<hr>



### variable target 

_Move target [Q16 microsteps]._ 
```C++
int64_t sstep_motion::target;
```




<hr>



### variable v\_max 

_Velocity limit._ 
```C++
int32_t sstep_motion::v_max;
```




<hr>



### variable v\_run 

_Requested velocity of a RUN command._ 
```C++
int32_t sstep_motion::v_run;
```




<hr>



### variable vel 

_Velocity [Q16 microsteps / period]._ 
```C++
int32_t sstep_motion::vel;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_internal.h`

