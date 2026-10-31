

# Struct sstep



[**ClassList**](annotated.md) **>** [**sstep**](structsstep.md)



_Driver instance._ [More...](#detailed-description)

* `#include <sstep_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**opaque**](#variable-opaque)  <br>_Private driver state._  |












































## Detailed Description


Allocated by the application (static, stack or pool) and initialised by [**sstep\_init()**](group__sstep__api.md#function-sstep_init). The content is private to the library: use the functions of sstep.h only. 


    
## Public Attributes Documentation




### variable opaque 

_Private driver state._ 
```C++
uint64_t sstep::opaque[SSTEP_INSTANCE_WORDS];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

