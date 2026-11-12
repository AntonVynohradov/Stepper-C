

# Struct sstep\_status



[**ClassList**](annotated.md) **>** [**sstep\_status**](structsstep__status.md)



_Snapshot returned by_ [_**sstep\_get\_status()**_](group__sstep__api.md#function-sstep_get_status) _, in engineering units._

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**emf\_d\_mv**](#variable-emf_d_mv)  <br>_Filtered back-EMF estimate along the commanded current vector [mV]; 0 when stall detection is off._  |
|  int32\_t | [**emf\_q\_mv**](#variable-emf_q_mv)  <br>_Filtered back-EMF estimate 90 degrees ahead of the commanded current vector [mV]; 0 when stall detection is off._  |
|  uint16\_t | [**faults**](#variable-faults)  <br>_Active_ [_**SSTEP\_FAULT\_OVERCURRENT**_](group__sstep__defs.md#define-sstep_fault_overcurrent) _... flags._ |
|  int32\_t | [**i\_a\_ma**](#variable-i_a_ma)  <br>_Last measured phase A current [mA]._  |
|  int32\_t | [**i\_b\_ma**](#variable-i_b_ma)  <br>_Last measured phase B current [mA]._  |
|  int32\_t | [**load\_angle\_cdeg**](#variable-load_angle_cdeg)  <br>_Estimated load angle, commanded minus rotor electrical angle [0.01 deg], same sign as the motor torque._  |
|  int32\_t | [**position**](#variable-position)  <br>_Commanded position [microsteps]._  |
|  int32\_t | [**rotor\_velocity**](#variable-rotor_velocity)  <br>_Estimated rotor velocity [microsteps/s], from the back-EMF magnitude; 0 when stall detection is off._  |
|  enum [**sstep\_selftest\_result**](group__sstep__defs.md#enum-sstep_selftest_result) | [**selftest**](#variable-selftest)  <br>_Result of the last_ [_**sstep\_selftest()**_](group__sstep__api.md#function-sstep_selftest) _._ |
|  uint16\_t | [**selftest\_duty**](#variable-selftest_duty)  <br>_Compare value of that phase at the end of its test._  |
|  int32\_t | [**selftest\_i\_ma**](#variable-selftest_i_ma)  <br>_Current of that phase at the end of its test [mA], positive in the expected direction._  |
|  uint8\_t | [**selftest\_phase**](#variable-selftest_phase)  <br>_Phase the result refers to: 0 = A, 1 = B._  |
|  uint8\_t | [**stall\_level\_pct**](#variable-stall_level_pct)  <br>_Stall detector level, 0 ._  |
|  enum [**sstep\_state**](group__sstep__defs.md#enum-sstep_state) | [**state**](#variable-state)  <br>_Driver state._  |
|  int32\_t | [**target**](#variable-target)  <br>_Target of the current move [microsteps]._  |
|  uint32\_t | [**vbus\_mv**](#variable-vbus_mv)  <br>_Last measured bus voltage [mV]._  |
|  int32\_t | [**velocity**](#variable-velocity)  <br>_Commanded velocity [microsteps/s]._  |












































## Public Attributes Documentation




### variable emf\_d\_mv 

_Filtered back-EMF estimate along the commanded current vector [mV]; 0 when stall detection is off._ 
```C++
int32_t sstep_status::emf_d_mv;
```




<hr>



### variable emf\_q\_mv 

_Filtered back-EMF estimate 90 degrees ahead of the commanded current vector [mV]; 0 when stall detection is off._ 
```C++
int32_t sstep_status::emf_q_mv;
```




<hr>



### variable faults 

_Active_ [_**SSTEP\_FAULT\_OVERCURRENT**_](group__sstep__defs.md#define-sstep_fault_overcurrent) _... flags._
```C++
uint16_t sstep_status::faults;
```




<hr>



### variable i\_a\_ma 

_Last measured phase A current [mA]._ 
```C++
int32_t sstep_status::i_a_ma;
```




<hr>



### variable i\_b\_ma 

_Last measured phase B current [mA]._ 
```C++
int32_t sstep_status::i_b_ma;
```




<hr>



### variable load\_angle\_cdeg 

_Estimated load angle, commanded minus rotor electrical angle [0.01 deg], same sign as the motor torque._ 
```C++
int32_t sstep_status::load_angle_cdeg;
```




<hr>



### variable position 

_Commanded position [microsteps]._ 
```C++
int32_t sstep_status::position;
```




<hr>



### variable rotor\_velocity 

_Estimated rotor velocity [microsteps/s], from the back-EMF magnitude; 0 when stall detection is off._ 
```C++
int32_t sstep_status::rotor_velocity;
```




<hr>



### variable selftest 

_Result of the last_ [_**sstep\_selftest()**_](group__sstep__api.md#function-sstep_selftest) _._
```C++
enum sstep_selftest_result sstep_status::selftest;
```




<hr>



### variable selftest\_duty 

_Compare value of that phase at the end of its test._ 
```C++
uint16_t sstep_status::selftest_duty;
```




<hr>



### variable selftest\_i\_ma 

_Current of that phase at the end of its test [mA], positive in the expected direction._ 
```C++
int32_t sstep_status::selftest_i_ma;
```




<hr>



### variable selftest\_phase 

_Phase the result refers to: 0 = A, 1 = B._ 
```C++
uint8_t sstep_status::selftest_phase;
```




<hr>



### variable stall\_level\_pct 

_Stall detector level, 0 ._ 
```C++
uint8_t sstep_status::stall_level_pct;
```



. 100 % of the trip. 


        

<hr>



### variable state 

_Driver state._ 
```C++
enum sstep_state sstep_status::state;
```




<hr>



### variable target 

_Target of the current move [microsteps]._ 
```C++
int32_t sstep_status::target;
```




<hr>



### variable vbus\_mv 

_Last measured bus voltage [mV]._ 
```C++
uint32_t sstep_status::vbus_mv;
```




<hr>



### variable velocity 

_Commanded velocity [microsteps/s]._ 
```C++
int32_t sstep_status::velocity;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

