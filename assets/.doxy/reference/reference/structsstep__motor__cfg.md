

# Struct sstep\_motor\_cfg



[**ClassList**](annotated.md) **>** [**sstep\_motor\_cfg**](structsstep__motor__cfg.md)



_Motor parameters, from the datasheet or measured._ [More...](#detailed-description)

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**full\_steps\_per\_rev**](#variable-full_steps_per_rev)  <br>_Full steps per revolution._  |
|  uint32\_t | [**i\_rated\_ma**](#variable-i_rated_ma)  <br>_Rated phase current [mA]._  |
|  uint32\_t | [**ke\_uv\_s\_per\_rad**](#variable-ke_uv_s_per_rad)  <br>_Back-EMF constant, peak phase voltage per mechanical speed [uV\*s/rad] (equals Kt in uN\*m/A)._  |
|  uint32\_t | [**l\_uh**](#variable-l_uh)  <br>_Phase inductance [uH]._  |
|  uint32\_t | [**r\_mohm**](#variable-r_mohm)  <br>_Phase resistance at 25 degC [mOhm]._  |












































## Detailed Description


Example: 17HS19-2004S1. 


    
## Public Attributes Documentation




### variable full\_steps\_per\_rev 

_Full steps per revolution._ 
```C++
uint16_t sstep_motor_cfg::full_steps_per_rev;
```



1.8 deg: 200. 


        

<hr>



### variable i\_rated\_ma 

_Rated phase current [mA]._ 
```C++
uint32_t sstep_motor_cfg::i_rated_ma;
```



17HS19: 2000. 


        

<hr>



### variable ke\_uv\_s\_per\_rad 

_Back-EMF constant, peak phase voltage per mechanical speed [uV\*s/rad] (equals Kt in uN\*m/A)._ 
```C++
uint32_t sstep_motor_cfg::ke_uv_s_per_rad;
```



0 = unknown: no back-EMF feed-forward. 


        

<hr>



### variable l\_uh 

_Phase inductance [uH]._ 
```C++
uint32_t sstep_motor_cfg::l_uh;
```



17HS19: 3000. 


        

<hr>



### variable r\_mohm 

_Phase resistance at 25 degC [mOhm]._ 
```C++
uint32_t sstep_motor_cfg::r_mohm;
```



17HS19: 1600. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

