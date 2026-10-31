

# Struct sstep\_status



[**ClassList**](annotated.md) **>** [**sstep\_status**](structsstep__status.md)



_Snapshot returned by_ [_**sstep\_get\_status()**_](group__sstep__api.md#function-sstep_get_status) _, in engineering units._

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**faults**](#variable-faults)  <br>_Active_ [_**SSTEP\_FAULT\_OVERCURRENT**_](group__sstep__defs.md#define-sstep_fault_overcurrent) _... flags._ |
|  int32\_t | [**i\_a\_ma**](#variable-i_a_ma)  <br>_Last measured phase A current [mA]._  |
|  int32\_t | [**i\_b\_ma**](#variable-i_b_ma)  <br>_Last measured phase B current [mA]._  |
|  int32\_t | [**position**](#variable-position)  <br>_Commanded position [microsteps]._  |
|  enum [**sstep\_state**](group__sstep__defs.md#enum-sstep_state) | [**state**](#variable-state)  <br>_Driver state._  |
|  int32\_t | [**target**](#variable-target)  <br>_Target of the current move [microsteps]._  |
|  uint32\_t | [**vbus\_mv**](#variable-vbus_mv)  <br>_Last measured bus voltage [mV]._  |
|  int32\_t | [**velocity**](#variable-velocity)  <br>_Commanded velocity [microsteps/s]._  |












































## Public Attributes Documentation




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



### variable position 

_Commanded position [microsteps]._ 
```C++
int32_t sstep_status::position;
```




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

