

# Struct sstep\_setpoint



[**ClassList**](annotated.md) **>** [**sstep\_setpoint**](structsstep__setpoint.md)



_Setpoint handed from the slow loop to the fast loop._ 

* `#include <sstep_internal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**sstep\_angle\_t**](group__sstep__math.md#typedef-sstep_angle_t) | [**angle**](#variable-angle)  <br>_Electrical angle at the start of the slow period._  |
|  int32\_t | [**dangle**](#variable-dangle)  <br>_Angle increment per fast loop period._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**ff**](#variable-ff)  <br>_Back-EMF feed-forward amplitude [pu of V\_base]._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**i\_amp**](#variable-i_amp)  <br>_Current amplitude [pu]._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**inv\_vbus**](#variable-inv_vbus)  <br>_V\_base / measured bus voltage._  |












































## Public Attributes Documentation




### variable angle 

_Electrical angle at the start of the slow period._ 
```C++
sstep_angle_t sstep_setpoint::angle;
```




<hr>



### variable dangle 

_Angle increment per fast loop period._ 
```C++
int32_t sstep_setpoint::dangle;
```




<hr>



### variable ff 

_Back-EMF feed-forward amplitude [pu of V\_base]._ 
```C++
sstep_real_t sstep_setpoint::ff;
```




<hr>



### variable i\_amp 

_Current amplitude [pu]._ 
```C++
sstep_real_t sstep_setpoint::i_amp;
```




<hr>



### variable inv\_vbus 

_V\_base / measured bus voltage._ 
```C++
struct sstep_gain sstep_setpoint::inv_vbus;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_internal.h`

