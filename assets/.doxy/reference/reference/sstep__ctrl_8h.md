

# File sstep\_ctrl.h



[**FileList**](files.md) **>** [**inc**](dir_bfccd401955b95cf8c75461437045ac0.md) **>** [**sstep\_ctrl.h**](sstep__ctrl_8h.md)

[Go to the source code of this file](sstep__ctrl_8h_source.md)



* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "sstep_config.h"`
* `#include "sstep_defs.h"`
* `#include "sstep_math.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**sstep**](structsstep.md) <br>_Driver instance._  |
| struct | [**sstep\_callbacks**](structsstep__callbacks.md) <br>_Hardware callbacks._  |
| struct | [**sstep\_config**](structsstep__config.md) <br>_Complete configuration given to_ [_**sstep\_init()**_](group__sstep__ctrl.md#function-sstep_init) _._ |
| struct | [**sstep\_ctrl\_cfg**](structsstep__ctrl__cfg.md) <br>_Control and motion parameters._  |
| struct | [**sstep\_motion**](structsstep__motion.md) <br>_Motion generator state._  |
| struct | [**sstep\_motor\_cfg**](structsstep__motor__cfg.md) <br>_Motor parameters, from the datasheet or measured._  |
| struct | [**sstep\_sample**](structsstep__sample.md) <br>_Raw measurements of one fast loop period, as read from the ADC._  |
| struct | [**sstep\_setpoint**](structsstep__setpoint.md) <br>_Setpoint handed from the slow loop to the fast loop._  |
| struct | [**sstep\_stage\_cfg**](structsstep__stage__cfg.md) <br>_Power stage and measurement parameters._  |
| struct | [**sstep\_status**](structsstep__status.md) <br>_Snapshot returned by_ [_**sstep\_get\_status()**_](group__sstep__ctrl.md#function-sstep_get_status) _, in engineering units._ |



















































------------------------------
The documentation for this class was generated from the following file `inc/sstep_ctrl.h`

