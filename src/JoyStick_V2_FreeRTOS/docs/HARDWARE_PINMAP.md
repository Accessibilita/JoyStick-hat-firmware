# Phase 1 reviewed MCU pin map

Source: uploaded KiCad 9 project, current `JoyStick-Hat-V2-Re-Imagined.xml` netlist.
The KiCad netlist—not the legacy firmware `.ioc`—was used as the electrical source of truth.

| Function | STM32F446VET6 pin | Phase 1 configuration |
|---|---|---|
| Joystick Y | PA0 / ADC1_IN0 | ADC scan rank 1 |
| Joystick X | PA1 / ADC1_IN1 | ADC scan rank 2 |
| LED latch 1 | PA4 | output, low at startup |
| LED SPI clock | PA5 / SPI1_SCK | SPI output |
| LED blank 2 | PA6 | output, high at startup |
| LED SPI data | PA7 / SPI1_MOSI | SPI output |
| LED blank 1 | PC4 | output, high at startup |
| LED latch 2 | PC5 | output, low at startup |
| MAX3535 receiver enable | PC8 | output, low (`nRE` active) |
| MAX3535 driver enable | PC9 | output, low (driver disabled) |
| MAX3535 DI | PE7 | safe GPIO output low; UART blocked |
| MAX3535 RO1 | PE8 | GPIO input; UART blocked |
| Button 1–4 | PE9–PE12 | external 2.2 kΩ pull-ups, active low |
| 3.3 V regulator PG | PE13 | input, active high |
| Rotary 1 contacts | PD8, PD10, PD12 | external 2.2 kΩ pull-ups, active low |
| Rotary 2 contacts | PD9, PD11, PD13 | external 2.2 kΩ pull-ups, active low |
| HSE | PH0/PH1 | 8 MHz crystal |

## Deliberate UART exclusion

The PCB connects the MAX3535 transmit input `DI` to PE7 and receive output `RO1` to PE8. On
this MCU, UART5 assigns RX to PE7 and TX to PE8. The electrical directions are reversed, so
Phase 1 does not initialize UART5 and keeps the RS-485 driver disabled.
