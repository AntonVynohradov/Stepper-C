

# Struct sstep\_gain



[**ClassList**](annotated.md) **>** [**sstep\_gain**](structsstep__gain.md)



_Gain of any magnitude: value = mant / 2^15 \* 2^exp._ [More...](#detailed-description)

* `#include <sstep_math.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int8\_t | [**exp**](#variable-exp)  <br>_Binary exponent,_ [_**SSTEP\_GAIN\_EXP\_MIN**_](group__sstep__math.md#define-sstep_gain_exp_min) _._ |
|  int16\_t | [**mant**](#variable-mant)  <br>_Q15 mantissa._  |












































## Detailed Description


Created by [**sstep\_gain\_from\_ratio()**](group__sstep__math.md#function-sstep_gain_from_ratio); the mantissa is normalised (16384 &lt;= \|mant\| &lt;= 32767) for the best resolution, or 0. 


    
## Public Attributes Documentation




### variable exp 

_Binary exponent,_ [_**SSTEP\_GAIN\_EXP\_MIN**_](group__sstep__math.md#define-sstep_gain_exp_min) _._
```C++
int8_t sstep_gain::exp;
```



. [**SSTEP\_GAIN\_EXP\_MAX**](group__sstep__math.md#define-sstep_gain_exp_max). 


        

<hr>



### variable mant 

_Q15 mantissa._ 
```C++
int16_t sstep_gain::mant;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_math.h`

