

# Struct sstep\_ctrl\_cfg



[**ClassList**](annotated.md) **>** [**sstep\_ctrl\_cfg**](structsstep__ctrl__cfg.md)



_Control and motion parameters._ [More...](#detailed-description)

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**accel**](#variable-accel)  <br>_Acceleration [microsteps/s^2]._  |
|  uint32\_t | [**current\_bw\_hz**](#variable-current_bw_hz)  <br>_Current loop bandwidth [Hz]; the PI gains are computed from R, L and this value._  |
|  uint32\_t | [**decel**](#variable-decel)  <br>_Deceleration [microsteps/s^2]; 0 = same as accel._  |
|  enum sstep\_drive\_mode | [**drive\_mode**](#variable-drive_mode)  <br>_Current or voltage drive._  |
|  uint32\_t | [**fast\_loop\_hz**](#variable-fast_loop_hz)  <br>_Rate of_ [_**sstep\_fast\_loop()**_](group__sstep__api.md#function-sstep_fast_loop) _[Hz]._ |
|  uint32\_t | [**hold\_delay\_ms**](#variable-hold_delay_ms)  <br>_Standstill time before switching to i\_hold\_ma [ms]._  |
|  uint32\_t | [**i\_hold\_ma**](#variable-i_hold_ma)  <br>_Peak phase current at standstill [mA]._  |
|  uint32\_t | [**i\_run\_ma**](#variable-i_run_ma)  <br>_Peak phase current while moving [mA]._  |
|  uint32\_t | [**i\_slew\_ma\_per\_ms**](#variable-i_slew_ma_per_ms)  <br>_Largest change of the current amplitude [mA/ms]: from 0 after_ [_**sstep\_enable()**_](group__sstep__api.md#function-sstep_enable) _, between run and hold current, after_[_**sstep\_set\_current()**_](group__sstep__api.md#function-sstep_set_current) _; 0 = at once._ |
|  bool | [**i\_trip\_voltage\_drive**](#variable-i_trip_voltage_drive)  <br>_Overcurrent trips also in SSTEP\_DRIVE\_VOLTAGE (the board measures the phase currents there too)._  |
|  uint16\_t | [**microsteps**](#variable-microsteps)  <br>_Microsteps per full step (power of two, 1 ._  |
|  uint32\_t | [**slow\_loop\_hz**](#variable-slow_loop_hz)  <br>_Rate of_ [_**sstep\_slow\_loop()**_](group__sstep__api.md#function-sstep_slow_loop) _[Hz]._ |
|  uint32\_t | [**stall\_filter\_us**](#variable-stall_filter_us)  <br>_Time constant of the back-EMF low-pass filter [us], at least one fast loop period._  |
|  uint32\_t | [**stall\_speed**](#variable-stall_speed)  <br>_Slip threshold [microsteps/s]: rotor speed that counts as motion while the command is at rest or slower than this; above it, the synchronism check._  |
|  uint16\_t | [**stall\_sync\_pct**](#variable-stall_sync_pct)  <br>_Synchronism threshold [%] (1 ._  |
|  uint32\_t | [**stall\_time\_ms**](#variable-stall_time_ms)  <br>_Stall detection: time the stall condition must persist before_ [_**SSTEP\_FAULT\_STALL**_](group__sstep__defs.md#define-sstep_fault_stall) _[ms]; 0 = stall detection off (the other stall\_\* fields are then ignored)._ |
|  uint32\_t | [**v\_max**](#variable-v_max)  <br>_Maximum velocity [microsteps/s]._  |












































## Detailed Description


All of them except the loop rates can be changed at run time with the sstep\_set\_\*() functions. 


    
## Public Attributes Documentation




### variable accel 

_Acceleration [microsteps/s^2]._ 
```C++
uint32_t sstep_ctrl_cfg::accel;
```




<hr>



### variable current\_bw\_hz 

_Current loop bandwidth [Hz]; the PI gains are computed from R, L and this value._ 
```C++
uint32_t sstep_ctrl_cfg::current_bw_hz;
```




<hr>



### variable decel 

_Deceleration [microsteps/s^2]; 0 = same as accel._ 
```C++
uint32_t sstep_ctrl_cfg::decel;
```




<hr>



### variable drive\_mode 

_Current or voltage drive._ 
```C++
enum sstep_drive_mode sstep_ctrl_cfg::drive_mode;
```




<hr>



### variable fast\_loop\_hz 

_Rate of_ [_**sstep\_fast\_loop()**_](group__sstep__api.md#function-sstep_fast_loop) _[Hz]._
```C++
uint32_t sstep_ctrl_cfg::fast_loop_hz;
```




<hr>



### variable hold\_delay\_ms 

_Standstill time before switching to i\_hold\_ma [ms]._ 
```C++
uint32_t sstep_ctrl_cfg::hold_delay_ms;
```




<hr>



### variable i\_hold\_ma 

_Peak phase current at standstill [mA]._ 
```C++
uint32_t sstep_ctrl_cfg::i_hold_ma;
```




<hr>



### variable i\_run\_ma 

_Peak phase current while moving [mA]._ 
```C++
uint32_t sstep_ctrl_cfg::i_run_ma;
```




<hr>



### variable i\_slew\_ma\_per\_ms 

_Largest change of the current amplitude [mA/ms]: from 0 after_ [_**sstep\_enable()**_](group__sstep__api.md#function-sstep_enable) _, between run and hold current, after_[_**sstep\_set\_current()**_](group__sstep__api.md#function-sstep_set_current) _; 0 = at once._
```C++
uint32_t sstep_ctrl_cfg::i_slew_ma_per_ms;
```




<hr>



### variable i\_trip\_voltage\_drive 

_Overcurrent trips also in SSTEP\_DRIVE\_VOLTAGE (the board measures the phase currents there too)._ 
```C++
bool sstep_ctrl_cfg::i_trip_voltage_drive;
```




<hr>



### variable microsteps 

_Microsteps per full step (power of two, 1 ._ 
```C++
uint16_t sstep_ctrl_cfg::microsteps;
```



. [**SSTEP\_CFG\_MICROSTEPS\_MAX**](group__sstep__config.md#define-sstep_cfg_microsteps_max)). 


        

<hr>



### variable slow\_loop\_hz 

_Rate of_ [_**sstep\_slow\_loop()**_](group__sstep__api.md#function-sstep_slow_loop) _[Hz]._
```C++
uint32_t sstep_ctrl_cfg::slow_loop_hz;
```




<hr>



### variable stall\_filter\_us 

_Time constant of the back-EMF low-pass filter [us], at least one fast loop period._ 
```C++
uint32_t sstep_ctrl_cfg::stall_filter_us;
```




<hr>



### variable stall\_speed 

_Slip threshold [microsteps/s]: rotor speed that counts as motion while the command is at rest or slower than this; above it, the synchronism check._ 
```C++
uint32_t sstep_ctrl_cfg::stall_speed;
```




<hr>



### variable stall\_sync\_pct 

_Synchronism threshold [%] (1 ._ 
```C++
uint16_t sstep_ctrl_cfg::stall_sync_pct;
```



. 100): smallest back-EMF along the commanded motion, relative to Ke \* speed. 


        

<hr>



### variable stall\_time\_ms 

_Stall detection: time the stall condition must persist before_ [_**SSTEP\_FAULT\_STALL**_](group__sstep__defs.md#define-sstep_fault_stall) _[ms]; 0 = stall detection off (the other stall\_\* fields are then ignored)._
```C++
uint32_t sstep_ctrl_cfg::stall_time_ms;
```



Requires SSTEP\_DRIVE\_CURRENT and ke\_uv\_s\_per\_rad &gt; 0. 


        

<hr>



### variable v\_max 

_Maximum velocity [microsteps/s]._ 
```C++
uint32_t sstep_ctrl_cfg::v_max;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

