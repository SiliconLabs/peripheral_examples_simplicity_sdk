# Peripheral Examples - TIMER Single Edge Output Compare #

## Summary ##

This project demonstrates single output compare using the TIMER peripheral. TIMER0 is initialized for output compare on capture/compare channel 0, which is routed to the GPIO pin. The compare value is set such that after 3 seconds, the GPIO pin is set high.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Input Capture Mode

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Use an oscilloscope to measure the GPIO pin specified below.
3. The pin should go high after 3 seconds.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)
