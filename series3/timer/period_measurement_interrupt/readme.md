# Peripheral Examples - TIMER Period Measurement Interrupt #

## Summary ##

This project demonstrates period measurement using TIMER0.  The module is configured to request an interrupt after capturing a pair of falling edges. In the IRQ handler, in addition to saving the two edge times, the overflow flag is checked in order to account for two edges that span the time during which the counter rolls over from 0xFFFFFFFF (TIMER0 is 32 bits wide) to 0.

Upon exiting the IRQ handler, the period is calculated and returned as an integer value in microseconds, thus the measuredPeriod value will show 1000 for an input signal with a period of 1 kHz.

> Note: The range of frequencies this program can measure accurately is limited by the selected frequency of the EM01GRPACLK, prescaling of the local TIMER clock, and the counter width of the selected TIMER, e.g. TIMER0 is 32 bits wide while other TIMERs are generally 16 bits wide.

## Peripherals used ##

* EM01GRPACLK - Sourced from HFRCODPLL @ 38 MHz
* TIMER0 - Input Capture Mode

## How to Test ##

1. Build the project and download it to the Starter Kit.
2. Connect a periodic signal to the GPIO pin specified below.
3. Go into debug mode and click run.
4. View the measuredPeriod (time period in us) global variable in the debugger.
   
> Note: The first pass through app_process_action() returns measuredPeriod equal to zero, as this executes prior to power manager entering EM1. Subsequent iterations through app_process_action() return measuredPeriod equal to time period between two falling clock edge captures by TIMER0, triggered by interrupt.

## Hardware & Connections ##

* Board: Silicon Labs SixG301 Radio Board (BRD4407A) + Wireless Pro Kit Mainboard
    * Device: SIMG301M104LIL
        * PA06 - TIM0_CC0 (Expansion Header Pin 11, P08)
