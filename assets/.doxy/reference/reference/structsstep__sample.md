

# Struct sstep\_sample



[**ClassList**](annotated.md) **>** [**sstep\_sample**](structsstep__sample.md)



_Raw measurements of one fast loop period, as read from the ADC._ 

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**i\_a**](#variable-i_a)  <br>_Phase A current [ADC counts]._  |
|  uint16\_t | [**i\_b**](#variable-i_b)  <br>_Phase B current [ADC counts]._  |
|  uint16\_t | [**vbus**](#variable-vbus)  <br>_Bus voltage [ADC counts]._  |












































## Public Attributes Documentation




### variable i\_a 

_Phase A current [ADC counts]._ 
```C++
uint16_t sstep_sample::i_a;
```



Ignored in SSTEP\_DRIVE\_VOLTAGE. 


        

<hr>



### variable i\_b 

_Phase B current [ADC counts]._ 
```C++
uint16_t sstep_sample::i_b;
```



Ignored in SSTEP\_DRIVE\_VOLTAGE. 


        

<hr>



### variable vbus 

_Bus voltage [ADC counts]._ 
```C++
uint16_t sstep_sample::vbus;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

