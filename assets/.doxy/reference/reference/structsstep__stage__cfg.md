

# Struct sstep\_stage\_cfg



[**ClassList**](annotated.md) **>** [**sstep\_stage\_cfg**](structsstep__stage__cfg.md)



_Power stage and measurement parameters._ [More...](#detailed-description)

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**adc\_full**](#variable-adc_full)  <br>_Largest ADC reading (4095 for 12 bits)._  |
|  int32\_t | [**adc\_i\_span**](#variable-adc_i_span)  <br>_Counts from 0 A to +i\_full\_scale\_ma; negative for an inverting amplifier._  |
|  uint16\_t | [**adc\_i\_zero**](#variable-adc_i_zero)  <br>_Current ADC reading at 0 A [counts]._  |
|  uint32\_t | [**adc\_vbus\_mv\_full**](#variable-adc_vbus_mv_full)  <br>_Bus voltage at ADC reading adc\_full [mV]._  |
|  uint32\_t | [**dead\_time\_ns**](#variable-dead_time_ns)  <br>_Dead time [ns], for compensation; 0 = none._  |
|  uint32\_t | [**i\_full\_scale\_ma**](#variable-i_full_scale_ma)  <br>_Phase current at ADC full scale [mA]; base of the per-unit current._  |
|  uint32\_t | [**i\_trip\_ma**](#variable-i_trip_ma)  <br>_Overcurrent trip [mA]; must not exceed i\_full\_scale\_ma._  |
|  uint32\_t | [**pwm\_freq\_hz**](#variable-pwm_freq_hz)  <br>_PWM frequency [Hz]._  |
|  uint16\_t | [**pwm\_period\_ticks**](#variable-pwm_period_ticks)  <br>_Compare value for 100 % duty cycle._  |
|  enum [**sstep\_topology**](group__sstep__defs.md#enum-sstep_topology) | [**topology**](#variable-topology)  <br>_Power stage topology._  |
|  uint32\_t | [**vbus\_max\_mv**](#variable-vbus_max_mv)  <br>_Overvoltage trip [mV]; 0 = disabled._  |
|  uint32\_t | [**vbus\_min\_mv**](#variable-vbus_min_mv)  <br>_Undervoltage trip [mV]; 0 = disabled._  |
|  uint32\_t | [**vbus\_nominal\_mv**](#variable-vbus_nominal_mv)  <br>_Nominal bus voltage [mV]; base of the per-unit voltage._  |












































## Detailed Description


Example: B-G431B-ESC1. 


    
## Public Attributes Documentation




### variable adc\_full 

_Largest ADC reading (4095 for 12 bits)._ 
```C++
uint16_t sstep_stage_cfg::adc_full;
```




<hr>



### variable adc\_i\_span 

_Counts from 0 A to +i\_full\_scale\_ma; negative for an inverting amplifier._ 
```C++
int32_t sstep_stage_cfg::adc_i_span;
```




<hr>



### variable adc\_i\_zero 

_Current ADC reading at 0 A [counts]._ 
```C++
uint16_t sstep_stage_cfg::adc_i_zero;
```




<hr>



### variable adc\_vbus\_mv\_full 

_Bus voltage at ADC reading adc\_full [mV]._ 
```C++
uint32_t sstep_stage_cfg::adc_vbus_mv_full;
```




<hr>



### variable dead\_time\_ns 

_Dead time [ns], for compensation; 0 = none._ 
```C++
uint32_t sstep_stage_cfg::dead_time_ns;
```




<hr>



### variable i\_full\_scale\_ma 

_Phase current at ADC full scale [mA]; base of the per-unit current._ 
```C++
uint32_t sstep_stage_cfg::i_full_scale_ma;
```




<hr>



### variable i\_trip\_ma 

_Overcurrent trip [mA]; must not exceed i\_full\_scale\_ma._ 
```C++
uint32_t sstep_stage_cfg::i_trip_ma;
```




<hr>



### variable pwm\_freq\_hz 

_PWM frequency [Hz]._ 
```C++
uint32_t sstep_stage_cfg::pwm_freq_hz;
```




<hr>



### variable pwm\_period\_ticks 

_Compare value for 100 % duty cycle._ 
```C++
uint16_t sstep_stage_cfg::pwm_period_ticks;
```




<hr>



### variable topology 

_Power stage topology._ 
```C++
enum sstep_topology sstep_stage_cfg::topology;
```



ESC1: SSTEP\_TOPOLOGY\_SHARED\_LEG. 


        

<hr>



### variable vbus\_max\_mv 

_Overvoltage trip [mV]; 0 = disabled._ 
```C++
uint32_t sstep_stage_cfg::vbus_max_mv;
```




<hr>



### variable vbus\_min\_mv 

_Undervoltage trip [mV]; 0 = disabled._ 
```C++
uint32_t sstep_stage_cfg::vbus_min_mv;
```




<hr>



### variable vbus\_nominal\_mv 

_Nominal bus voltage [mV]; base of the per-unit voltage._ 
```C++
uint32_t sstep_stage_cfg::vbus_nominal_mv;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

