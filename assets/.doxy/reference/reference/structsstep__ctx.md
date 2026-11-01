

# Struct sstep\_ctx



[**ClassList**](annotated.md) **>** [**sstep\_ctx**](structsstep__ctx.md)



_Driver state behind the opaque_ [_**sstep**_](structsstep.md) _._[More...](#detailed-description)

* `#include <sstep_internal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**adc\_i**](#variable-adc_i)  <br>_Current per ADC count [pu]._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**adc\_v**](#variable-adc_v)  <br>_Bus voltage per ADC count [pu]._  |
|  uint16\_t | [**adv**](#variable-adv)  <br>_Angle increments applied since the hand-off._  |
|  [**sstep\_angle\_t**](group__sstep__math.md#typedef-sstep_angle_t) | [**angle\_offset**](#variable-angle_offset)  <br>_Electrical angle of position 0._  |
|  uint8\_t | [**angle\_shift**](#variable-angle_shift)  <br>_14 - log2(microsteps)._  |
|  struct [**sstep\_config**](structsstep__config.md) | [**cfg**](#variable-cfg)  <br>_Copy of the configuration._  |
|  uint16\_t | [**compare**](#variable-compare)  <br>_Last compare values._  |
|  struct [**sstep\_setpoint**](structsstep__setpoint.md) | [**cur**](#variable-cur)  <br>_Setpoint in use._  |
|  [**sstep\_integ\_t**](group__sstep__math.md#typedef-sstep_integ_t) | [**emf**](#variable-emf)  <br>_Filtered back-EMF, d and q axis [pu V]._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**emf\_a**](#variable-emf_a)  <br>_L / Ts + R / 2 [pu V / pu A]._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**emf\_alpha**](#variable-emf_alpha)  <br>_Ts / tau of the back-EMF filter._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**emf\_b**](#variable-emf_b)  <br>_L / Ts - R / 2 [pu V / pu A]._  |
|  int64\_t | [**emf\_off**](#variable-emf_off)  <br>_Mean back-EMF at rest, d and q [mV \* 2^16]._  |
|  volatile uint16\_t | [**faults**](#variable-faults)  <br>_Active faults._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**ff\_cmd**](#variable-ff_cmd)  <br>_Back-EMF of vel\_cmd [pu V]._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**ff\_gain**](#variable-ff_gain)  <br>_Feed-forward per microstep/s [pu]._  |
|  bool | [**hold\_active**](#variable-hold_active)  <br>_Hold current applied._  |
|  uint32\_t | [**hold\_ticks**](#variable-hold_ticks)  <br>_Slow periods at standstill before hold._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**i\_hold**](#variable-i_hold)  <br>_Hold current [pu]._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**i\_meas**](#variable-i_meas)  <br>_Last measured phase currents [pu]._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**i\_prev**](#variable-i_prev)  <br>_Phase currents of the previous period [pu]._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**i\_run**](#variable-i_run)  <br>_Run current [pu]._  |
|  uint16\_t | [**i\_trip\_counts**](#variable-i_trip_counts)  <br>_Overcurrent trip [ADC counts from zero]._  |
|  [**sstep\_integ\_t**](group__sstep__math.md#typedef-sstep_integ_t) | [**integ**](#variable-integ)  <br>_PI integrators [pu V]._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**inv\_vbus**](#variable-inv_vbus)  <br>_Last V\_base / bus voltage._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**kb**](#variable-kb)  <br>_Anti-windup gain Ts / Tt._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**ki**](#variable-ki)  <br>_Integral gain per fast loop period._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**kp**](#variable-kp)  <br>_Proportional gain [pu V / pu A]._  |
|  struct [**sstep\_motion**](structsstep__motion.md) | [**mo**](#variable-mo)  <br>_Motion generator._  |
|  struct [**sstep\_gain**](structsstep__gain.md) | [**r\_pu**](#variable-r_pu)  <br>_Phase resistance [pu] (voltage drive)._  |
|  uint16\_t | [**ratio**](#variable-ratio)  <br>_Fast loop periods per slow loop period._  |
|  uint32\_t | [**ready**](#variable-ready)  <br>_SSTEP\_READY after a successful_ [_**sstep\_init()**_](group__sstep__api.md#function-sstep_init) _._ |
|  struct [**sstep\_setpoint**](structsstep__setpoint.md) | [**sp**](#variable-sp)  <br>_Setpoints; the fast loop owns sp[sp\_active]._  |
|  volatile uint8\_t | [**sp\_active**](#variable-sp_active)  <br>_Buffer in use by the fast loop._  |
|  volatile uint8\_t | [**sp\_pending**](#variable-sp_pending)  <br>_1: sp[1 - sp\_active] is complete and new._  |
|  uint32\_t | [**stall\_count**](#variable-stall_count)  <br>_Stall detector up/down counter._  |
|  int32\_t | [**stall\_emf\_mv**](#variable-stall_emf_mv)  <br>_Back-EMF of stall\_speed [mV]._  |
|  uint32\_t | [**stall\_ticks**](#variable-stall_ticks)  <br>_Slow periods of stall to trip; 0 = detection off._  |
|  enum [**sstep\_state**](group__sstep__defs.md#enum-sstep_state) | [**state**](#variable-state)  <br>_Driver state._  |
|  uint32\_t | [**still**](#variable-still)  <br>_Slow periods at standstill._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**v\_hist**](#variable-v_hist)  <br>_Applied phase voltages computed 1 and 2 periods ago [pu V]._  |
|  uint16\_t | [**vbus\_max\_raw**](#variable-vbus_max_raw)  <br>_Overvoltage trip [ADC counts], 0 = off._  |
|  uint16\_t | [**vbus\_min\_raw**](#variable-vbus_min_raw)  <br>_Undervoltage trip [ADC counts], 0 = off._  |
|  volatile uint16\_t | [**vbus\_raw**](#variable-vbus_raw)  <br>_Last bus voltage [ADC counts]._  |
|  int32\_t | [**vel\_cmd**](#variable-vel_cmd)  <br>_Commanded velocity of the period in use [microsteps/s]._  |












































## Detailed Description


Its layout may change in any release. 


    
## Public Attributes Documentation




### variable adc\_i 

_Current per ADC count [pu]._ 
```C++
struct sstep_gain sstep_ctx::adc_i;
```




<hr>



### variable adc\_v 

_Bus voltage per ADC count [pu]._ 
```C++
struct sstep_gain sstep_ctx::adc_v;
```




<hr>



### variable adv 

_Angle increments applied since the hand-off._ 
```C++
uint16_t sstep_ctx::adv;
```




<hr>



### variable angle\_offset 

_Electrical angle of position 0._ 
```C++
sstep_angle_t sstep_ctx::angle_offset;
```




<hr>



### variable angle\_shift 

_14 - log2(microsteps)._ 
```C++
uint8_t sstep_ctx::angle_shift;
```




<hr>



### variable cfg 

_Copy of the configuration._ 
```C++
struct sstep_config sstep_ctx::cfg;
```




<hr>



### variable compare 

_Last compare values._ 
```C++
uint16_t sstep_ctx::compare[SSTEP_LEGS_MAX];
```




<hr>



### variable cur 

_Setpoint in use._ 
```C++
struct sstep_setpoint sstep_ctx::cur;
```




<hr>



### variable emf 

_Filtered back-EMF, d and q axis [pu V]._ 
```C++
sstep_integ_t sstep_ctx::emf[2];
```




<hr>



### variable emf\_a 

_L / Ts + R / 2 [pu V / pu A]._ 
```C++
struct sstep_gain sstep_ctx::emf_a;
```




<hr>



### variable emf\_alpha 

_Ts / tau of the back-EMF filter._ 
```C++
struct sstep_gain sstep_ctx::emf_alpha;
```




<hr>



### variable emf\_b 

_L / Ts - R / 2 [pu V / pu A]._ 
```C++
struct sstep_gain sstep_ctx::emf_b;
```




<hr>



### variable emf\_off 

_Mean back-EMF at rest, d and q [mV \* 2^16]._ 
```C++
int64_t sstep_ctx::emf_off[2];
```




<hr>



### variable faults 

_Active faults._ 
```C++
volatile uint16_t sstep_ctx::faults;
```




<hr>



### variable ff\_cmd 

_Back-EMF of vel\_cmd [pu V]._ 
```C++
sstep_real_t sstep_ctx::ff_cmd;
```




<hr>



### variable ff\_gain 

_Feed-forward per microstep/s [pu]._ 
```C++
struct sstep_gain sstep_ctx::ff_gain;
```




<hr>



### variable hold\_active 

_Hold current applied._ 
```C++
bool sstep_ctx::hold_active;
```




<hr>



### variable hold\_ticks 

_Slow periods at standstill before hold._ 
```C++
uint32_t sstep_ctx::hold_ticks;
```




<hr>



### variable i\_hold 

_Hold current [pu]._ 
```C++
sstep_real_t sstep_ctx::i_hold;
```




<hr>



### variable i\_meas 

_Last measured phase currents [pu]._ 
```C++
sstep_real_t sstep_ctx::i_meas[2];
```




<hr>



### variable i\_prev 

_Phase currents of the previous period [pu]._ 
```C++
sstep_real_t sstep_ctx::i_prev[2];
```




<hr>



### variable i\_run 

_Run current [pu]._ 
```C++
sstep_real_t sstep_ctx::i_run;
```




<hr>



### variable i\_trip\_counts 

_Overcurrent trip [ADC counts from zero]._ 
```C++
uint16_t sstep_ctx::i_trip_counts;
```




<hr>



### variable integ 

_PI integrators [pu V]._ 
```C++
sstep_integ_t sstep_ctx::integ[2];
```




<hr>



### variable inv\_vbus 

_Last V\_base / bus voltage._ 
```C++
struct sstep_gain sstep_ctx::inv_vbus;
```




<hr>



### variable kb 

_Anti-windup gain Ts / Tt._ 
```C++
struct sstep_gain sstep_ctx::kb;
```




<hr>



### variable ki 

_Integral gain per fast loop period._ 
```C++
struct sstep_gain sstep_ctx::ki;
```




<hr>



### variable kp 

_Proportional gain [pu V / pu A]._ 
```C++
struct sstep_gain sstep_ctx::kp;
```




<hr>



### variable mo 

_Motion generator._ 
```C++
struct sstep_motion sstep_ctx::mo;
```




<hr>



### variable r\_pu 

_Phase resistance [pu] (voltage drive)._ 
```C++
struct sstep_gain sstep_ctx::r_pu;
```




<hr>



### variable ratio 

_Fast loop periods per slow loop period._ 
```C++
uint16_t sstep_ctx::ratio;
```




<hr>



### variable ready 

_SSTEP\_READY after a successful_ [_**sstep\_init()**_](group__sstep__api.md#function-sstep_init) _._
```C++
uint32_t sstep_ctx::ready;
```




<hr>



### variable sp 

_Setpoints; the fast loop owns sp[sp\_active]._ 
```C++
struct sstep_setpoint sstep_ctx::sp[2];
```




<hr>



### variable sp\_active 

_Buffer in use by the fast loop._ 
```C++
volatile uint8_t sstep_ctx::sp_active;
```




<hr>



### variable sp\_pending 

_1: sp[1 - sp\_active] is complete and new._ 
```C++
volatile uint8_t sstep_ctx::sp_pending;
```




<hr>



### variable stall\_count 

_Stall detector up/down counter._ 
```C++
uint32_t sstep_ctx::stall_count;
```




<hr>



### variable stall\_emf\_mv 

_Back-EMF of stall\_speed [mV]._ 
```C++
int32_t sstep_ctx::stall_emf_mv;
```




<hr>



### variable stall\_ticks 

_Slow periods of stall to trip; 0 = detection off._ 
```C++
uint32_t sstep_ctx::stall_ticks;
```




<hr>



### variable state 

_Driver state._ 
```C++
enum sstep_state sstep_ctx::state;
```




<hr>



### variable still 

_Slow periods at standstill._ 
```C++
uint32_t sstep_ctx::still;
```




<hr>



### variable v\_hist 

_Applied phase voltages computed 1 and 2 periods ago [pu V]._ 
```C++
sstep_acc_t sstep_ctx::v_hist[2][2];
```




<hr>



### variable vbus\_max\_raw 

_Overvoltage trip [ADC counts], 0 = off._ 
```C++
uint16_t sstep_ctx::vbus_max_raw;
```




<hr>



### variable vbus\_min\_raw 

_Undervoltage trip [ADC counts], 0 = off._ 
```C++
uint16_t sstep_ctx::vbus_min_raw;
```




<hr>



### variable vbus\_raw 

_Last bus voltage [ADC counts]._ 
```C++
volatile uint16_t sstep_ctx::vbus_raw;
```




<hr>



### variable vel\_cmd 

_Commanded velocity of the period in use [microsteps/s]._ 
```C++
int32_t sstep_ctx::vel_cmd;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_internal.h`

