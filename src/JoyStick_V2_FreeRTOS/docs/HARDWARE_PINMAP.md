# Reviewed MCU Pin Map

For this firmware pass the KiCad design is the electrical source of truth.

The legacy `.ioc` is not.

That distinction matters because several of the original generated peripheral assignments disagree with the actual board.

| Function | STM32F446VET6 | Current Phase-1 use |
|---|---|---|
| Joystick Y | PA0 / ADC1_IN0 | ADC scan rank 1 |
| Joystick X | PA1 / ADC1_IN1 | ADC scan rank 2 |
| LED latch 1 | PA4 | GPIO output, low at startup |
| LED SPI clock | PA5 / SPI1_SCK | SPI clock |
| LED blank 2 | PA6 | GPIO output, high/blanked |
| LED SPI data | PA7 / SPI1_MOSI | SPI data |
| LED blank 1 | PC4 | GPIO output, high/blanked |
| LED latch 2 | PC5 | GPIO output, low |
| MAX3535 receiver enable | PC8 | GPIO control |
| MAX3535 driver enable | PC9 | GPIO low, transmitter disabled |
| MAX3535 DI | PE7 | safe GPIO output; UART blocked |
| MAX3535 RO1 | PE8 | GPIO input; UART blocked |
| Buttons 1–4 | PE9–PE12 | active-low inputs, external pull-ups |
| 3.3 V regulator PG | PE13 | active-high input |
| Rotary 1 | PD8, PD10, PD12 | active-low inputs |
| Rotary 2 | PD9, PD11, PD13 | active-low inputs |
| HSE | PH0 / PH1 | 8 MHz crystal |

## Why UART5 is excluded

The MAX3535 transmit input is wired to PE7 and receive output is wired to PE8.

On the STM32F446, the relevant UART5 assignment is:

```text
PE7 = RX
PE8 = TX
```

The directions do not match.

Phase 1 therefore does not initialize UART5 and does not create a software workaround that hides the board problem.

PC9 keeps the MAX3535 transmitter disabled.

See `HARDWARE_BLOCKERS.md` for the engineering consequence.
