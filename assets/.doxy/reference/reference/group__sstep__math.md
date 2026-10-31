

# Group sstep\_math



[**Modules**](modules.md) **>** [**sstep\_math**](group__sstep__math.md)



_Types and operations that hide whether the library computes in fixed point or in float._ [More...](#detailed-description)
















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**sstep\_gain**](structsstep__gain.md) <br>_Gain of any magnitude: value = mant / 2^15 \* 2^exp._  |
| struct | [**sstep\_sincos**](structsstep__sincos.md) <br>_Sine and cosine of one angle, see_ [_**sstep\_sin\_cos()**_](group__sstep__math.md#function-sstep_sin_cos) _._ |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef int32\_t | [**sstep\_acc\_t**](#typedef-sstep_acc_t)  <br>_Per-unit accumulator (Q15 in 32 bits), used by integrators and filters._  |
| typedef uint32\_t | [**sstep\_angle\_t**](#typedef-sstep_angle_t)  <br>_Electrical angle: one full electrical turn (4 full steps) = 2^32._  |
| typedef int32\_t | [**sstep\_integ\_t**](#typedef-sstep_integ_t)  <br>_Integrator: Q27 in 32 bits (range +/-16 pu, resolution 7.5e-9 pu)._  |
| typedef int16\_t | [**sstep\_real\_t**](#typedef-sstep_real_t)  <br>_Per-unit signal (Q15)._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  int | [**sstep\_gain\_from\_ratio**](#function-sstep_gain_from_ratio) (int32\_t num, int32\_t den, struct [**sstep\_gain**](structsstep__gain.md) \* gain) <br>_Create a gain equal to num / den._  |
|  int | [**sstep\_gain\_from\_ratio64**](#function-sstep_gain_from_ratio64) (int64\_t num, int64\_t den, struct [**sstep\_gain**](structsstep__gain.md) \* gain) <br>_Create a gain equal to num / den with 64-bit operands, for gains computed from several engineering values (for example L \* wc \* I\_base / V\_base)._  |
|  int | [**sstep\_real\_from\_ratio**](#function-sstep_real_from_ratio) (int32\_t num, int32\_t den, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) \* out) <br>_Convert num / den to a per-unit signal, rounded and saturated._  |
|  int32\_t | [**sstep\_real\_to\_units**](#function-sstep_real_to_units) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) x, int32\_t base) <br>_Convert a per-unit value back to engineering units: round(x \* base)._  |
|  struct [**sstep\_sincos**](structsstep__sincos.md) | [**sstep\_sin\_cos**](#function-sstep_sin_cos) ([**sstep\_angle\_t**](group__sstep__math.md#typedef-sstep_angle_t) angle) <br>_Sine and cosine of an electrical angle from a quarter-wave table (_ [_**SSTEP\_CFG\_SINE\_TABLE\_BITS**_](group__sstep__config.md#define-sstep_cfg_sine_table_bits) _) with linear interpolation._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_abs**](#function-sstep_abs) ([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) a) <br>_Saturating \|a\|._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_acc\_abs**](#function-sstep_acc_abs) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) a) <br>_Saturating \|a\| of an accumulator._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_acc\_add**](#function-sstep_acc_add) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) a, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) b) <br>_Saturating accumulator a + b._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_acc\_clamp**](#function-sstep_acc_clamp) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) x, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) lo, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) hi) <br>_Accumulator limited to lo ._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_acc\_from\_real**](#function-sstep_acc_from_real) ([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) a) <br>_Convert a signal to an accumulator (exact)._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_acc\_halve**](#function-sstep_acc_halve) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) a) <br>_a / 2 (rounds toward minus infinity)._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_acc\_in**](#function-sstep_acc_in) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) x) <br>_Limit an accumulator to +/-_ [_**SSTEP\_ACC\_IN\_MAX**_](group__sstep__math.md#define-sstep_acc_in_max) _._ |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_acc\_scale**](#function-sstep_acc_scale) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) x, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) k) <br>_x \* k for \|x\| &lt; 2 pu (saturated first), rounded._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_acc\_to\_real**](#function-sstep_acc_to_real) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) a) <br>_Convert an accumulator to a signal, saturating._  |
|  uint16\_t | [**sstep\_acc\_to\_ticks**](#function-sstep_acc_to_ticks) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) duty, uint16\_t period) <br>_Duty cycle (0 ._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_add**](#function-sstep_add) ([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) a, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) b) <br>_Saturating a + b._  |
|  int32\_t | [**sstep\_asr**](#function-sstep_asr) (int32\_t x, uint32\_t n) <br>_Arithmetic shift right (rounds toward minus infinity), written with unsigned shifts only (MISRA C:2012 Rule 10.1); compilers emit one ASR instruction._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_clamp**](#function-sstep_clamp) ([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) x, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) lo, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) hi) <br>_x limited to lo ._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_div\_unit**](#function-sstep_div_unit) ([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) num, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) den) <br>_num / den for 0 &lt;= num &lt;= den, den &gt; 0, as a signal in 0 ._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_gain\_mul**](#function-sstep_gain_mul) (const struct [**sstep\_gain**](structsstep__gain.md) \* g, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) x) <br>_Gain times signal: g \* x, rounded to nearest._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_gain\_mul\_acc**](#function-sstep_gain_mul_acc) (const struct [**sstep\_gain**](structsstep__gain.md) \* g, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) x) <br>_Gain times accumulator: g \* x for \|x\| &lt; 4 pu (saturated first)._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_gain\_mul\_int**](#function-sstep_gain_mul_int) (const struct [**sstep\_gain**](structsstep__gain.md) \* g, int32\_t n) <br>_Gain times integer: g \* n as a per-unit accumulator, saturating._  |
|  [**sstep\_integ\_t**](group__sstep__math.md#typedef-sstep_integ_t) | [**sstep\_integ\_add**](#function-sstep_integ_add) ([**sstep\_integ\_t**](group__sstep__math.md#typedef-sstep_integ_t) integ, const struct [**sstep\_gain**](structsstep__gain.md) \* g, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) x) <br>_Integrator + g \* x (x a signal), saturating._  |
|  [**sstep\_integ\_t**](group__sstep__math.md#typedef-sstep_integ_t) | [**sstep\_integ\_add\_acc**](#function-sstep_integ_add_acc) ([**sstep\_integ\_t**](group__sstep__math.md#typedef-sstep_integ_t) integ, const struct [**sstep\_gain**](structsstep__gain.md) \* g, [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) x) <br>_Integrator + g \* x (x an accumulator, \|x\| &lt; 4 pu), saturating._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**sstep\_integ\_to\_acc**](#function-sstep_integ_to_acc) ([**sstep\_integ\_t**](group__sstep__math.md#typedef-sstep_integ_t) integ) <br>_Integrator value as an accumulator (rounded)._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_mul**](#function-sstep_mul) ([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) a, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) b) <br>_a \* b, rounded to nearest; (-1.0) \* (-1.0) saturates to +0.99997._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_neg**](#function-sstep_neg) ([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) a) <br>_Saturating -a (-(-1.0) gives +0.99997)._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_sat**](#function-sstep_sat) (int32\_t x) <br>_Saturate a 32-bit value to_ [_**sstep\_real\_t**_](group__sstep__math.md#typedef-sstep_real_t) _._ |
|  int32\_t | [**sstep\_shift\_round**](#function-sstep_shift_round) (int32\_t p, int32\_t sh) <br>_Shift with rounding: sh &gt; 0 divides by 2^sh (round half up), sh &lt; 0 multiplies by 2^-sh with saturation._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_sub**](#function-sstep_sub) ([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) a, [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) b) <br>_Saturating a - b._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SSTEP\_ACC\_C**](group__sstep__math.md#define-sstep_acc_c) (x) `(([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t))((x) \* 32768.0))`<br>_Accumulator constant from a floating literal (initializers only)._  |
| define  | [**SSTEP\_ACC\_HALF**](group__sstep__math.md#define-sstep_acc_half)  `(([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t))16384)`<br>_0.5 as an accumulator._  |
| define  | [**SSTEP\_ACC\_IN\_MAX**](group__sstep__math.md#define-sstep_acc_in_max)  `(([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t))131071)`<br>_Largest magnitude accepted by_ [_**sstep\_gain\_mul\_acc()**_](group__sstep__math.md#function-sstep_gain_mul_acc) _and the integrator updates from accumulators (4 pu - 2^-15); larger inputs are saturated._ |
| define  | [**SSTEP\_ACC\_MAX**](group__sstep__math.md#define-sstep_acc_max)  `(([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t))INT32\_MAX)`<br>_Largest_ [_**sstep\_acc\_t**_](group__sstep__math.md#typedef-sstep_acc_t) _(+65536 - 2^-15)._ |
| define  | [**SSTEP\_ACC\_MIN**](group__sstep__math.md#define-sstep_acc_min)  `(([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t))INT32\_MIN)`<br>_Smallest_ [_**sstep\_acc\_t**_](group__sstep__math.md#typedef-sstep_acc_t) _(-65536)._ |
| define  | [**SSTEP\_ACC\_ONE**](group__sstep__math.md#define-sstep_acc_one)  `(([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t))32768)`<br>_1.0 as an accumulator._  |
| define  | [**SSTEP\_ACC\_SCALE\_MAX**](group__sstep__math.md#define-sstep_acc_scale_max)  `(([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t))65535)`<br>_Largest magnitude accepted by_ [_**sstep\_acc\_scale()**_](group__sstep__math.md#function-sstep_acc_scale) _(2 pu - 2^-15)._ |
| define  | [**SSTEP\_ACC\_ZERO**](group__sstep__math.md#define-sstep_acc_zero)  `(([**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t))0)`<br>_Zero as an accumulator._  |
| define  | [**SSTEP\_GAIN\_EXP\_MAX**](group__sstep__math.md#define-sstep_gain_exp_max)  `15`<br>_Largest gain exponent (gains up to 32767)._  |
| define  | [**SSTEP\_GAIN\_EXP\_MIN**](group__sstep__math.md#define-sstep_gain_exp_min)  `(-16)`<br>_Smallest gain exponent._  |
| define  | [**SSTEP\_INTEG\_FRAC\_BITS**](group__sstep__math.md#define-sstep_integ_frac_bits)  `27`<br>_Fractional bits of_ [_**sstep\_integ\_t**_](group__sstep__math.md#typedef-sstep_integ_t) _._ |
| define  | [**SSTEP\_REAL\_C**](group__sstep__math.md#define-sstep_real_c) (x) `/* multi line expression */`<br>_Constant from a floating literal, saturated to -1.0 ._  |
| define  | [**SSTEP\_REAL\_FRAC\_BITS**](group__sstep__math.md#define-sstep_real_frac_bits)  `15`<br>_Number of fractional bits of_ [_**sstep\_real\_t**_](group__sstep__math.md#typedef-sstep_real_t) _and_[_**sstep\_acc\_t**_](group__sstep__math.md#typedef-sstep_acc_t) _._ |
| define  | [**SSTEP\_REAL\_MAX**](group__sstep__math.md#define-sstep_real_max)  `(([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t))INT16\_MAX)`<br>_Largest_ [_**sstep\_real\_t**_](group__sstep__math.md#typedef-sstep_real_t) _(+0.99997)._ |
| define  | [**SSTEP\_REAL\_MIN**](group__sstep__math.md#define-sstep_real_min)  `(([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t))INT16\_MIN)`<br>_Smallest_ [_**sstep\_real\_t**_](group__sstep__math.md#typedef-sstep_real_t) _(-1.0)._ |
| define  | [**SSTEP\_REAL\_ZERO**](group__sstep__math.md#define-sstep_real_zero)  `(([**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t))0)`<br>_Zero as a signal._  |

## Detailed Description


The control code is written once against these types and operations; the backend is selected at build time with [**SSTEP\_CFG\_MATH\_BACKEND**](group__sstep__config.md#define-sstep_cfg_math_backend).


**Per-unit signals.** Inside the control loops every electrical signal is normalised to a base value computed by [**sstep\_init()**](group__sstep__api.md#function-sstep_init):
* current: 1.0 = power stage current full scale (`i_full_scale_ma`),
* voltage: 1.0 = nominal bus voltage (`vbus_nominal_mv`),
* duty cycle: 1.0 = 100 %.




Both backends therefore see values in about -1..+1, which makes Q15 exact enough and lets the two backends be compared sample by sample in the tests.



|Type   |Fixed point backend   |Float backend    |
|-----|-----|-----|
|[**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t)   |`int16_t`, Q15 (1.0 = 32768, range -1 .. +0.99997)   |`float`, same range    |
|[**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t)   |`int32_t`, Q15 with 16 integer bits (range +/-65536)   |`float`, same range    |
|[**sstep\_angle\_t**](group__sstep__math.md#typedef-sstep_angle_t)   |`uint32_t`, one electrical turn = 2^32   |same    |
|`struct  sstep_gain`   |Q15 mantissa and binary exponent   |`float`   |






**Same behaviour in both backends.** Every operation saturates to the ranges in the table instead of overflowing, in both backends, so that a test can compare them. Angles wrap by design.


**Speed.** The per-sample operations are `static inline` and branch-light; the fixed point backend uses only 32-bit multiplications (no 64-bit arithmetic), so it suits Cortex-M0. Functions that convert engineering units (`*_from_ratio`, [**sstep\_real\_to\_units()**](group__sstep__math.md#function-sstep_real_to_units)) use 64-bit integers and division: call them at initialisation or from the slow loop, not from the fast loop. 


    
## Public Types Documentation




### typedef sstep\_acc\_t 

_Per-unit accumulator (Q15 in 32 bits), used by integrators and filters._ 
```
typedef int32_t sstep_acc_t;
```




<hr>



### typedef sstep\_angle\_t 

_Electrical angle: one full electrical turn (4 full steps) = 2^32._ 
```
typedef uint32_t sstep_angle_t;
```




<hr>



### typedef sstep\_integ\_t 

_Integrator: Q27 in 32 bits (range +/-16 pu, resolution 7.5e-9 pu)._ 
```
typedef int32_t sstep_integ_t;
```



The extra 12 fractional bits keep small increments (Ki \* Ts \* e is often below one Q15 LSB) instead of rounding them away. 


        

<hr>



### typedef sstep\_real\_t 

_Per-unit signal (Q15)._ 
```
typedef int16_t sstep_real_t;
```




<hr>
## Public Functions Documentation




### function sstep\_gain\_from\_ratio 

_Create a gain equal to num / den._ 
```
int sstep_gain_from_ratio (
    int32_t num,
    int32_t den,
    struct sstep_gain * gain
) 
```



Initialisation only (64-bit division). 

**Parameters:**


* `num` Numerator. 
* `den` Denominator, not 0. 
* `gain` Result. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) Success. 
* [**SSTEP\_ERR\_PARAM**](group__sstep__defs.md#define-sstep_err_param) gain is NULL, den is 0, or \|num / den\| rounds above 32767. 



**Note:**

Every non-zero int32 ratio is at least 2^-31 and is representable; ratios below 2^-16 lose resolution (the mantissa is no longer normalised). 





        

<hr>



### function sstep\_gain\_from\_ratio64 

_Create a gain equal to num / den with 64-bit operands, for gains computed from several engineering values (for example L \* wc \* I\_base / V\_base)._ 
```
int sstep_gain_from_ratio64 (
    int64_t num,
    int64_t den,
    struct sstep_gain * gain
) 
```



Initialisation and slow loop only: at most three 64-bit divisions. 

**Parameters:**


* `num` Numerator, \|num\| &lt; 2^62. 
* `den` Denominator, 0 &lt; \|den\| &lt; 2^62. 
* `gain` Result. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (gain NULL, den 0, an operand out of range, or \|num / den\| rounds above 32767). Ratios below 2^-31 give 0 (fixed point). 




        

<hr>



### function sstep\_real\_from\_ratio 

_Convert num / den to a per-unit signal, rounded and saturated._ 
```
int sstep_real_from_ratio (
    int32_t num,
    int32_t den,
    sstep_real_t * out
) 
```



Initialisation and slow loop only (64-bit division). 

**Parameters:**


* `num` Numerator, for example 1500 mA. 
* `den` Denominator, the base, for example 4000 mA; not 0. 
* `out` Result. 



**Return value:**


* [**SSTEP\_OK**](group__sstep__defs.md#define-sstep_ok) (out NULL or den 0). 




        

<hr>



### function sstep\_real\_to\_units 

_Convert a per-unit value back to engineering units: round(x \* base)._ 
```
int32_t sstep_real_to_units (
    sstep_acc_t x,
    int32_t base
) 
```



Slow loop and status only. 

**Parameters:**


* `x` Per-unit accumulator. 
* `base` Base in engineering units, for example 4000 mA. 



**Returns:**

x \* base, rounded to nearest and saturated to int32\_t. 





        

<hr>



### function sstep\_sin\_cos 

_Sine and cosine of an electrical angle from a quarter-wave table (_ [_**SSTEP\_CFG\_SINE\_TABLE\_BITS**_](group__sstep__config.md#define-sstep_cfg_sine_table_bits) _) with linear interpolation._
```
struct sstep_sincos sstep_sin_cos (
    sstep_angle_t angle
) 
```



Fast loop safe. Quadrant symmetric: sin(a + 90 deg) is bit-exactly cos(a). 

**Parameters:**


* `angle` Angle, 2^32 = one turn. 



**Returns:**

Sine and cosine. 





        

<hr>
## Public Static Functions Documentation




### function sstep\_abs 

_Saturating \|a\|._ 
```
static inline sstep_real_t sstep_abs (
    sstep_real_t a
) 
```




<hr>



### function sstep\_acc\_abs 

_Saturating \|a\| of an accumulator._ 
```
static inline sstep_acc_t sstep_acc_abs (
    sstep_acc_t a
) 
```




<hr>



### function sstep\_acc\_add 

_Saturating accumulator a + b._ 
```
static inline sstep_acc_t sstep_acc_add (
    sstep_acc_t a,
    sstep_acc_t b
) 
```




<hr>



### function sstep\_acc\_clamp 

_Accumulator limited to lo ._ 
```
static inline sstep_acc_t sstep_acc_clamp (
    sstep_acc_t x,
    sstep_acc_t lo,
    sstep_acc_t hi
) 
```



. hi (requires lo &lt;= hi). 


        

<hr>



### function sstep\_acc\_from\_real 

_Convert a signal to an accumulator (exact)._ 
```
static inline sstep_acc_t sstep_acc_from_real (
    sstep_real_t a
) 
```




<hr>



### function sstep\_acc\_halve 

_a / 2 (rounds toward minus infinity)._ 
```
static inline sstep_acc_t sstep_acc_halve (
    sstep_acc_t a
) 
```




<hr>



### function sstep\_acc\_in 

_Limit an accumulator to +/-_ [_**SSTEP\_ACC\_IN\_MAX**_](group__sstep__math.md#define-sstep_acc_in_max) _._
```
static inline sstep_acc_t sstep_acc_in (
    sstep_acc_t x
) 
```




<hr>



### function sstep\_acc\_scale 

_x \* k for \|x\| &lt; 2 pu (saturated first), rounded._ 
```
static inline sstep_acc_t sstep_acc_scale (
    sstep_acc_t x,
    sstep_real_t k
) 
```




<hr>



### function sstep\_acc\_to\_real 

_Convert an accumulator to a signal, saturating._ 
```
static inline sstep_real_t sstep_acc_to_real (
    sstep_acc_t a
) 
```




<hr>



### function sstep\_acc\_to\_ticks 

_Duty cycle (0 ._ 
```
static inline uint16_t sstep_acc_to_ticks (
    sstep_acc_t duty,
    uint16_t period
) 
```



. 1, clamped) to PWM compare ticks, rounded. 


        

<hr>



### function sstep\_add 

_Saturating a + b._ 
```
static inline sstep_real_t sstep_add (
    sstep_real_t a,
    sstep_real_t b
) 
```




<hr>



### function sstep\_asr 

_Arithmetic shift right (rounds toward minus infinity), written with unsigned shifts only (MISRA C:2012 Rule 10.1); compilers emit one ASR instruction._ 
```
static inline int32_t sstep_asr (
    int32_t x,
    uint32_t n
) 
```





**Parameters:**


* `x` Value. 
* `n` Shift, 0..31. 



**Returns:**

floor(x / 2^n). 





        

<hr>



### function sstep\_clamp 

_x limited to lo ._ 
```
static inline sstep_real_t sstep_clamp (
    sstep_real_t x,
    sstep_real_t lo,
    sstep_real_t hi
) 
```



. hi (requires lo &lt;= hi). 


        

<hr>



### function sstep\_div\_unit 

_num / den for 0 &lt;= num &lt;= den, den &gt; 0, as a signal in 0 ._ 
```
static inline sstep_real_t sstep_div_unit (
    sstep_acc_t num,
    sstep_acc_t den
) 
```



. +0.99997 (rounded, 1.0 saturates). One 32-bit division. 


        

<hr>



### function sstep\_gain\_mul 

_Gain times signal: g \* x, rounded to nearest._ 
```
static inline sstep_acc_t sstep_gain_mul (
    const struct sstep_gain * g,
    sstep_real_t x
) 
```





**Parameters:**


* `g` Gain from [**sstep\_gain\_from\_ratio()**](group__sstep__math.md#function-sstep_gain_from_ratio). 
* `x` Signal. 



**Returns:**

Product as an accumulator (cannot overflow: \|g \* x\| &lt; 32768). 





        

<hr>



### function sstep\_gain\_mul\_acc 

_Gain times accumulator: g \* x for \|x\| &lt; 4 pu (saturated first)._ 
```
static inline sstep_acc_t sstep_gain_mul_acc (
    const struct sstep_gain * g,
    sstep_acc_t x
) 
```



The two least significant bits of x are dropped (resolution 1.2e-4 pu). 


        

<hr>



### function sstep\_gain\_mul\_int 

_Gain times integer: g \* n as a per-unit accumulator, saturating._ 
```
static inline sstep_acc_t sstep_gain_mul_int (
    const struct sstep_gain * g,
    int32_t n
) 
```



Used for ADC counts: with g = 1 / (counts per unit), the result is the signal in pu. 

**Parameters:**


* `g` Gain from [**sstep\_gain\_from\_ratio()**](group__sstep__math.md#function-sstep_gain_from_ratio). 
* `n` Integer, -32768 .. 32767 (larger values are saturated first). 



**Returns:**

g \* n in Q15, saturated to the accumulator range. 





        

<hr>



### function sstep\_integ\_add 

_Integrator + g \* x (x a signal), saturating._ 
```
static inline sstep_integ_t sstep_integ_add (
    sstep_integ_t integ,
    const struct sstep_gain * g,
    sstep_real_t x
) 
```




<hr>



### function sstep\_integ\_add\_acc 

_Integrator + g \* x (x an accumulator, \|x\| &lt; 4 pu), saturating._ 
```
static inline sstep_integ_t sstep_integ_add_acc (
    sstep_integ_t integ,
    const struct sstep_gain * g,
    sstep_acc_t x
) 
```




<hr>



### function sstep\_integ\_to\_acc 

_Integrator value as an accumulator (rounded)._ 
```
static inline sstep_acc_t sstep_integ_to_acc (
    sstep_integ_t integ
) 
```




<hr>



### function sstep\_mul 

_a \* b, rounded to nearest; (-1.0) \* (-1.0) saturates to +0.99997._ 
```
static inline sstep_real_t sstep_mul (
    sstep_real_t a,
    sstep_real_t b
) 
```




<hr>



### function sstep\_neg 

_Saturating -a (-(-1.0) gives +0.99997)._ 
```
static inline sstep_real_t sstep_neg (
    sstep_real_t a
) 
```




<hr>



### function sstep\_sat 

_Saturate a 32-bit value to_ [_**sstep\_real\_t**_](group__sstep__math.md#typedef-sstep_real_t) _._
```
static inline sstep_real_t sstep_sat (
    int32_t x
) 
```





**Parameters:**


* `x` Value in Q15. 



**Returns:**

x limited to [**SSTEP\_REAL\_MIN**](group__sstep__math.md#define-sstep_real_min) .. [**SSTEP\_REAL\_MAX**](group__sstep__math.md#define-sstep_real_max). 





        

<hr>



### function sstep\_shift\_round 

_Shift with rounding: sh &gt; 0 divides by 2^sh (round half up), sh &lt; 0 multiplies by 2^-sh with saturation._ 
```
static inline int32_t sstep_shift_round (
    int32_t p,
    int32_t sh
) 
```





**Parameters:**


* `p` Value. 
* `sh` Shift, -31 .. 31. 



**Returns:**

Shifted value. 





        

<hr>



### function sstep\_sub 

_Saturating a - b._ 
```
static inline sstep_real_t sstep_sub (
    sstep_real_t a,
    sstep_real_t b
) 
```




<hr>
## Macro Definition Documentation





### define SSTEP\_ACC\_C 

_Accumulator constant from a floating literal (initializers only)._ 
```
#define SSTEP_ACC_C (
    x
) `(( sstep_acc_t )((x) * 32768.0))`
```




<hr>



### define SSTEP\_ACC\_HALF 

_0.5 as an accumulator._ 
```
#define SSTEP_ACC_HALF `(( sstep_acc_t )16384)`
```




<hr>



### define SSTEP\_ACC\_IN\_MAX 

_Largest magnitude accepted by_ [_**sstep\_gain\_mul\_acc()**_](group__sstep__math.md#function-sstep_gain_mul_acc) _and the integrator updates from accumulators (4 pu - 2^-15); larger inputs are saturated._
```
#define SSTEP_ACC_IN_MAX `(( sstep_acc_t )131071)`
```




<hr>



### define SSTEP\_ACC\_MAX 

_Largest_ [_**sstep\_acc\_t**_](group__sstep__math.md#typedef-sstep_acc_t) _(+65536 - 2^-15)._
```
#define SSTEP_ACC_MAX `(( sstep_acc_t )INT32_MAX)`
```




<hr>



### define SSTEP\_ACC\_MIN 

_Smallest_ [_**sstep\_acc\_t**_](group__sstep__math.md#typedef-sstep_acc_t) _(-65536)._
```
#define SSTEP_ACC_MIN `(( sstep_acc_t )INT32_MIN)`
```




<hr>



### define SSTEP\_ACC\_ONE 

_1.0 as an accumulator._ 
```
#define SSTEP_ACC_ONE `(( sstep_acc_t )32768)`
```




<hr>



### define SSTEP\_ACC\_SCALE\_MAX 

_Largest magnitude accepted by_ [_**sstep\_acc\_scale()**_](group__sstep__math.md#function-sstep_acc_scale) _(2 pu - 2^-15)._
```
#define SSTEP_ACC_SCALE_MAX `(( sstep_acc_t )65535)`
```




<hr>



### define SSTEP\_ACC\_ZERO 

_Zero as an accumulator._ 
```
#define SSTEP_ACC_ZERO `(( sstep_acc_t )0)`
```




<hr>



### define SSTEP\_GAIN\_EXP\_MAX 

_Largest gain exponent (gains up to 32767)._ 
```
#define SSTEP_GAIN_EXP_MAX `15`
```




<hr>



### define SSTEP\_GAIN\_EXP\_MIN 

_Smallest gain exponent._ 
```
#define SSTEP_GAIN_EXP_MIN `(-16)`
```




<hr>



### define SSTEP\_INTEG\_FRAC\_BITS 

_Fractional bits of_ [_**sstep\_integ\_t**_](group__sstep__math.md#typedef-sstep_integ_t) _._
```
#define SSTEP_INTEG_FRAC_BITS `27`
```




<hr>



### define SSTEP\_REAL\_C 

_Constant from a floating literal, saturated to -1.0 ._ 
```
#define SSTEP_REAL_C (
    x
) `/* multi line expression */`
```



. +0.99997. An arithmetic constant expression: use it in initializers of static or `const` objects, where the compiler evaluates it and no floating point code is generated. It is not an integer constant expression (C11 6.6), so it cannot be used in `#if`, `case` or `static_assert`. 


        

<hr>



### define SSTEP\_REAL\_FRAC\_BITS 

_Number of fractional bits of_ [_**sstep\_real\_t**_](group__sstep__math.md#typedef-sstep_real_t) _and_[_**sstep\_acc\_t**_](group__sstep__math.md#typedef-sstep_acc_t) _._
```
#define SSTEP_REAL_FRAC_BITS `15`
```




<hr>



### define SSTEP\_REAL\_MAX 

_Largest_ [_**sstep\_real\_t**_](group__sstep__math.md#typedef-sstep_real_t) _(+0.99997)._
```
#define SSTEP_REAL_MAX `(( sstep_real_t )INT16_MAX)`
```




<hr>



### define SSTEP\_REAL\_MIN 

_Smallest_ [_**sstep\_real\_t**_](group__sstep__math.md#typedef-sstep_real_t) _(-1.0)._
```
#define SSTEP_REAL_MIN `(( sstep_real_t )INT16_MIN)`
```




<hr>



### define SSTEP\_REAL\_ZERO 

_Zero as a signal._ 
```
#define SSTEP_REAL_ZERO `(( sstep_real_t )0)`
```




<hr>

------------------------------


