

# Struct sstep\_callbacks



[**ClassList**](annotated.md) **>** [**sstep\_callbacks**](structsstep__callbacks.md)



_Hardware callbacks._ [More...](#detailed-description)

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**context**](#variable-context)  <br>_Passed unchanged to every callback._  |
|  sstep\_event\_fn | [**on\_event**](#variable-on_event)  <br>_Optional, NULL if unused._  |
|  sstep\_fault\_fn | [**on\_fault**](#variable-on_fault)  <br>_Optional, NULL if unused._  |
|  sstep\_output\_enable\_fn | [**output\_enable**](#variable-output_enable)  <br>_Required._  |
|  sstep\_pwm\_write\_fn | [**pwm\_write**](#variable-pwm_write)  <br>_Required._  |












































## Detailed Description


`pwm_write` and `output_enable` are required. 


    
## Public Attributes Documentation




### variable context 

_Passed unchanged to every callback._ 
```C++
void* sstep_callbacks::context;
```




<hr>



### variable on\_event 

_Optional, NULL if unused._ 
```C++
sstep_event_fn sstep_callbacks::on_event;
```




<hr>



### variable on\_fault 

_Optional, NULL if unused._ 
```C++
sstep_fault_fn sstep_callbacks::on_fault;
```




<hr>



### variable output\_enable 

_Required._ 
```C++
sstep_output_enable_fn sstep_callbacks::output_enable;
```




<hr>



### variable pwm\_write 

_Required._ 
```C++
sstep_pwm_write_fn sstep_callbacks::pwm_write;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

