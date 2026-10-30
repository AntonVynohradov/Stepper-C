

# Struct sstep\_bridge\_out



[**ClassList**](annotated.md) **>** [**sstep\_bridge\_out**](structsstep__bridge__out.md)



_Result of the power stage mapping._ 

* `#include <sstep_internal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**compare**](#variable-compare)  <br>_Compare values per leg._  |
|  bool | [**limited**](#variable-limited)  <br>_The request was scaled down._  |
|  [**sstep\_acc\_t**](group__sstep__math.md#typedef-sstep_acc_t) | [**v**](#variable-v)  <br>_Applied phase voltages [pu of V\_base]._  |












































## Public Attributes Documentation




### variable compare 

_Compare values per leg._ 
```C++
uint16_t sstep_bridge_out::compare[SSTEP_LEGS_MAX];
```




<hr>



### variable limited 

_The request was scaled down._ 
```C++
bool sstep_bridge_out::limited;
```




<hr>



### variable v 

_Applied phase voltages [pu of V\_base]._ 
```C++
sstep_acc_t sstep_bridge_out::v[2];
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/sstep_internal.h`

