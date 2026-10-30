

# File sstep\_motion.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**sstep\_motion.c**](sstep__motion_8c.md)

[Go to the source code of this file](sstep__motion_8c_source.md)



* `#include "sstep_internal.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**sstep\_isqrt64**](#function-sstep_isqrt64) (uint64\_t x) <br>_Integer square root: floor(sqrt(x))._  |
|  uint8\_t | [**sstep\_motion\_step**](#function-sstep_motion_step) (struct [**sstep\_motion**](structsstep__motion.md) \* mo) <br>_One slow loop period of the motion generator: trapezoidal velocity profile with one-period look-ahead braking for moves, exact stop on the target._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**abs\_i32**](#function-abs_i32) (int32\_t v) <br>_\|v\|; velocities are limited to \|v\| &lt; 2^30, so -v cannot overflow._  |
|  int64\_t | [**abs\_i64**](#function-abs_i64) (int64\_t v) <br>_\|v\|; positions differences are far below 2^63._  |
|  int32\_t | [**clamp\_i32**](#function-clamp_i32) (int32\_t v, int32\_t lo, int32\_t hi) <br>_v limited to lo ._  |
|  int64\_t | [**half\_floor**](#function-half_floor) (int64\_t v) <br>_floor(v / 2) without shifting a signed value._  |
|  int32\_t | [**target\_velocity**](#function-target_velocity) (const struct [**sstep\_motion**](structsstep__motion.md) \* mo) <br>_Velocity the profile aims at in this period._  |


























## Public Functions Documentation




### function sstep\_isqrt64 

_Integer square root: floor(sqrt(x))._ 
```C++
uint32_t sstep_isqrt64 (
    uint64_t x
) 
```





**Parameters:**


* `x` Value. 



**Returns:**

floor(sqrt(x)). 





        

<hr>



### function sstep\_motion\_step 

_One slow loop period of the motion generator: trapezoidal velocity profile with one-period look-ahead braking for moves, exact stop on the target._ 
```C++
uint8_t sstep_motion_step (
    struct sstep_motion * mo
) 
```





**Parameters:**


* `mo` Motion state; pos and vel are updated. 



**Returns:**

SSTEP\_MOTION\_DONE when a move reached its target in this period, else 0. 





        

<hr>
## Public Static Functions Documentation




### function abs\_i32 

_\|v\|; velocities are limited to \|v\| &lt; 2^30, so -v cannot overflow._ 
```C++
static int32_t abs_i32 (
    int32_t v
) 
```




<hr>



### function abs\_i64 

_\|v\|; positions differences are far below 2^63._ 
```C++
static int64_t abs_i64 (
    int64_t v
) 
```




<hr>



### function clamp\_i32 

_v limited to lo ._ 
```C++
static int32_t clamp_i32 (
    int32_t v,
    int32_t lo,
    int32_t hi
) 
```



. hi. 


        

<hr>



### function half\_floor 

_floor(v / 2) without shifting a signed value._ 
```C++
static int64_t half_floor (
    int64_t v
) 
```




<hr>



### function target\_velocity 

_Velocity the profile aims at in this period._ 
```C++
static int32_t target_velocity (
    const struct sstep_motion * mo
) 
```



A move aims at the largest v\_new for which the distance of this period, (v + v\_new) / 2, plus the braking distance from v\_new, v\_new^2 / (2 dec), fits into the distance left: v\_new = (-dec + sqrt(dec^2 + 8 dec \|rem\| - 4 dec \|v\|)) / 2. No overshoot; the velocity changes by at most acc or dec per period, except in the last period of a move, where the remaining v &lt;= 2 dec (a few microsteps per second) drops to zero on the target. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_motion.c`

