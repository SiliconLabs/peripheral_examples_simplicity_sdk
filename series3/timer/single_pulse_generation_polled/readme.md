# Peripheral Examples - TIMER Single Pulse Gerenation Polled #

## Summary ##

This project demonstrates polled single pulse generation using TIMER output compare. Capture/compare channel 0 is configured to toggle the GPIO output pin specified below on each compare event. The first compare event is set for NUM_SEC_DELAY seconds after the TIMER starts counting. The pin is driven high when the match occurs, and the second compare event is then set to occur PULSE_WIDTH milliseconds after the first compare event, and to toggle the GPIO output low.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Output Compare Mode

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Use an oscilloscope to measure the GPIO pin specified below.
3. A 100 ms pulse should be generated after 3.5 seconds.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)
