# Examples

| Example | Role | Platform | Built and run in CI |
| --- | --- | --- | --- |
| [Bare metal](#bare-metal) | Device | Any MCU; runs on a PC against a simulated board | Yes, as a self-checking test |
| [Host serial port](#host-serial-port) | Controller | Linux, macOS, Windows | Built |
| [Zephyr](#zephyr) | Device | Any Zephyr board | No, needs a Zephyr workspace |

```bash
cmake --preset "[Examples]Stepper Motor Control"
cmake --build "build/[Examples]Stepper Motor Control"
ctest --test-dir "build/[Examples]Stepper Motor Control" -L example --output-on-failure
```

## Bare metal

## Host serial port

## Zephyr
