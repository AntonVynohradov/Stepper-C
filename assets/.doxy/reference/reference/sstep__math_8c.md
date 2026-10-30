

# File sstep\_math.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_math.c**](sstep__math_8c.md)

[Go to the source code of this file](sstep__math_8c_source.md)



* `#include "sstep_math.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include "sstep_defs.h"`
* `#include "sstep_sin_table.h"`







































## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**bit\_length**](#function-bit_length) (uint64\_t v) <br>_Number of significant bits of v (0 for 0); binary search, 6 steps._  |
|  [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**quarter\_sin**](#function-quarter_sin) (uint32\_t pos) <br>_sin(pos) for 0 &lt;= pos &lt; 90 deg (pos in 2^-32 turn), interpolated._  |
|  uint64\_t | [**ratio\_round**](#function-ratio_round) (uint64\_t n, uint64\_t d, uint32\_t s) <br>_round(n \* 2^s / d); the caller guarantees n \* 2^s &lt; 2^63._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**FRAC\_BITS**](sstep__math_8c.md#define-frac_bits)  `(QUADRANT\_BITS - [**SSTEP\_CFG\_SINE\_TABLE\_BITS**](group__sstep__config.md#define-sstep_cfg_sine_table_bits))`<br>_Bits of the angle below the table index (interpolation fraction)._  |
| define  | [**QUADRANT\_BITS**](sstep__math_8c.md#define-quadrant_bits)  `30U`<br>_Bits of an angle inside one quadrant._  |
| define  | [**QUADRANT\_MASK**](sstep__math_8c.md#define-quadrant_mask)  `0x3FFFFFFFUL`<br>_Mask of the angle inside one quadrant._  |
| define  | [**RATIO64\_LIMIT**](sstep__math_8c.md#define-ratio64_limit)  `((int64\_t)1 &lt;&lt; 62)`<br>_Operand limit of_ [_**sstep\_gain\_from\_ratio64()**_](group__sstep__math.md#function-sstep_gain_from_ratio64) _: 2^62._ |

## Public Static Functions Documentation




### function bit\_length 

_Number of significant bits of v (0 for 0); binary search, 6 steps._ 
```C++
static uint32_t bit_length (
    uint64_t v
) 
```




<hr>



### function quarter\_sin 

_sin(pos) for 0 &lt;= pos &lt; 90 deg (pos in 2^-32 turn), interpolated._ 
```C++
static sstep_real_t quarter_sin (
    uint32_t pos
) 
```



The index is at most N - 1, so idx + 1 is inside the table (N + 1 entries). 


        

<hr>



### function ratio\_round 

_round(n \* 2^s / d); the caller guarantees n \* 2^s &lt; 2^63._ 
```C++
static uint64_t ratio_round (
    uint64_t n,
    uint64_t d,
    uint32_t s
) 
```




<hr>
## Macro Definition Documentation





### define FRAC\_BITS 

_Bits of the angle below the table index (interpolation fraction)._ 
```C++
#define FRAC_BITS `(QUADRANT_BITS - SSTEP_CFG_SINE_TABLE_BITS )`
```




<hr>



### define QUADRANT\_BITS 

_Bits of an angle inside one quadrant._ 
```C++
#define QUADRANT_BITS `30U`
```




<hr>



### define QUADRANT\_MASK 

_Mask of the angle inside one quadrant._ 
```C++
#define QUADRANT_MASK `0x3FFFFFFFUL`
```




<hr>



### define RATIO64\_LIMIT 

_Operand limit of_ [_**sstep\_gain\_from\_ratio64()**_](group__sstep__math.md#function-sstep_gain_from_ratio64) _: 2^62._
```C++
#define RATIO64_LIMIT `((int64_t)1 << 62)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_math.c`

