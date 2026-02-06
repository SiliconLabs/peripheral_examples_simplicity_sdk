# Peripheral Examples - TIMER Frequency Generation #

## Summary ##

This project demonstrates frequency generation using the TIMER peripheral. TIMER0 is initialized for output compare on Compare/Capture channel 0 which is routed to a GPIO Pin. The Top value is set such that on each overflow, the output toggles at the desired frequency: OUT_FREQ initially set to 1000 Hz. 

> NOTE: The range of frequencies the program can generate is limited. The maximal frequency is 1/2 the clock frequency (Setting the top value to 0 causes an overflow on each clock cycle). The minimum frequency is (clock frequency) / (2 * 2^32 * prescale). With a 38 MHz HFRCO, the maximum frequency is 19 MHz and the minimum frequency this example will support it 1 Hz.  Frequencies well below 1 Hz are posible by directly seting the top value of the timer.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Output Compare Mode

## How to Test ##

1. Build the project and download to the Pro Kit
2. Measure waveform on GPIO pin (see board specific pin below).

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)
