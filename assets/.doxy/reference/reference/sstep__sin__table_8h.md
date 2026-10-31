

# File sstep\_sin\_table.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_sin\_table.h**](sstep__sin__table_8h.md)

[Go to the source code of this file](sstep__sin__table_8h_source.md)



* `#include "sstep_math.h"`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const [**sstep\_real\_t**](group__sstep__math.md#typedef-sstep_real_t) | [**sstep\_sin\_table**](#variable-sstep_sin_table)  <br>_sin(k \* 90 deg / N), k = 0 ._  |











































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SSTEP\_SIN\_TABLE\_N**](sstep__sin__table_8h.md#define-sstep_sin_table_n)  `((uint32\_t)1U &lt;&lt; [**SSTEP\_CFG\_SINE\_TABLE\_BITS**](group__sstep__config.md#define-sstep_cfg_sine_table_bits))`<br>_Number of table intervals per quarter turn._  |
| define  | [**SSTEP\_SIN\_TABLE\_SIZE**](sstep__sin__table_8h.md#define-sstep_sin_table_size)  `(SSTEP\_SIN\_TABLE\_N + 1U)`<br>_Number of table entries: sin(0) ._  |

## Public Attributes Documentation




### variable sstep\_sin\_table 

_sin(k \* 90 deg / N), k = 0 ._ 
```C++
const sstep_real_t sstep_sin_table[SSTEP_SIN_TABLE_SIZE];
```



. N. Read-only (flash). 


        

<hr>
## Macro Definition Documentation





### define SSTEP\_SIN\_TABLE\_N 

_Number of table intervals per quarter turn._ 
```C++
#define SSTEP_SIN_TABLE_N `((uint32_t)1U << SSTEP_CFG_SINE_TABLE_BITS )`
```




<hr>



### define SSTEP\_SIN\_TABLE\_SIZE 

_Number of table entries: sin(0) ._ 
```C++
#define SSTEP_SIN_TABLE_SIZE `(SSTEP_SIN_TABLE_N + 1U)`
```



. sin(90 deg) inclusive. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_sin_table.h`

